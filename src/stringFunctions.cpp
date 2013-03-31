/*
stringFunctions.cpp - String related Stutsk functions built into the interpreter are implemented here
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
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <cryptopp/md5.h> 
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/crc.h>
#include <cryptopp/base64.h>


void BuiltIns::_f_ord(Context* Context)
{   
	/* arguments: <T_STRING char> ord
	   returnvalue: <T_INTEGER>
	   description: Returns the integer representation of the character (0-255).
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	string chr = *giveString(token1);
	stutskInteger number = static_cast<stutskInteger>(chr[0]);
	pushInteger(number);
}

void BuiltIns::_f_chr(Context* Context)
{
	/* arguments: <T_INTEGER value> chr
	   returnvalue: <T_STRING>
	   description: Returns the character representation of the integer (\0 - \255).
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back(); 
	*pushString() = static_cast<char>(giveInteger(token1));
}

void BuiltIns::_f_pos(Context* context) {
	/* arguments: <T_STRING needle> <T_STRING haystack> pos
	   returnvalue: <T_INTEGER>
	   description: Returns the location of substring `needle` in `haystack` or -1 if it
	     is not found.
	   notes: 
	*/
	Token tString = stack_back_safe();
	stutskStack.pop_back();
	Token tSubString = stack_back_safe();
	stutskStack.pop_back();

	StringPtr haystack = giveString(tString);
	StringPtr needle = giveString(tSubString);

	size_t pos = haystack->find(*needle);

	if (pos == string::npos) pushInteger(-1);
	else pushInteger(pos);
}

