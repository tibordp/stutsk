/*
builtinFunctions.hpp - Stutsk functions built into the interpreter are declared here
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

#ifndef BUILTINFUNCTIONS_HPP
#define BUILTINFUNCTIONS_HPP

#include <fstream>
#include <iostream>

#include <stutskInterpreter.h>
#include <boost/random.hpp>

// Regular Expression:
// -------------------
// extern void _f_(.*)\(Context\* context\)
// funcMap["\1"] = &BuiltIns::_f_\1

namespace BuiltIns {
	// Standard I/0
	extern void _f_commandline(Context* context);
	extern void _f_print(Context* context);
	extern void _f_error(Context* context);
	extern void _f_readline(Context* context);
	extern void _f_read(Context* context);
	extern void _f_readchar(Context* context);
	extern void _f_eof(Context* context);

	// File I/o
	extern void _f_readfile(Context* context);
	extern void _f_writefile(Context* context);
	extern void _f_appendfile(Context* context);
	extern void _f_file_array(Context* context);
	extern void _f_fopen(Context* context);
	extern void _f_fread(Context* context);
	extern void _f_fwrite(Context* context);
	extern void _f_fseek(Context* context);
	extern void _f_feof(Context* context);
	extern void _f_fclose(Context* context);

	// Filesystem utilities
	extern void _f_file_exists(Context* context);
	extern void _f_file_size(Context* context);
	extern void _f_file_delete(Context* context);
	extern void _f_file_copy(Context* context);
	extern void _f_file_move(Context* context);
	extern void _f_mkdir(Context* context);
	extern void _f_rmdir(Context* context);
	extern void _f_pwd(Context* context);
	extern void _f_cwd(Context* context);
	extern void _f_is_directory(Context* context);
	extern void _f_readdir(Context* context);

	// Socket I/o
	extern void _f_socket_open(Context* context);
	extern void _f_socket_listen(Context* context);
	extern void _f_socket_accept(Context* context);
	extern void _f_socket_read(Context* context);
	extern void _f_socket_readline(Context* context);
	extern void _f_socket_write(Context* context);
	extern void _f_socket_eof(Context* context);
	extern void _f_socket_close(Context* context);
	extern void _f_socket_open(Context* context);
	extern void _f_socket_listen(Context* context);
	extern void _f_socket_accept(Context* context);
	extern void _f_socket_read(Context* context);
	extern void _f_socket_readline(Context* context);
	extern void _f_socket_write(Context* context);
	extern void _f_socket_eof(Context* context);
	extern void _f_socket_close(Context* context);

	// Memory functions
	extern void _f_length(Context* Context);
	extern void _f_setlength(Context* Context);
	extern void _f_slice(Context* Context);

	// String functions
	extern void _f_ord(Context* Context);
	extern void _f_chr(Context* Context);
	extern void _f_explode(Context* Context);
	extern void _f_pos(Context* Context);
	extern void _f_trim(Context* Context);
	extern void _f_uppercase(Context* Context);
	extern void _f_lowercase(Context* Context);

	// Regular expressions
	extern void _f_regex_validate(Context* Context);
	extern void _f_regex_replace(Context* Context);
	extern void _f_regex_match(Context* Context);
	extern void _f_regex_match_all(Context* Context);

	// MySQL
	extern void _f_mysql_test(Context* context);

	// Crypto
	extern void _f_crc32(Context* context);
	extern void _f_md5(Context* context);
	extern void _f_sha1(Context* context);	
	extern void _f_sha224(Context* context);
	extern void _f_sha256(Context* context);	
	extern void _f_sha384(Context* context);
	extern void _f_sha512(Context* context);	
	extern void _f_base64_encode(Context* context);
	extern void _f_base64_decode(Context* context);

	// Dictionary functions
	extern void _f_dictionary_new(Context* Context);
	extern void _f_dictionary_get(Context* Context);
	extern void _f_dictionary_set(Context* Context);

	// Array manipulation 
	extern void _f_array_append(Context* Context);
	extern void _f_array_insert(Context* Context);
	extern void _f_array_delete(Context* Context);
	extern void _f_array_reverse(Context* Context);
	extern void _f_array_shuffle(Context* Context);
	extern void _f_array_find(Context* Context);
	extern void _f_array_perform(Context* Context);
	extern void _f_array_sort(Context* Context);
	extern void _f_array_custom_sort(Context* Context);

	// Array manipulation (stacks, queues)
	extern void _f_array_pop(Context* Context);
	extern void _f_array_push(Context* Context);
	extern void _f_array_peek(Context* Context);
	extern void _f_array_pop_front(Context* Context);
	extern void _f_array_push_front(Context* Context);
	extern void _f_array_peek_front(Context* Context);	

	// Stack manipulation
	extern void _f_swp(Context* context);
	extern void _f_dmp(Context* context);
	extern void _f_dup(Context* context);
	extern void _f_roll(Context* context);
	extern void _f_swap(Context* context);
	extern void _f_stack_count(Context* context);
	extern void _f_stack_purge(Context* context);
	extern void _f_stack_empty(Context* context);

	// Program control
	extern void _f_do(Context* Context);
	extern void _f_inherit(Context* Context);
	extern void _f_lambda(Context* Context);
	extern void _f_recurse(Context* Context);
	extern void _f_include(Context* context);
	extern void _f_eval(Context* Context);
	extern void _f_uneval(Context* Context);	
	extern void _f_fork(Context* Context);
	extern void _f_sleep(Context* context);
	extern void _f_system(Context* context);
	extern void _f_exec(Context* context);
	extern void _f_exec_readline(Context* context);
	extern void _f_exec_print(Context* context);
	extern void _f_exec_read(Context* context);
	extern void _f_exec_readchar(Context* context);
	extern void _f_exec_eof(Context* context);
	extern void _f_is_def(Context* context);
	extern void _f_definition(Context* context);
	extern void _f_time(Context* context);
	extern void _f_env_list(Context* context);
	extern void _f_env_get(Context* context);

	// Date/Time
	extern void _f_now(Context* context);
	extern void _f_utc_difference(Context* context);
	extern void _f_time_array(Context* context);
	extern void _f_make_time(Context* context);
	extern void _f_day_of_week(Context* context);
	extern void _f_day_of(Context* context);
	extern void _f_month_of(Context* context);
	extern void _f_year_of(Context* context);
	extern void _f_second_of(Context* context);
	extern void _f_minute_of(Context* context);
	extern void _f_hour_of(Context* context);
	extern void _f_format_time(Context* context);
	extern void _f_parse_time(Context* context);

	// Typing
	extern void _f_is_null(Context* context);
	extern void _f_is_array(Context* context);
	extern void _f_is_codeblock(Context* context);
	extern void _f_is_string(Context* context);	
	extern void _f_is_integer(Context* context);
	extern void _f_is_float(Context* context);
	extern void _f_is_handle(Context* context);
	extern void _f_is_bool(Context* context);
	extern void _f_is_variable(Context* context);
	extern void _f_is_numeric(Context* context);

	// Debugging
	extern void _f___dumpglobalvariables(Context* context);
	extern void _f___dumpvariables(Context* context);
	extern void _f___dumpstack(Context* context);
	extern void _f___debug(Context* context);
	extern void _f___type(Context* context);	

	// Mathematic functions
	extern void _f_round(Context* context);
	extern void _f_floor(Context* context);
	extern void _f_ceil(Context* context);
	extern void _f_sqrt(Context* context);
	extern void _f_sin(Context* context);
	extern void _f_cos(Context* context);
	extern void _f_tan(Context* context);
	extern void _f_sinh(Context* context);
	extern void _f_cosh(Context* context);
	extern void _f_tanh(Context* context);
	extern void _f_asin(Context* context);
	extern void _f_acos(Context* context);
	extern void _f_atan(Context* context);
	extern void _f_asinh(Context* context);
	extern void _f_acosh(Context* context);
	extern void _f_atanh(Context* context);
	extern void _f_log(Context* context);
	extern void _f_log10(Context* context);
	extern void _f_abs(Context* context);
	extern void _f_max(Context* context);
	extern void _f_min(Context* context);
	extern void _f_pi(Context* context);
	extern void _f_euler(Context* context);
	extern void _f_from_base(Context* context);
	extern void _f_to_base(Context* context);
	extern void _f_random(Context* context);
	extern void _f_random_float(Context* context);
};

class RandomNumber
{
    public:
		boost::random::mt11213b generator;
        static RandomNumber& getInstance()
        {
            static RandomNumber instance;
            return instance;
        }
    private:
		RandomNumber() {generator.seed(time(NULL));  }
        RandomNumber(RandomNumber const&);   // Don't Implement.
        void operator=(RandomNumber const&); // Don't implement
 };

#endif
