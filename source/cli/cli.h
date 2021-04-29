#include <stdbool.h>
#include <stdint.h>

#ifndef CLI_PROGRAM_NAME_MAX
    #define CLI_PROGRAM_NAME_MAX 256
#endif // CLI_PROGRAM_NAME_MAX

#ifndef CLI_ARG_MAX
    #define CLI_ARG_MAX 64
#endif // CLI_ARG_MAX

#ifndef CLI_HELP_MAX
    #define CLI_HELP_MAX 4096
#endif // CLI_HELP_MAX

#ifndef CLI_ERROR_MAX
    #define CLI_ERROR_MAX 4096
#endif // CLI_ERROR_MAX

#define CLI_ARRAY_SIZE(ARRAY) (sizeof((ARRAY)) / sizeof((ARRAY)[0]))

#define CLI_ARGS(TYPE, ...)                                 \
    {                                                       \
        .count = CLI_ARRAY_SIZE(((TYPE[]) {__VA_ARGS__})),  \
        .data = (TYPE[]) { __VA_ARGS__}                     \
    }

#define CLI_OPTIONS(...) CLI_ARGS(cli_option, __VA_ARGS__)
#define CLI_POSITIONALS(...) CLI_ARGS(cli_positional, __VA_ARGS__)
#define CLI_SUBCOMMANDS(...) CLI_ARGS(cli_command, __VA_ARGS__)

#define CLI_DEFINE_ARRAY(T) struct T; typedef struct T##_array { int count; const struct T* data; } T##_array;

struct cli_result;
struct cli_command_result;

typedef int(*cli_command_callback)(const struct cli_command_result* command);

typedef struct cli_positional
{
    const char* name;
    const char* help;
} cli_positional;

typedef struct cli_option
{
    char        short_name;
    const char* long_name;
    const char* help;
    bool        required;
    int         nargs;
} cli_option;

CLI_DEFINE_ARRAY(cli_positional)
CLI_DEFINE_ARRAY(cli_option)
CLI_DEFINE_ARRAY(cli_command)

typedef struct cli_command
{
    const char*                 name;
    cli_positional_array        positionals;
    cli_option_array            options;
    cli_command_array           subcommands;
    const cli_command_callback  execute;
} cli_command;

typedef struct cli_parsed_option
{
    const char*     name;
    char* const*    args;
    int             nargs;
} cli_parsed_option;

typedef struct cli_command_result
{
    int                                 argc;
    char* const*                        argv;
    int                                 nargs_parsed;
    int                                 positionals_parsed;
    int                                 options_parsed;

    char                                usage[CLI_HELP_MAX];

    const char*                         positionals[CLI_ARG_MAX];
    cli_parsed_option                   options[CLI_ARG_MAX];
    cli_command_callback                execute;
    const struct cli_command_result*    subcommand;
} cli_command_result;

typedef struct cli_result
{
    char                    program_name[CLI_PROGRAM_NAME_MAX];
    const char*             program_path;
    const char*             usage;
    char                    error_message[CLI_ERROR_MAX];

    bool                    is_error;
    bool                    is_help;

    int                     commands_count;
    int                     executed_command;
    cli_command_result      commands[CLI_ARG_MAX];
} cli_result;

void cli_parse(const int argc, char* const* argv, cli_result* result, const cli_command* cli);

const cli_parsed_option* cli_get_option(const cli_command_result* result, const char* option_long_name);

int cli_execute(const cli_result* result);