void BuiltIns::_f_trim(Context* context) {
	/* arguments: <T_STRING str> trim
	   returnvalue: <T_STRING>
	   description: Trims the string of leading and trailing spaces.
	   notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();
	StringPtr str = giveString(token);
	stutskInteger start=0, end=str->length()-1, i;
	for (i=0; i<(signed)str->length(); i++)
		if ((unsigned int)(*str)[i] <= 32)
			start++;
		else 
			break;
	for (i = (signed)str->length()-1; i>=0; i--)
		if ((unsigned int)(*str)[i] <= 32)
			end--;
		else 
			break;
	*pushString() = str->substr(start, end-start+1);
}

void BuiltIns::_f_uppercase(Context* Context)
{
	/* arguments: <T_STRING str> uppercase
	   returnvalue: <T_STRING>
	   description: Converts the string to uppercase.
	   notes: ASCII only.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string str = *giveString(token1);
	boost::to_upper(str);
	*pushString() = str;
}

void BuiltIns::_f_lowercase(Context* Context)
{
	/* arguments: <T_STRING str> lowercase
	   returnvalue: <T_STRING>
	   description: Converts the string to lowercase.
	   notes: ASCII only.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string str = *giveString(token1);
	boost::to_lower(str);
	*pushString() = str;
}

void BuiltIns::_f_regex_validate(Context* Context)
{
	/* arguments: <T_STRING str> <T_STRING regex> regex_validate
	   returnvalue: <T_BOOL>
	   description: Returns true if `str` matches the regular expression `regex`.
	   notes: See boost:regex for syntax and caveats.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	string regexp = *giveString(token1);
	StringPtr hay =  giveString(token2);

	boost::regex e(regexp);
	pushBool(regex_match(*hay, e));
}

void BuiltIns::_f_regex_replace(Context* Context)
{
	/* arguments: <T_STRING str> <T_STRING regex> <T_STRING replace> regex_validate
	   returnvalue: <T_STRING>
	   description: Replaces substrings of `str` with `replace`, where it matches the
	     regular expression `regex`. 
	   notes: Replaces all occurences, the whole string does not have to match. See 
	     boost:regex for syntax and caveats.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	Token token3 = stack_back_safe();
	stutskStack.pop_back();

	string regex_orig = *giveString(token2);
	string regex_rep = *giveString(token1);
		
	StringPtr hay = giveString(token3);

	boost::regex e(regex_orig);
	*pushString() = boost::regex_replace(*hay, e, regex_rep);
}

void BuiltIns::_f_regex_match(Context* Context)
{
	/* arguments: <T_STRING str> <T_STRING regex> regex_match
	   returnvalue: <T_ARRAY> [ ( <T_STRING> <T_STRING> ... ) ]
	   description: Returns the array of values of capture groups while evaluating the regular 
	     expression `regex` on string `str` or an empty array if the string does not match.
	   notes: See boost:regex for syntax and caveats.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	string regexp = *giveString(token1);
	StringPtr hay =  giveString(token2);

	boost::regex e(regexp);
	boost::smatch what;

	TokenListPtr matches = TokenListPtr(new TokenList());
	
	if (regex_match(*hay, what, e)) // Èe ni validno, potem vrnemo prazno polje
	{		
		Token match(T_STRING);
		for (unsigned i=1;i<what.size();i++)
		{
			match.asString = StringPtr(new string(what[i]));
			matches->push_back(match);
		}
	}

	Token addToken(T_ARRAY);
	addToken.asTokenList = matches;
	stutskStack.push_back(addToken);
}

void BuiltIns::_f_regex_match_all(Context* Context)
{
	/* arguments: <T_STRING str> <T_STRING regex> regex_match_all
	   returnvalue: <T_ARRAY> [ ( <T_STRING> <T_STRING> ... ) ]
	   description: Returns the array of values of capture groups while evaluating the regular 
	     expression `regex` on string `str` or an empty array if the string does not match.
	   notes: See boost:regex for syntax and caveats.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	string regexp = *giveString(token1);
	StringPtr hay =  giveString(token2);

	boost::regex e(regexp);

	TokenListPtr all_matches = TokenListPtr(new TokenList());
	
   std::string::const_iterator start, end; 
   start = hay->begin(); 
   end = hay->end(); 

   boost::match_results<std::string::const_iterator> what;
   boost::match_flag_type flags = boost::match_default; 

   while(regex_search(start, end, what, e, flags)) 
   { 
	    Token matchesToken(T_ARRAY);
		matchesToken.asTokenList = TokenListPtr(new TokenList());
		for (unsigned i=0;i<what.size();i++)
		{
			Token match(T_STRING);
			match.asString = StringPtr(new string(what[i].first,what[i].second));
			matchesToken.asTokenList->push_back(match);
		}

		all_matches->push_back(matchesToken);
		start = what[0].second;
   } 

	Token addToken(T_ARRAY);
	addToken.asTokenList = all_matches;
	stutskStack.push_back(addToken);
}

void BuiltIns::_f_crc32(Context* Context)
{
    /* arguments: <T_STRING str> crc32
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with CRC32 algorithm (hex-encoded).
	   notes: 
	*/
	CryptoPP::CRC32 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_md5(Context* Context)
{
    /* arguments: <T_STRING str> md5
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with MD5 algorithm (hex-encoded).
	   notes: MD5 is considered broken for cryptographic use.
	*/
	CryptoPP::MD5 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_sha1(Context* Context)
{
    /* arguments: <T_STRING str> sha1
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with SHA1 algorithm (hex-encoded).
	   notes: MD5 is considered broken for cryptographic use.
	*/
	CryptoPP::SHA1 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_sha224(Context* Context)
{
    /* arguments: <T_STRING str> sha224
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with SHA-224 algorithm (hex-encoded).
	   notes:
	*/
	CryptoPP::SHA224 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_sha256(Context* Context)
{
    /* arguments: <T_STRING str> sha256
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with SHA-256 algorithm (hex-encoded).
	   notes:
	*/
	CryptoPP::SHA256 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_sha384(Context* Context)
{
    /* arguments: <T_STRING str> sha384
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with SHA-384 algorithm (hex-encoded).
	   notes:
	*/
	CryptoPP::SHA384 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_sha512(Context* Context)
{
    /* arguments: <T_STRING str> sha512
	   returnvalue: <T_STRING>
	   description: Returns the hash of `str` with SHA-512 algorithm (hex-encoded).
	   notes:
	*/
	CryptoPP::SHA512 hash;
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;	

	CryptoPP::StringSource foo(*giveString(token1), true,
      new CryptoPP::HashFilter(hash,
	    new CryptoPP::HexEncoder(
           new CryptoPP::StringSink(result), false)));

	*pushString() = result;
}

void BuiltIns::_f_base64_encode(Context* Context)
{
    /* arguments: <T_STRING str> base64_encode
	   returnvalue: <T_STRING>
	   description: Encodes `str` with base-64 algorithm.
	   notes:
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;

	CryptoPP::StringSource foo(*giveString(token1), true,
		new CryptoPP::Base64Encoder(
         new CryptoPP::StringSink(result)));

	*pushString() = result;
}

void BuiltIns::_f_base64_decode(Context* Context)
{
    /* arguments: <T_STRING str> base64_decode
	   returnvalue: <T_STRING>
	   description: Decodes `str` with base-64 algorithm.
	   notes:
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	string result;

	CryptoPP::StringSource foo(*giveString(token1), true,
		new CryptoPP::Base64Decoder(
         new CryptoPP::StringSink(result)));

	*pushString() = result;
}

void BuiltIns::_f_explode(Context* Context)
{
    /* arguments: <T_STRING str> <T_STRING delimiter> explode
	   returnvalue: <T_ARRAY> [ ( <T_STRING> <T_STRING> ... ) ] 
	   description: Delimits `str` with `delimiter` and returns the array of subbstrings.
	   notes:
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	string text = *giveString(token2);
	string separator = *giveString(token1);
	

	TokenListPtr matches = TokenListPtr(new TokenList());
	
	Token match(T_STRING);

    int found;
    found = text.find_first_of(separator);
    while(found != string::npos){
        if(found >= 0){
			match.asString = StringPtr(new string(text.substr(0,found)));
			matches->push_back(match);
        }
        text = text.substr(found+1);
        found = text.find_first_of(separator);
    }
    
	match.asString = StringPtr(new string(text));
	matches->push_back(match);

	Token addToken(T_ARRAY);
	addToken.asTokenList = matches;
	stutskStack.push_back(addToken);
}