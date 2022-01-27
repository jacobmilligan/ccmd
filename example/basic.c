/*
 *  cmd.c
 *  ccmd
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#include <ccmd.h>

#include <stdio.h>
#include <stdlib.h>

#define FILE_BUFFER_SIZE 4096
static char file_buffer[FILE_BUFFER_SIZE] = { 0 };

ccmd_status print_header_command(const ccmd_command_result* program, const ccmd_command_result* command)
{
    const bool verbose = ccmd_has_option(program, "verbose");

    if (verbose)
    {
        printf("%s [verbose]: printing header\n", program->name);
    }

    if (ccmd_has_option(command, "u"))
    {
        printf("--- READ FILE AND PRINT V1.0 ---\n\n");
    }
    else
    {
        printf("--- Read file and print v1.0 ---\n\n");
    }

    return CCMD_STATUS_SUCCESS;
}

int main(int argc, char** argv)
{
    ccmd_command_result commands[8];
    ccmd_parsed_args options[16];
    ccmd_result result = { .commands = CCMD_ARRAY_VIEW(commands), .options = CCMD_ARRAY_VIEW(options) };
    ccmd_status status = ccmd_parse(&result, argc, argv, &(ccmd_command) {
        .help = "Reads some files and prints them to stdout",
        .positionals = CCMD_ARRAY_VIEW((ccmd_positional[]) {
            { .name = "first-file", .help = "File to read and print to stdout" }
        }),
        .options = CCMD_ARRAY_VIEW((ccmd_option[]) {
            { .short_name = 'v', .long_name = "verbose", .help = "Prints status of the commands", .nargs = 0, .required = false },
            { .short_name = 's', .long_name = "second-file", .help = "Path to another file to read and append to stdout", .nargs = 1, .required = true },
        }),
        .subcommands = CCMD_ARRAY_VIEW((ccmd_command[]) {
            {
                .name = "print-header",
                .help = "Prints a header before printing any file contents",
                .run = print_header_command, // callback executed if this subcommand is requested
                .options = CCMD_ARRAY_VIEW((ccmd_option[]){
                    { .short_name = 'u', .long_name = "--uppercase", .help = "Print an UPPERCASE header", .nargs = 0, .required = false },
                })
            }
        })
    });

    if (status != CCMD_STATUS_SUCCESS)
    {
        return status == CCMD_STATUS_ERROR ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    const bool verbose = ccmd_has_option(result.program_command, "verbose");

    if (verbose)
    {
        printf("%s [verbose]: Verbose output is turned on\n", result.program_command->name);
    }

    // run any subcommands that might have been encountered before printing the files
    if (ccmd_run(&result) != CCMD_STATUS_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    const char* paths[] = {
        ccmd_get_positional(result.program_command, 0),                     // first file
        ccmd_get_option(result.program_command, "second-file")->args[0],    // second file
    };

    if (verbose)
    {
        for (int i = 0; i < CCMD_ARRAY_SIZE(paths); ++i)
        {
            printf("%s [verbose]: file %d: %s\n", result.program_command->name, i, paths[0]);
        }
    }

    for (int i = 0; i < CCMD_ARRAY_SIZE(paths); ++i)
    {
        FILE* file = fopen(paths[i], "r");
        {
            if (file == NULL)
            {
                fprintf(stderr, "%s: error: invalid file path: %s\n", result.program_command->name, paths[i]);
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