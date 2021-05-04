/*
 *  cmd.c
 *  cli
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#include "cli.h"
#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>


#if CLI_OS_WINDOWS == 1
    #define CLI_SEPERATOR_CHAR '\\'
#else
    #define CLI_SEPERATOR_CHAR '/'
#endif // CLI_OS_WINDOWS == 1

#define CLI_HELP_MIN_COLS 16

#define CLI_MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#define CLI_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define CLI_ERROR_KEY_CATEGORY(KEY) ((KEY) & ((1 << 16) - 1))
#define CLI_ERROR_KEY_ARG_TYPE(KEY) ((KEY) >> 16)
#define CLI_ERROR_KEY(CATEGORY, ARG_TYPE) ((CATEGORY) | ((ARG_TYPE) << 16))

typedef enum cli_argument_type
{
    CLI_ARGUMENT_OPTION,
    CLI_ARGUMENT_OPTION_N_OR_MORE,
    CLI_ARGUMENT_POSITIONAL,
    CLI_ARGUMENT_SUBCOMMAND,
    CLI_ARGUMENT_INVALID
} cli_argument_type;

typedef enum cli_token_type
{
    CLI_TOKEN_SHORT_OPTION,
    CLI_TOKEN_LONG_OPTION,
    CLI_TOKEN_POSITIONAL,
    CLI_TOKEN_SUBCOMMAND,
    CLI_TOKEN_DELIMITER,
    CLI_TOKEN_INVALID
} cli_token_type;

typedef struct cli_token
{
    cli_token_type  type;
    const char*     value;
    int32_t         length;
} cli_token;

typedef struct cli_formatter
{
    int32_t                     buffer_capacity;
    int32_t                     length;
    char*                       buffer;
} cli_formatter;

typedef struct cli_error
{
    uint32_t    key;
    int32_t     int32;
    char        char8;
    const char* str;
} cli_error;

typedef struct cli_error_list
{
    int32_t     count;
    cli_error   data[CLI_ARG_MAX];
} cli_error_list;

typedef struct cli_parser
{
    const cli_command**     command_infos;
    cli_command_result*     command_result;
    cli_result*             program_result;
    cli_error_list*         errors;
} cli_parser;


/*
 **************************
 *
 * Utils API
 *
 **************************
 */
int cli_option_display_length(const cli_option* opt)
{
    int size = 2; // 2 extra for '--'
    if (opt->long_name != NULL)
    {
        size += (int)strlen(opt->long_name);
    }
    if (opt->short_name != '\0')
    {
        size += 4; // 3 extra for the leading '-' and trailing ', '
    }
    return size;
}

cli_parsed_args* add_option(cli_parser* parser)
{
    const int index = parser->program_result->option_count;
    assert(index < CLI_ARG_MAX - 1);

    ++parser->program_result->option_count;
    ++parser->command_result->options.count;

    if (parser->command_result->options.data == NULL)
    {
        parser->command_result->options.data = &parser->program_result->options[index];
    }

    return &parser->program_result->options[index];
}

void cli_add_error(cli_error_list* errors, const cli_error_category category, const enum cli_argument_type arg_type, const char char8, const char* str, const int32_t int32)
{
    if (errors->count >= CLI_ARRAY_SIZE(errors->data))
    {
        fprintf(stderr, "too many errors were generated\n");
        return;
    }

    cli_error* stored = &errors->data[errors->count++];
    stored->key = CLI_ERROR_KEY(category, arg_type);
    stored->char8 = char8;
    stored->str = str;
    stored->int32 = int32;
}

