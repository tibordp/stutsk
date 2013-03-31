/*
conversionUtils.cpp - dynamic type conversion is largely implemeted here
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
#include <algorithm>

/* getNth - returns n-th value of the array/string  */
void getNth(Token &newToken, stutskInteger i)
{
	Token tempToken = newToken; 
	if (tempToken.tokenType == T_VARIABLE) 
		recurseVariables(tempToken);
	if ((tempToken.tokenType == T_STRING) && (i >= 0)) {
		newToken.tokenType = T_STRING;
		if (i < 0)
			throw StutskException(ET_ERROR, "String index underflow");
		if (i >= (signed)tempToken.asString->length())
			throw StutskException(ET_ERROR, "String index overflow");
		newToken.asString = StringPtr(new string);
		*newToken.asString = (*tempToken.asString)[i];
	} 
	else if ((tempToken.tokenType == T_ARRAY) && (i >= 0)) {
		if (i < 0)
			throw StutskException(ET_ERROR, "Array index underflow");
		if (i >= (signed)tempToken.asTokenList->size())
			throw StutskException(ET_ERROR, "Array index overflow");
		newToken = (*tempToken.asTokenList)[i];
	}
	else
		throw StutskException(ET_ERROR, "Cannot index an object");
}

/* recurseVariables - recurses the references variable token and returns its final value
     dumpLast -  if true (default) returns the final value, if false, it returns an array/string 
	  	         containing the final value
     noRecurse - if false (default) recurses all the way through the references, otherwise only 
	  			 removes ONE layer of indirection */
stutskInteger recurseVariables(Token & newToken, bool norecurse, bool dumpLast) {
	if (newToken.tokenType == T_VARIABLE) {
		TokenMap::iterator iter;
		iter = newToken.data.asVariable.context->variables.find
			(newToken.data.asVariable.name);

		if (iter != newToken.data.asVariable.context->variables.end()) {
			vector<stutskInteger> origIndex = newToken.index;
			newToken = iter->second;
			if (newToken.tokenType == T_VARIABLE)
				if (!norecurse) recurseVariables(newToken);

			for (vector<stutskInteger>::iterator it = origIndex.begin(); it != origIndex.end(); ++it) 
			{
				if ((!dumpLast) && (it+1==origIndex.end()))
					return *it;                                    
				getNth(newToken, *it);                 
			}      

			if (newToken.tokenType == T_VARIABLE)
				if (!norecurse) recurseVariables(newToken);
		} 		
		else {
			stringstream ExceptionText;
			ExceptionText << "Variable '" << newToken.data.asVariable.name <<
				"' not defined in context";
			throw StutskException(ET_ERROR, ExceptionText.str());
		}
	}

	return 0;
}

// tokenNumericCompare - returns the size relation between two numerical tokens
CompareNumeric tokenNumericCompare(const Token& token1, const Token& token2)
{
	stutskInteger i1, i2;	
	stutskFloat f1, f2;
	string s1, s2;

	switch (giveGCD(token1, f1, i1, s1)) {
	case NT_STRING:
	case NT_INVALID:
		throw StutskException(ET_ERROR, "Token is not a numeric type");
	case NT_INTEGER:
		switch (giveGCD(token2, f2, i2, s1)) {
		case NT_STRING:
		case NT_INVALID:
			throw StutskException(ET_ERROR, "Token is not a numeric type");
		case NT_INTEGER:
			return compareValue(i2, i1);
			break;
		case NT_FLOAT:
			return compareValue(f2, i1);
			break;
		}
		break;
	case NT_FLOAT:
		return compareValue(giveFloat(token2) , f1);
		break;
	}
	throw StutskException(ET_ERROR, "Token is not a numeric type");
}

