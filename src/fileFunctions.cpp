/*
fileFunctions.cpp - File and filesystem Stutsk functions built into the interpreter are implemented here
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
#include <boost/filesystem.hpp>

void BuiltIns::_f_readfile(Context* context) {
	/* arguments: <T_STRING filename> readfile
	   returnvalue: <T_STRING>
	   description: Reads the contents of a file `filename` into a string.
	   notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();

	string filename = *giveString(token);

	ifstream readfile(filename.c_str());
	if (!readfile)
		throw StutskException(ET_ERROR, "Cannot open file \"" +
		filename + "\"");
	stringstream filebuf;

	filebuf << readfile.rdbuf();	

	*pushString() = filebuf.str();
}


void BuiltIns::_f_writefile(Context* context) {
	/* arguments: <T_STRING data> <T_STRING filename> writefile
	   returnvalue: 
	   description: Writes `data` into the file `filename` overwriting the existing data.
	   notes: If file does not exist, it is created.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	string filename = *giveString(token1);
	StringPtr contents = giveString(token2);

	ofstream myfile;
	myfile.open (filename, ios::out | ios::binary);
	if (myfile.is_open())
	{
		myfile << *contents;
		myfile.close();
	}
	else
		throw StutskException(ET_ERROR, "Cannot open a file");

}

void BuiltIns::_f_appendfile(Context* context) {
	/* arguments: <T_STRING data> <T_STRING filename> writefile
	   returnvalue: 
	   description: Appends `data` into the file `filename`.
	   notes: If file does not exist, it is created.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	string filename = *giveString(token1);
	StringPtr contents = giveString(token2);

	ofstream myfile;
	myfile.open (filename, ios::out | ios::app | ios::binary);
	if (myfile.is_open())
	{
		myfile << *contents;
		myfile.close();
	}
	else
		throw StutskException(ET_ERROR, "Cannot open a file");

}


void BuiltIns::_f_file_array(Context* context) {
	/* arguments: <T_STRING filename> writefile
	   returnvalue: <T_ARRAY> [ ( <T_STRING> <T_STRING> <T_STRING> ... ) ]
	   description: Returns the lines of a file `filename` in an array.
	   notes: 
	*/
	Token token = stack_back_safe();
	stutskStack.pop_back();

	string filename = *giveString(token);

	ifstream readfile(filename.c_str());
	if (!readfile)
		throw StutskException(ET_ERROR, "Cannot open file \"" +
		filename + "\"");

	TokenListPtr matches = TokenListPtr(new TokenList());
	Token match(T_STRING);

	while (!readfile.eof())
	{
		match.asString = StringPtr(new string);
		getline(readfile, *match.asString);
		matches->push_back(match);
	}

	Token addToken(T_ARRAY);
	addToken.asTokenList = matches;
	stutskStack.push_back(addToken);
}

