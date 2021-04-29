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


#if CLI_OS_WINDOWS == 1
    #define CLI_SEPERATOR_CHAR '\\'
#else
    #define CLI_SEPERATOR_CHAR '/'
#endif // CLI_OS_WINDOWS == 1

#define CLI_HELP_MIN_COLS 16

typedef enum cli_parse_status
{
    CLI_PARSE_RESULT_PARSED,
    CLI_PARSE_RESULT_ERROR,
    CLI_PARSE_RESULT_HELP
} cli_parse_status;

typedef enum cli_element_type
{
    CLI_ELEMENT_SHORT_OPTION,
    CLI_ELEMENT_LONG_OPTION,
    CLI_ELEMENT_POSITIONAL,
    CLI_ELEMENT_SUBCOMMAND,
    CLI_ELEMENT_DELIMITER,
    CLI_ELEMENT_INVALID
} cli_element_type;

typedef struct cli_formatter
{
    int32_t buffer_capacity;
    char*   buffer;

    int32_t length;
} cli_formatter;

typedef struct cli_missing_options
{
    int32_t             count;
    const cli_option*   data[CLI_ARG_MAX];
} cli_missing_options;

typedef struct cli_element
{
    cli_element_type    type;
    const char*         value;
    int32_t             length;
} cli_element;

typedef struct cli_parser
{
    const cli_command*  command_info;
    cli_command_result* command_result;
    cli_formatter       error_formatter;
    cli_missing_options missing;
} cli_parser;


/*
 **************************
 *
 * Utils API
 *
 **************************
 */
#define CLI_MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#define CLI_MIN(X, Y) ((X) < (Y) ? (X) : (Y))

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

/*
 **************************
 *
 * String formatter API
 *
 **************************
 */
int cli_fmt(struct cli_formatter* formatter, const char* fmt, ...)
{
    va_list args;
        va_start(args, fmt);

    int count = vsnprintf(formatter->buffer + formatter->length, formatter->buffer_capacity - formatter->length, fmt, args);
    if (count > 0)
    {
        formatter->length += count;
    }

        va_end(args);

    return count;
}

int cli_fmt_spaces_base(struct cli_formatter* formatter, const int min, const int label_length)
{
    const int spaces = min - label_length;

    for (int i = 0; i < spaces; ++i)
    {
        const int format_count = cli_fmt(formatter, " ");
        if (format_count <= 0)
        {
            return CLI_MAX(0, i - 1);
        }
    }

    return spaces;
}

int cli_fmt_spaces_arg(struct cli_formatter* formatter, const int min, const char* name)
{
    return cli_fmt_spaces_base(formatter, min, (int)strlen(name));
}

int cli_fmt_spaces_opt(struct cli_formatter* formatter, const int min, const cli_option* option)
{
    return cli_fmt_spaces_base(formatter, min, cli_option_display_length(option));
}

/*
 **************************
 *
 * Missing options API
 *
 **************************
 */
void cli_missing_options_init(cli_parser* parser)
{
    parser->missing.count = 0;

    for (int i = 0; i < parser->command_info->options.count; ++i)
    {
        if (parser->command_info->options.data[i].required)
        {
            assert(parser->missing.count < CLI_ARG_MAX);
            parser->missing.data[parser->missing.count++] = &parser->command_info->options.data[i];
        }
    }
}

void cli_missing_options_mark_parsed(cli_parser* parser, const cli_option* option)
{
    for (int i = 0; i < parser->missing.count; ++i)
    {
        if (option != parser->missing.data[i])
        {
            continue;
        }

        // swap and pop the last item if i is not last
        if (i == parser->missing.count - 1)
        {
            --parser->missing.count;
            break;
        }
        else
        {
            const cli_option* tmp = parser->missing.data[parser->missing.count];
            parser->missing.data[parser->missing.count] = parser->missing.data[i];
            parser->missing.data[i] = tmp;
        }
    }
}

