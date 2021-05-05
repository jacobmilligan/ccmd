#include <stdbool.h>
#include <stdint.h>

#ifndef CLI_PROGRAM_NAME_MAX
    #define CLI_PROGRAM_NAME_MAX 256
#endif // CLI_PROGRAM_NAME_MAX

#ifndef CLI_ARG_MAX
    #define CLI_ARG_MAX 64
#endif // CLI_ARG_MAX

#define CLI_0_OR_MORE INT32_MIN
#define CLI_N_OR_MORE(N) ((N) <= 0 ? CLI_0_OR_MORE : (CLI_0_OR_MORE + (N)))

#define CLI_ARRAY_SIZE(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

struct cli_result;
struct cli_command_result;

typedef enum cli_status
{
    CLI_STATUS_SUCCESS,
    CLI_STATUS_ERROR,
    CLI_STATUS_HELP,
    CLI_STATUS_COUNT
} cli_status;

#define CLI_ARRAY_VIEW_TYPE(T) struct { int32_t count; T* data; } // NOLINT(bugprone-macro-parentheses)
#define CLI_ARRAY_VIEW(...) { CLI_ARRAY_SIZE((__VA_ARGS__)), __VA_ARGS__ }

typedef struct cli_error
{
    uint32_t    key;
    int32_t     int32;
    char        char8;
    const char* str;
} cli_error;

typedef struct cli_error_buffer
{
    int32_t     count;
    cli_error   data[CLI_ARG_MAX];
} cli_error_buffer;

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
    int32_t     nargs;
    bool        required;
} cli_option;

typedef cli_status(*cli_run_callback)(const struct cli_command_result* program, const struct cli_command_result* command);

typedef struct cli_command
{
    const char*         name;
    const char*         help;

    // buffers to redirect usage/error messages
    cli_error_buffer*   error_buffer;

    CLI_ARRAY_VIEW_TYPE(char)
    usage_buffer;

    CLI_ARRAY_VIEW_TYPE(const cli_positional)
    positionals;

    CLI_ARRAY_VIEW_TYPE(const cli_option)
    options;

    CLI_ARRAY_VIEW_TYPE(const struct cli_command)
    subcommands;

    cli_run_callback    run;
} cli_command;

typedef struct cli_parsed_args
{
    char            short_name;
    const char*     long_name;
    char* const*    args;
    int32_t         nargs;
} cli_parsed_args;

typedef struct cli_command_result
{
    const char*             name;

    CLI_ARRAY_VIEW_TYPE(char* const)
    positionals;
    CLI_ARRAY_VIEW_TYPE(const cli_parsed_args)
    options;

    cli_run_callback        run;
} cli_command_result;

typedef struct cli_result
{
    char                        program_name[CLI_PROGRAM_NAME_MAX];
    const char*                 program_path;
    int32_t                     option_count;
    int32_t                     commands_count;
    cli_parsed_args             options[CLI_ARG_MAX];
    cli_command_result          commands[CLI_ARG_MAX];
} cli_result;

cli_status cli_parse(cli_result* result, const int32_t argc, char* const* argv, const cli_command* cli);

cli_status cli_run(const cli_result* program);

cli_status cli_run_all(const cli_result* program);

bool cli_has_positional(const cli_command_result* command, const int32_t position);

const char* cli_get_positional(const cli_command_result* command, const int32_t position);

bool cli_has_option(const cli_command_result* command, const char* long_or_short_name);

const cli_parsed_args* cli_get_option(const cli_command_result* command, const char* long_or_short_name);