void BuiltIns::_f_fopen(Context* context) {
	/* arguments: <T_STRING filename> <T_STRING mode> fopen
	   returnvalue: <T_HANDLE>
	   description: Opens a file `filename` with mode `mode` and returns a handle to it.
	   notes: See libc fopen() reference for mode options.
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	string mode = *giveString(token1);
	string filename = *giveString(token2);

	Token fileToken(T_HANDLE);
	fileToken.data.asHandle.size = sizeof(FILE);
	fileToken.data.asHandle.ptr = fopen(filename.c_str(), mode.c_str());
	if (fileToken.data.asHandle.ptr == NULL)
		throw StutskException(ET_ERROR, "File operation failed");
	stutskStack.push_back(fileToken);
}

void BuiltIns::_f_fseek(Context* context) {
	/* arguments: <T_INTEGER pos> <T_HANDLE handle> fseek
	   returnvalue: 
	   description: Jumps to a position `pos` in file referenced by `handle`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token1);
	if (token1.tokenType != T_HANDLE) {
		throw StutskException(ET_ERROR, "Invalid file handle");
	}
	stutskInteger position = giveInteger(token2);

	fseek((FILE*)token1.data.asHandle.ptr, position, SEEK_SET);

	if (ferror((FILE*)token1.data.asHandle.ptr)) {
		throw StutskException(ET_ERROR, "File operation failed");
	}
}

void BuiltIns::_f_fread(Context* context) {
	/* arguments: <T_INTEGER count> <T_HANDLE handle> fread
	   returnvalue: <T_STRING>
	   description: Reads `count` bytes from current position in file referenced by `handle`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token1);
	if (token1.tokenType != T_HANDLE) {
		throw StutskException(ET_ERROR, "Invalid file handle");
	}
	stutskInteger length = giveInteger(token2);
	char *buf = new char[length];

	size_t bytesRead = fread(buf, 1, length, (FILE*)token1.data.asHandle.ptr);

	if (ferror((FILE*)token1.data.asHandle.ptr)) {
		throw StutskException(ET_ERROR, "File operation failed");
	}

	Token newString(T_STRING);
	newString.asString = StringPtr(new string (buf, bytesRead));
	stutskStack.push_back(newString);

	delete buf;
}


void BuiltIns::_f_fwrite(Context* context) {
	/* arguments: <T_STRING data> <T_HANDLE handle> fwrite
	   returnvalue: 
	   description: Writes `data` to a current position in file referenced by `handle`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token1);
	StringPtr data = giveString(token2);

	if (token1.tokenType != T_HANDLE) {
		throw StutskException(ET_ERROR, "Invalid file handle");
	}

	fwrite((void*)data->data(), 1, data->length(), (FILE*)token1.data.asHandle.ptr);
	if (ferror((FILE*)token1.data.asHandle.ptr)) {
		throw StutskException(ET_ERROR, "File operation failed");
	}
}


void BuiltIns::_f_feof(Context* context) {
	/* arguments: <T_HANDLE handle> feof
	   returnvalue: <T_BOOL>
	   description: Returns true if the end of the file referenced by `handle`is reached.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(token1);
	if (token1.tokenType != T_HANDLE) {
		throw StutskException(ET_ERROR, "Invalid file handle");
	}
	bool eof = feof((FILE*)token1.data.asHandle.ptr) != 0;
	if (ferror((FILE*)token1.data.asHandle.ptr)) {
		throw StutskException(ET_ERROR, "File operation failed");
	}
	pushBool(eof);
}

void BuiltIns::_f_fclose(Context* context) {
	/* arguments: <T_HANDLE handle> fclose
	   returnvalue: 
	   description: Closes the file.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	recurseVariables(token1);
	if (token1.tokenType != T_HANDLE) {
		throw StutskException(ET_ERROR, "Invalid file handle");
	}
	fclose((FILE*)token1.data.asHandle.ptr);
}


void BuiltIns::_f_file_exists(Context* Context)
{
	/* arguments: <T_STRING filename> file_exists
	   returnvalue: <T_BOOL>
	   description: Returns true if file `filename` exists.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	pushBool(boost::filesystem::exists(*giveString(token1)));
}

void BuiltIns::_f_file_size(Context* Context)
{
	/* arguments: <T_STRING filename> file_size
	   returnvalue: <T_INTEGER>
	   description: Returns the file size of `filenameÿ in bytes.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	pushInteger(boost::filesystem::file_size(*giveString(token1)));
}


void BuiltIns::_f_file_delete(Context* context)
{
	/* arguments: <T_STRING filename> file_delete
	   returnvalue: 
	   description: Deletes the file `filename`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	boost::system::error_code er;

	boost::filesystem::remove(*giveString(token1), er);

	if (er) 
		throw StutskException(ET_ERROR, "Error while removing file.");
}
void BuiltIns::_f_file_copy(Context* context)
{
	#ifndef BOOST_CPP0X_PROBLEM	
	/* arguments: <T_STRING src_filename> <T_STRING dst_filename> file_copy
	   returnvalue: 
	   description: Copies file `src_filename` to `dst_filename`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	boost::system::error_code er;
	boost::filesystem::copy_file(*giveString(token2),*giveString(token1),er);

	if (er) 
	#endif
		throw StutskException(ET_ERROR, "Error while copying file.");
}

void BuiltIns::_f_file_move(Context* context)
{
	/* arguments: <T_STRING src_filename> <T_STRING dst_filename> file_copy
	   returnvalue: 
	   description: Moves/renames file `src_filename` to `dst_filename`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	boost::system::error_code er;
	boost::filesystem::rename(*giveString(token2),*giveString(token1),er);

	if (er) 
		throw StutskException(ET_ERROR, "Error while moving file.");
}

void BuiltIns::_f_mkdir(Context* context)
{
	/* arguments: <T_STRING dirname> file_copy
	   returnvalue: 
	   description: Creates directory `dirname`
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	boost::system::error_code er;

	boost::filesystem::create_directory(*giveString(token1), er);

	if (er) 
		throw StutskException(ET_ERROR, "Error while creating directory.");
}

void BuiltIns::_f_rmdir(Context* context) 
{
	/* arguments: <T_STRING dirname> file_copy
	   returnvalue: 
	   description: Deletes the empty directory `dirname`
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	boost::system::error_code er;

	boost::filesystem::remove(*giveString(token1), er);

	if (er) 
		throw StutskException(ET_ERROR, "Error while removing directory.");
}

void BuiltIns::_f_pwd(Context* context) 
{
	/* arguments: pwd
	   returnvalue: <T_STRING>
	   description: Returns the path of the working directory.
	   notes: 
	*/
	boost::filesystem3::path path = boost::filesystem::current_path();
	*pushString() = path.string();
}

void BuiltIns::_f_cwd(Context* context) 
{
	/* arguments: <T_STRING dirname> cwd
	   returnvalue: 
	   description: Changes working directory to `dirname`.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	boost::filesystem::current_path(*giveString(token1));
}

void BuiltIns::_f_is_directory(Context* Context)
{
	/* arguments: <T_STRING filname> file_copy
	   returnvalue: <T_BOOL>
	   description: Returns true if `filename` is a directory.
	   notes: 
	*/
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	pushBool(boost::filesystem::is_directory(*giveString(token1)));
}

void BuiltIns::_f_readdir(Context* context) {
	/* arguments: <T_STRING dirname> readdir
	   returnvalue: <T_ARRAY> [ ( <T_STRING> <T_STRING> ... ) ]
	   description: Returns a list of files in a directory `dir_name`.
	   notes: Full paths are returned.
	*/
	using namespace boost::filesystem;	
	Token token1 = stack_back_safe();
	stutskStack.pop_back();

	path current_dir(*giveString(token1));
	TokenListPtr fArray = TokenListPtr(new TokenList()); 
	Token newToken(T_STRING);

	for (directory_iterator iter(current_dir), end;
		iter != end;
		++iter)
	{		
		newToken.asString = StringPtr(new string(iter->path().string()));
		fArray->push_back(newToken);
	}

	Token newArray(T_ARRAY);
	newArray.asTokenList = fArray;
	stutskStack.push_back(newArray);
}
