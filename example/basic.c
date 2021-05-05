/*
 *  cmd.c
 *  cli
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#include <cli/cli.h>

#include <stdio.h>
#include <stdlib.h>

#define FILE_BUFFER_SIZE 4096
static char file_buffer[FILE_BUFFER_SIZE] = { 0 };

cli_status print_header_command(const cli_command_result* program, const cli_command_result* command)
{
    const bool verbose = cli_has_option(program, "verbose");

    if (verbose)
    {
        printf("%s [verbose]: printing header\n", program->name);
    }

    if (cli_has_option(command, "u"))
    {
        printf("--- READ FILE AND PRINT V1.0 ---\n\n");
    }
    else
    {
        printf("--- Read file and print v1.0 ---\n\n");
    }

    return CLI_STATUS_SUCCESS;
}

int main(int argc, char** argv)
{
    cli_result result;
    cli_status status = cli_parse(&result, argc, argv, &(cli_command) {
        .help = "Reads some files and prints them to stdout",
        .positionals = CLI_ARRAY_VIEW((cli_positional[]) {
            { .name = "first-file", .help = "File to read and print to stdout" }
        }),
        .options = CLI_ARRAY_VIEW((cli_option[]) {
            { .short_name = 'v', .long_name = "verbose", .help = "Prints status of the commands", .nargs = 0, .required = false },
            { .short_name = 's', .long_name = "second-file", .help = "Path to another file to read and append to stdout", .nargs = 1, .required = true },
        }),
        .subcommands = CLI_ARRAY_VIEW((cli_command[]) {
            {
                .name = "print-header",
                .help = "Prints a header before printing any file contents",
                .run = print_header_command, // callback executed if this subcommand is requested
                .options = CLI_ARRAY_VIEW((cli_option[]){
                    { .short_name = 'u', .long_name = "--uppercase", .help = "Print an UPPERCASE header", .nargs = 0, .required = false },
                })
            }
        })
    });

    if (status != CLI_STATUS_SUCCESS)
    {
        return status == CLI_STATUS_ERROR ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    const cli_command_result* program = &result.commands[0];
    const bool verbose = cli_has_option(program, "verbose");

    if (verbose)
    {
        printf("%s [verbose]: Verbose output is turned on\n", program->name);
    }

    // run any subcommands that might have been encountered before printing the files
    if (cli_run(&result) != CLI_STATUS_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    const char* paths[] = {
        cli_get_positional(program, 0),                     // first file
        cli_get_option(program, "second-file")->args[0],    // second file
    };

    if (verbose)
    {
        for (int i = 0; i < CLI_ARRAY_SIZE(paths); ++i)
        {
            printf("%s [verbose]: file %d: %s\n", program->name, i, paths[0]);
        }
    }

    for (int i = 0; i < CLI_ARRAY_SIZE(paths); ++i)
    {
        FILE* file = fopen(paths[i], "r");
        {
            if (file == NULL)
            {
                fprintf(stderr, "%s: error: invalid file path: %s\n", program->name, paths[i]);
                return EXIT_FAILURE;
            }

            fseek(file, 0, SEEK_END);
            const int file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            fread(file_buffer, 1, file_size > FILE_BUFFER_SIZE ? FILE_BUFFER_SIZE : file_size, file);
        }
        fclose(file);
        printf("%s\n", file_buffer);
    }

    return EXIT_SUCCESS;
}