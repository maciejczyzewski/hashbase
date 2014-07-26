/*
 * hashbase - https://github.com/MaciejCzyzewski/hashbase
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Maciej A. Czyzewski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Maciej A. Czyzewski <maciejanthonyczyzewski@gmail.com>
 */

#ifndef _HB_ARGS_H_
#define _HB_ARGS_H_

#include <stddef.h>

/*
	Enum: args_option_type_t
		Types of supported options by system.

		ARGS_OPTION_TYPE_NO_ARG   - The option can have no argument
		ARGS_OPTION_TYPE_REQUIRED - The option requires an argument (--option=arg, -o arg)
		ARGS_OPTION_TYPE_OPTIONAL - The option-argument is optional

		ARGS_OPTION_TYPE_FLAG_SET - The option is a flag and value will be set to flag
		ARGS_OPTION_TYPE_FLAG_AND - The option is a flag and value will be and:ed with flag
		ARGS_OPTION_TYPE_FLAG_OR  - The option is a flag and value will be or:ed with flag
*/
typedef enum args_option_type {
    ARGS_OPTION_TYPE_NO_ARG,
    ARGS_OPTION_TYPE_REQUIRED,
    ARGS_OPTION_TYPE_OPTIONAL,
    ARGS_OPTION_TYPE_FLAG_SET,
    ARGS_OPTION_TYPE_FLAG_AND,
    ARGS_OPTION_TYPE_FLAG_OR
} args_option_type_t;

/**
 * Helper-macro to define end-element in options-array.
 * Mostly helpful on higher warning-level where compiler would complain for { 0 }
 */
#define ARGS_OPTIONS_END { 0, 0, ARGS_OPTION_TYPE_NO_ARG, 0, 0, 0, 0 }

/*
	Struct: args_option
		Option in system.

	Members:
		name       - Long name of argument, set to NULL if only short name is valid.
		name_short - Short name of argument, set to 0 if only long name is valid.
		type       - Type of option, see <args_option_type>.
		flag       - Pointer to flag to set if option is of flag-type, set to null NULL if option is not of flag-type.
		value      - If option is of flag-type, this value will be set/and:ed/or:ed to the flag, else it will be returned from GetOpt when option is found.
		desc       - Description of option.
		value_desc - Short description of valid values to the option, will only be used when generating help-text. example: "--my_option=<value_desc_goes_here>"
*/
typedef struct args_option {
    const char*          name;
    int                  name_short;
    args_option_type_t type;
    int*                 flag;
    int                  value;
    const char*          desc;
    const char*          value_desc;
} args_option_t;

/*
	Struct: args_context_t
		Context used while parsing options.
		Need to be initialized by <args_create_context> before usage. If reused a re-initialization by <args_create_context> is needed.
		Do not modify data in this struct manually!

	Members:
		argc            - Internal variable
		argv            - Internal variable
		opts            - Internal variable
		num_opts        - Internal variable
		current_index   - Internal variable
		current_opt_arg - Used to return values. See <args_next>
*/
typedef struct args_context {
    int                    argc;
    const char**           argv;
    const args_option_t* opts;
    int                    num_opts;
    int                    current_index;
    const char*            current_opt_arg;
} args_context_t;

/*
	Function: args_create_context
		Initializes an args_context_t-struct to be used by <args_next>

	Arguments:
		ctx  - Pointer to context to initialize.
		argc - argc from "int main(int argc, char** argv)" or equal.
		argv - argv from "int main(int argc, char** argv)" or equal. Data need to be valid during option-parsing and usage of data.
		opts - Pointer to array with options that should be looked for. Should end with an option that is all zeroed!

	Returns:
		0 on success, otherwise error-code.
*/
int args_create_context( args_context_t* ctx, int argc, const char** argv, const args_option_t* opts );

/*
	Function: args_next
		Used to parse argc/argv with the help of a args_context_t.
		Tries to parse the next token in ctx and return id depending on status.

	Arguments:
		ctx - Pointer to a initialized <args_context_t>

	Returns:
		- '!' on error. ctx->current_opt_arg will be set to flag-name! Errors that can occur,
		      Argument missing if argument is required or Argument found when there should be none.
		- '?' if item was an unrecognized option, ctx->current_opt_arg will be set to item!
		- '+' if item was no option, ctx->current_opt_arg will be set to item!
		- '0' if the opt was a flag and it was set. ctx->current_opt_arg will be set to flag-name!
		      the value stored is value in the found option.
		- -1 no more options to parse!
*/
int args_next( args_context_t* ctx );

/*
	Function: GetOptCreateHelpString
		Builds a string that describes all options for use with the --help-flag etc.

	Arguments:
		ctx         - Pointer to a initialized <args_context_t>
		buffer      - Pointer to buffer to build string in.
		buffer_size - Size of buffer.

	Returns:
		buffer filled with a help-string.
*/
const char* args_create_help_string( args_context_t* ctx, char* buffer, size_t buffer_size );

#endif