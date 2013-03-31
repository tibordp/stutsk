/*
stutskInterpreter.hpp - header file
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

#ifndef STUTSKINTERPRETER_HPP
#define STUTSKINTERPRETER_HPP

#define _WIN32_WINNT 0x0501
#define _CRT_SECURE_NO_WARNINGS 1

#include <cstdlib>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <map>
#include <cstring>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

#ifdef ARM
	// ARM seems to not support the long float
	typedef double stutskFloat;
#else
	typedef long double stutskFloat;
#endif
typedef long long  stutskInteger;

enum stutskTokenType {
	T_EMPTY, T_OPERATOR, T_FUNCCALL, T_VARIABLE, T_INTEGER, T_BOOL, T_FLOAT,
	T_STRING, T_CODEBLOCK, T_ARRAY, T_DICTIONARY, T_HANDLE
};

enum OperatorType {
	OP_INVALID, OP_ASSIG, OP_ASSIG_REF, OP_PLUSPLUS, OP_MINMIN, OP_EQ, OP_SAME, OP_NOTEQ,
	OP_LESSTHAN, OP_MORETHAN, OP_LESSTHAN_EQ, OP_MORETHAN_EQ, OP_OR, OP_AND, OP_NOT, OP_PLUS, OP_MINUS,
	OP_MULTIP, OP_DIV, OP_DIVINT, OP_MOD, OP_TERNARY, OP_IF, OP_IFELSE,
	OP_CONCAT, OP_REPEAT, OP_FOREVER, OP_ARRAY, OP_FOREACH, OP_GLOBAL, OP_AUTO,
	OP_STATIC, OP_FUNC, OP_BREAK, OP_EXIT, OP_HALT, OP_TRY, OP_THROW, OP_DEREF,
	OP_UNSET, OP_POWER, OP_CONTINUE, OP_SWITCH
};

enum VariableScope {
	VS_AUTO, VS_GLOBAL, VS_STATIC
};

typedef enum _en_ExceptionType {
	ET_PARSER, ET_ERROR, ET_WARNING, ET_CUSTOM, ET_SYSTEM
} ExceptionType;

typedef enum _en_GCDType {
	NT_INTEGER, NT_FLOAT, NT_STRING, NT_INVALID
} GCDType;

typedef enum _en_CompareNumeric {
	CN_SMALLER, CN_EQUAL, CN_LARGER
} CompareNumeric;

template<class T,class Z>
CompareNumeric compareValue(T value1, Z value2)
{
	if (value1 < value2) return CN_SMALLER;
	if (value1 > value2) return CN_LARGER;
	return CN_EQUAL;
}

struct Token;
class Context;

typedef vector<Token> TokenList;
typedef deque <Token> TokenStack;

typedef void(*BuiltInFunction)(Context*);

extern CompareNumeric tokenNumericCompare(const Token& token1, const Token& token2);
extern bool tokenEqual(const Token& token1, const Token& token2);
extern bool tokenSame(const Token& token1, const Token& token2);

typedef map<string, BuiltInFunction>BuiltinFunctionsMap;
typedef map<string, OperatorType>OperatorMap;
typedef map<string, Token>TokenMap;
typedef map<string, BuiltInFunction>BuiltinFunctionsMap;
typedef map<string, TokenList>UserFunctionsMap;
typedef map<string, VariableScope>VariableScopeMap;
typedef map<string, string> StringMap;

struct ciLessBoost : std::binary_function<std::string, std::string, bool>
{
	bool operator() (const std::string & s1, const std::string & s2) const {
		return boost::algorithm::lexicographical_compare(s1, s2, boost::algorithm::is_iless());
	}
};

typedef map<string, string, ciLessBoost> StringMapCI;

// Shorthand type definitions
typedef boost::shared_ptr<std::string> StringPtr;
typedef boost::shared_ptr<TokenList> TokenListPtr;
typedef boost::shared_ptr<TokenMap> TokenMapPtr;

class ParseContext;

struct DebugInfo {
	int  context_id;
	int  lineNum;
	int  columnNum;
	int  tokenLength;
	long relativePos;
};

extern DebugInfo errorToken;
extern vector<ParseContext> parseContexts;

class StutskException {

public:
	  StutskException(ExceptionType type, const string& msg);
	  StutskException(ExceptionType type, const string& msg, 
		  int line_number, ParseContext* context);

	  string getMessage() const;
	  long getLineNumber() const;
	  string getFileName() const;
	  string getFormattedMessage() const;
private:
	string msg_;
	ExceptionType type_;
	int line_number_;
	const ParseContext* context_;
};

class ParseContext {
private:
	size_t id_;
public:
	size_t id()
	{
		return id_;
	};
	string SourceCode;
	string FileName;
	static ParseContext* newContext()
	{
		parseContexts.push_back(ParseContext());
		parseContexts.back().id_ = parseContexts.size()-1;
		return &parseContexts.back();
	};

	static const ParseContext& get_by_id(size_t id)
	{
		if (id >= parseContexts.size())
			throw StutskException(ET_CUSTOM, "Invalid parser context ID");
		return parseContexts.at(id);
	}
};



struct Token {
	stutskTokenType tokenType;

	StringPtr asString;
	TokenListPtr asTokenList;
	TokenMapPtr asDictionary;

	union _un_TokenData {
		OperatorType operatorType;
		char asFunctionName[32];

		struct _un_VariableData {
			char name[32];
			Context *context;
		}

		asVariable;

		struct _un_Handle {
			void * ptr;
			stutskInteger size;
		}

		asHandle;

		stutskInteger asInteger;
		bool asBool;
		stutskFloat asFloat;
	}

	data;

	vector<stutskInteger> index;

	DebugInfo* debugInfo;

	Token(stutskTokenType ttype = T_EMPTY) : tokenType (ttype) {};
};

// Filename and number of the last OPERATOR or FUNCTION executed - used for debugging

class Parser {
private:
	const string& sourceCode;
	ParseContext* parseContext;

	long codePosition;
	long linenumber_;
	long columnnumber_;
	long relativeposition_;

	void readString(string &output);
	bool readInteger(stutskInteger &value);
	bool readFloat(stutskFloat &value);
	void readCodeblock(bool isArray, string &output);
	string readIdentifier();
	string readToWhite();
	void advanceCursor();
	bool isNumeric(char symbol);
	void skipComment();
public:
	static std::string stringEscape(std::string input);
	static std::string serializeToken(const Token &token);
	static string getOperatorSymbol(OperatorType oper);
	Parser(const string& source, ParseContext* parse_context, 
		int linenumber = 1, int columnnumber = 1, int relativeposition = 0) : 
	sourceCode(source), parseContext(parse_context), codePosition(0),
		linenumber_(linenumber), columnnumber_(columnnumber), relativeposition_(relativeposition) {};

	void parse(TokenList& retArray);
};

namespace BuiltIns {
	extern BuiltinFunctionsMap populateFunctions();
	// Debugging functions
	extern string dumpTokens(TokenList& tokens, int niveau = 0);
	extern string dumpTokens(TokenStack& tokens, int niveau = 0);
	extern string dumpValue(Token& token, int niveau = 0, int i = 0);
	extern string dumpVariables(Context &context);
	extern string dumpDictionary(TokenMap& tokenMap, int niveau = 0);
	extern string dumpValue(Token& token, int niveau, string i); 
	extern string tokenType(stutskTokenType type);
};

namespace Operators {
	extern OperatorMap populateOperators();
	extern void doOperator(Context *context, OperatorType oper); 
	extern bool readOperator(const string& input, OperatorType &operType);
};

#include <boost/noncopyable.hpp>

class Context : boost::noncopyable {
public:
	string functionName; // __main for a main
	TokenMap variables;    
	// functions should copy their definition to the context for anonymous recursion
	Context *parentContext;
	const TokenList& sourceCode;
	VariableScopeMap variableScopeMap;
	Context(const TokenList& source, Context *parent) : parentContext(parent), sourceCode(source) { } 
	VariableScope findVariableScope(string variableName);
	void run();
	void run(const TokenList& source, string blockFunction);
};

template<class T>
bool fromString(const string& input, T& output) {
	stringstream ss(input);
	return (ss >> output) && ss.eof();
}

template<class T>
bool toString(const T& input, string &output) {
	stringstream ss;
	if (!(ss << input))
		return false;
	else {
		output = ss.str();
		return true;
	}
}

extern string extractFileName(const string& fn);

extern void getNth(Token &newToken, stutskInteger i);
extern stutskInteger recurseVariables(Token& newToken, bool norecurse = false, bool dumpLast = true);

extern GCDType giveGCD(const Token& token, stutskFloat &floatV, stutskInteger &intV,
	string &stringV);

extern stutskInteger giveInteger(const Token& token);
extern stutskFloat   giveFloat(const Token& token);
extern bool          giveBool(const Token& token);
extern void          giveStringCopy(const Token& token, string& str);
extern StringPtr     giveString(const Token& token);

// ------------------------- GLOBAL VARIABLES ------------------------------ //

extern StringMapCI environmentVars;
extern UserFunctionsMap userFunctions;
extern BuiltinFunctionsMap builtinFunctions;
extern OperatorMap stutskOperators;
extern TokenStack stutskStack;
extern Context *mainContext;
extern OperatorType exitVar;
extern vector<string> includePaths;
extern vector<string> customArguments;
extern Token copy_token(Token token);

// ------------------------- STACK SHORTHANDS ------------------------------ //

inline Token stack_back_safe()
{
	if (stutskStack.empty()) throw StutskException(ET_ERROR, "Stack is empty");
	return stutskStack.back();
}

inline void pushFloat(stutskFloat a) {
	Token newToken(T_FLOAT);
	newToken.data.asFloat = a;
	stutskStack.push_back(newToken);
}

inline void pushInteger(stutskInteger a) {
	Token newToken(T_INTEGER);
	newToken.data.asInteger = a;
	stutskStack.push_back(newToken);
}

inline void pushBool(bool a) {
	Token newToken(T_BOOL);
	newToken.data.asBool = a;
	stutskStack.push_back(newToken);
}

inline StringPtr pushString() {
	Token newToken(T_STRING);
	newToken.asString = StringPtr(new string);
	stutskStack.push_back(newToken);
	return newToken.asString;
}

// --------------------------------------------------------

#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <boost/integer.hpp>

extern boost::asio::io_service io_service;

class Debugger : boost::noncopyable {
private:
	string debugger_hostname;
	deque<Context*> contextStack;
	int debugger_port;
	bool debugging;
	boost::shared_ptr<boost::asio::ip::tcp::socket> socket;

	class DebuggerException : std::exception {};

	void send_message(const void* data, size_t length);
	template<class T> void send_message(T data);
	void read_message(void* data, size_t length);
	template<typename T> void read_message(T& data);
	template<class T> void dump_list(const T& tokens);
	void dump_dictionary(const TokenMap& tokens);
	void dump_token(const Token& token);
	void message_loop();
	bool parse_message(boost::uint16_t message_id);
public:
	static Debugger& get_instance() 
	{
		static Debugger instance;
		return instance;
	};

	bool is_debugging();
	void connect(string hostname, int port);
	void step();
	void step_in(Context* context, const string& name);
	void step_out();
	void disconnect();
	
	Debugger() : debugging(false) {};
	~Debugger();
};

#endif
