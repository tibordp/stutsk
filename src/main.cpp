/*
main.c - Interpreter for a Stutsk programming language
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

#include <iostream>
#include <memory>
#include <fstream>
#include <cstring>

#include <boost/program_options.hpp>

using namespace std;

#include <stutskInterpreter.h>

#if defined _WIN64
const string stutskVersion = string("Stutsk Win32-64 v1.1 (") + string(__DATE__) + string(")");
#elif defined _WIN32
const string stutskVersion = string("Stutsk Win32 v1.1 (") + string(__DATE__) + string(")");
#elif defined ARM
const string stutskVersion = string("Stutsk ARM v1.1 (") + string(__DATE__) + string(")");
#else
const string stutskVersion = string("Stutsk Linux v1.1 (") + string(__DATE__) + string(")");
#endif
// --------------------- GLOBAL VARIABLES ----------------------------------- //


UserFunctionsMap userFunctions;
StringMapCI environmentVars;
BuiltinFunctionsMap builtinFunctions = BuiltIns::populateFunctions();
OperatorMap stutskOperators = Operators::populateOperators();

TokenStack stutskStack;
Context *mainContext;
OperatorType exitVar;
DebugInfo errorToken;
string sourceCodeText;

vector<string> includePaths, customArguments;

vector<ParseContext> parseContexts;

// --------------------- IMPLEMENTATION ------------------------------------- //


string extractFileName(const string& fn) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	string result = fn.substr(fn.find_last_of("\\") + 1);
#else
	string result = fn.substr(fn.find_last_of("/") + 1);
#endif
	if (result.empty())
		return fn;
	else
		return result;
}

StutskException::StutskException(ExceptionType type, const string& msg) : 
msg_(msg), type_(type), line_number_(errorToken.lineNum) {
		context_ = &parseContexts[errorToken.context_id];
};

StutskException::StutskException(ExceptionType type, const string& msg, 
	int line_number, ParseContext* context) : msg_(msg),
	context_(context), type_(type), line_number_(line_number) {};


string StutskException::getMessage() const {
	return msg_;
}

long StutskException::getLineNumber() const {
	return line_number_;
}

string StutskException::getFileName() const {
	return context_->FileName;
}

string StutskException::getFormattedMessage() const {
	stringstream ss;

	switch (type_) {
	case ET_CUSTOM:
		ss << context_->FileName << ":" << line_number_ << ": ";
		ss << "exception: " << msg_ << "\n";
		break;
	case ET_ERROR:
		ss << context_->FileName << ":" << line_number_ << ": ";
		ss << "error: " << msg_ << "\n";
		break;
	case ET_SYSTEM:
		ss << "system error: " << msg_ << "\n";
		break;
	case ET_WARNING:
		ss << "warning: " << msg_ << "\n";
		break;
	case ET_PARSER:
		ss << context_->FileName << ":" << line_number_ << ": ";
		ss << "parser error: " << msg_ << "\n";
		break;
	default:	
		ss << "unknown error: " << msg_ << "\n";
		break;

	}
	return ss.str();
}


/* copy_token - a routine to copy a token producing independent copies of referenced objects, such
as strings and arrays */
Token copy_token(Token token)
{
	switch (token.tokenType) {
		/* Be careful if you chose to implement  codeblocks that can be modified programatically 
		as they are not copied. */
	case T_ARRAY:
		token.asTokenList = TokenListPtr(new TokenList(*token.asTokenList));
		for (TokenList::iterator iter=token.asTokenList->begin();iter!=token.asTokenList->end();++iter)
			*iter = copy_token(*iter);
		break;
	case T_STRING:
		token.asString = StringPtr(new string(*token.asString));
		break;
	default: ;
	}
	return token;
}

VariableScope Context::findVariableScope(string variableName) {
	VariableScopeMap::iterator iter = variableScopeMap.find(variableName);
	if (iter != variableScopeMap.end())
		return iter->second;
	else
		return VS_AUTO;
}


