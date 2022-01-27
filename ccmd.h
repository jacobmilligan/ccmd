#include <stdbool.h>
#include <stdint.h>

/*
 ****************************************************
 *
 * DLL API macro boilerplate for dllexport/dllimport
 *
 ****************************************************
 */
#if defined(CCMD_DLL) && !defined(CCMD_API)
    #if defined(__WIN32__) || defined(__WINDOWS__) || defined(_WIN64) || defined(_WIN32) || defined(_WINDOWS) || defined(__TOS_WIN__)
        #ifdef CCMD_IMPLEMENTATION
            #define CCMD_API __declspec(dllexport)
        #else
            #define CCMD_API __declspec(dllimport)
        #endif // CCMD_IMPLEMENTATION
    #endif // defined(__WIN32__) || *
#endif // CCMD_DLL
#ifndef CCMD_API
    #define CCMD_API
#endif // CCMD_API

/*
 ****************************************************
 *
 * DLL API macro boilerplate for dllexport/dllimport
 *
 ****************************************************
 */
#ifndef CCMD_PROGRAM_NAME_MAX
    #define CCMD_PROGRAM_NAME_MAX 256
#endif // CCMD_PROGRAM_NAME_MAX

#ifndef CCMD_ERROR_MAX
    #define CCMD_ERROR_MAX 64
#endif // CCMD_ERROR_MAX

#define CCMD_0_OR_MORE INT32_MIN
#define CCMD_N_OR_MORE(N) ((N) <= 0 ? CCMD_0_OR_MORE : (CCMD_0_OR_MORE + (N)))

#define CCMD_ARRAY_SIZE(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define CCMD_ARRAY_VIEW_TYPE(T) struct { int32_t count; T* data; } // NOLINT(bugprone-macro-parentheses)
#define CCMD_ARRAY_VIEW(...) { CCMD_ARRAY_SIZE((__VA_ARGS__)), __VA_ARGS__ }
#define CCMD_ARRAY_VIEW_INPLACE(AV, ...) AV.count = CCMD_ARRAY_SIZE((__VA_ARGS__)); AV.data = __VA_ARGS__

struct ccmd_result;
struct ccmd_command_result;

typedef enum ccmd_status
{
    CCMD_STATUS_SUCCESS,
    CCMD_STATUS_ERROR,
    CCMD_STATUS_HELP,
    CCMD_STATUS_COUNT
} ccmd_status;

typedef struct ccmd_error
{
    uint32_t    key;
    int32_t     int32;
    char        char8;
    const char* str;
} ccmd_error;

typedef struct ccmd_positional
{
    const char* name;
    const char* help;
} ccmd_positional;

typedef struct ccmd_option
{
    char        short_name;
    const char* long_name;
    const char* help;
    int32_t     nargs;
    bool        required;
} ccmd_option;

typedef ccmd_status(*ccmd_run_callback)(const struct ccmd_command_result* program, const struct ccmd_command_result* command);

typedef struct ccmd_command
{
    const char*             name;
    const char*             help;

    CCMD_ARRAY_VIEW_TYPE(const ccmd_positional)
    positionals;

    CCMD_ARRAY_VIEW_TYPE(const ccmd_option)
    options;

    CCMD_ARRAY_VIEW_TYPE(const struct ccmd_command)
    subcommands;

    ccmd_run_callback    run;
} ccmd_command;

typedef struct ccmd_parsed_args
{
    char            short_name;
    const char*     long_name;
    char* const*    args;
    int32_t         nargs;
} ccmd_parsed_args;

typedef struct ccmd_command_result
{
    const char*             name;

    CCMD_ARRAY_VIEW_TYPE(char* const)
    positionals;
    CCMD_ARRAY_VIEW_TYPE(const ccmd_parsed_args)
    options;

    ccmd_run_callback        run;
} ccmd_command_result;

typedef struct ccmd_result
{
    char                            program_name[CCMD_PROGRAM_NAME_MAX];
    const char*                     program_path;
    const ccmd_command_result*      program_command;
    int32_t                         option_count;
    int32_t                         commands_count;
    int32_t                         error_count;

    // buffers to redirect usage/error messages
    CCMD_ARRAY_VIEW_TYPE(ccmd_error)
    errors;

    CCMD_ARRAY_VIEW_TYPE(char)
    usage;

    CCMD_ARRAY_VIEW_TYPE(ccmd_parsed_args)
    options;

    CCMD_ARRAY_VIEW_TYPE(ccmd_command_result)
    commands;
} ccmd_result;


#ifdef __cplusplus
extern "C" {
#endif

CCMD_API ccmd_status ccmd_parse(ccmd_result* result, const int32_t argc, char* const* argv, const ccmd_command* cli);

CCMD_API ccmd_status ccmd_run(const ccmd_result* program);

CCMD_API ccmd_status ccmd_run_all(const ccmd_result* program);

CCMD_API bool ccmd_has_positional(const ccmd_command_result* command, const int32_t position);

CCMD_API const char* ccmd_get_positional(const ccmd_command_result* command, const int32_t position);

CCMD_API bool ccmd_has_option(const ccmd_command_result* command, const char* long_or_short_name);

CCMD_API const ccmd_parsed_args* ccmd_get_option(const ccmd_command_result* command, const char* long_or_short_name);


#ifdef __cplusplus
}
#endif
