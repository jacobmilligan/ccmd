/*
 *  cmd.c
 *  ccmd
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#include <ccmd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

ccmd_status print_string(const ccmd_command_result* program, const ccmd_command_result* command)
{
    srand((unsigned int)time(NULL));

    const bool verbose = ccmd_get_option(program, "verbose") != NULL;
    const bool always = ccmd_get_option(command, "a") != NULL;
    const char* string_to_print = ccmd_get_positional(command, 0);

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
    return CCMD_STATUS_SUCCESS;
}

ccmd_status dump_files(const ccmd_command_result* program, const ccmd_command_result* command)
{
    const bool verbose = ccmd_has_option(program, "verbose");
    const ccmd_parsed_args* input_string = ccmd_get_option(command, "input");
    const ccmd_parsed_args* output_paths = ccmd_get_option(command, "output");

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

    return CCMD_STATUS_SUCCESS;
}

int main(int argc, char** argv)
{
#define CCMD_EXAMPLE_REDIRECT_OUTPUT 0

#if CCMD_EXAMPLE_REDIRECT_OUTPUT == 1
    static char usage_buffer[4096];
    static char error_buffer[4096];
#endif // CCMD_EXAMPLE_REDIRECT_OUTPUT

    ccmd_command_result commands[8];
    ccmd_parsed_args options[16];
    ccmd_result result = { .commands = CCMD_ARRAY_VIEW(commands), .options = CCMD_ARRAY_VIEW(options) };
    const ccmd_status status = ccmd_parse(&result, argc, argv, &(ccmd_command) {
        .name = "program-name",
        .help = "A program for doing things",

#if CCMD_EXAMPLE_REDIRECT_OUTPUT == 1
        .usage_buffer = CCMD_ARRAY_VIEW(usage_buffer),
        .error_buffer = CCMD_ARRAY_VIEW(error_buffer),
#endif // CCMD_EXAMPLE_REDIRECT_OUTPUT == 1

        .options = CCMD_ARRAY_VIEW((ccmd_option[]) {
            { .short_name = 'v', .long_name = "verbose", .help = "prints status of the commands", .nargs = 0, .required = false }
        }),
        .subcommands = CCMD_ARRAY_VIEW((ccmd_command[]) {
            {
                .name = "dump-files",
                .help = "dumps input to a given set of absolute filepaths",
                .run = dump_files,
                .options = CCMD_ARRAY_VIEW((ccmd_option[]){
                    { .short_name = 'i', .long_name = "input", .help = "string to dump to the file/s", .nargs = 1, .required = true },
                    { .short_name = 'o', .long_name = "output", .help = "file/s to dump to", .nargs = CCMD_N_OR_MORE(1), .required = true }
                })
            },
            {
                .name = "print-string",
                .run = print_string,
                .positionals = CCMD_ARRAY_VIEW((ccmd_positional[]) {
                    { .name = "string", .help = "the string to print" }
                }),
                .options = CCMD_ARRAY_VIEW((ccmd_option[]) {
                    { .short_name = 'a', .long_name = "always", .help = "always prints the string - if not specified, the thing will be random", .nargs = 0, .required = false }
                })
            }
        })
    });

#if CCMD_EXAMPLE_REDIRECT_OUTPUT == 1
    switch (status)
    {
        case CCMD_STATUS_ERROR:
        {
            fprintf(stderr, "%s: error: %s\n", result.program_name, error_buffer);
            break;
        }
        case CCMD_STATUS_HELP:
        {
            printf("\n--- Custom usage message ---\n\n%s\n", usage_buffer);
            break;
        }
        default: break;
    }
#endif // CCMD_EXAMPLE_REDIRECT_OUTPUT

    if (status != CCMD_STATUS_SUCCESS)
    {
        return status == CCMD_STATUS_ERROR ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    const bool verbose = ccmd_get_option(result.program_command, "verbose") != NULL;
    if (verbose)
    {
        printf("%s [verbose]: executing program...\n", result.program_name);
    }

    if (ccmd_run(&result) != CCMD_STATUS_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}