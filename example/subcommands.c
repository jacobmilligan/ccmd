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

cli_status print_string(const cli_command_result* program, const cli_command_result* command)
{
    srand((unsigned int)time(NULL));

    const bool verbose = cli_get_option(program, "verbose") != NULL;
    const bool always = cli_get_option(command, "a") != NULL;
    const char* string_to_print = cli_get_positional(command, 0);

    if (verbose)
    {
        if (always)
        {
            printf("%s [verbose]: printing \"%s\"...\n", program->name, string_to_print);
        }
        else
        {
            printf("%s [verbose]: attempting to print \"%s\"...\n", program->name, string_to_print);
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
    return CLI_STATUS_SUCCESS;
}

cli_status dump_files(const cli_command_result* program, const cli_command_result* command)
{
    const bool verbose = cli_has_option(program, "verbose");
    const cli_parsed_args* input_string = cli_get_option(command, "input");
    const cli_parsed_args* output_paths = cli_get_option(command, "output");

    for (int i = 0; i < output_paths->nargs; ++i)
    {
        if (verbose)
        {
            printf("%s [verbose]: dumping to %s\n", program->name, output_paths->args[i]);
        }

        FILE* file = fopen(output_paths->args[i], "w");
        fwrite(input_string->args[0], strlen(input_string->args[0]), 1, file);
        fclose(file);
    }

    return CLI_STATUS_SUCCESS;
}

int main(int argc, char** argv)
{
#define CLI_EXAMPLE_REDIRECT_OUTPUT 0

#if CLI_EXAMPLE_REDIRECT_OUTPUT == 1
    static char usage_buffer[4096];
    static char error_buffer[4096];
#endif // CLI_EXAMPLE_REDIRECT_OUTPUT

    cli_result result;
    const cli_status status = cli_parse(&result, argc, argv, &(cli_command) {
        .name = "program-name",
        .help = "A program for doing things",

#if CLI_EXAMPLE_REDIRECT_OUTPUT == 1
        .usage_buffer = CLI_ARRAY_VIEW(usage_buffer),
        .error_buffer = CLI_ARRAY_VIEW(error_buffer),
#endif // CLI_EXAMPLE_REDIRECT_OUTPUT == 1

        .options = CLI_ARRAY_VIEW((cli_option[]) {
            { .short_name = 'v', .long_name = "verbose", .help = "prints status of the commands", .nargs = 0, .required = false }
        }),
        .subcommands = CLI_ARRAY_VIEW((cli_command[]) {
            {
                .name = "dump-files",
                .help = "dumps input to a given set of absolute filepaths",
                .run = dump_files,
                .options = CLI_ARRAY_VIEW((cli_option[]){
                    { .short_name = 'i', .long_name = "input", .help = "string to dump to the file/s", .nargs = 1, .required = true },
                    { .short_name = 'o', .long_name = "output", .help = "file/s to dump to", .nargs = CLI_N_OR_MORE(1), .required = true }
                })
            },
            {
                .name = "print-string",
                .run = print_string,
                .positionals = CLI_ARRAY_VIEW((cli_positional[]) {
                    { .name = "string", .help = "the string to print" }
                }),
                .options = CLI_ARRAY_VIEW((cli_option[]) {
                    { .short_name = 'a', .long_name = "always", .help = "always prints the string - if not specified, the thing will be random", .nargs = 0, .required = false }
                })
            }
        })
    });

#if CLI_EXAMPLE_REDIRECT_OUTPUT == 1
    switch (status)
    {
        case CLI_STATUS_ERROR:
        {
            fprintf(stderr, "%s: error: %s\n", result.program_name, error_buffer);
            break;
        }
        case CLI_STATUS_HELP:
        {
            printf("\n--- Custom usage message ---\n\n%s\n", usage_buffer);
            break;
        }
        default: break;
    }
#endif // CLI_EXAMPLE_REDIRECT_OUTPUT

    if (status != CLI_STATUS_SUCCESS)
    {
        return status == CLI_STATUS_ERROR ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    const bool verbose = cli_get_option(&result.commands[0], "verbose") != NULL;
    if (verbose)
    {
        printf("%s [verbose]: executing program...\n", result.program_name);
    }

    if (cli_run(&result) != CLI_STATUS_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}