void cli_remove_error(cli_error_list* errors, const cli_error_category category, const enum cli_argument_type arg_type, const char char8, const char* str)
{
    const uint32_t key = CLI_ERROR_KEY(category, arg_type);
    for (int i = 0; i < errors->count; ++i)
    {
        const cli_error* error = &errors->data[i];
        if (error->key != key
            || error->char8 != char8
            || (str == NULL) != (error->str == NULL)
            || strcmp(error->str, str) != 0)
        {
            continue;
        }

        // found the error - swap the last with this error and decrement
        if (i < errors->count - 1)
        {
            memcpy(&errors->data[i], &errors->data[errors->count - 1], sizeof(cli_error));
        }

        --errors->count;
        break;
    }
}

/*
 **************************
 *
 * String formatter API
 *
 **************************
 */
int cli_fmt_vsnprintf(struct cli_formatter* formatter, const char* format, va_list args)
{
    char* dst = formatter->buffer + formatter->length;
    const int dst_size = formatter->buffer_capacity - formatter->length;

    int count = vsnprintf(dst, dst_size, format, args);
    if (count > 0)
    {
        formatter->length += count;
    }

    return count;
}

int cli_fmt(struct cli_formatter* formatter, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int count = cli_fmt_vsnprintf(formatter, format, args);
    va_end(args);
    return count;
}

int cli_fmt_putc(struct cli_formatter* formatter, const char c)
{
    if (formatter->length >= formatter->buffer_capacity)
    {
        return 0;
    }

    formatter->buffer[formatter->length++] = c;
    if (formatter->length < formatter->buffer_capacity)
    {
        formatter->buffer[formatter->length] = '\0';
    }
    return 1;
}

int cli_fmt_puts(struct cli_formatter* formatter, const char* string)
{
    if (string == NULL)
    {
        return 0;
    }

    const char* ptr = string;
    while (*ptr != '\0' && formatter->length < formatter->buffer_capacity)
    {
        formatter->buffer[formatter->length] = *ptr;
        ++formatter->length;
        ++ptr;
    }

    if (formatter->length < formatter->buffer_capacity)
    {
        formatter->buffer[formatter->length] = '\0';
    }

    return (int)(ptr - string);
}

int cli_fmt_put_option_name(struct cli_formatter* formatter, const char short_name, const char* long_name)
{
    int count = 0;

    if (short_name != '\0')
    {
        count += cli_fmt_putc(formatter, '-');
        count += cli_fmt_putc(formatter, short_name);
    }
    if (short_name != '\0' && long_name != NULL)
    {
        count += cli_fmt_putc(formatter, '/');
    }
    if (long_name != NULL)
    {
        count += cli_fmt_puts(formatter, "--");
        count += cli_fmt_puts(formatter, long_name);
    }

    return count;
}

int cli_fmt_spaces_base(struct cli_formatter* formatter, const int column_size, const int label_length)
{
    const int spaces = CLI_MIN(column_size - label_length, formatter->buffer_capacity - formatter->length);
    memset(formatter->buffer + formatter->length, ' ', spaces);
    formatter->length += spaces;
    formatter->buffer[formatter->length] = '\0';
    return spaces;
}

int cli_fmt_spaces_arg(struct cli_formatter* formatter, const int column_size, const char* name)
{
    return cli_fmt_spaces_base(formatter, column_size, (int)strlen(name));
}

int cli_fmt_spaces_opt(struct cli_formatter* formatter, const int column_size, const cli_option* option)
{
    return cli_fmt_spaces_base(formatter, column_size, cli_option_display_length(option));
}

static int cli_qsort_error_comp(const void* lhs, const void* rhs)
{
    const cli_error* lhs_error = (const cli_error*)lhs;
    const cli_error* rhs_error = (const cli_error*)rhs;
    return CLI_ERROR_KEY_CATEGORY(lhs_error->key) < CLI_ERROR_KEY_CATEGORY(rhs_error->key);
}

