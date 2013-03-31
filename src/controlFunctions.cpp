/*
controlFunctions.cpp - Program flow and management Stutsk functions built into the interpreter are implemented here
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
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>

#ifdef FORK_CAPABLE
#include <unistd.h>
#endif

#include "exec-stream.h"

void BuiltIns::_f_do(Context* context) {
	/* arguments: <T_CODEBLOCK c1> do
	returnvalue: 
	description: Executes c1.
	notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back(); // Name

	recurseVariables(token1);
	if (token1.tokenType != T_CODEBLOCK) {
		throw StutskException(ET_ERROR, "Token is not a codeblock");
	}
	else {
		context->run(*token1.asTokenList, "do");
	}
}

void BuiltIns::_f_inherit(Context* context) {
	/* arguments: <T_CODEBLOCK c1> inherit
	returnvalue: 
	description: Executes c1 in the parent context of the calling function.
	notes: Useful for creating own control structures.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back(); // Name

	recurseVariables(token1);
	if (token1.tokenType != T_CODEBLOCK) {
		throw StutskException(ET_ERROR, "Token is not a codeblock");
	}
	else {
		if (context->parentContext == NULL)
			throw StutskException(ET_ERROR, "Parent context is null");
		else
			context->parentContext->run(*token1.asTokenList, "inherit");
	}
	return;
}

void BuiltIns::_f_lambda(Context* context) {
	/* arguments: <T_CODEBLOCK c1> lambda
	returnvalue: 
	description: Creates a new anonymous context and executes `c1` in it.
	notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back(); 

	recurseVariables(token1);
	if (token1.tokenType != T_CODEBLOCK) {
		throw StutskException(ET_ERROR, "Token is not a codeblock");
	}
	else {
		Context newCtx(*token1.asTokenList, context);
		newCtx.functionName = "<anonymous function>";
		newCtx.run(*token1.asTokenList, "lambda");
		if (exitVar == OP_EXIT) {
			exitVar = OP_INVALID;
		}
	}
}

void BuiltIns::_f_recurse(Context* context) {
	/* arguments: recurse
	returnvalue: 
	description: Reinvokes current function.
	notes: Can be used in conjunction with lambda functions to create true anonymous
	recursion.
	*/
	Context newCtx(context->sourceCode, context);
	newCtx.run(context->sourceCode, "recurse");
	if (exitVar == OP_EXIT) {
		exitVar = OP_INVALID;
	}
}

void BuiltIns::_f_fork(Context* context) {
	/* arguments: <T_CODEBLOCK c1> <T_CODEBLOCK c2> fork
	returnvalue: 
	description: Forks the current process and executes c1 in the original process and c2 in 
	the newly spawned one.
	notes: Only available on POSIX platforms. 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back(); // Name
	Token token2 = stack_back_safe();
	stutskStack.pop_back(); // Name
#ifdef FORK_CAPABLE
	recurseVariables(token1);
	if (token1.tokenType != T_CODEBLOCK || token2.tokenType != T_CODEBLOCK) {
		throw StutskException(ET_ERROR, "Token is not a codeblock");
	}
	else {
		if (fork())		
		{
			context->run(*token2.asTokenList, "fork");
		}
		else
		{
			context->run(*token1.asTokenList, "fork");
		}
	}
#else
	throw StutskException(ET_ERROR, "fork() is not available on non-POSIX operating systems");
#endif
}

void BuiltIns::_f_include(Context* context) {
	/* arguments: <T_STRING filename> include
	returnvalue: 
	description: Parses and executes Stutsk source code contained in file filename.
	notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();

	string filename = *giveString(token);

	ifstream readfile(filename.c_str());

	if (!readfile)
		throw StutskException(ET_ERROR, "Cannot open file \"" +
		filename + "\"");

	stringstream sourceCodeStream;
	sourceCodeStream << readfile.rdbuf();

	// LEAK!
	// Context must persist past inclusion for objects left on stack (yeah, I know)
	ParseContext* parseContext = ParseContext::newContext();

	parseContext->SourceCode = sourceCodeStream.str();
	parseContext->FileName = filename;

	Parser parser(parseContext->SourceCode, parseContext);

	TokenList sourceCode;

	parser.parse(sourceCode);
	context->run(sourceCode, "include");
}

void BuiltIns::_f_eval(Context* context) {
	/* arguments: <T_STRING source> eval
	returnvalue: 
	description: Parses and executes source as Stutsk source code.
	notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();

	ParseContext* parseContext = ParseContext::newContext();

	parseContext->SourceCode = *giveString(token);
	parseContext->FileName = "<eval>";

	Parser parser(parseContext->SourceCode, parseContext);

	TokenList sourceCode;

	parser.parse(sourceCode);
	context->run(sourceCode, "eval");
}


void BuiltIns::_f_swap(Context* context) {
	/* arguments: <...> <T_INTEGER n> swap
	returnvalue: <...>
	description: Reverses the order of n topmost tokens on the stack.
	notes: 
	*/
	Token indexToken = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger number = giveInteger(indexToken);
	if (stutskStack.size() < number)
		throw StutskException(ET_ERROR, "Not enough tokens on stack.");
	if (number < 0)
		throw StutskException(ET_ERROR, "Number must not be negative");

	reverse(stutskStack.end() - number, stutskStack.end());
}

