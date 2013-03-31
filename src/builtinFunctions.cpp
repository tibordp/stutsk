/*
builtinFunctions.cpp - Stutsk functions built into the interpreter are registered here
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

BuiltinFunctionsMap BuiltIns::populateFunctions() {
	BuiltinFunctionsMap funcMap;
	// Standard I/0
	//funcMap["commandline"] = &BuiltIns::_f_commandline;
	funcMap["print"] = &BuiltIns::_f_print;
	funcMap["error"] = &BuiltIns::_f_error;
	funcMap["readline"] = &BuiltIns::_f_readline;
	funcMap["read"] = &BuiltIns::_f_read;
	funcMap["readchar"] = &BuiltIns::_f_readchar;
	funcMap["eof"] = &BuiltIns::_f_eof;

	// File I/o
	//funcMap["readfile"] = &BuiltIns::_f_readfile;
	//funcMap["writefile"] = &BuiltIns::_f_writefile;
	//funcMap["appendfile"] = &BuiltIns::_f_appendfile;
	//funcMap["file_array"] = &BuiltIns::_f_file_array;
	//funcMap["fopen"] = &BuiltIns::_f_fopen;
	//funcMap["fread"] = &BuiltIns::_f_fread;
	//funcMap["fwrite"] = &BuiltIns::_f_fwrite;
	//funcMap["fseek"] = &BuiltIns::_f_fseek;
	//funcMap["feof"] = &BuiltIns::_f_feof;
	//funcMap["fclose"] = &BuiltIns::_f_fclose;

	// Filesystem utilities
	//funcMap["file_exists"] = &BuiltIns::_f_file_exists;
	//funcMap["file_size"] = &BuiltIns::_f_file_size;
	//funcMap["file_delete"] = &BuiltIns::_f_file_delete;
	//funcMap["file_copy"] = &BuiltIns::_f_file_copy;
	//funcMap["file_move"] = &BuiltIns::_f_file_move;
	//funcMap["mkdir"] = &BuiltIns::_f_mkdir;
	//funcMap["rmdir"] = &BuiltIns::_f_rmdir;
	//funcMap["pwd"] = &BuiltIns::_f_pwd;
	//funcMap["cwd"] = &BuiltIns::_f_cwd;
	//funcMap["is_directory"] = &BuiltIns::_f_is_directory;
	//funcMap["readdir"] = &BuiltIns::_f_readdir;

	// Socket I/o
	funcMap["socket_open"] = &BuiltIns::_f_socket_open;
	//funcMap["socket_listen"] = &BuiltIns::_f_socket_listen;
	//funcMap["socket_accept"] = &BuiltIns::_f_socket_accept;
	funcMap["socket_read"] = &BuiltIns::_f_socket_read;
	funcMap["socket_readline"] = &BuiltIns::_f_socket_readline;
	funcMap["socket_write"] = &BuiltIns::_f_socket_write;
	funcMap["socket_eof"] = &BuiltIns::_f_socket_eof;
	funcMap["socket_close"] = &BuiltIns::_f_socket_close;
	funcMap["socket_open"] = &BuiltIns::_f_socket_open;
	//funcMap["socket_listen"] = &BuiltIns::_f_socket_listen;
	//funcMap["socket_accept"] = &BuiltIns::_f_socket_accept;
	funcMap["socket_read"] = &BuiltIns::_f_socket_read;
	funcMap["socket_readline"] = &BuiltIns::_f_socket_readline;
	funcMap["socket_write"] = &BuiltIns::_f_socket_write;
	funcMap["socket_eof"] = &BuiltIns::_f_socket_eof;
	funcMap["socket_close"] = &BuiltIns::_f_socket_close;

	// Memory functions
	funcMap["length"] = &BuiltIns::_f_length;
	funcMap["setlength"] = &BuiltIns::_f_setlength;
	funcMap["slice"] = &BuiltIns::_f_slice;

	// String functions
	funcMap["ord"] = &BuiltIns::_f_ord;
	funcMap["chr"] = &BuiltIns::_f_chr;
	funcMap["explode"] = &BuiltIns::_f_explode;
	funcMap["pos"] = &BuiltIns::_f_pos;
	funcMap["trim"] = &BuiltIns::_f_trim;
	funcMap["uppercase"] = &BuiltIns::_f_uppercase;
	funcMap["lowercase"] = &BuiltIns::_f_lowercase;

	// Regular expressions
	funcMap["regex_validate"] = &BuiltIns::_f_regex_validate;
	funcMap["regex_replace"] = &BuiltIns::_f_regex_replace;
	funcMap["regex_match"] = &BuiltIns::_f_regex_match;
	funcMap["regex_match_all"] = &BuiltIns::_f_regex_match_all;

	// MySQL
	//funcMap["mysql_test"] = &BuiltIns::_f_mysql_test;

	// Crypto
	funcMap["crc32"] = &BuiltIns::_f_crc32;
	funcMap["md5"] = &BuiltIns::_f_md5;
	funcMap["sha1"] = &BuiltIns::_f_sha1;	
	funcMap["sha224"] = &BuiltIns::_f_sha224;
	funcMap["sha256"] = &BuiltIns::_f_sha256;	
	funcMap["sha384"] = &BuiltIns::_f_sha384;
	funcMap["sha512"] = &BuiltIns::_f_sha512;	
	funcMap["base64_encode"] = &BuiltIns::_f_base64_encode;
	funcMap["base64_decode"] = &BuiltIns::_f_base64_decode;

	// Dictionary functions
	funcMap["dictionary_new"] = &BuiltIns::_f_dictionary_new;
	funcMap["dictionary_get"] = &BuiltIns::_f_dictionary_get;
	funcMap["dictionary_set"] = &BuiltIns::_f_dictionary_set;

	// Array manipulation 
	funcMap["array_append"] = &BuiltIns::_f_array_append;
	funcMap["array_insert"] = &BuiltIns::_f_array_insert;
	funcMap["array_delete"] = &BuiltIns::_f_array_delete;
	funcMap["array_reverse"] = &BuiltIns::_f_array_reverse;
	funcMap["array_shuffle"] = &BuiltIns::_f_array_shuffle;
	funcMap["array_find"] = &BuiltIns::_f_array_find;
	funcMap["array_perform"] = &BuiltIns::_f_array_perform;
	funcMap["array_sort"] = &BuiltIns::_f_array_sort;
	funcMap["array_custom_sort"] = &BuiltIns::_f_array_custom_sort;

	// Array manipulation (stacks, queues)
	funcMap["array_pop"] = &BuiltIns::_f_array_pop;
	funcMap["array_push"] = &BuiltIns::_f_array_push;
	funcMap["array_peek"] = &BuiltIns::_f_array_peek;
	funcMap["array_pop_front"] = &BuiltIns::_f_array_pop_front;
	funcMap["array_push_front"] = &BuiltIns::_f_array_push_front;
	funcMap["array_peek_front"] = &BuiltIns::_f_array_peek_front;	

	// Stack manipulation
	funcMap["swp"] = &BuiltIns::_f_swp;
	funcMap["dmp"] = &BuiltIns::_f_dmp;
	funcMap["dup"] = &BuiltIns::_f_dup;
	funcMap["roll"] = &BuiltIns::_f_roll;
	funcMap["swap"] = &BuiltIns::_f_swap;
	funcMap["stack_count"] = &BuiltIns::_f_stack_count;
	funcMap["stack_purge"] = &BuiltIns::_f_stack_purge;
	funcMap["stack_empty"] = &BuiltIns::_f_stack_empty;

	// Program control
	funcMap["do"] = &BuiltIns::_f_do;
	funcMap["inherit"] = &BuiltIns::_f_inherit;
	funcMap["lambda"] = &BuiltIns::_f_lambda;
	funcMap["recurse"] = &BuiltIns::_f_recurse;
	funcMap["include"] = &BuiltIns::_f_include;
	funcMap["eval"] = &BuiltIns::_f_eval;
	funcMap["uneval"] = &BuiltIns::_f_uneval;	
	//funcMap["fork"] = &BuiltIns::_f_fork;
	funcMap["sleep"] = &BuiltIns::_f_sleep;
	//funcMap["system"] = &BuiltIns::_f_system;
	//funcMap["exec"] = &BuiltIns::_f_exec;
	//funcMap["exec_readline"] = &BuiltIns::_f_exec_readline;
	//funcMap["exec_read"] = &BuiltIns::_f_exec_read;
	//funcMap["exec_print"] = &BuiltIns::_f_exec_print;
	//funcMap["exec_readchar"] = &BuiltIns::_f_exec_readchar;
	//funcMap["exec_eof"] = &BuiltIns::_f_exec_eof;
	funcMap["is_def"] = &BuiltIns::_f_is_def;
	funcMap["definition"] = &BuiltIns::_f_definition;
	funcMap["time"] = &BuiltIns::_f_time;
	//funcMap["env_list"] = &BuiltIns::_f_env_list;
	//funcMap["env_get"] = &BuiltIns::_f_env_get;

	// Date/Time
	funcMap["now"] = &BuiltIns::_f_now;
	funcMap["utc_difference"] = &BuiltIns::_f_utc_difference;
	funcMap["time_array"] = &BuiltIns::_f_time_array;
	funcMap["make_time"] = &BuiltIns::_f_make_time;
	funcMap["day_of_week"] = &BuiltIns::_f_day_of_week;
	funcMap["day_of"] = &BuiltIns::_f_day_of;
	funcMap["month_of"] = &BuiltIns::_f_month_of;
	funcMap["year_of"] = &BuiltIns::_f_year_of;
	funcMap["second_of"] = &BuiltIns::_f_second_of;
	funcMap["minute_of"] = &BuiltIns::_f_minute_of;
	funcMap["hour_of"] = &BuiltIns::_f_hour_of;
	funcMap["format_time"] = &BuiltIns::_f_format_time;
	funcMap["parse_time"] = &BuiltIns::_f_parse_time;

	// Typing
	funcMap["is_null"] = &BuiltIns::_f_is_null;
	funcMap["is_array"] = &BuiltIns::_f_is_array;
	funcMap["is_codeblock"] = &BuiltIns::_f_is_codeblock;
	funcMap["is_string"] = &BuiltIns::_f_is_string;	
	funcMap["is_integer"] = &BuiltIns::_f_is_integer;
	funcMap["is_float"] = &BuiltIns::_f_is_float;
	funcMap["is_handle"] = &BuiltIns::_f_is_handle;
	funcMap["is_bool"] = &BuiltIns::_f_is_bool;
	funcMap["is_variable"] = &BuiltIns::_f_is_variable;
	funcMap["is_numeric"] = &BuiltIns::_f_is_numeric;

	// Debugging
	funcMap["__dumpglobalvariables"] = &BuiltIns::_f___dumpglobalvariables;
	funcMap["__dumpvariables"] = &BuiltIns::_f___dumpvariables;
	funcMap["__dumpstack"] = &BuiltIns::_f___dumpstack;
	//funcMap["__debug"] = &BuiltIns::_f___debug;
	funcMap["__type"] = &BuiltIns::_f___type;	

	// Mathematic functions
	funcMap["round"] = &BuiltIns::_f_round;
	funcMap["floor"] = &BuiltIns::_f_floor;
	funcMap["ceil"] = &BuiltIns::_f_ceil;
	funcMap["sqrt"] = &BuiltIns::_f_sqrt;
	funcMap["sin"] = &BuiltIns::_f_sin;
	funcMap["cos"] = &BuiltIns::_f_cos;
	funcMap["tan"] = &BuiltIns::_f_tan;
	funcMap["sinh"] = &BuiltIns::_f_sinh;
	funcMap["cosh"] = &BuiltIns::_f_cosh;
	funcMap["tanh"] = &BuiltIns::_f_tanh;
	funcMap["asin"] = &BuiltIns::_f_asin;
	funcMap["acos"] = &BuiltIns::_f_acos;
	funcMap["atan"] = &BuiltIns::_f_atan;
	funcMap["asinh"] = &BuiltIns::_f_asinh;
	funcMap["acosh"] = &BuiltIns::_f_acosh;
	funcMap["atanh"] = &BuiltIns::_f_atanh;
	funcMap["log"] = &BuiltIns::_f_log;
	funcMap["log10"] = &BuiltIns::_f_log10;
	funcMap["abs"] = &BuiltIns::_f_abs;
	funcMap["max"] = &BuiltIns::_f_max;
	funcMap["min"] = &BuiltIns::_f_min;
	funcMap["pi"] = &BuiltIns::_f_pi;
	funcMap["euler"] = &BuiltIns::_f_euler;
	funcMap["from_base"] = &BuiltIns::_f_from_base;
	funcMap["to_base"] = &BuiltIns::_f_to_base;
	funcMap["random"] = &BuiltIns::_f_random;
	funcMap["random_float"] = &BuiltIns::_f_random_float;

	BuiltinFunctionsMap::const_iterator iter;

	for (iter = funcMap.begin(); iter != funcMap.end(); ++iter)
	{
		funcMap["__builtin_" + iter->first] = iter->second;  
	}

	return funcMap;
}
