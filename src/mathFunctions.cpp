/*
mathFunctions.cpp - Mathematical Stutsk functions built into the interpreter are implemented here
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
#include <boost/math/special_functions.hpp>
#include <cmath>
#include <boost/random.hpp>

void BuiltIns::_f_round(Context* context) {
    /* arguments: <T_FLOAT value> round
	   returnvalue: <T_INTEGER> 
	   description: Returns `value` rounded to the nearest integer.
	   notes:
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger value = (stutskInteger)(0.5 + giveFloat(upper));
	pushInteger(value);
}

void BuiltIns::_f_floor(Context* context) {
    /* arguments: <T_FLOAT value> floor
	   returnvalue: <T_INTEGER> 
	   description: Returns `value` rounded down.
	   notes:
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger value = (stutskInteger)floor(giveFloat(upper));
	pushInteger(value);
}

void BuiltIns::_f_ceil(Context* context) {
    /* arguments: <T_FLOAT value> ceil
	   returnvalue: <T_INTEGER> 
	   description: Returns `value` rounded up.
	   notes:
	*/
	Token upper = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger value = (stutskInteger)ceil(giveFloat(upper));
	pushInteger(value);
}

void BuiltIns::_f_sqrt(Context* context) {
    /* arguments: <T_FLOAT value> sqrt
	   returnvalue: <T_FLOAT> 
	   description: Returns the square root of `value`.
	   notes:
	*/
	Token sqrtToken = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = sqrt(giveFloat(sqrtToken));
	pushFloat(value);
}

void BuiltIns::_f_abs(Context* context) {
    /* arguments: <T_FLOAT value> abs
	   returnvalue: <T_FLOAT> 
	   description: Returns the absolute value of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger i1;
	stutskFloat f1;
	string s1;
	switch (giveGCD(token, f1, i1, s1))
	{
	case NT_INVALID:
	case NT_STRING:
		throw StutskException(ET_ERROR, "Token not a numeric type"); 
		break;
	case NT_FLOAT: 
		f1 = fabs(f1);
		pushFloat(f1);
		break;
	case NT_INTEGER: 
		i1 = abs(i1);
		pushInteger(i1);
		break;
	}
}

void BuiltIns::_f_sinh(Context* context) {
    /* arguments: <T_FLOAT value> sinh
	   returnvalue: <T_FLOAT> 
	   description: Returns the hyperbolic sine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = sinh(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_cosh(Context* context) {
    /* arguments: <T_FLOAT value> cosh
	   returnvalue: <T_FLOAT> 
	   description: Returns the hyperbolic cosine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = cosh(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_tanh(Context* context) {
    /* arguments: <T_FLOAT value> tanh
	   returnvalue: <T_FLOAT> 
	   description: Returns the hyperbolic tangens of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = tanh(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_asinh(Context* context) {
    /* arguments: <T_FLOAT value> asinh
	   returnvalue: <T_FLOAT> 
	   description: Returns the inverse hyperbolic sine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = boost::math::asinh(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_acosh(Context* context) {
    /* arguments: <T_FLOAT value> acosh
	   returnvalue: <T_FLOAT> 
	   description: Returns the hyperbolic cosine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = boost::math::acosh(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_atanh(Context* context) {
    /* arguments: <T_FLOAT value> atanh
	   returnvalue: <T_FLOAT> 
	   description: Returns the inverse hyperbolic tangens of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = boost::math::atanh(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_sin(Context* context) {
    /* arguments: <T_FLOAT value> sin
	   returnvalue: <T_FLOAT> 
	   description: Returns the sine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = sin(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_cos(Context* context) {
    /* arguments: <T_FLOAT value> cos
	   returnvalue: <T_FLOAT> 
	   description: Returns the cosine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = cos(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_tan(Context* context) {
    /* arguments: <T_FLOAT value> tan
	   returnvalue: <T_FLOAT> 
	   description: Returns the tangens of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = tan(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_asin(Context* context) {
    /* arguments: <T_FLOAT value> asin
	   returnvalue: <T_FLOAT> 
	   description: Returns the inverse sine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = asin(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_acos(Context* context) {
    /* arguments: <T_FLOAT value> acos
	   returnvalue: <T_FLOAT> 
	   description: Returns the inverse cosine of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = acos(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_atan(Context* context) {
    /* arguments: <T_FLOAT value> atan
	   returnvalue: <T_FLOAT> 
	   description: Returns the inverse tangens of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = atan(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_log(Context* context) {
    /* arguments: <T_FLOAT value> log
	   returnvalue: <T_FLOAT> 
	   description: Returns the natural logarithm of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = log(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_log10(Context* context) {
    /* arguments: <T_FLOAT value> log10
	   returnvalue: <T_FLOAT> 
	   description: Returns the base-10 logarithm of `value`.
	   notes:
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	stutskFloat value = log10(giveFloat(token));
	pushFloat(value);
}

void BuiltIns::_f_pi(Context* context) {
    /* arguments: pi
	   returnvalue: <T_FLOAT> 
	   description: Returns the value of constant Pi (3.14159...)
	   notes:
	*/
	stutskFloat value = boost::math::constants::pi<stutskFloat>();
	pushFloat(value);
}

void BuiltIns::_f_euler(Context* context) {
    /* arguments: euler
	   returnvalue: <T_FLOAT> 
	   description: Returns the value of constant e (2.7182...)
	   notes:
	*/
	stutskFloat value = boost::math::constants::e<stutskFloat>();
	pushFloat(value);
}

