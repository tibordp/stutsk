/*
arrayFunctions.cpp - Memory management Stutsk functions built into the interpreter are implemented here
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

void BuiltIns::_f_length(Context* context) {
	/* arguments: <T_ARRAY token> length
	   arguments: <T_STRING token> length
	   arguments: <T_DICTIONARY token> length
	   returnvalue: <T_INTEGER>
	   description: Returns the number of elements in token (characters in strings)
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();	

	recurseVariables(token1);

	switch (token1.tokenType)
	{
		case T_ARRAY: 
			pushInteger(token1.asTokenList->size());
			break;
		case T_DICTIONARY: 
			pushInteger(token1.asDictionary->size());
			break;
		default:
			pushInteger(giveString(token1)->size());
			break;
	}		
}


void BuiltIns::_f_setlength(Context* Context)
{
	/* arguments: <T_ARRAY token> setlength
	   arguments: <T_STRING token> setlength
	   arguments: <T_VARIABLE token> setlength
	   returnvalue: <T_ARRAY>
	   returnvalue: <T_STRING>
	   returnvalue: 
	   description: Resizes an array/string. If token is a variable it doesn't return anything
	     otherwise returns a resized array/string.
	   notes: If new length is larger, it fills the string with \0 and array with T_EMPTY tokens 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	stutskInteger newLength = giveInteger(token2);

	bool isVariable = token1.tokenType == T_VARIABLE;

	recurseVariables(token1); 

	if (newLength < 0) 
		throw StutskException(ET_ERROR, "Length cannot be negative");

	switch (token1.tokenType)
	{
		case T_ARRAY: 
			token1.asTokenList->resize(newLength, Token(T_EMPTY));
			break;		
		case T_STRING: 
			token1.asString->resize(newLength, '\n');
			break;	
		default:
			{
			  StringPtr token_string = giveString(token1);
			  token_string->resize(newLength, '\0');
			  token1.tokenType = T_STRING;
			  token1.asString.swap(token_string); // TODO - preveri
			}
			break;
	}	

	if (!isVariable)
		stutskStack.push_back(token1); 
}


void BuiltIns::_f_slice(Context* Context) {
	/* arguments: <T_ARRAY token> <T_INTEGER start> <T_INTEGER end> slice
	   arguments: <T_STRING token> <T_INTEGER start> <T_INTEGER end> slice
	   returnvalue: <T_ARRAY>
	   returnvalue: <T_STRING>
	   description: Extract elements from array/returns a substring from index start to index end (inclusively)
	   notes: 
	*/
	Token tEnd = stack_back_safe();
	stutskStack.pop_back();

	Token tStart = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger startIdx = giveInteger(tStart);

	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger endIdx = giveInteger(tEnd);

	recurseVariables(token1);  

	switch (token1.tokenType)
	{
		case T_ARRAY: 
			{ 
				if (token1.tokenType != T_ARRAY)
					throw StutskException(ET_ERROR, "Token is not an array");
				if (startIdx < 0 || endIdx < 0)
					throw StutskException(ET_ERROR, "Array index underflow");
				if (startIdx >= token1.asTokenList->size() || 
					endIdx   >= token1.asTokenList->size())
					throw StutskException(ET_ERROR, "Array index overflow");
				if (startIdx > endIdx)
					throw StutskException(ET_ERROR, "Index mismatch");

				Token newArray(T_ARRAY);
				newArray.asTokenList = TokenListPtr (new 
					TokenList(token1.asTokenList->begin() + startIdx,
					token1.asTokenList->begin() + endIdx + 1));

				stutskStack.push_back(newArray);
				break; 
			}
		default:
			{
				StringPtr hayStack = giveString(token1);

				if (startIdx < 0 || endIdx < 0)
					throw StutskException(ET_ERROR, "String index underflow");
				if ( startIdx >= hayStack->size() || 
					endIdx   >= hayStack->size() )
					throw StutskException(ET_ERROR, "String index overflow");
				if (startIdx > endIdx)
					throw StutskException(ET_ERROR, "Index mismatch");

				stutskInteger start = giveInteger(tStart), end = giveInteger(tEnd); 
				*pushString() = hayStack->substr(start, end-start+1);
			}
			break;
	}		
}