// tokenSame - checks whether tokens are strictly equal ( 1 != 1. != "1" != TRUE , ... )
bool tokenSame(const Token& token1, const Token& token2)
{
	if (token1.tokenType != token1.tokenType)
		return false;
	else
	{
		switch (token1.tokenType)
		{
			case T_EMPTY:     return true;
			case T_OPERATOR:  return token1.data.operatorType == token2.data.operatorType; 
			case T_FUNCCALL:  return strcmp(token1.data.asFunctionName, token2.data.asFunctionName) == 0;  
			case T_VARIABLE:  return strcmp(token1.data.asVariable.name, token1.data.asVariable.name) == 0 &&
										    token1.data.asVariable.context == token2.data.asVariable.context &&
							 			    token1.index == token2.index; 
			case T_INTEGER:   return token1.data.asInteger == token2.data.asInteger;
			case T_BOOL:      return token1.data.asInteger == token2.data.asInteger;
			case T_FLOAT:     return token1.data.asFloat == token2.data.asFloat;
			case T_STRING:    return *token1.asString == *token2.asString;
			case T_CODEBLOCK:
			case T_ARRAY:     if (token1.asTokenList->size() != token2.asTokenList->size())
							    return false;
				              // Be careful when using equal, it may not behave as you think
							  return equal(token1.asTokenList->begin(),token1.asTokenList->end(),
								 		   token2.asTokenList->begin(), tokenSame);
			case T_HANDLE:    return token1.data.asHandle.ptr == token2.data.asHandle.ptr &&
								  	 token1.data.asHandle.size == token2.data.asHandle.size;
			// Types that are not defined yet, should default to never  being the same
			default:          return false;
		}
	}
}

// tokenEqual - checks whether tokens are equal in terms of soft typing ( 1 = 1. = "1" = TRUE , ... )
bool tokenEqual(const Token& token1, const Token& token2)
{
	stutskInteger i1, i2;	
	stutskFloat f1, f2;
	string s1, s2;
	Token t1, t2;

	switch (giveGCD(token1, f1, i1, s1)) {
		// If the greatest common denominator of the token is a string.
		case NT_STRING:
			return (s1 == *giveString(token2));
		// If the greatest common denominator of the token is an integer.
		case NT_INTEGER:
			switch (giveGCD(token2, f2, i2, s2)) {
			  case NT_STRING:  return (*giveString(token1) == s2);
			  case NT_INTEGER: return (i2 == i1);
			  case NT_FLOAT:   return (f2 == i1);
			default:
				throw StutskException(ET_ERROR, "Token cannot be compared");
			}
		// If the greatest common denominator of the token is a float.
		case NT_FLOAT:
			switch (giveGCD(token2, f2, i2, s2)) {
			  case NT_STRING:  return (*giveString(token1) == s2);
			  case NT_INTEGER: return (f1 == i2);
			  case NT_FLOAT:   return (f1 == f2);
			default:
				throw StutskException(ET_ERROR, "Token cannot be compared");
			}
		// If the token cannot be soft-typed (arrays, handles, codeblocks)
		default:
			t1 = token1; t2 = token2; 
			recurseVariables(t1); recurseVariables(t2);

			if (t1.tokenType != t2.tokenType)
				throw StutskException(ET_ERROR, "Token cannot be compared");

			switch (t1.tokenType)
			{
			  case T_OPERATOR:  return t1.data.operatorType == t2.data.operatorType; 
			  case T_FUNCCALL:  return strcmp(t1.data.asFunctionName, t2.data.asFunctionName) == 0;  
			  case T_CODEBLOCK:
			  case T_ARRAY:     if (t1.asTokenList->size() != t2.asTokenList->size())
							      return false;
								// Be careful when using equal, it may not behave as you think			
								return equal(t1.asTokenList->begin(),t1.asTokenList->end(),
							                 t2.asTokenList->begin(), tokenEqual);
			  case T_HANDLE:    return t1.data.asHandle.ptr == t2.data.asHandle.ptr &&
							           t1.data.asHandle.size == t2.data.asHandle.size;
			  default: throw StutskException(ET_ERROR, "Token cannot be compared");
			}
	}
	throw StutskException(ET_ERROR, "Token cannot be compared");
}

/* giveGCD - returns the (figurative) greatest common denominator type of a token i.e.
			   for a T_STRING token "4.5" it returns NT_FLOAT. */
GCDType giveGCD(const Token& token, stutskFloat &floatV, stutskInteger &intV,
	string &stringV) {
		Token newToken;
		switch (token.tokenType) {
		case T_EMPTY:
			return NT_INVALID;
		case T_VARIABLE:
			newToken = token;
			recurseVariables(newToken);
			return giveGCD(newToken, floatV, intV, stringV);
		case T_INTEGER:
			intV = token.data.asInteger;
			return NT_INTEGER;
		case T_STRING:
			if (fromString(*token.asString, intV))
				return NT_INTEGER;
			else if (fromString(*token.asString, floatV))
				return NT_FLOAT;
			else {
				stringV = *token.asString;
				return NT_STRING;
			}
		case T_BOOL:
			intV = token.data.asBool ? 1 : 0;
			return NT_INTEGER;
		case T_FLOAT:
			floatV = token.data.asFloat;
			return NT_FLOAT;
		default:
			return NT_INVALID;
		}
}

