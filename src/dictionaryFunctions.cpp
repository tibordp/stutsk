/*
dictionaryFunctions.cpp - Associative container Stutsk functions built into the interpreter are implemented here
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

void BuiltIns::_f_dictionary_new(Context* context) {
	/* arguments: dictionary_new
	   returnvalue: <T_DICTIONARY> 
	   description: Creates a new dictionary.
	   notes: 
	*/
	Token newDictionary(T_DICTIONARY);
	newDictionary.asDictionary = TokenMapPtr(new TokenMap());
	stutskStack.push_back(newDictionary);
}

void BuiltIns::_f_dictionary_get(Context* context) {
	/* arguments: <key> <T_DICTIONARY dict> dictionary_set
	   returnvalue: <value> 
	   description: Returns value referenced by key `key` in dictionary `dict`.
	   notes: 
	*/
	Token dict_token = stack_back_safe();
	stutskStack.pop_back();
	Token index_token = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(dict_token);
	
	StringPtr index = giveString(index_token);

	if (dict_token.tokenType != T_DICTIONARY)
	{
		throw StutskException(ET_ERROR, "Token is not a dictionary.");
	}

	TokenMap::const_iterator it = dict_token.asDictionary->find(*index);
  
	if ( it == dict_token.asDictionary->end() )
	    stutskStack.push_back(Token(T_EMPTY));
    else 
		stutskStack.push_back(copy_token(it->second));
}


void BuiltIns::_f_dictionary_set(Context* context) {
	/* arguments: <value> <key> <T_DICTIONARY dict> dictionary_set
	   returnvalue: 
	   description: Sets `key` in dictionary `dict` to `value`.
	   notes: 
	*/
	Token dict_token = stack_back_safe();
	stutskStack.pop_back();
	Token index_token = stack_back_safe();
	stutskStack.pop_back();
	Token value_token = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(dict_token);
		
	StringPtr index = giveString(index_token);

	if (dict_token.tokenType != T_DICTIONARY)
	{
		throw StutskException(ET_ERROR, "Token is not a dictionary.");
	}

	(*dict_token.asDictionary)[*index] = copy_token(value_token);
}