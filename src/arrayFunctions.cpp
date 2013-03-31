/*
arrayFunctions.cpp - Array-related Stutsk functions built into the interpreter are implemented here
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
#include <algorithm>
#include <boost/random.hpp>

namespace BuiltIns {

	void _f_array_append(Context* Context)
	{
		/* arguments: <T_ARRAY array1> <T_ARRAY array2> array_append
		   arguments: <T_VARIABLE array1> <T_ARRAY array2> array_append
		   returnvalue: <T_ARRAY>
		   returnvalue:
		   description: Appends `array2` to `array1`. If `array` is a variable, it returns nothing, 
		     otherwise it returns a concatenated array.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();
		Token token2 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token2.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		recurseVariables(token2);   

		if (token2.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");      

		token2.asTokenList->insert( token2.asTokenList->end(), token1.asTokenList->begin(), token1.asTokenList->end() );

		if (!isVariable)
			stutskStack.push_back(token2);
	}

	void _f_array_insert(Context* Context)
	{
		/* arguments: <T_ARRAY array2> <T_INTEGER pos> <T_ARRAY array1> array_append
		   arguments: <T_ARRAY array2> <T_INTEGER pos> <T_VARIABLE array1> array_append
		   returnvalue: <T_ARRAY>
		   returnvalue:
		   description: Inserts `array2` into `array1` after `pos`-th element. 
		     If `array1` is a variable, it returns nothing, otherwise it returns the new array.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();
		Token token2 = stack_back_safe();
		stutskStack.pop_back();
		Token token3 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		stutskInteger pos = giveInteger(token2);

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		token1.asTokenList->insert(token1.asTokenList->begin()+pos,token3);

		if (!isVariable)
			stutskStack.push_back(token1); 
	}

	void _f_array_delete(Context* Context)
	{
		/* arguments: <T_INTEGER pos> <T_ARRAY array1> array_append
		   arguments: <T_INTEGER pos> <T_VARIABLE array1> array_append
		   returnvalue: <T_ARRAY>
		   returnvalue:
		   description: Deletes `pos`-th element from `array1`
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();
		Token token2 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		stutskInteger pos = giveInteger(token2);

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		token1.asTokenList->erase(token1.asTokenList->begin()+pos);

		if (!isVariable)
			stutskStack.push_back(token1); 
	}

	// Standard comparison function for array_sort
	bool __tokenCompare (Token t1, Token t2)
	{
		return (tokenNumericCompare(t1,t2) == CN_LARGER);
	}

	void _f_array_sort(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_sort
		   arguments: <T_VARIABLE array1> array_sort
		   returnvalue: <T_ARRAY>
		   returnvalue:
		   description: Sorts `array1` by numeric value from low to high.
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		std::sort(token1.asTokenList->begin(),token1.asTokenList->end(), __tokenCompare);

		if (!isVariable)
			stutskStack.push_back(token1);        
	}
	
	// Standard comparison class for array_custom_sort
	class TokenCompare {
	private:
		TokenList compare;
		Context* context;
	public:
		TokenCompare(TokenList & codeblock, Context* ctx)
		{ compare = codeblock;
		context = ctx; }
		bool operator() (Token i,Token j) 
		{ 
			stutskStack.push_back(i);
			stutskStack.push_back(j);
			context->run(compare, "<__token_compare>");
			Token token1 = stack_back_safe();
			stutskStack.pop_back();
			return giveBool(token1);
		}
	};

	void _f_array_custom_sort(Context* Context)
	{
		/* arguments: <T_ARRAY array1> <T_CODEBLOCK compare> array_custom_sort
		   arguments: <T_VARIABLE array1> <T_CODEBLOCK compare> array_custom_sort
		   returnvalue: <T_ARRAY>
		   returnvalue:
		   description: Sorts `array1` using a custom comparison function `compare`. Two elements are
		     pushed on stack and then the comparison function is executed. Comparison function returns
			 TRUE if the topmost token goes before the one below in the specific strict weak ordering 
			 it defines, FALSE otherwise.
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: `{ < } array_custom_sort` is equivalent to array_sort
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		Token token2 = stack_back_safe();
		stutskStack.pop_back();

		recurseVariables(token1);  

		if (token1.tokenType != T_CODEBLOCK)
			throw StutskException(ET_ERROR, "Token is not an codeblock");

		bool isVariable = token2.tokenType == T_VARIABLE;

		recurseVariables(token2);   

		if (token2.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		TokenCompare comparator(*token1.asTokenList, Context);
		std::sort(token2.asTokenList->begin(),token2.asTokenList->end(), comparator);

		if (!isVariable)
			stutskStack.push_back(token2);        
	}


	void _f_array_pop(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_pop
		   returnvalue: <token>
		   description: Removes the last element of array and pushes it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		stutskStack.push_back(token1.asTokenList->back()); 
		token1.asTokenList->pop_back();
	}

	void _f_array_push(Context* Context)
	{
		/* arguments: <token> <T_ARRAY array1> array_push
		   arguments: <token> <T_VARIABLE array1> array_push
		   returnvalue: <T_ARRAY> 
		   returnvalue: 
		   description: Inserts the token to the end the array.
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();
		Token token2 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		token1.asTokenList->push_back(token2);
		if (!isVariable)
			stutskStack.push_back(token1);        
	}

	void _f_array_peek(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_peek
		   returnvalue: <token>
		   description: Returns the last element of array but does not remove it.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		stutskStack.push_back(token1.asTokenList->back()); 
	}

	void _f_array_pop_front(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_pop_front
		   returnvalue: <token>
		   description: Removes the first element of array and pushes it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		stutskStack.push_back(token1.asTokenList->front()); 
		token1.asTokenList->erase(token1.asTokenList->begin());
	}

	void _f_array_push_front(Context* Context)
	{
		/* arguments: <token> <T_ARRAY array1> array_push_front
		   arguments: <token> <T_VARIABLE array1> array_push_front
		   returnvalue: <T_ARRAY> 
		   returnvalue: 
		   description: Inserts the token to the beginning the array.
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();
		Token token2 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		token1.asTokenList->insert(token1.asTokenList->begin(), token2);
		if (!isVariable)
			stutskStack.push_back(token1);        
	}


	void _f_array_peek_front(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_peek_front
		   returnvalue: <token>
		   description: Returns the first element of array but does not remove it.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		stutskStack.push_back(token1.asTokenList->front()); 
	}

	struct RandomShuffler : std::unary_function<stutskInteger, stutskInteger> {
      boost::mt11213b &_state;
	  unsigned operator()(stutskInteger i) {
          boost::uniform_int<stutskInteger> rng(0, i - 1);
          return rng(_state);
      }
      RandomShuffler(boost::mt11213b &state) : _state(state) {}
    };

	void _f_array_shuffle(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_shuffle
		   arguments: <T_VARIABLE array1> array_shuffle
		   returnvalue: <T_ARRAY> 
		   returnvalue: 
		   description: Randomly reorders the elements of array.
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		RandomShuffler randShuffler(RandomNumber::getInstance().generator);
		std::random_shuffle(token1.asTokenList->begin(), token1.asTokenList->end(), randShuffler);

		if (!isVariable)
			stutskStack.push_back(token1);   
	}

	void _f_array_find(Context* Context) {
		/* arguments: <token> <T_ARRAY array1> array_find
		   returnvalue: <T_INTEGER>  
		   description: Finds the element in the array and returns its index (-1 if not found).
		   notes: Comparison is done using == operator
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		Token token2 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		for (TokenList::size_type i = 0; i < token1.asTokenList->size(); i++ )
		{
			if (tokenEqual(token2, (*token1.asTokenList)[i]))
			{
				pushInteger(i);
				return;
			}
		}

		pushInteger(-1);
		return;
	} 

	void _f_array_perform(Context* Context)
	{
		/* arguments: <T_ARRAY array1> <T_CODEBLOCK func> array_perform
		   arguments: <T_VARIABLE array1> <T_CODEBLOCK func> array_perform
		   returnvalue: <T_ARRAY> 
		   returnvalue: 
		   description: Applies function `func` on every element of `array`. `func` should take
		     one argument and return one argument (the new element of the array).
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: Do not use `continue`.
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock
		Token token2 = stack_back_safe();
		stutskStack.pop_back(); // Codeblock

		recurseVariables(token1);
		if (token1.tokenType != T_CODEBLOCK)
			throw StutskException(ET_ERROR, "Token is not a codeblock");

		bool isVariable = token2.tokenType == T_VARIABLE;

		recurseVariables(token2); 
		if (token2.tokenType != T_ARRAY) {
			throw StutskException(ET_ERROR, "Token is not an array");
		}   

		for (TokenList::iterator it = token2.asTokenList->begin();
			it != token2.asTokenList->end(); ++it) {
				stutskStack.push_back(*it);
				Context->run(*token1.asTokenList, "array_perform");
				*it = stack_back_safe();
				stutskStack.pop_back();
				if (exitVar != OP_INVALID)
					break;
		}

		if (!isVariable)
			stutskStack.push_back(token2); 
	}

	void _f_array_reverse(Context* Context)
	{
		/* arguments: <T_ARRAY array1> array_reverse
		   arguments: <T_VARIABLE array1> array_reverse
		   returnvalue: <T_ARRAY> 
		   returnvalue: 
		   description: Reverses the order of the elements of array.
		     If `array1` is a variable, it returns nothing, otherwise it returns it on stack.
		   notes: 
		*/
		Token token1 = stack_back_safe();
		stutskStack.pop_back();

		bool isVariable = token1.tokenType == T_VARIABLE;

		recurseVariables(token1);   

		if (token1.tokenType != T_ARRAY)
			throw StutskException(ET_ERROR, "Token is not an array");

		reverse(token1.asTokenList->begin(),token1.asTokenList->end());

		if (!isVariable)
			stutskStack.push_back(token1);   
	}

}