int cli_sort_and_report_errors(cli_formatter* formatter, cli_error_list* errors)
{
    static const char* fmt_token_name[CLI_ARGUMENT_INVALID + 1] = {
        "option",       // CLI_ARGUMENT_OPTION
        "option",       // CLI_ARGUMENT_OPTION_N_OR_MORE
        "positional",   // CLI_ARGUMENT_POSITIONAL
        "subcommand",   // CLI_ARGUMENT_SUBCOMMAND
        "<#INVALID>",   // CLI_ARGUMENT_INVALID
    };

    // sort into error categories
    qsort(errors, errors->count, sizeof(const cli_error*), cli_qsort_error_comp);

    const int old_formatter_length = formatter->length;
    int category_progress[CLI_ERROR_CATEGORY_COUNT][2] = { { 0, 0 } };
    for (int i = 0; i < errors->count; ++i)
    {
        ++category_progress[CLI_ERROR_KEY_CATEGORY(errors->data[i].key)][1];
    }

    for (int i = 0; i < errors->count; ++i)
    {
        const cli_error* error = &errors->data[i];
        const cli_error_category category = CLI_ERROR_KEY_CATEGORY(error->key);
        const cli_argument_type arg_type = CLI_ERROR_KEY_ARG_TYPE(error->key);
        const int index_in_category = category_progress[category][0];
        const int last_index_in_category = category_progress[category][1] - 1;

        category_progress[category][0]++;

        switch (category)
        {
            case CLI_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT:
            {
                if (index_in_category == 0)
                {
                    cli_fmt_puts(formatter, "the following arguments are required: ");
                }

                if (arg_type == CLI_TOKEN_SHORT_OPTION || arg_type == CLI_TOKEN_LONG_OPTION)
                {
                    cli_fmt_put_option_name(formatter, error->char8, error->str);
                }
                else
                {
                    cli_fmt_puts(formatter, error->str);
                }

                cli_fmt_puts(formatter, index_in_category < last_index_in_category ? ", " : "\n");
                break;
            }
            case CLI_ERROR_CATEGORY_INVALID_NARGS:
            {
                cli_fmt_puts(formatter, "option ");
                cli_fmt_put_option_name(formatter, error->char8, error->str);

                if (arg_type == CLI_ARGUMENT_OPTION_N_OR_MORE)
                {
                    cli_fmt(formatter, " expected at least %d argument", error->int32);
                }
                else
                {
                    cli_fmt(formatter, " expected %d argument", error->int32);
                }

                if (error->int32 > 1)
                {
                    cli_fmt_putc(formatter, 's'); // plural nargs
                }

                cli_fmt_putc(formatter, '\n');
                break;
            }
            case CLI_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT:
            {
                cli_fmt(formatter, "unrecognized %s: %s\n", fmt_token_name[arg_type], error->str);
                break;
            }
            case CLI_ERROR_CATEGORY_INTERNAL:
            {
                cli_fmt(formatter, "internal error - %s\n", error->str);
                break;
            }
            default:
            {
                fprintf(stderr, "invalid error type: %d\n", arg_type);
                exit(1);
            }
        }
    }

    return formatter->length - old_formatter_length;
}

/*
 *****************************
 *
 * Generate a usage message
 * from the command spec
 *
 *****************************
 */