void BuiltIns::_f_sleep(Context* context) {
	/* arguments: <T_FLOAT sleeptime> sleep
	returnvalue: 
	description: Pauses execution for a number of seconds given in sleeptime.
	notes: 
	*/
	Token timeSleepToken = stack_back_safe();
	stutskFloat timeSleep = giveFloat(timeSleepToken);
	stutskStack.pop_back();
	boost::this_thread::sleep(boost::posix_time::milliseconds((long)(timeSleep*1000)));
}

void BuiltIns::_f_roll(Context* context) {
	/* arguments: <...> <T_INTEGER n> roll
	returnvalue: <...>
	description: Brings the n-th topmost token to the top of the stack.
	notes: 
	*/
	Token indexToken = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger index = giveInteger(indexToken);
	Token repToken = *(stutskStack.end() - index);
	stutskStack.erase(stutskStack.end() - index);
	stutskStack.push_back(repToken);
}

void BuiltIns::_f_swp(Context* context) {
	/* arguments: <token> <token> swp
	returnvalue: <token> <token> 
	description: Swaps the two topmost tokens on the stack.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	Token lower = stack_back_safe();
	stutskStack.pop_back();
	stutskStack.push_back(upper);
	stutskStack.push_back(lower);
}

void BuiltIns::_f_dup(Context* context) {
	/* arguments: <token> swp
	returnvalue: <token> <token>
	description: Duplicates the topmost token on the stack.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.push_back(copy_token(upper));
}

void BuiltIns::_f_dmp(Context* context) {
	/* arguments: <token> dmp
	returnvalue: 
	description: Discards the topmost token on the stack.
	notes: 
	*/
	stutskStack.pop_back();
}