/* Context::run - a main execution function that iterates through a codeblock and either pushes
tokens to stack or executes them (operators and functions)*/
void Context::run(const TokenList& source, string name) {
	BuiltinFunctionsMap::iterator bIter;
	UserFunctionsMap::iterator uIter;
	Token variable;

	Debugger::get_instance().step_in(this, name);

	for (TokenList::const_iterator it = source.begin(); it != source.end(); ++it) {

		errorToken = *(it->debugInfo);
		Debugger::get_instance().step();

		if (exitVar != OP_INVALID)
			break;

		switch (it->tokenType) {
		case T_OPERATOR:
			Operators::doOperator(this, it->data.operatorType); break;
			break;
		case T_FUNCCALL:
			// Builtin functions may be overriden by user-defined ones, so we check the latter first
			uIter = userFunctions.find(it->data.asFunctionName);

			if (uIter != userFunctions.end()) {
				Context functionContext(uIter->second, this);
				functionContext.functionName = it->data.asFunctionName;
				functionContext.run(uIter->second, it->data.asFunctionName);					
				if (exitVar == OP_EXIT) {
					exitVar = OP_INVALID;
				}
			}
			else {
				bIter = builtinFunctions.find(it->data.asFunctionName);
				if (bIter != builtinFunctions.end()) {
					bIter->second(this);	
				}
				else {
					stringstream ExceptionText;
					ExceptionText << "Unknown function '" <<
						it->data.asFunctionName << "'.";
					throw StutskException(ET_ERROR, ExceptionText.str());
				}
			}
			break;
		case T_ARRAY: {
			TokenStack::size_type oldSize = stutskStack.size();
			run(*it->asTokenList, "<array>");

			Token newArray(T_ARRAY);
			newArray.asTokenList = TokenListPtr(new TokenList());

			// Move tokens from stack to the new array
			if (oldSize < stutskStack.size())
			{
				newArray.asTokenList->insert(
					newArray.asTokenList->end(),
					stutskStack.begin() + oldSize,
					stutskStack.end());
				stutskStack.erase(
					stutskStack.begin() + oldSize,
					stutskStack.end());
			}

			stutskStack.push_back(newArray); 
					  } break;
		case T_VARIABLE:
			switch (findVariableScope(it->data.asVariable.name)) {
			case VS_AUTO:
				variable = *it;
				variable.data.asVariable.context = this;
				break;
			case VS_GLOBAL:
				variable = *it;
				variable.data.asVariable.context = mainContext;
				break;
			case VS_STATIC:
				variable = *it;
				string newName = it->data.asVariable.name;
				newName += "$" + functionName;
				strcpy(variable.data.asVariable.name, newName.c_str());

				variable.data.asVariable.context = mainContext;
				break;
			}
			stutskStack.push_back(variable);
			break;
			// We copy strigns, because they can be mutable (see . operator) but not codeblock, 
			// as they are immutable.
		case T_STRING:
			stutskStack.push_back(copy_token(*it));
			break;
			// If token is an atomic value, we just push it onto a stack
		default:
			stutskStack.push_back(*it);
			break;
		}

		// If exitVar is not unset, we must terminate the execution of the codeblock
		if (exitVar != OP_INVALID)
			break;
	}

	Debugger::get_instance().step_out();
}

/* Parser::getOperatorSymbol - returns a textual representation of an operator from its type */
string Parser::getOperatorSymbol(OperatorType oper) {
	OperatorMap::const_iterator iter;

	for (iter = stutskOperators.begin(); iter != stutskOperators.end(); ++iter)
	{
		if (iter->second == oper) {
			return iter->first;
			break;
		}
	}
	return "";
}

