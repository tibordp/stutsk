/*
stdioFunctions.cpp - Standard I/O related Stutsk functions built into the interpreter are implemented here
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

void BuiltIns::_f_commandline(Context* context) {
	/* arguments: commandline
	   returnvalue: <T_ARRAY> 
	   description: Returns an array of additional command line arguments passed to the interpreter
	   notes: 
	*/
	
	Token parameters(T_ARRAY);
	parameters.asTokenList = TokenListPtr(new TokenList());
	for (vector<string>::const_iterator iter = customArguments.begin(); 
		iter != customArguments.end(); ++iter)
	{
		Token parameter(T_STRING);
		parameter.asString = StringPtr(new string(*iter));
		parameters.asTokenList->push_back(parameter);
	}
	stutskStack.push_back(parameters);
}

void BuiltIns::_f_print(Context* context) {
	/* arguments: <T_STRING text> print
	   returnvalue: 
	   description: Prints text to standard output.
	   notes: 
	*/
	Token tokenToPrint = stack_back_safe();
	cout << *giveString(tokenToPrint);
	stutskStack.pop_back();
}

void BuiltIns::_f_error(Context* context) {
	/* arguments: <T_STRING text> error
	   returnvalue: 
	   description: Prints text to standard error.
	   notes: 
	*/
	Token tokenToPrint = stack_back_safe();
	cerr << *giveString(tokenToPrint);
	stutskStack.pop_back();
}

void BuiltIns::_f_readline(Context* context) {
	/* arguments: readline
	   returnvalue: <T_STRING text>
	   description: Reads a line from standard input.
	   notes: 
	*/
	getline(cin, *pushString());
}

void BuiltIns::_f_read(Context* context) {
	/* arguments: <T_INTEGER count> read
	   returnvalue: <T_STRING text>
	   description: Reads count characters from standard input.
	   notes: 
	*/
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	stutskInteger length = giveInteger(token2);
	auto_ptr<char> buf = auto_ptr<char>(new char[length]);

	cin.read(buf.get(), length);

	if (cin.bad()) {
		throw StutskException(ET_ERROR, "File operation failed");
	}

	Token newString(T_STRING);
	newString.asString = StringPtr(new string(buf.get(), cin.gcount()));
	stutskStack.push_back(newString);
}


void BuiltIns::_f_readchar(Context* context) {
	/* arguments: readchar
	   returnvalue: <T_STRING character>
	   description: Reads a single character from standard input.
	   notes: Equivalent to `1 read`
	*/
	char buf;
	cin.read(&buf, 1);

	if (cin.bad()) {
		throw StutskException(ET_ERROR, "File operation failed");
	}

	Token newString(T_STRING);
	// Returns either "<char>" or ""
	newString.asString = StringPtr(new string (&buf, cin.gcount()));
	stutskStack.push_back(newString);
}

void BuiltIns::_f_eof(Context* context) {
	/* arguments: eof
	   returnvalue: <T_BOOL eof>
	   description: Returns true if standard input pipe is closed.
	   notes: 
	*/
	pushBool(cin.eof());
}