void BuiltIns::_f_uneval(Context* context) {
	/* arguments: <token> uneval
	returnvalue: <T_STRING>
	description: Serializes a token into a textual representation that is valid Stutsk source.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	*pushString() = Parser::serializeToken(upper);
}

void BuiltIns::_f_is_def(Context* context) {
	/* arguments: <T_VARIABLE var> is_def
	arguments: <T_STRING func> is_def
	returnvalue: <T_BOOL>
	description: If token is a string, returns true if a function with name func exists. 
	If token is a variable, it returns true if it has been initialized.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	if (upper.tokenType == T_STRING) { // Zadeva je funkcija
		BuiltinFunctionsMap::iterator bIter =
			builtinFunctions.find(*upper.asString);
		if (bIter != builtinFunctions.end())
			pushBool(true);
		else {
			UserFunctionsMap::iterator uIter =
				userFunctions.find(*upper.asString);
			pushBool(uIter != userFunctions.end());
		}

	}
	else if (upper.tokenType == T_VARIABLE) { // Zadeva je variabla
		TokenMap::iterator iter = upper.data.asVariable.context->variables.find
			(upper.data.asVariable.name);
		pushBool(iter != upper.data.asVariable.context->variables.end());
	}
	else
		throw StutskException(ET_ERROR, "Invalid argument");
}

void BuiltIns::_f_system(Context* context) {
	/* arguments: <T_STRING filename> system
	returnvalue: 
	description: Executes an executable given by filename.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	system(giveString(upper)->c_str());
}

void BuiltIns::_f_exec(Context* context) {
	/* arguments: <T_STRING filename> exec
	returnvalue: 
	description: Executes an executable given by filename.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	exec_stream_t *exec_stream = new exec_stream_t();
	exec_stream->set_wait_timeout(exec_stream_t::stream_kind_t::s_all, -1);
	exec_stream->start(*giveString(upper), "");
	Token new_token(T_HANDLE);
	new_token.data.asHandle.ptr = exec_stream;
	new_token.data.asHandle.size = sizeof(exec_stream_t);
	stutskStack.push_back(new_token);
}


void BuiltIns::_f_exec_readline(Context* context) {
	/* arguments: readline
	returnvalue: <T_STRING text>
	description: Reads a line from standard input.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(upper);

	if (upper.tokenType != T_HANDLE) 
		throw StutskException(ET_ERROR, "Token is not a handle");

	exec_stream_t *exec_stream = static_cast<exec_stream_t*>(upper.data.asHandle.ptr);

	getline(exec_stream->out(), *pushString());
}

void BuiltIns::_f_exec_read(Context* context) {
	/* arguments: <T_INTEGER count> read
	returnvalue: <T_STRING text>
	description: Reads count characters from standard input.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(upper);

	if (upper.tokenType != T_HANDLE) 
		throw StutskException(ET_ERROR, "Token is not a handle");

	exec_stream_t *exec_stream = static_cast<exec_stream_t*>(upper.data.asHandle.ptr);

	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	stutskInteger length = giveInteger(token2);
	auto_ptr<char> buf = auto_ptr<char>(new char[length]);

	exec_stream->out().read(buf.get(), length);

	if (exec_stream->out().bad()) {
		throw StutskException(ET_ERROR, "File operation failed");
	}

	Token newString(T_STRING);
	newString.asString = StringPtr(new string(buf.get(), exec_stream->out().gcount()));
	stutskStack.push_back(newString);
}


void BuiltIns::_f_exec_readchar(Context* context) {
	/* arguments: readchar
	returnvalue: <T_STRING character>
	description: Reads a single character from standard input.
	notes: Equivalent to `1 read`
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(upper);

	if (upper.tokenType != T_HANDLE) 
		throw StutskException(ET_ERROR, "Token is not a handle");

	exec_stream_t *exec_stream = static_cast<exec_stream_t*>(upper.data.asHandle.ptr);

	char buf;
	exec_stream->out().read(&buf, 1);

	if (exec_stream->out().bad()) {
		throw StutskException(ET_ERROR, "File operation failed");
	}

	Token newString(T_STRING);
	// Returns either "<char>" or ""
	newString.asString = StringPtr(new string (&buf, exec_stream->out().gcount()));
	stutskStack.push_back(newString);
}

void BuiltIns::_f_exec_eof(Context* context) {
	/* arguments: eof
	returnvalue: <T_BOOL eof>
	description: Returns true if standard input pipe is closed.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(upper);

	if (upper.tokenType != T_HANDLE) 
		throw StutskException(ET_ERROR, "Token is not a handle");

	exec_stream_t *exec_stream = static_cast<exec_stream_t*>(upper.data.asHandle.ptr);

	pushBool(exec_stream->out().eof());
}

void BuiltIns::_f_exec_print(Context* context) {
	/* arguments: eof
	returnvalue: <T_BOOL eof>
	description: Returns true if standard input pipe is closed.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(upper);

	if (upper.tokenType != T_HANDLE) 
		throw StutskException(ET_ERROR, "Token is not a handle");

	exec_stream_t *exec_stream = static_cast<exec_stream_t*>(upper.data.asHandle.ptr);

	Token tokenToPrint = stack_back_safe();
	exec_stream->in() << *giveString(tokenToPrint);

	stutskStack.pop_back();
}

void BuiltIns::_f_is_string(Context* context) {
	/* arguments: <value> is_string
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_STRING.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_STRING);
}

void BuiltIns::_f_is_integer(Context* context) {
	/* arguments: <value> is_integer
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_INTEGER.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_INTEGER);
}

void BuiltIns::_f_is_float(Context* context) {
	/* arguments: <value> is_float
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_FLOAT.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_FLOAT);
}

void BuiltIns::_f_is_array(Context* context) {
	/* arguments: <value> is_array
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_ARRAY.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_ARRAY);
}

void BuiltIns::_f_is_handle(Context* context) {
	/* arguments: <value> is_handle
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_HANDLE.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_HANDLE);
}

void BuiltIns::_f_is_bool(Context* context) {
	/* arguments: <value> is_bool
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_BOOL.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_BOOL);
}

void BuiltIns::_f_is_numeric(Context* context) {
	/* arguments: <value> is_numeric
	returnvalue: <T_BOOL>
	description: Returns true if value is numeric (regardless of its actual type).
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	string s1; stutskInteger i1; stutskFloat f1;
	switch (giveGCD(upper, f1, i1, s1))
	{
	case NT_FLOAT:
	case NT_INTEGER:
		pushBool(true);
		break;
	default:
		pushBool(false);
	}
}

void BuiltIns::_f_is_codeblock(Context* context) {
	/* arguments: <value> is_codeblock
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_CODEBLOCK.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_CODEBLOCK);
}

void BuiltIns::_f_is_variable(Context* context) {
	/* arguments: <token> is_variable
	returnvalue: <T_BOOL>
	description: Returns true if the token is a reference.
	notes: 
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	pushBool(upper.tokenType == T_VARIABLE);
}

void BuiltIns::_f_is_null(Context* context) {
	/* arguments: <value> is_null
	returnvalue: <T_BOOL>
	description: Returns true if value is of type T_BOOL.
	notes: Recurses variables.
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(upper);
	pushBool(upper.tokenType == T_EMPTY);
}


void BuiltIns::_f___dumpglobalvariables(Context* context)
{
	/* arguments: __dumpglobalvariables
	returnvalue: 
	description: Prints global variables on standard output in a human-readable format.
	notes: 
	*/	
	cout << dumpVariables(*mainContext);
}