/* Parser::parse - parses a string of Stutsk source and converts it into a codeblock (deque of tokens) */
void Parser::parse(TokenList& retArray) {
	Token currentToken;
	OperatorType operType;

	stutskInteger asInteger;
	stutskFloat asFloat;
	int oldLineNum, oldColumnNum, oldRelativePosition, startPosition;
	string blockSource;

	while (codePosition + 1 <= (signed)sourceCode.length()) {

		currentToken = Token();

		// First we skip all the whitespace.
		while (codePosition + 1 <= (signed)sourceCode.length()) {
			if (isspace(sourceCode[codePosition]))
				advanceCursor();
			else
				break;
		}

		if (codePosition + 1 > (signed)sourceCode.length())
			break;

		currentToken.tokenType = T_EMPTY;
		oldLineNum = linenumber_;
		oldColumnNum = columnnumber_;
		oldRelativePosition = relativeposition_;
		startPosition = codePosition;

		// We make our primary decision based on a first char of the character sequence
		switch (sourceCode[codePosition]) {
		case '"':
			advanceCursor();
			currentToken.tokenType = T_STRING;
			currentToken.asString = StringPtr(new string());
			readString(*currentToken.asString);
			break;

		case '(':
		case '{':
			{
				advanceCursor();
				currentToken.tokenType = 
					sourceCode[codePosition-1] == '(' ? T_ARRAY : T_CODEBLOCK;
				readCodeblock(sourceCode[codePosition-1] == '(', blockSource);

				Parser subparser(blockSource, parseContext, oldLineNum, oldColumnNum+1, oldRelativePosition+1);

				currentToken.asTokenList = TokenListPtr(new TokenList());
				subparser.parse(*currentToken.asTokenList);
				break;
			}
		case '$':
			advanceCursor();
			currentToken.tokenType = T_VARIABLE;
			strcpy(currentToken.data.asVariable.name, readIdentifier().c_str());
			currentToken.data.asVariable.context = NULL;
			break;

		case '#':
		case ';':
			// Comments are one-line only, so we skip all the characters to the newline
			while (codePosition + 1 <= (signed)sourceCode.length()) {
				if ((sourceCode[codePosition] == '\r') ||
					(sourceCode[codePosition] == '\n'))
					break;
				else
					advanceCursor();
			}
			continue;

			// If we cannot make a decision based on a first char, we read caracters to the next whitespace
			// and match the sequence as whole.

		default:
			string tokenString = readToWhite();
			if (tokenString == "TRUE") {
				currentToken.tokenType = T_BOOL;
				currentToken.data.asBool = true;
			}
			else if (tokenString == "FALSE") {
				currentToken.tokenType = T_BOOL;
				currentToken.data.asBool = false;
			}
			else if (tokenString == "NULL") {
				currentToken.tokenType = T_EMPTY;
			}
			else if (Operators::readOperator(tokenString, operType)) {
				currentToken.tokenType = T_OPERATOR;
				currentToken.data.operatorType = operType;
			}
			else if (fromString(tokenString, asInteger)) {
				currentToken.tokenType = T_INTEGER;
				currentToken.data.asInteger = asInteger;
			}
			else if (fromString(tokenString, asFloat)) {
				currentToken.tokenType = T_FLOAT;
				currentToken.data.asFloat = asFloat;
			}
			else {
				currentToken.tokenType = T_FUNCCALL;
				strcpy(currentToken.data.asFunctionName, tokenString.c_str());
			}
		}

		DebugInfo* db = new DebugInfo;

		db->context_id = parseContext->id();
		db->columnNum = oldColumnNum;
		db->lineNum = oldLineNum;
		db->relativePos = oldRelativePosition;
		db->tokenLength = codePosition - startPosition;

		currentToken.debugInfo = db;

		advanceCursor();

		retArray.push_back(currentToken);
	}
}

inline void Parser::advanceCursor() {
	if (codePosition + 1 > (signed)sourceCode.length()) return;
	if (sourceCode[codePosition] == '\n')
	{
		linenumber_++;
		columnnumber_ = 0;
	}
	columnnumber_++;
	relativeposition_++;
	codePosition++;
}