cli_parse_status cli_report_missing(cli_parser* parser)
{
    cli_fmt(&parser->error_formatter, "missing required options:");
    for (int i = 0; i < parser->missing.count; ++i)
    {
        cli_fmt(&parser->error_formatter, " -%c/--%s", parser->missing.data[i]->short_name, parser->missing.data[i]->long_name);
        if (i < parser->missing.count - 1)
        {
            cli_fmt(&parser->error_formatter, ",");
        }
    }

    return CLI_PARSE_RESULT_ERROR;
}

/*
 *****************************
 *
 * Generate a usage message
 * from the command spec
 *
 *****************************
 */
void cli_generate_help(const cli_parser* parser, const char* default_program_name)
{
    struct cli_formatter usage_formatter = { .buffer_capacity = CLI_HELP_MAX, .buffer = parser->command_result->usage };
    const cli_command* command = parser->command_info;

    cli_fmt(&usage_formatter, "usage: ");

    if (command->name != NULL)
    {
        cli_fmt(&usage_formatter, "%s ", command->name);
    }
    else
    {
        cli_fmt(&usage_formatter, "%s ", default_program_name);
    }

    int help_spacing = 0;

    if (command->options.count > 0)
    {
        // Print out all the required options
        for (int opt_idx = 0; opt_idx < command->options.count; ++opt_idx)
        {
            // calculate max spacing for help strings
            help_spacing = CLI_MAX(help_spacing, cli_option_display_length(&command->options.data[opt_idx]));

            if (command->options.data[opt_idx].required)
            {
                const char* long_name = command->options.data[opt_idx].long_name;
                const int nargs = command->options.data[opt_idx].nargs;
                cli_fmt(&usage_formatter, "--%s %s", long_name, nargs != 0 ? "ARGS " : "");
            }
        }

        cli_fmt(&usage_formatter, "[options...] ");
    }

    if (command->positionals.count > 0)
    {
        for (int pos_idx = 0; pos_idx < command->positionals.count; ++pos_idx)
        {
            // calculate max spacing for help strings
            help_spacing = CLI_MAX(help_spacing, (int)strlen(command->positionals.data[pos_idx].name));

            // print out positionals with specific amount of spacing, i.e
            // `program positional1 positional2 ...`
            cli_fmt(&usage_formatter, "%s ", command->positionals.data[pos_idx].name);
        }
    }

    if (command->subcommands.count > 0)
    {
        cli_fmt(&usage_formatter, "<command> ");
        for (int i = 0; i < command->subcommands.count; ++i)
        {
            // calculate max spacing for help strings
            help_spacing = CLI_MAX(help_spacing, (int)strlen(command->subcommands.data[i].name));
        }
    }

    help_spacing = CLI_MAX(CLI_HELP_MIN_COLS, help_spacing + 4); // at least 4 spaces between the arg and the desc


    // output the positional args
    if (command->positionals.count > 0)
    {
        cli_fmt(&usage_formatter, "\n\nArguments:\n");

        // print out positionals with specific amount of spacing, i.e `positional1  help string`
        for (int pos_idx = 0; pos_idx < command->positionals.count; ++pos_idx)
        {
            cli_fmt(&usage_formatter, "  %s", command->positionals.data[pos_idx].name);
            cli_fmt_spaces_arg(&usage_formatter, help_spacing, command->positionals.data[pos_idx].name);
            cli_fmt(&usage_formatter, "%s\n", command->positionals.data[pos_idx].help);
        }
    }

    cli_fmt(&usage_formatter, "\nOptions:\n  -h, --help");
    cli_fmt_spaces_arg(&usage_formatter, help_spacing, "-h, --help");
    cli_fmt(&usage_formatter, "Returns this help message\n");

    if (command->options.count > 0)
    {
        // print out options, i.e `-o, --option1  help string`
        for (int opt_idx = 0; opt_idx < command->options.count; ++opt_idx)
        {
            cli_fmt(&usage_formatter, "  ");

            // Write out short name
            if (command->options.data[opt_idx].short_name != '\0')
            {
                cli_fmt(&usage_formatter, "-%c, ", command->options.data[opt_idx].short_name);
            }

            // Write long name
            cli_fmt(&usage_formatter, "--%s", command->options.data[opt_idx].long_name);
            cli_fmt_spaces_opt(&usage_formatter, help_spacing, &command->options.data[opt_idx]);
            // Write help string
            cli_fmt(&usage_formatter, "%s%s\n", command->options.data[opt_idx].help);
        }
    }

    // generate just a list of names for subcommands
    if (command->subcommands.count > 0)
    {
        cli_fmt(&usage_formatter, "\nCommands:\n");

        for (int cmd_idx = 0; cmd_idx < command->subcommands.count; ++cmd_idx)
        {
            cli_fmt(&usage_formatter, "  %s", command->subcommands.data[cmd_idx].name);

            if (cmd_idx < command->subcommands.count - 1)
            {
                cli_fmt(&usage_formatter, ", ");
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
cli_element cli_parse_element(const cli_parser* parser, const char* arg)
{
    if (arg == NULL)
    {
        return (cli_element) { .type = CLI_ELEMENT_INVALID };
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
        return (cli_element) { .type = CLI_ELEMENT_INVALID };
    }

    if (leading_dashes == 1)
    {
        return (cli_element) { .type = CLI_ELEMENT_SHORT_OPTION, .value = arg + 1, .length = 1 };
    }

    if (leading_dashes == 2)
    {
        // A solitary '--' argument indicates the command line should stop parsing
        const cli_element_type type = length == 2 ? CLI_ELEMENT_DELIMITER : CLI_ELEMENT_LONG_OPTION;
        return (cli_element) { .type = type, .value = arg + 2, .length = length - 2 };
    }

    const cli_command_result* command_result = parser->command_result;
    const cli_command* command_info = parser->command_info;
    const bool all_positionals_parsed = command_result->positionals_parsed >= command_info->positionals.count;
    const bool can_parse_subcommand = command_result->subcommand == NULL && command_info->subcommands.count > 0;

    return (cli_element) {
        .type = (all_positionals_parsed && can_parse_subcommand)  ? CLI_ELEMENT_SUBCOMMAND : CLI_ELEMENT_POSITIONAL,
        .value = arg,
        .length = length
    };
}

bool cli_compare_option(const cli_element* element, const char expected_short_name, const char* expected_long_name)
{
    if (element->type != CLI_ELEMENT_SHORT_OPTION && element->type != CLI_ELEMENT_LONG_OPTION)
    {
        return false;
    }

    if (element->length == 1 && element->value[0] == expected_short_name)
    {
        return true;
    }

    return strncmp(expected_long_name, element->value, element->length) == 0;
}

int cli_find_option(const cli_command* command, const cli_element* element)
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

cli_parse_status cli_parse_command(const int argc, char* const* argv, cli_result* program_result, cli_parser* parser)
{
    assert(program_result->commands_count < CLI_ARG_MAX);

    // set defaults
    parser->command_result->nargs_parsed = 0;
    parser->command_result->options_parsed = 0;
    parser->command_result->positionals_parsed = 0;
    parser->command_result->argc = argc;
    parser->command_result->argv = argv;
    parser->command_result->execute = parser->command_info->execute;
    parser->command_result->subcommand = NULL;

    cli_generate_help(parser, program_result->program_name);

    if (argc <= 0)
    {
        return CLI_PARSE_RESULT_HELP;
    }

    cli_missing_options_init(parser);

    const cli_command* command_info = parser->command_info;
    cli_command_result* command_result = parser->command_result;

    while (command_result->nargs_parsed < command_result->argc)
    {
        const char* arg = command_result->argv[command_result->nargs_parsed++];
        const cli_element element = cli_parse_element(parser, arg);

        switch (element.type)
        {
            case CLI_ELEMENT_INVALID:
            {
                // very dodgy - something went terribly wrong
                cli_fmt(&parser->error_formatter, "internal error -- invalid argument string pointer");
                return CLI_PARSE_RESULT_ERROR;
            }
            case CLI_ELEMENT_DELIMITER:
            {
                // detected ' -- ' : game over, man
                return CLI_PARSE_RESULT_PARSED;
            }
            case CLI_ELEMENT_SHORT_OPTION:
            case CLI_ELEMENT_LONG_OPTION:
            {
                // exit early and show help if this is the implicit -h/--help option
                if (cli_compare_option(&element, 'h', "help"))
                {
                    return CLI_PARSE_RESULT_HELP;
                }

                // find the given option and validate if exists
                const int option_index = cli_find_option(parser->command_info, &element);

                if (option_index < 0)
                {
                    cli_fmt(&parser->error_formatter, "unrecognized option %s", element.value);
                    return CLI_PARSE_RESULT_ERROR;
                }

                // parse all the arguments for the option
                const cli_option* option_info = &parser->command_info->options.data[option_index];
                const int preparsed_nargs = parser->command_result->nargs_parsed;

                // normal, restricted narg range [0, nargs]
                if (option_info->nargs >= 0)
                {
                    // fail if we will exceed argc without being able to parse all the required option arguments
                    if (CLI_MIN(parser->command_result->argc, option_info->nargs) < option_info->nargs)
                    {
                        cli_fmt(&parser->error_formatter,
                            "option -%c/--%s expected %d argument%s",
                            option_info->short_name,
                            option_info->long_name,
                            option_info->nargs,
                            option_info->nargs > 1 ? "s" : "" // plural on argument/s
                        );

                        return CLI_PARSE_RESULT_ERROR;
                    }

                    // otherwise we can safely skip past all options args
                    parser->command_result->nargs_parsed += option_info->nargs;
                }
                else
                {
                    // special narg range [1, argc]
                    for (int i = parser->command_result->nargs_parsed; i < parser->command_result->argc; ++i)
                    {
                        // we can't just verify argc we have to actually search through for the next '-/--'
                        if (*parser->command_result->argv[parser->command_result->nargs_parsed] == '-')
                        {
                            break;
                        }
                        ++parser->command_result->nargs_parsed;
                    }

                    const int min_nargs_required = option_info->nargs - CLI_0_OR_MORE; // repeated nargs is some distance from CLI_0_OR_MORE
                    if (parser->command_result->nargs_parsed - preparsed_nargs < min_nargs_required)
                    {
                        cli_fmt(&parser->error_formatter,
                            "option -%c/--%s expected at least %d arguments",
                            option_info->short_name,
                            option_info->long_name,
                            min_nargs_required
                        );

                        return CLI_PARSE_RESULT_ERROR;
                    }
                }

                // option parse success - add a new parsed one
                cli_parsed_option* option_result = &parser->command_result->options[parser->command_result->options_parsed++];
                option_result->long_name = option_info->long_name;
                option_result->short_name = option_info->short_name;
                option_result->nargs = parser->command_result->nargs_parsed - preparsed_nargs;
                option_result->args = option_info->nargs != 0 ? &parser->command_result->argv[preparsed_nargs] : NULL;

                // Remove from list of missing required options
                if (option_info->required)
                {
                    cli_missing_options_mark_parsed(parser, option_info);
                }
            }
            case CLI_ELEMENT_POSITIONAL:
            {
                // just add this to the positional array
                command_result->positionals[command_result->positionals_parsed++] = element.value;
                break;
            }
            case CLI_ELEMENT_SUBCOMMAND:
            {
                // if all the positionals have been parsed then this is either a subcommand or otherwise it's invalid
                const cli_command* subcommand_info = NULL;
                for (int i = 0; i < command_info->subcommands.count; ++i)
                {
                    if (strncmp(element.value, command_info->subcommands.data[i].name, element.length) == 0)
                    {
                        subcommand_info = &command_info->subcommands.data[i];
                        break;
                    }
                }

                // invalid - no such command
                if (subcommand_info == NULL)
                {
                    cli_fmt(&parser->error_formatter, "unrecognized argument: %s", arg);
                    return CLI_PARSE_RESULT_ERROR;
                }

                // ensure all required options were found before moving to a subparser
                if (parser->missing.count > 0)
                {
                    return cli_report_missing(parser);
                }

                // valid subcommand - recursively parse. Since positionals take precedence over subcommands this is fine
                assert(program_result->commands_count < CLI_ARG_MAX);

                // setup the parser for the next recursive subcommand parse call
                parser->command_info = subcommand_info;
                parser->command_result = &program_result->commands[program_result->commands_count++];

                // register the parsed command
                program_result->executed_command = program_result->commands_count - 1;
                command_result->execute = command_info->execute;
                command_result->subcommand = parser->command_result;

                // parse the subcommand
                const int subcommand_argc = argc - command_result->nargs_parsed;
                char* const* subcommand_argv = argv + command_result->nargs_parsed;
                return cli_parse_command(subcommand_argc, subcommand_argv, program_result, parser);
            }
        }
    }

    return CLI_PARSE_RESULT_PARSED;
}

/*
 *****************************
 *
 * Public API implementations
 *
 *****************************
 */
void cli_parse(const int argc, char* const* argv, cli_result* result, const cli_command* cli)
{
    // defaults for the result
    result->commands_count = 0;
    result->is_error = false;
    result->is_help = false;
    result->executed_command = 0;
    result->program_path = "";

    int subcommand_argc = argc;
    char* const* subcommand_argv = argv;

    // Check if empty args -- this should only occur with an adhoc command line, not one generated by the OS
    if (argc > 0)
    {
        assert(argv[0] != NULL);

        // Otherwise it's a normal command line
        result->program_path = argv[0];
        const int program_path_len = (int)strlen(result->program_path);

        // adjust command args to skip program name
        subcommand_argc = argc - 1;
        subcommand_argv = argv + 1;
    }

    // use the explicitly-specified name if one was provided
    if (cli->name != NULL)
    {
        strcpy(result->program_name, cli->name);
    }
    else if (result->program_path != NULL)
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
        const int full_exe_name = CLI_MIN(strlen(exe_name), CLI_PROGRAM_NAME_MAX);
        strncpy(result->program_name, exe_name, full_exe_name);

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
    else
    {
        memset(result->program_name, 0, CLI_PROGRAM_NAME_MAX);
    }

    const cli_parse_status status = cli_parse_command(subcommand_argc, subcommand_argv, result, &(cli_parser){
        .command_result = &result->commands[result->commands_count++],
        .command_info = cli,
        .error_formatter = (cli_formatter) { .buffer = result->error_message, .buffer_capacity = CLI_ERROR_MAX }
    });

    assert(result->commands_count > 0);
    result->usage = result->commands[0].usage;

    switch (status)
    {
        case CLI_PARSE_RESULT_ERROR:
        {
            result->is_error = true;
            break;
        }
        case CLI_PARSE_RESULT_HELP:
        {
            result->is_help = true;
            break;
        }
        default: break;
    }
}

const cli_parsed_option* cli_get_option(const cli_command_result* result, const char* option_long_name)
{
    for (int i = 0; i < result->options_parsed; ++i)
    {
        if (strcmp(result->options[i].long_name, option_long_name) == 0)
        {
            return &result->options[i];
        }
    }

    return NULL;
}

const cli_parsed_option* cli_get_option_short(const cli_command_result* result, const char option_short_name)
{
    for (int i = 0; i < result->options_parsed; ++i)
    {
        if (result->options[i].short_name == option_short_name)
        {
            return &result->options[i];
        }
    }
    return NULL;
}

const cli_parsed_option* cli_get_program_option(const cli_result* result, const char* option_long_name)
{
    return cli_get_option(&result->commands[0], option_long_name);
}

int cli_execute(const cli_result* result)
{
    assert(result->executed_command >= 0);
    assert(result->commands_count >= 0);
    assert(result->executed_command <= result->commands_count);

    const cli_command_result* command_result = &result->commands[result->executed_command];

    if (command_result->execute == NULL)
    {
        return 0;
    }

    return command_result->execute(result, command_result);
}