/* giveGCD - tries to convert a token to T_INTEGER and returns its numerical value */
stutskInteger giveInteger(const Token& token) {
	Token newToken;
	switch (token.tokenType) {
	case T_EMPTY:
		return 0;
	case T_VARIABLE:
		newToken = token;
		recurseVariables(newToken);
		return giveInteger(newToken);
	case T_INTEGER:
		return token.data.asInteger;
	case T_STRING:
		stutskInteger intV;
		stutskFloat floatV;		
		if (fromString(*token.asString, intV))
			return intV;
		else if (fromString(*token.asString, floatV))
			return (stutskInteger) floatV;
		else {
			stringstream ExceptionText;
			ExceptionText << "Cannot convert '" << *token.asString <<
				"' to integer";
			throw StutskException(ET_ERROR, ExceptionText.str());
		}
	case T_BOOL:
		return token.data.asBool ? 1 : 0;
	case T_FLOAT:
		return (stutskInteger) token.data.asFloat;

	default:
		stringstream ExceptionText;
		ExceptionText << "Cannot convert token to integer";
		throw StutskException(ET_ERROR, ExceptionText.str());
	}
}

/* giveFloat - tries to convert a token to T_FLOAT and returns its numerical value */
stutskFloat giveFloat(const Token& token) {
	Token newToken;
	switch (token.tokenType) {
	case T_EMPTY:
		return 0;
	case T_VARIABLE:
		newToken = token;
		recurseVariables(newToken);
		return giveFloat(newToken);
	case T_INTEGER:
		return static_cast<stutskFloat>(token.data.asInteger);
	case T_STRING:
		stutskFloat number;
		if (fromString(*token.asString, number))
			return number;
		else {
			stringstream ExceptionText;
			ExceptionText << "Cannot convert '" << *token.asString <<
				"' to float";
			throw StutskException(ET_ERROR, ExceptionText.str());
		}
	case T_BOOL:
		return token.data.asBool ? 1. : 0.;
	case T_FLOAT:
		return token.data.asFloat;
	default:
		stringstream ExceptionText;
		ExceptionText << "Cannot convert token to float";
		throw StutskException(ET_ERROR, ExceptionText.str());
	}
}


/* giveString - returns a pointer to the string from a token. If token is not 
     T_STRING, it generates new string, converts token to it, then returns
	 its pointer. Use to prevent copying of long strings, when read-only access
	 is needed. 
	 Usage note: when string is small (regex, filename, ...), use:
		string value = *giveString(token); 
	 When string MAY be large and we don't change it (pos, substr, 
	 fwrite, writefile, ...), use: 
		StringPtr value = giveString(token); */
StringPtr giveString(const Token& token) {
	Token newToken;
	switch (token.tokenType) {
	case T_VARIABLE:
		newToken = token;
		recurseVariables(newToken);
		return giveString(newToken);
		break;
	case T_STRING:
		return token.asString;
		break;
	case T_EMPTY:
		return StringPtr(new string("")); 
		break;
	case T_INTEGER: {
		StringPtr result = StringPtr(new string);
		toString(token.data.asInteger, *result);
		return result; }
		break;
	case T_FLOAT: {
		StringPtr result = StringPtr(new string);
		toString(token.data.asFloat, *result);
		return result; }
		break;
	case T_BOOL:
		return StringPtr(new string(token.data.asBool ? "TRUE" : "FALSE"));
		break;
	default:
		stringstream ExceptionText;
		ExceptionText << "Cannot convert token to string";
		throw StutskException(ET_ERROR, ExceptionText.str());
	}
}

/* giveFloat - tries to convert a token to T_BOOL and returns its value */
bool giveBool(const Token& token) {
	Token newToken;
	switch (token.tokenType) {
	case T_EMPTY:
		return false;
	case T_VARIABLE:
		newToken = token;
		recurseVariables(newToken);
		return giveBool(newToken);
	case T_INTEGER:
		return (token.data.asInteger != 0);
	case T_STRING:
		return (token.asString->length() != 0);
	case T_BOOL:
		return token.data.asBool;
	case T_FLOAT:
		return (token.data.asFloat != 0);
	default:
		stringstream ExceptionText;
		ExceptionText << "Cannot convert token to boolean";
		throw StutskException(ET_ERROR, ExceptionText.str());
	}
}