inline bool isIdent(char ch) {
	return (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) ||
		((ch >= '0') && (ch <= '9')) ||  (ch == '_'));
}

/* Parser::readToWhite - reads word - characters till nex whitespace */
string Parser::readToWhite() {
	stringstream result;
	while (codePosition + 1 <= (signed)sourceCode.length()) {
		if (isspace(sourceCode[codePosition]))
			break;
		result << sourceCode[codePosition];
		advanceCursor();
	}
	return result.str();
}

/* Parser::readIdentifier - reads identifier (rejects invalid chars) */
string Parser::readIdentifier() {
	stringstream result;
	while (codePosition + 1 <= (signed)sourceCode.length()) {
		if (isspace(sourceCode[codePosition])) 
			break;
		if (!isIdent(sourceCode[codePosition]))
			throw StutskException(ET_PARSER, "Invalid character in variable name", linenumber_,
			parseContext);
		result << sourceCode[codePosition];
		advanceCursor();
	}
	return result.str();
}

/* Parser::readString - reads string literal from Stutsk source code */
void Parser::readString(string &output) {
	stringstream result;
	bool escapeModifier = false;

	while (codePosition + 1 <= (signed)sourceCode.length()) {
		if (escapeModifier) {
			switch (sourceCode[codePosition]) {
			case '\\':
				result << "\\";
				break;
			case 'n':
				result << "\n";
				break;
			case 'r':
				result << "\r";
				break;
			case 't':
				result << "\t";
				break;
			case '"':
				result << "\"";
				break;
			default:
				throw StutskException(ET_PARSER, "Wrong escape code", 
					linenumber_, parseContext);
				break;
			}
			escapeModifier = false;
		}
		else {
			if (sourceCode[codePosition] == '\\')
				escapeModifier = true;
			else if (sourceCode[codePosition] == '"')
				break;
			else
				result << sourceCode[codePosition];
		}
		advanceCursor();
	}

	if (sourceCode[codePosition] != '"')
		throw StutskException(ET_PARSER, "Unterminated string", linenumber_,
		parseContext);

	advanceCursor();
	output = result.str();
}

/* Parser::stringEscape - adds the escape sequences to a string, so it can be serialized */
std::string Parser::stringEscape(std::string input)
{
	char froms[5] = {'\\','\"','\n','\r','\t'};
	std::string tos[5] = {"\\\\","\\\"","\\n","\\r","\\t"};
	unsigned int i; unsigned long j; bool finished = false;
	stringstream ss;
	for (j=0;j<input.size();j++)
	{
		finished = true;
		for (i=0;i<5;i++)
			if (input[j] == froms[i])
			{ 
				ss << tos[i];
				finished = false; break;
			}
			if (finished) ss << input[j];
	}
	return ss.str();
}

/* Parser::serializeToken - serializes a token into a textual representation, such that if eval-ed, 
it would return the same result. Doesn't serialize array contexts/indices, T_HANDLEs. */
std::string Parser::serializeToken(const Token &token)
{
	string temp; TokenList::iterator iter; stringstream ss;
	switch (token.tokenType) 
	{
	case T_EMPTY: ;
		return "NULL";
	case T_INTEGER:
		if (toString(token.data.asInteger, temp))
			return temp;
		else
			throw StutskException(ET_ERROR, "Error while serializing a value.");
	case T_FLOAT:
		if (toString(token.data.asFloat, temp))
			return temp;
		else
			throw StutskException(ET_ERROR, "Error while serializing a value.");
	case T_BOOL:
		return token.data.asBool ? "TRUE" : "FALSE";
	case T_STRING:
		ss << "\"" << stringEscape(*token.asString) << "\"";
		return ss.str()
			;
	case T_ARRAY:
		ss << "( ";
		for (iter = token.asTokenList->begin();
			iter!=token.asTokenList->end();
			++iter)
			ss << serializeToken(*iter) << " ";
		ss << ")";
		return ss.str();	
	case T_VARIABLE:
		return string("$") + token.data.asVariable.name;
	case T_CODEBLOCK:
		ss << "{ ";
		for (iter = token.asTokenList->begin();
			iter!=token.asTokenList->end();
			++iter)
			ss << serializeToken(*iter) << " ";
		ss << "}";
		return ss.str();				
	case T_OPERATOR:
		return Parser::getOperatorSymbol(token.data.operatorType);
	case T_FUNCCALL:
		return token.data.asFunctionName;
	default:
		throw StutskException(ET_ERROR, "Cannot serialize a value.");
	}
	return "";
}