void BuiltIns::_f___dumpvariables(Context* context)
{
	/* arguments: __dumpvariables
	returnvalue: 
	description: Prints variables of current context on standard output in a human-readable format.
	notes: 
	*/
	cout << dumpVariables(*context);
}

void BuiltIns::_f___dumpstack(Context* context) {
	/* arguments: __dumpstack
	returnvalue: 
	description: Prints the whole stack on standard output in a human-readable format.
	notes: 
	*/
	cout << BuiltIns::dumpTokens(stutskStack, 0);
}

void BuiltIns::_f___debug(Context* context) {
	/* arguments: <...> __debug
	returnvalue: <...>
	description: Does various things during the debuging of Stutsk interpreter.
	notes: Internal use only.
	*/
	pushInteger(sizeof(Token));
}

void BuiltIns::_f___type(Context* context) {
	/* arguments: <token> __type
	returnvalue: <T_STRING>
	description: Returns a string representing token's type ("T_EMPTY", "T_INTEGER", ...)
	notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	*pushString() = tokenType(token.tokenType);
}

void BuiltIns::_f_time(Context* context) 
{
	/* arguments: <T_CODEBLOCK code> time
	returnvalue: <T_FLOAT>
	description: Executes codeblock code and returns its execution time in seconds.
	notes:
	*/
	using namespace boost::chrono;

	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	if (token1.tokenType != T_CODEBLOCK) {
		throw StutskException(ET_ERROR, "Token is not a codeblock");
	}
	recurseVariables(token1);

	auto t1 = high_resolution_clock::now();	
	context->run(*token1.asTokenList, "time");
	auto t2 = high_resolution_clock::now();
	auto tt = duration_cast<nanoseconds>(t2-t1);

	pushFloat( (stutskFloat)tt.count() / 1000000000 );
}