void cli_generate_usage(cli_formatter* usage_formatter, const int32_t command_count, const cli_command* const* commands)
{
    cli_fmt(usage_formatter, "usage: ");

    const cli_command* executed_command = commands[command_count - 1];
    int help_spacing = 0;

    for (int i = 0; i < command_count; ++i)
    {
        const cli_command* command = commands[i];

        if (command->name != NULL)
        {
            cli_fmt(usage_formatter, "%s ", command->name);
        }

        if (command->options.count > 0)
        {
            // Print out all the required options
            for (int opt_idx = 0; opt_idx < command->options.count; ++opt_idx)
            {
                if (command->options.data[opt_idx].required)
                {
                    const char* long_name = command->options.data[opt_idx].long_name;
                    const int nargs = command->options.data[opt_idx].nargs;
                    cli_fmt(usage_formatter, "--%s %s", long_name, nargs != 0 ? "ARGS " : "");
                }

                if (command == executed_command)
                {
                    // calculate max spacing for help strings
                    help_spacing = CLI_MAX(help_spacing, cli_option_display_length(&command->options.data[opt_idx]));
                }
            }

            cli_fmt(usage_formatter, "[options...] ");
        }

        if (command->positionals.count > 0)
        {
            for (int pos_idx = 0; pos_idx < command->positionals.count; ++pos_idx)
            {
                // print out positionals with specific amount of spacing, i.e
                // `program positional1 positional2 ...`
                cli_fmt(usage_formatter, "%s ", command->positionals.data[pos_idx].name);

                if (command == executed_command)
                {
                    // calculate max spacing for help strings
                    help_spacing = CLI_MAX(help_spacing, (int)strlen(command->positionals.data[pos_idx].name));
                }
            }
        }

        if (command == executed_command)
        {
            if (command->subcommands.count > 0)
            {
                cli_fmt(usage_formatter, "<command> ");
                for (int sc = 0; sc < command->subcommands.count; ++sc)
                {
                    // calculate max spacing for help strings
                    help_spacing = CLI_MAX(help_spacing, (int)strlen(command->subcommands.data[sc].name));
                }
            }
        }
    }

    if (executed_command->help != NULL)
    {
        cli_fmt(usage_formatter, "\n\n%s\n", executed_command->help);
    }

    help_spacing = CLI_MAX(CLI_HELP_MIN_COLS, help_spacing + 4); // at least 4 spaces between the arg and the desc

    // output the positional args
    if (executed_command->positionals.count > 0)
    {
        cli_fmt(usage_formatter, "\n\nArguments:\n");

        // print out positionals with specific amount of spacing, i.e `positional1  help string`
        for (int pos_idx = 0; pos_idx < executed_command->positionals.count; ++pos_idx)
        {
            cli_fmt(usage_formatter, "  %s", executed_command->positionals.data[pos_idx].name);
            cli_fmt_spaces_arg(usage_formatter, help_spacing, executed_command->positionals.data[pos_idx].name);
            cli_fmt(usage_formatter, "%s\n", executed_command->positionals.data[pos_idx].help);
        }
    }

    cli_fmt(usage_formatter, "\nOptions:\n  -h, --help");
    cli_fmt_spaces_arg(usage_formatter, help_spacing, "-h, --help");
    cli_fmt(usage_formatter, "Returns this help message\n");

    if (executed_command->options.count > 0)
    {
        // print out options, i.e `-o, --option1  help string`
        for (int opt_idx = 0; opt_idx < executed_command->options.count; ++opt_idx)
        {
            cli_fmt(usage_formatter, "  ");

            // Write out short name
            if (executed_command->options.data[opt_idx].short_name != '\0')
            {
                cli_fmt(usage_formatter, "-%c, ", executed_command->options.data[opt_idx].short_name);
            }

            // Write long name
            cli_fmt(usage_formatter, "--%s", executed_command->options.data[opt_idx].long_name);
            cli_fmt_spaces_opt(usage_formatter, help_spacing, &executed_command->options.data[opt_idx]);
            // Write help string
            cli_fmt(usage_formatter, "%s\n", executed_command->options.data[opt_idx].help);
        }
    }

    // generate just a list of names for subcommands
    if (executed_command->subcommands.count > 0)
    {
        cli_fmt(usage_formatter, "\nCommands:\n  ");

        for (int cmd_idx = 0; cmd_idx < executed_command->subcommands.count; ++cmd_idx)
        {
            cli_fmt(usage_formatter, "%s", executed_command->subcommands.data[cmd_idx].name);

            if (cmd_idx < executed_command->subcommands.count - 1)
            {
                cli_fmt(usage_formatter, ", ");
            }
        }
    }
}


/*
 *****************************
 *
 * Subcommand/root command
 * parsing API
 *
 *****************************
 */