/* Parser::readCodeblock - reads either a { } codeblock literal or a ( ) array literal */
void Parser::readCodeblock(bool isArray, string &output) {
	stringstream result;
	bool inString = false;
	bool escapeModifier = false;
	int niveau = 1;

	while (codePosition + 1 <= (signed)sourceCode.length()) {
		if (!inString)
		{
			if (sourceCode[codePosition] == '"')
				inString = true;
			if (isArray)
				switch (sourceCode[codePosition]) {
				case '(':
					niveau++;
					break;
				case ')':
					niveau--;
					break;
			}
			else
				switch (sourceCode[codePosition]) {
				case '{':
					niveau++;
					break;
				case '}':
					niveau--;
					break;
			}
			if (niveau == 0)
				break;
		}
		else
		{
			if (escapeModifier)
				escapeModifier = false;
			else if (sourceCode[codePosition] == '\\')
				escapeModifier = true;
			else if (sourceCode[codePosition] == '"')
				inString = false;
		}
		result << sourceCode[codePosition];
		advanceCursor();
	}

	if (inString || (codePosition == (signed)sourceCode.length()))
		throw StutskException(ET_PARSER, "Unterminated string", linenumber_, parseContext);

	if (isArray) {
		if (sourceCode[codePosition] != ')')
			throw StutskException(ET_PARSER, "Unterminated array", linenumber_, parseContext);
	}
	else {
		if (sourceCode[codePosition] != '}')
			throw StutskException(ET_PARSER, "Unterminated codeblock",
			linenumber_, parseContext);
	}

	advanceCursor();
	output = result.str();
}

string implode(vector<stutskInteger>& a)
{
	stringstream result;       
	for (vector<stutskInteger>::iterator it = a.begin(); it!=a.end(); ++it)    
		result << *it << " ";
	return result.str();     
}

string BuiltIns::dumpVariables(Context &context)
{
	stringstream result;
	for (TokenMap::iterator it = context.variables.begin(); it != context.variables.end(); ++it) {
		result << it->first << ": " << dumpValue(it->second, 0) << "\n";	
	}
	return result.str();
}

string BuiltIns::tokenType(stutskTokenType type)
{
	switch (type)
	{
	case T_EMPTY: return "T_EMPTY";		
	case T_OPERATOR: return "T_OPERATOR";		
	case T_FUNCCALL: return "T_FUNCCALL";		
	case T_VARIABLE: return "T_VARIABLE";		
	case T_INTEGER: return "T_INTEGER";		
	case T_BOOL: return "T_BOOL";		
	case T_FLOAT: return "T_FLOAT";		
	case T_STRING: return "T_STRING";		
	case T_CODEBLOCK: return "T_CODEBLOCK";		
	case T_ARRAY: return "T_ARRAY";		
	case T_HANDLE: return "T_HANDLE";
	case T_DICTIONARY: return "T_DICTIONARY";
	default: return ""; 
	}
}

string BuiltIns::dumpDictionary(TokenMap& tokenMap, int niveau)
{
	stringstream result;
	 
	for (TokenMap::iterator iter = tokenMap.begin(); 
		iter != tokenMap.end(); ++iter) {
			result << BuiltIns::dumpValue(iter->second, niveau, iter->first);
	}
	return result.str();
}

