/*
 *  cmd.c
 *  cli
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#include <cli/cli.h>

#include <stdio.h>


int cb(const cli_result* result)
{
    printf("Hey!");
    return 0;
}

int main(int argc, char** argv)
{
    cli_result result;
    cli_parse(argc, argv, &result, &(cli_command) {
        .name = "test-app",

        .positionals = CLI_POSITIONALS(
        {
            .name = "my-command",
            .help = "a positional command"
        }),

        .options = CLI_OPTIONS(
        {
            .short_name = 'i',
            .long_name = "input",
            .help = "gets input",
            .nargs = 1,
            .required = false
        },
        {
            .short_name = 'o',
            .long_name = "output",
            .help = "outputs to file",
            .nargs = 1,
            .required = true
        }),

        .subcommands = CLI_SUBCOMMANDS(
        {
            .name = "do-thing",
            .execute = cb,
            .options = CLI_ARGS(cli_option,
                {
                    .short_name = 'a',
                    .long_name = "always",
                    .nargs = 0,
                    .required = false
                }
            )
        })
    });

    if (result.is_error)
    {
        fprintf(stderr, "%s: %s\n", result.program_name, result.error_message);
    }
    else if (result.is_help)
    {
        printf("%s\n", result.commands[0].usage);
    }
    else
    {
        printf("input: %s\n", cli_get_option(&result.commands[0], "input")->args[0]);
    }

    return cli_execute(&result);
}