cli_token cli_parse_element(const cli_parser* parser, const char* arg)
{
    if (arg == NULL)
    {
        return (cli_token) { .type = CLI_TOKEN_INVALID };
    }

    int leading_dashes = 0;
    int length = 0;
    for (; arg[length] != '\0'; ++length)
    {
        if (arg[length] == '-' && length == leading_dashes)
        {
            ++leading_dashes;
        }
    }

    if (length == 0)
    {
        return (cli_token) { .type = CLI_TOKEN_INVALID };
    }

    if (leading_dashes == 1)
    {
        return (cli_token) { .type = CLI_TOKEN_SHORT_OPTION, .value = arg + 1, .length = 1 };
    }

    if (leading_dashes == 2)
    {
        // A solitary '--' argument indicates the command line should stop parsing
        const cli_token_type type = length == 2 ? CLI_TOKEN_DELIMITER : CLI_TOKEN_LONG_OPTION;
        return (cli_token) { .type = type, .value = arg + 2, .length = length - 2 };
    }

    const cli_command_result* command_result = parser->command_result;
    const cli_command* command_info = parser->command_infos[parser->program_result->commands_count - 1];
    const cli_command_result* subcommand = &parser->program_result->commands[parser->program_result->commands_count - 1];
    const bool all_positionals_parsed = command_result->positionals.count >= command_info->positionals.count;
    const bool has_parsed_subcommands = subcommand > parser->command_result;

    return (cli_token) {
        .type = (all_positionals_parsed && !has_parsed_subcommands)  ? CLI_TOKEN_SUBCOMMAND : CLI_TOKEN_POSITIONAL,
        .value = arg,
        .length = length
    };
}

bool cli_compare_option(const cli_token* element, const char expected_short_name, const char* expected_long_name)
{
    if (element->type != CLI_TOKEN_SHORT_OPTION && element->type != CLI_TOKEN_LONG_OPTION)
    {
        return false;
    }

    if (element->length == 1 && element->value[0] == expected_short_name)
    {
        return true;
    }

    return strncmp(expected_long_name, element->value, element->length) == 0;
}

int cli_find_option(const cli_command* command, const cli_token* element)
{
    for (int i = 0; i < command->options.count; ++i)
    {
        if (cli_compare_option(element, command->options.data[i].short_name, command->options.data[i].long_name))
        {
            return i;
        }
    }

    return -1;
}