/* BuiltIns::dumpValue - creates a descriptive textual representation of a token - recursing if
it is a variable/array/codeblock. Used for debugging. */

string BuiltIns::dumpValue(Token& token, int niveau, int i)
{
	string s;
	toString(i, s);
	return dumpValue(token, niveau, s);
}

string BuiltIns::dumpValue(Token& token, int niveau, string i) {
	string identString;
	stringstream result;
	TokenMap::iterator iter;

	for (int j = 0; j < niveau; j++) {
		identString += "  ";
	}

	switch (token.tokenType) {
	case T_EMPTY:
		result << identString << i << ": T_EMPTY\n";
		break;
	case T_OPERATOR:
		result << identString << i << ": T_OPERATOR (" <<
			Parser::getOperatorSymbol(token.data.operatorType) << ")\n";
		break;
	case T_FUNCCALL:
		result << identString << i << ": T_FUNCCALL (" <<
			token.data.asFunctionName << ")\n";
		break;
	case T_VARIABLE:
		if (token.index.size() > 0) {
			result << identString << i << ": T_VARIABLE (" <<
				token.data.asVariable.name << ", index: ( " <<
				implode(token.index) << "), ctx: " << (long)
				token.data.asVariable.context << ")\n";
			if (token.data.asVariable.context == NULL)
				break;
			iter = token.data.asVariable.context->variables.find
				(token.data.asVariable.name);
			if (iter != token.data.asVariable.context->variables.end()) {
				result << dumpValue(iter->second, niveau + 2);
			}
			else {
				result << identString << "    (value undefined)\n";
			}
		}
		else {
			result << identString << i << ": T_VARIABLE (" <<
				token.data.asVariable.name << ", ctx: " << (long)
				token.data.asVariable.context << ")\n";
			if (token.data.asVariable.context == NULL)
				break;
			iter = token.data.asVariable.context->variables.find
				(token.data.asVariable.name);
			if (iter != token.data.asVariable.context->variables.end()) {
				result << dumpValue(iter->second, niveau + 2);
			}
			else {
				result << identString << "    (value undefined)\n";
			}
		}
		break;
	case T_INTEGER:
		result << identString << i << ": T_INTEGER (" <<
			token.data.asInteger << ")\n";
		break;
	case T_BOOL:
		result << identString << i << ": T_BOOL (" << (token.data.asBool ?
			"TRUE" : "FALSE") << ")\n";
		break;
	case T_FLOAT:
		result << identString << i << ": T_FLOAT (" <<
			token.data.asFloat << ")\n";
		break;
	case T_STRING:
		result << identString << i << ": T_STRING[" << (*token.asString)
			.length() << "] (\"" << *token.asString << "\")\n";
		break;
	case T_CODEBLOCK:
		result << identString << i << ": T_CODEBLOCK[" <<
			(*token.asTokenList).size() << "]: \n" << dumpTokens
			(*token.asTokenList, niveau + 4);
		break;
	case T_ARRAY:
		result << identString << i << ": T_ARRAY[" << (*token.asTokenList).size
			() << "]: \n" << dumpTokens(*token.asTokenList, niveau + 4);
		break;
	case T_DICTIONARY:
		result << identString << i << ": T_DICTIONARY[" <<
			(*token.asDictionary).size() << "]: \n" << dumpDictionary
			(*token.asDictionary, niveau + 4);
	case T_HANDLE:
		result << identString << i << ": T_HANDLE (" << (long)
			token.data.asHandle.ptr << ", size: " << (long)
			token.data.asHandle.size << ")\n";
		break;
	}
	return result.str();
}

/* BuiltIns::dumpTokens - creates a descriptive textual representation of a list of tokens. Used for
debugging */
string BuiltIns::dumpTokens(TokenList& tokens, int niveau) {
	int i;
	stringstream result;
	for (i = 0; i < (signed)tokens.size(); i++) {
		result << BuiltIns::dumpValue(tokens[i], niveau, i);
	}
	return result.str();
}