void BuiltIns::_f_stack_count(Context* context)
{
	/* arguments: stack_count
	returnvalue: <T_INTEGER>
	description: Returns the number of elements on stack.
	notes:
	*/
	pushInteger(stutskStack.size());
}

void BuiltIns::_f_stack_purge(Context* context)
{
	/* arguments: stack_purge
	returnvalue:
	description: Clears the main stack.
	notes: Use sparingly.
	*/
	stutskStack.clear();
}

void BuiltIns::_f_stack_empty(Context* context)
{
	/* arguments: stack_empty
	returnvalue: <T_BOOL>
	description: Returns true if the main stack is empty.
	notes:
	*/
	pushBool(stutskStack.size() == 0);
}

void BuiltIns::_f_definition(Context* context)
{
	/* arguments: <T_STRING func> definition
	returnvalue: <T_CODEBLOCK>
	description: Returns a codeblock that is executed when function with name func is called.
	notes:
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	string functionName = *giveString(token1);
	Token codeBlock(T_CODEBLOCK);

	UserFunctionsMap::iterator uIter = userFunctions.find(functionName);
	if (uIter != userFunctions.end()) {
		codeBlock.asTokenList = TokenListPtr(new TokenList(uIter->second));
		stutskStack.push_back(codeBlock);
	}
	else {
		BuiltinFunctionsMap::iterator bIter = builtinFunctions.find(functionName);
		if (bIter != builtinFunctions.end()) {
			if (functionName.substr(0,10) != "__builtin_")
				functionName =  string("__builtin_") + functionName;
			Token functionToken(T_FUNCCALL);
			strcpy(functionToken.data.asFunctionName, functionName.c_str());
			codeBlock.asTokenList = TokenListPtr(new TokenList());
			codeBlock.asTokenList->push_back(functionToken);
			stutskStack.push_back(codeBlock);
		}
		else {
			stringstream ExceptionText;
			ExceptionText << "Unknown function '" <<
				functionName << "'.";
			throw StutskException(ET_ERROR, ExceptionText.str());
		}
	}
}

void BuiltIns::_f_env_list(Context* context)
{
	/* arguments: env_list
	returnvalue: <T_ARRAY> [ ( ( <name> <value> ) ( <name> <value> ) ... ) ]
	description: Returns an array of environment strings (name-value pairs).
	notes:
	*/
	Token envVars(T_ARRAY);
	envVars.asTokenList = TokenListPtr(new TokenList());
	for (StringMapCI::const_iterator iter = environmentVars.cbegin(); 
		iter != environmentVars.cend(); ++iter)
	{
		Token kvPair(T_ARRAY);
		kvPair.asTokenList = TokenListPtr(new TokenList());
		Token key(T_STRING); Token value(T_STRING);
		key.asString = StringPtr(new string(iter->first));
		value.asString = StringPtr(new string(iter->second));
		kvPair.asTokenList->push_back(key);
		kvPair.asTokenList->push_back(value);
		envVars.asTokenList->push_back(kvPair);
	}
	stutskStack.push_back(envVars);
}

void BuiltIns::_f_env_get(Context* context)
{
	/* arguments: <T_STRING name> env_get
	returnvalue: <T_STRING>
	description: Returns a value of an environment string `name`
	notes: If env-var is nonexistent, it returns NULL.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	string varName = *giveString(token1);
	StringMapCI::const_iterator iter = environmentVars.find(varName);
	if (iter != environmentVars.end())
		*pushString() = iter->second;	
	else
		stutskStack.push_back(Token(T_EMPTY));
}