cli_status cli_parse_command(const int argc, char* const* argv, cli_parser* parser)
{
    assert(parser->program_result->commands_count < CLI_ARG_MAX);

    // get command info
    const cli_command* command_info = parser->command_infos[parser->program_result->commands_count - 1];
    cli_command_result* command_result = parser->command_result;

    // setup parser
    cli_error_list* errors = parser->errors;
    for (int i = 0; i < command_info->positionals.count && errors->count < CLI_ARRAY_SIZE(errors->data); ++i)
    {
        cli_add_error(errors, CLI_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CLI_ARGUMENT_POSITIONAL,
            '\0', command_info->positionals.data[i].name, 1
        );
    }
    for (int i = 0; i < command_info->options.count && errors->count < CLI_ARRAY_SIZE(errors->data); ++i)
    {
        if (!command_info->options.data[i].required)
        {
            continue;
        }

        cli_add_error(errors, CLI_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CLI_ARGUMENT_OPTION,
            command_info->options.data[i].short_name,
            command_info->options.data[i].long_name,
            command_info->options.data[i].nargs
        );
    }

    // setup command defaults
    command_result->options.count = command_result->positionals.count= 0;
    command_result->options.data = command_result->options.data = NULL;
    command_result->run = command_info->run;
    if (command_info->name != NULL)
    {
        command_result->name = command_info->name;
    }

    int nargs_parsed = 0;

    while (nargs_parsed < argc)
    {
        const cli_token token = cli_parse_element(parser, argv[nargs_parsed++]);

        switch (token.type)
        {
            case CLI_TOKEN_INVALID:
            {
                // very dodgy - something went terribly wrong
                cli_add_error(errors, CLI_ERROR_CATEGORY_INTERNAL, CLI_ARGUMENT_INVALID,
                    '\0', "invalid argument string detected", 0
                );
                return CLI_STATUS_ERROR;
            }
            case CLI_TOKEN_DELIMITER:
            {
                // detected ' -- ' : game over, man
                return CLI_STATUS_SUCCESS;
            }
            case CLI_TOKEN_SHORT_OPTION:
            case CLI_TOKEN_LONG_OPTION:
            {
                // exit early and show help if this is the implicit -h/--help option
                if (cli_compare_option(&token, 'h', "help"))
                {
                    return CLI_STATUS_HELP;
                }

                // find the given option and validate if exists
                const int option_index = cli_find_option(command_info, &token);

                if (option_index < 0)
                {
                    cli_add_error(errors, CLI_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT, CLI_ARGUMENT_OPTION,
                        '\0', token.value, 0
                    );
                    return CLI_STATUS_ERROR;
                }

                // parse all the arguments for the option
                const cli_option* option_info = &command_info->options.data[option_index];
                const int preparsed_nargs = nargs_parsed;

                // Remove from list of missing required options
                if (option_info->required)
                {
                    cli_remove_error(errors, CLI_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CLI_ARGUMENT_OPTION, option_info->short_name, option_info->long_name);
                }

                // normal, restricted narg range [0, nargs]
                if (option_info->nargs >= 0)
                {
                    // fail if we will exceed argc without being able to parse all the required option arguments
                    if (CLI_MIN(argc, option_info->nargs) < option_info->nargs)
                    {
                        cli_add_error(errors, CLI_ERROR_CATEGORY_INVALID_NARGS, CLI_ARGUMENT_OPTION,
                            option_info->short_name,
                            option_info->long_name,
                            option_info->nargs
                        );
                        return CLI_STATUS_ERROR;
                    }

                    // otherwise we can safely skip past all options args
                    nargs_parsed += option_info->nargs;
                }
                else
                {
                    // special narg range [1, argc]
                    for (int i = nargs_parsed; i < argc; ++i)
                    {
                        // we can't just verify argc we have to actually search through for the next '-/--'
                        if (*(argv[nargs_parsed]) == '-')
                        {
                            break;
                        }
                        ++nargs_parsed;
                    }

                    const int min_nargs_required = option_info->nargs - CLI_0_OR_MORE; // repeated nargs is some distance from CLI_0_OR_MORE
                    if (nargs_parsed - preparsed_nargs < min_nargs_required)
                    {
                        cli_add_error(errors, CLI_ERROR_CATEGORY_INVALID_NARGS, CLI_ARGUMENT_OPTION_N_OR_MORE,
                            option_info->short_name,
                            option_info->long_name,
                            min_nargs_required
                        );
                        return CLI_STATUS_ERROR;
                    }
                }

                // option parse success - add a new parsed one
                cli_parsed_args* option_result = add_option(parser);
                option_result->long_name = option_info->long_name;
                option_result->short_name = option_info->short_name;
                option_result->nargs = nargs_parsed - preparsed_nargs;
                option_result->args = option_info->nargs != 0 ? &(argv[preparsed_nargs]) : NULL;
            }
            case CLI_TOKEN_POSITIONAL:
            {
                if (parser->command_result->positionals.data == NULL)
                {
                    parser->command_result->positionals.data = argv + nargs_parsed;
                }

                // just add this to the positional array
                ++parser->command_result->positionals.count;

                // mark required positional as parsed
                cli_remove_error(errors, CLI_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CLI_ARGUMENT_POSITIONAL, '\0', token.value);
                break;
            }
            case CLI_TOKEN_SUBCOMMAND:
            {
                // if all the positionals have been parsed then this is either a subcommand or otherwise it's invalid
                const cli_command* subcommand_info = NULL;
                for (int i = 0; i < command_info->subcommands.count; ++i)
                {
                    if (strncmp(token.value, command_info->subcommands.data[i].name, token.length) == 0)
                    {
                        subcommand_info = &command_info->subcommands.data[i];
                        break;
                    }
                }

                // invalid - no such command
                if (subcommand_info == NULL)
                {
                    cli_add_error(errors, CLI_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT, CLI_ARGUMENT_SUBCOMMAND,
                        '\0', token.value, 0
                    );
                }

                // ensure all required options were found before moving to a subparser
                if (errors->count > 0)
                {
                    return CLI_STATUS_ERROR;
                }

                // valid subcommand - recursively parse. Since positionals take precedence over subcommands this is fine
                assert(parser->program_result->commands_count < CLI_ARRAY_SIZE(parser->program_result->commands));

                // setup the parser for the next recursive subcommand parse call
                parser->command_infos[parser->program_result->commands_count] = subcommand_info;
                parser->command_result = &parser->program_result->commands[parser->program_result->commands_count];
                ++parser->program_result->commands_count;

                // parse the subcommand
                const int subcommand_argc = argc - nargs_parsed;
                char* const* subcommand_argv = argv + nargs_parsed;
                return cli_parse_command(subcommand_argc, subcommand_argv, parser);
            }
        }
    }

    return errors->count > 0 ? CLI_STATUS_ERROR : CLI_STATUS_SUCCESS;
}

