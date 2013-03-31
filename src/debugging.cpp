/*
debugging.cpp - debugger interface is implemented here
Coded by Tibor Djurica Potpara and Maj Smerkol

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stutskInterpreter.h>
#include <boost/asio.hpp>
#include <boost/integer.hpp>

void Debugger::send_message(const void* data, size_t length)
{
	boost::system::error_code error;
	boost::asio::write(*socket, boost::asio::buffer(data,length),
		boost::asio::transfer_all(),error);

	if (error) throw DebuggerException();
}

template<class T>
void Debugger::send_message(T data)
{
	send_message(reinterpret_cast<void*>(&data), sizeof(T));
}

void Debugger::read_message(void* data, size_t length)
{
	boost::system::error_code error;
	size_t readLength = boost::asio::read(*socket, boost::asio::buffer(data, length), 
		boost::asio::transfer_exactly(length), error);

	if (error || (readLength != length)) throw DebuggerException();
}

template<typename T>
void Debugger::read_message(T& data)
{
	read_message(reinterpret_cast<void*>(&data), sizeof(T));
}

template<class T>
void Debugger::dump_list(const T& tokens)
{
	send_message<boost::int64_t>(tokens.size());
	for (auto iter=tokens.cbegin();
		iter != tokens.cend();
		++iter)
	{
		dump_token(*iter);
	}
}

void Debugger::dump_dictionary(const TokenMap& tokens)
{
	send_message<boost::int64_t>(tokens.size());
	for (auto iter=tokens.cbegin();
		iter != tokens.cend();
		++iter)
	{
		send_message<boost::int64_t>(iter->first.size());
		send_message(iter->first.data(),
			iter->first.size());
		dump_token(iter->second);
	}
}

void Debugger::dump_token(const Token& token)
{
	send_message<boost::uint8_t>(
		static_cast<unsigned char>(token.tokenType));
	switch (token.tokenType)
	{
	case T_EMPTY: break;		
	case T_OPERATOR: 	
		send_message<boost::uint8_t>(token.data.operatorType);
		break;
	case T_FUNCCALL: 
		{	int length = strlen(token.data.asFunctionName);
		send_message<boost::uint8_t>(length);
		send_message(token.data.asFunctionName,length);
		break; }
	case T_VARIABLE:
		{	int length = strlen(token.data.asVariable.name);
		send_message<boost::uint8_t>(length);
		send_message(token.data.asVariable.name,length);
		if (token.data.asVariable.context != NULL)
		{
			send_message<boost::uint8_t>(1);
			send_message<boost::uint8_t>(token.data.asVariable.context->functionName.length());
			send_message(token.data.asVariable.context->functionName.data(),
				token.data.asVariable.context->functionName.length());
			auto iter = token.data.asVariable.context->
				variables.find(token.data.asVariable.name);
			if (iter != token.data.asVariable.context->variables.end())
			{
				send_message<boost::uint8_t>(1);
				send_message<boost::uint16_t>(token.index.size());
				for (auto it=token.index.cbegin(); it != token.index.cend(); ++it)
				{
					send_message<boost::int64_t>(*it);
				}
				dump_token(iter->second);
			} 
			else
				send_message<boost::uint8_t>(0);
		}
		else
		{
			send_message<boost::uint8_t>(0);
		}
		break; }
	case T_INTEGER: 	
		send_message<boost::int64_t>(token.data.asInteger);
		break;
	case T_BOOL: 		
		send_message<boost::uint8_t>(token.data.asBool ? 1 : 0);
		break;
	case T_FLOAT:
		send_message<long double>(token.data.asFloat);
		break;
	case T_STRING: 
		send_message<boost::int64_t>(token.asString->size());
		send_message(token.asString->data(),token.asString->size());
		break; 
	case T_CODEBLOCK:
	case T_ARRAY:
		dump_list(*token.asTokenList);
		break;
	case T_HANDLE: break;
	case T_DICTIONARY: 
		dump_dictionary(*token.asDictionary);
		break;
	default: ;
	}
};

void Debugger::message_loop()
{
	boost::uint16_t messageId;
	do 
	{
		read_message<boost::uint16_t>(messageId);
	} while (parse_message(messageId));
};

bool Debugger::parse_message(boost::uint16_t message_id)
{
	if (message_id == 0) // DI_CONTINUE
		return false;

	if (message_id == 0x0001) // DI_STACK_DUMP
	{
		send_message<boost::uint16_t>(0x1000);
		dump_list<TokenStack>(stutskStack);
	}

	if (message_id == 0x0002) // DI_DETACH
	{
		send_message<boost::uint16_t>(0x0003);
		debugging = false;
		return false;
	}

	if (message_id == 0x0003) // DI_TERMINATE
	{
		send_message<boost::uint16_t>(0x0003);
		exitVar = OP_HALT;
		debugging = false;
		return false;
	}

	if (message_id == 0x0004) // DI_LOCATION
	{
		send_message<boost::uint16_t>(0x1001);
		send_message<boost::uint32_t>(errorToken.context_id);
		send_message<boost::uint32_t>(errorToken.lineNum);
		send_message<boost::uint32_t>(errorToken.columnNum);
		send_message<boost::uint32_t>(errorToken.tokenLength);
		send_message<boost::uint32_t>(errorToken.relativePos);
	}

	if (message_id == 0x0005) // DI_SOURCE_CODE
	{
		boost::uint32_t source_context_id;	
		read_message<boost::uint32_t>(source_context_id);

		send_message<boost::uint16_t>(0x1002);

		const ParseContext& ctx = ParseContext::get_by_id(source_context_id);

		send_message<boost::uint32_t>(ctx.FileName.length());
		send_message(ctx.FileName.data(), ctx.FileName.size());
		send_message<boost::uint32_t>(ctx.SourceCode.length());
		send_message(ctx.SourceCode.data(), ctx.SourceCode.size());
	}

	if (message_id == 0x0006) // DI_INJECT
	{
		boost::uint32_t source_code_length;	
		read_message<boost::uint32_t>(source_code_length);

		ParseContext* newContext = ParseContext::newContext();
		newContext->FileName = "<debugger>";

		DebugInfo old_errorToken = errorToken;

		std::vector<char> buffer(source_code_length);
		read_message(&buffer[0], source_code_length);

		newContext->SourceCode = string(buffer.begin(), buffer.end());
		Parser newParser(newContext->SourceCode, newContext);
		TokenList parsed;
		newParser.parse(parsed);
		contextStack.back()->run(parsed,"<debugger>");

		errorToken = old_errorToken;

		send_message<boost::uint16_t>(0x0000);
	}

	if (message_id == 0x0007) // DI_STACK_DUMP
	{
		send_message<boost::uint16_t>(0x1003);
		send_message<boost::uint32_t>(mainContext->variables.size());

		for (TokenMap::const_iterator iter = mainContext->variables.begin(); 
			iter != mainContext->variables.end(); ++iter)
		{
			send_message<boost::uint8_t>(iter->first.length());
			send_message(iter->first.data(), iter->first.size());				
			dump_token(iter->second);
		}
	}

	if (message_id == 0x0008) // DI_STACK_DUMP
	{
		send_message<boost::uint16_t>(0x1004);
		send_message<boost::uint32_t>(contextStack.back()->variables.size());

		for (TokenMap::const_iterator iter = contextStack.back()->variables.begin(); 
			iter != contextStack.back()->variables.end(); ++iter)
		{
			send_message<boost::uint8_t>(iter->first.length());
			send_message(iter->first.data(), iter->first.size());				
			dump_token(iter->second);
		}
	}

	return true;
};


bool Debugger::is_debugging() { return debugging; };

void Debugger::connect(string hostname, int port) 
{
	if (debugging)
		throw StutskException(ET_SYSTEM, "Debugging session is already active");

	debugger_hostname = hostname; 
	debugger_port = port;

	boost::asio::ip::tcp::resolver resolver(io_service);
	string port_string; toString<int>(debugger_port, port_string);
	boost::asio::ip::tcp::resolver::query query(debugger_hostname, port_string);
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	boost::asio::ip::tcp::resolver::iterator end;

	socket = boost::shared_ptr<boost::asio::ip::tcp::socket>
		(new boost::asio::ip::tcp::socket(io_service)); 

	boost::system::error_code error = boost::asio::error::host_not_found;

	while (error && endpoint_iterator != end)
	{
		socket->close();
		socket->connect(*endpoint_iterator++, error);
	}

	if (error)
		throw StutskException(ET_SYSTEM, "Cannot establish a debugging session");

	debugging = true;
};

void Debugger::step()
{
	if (!debugging) return;
	try
	{
		send_message<boost::uint16_t>(0); // step
		message_loop();
	} catch (const DebuggerException& e)
	{
		throw StutskException(ET_SYSTEM, "Connection to debugger broken");
	}
}

void Debugger::step_in(Context* context, const string& name)
{
	if (!debugging) return;
	try
	{
		contextStack.push_back(context);
		send_message<boost::uint16_t>(1); // step-in
		send_message<boost::uint16_t>(name.length());
		send_message(name.data(), name.length());
		message_loop();
	} catch (const DebuggerException& e)
	{
		throw StutskException(ET_SYSTEM, "Connection to debugger broken");
	}
}

void Debugger::step_out()
{
	if (!debugging) return;
	try
	{
		contextStack.pop_back();
		send_message<boost::uint16_t>(2); // step-out
		message_loop();
	} catch (const DebuggerException& e)
	{
		throw StutskException(ET_SYSTEM, "Connection to debugger broken");
	}
}


/*
TBI:
void Debugger::exception(const StutskException& e)
{
	
}
*/

void Debugger::disconnect()
{
	if (!debugging || !socket->is_open()) return;
	{
		send_message<boost::uint16_t>(3);
	}
	socket->close();
	debugging = false;
}

Debugger::~Debugger()
{
	// If the smart pointer has been initialized
	if (socket)
		socket->close();
}