/* BuiltIns::dumpTokens - creates a descriptive textual representation of a list of tokens. Used for
debugging */
string BuiltIns::dumpTokens(TokenStack& tokens, int niveau) {
	int i;
	stringstream result;
	for (i = 0; i < (signed)tokens.size(); i++) {
		result << BuiltIns::dumpValue(tokens[i], niveau, i);
	}
	return result.str();
}

/* main - logic for command-line parameters and outermost exception handling. Also an entrypoint to
the interpreter :) */
int main(int ac, char* av[], char* envp[]) {
	// We use boost::program_options to parse command line parameters
	using namespace boost::program_options;

	// Populate environment variable map
	while (*envp != NULL)
	{
		std::string varString(*envp); 
		std::string::size_type pos = varString.find('=');
		if (pos == std::string::npos) 
			environmentVars[varString] = "";
		else
			environmentVars[varString.substr(0, pos)] = 
			varString.substr(pos+1);
		envp++;
	}

	options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("version,v", "print the version number")
		("include-path,I", value< vector<string> >(), "include path")
		("input-file,i", value<string>(), "input file")
		("debug,D", "start interpreter with debugging")
		("debugger-address", value<string>()->default_value("localhost"), 
			"IP(v6) adddress or hostname of the debugger")
		("debugger-port", value<int>()->default_value(8455), "port or the debugger")
		;

	variables_map vm;
	vector<string> additionalParameters;

	try {
		parsed_options parsed = command_line_parser(ac, av).
			options(desc).allow_unregistered().run();
		store(parsed, vm);
		additionalParameters = collect_unrecognized(parsed.options, 
			include_positional);
		notify(vm);
	} catch (const std::exception &e) {
		cerr << "error: Invalid command line options, use -h for help (" << e.what() << ")\n";
		return 1;
	} 

	if (vm.count("help")) {
		cout << stutskVersion << endl << desc << "\n";
		return EXIT_SUCCESS;
	}
	if (vm.count("version")) {
		cout << stutskVersion << endl;
		return EXIT_SUCCESS;
	}
	if (vm.count("include-path"))
		includePaths = vm["include-path"].as<vector<string>>();

	string inputFileName;

	if (!vm.count("input-file"))
		if (additionalParameters.empty()) 
		{
			cerr << "error: No input file specified\n";
			return EXIT_FAILURE;
		} 
		else
		{
			inputFileName = additionalParameters[0];
			additionalParameters.erase(additionalParameters.begin());
		}
	else
		inputFileName = vm["input-file"].as<string>();

	customArguments.swap(additionalParameters);

	std::ifstream inputFile(inputFileName);

	if (!inputFile)
	{
		cerr << "error: Cannot open input file\n";
		return EXIT_FAILURE;
	}

	try {
		if (vm.count("debug"))
		{
			Debugger::get_instance().connect(
				vm["debugger-address"].as<string>(),
				vm["debugger-port"].as<int>());
		}

		ParseContext* parseContext = ParseContext::newContext();

		parseContext->SourceCode = string(
			std::istreambuf_iterator<char>(inputFile),
			std::istreambuf_iterator<char>());
		parseContext->FileName = inputFileName;

		// We generate a Parser object
		Parser parser(parseContext->SourceCode, parseContext);

		// Initialization of global variables8
		exitVar = OP_INVALID;

		TokenList sourceCode;	

		parser.parse(sourceCode);

		Context mainCtx(sourceCode, NULL);
		mainContext = &mainCtx;
		mainContext->run(sourceCode, "<main>");
		Debugger::get_instance().disconnect();
	}
	catch (const StutskException &e) {
		cerr << e.getFormattedMessage();
		return EXIT_FAILURE;
	}
	catch (const std::exception &e) {
		cerr << e.what();
	    return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