/*
 *****************************
 *
 * Public API implementations
 *
 *****************************
 */
cli_status cli_parse(cli_result* result, const int32_t argc, char* const* argv, const cli_command* cli)
{
    // defaults for the result
    memset(result, 0, sizeof(cli_result));
    result->program_path = "";

    int subcommand_argc = argc;
    char* const* subcommand_argv = argv;

    // Check if empty args -- this should only occur with an adhoc command line, not one generated by the OS
    if (argc > 0)
    {
        assert(argv[0] != NULL);

        // Otherwise it's a normal command line
        result->program_path = argv[0];

        // adjust command args to skip program name
        subcommand_argc = argc - 1;
        subcommand_argv = argv + 1;
    }

    // use the explicitly-specified name if one was provided
    if (result->program_path != NULL)
    {
        // otherwise try and discover the program name from the program path if there are any args
        const int program_path_len = (int)strlen(result->program_path);

        // get the filename from the program path (i.e. program.exe)
        const char* exe_name = result->program_path;
        for (int i = 0; i < program_path_len; ++i)
        {
            if (result->program_path[i] != CLI_SEPERATOR_CHAR || i >= program_path_len - 1)
            {
                continue;
            }

            exe_name = &result->program_path[i + 1];
        }

        // assign the program name and then try and remove the extension if one exists
CLI_PUSH_WARNING
        CLI_DISABLE_WARNING_GCC(-Wstringop-overflow)
        const int full_exe_name = CLI_MIN(strlen(exe_name), CLI_PROGRAM_NAME_MAX);
        strncpy(result->program_name, exe_name, full_exe_name);
CLI_POP_WARNING

        // get the length of the default program name up to the extension and remove it, i.e. program.exe -> program
        // adjust the command line to exclude the program path
        int last_dot = full_exe_name;
        for (int i = 0; i < full_exe_name; ++i)
        {
            if (result->program_name[i] == '.')
            {
                last_dot = i;
            }
        }

        if (last_dot < CLI_PROGRAM_NAME_MAX)
        {
            // zero-out the remaining path
            memset(result->program_name + last_dot, 0, CLI_PROGRAM_NAME_MAX - last_dot);
        }
    }

    // Set default name for the program command, otherwise it will be assigned in the parse_command call if cli defines one
    cli_command_result* program_command = &result->commands[0];
    program_command->name = result->program_name;
    result->program_command = program_command;

    const cli_command* parsed_commands[CLI_ARG_MAX] = { cli, NULL };
    cli_error_list errors = { 0 };
    const cli_status status = cli_parse_command(subcommand_argc, subcommand_argv, &(cli_parser) {
        .command_result = &result->commands[result->commands_count++],
        .command_infos = parsed_commands,
        .program_result = result,
        .errors = &errors
    });

    switch (status)
    {
        case CLI_STATUS_ERROR:
        {
            if (cli->error_buffer.data == NULL)
            {
                char error_buffer[4096];
                cli_formatter formatter = { .buffer_capacity = CLI_ARRAY_SIZE(error_buffer), .buffer = error_buffer };
                cli_sort_and_report_errors(&formatter, &errors);
                fprintf(stderr, "%s: error: %s\n", result->program_command->name, error_buffer);
            }
            else
            {
                cli_formatter formatter = { .buffer_capacity = cli->error_buffer.count, .buffer = cli->error_buffer.data };
                cli_sort_and_report_errors(&formatter, &errors);
            }
            break;
        }
        case CLI_STATUS_HELP:
        {
            if (cli->usage_buffer.data != NULL)
            {
                cli_formatter formatter = { .buffer_capacity = cli->usage_buffer.count, .buffer = cli->usage_buffer.data };
                cli_generate_usage(&formatter, result->commands_count, parsed_commands);
            }
            else
            {
                char usage_buffer[4096];
                cli_formatter formatter = { .buffer_capacity = CLI_ARRAY_SIZE(usage_buffer), .buffer = usage_buffer };
                cli_generate_usage(&formatter, result->commands_count, parsed_commands);
                printf("%s\n", usage_buffer);
            }
            break;
        }
        case CLI_STATUS_SUCCESS:
        {
            // generate the default program help even if help wasn't requested if a usage buffer is assigned
            if (cli->usage_buffer.data != NULL)
            {
                cli_formatter formatter = { .buffer_capacity = cli->usage_buffer.count, .buffer = cli->usage_buffer.data };
                cli_generate_usage(&formatter, result->commands_count, parsed_commands);
            }
            break;
        }
        default:
        {
            fprintf(stderr, "%s: internal error - invalid status %d\n", program_command->name, status);
            exit(1);
        }
    }

    return status;
}

