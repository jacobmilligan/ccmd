/*
 *  cmd.c
 *  cli
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#include <cli/cli.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int print_string(const cli_result* program, const cli_command_result* command)
{
    srand((unsigned int)time(NULL));

    const bool verbose = cli_get_program_option(program, "verbose") != NULL;
    const bool always = cli_get_option_short(command, 'a') != NULL;
    const char* string_to_print = command->positionals[0];

    if (verbose)
    {
        if (always)
        {
            printf("%s [verbose]: printing \"%s\"...\n", program->program_name, string_to_print);
        }
        else
        {
            printf("%s [verbose]: attempting to print \"%s\"...\n", program->program_name, string_to_print);
        }
    }

    if (rand() % 4 == 0 || always)
    {
        printf("%s\n", string_to_print);
    }
    else
    {
        printf("...\n");
    }
    return 0;
}

int dump_files(const cli_result* program, const cli_command_result* command)
{
    const bool verbose = cli_get_program_option(program, "verbose") != NULL;
    const cli_parsed_option* input_string = cli_get_option(command, "input");
    const cli_parsed_option* output_paths = cli_get_option(command, "output");

    for (int i = 0; i < output_paths->nargs; ++i)
    {
        if (verbose)
        {
            printf("%s [verbose]: dumping to %s\n", program->program_name, output_paths->args[i]);
        }

        FILE* file = fopen(output_paths->args[i], "w");
        fwrite(input_string->args[0], strlen(input_string->args[0]), 1, file);
        fclose(file);
    }

    return 0;
}

int main(int argc, char** argv)
{
    cli_result result;
    cli_parse(argc, argv, &result, &(cli_command) {
        .options = CLI_OPTIONS(
        {
            .short_name = 'v',
            .long_name = "verbose",
            .help = "prints status of the commands",
            .required = false,
            .nargs = 0
        }),
        .subcommands = CLI_SUBCOMMANDS(
        {
            .name = "dump-files",
            .execute = dump_files,
            .options = CLI_OPTIONS(
            {
                .short_name = 'i',
                .long_name = "input",
                .help = "string to dump to the file/s",
                .nargs = 1,
                .required = true
            },
            {
                .short_name = 'o',
                .long_name = "output",
                .help = "file/s to dump to",
                .nargs = CLI_N_OR_MORE(1),
                .required = true
            }),
        },
        {
            .name = "print-string",
            .execute = print_string,
            .positionals = CLI_POSITIONALS(
            {
                .name = "string",
                .help = "the string to print"
            }),
            .options = CLI_OPTIONS(
            {
                .short_name = 'a',
                .long_name = "always",
                .help = "always does the thing - if not specified, the thing will be random",
                .nargs = 0,
                .required = false
            })
        })
    });

    if (result.is_error)
    {
        fprintf(stderr, "%s: error: %s\n", result.program_name, result.error_message);
        return 1;
    }

    if (result.is_help)
    {
        printf("%s\n", result.commands[0].usage);
        return 0;
    }

    const bool verbose = cli_get_program_option(&result, "verbose") != NULL;
    if (verbose)
    {
        printf("%s [verbose]: executing program...\n", result.program_name);
    }

    return cli_execute(&result);
}