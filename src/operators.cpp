/*
operators.cpp - Stutsk operators are implemented here
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
#include <cmath>

OperatorMap Operators::populateOperators() {
	OperatorMap operMap;
	// Real "operators"
	operMap["="] = OP_ASSIG; // check
	operMap["&="] = OP_ASSIG_REF; // check
	operMap["@"] = OP_DEREF; // check
	operMap["++"] = OP_PLUSPLUS; // check
	operMap["--"] = OP_MINMIN; // check
	operMap["=="] = OP_EQ; // check
	operMap["==="] = OP_SAME; // check
	operMap["!="] = OP_NOTEQ; // check
	operMap["<"] = OP_LESSTHAN; // check
	operMap[">"] = OP_MORETHAN; // check
	operMap["<="] = OP_LESSTHAN_EQ; // check
	operMap[">="] = OP_MORETHAN_EQ; // check
	operMap["||"] = OP_OR; // check
	operMap["&&"] = OP_AND; // check
	operMap["!"] = OP_NOT; // check
	operMap["+"] = OP_PLUS; // check
	operMap["-"] = OP_MINUS; // check
	operMap["*"] = OP_MULTIP; // check
	operMap["/"] = OP_DIV; // check
	operMap["^"] = OP_POWER; // check
	operMap["div"] = OP_DIVINT; // check
	operMap["%"] = OP_MOD; // check
	operMap["?"] = OP_TERNARY; // check
	operMap["."] = OP_CONCAT; // check
	operMap["[]"] = OP_ARRAY; // check
	// Control structures
	operMap["if"] = OP_IF; // check
	operMap["ifelse"] = OP_IFELSE; // check

	operMap["repeat"] = OP_REPEAT; // check
	operMap["forever"] = OP_FOREVER; // check
	operMap["foreach"] = OP_FOREACH; // check
	operMap["switch"] = OP_SWITCH; // check

	operMap["break"] = OP_BREAK; // check
	operMap["continue"] = OP_CONTINUE; // check
	operMap["exit"] = OP_EXIT; // check	
	operMap["halt"] = OP_HALT; // check

	operMap["global"] = OP_GLOBAL; // check
	operMap["auto"] = OP_AUTO; // check
	operMap["static"] = OP_STATIC; // check

	operMap["function"] = OP_FUNC; // check
	operMap["unset"] = OP_UNSET; // check

	operMap["try"] = OP_TRY; // check
	operMap["throw"] = OP_THROW; // check

	return operMap;
}

bool Operators::readOperator(const string& input, OperatorType &operType) {
	OperatorMap::iterator iter;

	iter = stutskOperators.find(input);

	if (iter != stutskOperators.end()) {
		operType = iter->second;
		return true;
	}
	else
		return false;
}

void setVariable(Token& var, Token& value, bool saveAsReference = false)
{
	stutskInteger i1;
	if (var.tokenType != T_VARIABLE) {
		throw StutskException(ET_ERROR, "Token is not a variable");
	}
	else {
		if (!saveAsReference)
			recurseVariables(value);

		if (var.index.size() > 0) {
			i1 = recurseVariables(var, false, false);
			if (var.tokenType == T_ARRAY) {
				if (i1 < (signed)var.asTokenList->size()) {
					if (saveAsReference)
						(*var.asTokenList)[i1] = value;
					else
						(*var.asTokenList)[i1] = copy_token(value);
				}
				else
					throw StutskException(ET_ERROR, "Array index overflow");
			}
			else if (var.tokenType == T_STRING && !saveAsReference) {
				if (i1 < (signed)var.asString->length()) {
					string s1 = *giveString(value);
					if (s1.length() == 1)
						(*var.asString)[i1] = s1[0];
					else
						throw StutskException(ET_ERROR,
						"Left side is not a char");

				}
				else
					throw StutskException(ET_ERROR,
					"String index overflow");
			}
			else
				throw StutskException(ET_ERROR,
				"Right side cannot be assigned to");

		}
		else {
			if (saveAsReference)
				var.data.asVariable.context->variables
				[var.data.asVariable.name] = value;
			else
				var.data.asVariable.context->variables
				[var.data.asVariable.name] = copy_token(value);
		}
	}
}

void Operators::doOperator(Context *context, OperatorType oper) {
	Token token1, token2, token3;
	stutskFloat f1, f2;
	stutskInteger i1, i2;
	string s1, s2;

	switch (oper) {
	case OP_ASSIG:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Variable
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Value
		setVariable(token1, token2); 
		break;
	case OP_ASSIG_REF:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Variable
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Value
		setVariable(token1, token2, true); 
		break;
	case OP_FUNC:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Name
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		recurseVariables(token2);
		if (token2.tokenType != T_CODEBLOCK) {
			throw StutskException(ET_ERROR, "Token is not a codeblock");
		}
		else {
			OperatorType dummy;
			string functionName = *giveString(token1);
			if (readOperator(functionName, dummy))
				throw StutskException(ET_ERROR, "Cannot override an operator");
			if (functionName.substr(0,10) == "__builtin_")
				throw StutskException(ET_ERROR, "Cannot override an internal function");
			userFunctions[functionName] = *token2.asTokenList;
		}

		break;
	case OP_UNSET:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		if (token1.tokenType == T_VARIABLE) {
			struct Token::_un_TokenData::_un_VariableData *var = & token1.data.asVariable;
			TokenMap::iterator iter = var->context->variables.find(var->name);
			if (iter != var->context->variables.end())
				var->context->variables.erase(iter);
		}
		else {
			UserFunctionsMap::iterator iter = userFunctions.find(*giveString(token1));
			if (iter != userFunctions.end())
				userFunctions.erase(iter);
		}
		break;
	case OP_DEREF:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		if (token1.tokenType != T_VARIABLE) {
			throw StutskException(ET_ERROR, "Token is not a variable");
		}
		else {
			recurseVariables(token1, true);
			stutskStack.push_back(copy_token(token1));
		}
		break;
	case OP_PLUSPLUS:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Name
		if (token1.tokenType == T_VARIABLE) {
			switch (giveGCD(token1, f1, i1, s1)) {
			case NT_INVALID:
			case NT_STRING:
				throw StutskException(ET_ERROR,
					"Variable is not a numeric type");
			case NT_INTEGER:
				token2.tokenType = T_INTEGER;
				token2.data.asInteger = i1 + 1;
				setVariable(token1, token2);
				break;
			case NT_FLOAT:
				token2.tokenType = T_FLOAT;
				token2.data.asFloat = f1 + 1.;
				setVariable(token1, token2); 
				break;
			}
		}
		else {
			switch (giveGCD(token1, f1, i1, s1)) {
			case NT_INVALID:
			case NT_STRING:
				throw StutskException(ET_ERROR, "Token is not a numeric type");
			case NT_INTEGER:
				pushInteger(++i1);
				break;
			case NT_FLOAT:
				pushFloat(++f1);
				break;
			}
		}
		break;
	case OP_MINMIN:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Name
		if (token1.tokenType == T_VARIABLE) {
			switch (giveGCD(token1, f1, i1, s1)) {
			case NT_INVALID:
			case NT_STRING:
				throw StutskException(ET_ERROR,
					"Variable is not a numeric type");
			case NT_INTEGER:
				token2.tokenType = T_INTEGER;
				token2.data.asInteger = i1 - 1;
				setVariable(token1, token2); 
				break;
			case NT_FLOAT:
				token2.tokenType = T_FLOAT;
				token2.data.asFloat = f1 - 1.;
				setVariable(token1, token2); 
				break;
			}
		}
		else {
			switch (giveGCD(token1, f1, i1, s1)) {
			case NT_INVALID:
			case NT_STRING:
				throw StutskException(ET_ERROR, "Token is not a numeric type");
			case NT_INTEGER:
				pushInteger(--i1);
				break;
			case NT_FLOAT:
				pushFloat(--f1);
				break;
			}
		}
		break;
	case OP_PLUS:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

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
				i1 += i2;
				pushInteger(i1);
				break;
			case NT_FLOAT:
				f2 += i1;
				pushFloat(f2);
				break;
			}
			break;
		case NT_FLOAT:
			f1 += giveFloat(token2);
			pushFloat(f1);
			break;
		}

		break;
	case OP_MINUS:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

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
				i2 -= i1;
				pushInteger(i2);
				break;
			case NT_FLOAT:
				f2 -= i1;
				pushFloat(f2);
				break;
			}
			break;
		case NT_FLOAT:
			f2 = giveFloat(token2) - f1;
			pushFloat(f2);
			break;
		}

		break;
	case OP_MULTIP:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

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
				i1 *= i2;
				pushInteger(i1);
				break;
			case NT_FLOAT:
				f2 *= i1;
				pushFloat(f2);
				break;
			}
			break;
		case NT_FLOAT:
			f1 *= giveFloat(token2);
			pushFloat(f1);
			break;
		}
		break;
	case OP_DIV:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		switch (giveGCD(token1, f1, i1, s1)) {
		case NT_STRING:
		case NT_INVALID:
			throw StutskException(ET_ERROR, "Token is not a numeric type");
		case NT_INTEGER:
			if (i1 == 0)
				throw StutskException(ET_ERROR, "Division by zero");
			switch (giveGCD(token2, f2, i2, s1)) {
			case NT_STRING:
			case NT_INVALID:
				throw StutskException(ET_ERROR, "Token is not a numeric type");
			case NT_INTEGER:
				f1 = (stutskFloat)i2 / (stutskFloat)i1;
				pushFloat(f1);
				break;
			case NT_FLOAT:
				f2 /= (stutskFloat)i1;
				pushFloat(f2);
				break;
			}
			break;
		case NT_FLOAT:
			if (f1 == 0.)
				throw StutskException(ET_ERROR, "Division by zero");
			f2 = giveFloat(token2) / f1;
			pushFloat(f2);
			break;
		}
		break;
	case OP_DIVINT:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

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
				i1 = (stutskInteger)i2 / i1;
				pushInteger(i1);
				break;
			case NT_FLOAT:
				i2 = (stutskInteger)f2 / i1;
				pushInteger(i2);
				break;
			}
			break;
		case NT_FLOAT:
			i2 = giveInteger(token2) / (stutskInteger)f1;
			pushInteger(i2);
			break;
		}
		break;
	case OP_MOD:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

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
				i1 = (stutskInteger)i2 % i1;
				pushInteger(i1);
				break;
			case NT_FLOAT:
				i2 = (stutskInteger)f2 % i1;
				pushInteger(i2);
				break;
			}
			break;
		case NT_FLOAT:
			i2 = giveInteger(token2) % (stutskInteger)f1;
			pushInteger(i2);
			break;
		}
		break;
	case OP_LESSTHAN:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		pushBool(tokenNumericCompare(token1, token2) == CN_SMALLER);
		break;
	case OP_MORETHAN:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		pushBool(tokenNumericCompare(token1, token2) == CN_LARGER);
		break;
	case OP_LESSTHAN_EQ:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		pushBool(tokenNumericCompare(token1, token2) != CN_LARGER);
		break;
	case OP_MORETHAN_EQ:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		pushBool(tokenNumericCompare(token1, token2) != CN_SMALLER);
		break;
	case OP_EQ:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		pushBool(tokenEqual(token1, token2));	
		break;
	case OP_SAME:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();
		pushBool(tokenSame(token1, token2));
		break;
	case OP_NOTEQ:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();

		pushBool(!tokenEqual(token1, token2));	
		break;
	case OP_GLOBAL:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Variable
		if (token1.tokenType != T_VARIABLE) {
			throw StutskException(ET_ERROR, "Token is not a variable");
		}
		else {
			context->variableScopeMap[token1.data.asVariable.name] = VS_GLOBAL;
		}
		break;
	case OP_AUTO:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Variable
		if (token1.tokenType != T_VARIABLE) {
			throw StutskException(ET_ERROR, "Token is not a variable");
		}
		else {
			context->variableScopeMap[token1.data.asVariable.name] = VS_AUTO;
		}
		break;
	case OP_STATIC:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Variable
		if (token1.tokenType != T_VARIABLE) {
			throw StutskException(ET_ERROR, "Token is not a variable");
		}
		else {
			context->variableScopeMap[token1.data.asVariable.name] = VS_STATIC;
		}
		break;
	case OP_FOREVER:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		recurseVariables(token1);
		if (token1.tokenType != T_CODEBLOCK) {
			throw StutskException(ET_ERROR, "Token is not a codeblock");
		}
		else {
			while (true) {
				context->run(*token1.asTokenList, "forever");
				if (exitVar == OP_CONTINUE)
					exitVar = OP_INVALID;
				if (exitVar != OP_INVALID)
					break;
			}
			if (exitVar == OP_BREAK)
				exitVar = OP_INVALID;
		}
		break;
	case OP_FOREACH:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		recurseVariables(token1);
		if (token1.tokenType != T_CODEBLOCK)
			throw StutskException(ET_ERROR, "Token is not a codeblock");

		recurseVariables(token2);
		if (token2.tokenType == T_ARRAY) {
			for (TokenList::const_iterator it = token2.asTokenList->begin();
				it != token2.asTokenList->end(); ++it) {
					stutskStack.push_back(*it);
					stringstream ss;
					ss << "foreach [" << 
						std::distance(token2.asTokenList->cbegin(), it) << "]";
					context->run(*token1.asTokenList, ss.str());
					if (exitVar == OP_CONTINUE)
						exitVar = OP_INVALID;
					if (exitVar != OP_INVALID)
						break;
			}
		}
		else
			if (token2.tokenType == T_DICTIONARY) {
				for (TokenMap::const_iterator it = token2.asDictionary->begin();
					it != token2.asDictionary->end(); ++it) {
						stutskStack.push_back(it->second);
						StringPtr f = pushString();
						*f =  it->first;

						stringstream ss;
						ss << "foreach [" << it->first << "]";
						context->run(*token1.asTokenList, ss.str());
						if (exitVar == OP_CONTINUE)
							exitVar = OP_INVALID;
						if (exitVar != OP_INVALID)
							break;
				}
			}
			else 
			{
				StringPtr s1_ptr = giveString(token2);
				for (int i = 0; i < (signed)s1_ptr->length(); i++) {
					*pushString() = (*s1_ptr)[i];
					stringstream ss;
					ss << "foreach [" << i << "]";
					context->run(*token1.asTokenList, ss.str());
					if (exitVar == OP_CONTINUE)
						exitVar = OP_INVALID;
					if (exitVar != OP_INVALID)
						break;
				}
			}

			if (exitVar == OP_BREAK)
				exitVar = OP_INVALID;
			break;

	case OP_REPEAT:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		i1 = giveInteger(token2);
		recurseVariables(token1);

		if (token1.tokenType != T_CODEBLOCK) {
			throw StutskException(ET_ERROR, "Token is not a codeblock");
		}
		else {
			while (i1 > 0) {
				stringstream ss;
				ss << "repeat [" << i1 << "]";
				context->run(*token1.asTokenList, ss.str());
				if (exitVar == OP_CONTINUE)
					exitVar = OP_INVALID;
				if (exitVar != OP_INVALID)
					break;
				i1--;
			}
			if (exitVar == OP_BREAK)
				exitVar = OP_INVALID;
		}
		break;
	case OP_TRY:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		recurseVariables(token1);
		recurseVariables(token2);
		if ((token2.tokenType != T_CODEBLOCK) ||
			(token2.tokenType != T_CODEBLOCK)) {
				throw StutskException(ET_ERROR, "Token is not a codeblock");
		}
		else {
			try {
				context->run(*token2.asTokenList, "try");
			}
			catch (StutskException &e) {
				*pushString() = e.getMessage();
				pushInteger(e.getLineNumber());
				*pushString() = e.getFileName();

				context->run(*token1.asTokenList, "try");
			}
			catch (std::exception &e) {
				*pushString() = e.what();
				pushInteger(errorToken.lineNum);
				*pushString() = ParseContext::get_by_id(errorToken.context_id).FileName;

				context->run(*token1.asTokenList, "try");
			}
		}
		break;
	case OP_POWER:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		f1 = pow(giveFloat(token2), giveFloat(token1));
		pushFloat(f1);

		break;
	case OP_THROW:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Error name
		throw StutskException(ET_CUSTOM, *giveString(token1));
		break;
	case OP_NOT:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		pushBool(!giveBool(token1));
		break;
	case OP_AND:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();
		pushBool(giveBool(token1) && giveBool(token2));
		break;
	case OP_OR:
		token1 = stack_back_safe();
		stutskStack.pop_back();
		token2 = stack_back_safe();
		stutskStack.pop_back();
		pushBool(giveBool(token1) || giveBool(token2));
		break;
	case OP_IF:
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		recurseVariables(token2);

		if (token2.tokenType != T_CODEBLOCK) {
			throw StutskException(ET_ERROR, "Token is not a codeblock");
		}
		else {
			if (giveBool(token1)) {
				context->run(*token2.asTokenList, "if");
			}
		}
		break;
	case OP_IFELSE:
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token3 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		recurseVariables(token2);
		recurseVariables(token3);

		if ((token2.tokenType != T_CODEBLOCK) ||
			(token3.tokenType != T_CODEBLOCK)) {
				throw StutskException(ET_ERROR, "Token is not a codeblock");
		}
		else {
			if (giveBool(token1)) {
				context->run(*token3.asTokenList, "ifelse");
			}
			else {
				context->run(*token2.asTokenList, "ifelse");
			}
		}
		break;
	case OP_TERNARY:
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token3 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		stutskStack.push_back((giveBool(token1) ? token3 : token2));
		break;
	case OP_CONCAT:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		// Be careful NOT to recurse variable here because we wouldn't want
		// to overwrite the saved value.

		if (token2.tokenType == T_STRING) // Performance optimization
		{
			token2.asString->append(*giveString(token1));
			stutskStack.push_back(token2);
		}
		else
			// Should be fast enough ...	
			*pushString() = *giveString(token2) + 
			*giveString(token1);

		break;
	case OP_SWITCH:
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		recurseVariables(token2);

		if (token2.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");
		else
		{	
			for (TokenList::iterator it = token2.asTokenList->begin();
				it != token2.asTokenList->end(); ++it) 
			{
				token3 = *it;
				recurseVariables(token3);
				if (token3.tokenType != T_ARRAY)
					throw StutskException(ET_ERROR, "Token is not an array");
				else
				{
					if (token3.asTokenList->size() != 2)
						throw StutskException(ET_ERROR, "Array of wrong length");
					if ((*token3.asTokenList)[1].tokenType != T_CODEBLOCK)
						throw StutskException(ET_ERROR, "Token is not a codeblock");
					if (tokenEqual(token1, (*token3.asTokenList)[0]))
					{
						context->run(*(*token3.asTokenList)[1].asTokenList, "switch");
						break;
					}
				}
			}
		}
		break;
	case OP_BREAK:
	case OP_CONTINUE:
	case OP_HALT:
	case OP_EXIT:
		exitVar = oper;
		break;
	case OP_ARRAY:
		token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		recurseVariables(token1);

		if (token2.tokenType == T_VARIABLE) {
			if (token1.tokenType == T_ARRAY) {
				for (TokenList::iterator it = token1.asTokenList->begin();
					it != token1.asTokenList->end(); ++it)
					token2.index.push_back(giveInteger(*it));
			}
			else
				token2.index.push_back(giveInteger(token1));
			stutskStack.push_back(token2);
		}
		else if (token2.tokenType == T_ARRAY) {
			if (token1.tokenType == T_ARRAY) {
				for (TokenList::iterator it = token1.asTokenList->begin();
					it != token1.asTokenList->end(); ++it) {
						i1 = giveInteger(*it);
						if (i1 < 0)
							throw StutskException(ET_ERROR, "Array index underflow");
						if ((signed)token2.asTokenList->size() >= i1 + 1) {
							token2 = (*token2.asTokenList)[i1];
						}
						else
							throw StutskException(ET_ERROR, "Array index overflow");
				}
				stutskStack.push_back(token2);
			}
			else {
				i1 = giveInteger(token1);
				if (i1 < 0)
					throw StutskException(ET_ERROR, "Array index underflow");
				if ((signed)token2.asTokenList->size() >= i1 + 1) {
					stutskStack.push_back((*token2.asTokenList)[i1]);
				}
				else
					throw StutskException(ET_ERROR, "Array index overflow");
			}
		}
		else {
			i1 = giveInteger(token1);
			StringPtr s1_ptr = giveString(token2);
			if (i1 < 0)
				throw StutskException(ET_ERROR, "String index underflow");
			if ((signed)s1_ptr->length() >= i1 + 1) {
				*pushString() = (*s1_ptr)[i1];
			}
			else
				throw StutskException(ET_ERROR, "String index overflow");
		}
	}
}