cli_status cli_run(const cli_result* program)
{
    const cli_command_result* cmd = &program->commands[program->commands_count - 1];

    if (cmd->run == NULL)
    {
        return CLI_STATUS_SUCCESS;
    }

    return cmd->run(program->program_command, cmd);
}

int cli_exit_code(const cli_status status)
{
    static int exit_code_table[CLI_STATUS_COUNT] = {
        0, // CLI_STATUS_SUCCESS
        1, // CLI_STATUS_ERROR
        0, // CLI_STATUS_HELP
    };
    return exit_code_table[status];
}

bool cli_has_option(const cli_command_result* command, const char* long_or_short_name)
{
    return cli_get_option(command, long_or_short_name) != NULL;
}

const cli_parsed_args* cli_get_option(const cli_command_result* command, const char* long_or_short_name)
{
    const int size = (int)strlen(long_or_short_name);

    if (size == 1)
    {
        // compare as short flag, i.e. -h
        for (int i = 0; i < command->options.count; ++i)
        {
            if (command->options.data[i].short_name == *long_or_short_name)
            {
                return &command->options.data[i];
            }
        }
    }
    else
    {
        // compare as long name, i.e. --help
        for (int i = 0; i < command->options.count; ++i)
        {
            if (strcmp(command->options.data[i].long_name, long_or_short_name) == 0)
            {
                return &command->options.data[i];
            }
        }
    }

    return NULL;
}

bool cli_has_positional(const cli_command_result* command, const int32_t position)
{
    return cli_get_positional(command, position) != NULL;
}

const char* cli_get_positional(const cli_command_result* command, const int32_t position)
{
    return position < command->positionals.count ? command->positionals.data[position] : NULL;
}