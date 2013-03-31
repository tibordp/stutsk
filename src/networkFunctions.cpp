/*
networkFunctions.cpp - Networking related Stutsk functions built into the interpreter are implemented here
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

#include <builtinFunctions.h>

#include <boost/asio.hpp>

boost::asio::io_service io_service;

void BuiltIns::_f_socket_listen(Context* context) {
    /* arguments: <T_STRING port> socket_listen
	   returnvalue: <T_HANDLE> 
	   description: Start litening on a TCP port `port` and returns an acceptor object.
	   notes: 
	     Port can either be numeric or a common port name, such as "www".
		 See boost::asio for further reference.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	tcp::acceptor* acceptor = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), (unsigned short)giveInteger(token1)));
	Token outputHandle(T_HANDLE);
	outputHandle.data.asHandle.ptr = (void*)acceptor;
	outputHandle.data.asHandle.size = sizeof(tcp::acceptor);
	stutskStack.push_back(outputHandle);
}

void BuiltIns::_f_socket_open(Context* context) {
    /* arguments: <T_STRING host> <T_STRING port> socket_open
	   returnvalue: <T_HANDLE> 
	   description: Opens a TCP connection to `host` on a port `port` and returns 
	     a handle to a netowrk socket.
	   notes: Host can be either a DNS name or a IPv4/IPv6 literal address.
	     Port can either be numeric or a common port name, such as "www".
		 See boost::asio for further reference.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	string hostname = *giveString(token2);
	string port = *giveString(token1);

	tcp::resolver resolver(io_service);
	tcp::resolver::query query(hostname.c_str(), port.c_str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	tcp::socket* socket = new tcp::socket(io_service);

	boost::system::error_code error = boost::asio::error::host_not_found;

	while (error && endpoint_iterator != end)
	{
		socket->close();
		socket->connect(*endpoint_iterator++, error);
	}
	if (error)
		throw StutskException(ET_ERROR, error.message()); // Some other error.

	Token outputHandle(T_HANDLE);
	outputHandle.data.asHandle.ptr = (void*)socket;
	outputHandle.data.asHandle.size = sizeof(tcp::socket);
	stutskStack.push_back(outputHandle);
}

void BuiltIns::_f_socket_accept(Context* context) {
    /* arguments: <T_HANDLE acceptor> socket_accept
	   returnvalue: <T_HANDLE> 
	   description: Waits for incoming connection and returns a network socket on successful
	     connection.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	recurseVariables(token1);

	if (token1.tokenType != T_HANDLE)
		throw StutskException(ET_ERROR, "Token not a handle");

	tcp::acceptor *acceptor = (tcp::acceptor*)token1.data.asHandle.ptr;

	tcp::socket* socket = new tcp::socket(io_service);

	acceptor->accept(*socket);

	Token outputHandle(T_HANDLE);
	outputHandle.data.asHandle.ptr = (void*)socket;
	outputHandle.data.asHandle.size = sizeof(tcp::socket);
	stutskStack.push_back(outputHandle);
}

void BuiltIns::_f_socket_read(Context* context) {
    /* arguments: <T_INTEGER count> <T_HANDLE socket> socket_read
	   returnvalue: <T_STRING> 
	   description: Reads `count` characters from a network socket.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	stutskInteger bufSize = giveInteger(token2);

	recurseVariables(token1);

	if (token1.tokenType != T_HANDLE)
		throw StutskException(ET_ERROR, "Token not a handle");

	tcp::socket *socket = (tcp::socket*)token1.data.asHandle.ptr;
	if (socket->is_open())
	{
		std::vector<char> buf(bufSize);

		boost::system::error_code error;

		size_t bytesRead = socket->read_some(boost::asio::buffer(buf), error);

		if (error == boost::asio::error::eof)
		{
			socket->shutdown(boost::asio::socket_base::shutdown_both);
			socket->close();
		}
		else if (error) 
			throw StutskException(ET_ERROR, error.message()); // Some other error.

		Token newString(T_STRING);
		newString.asString = StringPtr(new string (buf.begin(), buf.begin()+bytesRead));
		stutskStack.push_back(newString);
	}
	else
		pushString();
}

void BuiltIns::_f_socket_readline(Context* context) {
    /* arguments: <T_HANDLE socket> socket_readline
	   returnvalue: <T_STRING> 
	   description: Reads a single line from a network socket.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();	
	using boost::asio::ip::tcp;

	recurseVariables(token1);

	if (token1.tokenType != T_HANDLE)
		throw StutskException(ET_ERROR, "Token not a handle");

	tcp::socket *socket = (tcp::socket*)token1.data.asHandle.ptr;


	if (socket->is_open())
	{
		Token newString(T_STRING);
		newString.asString = StringPtr(new string);

		while (true)
		{
			std::vector<char> buf(1);

			boost::system::error_code error;

			if (read(*socket,boost::asio::buffer(buf),boost::asio::transfer_all(), error))
			{
				if (!(buf[0] == '\r' || buf[0] == '\n'))
					newString.asString->append(1, buf[0]);				
				if (buf[0] == '\n') break;
			}

			if (error == boost::asio::error::eof)
			{
				socket->shutdown(boost::asio::socket_base::shutdown_both);
				socket->close();
				break;
			}
			else if (error) 
				throw StutskException(ET_ERROR, error.message()); // Some other error.
		}

		stutskStack.push_back(newString);
	}
	else
		pushString();
}

void BuiltIns::_f_socket_write(Context* context) {
    /* arguments: <T_STRING data> <T_HANDLE socket> socket_write
	   returnvalue:  
	   description: Writes `data` to a network socket.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	recurseVariables(token1);

	if (token1.tokenType != T_HANDLE)
		throw StutskException(ET_ERROR, "Token not a handle");

	tcp::socket *socket = (tcp::socket*)token1.data.asHandle.ptr;

	boost::system::error_code error;

	boost::asio::write(*socket, boost::asio::buffer(*giveString(token2)),
		boost::asio::transfer_all(),error);

	if (error) 
		throw StutskException(ET_ERROR, error.message()); // Some other error.
}


void BuiltIns::_f_socket_eof(Context* context) {
    /* arguments: <T_HANDLE socket> socket_write
	   returnvalue: <T_BOOL>  
	   description: Returns true if the socket has been closed.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	recurseVariables(token1);

	if (token1.tokenType != T_HANDLE)
		throw StutskException(ET_ERROR, "Token not a handle");

	tcp::socket *socket = (tcp::socket*)token1.data.asHandle.ptr;

	pushBool(! socket->is_open());
}


void BuiltIns::_f_socket_close(Context* context)
{
    /* arguments: <T_HANDLE socket> socket_close
	   returnvalue: 
	   description: Closes the socket.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	using boost::asio::ip::tcp;

	recurseVariables(token1);

	if (token1.tokenType != T_HANDLE)
		throw StutskException(ET_ERROR, "Token not a handle");

	tcp::socket *socket = (tcp::socket*)token1.data.asHandle.ptr;

	socket->shutdown(boost::asio::socket_base::shutdown_both);

	socket->close();

	delete socket;
}