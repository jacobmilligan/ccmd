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

struct cli_formatter
{
    int     buffer_capacity;
    char*   buffer;

    int     length;
};

typedef struct cli_missing_options
{
    int                 count;
    const cli_option* data[CLI_ARG_MAX];
} cli_missing_options;


/*
 **************************
 *
 * Utils API
 *
 **************************
 */
int cli_max(const int x, const int y)
{
    return x >= y ? x : y;
}

int cli_option_display_length(const cli_option* opt)
{
    int size = 2 + (int)strlen(opt->long_name); // 2 extra for '--'
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
            return cli_max(0, i - 1);
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
void cli_missing_options_init(cli_missing_options* missing, const cli_command* command)
{
    missing->count = 0;

    for (int i = 0; i < command->options.count; ++i)
    {
        if (command->options.data[i].required)
        {
            assert(missing->count < CLI_ARG_MAX);
            missing->data[missing->count++] = &command->options.data[i];
        }
    }
}

void cli_missing_options_mark_parsed(cli_missing_options* missing, const cli_option* option)
{
    for (int i = 0; i < missing->count; ++i)
    {
        if (option != missing->data[i])
        {
            continue;
        }

        // swap and pop the last item if i is not last
        if (i == missing->count - 1)
        {
            --missing->count;
            break;
        }
        else
        {
            const cli_option* tmp = missing->data[missing->count];
            missing->data[missing->count] = missing->data[i];
            missing->data[i] = tmp;
        }
    }
}

void cli_missing_options_format(const cli_missing_options* missing, struct cli_formatter* formatter)
{
    cli_fmt(formatter, "missing required options:");
    for (int i = 0; i < missing->count; ++i)
    {
        cli_fmt(formatter, " -%c/--%s", missing->data[i]->short_name, missing->data[i]->long_name);
        if (i < missing->count - 1)
        {
            cli_fmt(formatter, ",");
        }
    }
}

/*
 *****************************
 *
 * Generate a usage message
 * from the command spec
 *
 *****************************
 */
void cli_generate_help(const cli_command* command, cli_command_result* result)
{
    struct cli_formatter usage_formatter = { .buffer_capacity = CLI_HELP_MAX, .buffer = result->usage };

    cli_fmt(&usage_formatter, "usage: ");

    if (command->name != NULL)
    {
        cli_fmt(&usage_formatter, "%s ", command->name);
    }

    int help_spacing = 0;

    if (command->options.count > 0)
    {
        // Print out all the required options
        for (int opt_idx = 0; opt_idx < command->options.count; ++opt_idx)
        {
            // calculate max spacing for help strings
            help_spacing = cli_max(help_spacing, cli_option_display_length(&command->options.data[opt_idx]));

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
            help_spacing = cli_max(help_spacing, (int)strlen(command->positionals.data[pos_idx].name));

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
            help_spacing = cli_max(help_spacing, (int)strlen(command->subcommands.data[i].name));
        }
    }

    help_spacing = cli_max(CLI_HELP_MIN_COLS, help_spacing + 4); // at least 4 spaces between the arg and the desc


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
bool cli_is_option(const char* option_name, const int option_name_length, const char expected_short_name, const char* expected_long_name)
{
    if (option_name_length == 1 && option_name[0] == expected_short_name)
    {
        return true;
    }

    return strncmp(expected_long_name, option_name, option_name_length) == 0;
}

int cli_find_option(const cli_command* command, const char* option_name, const int option_name_length)
{
    for (int i = 0; i < command->options.count; ++i)
    {
        if (cli_is_option(option_name, option_name_length, command->options.data[i].short_name, command->options.data[i].long_name))
        {
            return i;
        }
    }

    return -1;
}

cli_parse_status cli_parse_command(const int argc, char* const* argv, cli_result* program_result, const cli_command* command_info, cli_command_result* command_result, cli_missing_options* missing)
{
    assert(program_result->commands_count < CLI_ARG_MAX);

    struct cli_formatter error_fmt = { .buffer = program_result->error_message, .buffer_capacity = CLI_ERROR_MAX };

    // set defaults
    command_result->nargs_parsed = 0;
    command_result->options_parsed = 0;
    command_result->positionals_parsed = 0;
    command_result->argc = argc;
    command_result->argv = argv;
    command_result->execute = NULL;

    cli_generate_help(command_info, command_result);

    if (command_result->argc <= 0)
    {
        return CLI_PARSE_RESULT_HELP;
    }

    cli_missing_options_init(missing, command_info);

    while (command_result->nargs_parsed < command_result->argc)
    {
        const char* arg = command_result->argv[command_result->nargs_parsed++];
        const int arg_length = (int)strlen(arg);

        // A solitary '--' argument indicates the command line should stop parsing
        if (strncmp(arg, "--", arg_length) == 0)
        {
            break;
        }

        // Try parsing as an option (i.e. starts with -/--)
        if (arg_length >= 2 && arg[0] == '-')
        {
            int option_begin = 0;

            // find the name after the '--'
            for (int o = 0; o < arg_length; ++o)
            {
                if (arg[o] != '-')
                {
                    break;
                }

                ++option_begin;
            }

            // if there's more than two '--' then fail as unrecognized arg
            if (option_begin > 1)
            {
                cli_fmt(&error_fmt, "unrecognized option %s", arg);
                return CLI_PARSE_RESULT_ERROR;
            }

            const char* option_name = arg + option_begin;
            const int option_length = arg_length - option_begin;

            // exit early and show help if this is the implicit -h/--help option
            if (cli_is_option(option_name, option_length, 'h', "help"))
            {
                return CLI_PARSE_RESULT_HELP;
            }

            // find the given option and validate if exists
            const int option_index = cli_find_option(command_info, option_name, option_length);

            if (option_index < 0)
            {
                cli_fmt(&error_fmt, "unrecognized option %s", arg);
                return CLI_PARSE_RESULT_ERROR;
            }

            const cli_option* option_info = &command_info->options.data[option_index];

            for (int i = 0; i < option_info->nargs; ++i)
            {
                // fail if we exceeded argc but haven't parsed all the required option arguments
                if (command_result->nargs_parsed >= command_result->argc)
                {
                    cli_fmt(
                        &error_fmt,
                        "option -%c/--%s expected %d argument%s",
                        option_info->short_name,
                        option_info->long_name,
                        option_info->nargs,
                        option_info->nargs <= 1 ? "" : "s" // plural on argument/s
                    );

                    return false;
                }

                // next arg
                ++command_result->nargs_parsed;
            }

            // option parse success - add a new parsed one
            cli_parsed_option* option_result = &command_result->options[command_result->options_parsed++];
            option_result->name = option_info->long_name;
            option_result->nargs = option_info->nargs;
            option_result->args = option_info->nargs != 0 ? &argv[command_result->options_parsed] : NULL;

            // Remove from list of missing required options
            if (option_info->required)
            {
                cli_missing_options_mark_parsed(missing, option_info);
            }
        }
        else
        {
            // otherwise parse as positional or subcommand (depending on how many positionals have already been consumed)
            if (command_result->positionals_parsed < command_info->positionals.count)
            {
                command_result->positionals[command_result->positionals_parsed++] = arg;
            }
            else
            {
                // if all the positionals have been parsed then this is either a subcommand or invalid
                const cli_command* subcommand_info = NULL;
                for (int i = 0; i < command_info->subcommands.count; ++i)
                {
                    if (strncmp(arg, command_info->subcommands.data[i].name, arg_length) == 0)
                    {
                        subcommand_info = &command_info->subcommands.data[i];
                        break;
                    }
                }

                // invalid - no such command
                if (subcommand_info == NULL)
                {
                    cli_fmt(&error_fmt, "unrecognized argument: %s", arg);
                    return CLI_PARSE_RESULT_ERROR;
                }

                // ensure all required options were found before moving to a subparser
                if (missing->count > 0)
                {
                    cli_missing_options_format(missing, &error_fmt);
                    return CLI_PARSE_RESULT_ERROR;
                }

                // valid subcommand - recursively parse. Since positionals take precedence over subcommands this is fine
                assert(program_result->commands_count < CLI_ARG_MAX);
                cli_command_result* subcommand = &program_result->commands[program_result->commands_count++];
                program_result->executed_command = program_result->commands_count;
                command_result->execute = command_info->execute;
                command_result->subcommand = subcommand;

                const int subcommand_argc = argc - command_result->nargs_parsed;
                char* const* subcommand_argv = argv + command_result->nargs_parsed;
                return cli_parse_command(subcommand_argc, subcommand_argv, program_result, subcommand_info, subcommand, missing);
            }
        }
    }

    return true;
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
    result->program_name = NULL;
    result->commands_count = 0;
    result->is_error = false;
    result->is_help = false;
    result->executed_command = 0;

    int subcommand_argc = argc;
    char* const* subcommand_argv = argv;

    // Check if empty args -- this should only occur with an adhoc command line, not one generated by the OS
    if (argc <= 0)
    {
        result->program_name = "<>";
    }
    else
    {
        // Otherwise it's a normal command line -- so first, discover the program name from argc if any args
        const int program_path_len = (int)strlen(argv[0]);

        for (int i = 0; i < program_path_len; ++i)
        {
            if (argv[0][i] == CLI_SEPERATOR_CHAR && i < program_path_len - 1)
            {
                result->program_name = &argv[0][i + 1];
            }
        }

        if (result->program_name == NULL)
        {
            result->program_name = "<>";
        }
        else
        {
            // adjust the command line to exclude the program path
            subcommand_argc = argc - 1;
            subcommand_argv = argv + 1;
        }
    }

    cli_missing_options missing;
    cli_command_result* command = &result->commands[result->commands_count++];
    const cli_parse_status status = cli_parse_command(subcommand_argc, subcommand_argv, result, cli, command, &missing);

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
        if (strcmp(result->options[i].name, option_long_name) == 0)
        {
            return &result->options[i];
        }
    }

    return NULL;
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

    return command_result->execute(command_result);
}