void BuiltIns::_f_random(Context* context) {
    /* arguments: <T_INTEGER max> random
	   arguments: ( <T_INTEGER min> <T_INTEGER max> ) random
	   returnvalue: <T_INTEGER>
	   description: Returns either a whole random number on interval [0, max) if provided an integer
	     argument or a random number on interval [min, max] if provided an array.
	   notes: Uses Mersene Twister that is automatically seeded with system time when the interpreter starts.
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token);

	if (token.tokenType == T_ARRAY)
	{
		if (token.asTokenList->size() != 2)
			throw StutskException(ET_ERROR, "Invalid array length");
		stutskInteger min = giveInteger((*token.asTokenList)[0]);
		stutskInteger max = giveInteger((*token.asTokenList)[1]);
		if (min > max)
			throw StutskException(ET_ERROR, "Upper bound must bi higher than lower bound");

		boost::random::uniform_int_distribution<stutskInteger> distribution(0, max-min); // Max	
		pushInteger(distribution(RandomNumber::getInstance().generator)+min);
		// Vem, kaj si najbrž misliš tukaj ampak, ko sem to pisal, je tukaj bil en nasty bug v boostu,
		// namreè èe je stutskInteger = long long, potem je -1 vrgel ven kot 2^32, tako da je rajši zdaj tako 
	}
	else
	{
		stutskInteger max = giveInteger(token);
		if (max < 1)
			throw StutskException(ET_ERROR, "Number must be positive"); // [sic]
		boost::random::uniform_int_distribution<stutskInteger> distribution(0, max-1);	
		pushInteger(distribution(RandomNumber::getInstance().generator));
	}
}

void BuiltIns::_f_random_float(Context* context) {
    /* arguments: randomfloat
	   returnvalue: <T_FLOAT>
	   description: Returns either a real random number on interval [0, 1)
	   notes: Uses Mersene Twister that is automatically seeded with system time when the interpreter starts.
	*/
	boost::random::uniform_01<stutskFloat, stutskFloat>	distribution;
		pushFloat(distribution(RandomNumber::getInstance().generator));
}

const char baseChars[] = "0123456789abcdefghijklmnopqrstuvwxyz";

int getDigit(char chr)
{
	for (int i=0;i<36;i++)
		if (baseChars[i] == tolower(chr))
			return i;
	throw StutskException(ET_ERROR, "Invalid char in numeric string.");
}

void BuiltIns::_f_from_base(Context* context) {
    /* arguments: <T_STRING num_string> <T_INTEGER base> from_base
	   returnvalue: <T_INTEGER>
	   description: Converts `num_str` from numerical base of `base` to integer.
	   notes: Maximum base is 36 (digits are "0123456789abcdefghijklmnopqrstuvwxyz"). 
	     Negative numbers are written with '-' in front.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger base = giveInteger(token1);
	if (base < 2)
	throw StutskException(ET_ERROR, "Base must be higher than or equal to 2.");
	if (base > 36)
	throw StutskException(ET_ERROR, "Base cannot be higher than 36.");
	stutskInteger result = 0; 
	StringPtr text = giveString(token2);
	if (text->length() == 0)
	throw StutskException(ET_ERROR, "String is empty.");
	unsigned long i = 0; bool negative = false;
	if ((*text)[0] == '-') {
		i++;
		negative = true;
	}
	for (;i<text->length();i++)
		result = result * base + getDigit((*text)[i]);  
	pushInteger(negative ? -result : result);
}

void BuiltIns::_f_to_base(Context* context) {
    /* arguments: <T_INTEGER num> <T_INTEGER base> to_base
	   returnvalue: <T_STRING>
	   description: Converts `num` to numerical base of `base`.
	   notes: Maximum base is 36 (digits are "0123456789abcdefghijklmnopqrstuvwxyz").
	     Negative numbers are written with '-' in front.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	stutskInteger base = giveInteger(token1);
	stutskInteger number = giveInteger(token2);
	bool negative = (number < 0); number = abs(number);
	if (base < 2)
	throw StutskException(ET_ERROR, "Base must be higher than or equal to 2.");
	if (base > 36)
		throw StutskException(ET_ERROR, "Base cannot be higher than 36.");
	auto result = pushString();
	do
	{
		*result = baseChars[number % base] + *result;
		number = number / base;
	} while (number > 0);
	if (negative) *result = "-" + *result;
}

void BuiltIns::_f_max(Context* context) {
    /* arguments: <T_ARRAY values> max
	   returnvalue: <token>
	   description: Returns element of array with the highes numerical value.
	   notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token);
	
	if (token.tokenType != T_ARRAY)
		throw StutskException(ET_ERROR, "Token is not an array.");
	if (token.asTokenList->size() == 0)
		throw StutskException(ET_ERROR, "Array is empty.");
    
	Token maxToken;
	TokenList::iterator it = token.asTokenList->begin();
	maxToken = *(it++);

	for (;it != token.asTokenList->end(); ++it) {
		if (tokenNumericCompare(*it, maxToken) != CN_LARGER)
		  maxToken = *it;
	}

	stutskStack.push_back(maxToken);
}

void BuiltIns::_f_min(Context* context) {
    /* arguments: <T_ARRAY values> min
	   returnvalue: <token>
	   description: Returns element of array with the lowest numerical value.
	   notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token);
	
	if (token.tokenType != T_ARRAY)
		throw StutskException(ET_ERROR, "Token is not an array.");
	if (token.asTokenList->size() == 0)
		throw StutskException(ET_ERROR, "Array is empty.");
    
	Token minToken;
	TokenList::iterator it = token.asTokenList->begin();
	minToken = *(it++);

	for (;it != token.asTokenList->end(); ++it) {
		if (tokenNumericCompare(*it, minToken) != CN_SMALLER)
		  minToken = *it;
	}

	stutskStack.push_back(minToken);
}
