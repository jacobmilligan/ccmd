/*
 *  cplatform.h
 *  Collection of macros and utilities for writing cross-platform C/C++ code
 *
 *  Copyright (c) 2019 Jacob Milligan. All rights reserved.
 */

/*
 **********************
 *
 * # Build type setup
 *
 **********************
 */
#ifndef CPLATFORM_DEBUG
    #define CPLATFORM_DEBUG 0
#endif // CPLATFORM_DEBUG
#ifndef CPLATFORM_RELEASE
    #define CPLATFORM_RELEASE 0
#endif // CPLATFORM_RELEASE

/*
 ******************************************
 *
 * # String expansion and token pasting
 *
 ******************************************
 */
#define CPLATFORM_EXPAND(x) x
#define CPLATFORM_STRINGIFY(x) #x
#define CPLATFORM_CONCAT_BASE(x, y) x##y
#define CPLATFORM_CONCAT(x, y) CPLATFORM_CONCAT_BASE(x, y)

/*
 ******************************
 *
 * # OS platform detection
 *
 ******************************
 */
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
        #define CPLATFORM_OS_IOS 1
        #define CPLATFORM_OS_NAME_STRING "iOS Simulator"
    #elif TARGET_OS_IPHONE == 1
        #define CPLATFORM_OS_IOS 1
        #define CPLATFORM_OS_NAME_STRING "iOS"
    #elif TARGET_OS_MAC == 1
        #define CPLATFORM_OS_MACOS 1
        #define CPLATFORM_OS_NAME_STRING "MacOS"
    #endif // TARGET_*
#elif defined(__WIN32__) || defined(__WINDOWS__) || defined(_WIN64) || defined(_WIN32) || defined(_WINDOWS) || defined(__TOS_WIN__)
    #define CPLATFORM_OS_WINDOWS 1
    #define CPLATFORM_OS_NAME_STRING "Windows"
#elif defined(__linux__) || defined(__linux) || defined(linux_generic)
#define CPLATFORM_OS_LINUX 1
    #define CPLATFORM_OS_NAME_STRING "Linux"
#elif defined(__ANDROID__)
    #define CPLATFORM_OS_ANDROID 1
    #define CPLATFORM_ANDROID_API_LEVEL __ANDROID_API__
    #define CPLATFORM_OS_NAME_STRING "Android"
#endif // defined(<platform>)

#if CPLATFORM_OS_LINUX == 1 || CPLATFORM_OS_MACOS == 1 || CPLATFORM_OS_IOS == 1 || CPLATFORM_OS_ANDROID == 1
    #define CPLATFORM_OS_UNIX 1
#else
    #define CPLATFORM_OS_UNIX 0
#endif //

#ifndef CPLATFORM_OS_MACOS
    #define CPLATFORM_OS_MACOS 0
#endif // CPLATFORM_OS_MACOS
#ifndef CPLATFORM_OS_IOS
    #define CPLATFORM_OS_IOS 0
#endif // CPLATFORM_OS_IOS
#ifndef CPLATFORM_OS_ANDROID
    #define CPLATFORM_OS_ANDROID 0
#endif // CPLATFORM_OS_ANDROID
#ifndef CPLATFORM_OS_WINDOWS
    #define CPLATFORM_OS_WINDOWS 0
#endif // CPLATFORM_OS_WINDOWS
#ifndef CPLATFORM_OS_LINUX
    #define CPLATFORM_OS_LINUX 0
#endif // CPLATFORM_OS_LINUX

#ifndef CPLATFORM_OS_NAME_STRING
    #define CPLATFORM_OS_NAME_STRING "UNKNOWN_OS"
#endif // ifndef CPLATFORM_OS_NAME_STRING

/*
 ******************************
 *
 * # Compiler detection
 *
 ******************************
 */
#if defined(__clang__)
    #define CPLATFORM_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define CPLATFORM_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define CPLATFORM_COMPILER_MSVC 1
#endif // defined(_MSC_VER)

#ifndef CPLATFORM_COMPILER_CLANG
    #define CPLATFORM_COMPILER_CLANG 0
#endif // CPLATFORM_COMPILER_CLANG
#ifndef CPLATFORM_COMPILER_GCC
    #define CPLATFORM_COMPILER_GCC 0
#endif // CPLATFORM_COMPILER_GCC
#ifndef CPLATFORM_COMPILER_MSVC
    #define CPLATFORM_COMPILER_MSVC 0
#endif // CPLATFORM_COMPILER_MSVC

#if CPLATFORM_COMPILER_CLANG == 0  && CPLATFORM_COMPILER_GCC == 0 && CPLATFORM_COMPILER_MSVC == 0
    #define CPLATFORM_COMPILER_UNKNOWN 1
#else
    #define CPLATFORM_COMPILER_UNKNOWN 0
#endif // CPLATFORM_COMPILER_CLANG == 0 || CPLATFORM_COMPILER_GCC == 0 || CPLATFORM_COMPILER_MSVC == 0

// Clang and GCC shared definitions
#if CPLATFORM_COMPILER_CLANG == 1 || CPLATFORM_COMPILER_GCC == 1
#define CPLATFORM_PACKED(n)                       __attribute__((packed, aligned(n)))
    #define CPLATFORM_FUNCTION_NAME                   __PRETTY_FUNCTION__
    #define CPLATFORM_FORCE_INLINE inline             __attribute__((always_inline))
    #define CPLATFORM_PRINTFLIKE(fmt, firstvararg)    __attribute__((__format__ (__printf__, fmt, firstvararg)))
    #define CPLATFORM_LIKELY(statement)               __builtin_expect((statement), 1)
    #define CPLATFORM_UNLIKELY(statement)             __builtin_expect((statement), 0)
    #define CPLATFORM_EXPORT_SYMBOL                   __attribute__ ((visibility("default")))
#endif // CPLATFORM_COMPILER_CLANG || CPLATFORM_COMPILER_GCC

// Compiler-specific non-shared definitions
#if CPLATFORM_COMPILER_CLANG == 1
#define CPLATFORM_PUSH_WARNING                _Pragma("clang diagnostic push")
    #define CPLATFORM_POP_WARNING                 _Pragma("clang diagnostic pop")
    #define CPLATFORM_DISABLE_WARNING_CLANG(W)    _Pragma(CPLATFORM_STRINGIFY(clang diagnostic ignored W))
#elif CPLATFORM_COMPILER_GCC == 1
#define CPLATFORM_PUSH_WARNING                _Pragma("GCC diagnostic push")
    #define CPLATFORM_POP_WARNING                 _Pragma("GCC diagnostic pop")
    #define CPLATFORM_DISABLE_WARNING_GCC(W)      _Pragma(CPLATFORM_STRINGIFY(GCC diagnostic ignored # W))
#elif CPLATFORM_COMPILER_MSVC == 1
// Displays type information, calling signature etc. much like __PRETTY_FUNCTION__
// see: https://msdn.microsoft.com/en-us/library/b0084kay.aspx
    #define CPLATFORM_FUNCTION_NAME                   __FUNCSIG__
    #define CPLATFORM_FORCE_INLINE                    __forceinline
    #define CPLATFORM_PRINTFLIKE(fmt, firstvararg)
    #define CPLATFORM_PUSH_WARNING                    __pragma(warning( push ))
    #define CPLATFORM_DISABLE_WARNING_MSVC(w)         __pragma(warning( disable: w ))
    #define CPLATFORM_POP_WARNING                     __pragma(warning( pop ))
    #define CPLATFORM_DISABLE_WARNING_CLANG(w)
    #define CPLATFORM_LIKELY(statement)               (statement)
    #define CPLATFORM_UNLIKELY(statement)             (statement)
    #if _WIN64
    #else
        #define CPLATFORM_ARCH_32BIT
    #endif // _WIN64
#endif // CPLATFORM_COMPILER_*

#ifndef CPLATFORM_DISABLE_WARNING_CLANG
    #define CPLATFORM_DISABLE_WARNING_CLANG(W)
#endif // CPLATFORM_DISABLE_WARNING_CLANG
#ifndef CPLATFORM_DISABLE_WARNING_GCC
    #define CPLATFORM_DISABLE_WARNING_GCC(W)
#endif // CPLATFORM_DISABLE_WARNING_GCC
#ifndef CPLATFORM_DISABLE_WARNING_MSVC
    #define CPLATFORM_DISABLE_WARNING_MSVC(W)
#endif // CPLATFORM_DISABLE_WARNING_MSVC

// Define dllexport/import for windows if compiler is MSVC or Clang (supported in clang since at least v3.5)
#if CPLATFORM_OS_WINDOWS == 1 && CPLATFORM_COMPILER_GCC == 0
    #ifdef CPLATFORM_DLL
#undef CPLATFORM_EXPORT_SYMBOL
        #undef CPLATFORM_IMPORT_SYMBOL
        #define CPLATFORM_EXPORT_SYMBOL   __declspec(dllexport)
        #define CPLATFORM_IMPORT_SYMBOL   __declspec(dllimport)
    #endif // CPLATFORM_DLL
#endif

#ifndef CPLATFORM_EXPORT_SYMBOL
    #define CPLATFORM_EXPORT_SYMBOL
#endif // CPLATFORM_EXPORT_SYMBOL
#ifndef CPLATFORM_IMPORT_SYMBOL
    #define CPLATFORM_IMPORT_SYMBOL
#endif // CPLATFORM_IMPORT_SYMBOL

#if defined(__OBJC__)
    #define CPLATFORM_OBJC_RELEASE(object) [object release], (object) = nil
#else
    #define CPLATFORM_OBJC_RELEASE
#endif // defined(__OBJC__)

/*
 **********************************************************************************************************************
 *
 * # Processor architecture
 *
 * this only works for x86_64 currently.
 * see: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
 *
 **********************************************************************************************************************
 */
#if defined(__x86_64__) || defined(_M_X64)
    #define CPLATFORM_ARCH_64BIT 1
    #define CPLATFORM_ARCH_BITS 64
#else
#define CPLATFORM_ARCH_32BIT 1
    #define CPLATFORM_ARCH_BITS 32
#endif // Processor arch

/*
 ********************************************************
 *
 * # Endianness
 * Only present if supplied by build system so define
 * as little endian by default if missing
 *
 *******************************************************
 */
#if !defined(CPLATFORM_BIG_ENDIAN) && !defined(CPLATFORM_LITTLE_ENDIAN)
    #define CPLATFORM_LITTLE_ENDIAN
#endif // CPLATFORM_LITTLE_ENDIAN


/*
 **************************************************
 *
 * # Assertion configuration
 * allow user to force assertions on if needed
 *
 **************************************************
 */
#if CPLATFORM_FORCE_ASSERTIONS_ENABLED == 1
    #define CPLATFORM_ENABLE_ASSERTIONS CPLATFORM_FORCE_ASSERTIONS_ENABLED
#else
    #if CPLATFORM_DEBUG == 1
        #define CPLATFORM_ENABLE_ASSERTIONS 1
    #else
        #define CPLATFORM_ENABLE_ASSERTIONS 0
    #endif // CPLATFORM_DEBUG == 1
#endif // !defined(CPLATFORM_FORCE_ASSERTIONS_ENABLED)

/*
 **************************************************
 *
 * # Utilities
 * these are concise and reused enough across
 * different files that they warrant being added
 * to config.h
 *
 **************************************************
 */
#define CPLATFORM_BEGIN_MACRO_BLOCK do {
#define CPLATFORM_END_MACRO_BLOCK } while (false)
#define CPLATFORM_EXPLICIT_SCOPE(x) do { x } while (false)

#define CPLATFORM_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define CPLATFORM_MAX(X, Y) ((X) >= (Y) ? (X) : (Y))

#define CPLATFORM_ROUND_UP(SIZE, ALIGNMENT) (((SIZE) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#define CPLATFORM_UNUSED(x) (void)(x)

// Allows using NOLINT for clang-tidy inside macros
#define CPLATFORM_NOLINT(...) __VA_ARGS__ // NOLINT

#if CPLATFORM_OS_WINDOWS == 1
    #define CPLATFORM_PATH_SEPARATOR '\\'
#else
    #define CPLATFORM_PATH_SEPARATOR '/'
#endif // CPLATFORM_OS_WINDOWS == 1

// Macros for working with explicit struct padding
#ifdef __cplusplus
    #define CPLATFORM_PAD(N) char CPLATFORM_CONCAT(padding, __LINE__)[N] { 0 }
#else
    #define CPLATFORM_PAD(N) char CPLATFORM_CONCAT(padding, __LINE__)[N]
#endif // __cplusplus

#define CPLATFORM_DISABLE_PADDING_WARNINGS CPLATFORM_DISABLE_WARNING_MSVC(4121 4820)

/*
 **********************************************************************************
 *
 * # CPLATFORM_ALLOCA & CPLATFORM_ALLOCA_ARRAY
 *
 * Portable alloca implementations - CPLATFORM_ALLOCA_ARRAY
 * calls CPLATFORM_ALLOCA with size==sizeof(T)*count and alignment==alignof(T)
 *
 **********************************************************************************
 */
// Must be a macro because even a FORCE_INLINE function may possibly free the memory
#if CPLATFORM_OS_WINDOWS == 1
    #define CPLATFORM_ALLOCA(SIZE, ALIGNMENT) (_alloca(CPLATFORM_ROUND_UP(SIZE, ALIGNMENT)))
#else
    #define CPLATFORM_ALLOCA(SIZE, ALIGNMENT) (alloca(CPLATFORM_ROUND_UP(SIZE, ALIGNMENT)))
#endif // CPLATFORM_OS_WINDOWS == 1

#ifdef __cplusplus
    #define CPLATFORM_ALLOCA_ARRAY(T, SIZE) (static_cast<T*>(CPLATFORM_ALLOCA(sizeof(T) * SIZE, alignof(T))))
#else
    #define CPLATFORM_ALLOCA_ARRAY(T, SIZE) ((T*)CPLATFORM_ALLOCA(sizeof(T) * (SIZE), _Alignof(T)))
#endif // __cplusplus

/*
 **************
 *
 * C++ utils
 *
 **************
 */
#ifdef __cplusplus
/*
 * # CPLATFORM_MOVE
 *
 * this is a replacement for std::forward used to avoid having to include <utility> (which also includes <type_traits>)
 */
#define CPLATFORM_FORWARD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace ccmd {
    #ifndef CPLATFORM_STATIC_ARRAY_LENGTH_INLINE
        #define CPLATFORM_STATIC_ARRAY_LENGTH_INLINE
        template <typename T, int Size>
        inline constexpr int static_array_length(T(&)[Size])
        {
            return Size;
        }
    #endif // CPLATFORM_STATIC_ARRAY_LENGTH_INLINE
} // namespace ccmd
#endif


/*
 *  cmd.c
 *  ccmd
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#ifndef CCMD_IMPLEMENTATION
    #define CCMD_IMPLEMENTATION
#endif // CCMD_IMPLEMENTATION

#include "ccmd.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#define CCMD_HELP_MIN_COLS 16
#define CCMD_ERROR_KEY_CATEGORY(KEY) ((KEY) & ((1 << 16) - 1))
#define CCMD_ERROR_KEY_ARG_TYPE(KEY) ((KEY) >> 16)
#define CCMD_ERROR_KEY(CATEGORY, ARG_TYPE) ((CATEGORY) | ((ARG_TYPE) << 16))

typedef enum ccmd_error_category
{
    CCMD_ERROR_CATEGORY_UNKNOWN,
    CCMD_ERROR_CATEGORY_INVALID_NARGS,
    CCMD_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT,
    CCMD_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT,
    CCMD_ERROR_CATEGORY_INTERNAL,
    CCMD_ERROR_CATEGORY_COUNT
} ccmd_error_category;

typedef enum ccmd_argument_type
{
    CCMD_ARGUMENT_OPTION,
    CCMD_ARGUMENT_OPTION_N_OR_MORE,
    CCMD_ARGUMENT_POSITIONAL,
    CCMD_ARGUMENT_SUBCOMMAND,
    CCMD_ARGUMENT_INVALID
} ccmd_argument_type;

typedef enum ccmd_token_type
{
    CCMD_TOKEN_SHORT_OPTION,
    CCMD_TOKEN_LONG_OPTION,
    CCMD_TOKEN_POSITIONAL,
    CCMD_TOKEN_SUBCOMMAND,
    CCMD_TOKEN_DELIMITER,
    CCMD_TOKEN_INVALID
} ccmd_token_type;

typedef struct ccmd_token
{
    ccmd_token_type  type;
    const char*     value;
    int32_t         length;
} ccmd_token;

typedef struct ccmd_formatter
{
    int32_t                     buffer_capacity;
    int32_t                     length;
    char*                       buffer;
} ccmd_formatter;

typedef struct ccmd_parser
{
    const ccmd_command**     command_infos;
    ccmd_command_result*     command_result;
    ccmd_result*             program_result;
} ccmd_parser;


/*
 **************************
 *
 * Utils API
 *
 **************************
 */
int ccmd_option_display_length(const ccmd_option* opt)
{
    int size = 2; // 2 extra for '--'
    if (opt->long_name != NULL)
    {
        size += (int)strlen(opt->long_name);
    }
    if (opt->short_name != '\0')
    {
        size += 4; // 3 extra for the leading '-' and trailing ', '
    }
    return size;
}

ccmd_parsed_args* add_option(ccmd_parser* parser)
{
    const int index = parser->program_result->option_count;
    assert(index < parser->program_result->options.count - 1);

    ++parser->program_result->option_count;
    ++parser->command_result->options.count;

    if (parser->command_result->options.data == NULL)
    {
        parser->command_result->options.data = &parser->program_result->options.data[index];
    }

    return &parser->program_result->options.data[index];
}

void ccmd_add_error(ccmd_result* result, const ccmd_error_category category, const enum ccmd_argument_type arg_type, const char char8, const char* str, const int32_t int32)
{
    if (result->error_count >= result->errors.count)
    {
        fprintf(stderr, "too many errors were generated\n");
        return;
    }

    ccmd_error* stored = &result->errors.data[result->error_count++];
    stored->key = CCMD_ERROR_KEY(category, arg_type);
    stored->char8 = char8;
    stored->str = str;
    stored->int32 = int32;
}

void ccmd_remove_error(ccmd_result* result, const ccmd_error_category category, const enum ccmd_argument_type arg_type, const char char8, const char* str)
{
    const uint32_t key = CCMD_ERROR_KEY(category, arg_type);
    for (int i = 0; i < result->error_count; ++i)
    {
        const ccmd_error* error = &result->errors.data[i];
        if (error->key != key
            || error->char8 != char8
            || (str == NULL) != (error->str == NULL)
            || strcmp(error->str, str) != 0)
        {
            continue;
        }

        // found the error - swap the last with this error and decrement
        if (i < result->error_count - 1)
        {
            memcpy(&result->errors.data[i], &result->errors.data[result->error_count - 1], sizeof(ccmd_error));
        }

        --result->error_count;
        break;
    }
}

/*
 **************************
 *
 * String formatter API
 *
 **************************
 */
int ccmd_fmt_vsnprintf(struct ccmd_formatter* formatter, const char* format, va_list args)
{
    char* dst = formatter->buffer + formatter->length;
    const int dst_size = formatter->buffer_capacity - formatter->length;

    int count = vsnprintf(dst, dst_size, format, args);
    if (count > 0)
    {
        formatter->length += count;
    }

    return count;
}

int ccmd_fmt(struct ccmd_formatter* formatter, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int count = ccmd_fmt_vsnprintf(formatter, format, args);
    va_end(args);
    return count;
}

int ccmd_fmt_putc(struct ccmd_formatter* formatter, const char c)
{
    if (formatter->length >= formatter->buffer_capacity)
    {
        return 0;
    }

    formatter->buffer[formatter->length++] = c;
    if (formatter->length < formatter->buffer_capacity)
    {
        formatter->buffer[formatter->length] = '\0';
    }
    return 1;
}

int ccmd_fmt_puts(struct ccmd_formatter* formatter, const char* string)
{
    if (string == NULL)
    {
        return 0;
    }

    const char* ptr = string;
    while (*ptr != '\0' && formatter->length < formatter->buffer_capacity)
    {
        formatter->buffer[formatter->length] = *ptr;
        ++formatter->length;
        ++ptr;
    }

    if (formatter->length < formatter->buffer_capacity)
    {
        formatter->buffer[formatter->length] = '\0';
    }

    return (int)(ptr - string);
}

int ccmd_fmt_put_option_name(struct ccmd_formatter* formatter, const char short_name, const char* long_name)
{
    int count = 0;

    if (short_name != '\0')
    {
        count += ccmd_fmt_putc(formatter, '-');
        count += ccmd_fmt_putc(formatter, short_name);
    }
    if (short_name != '\0' && long_name != NULL)
    {
        count += ccmd_fmt_putc(formatter, '/');
    }
    if (long_name != NULL)
    {
        count += ccmd_fmt_puts(formatter, "--");
        count += ccmd_fmt_puts(formatter, long_name);
    }

    return count;
}

int ccmd_fmt_spaces_base(struct ccmd_formatter* formatter, const int column_size, const int label_length)
{
    const int spaces = CPLATFORM_MIN(column_size - label_length, formatter->buffer_capacity - formatter->length);
    memset(formatter->buffer + formatter->length, ' ', spaces);
    formatter->length += spaces;
    formatter->buffer[formatter->length] = '\0';
    return spaces;
}

int ccmd_fmt_spaces_arg(struct ccmd_formatter* formatter, const int column_size, const char* name)
{
    return ccmd_fmt_spaces_base(formatter, column_size, (int)strlen(name));
}

int ccmd_fmt_spaces_opt(struct ccmd_formatter* formatter, const int column_size, const ccmd_option* option)
{
    return ccmd_fmt_spaces_base(formatter, column_size, ccmd_option_display_length(option));
}

static int ccmd_qsort_error_comp(const void* lhs, const void* rhs)
{
    const ccmd_error* lhs_error = (const ccmd_error*)lhs;
    const ccmd_error* rhs_error = (const ccmd_error*)rhs;
    return (int)CCMD_ERROR_KEY_CATEGORY(lhs_error->key) - (int)CCMD_ERROR_KEY_CATEGORY(rhs_error->key);
}

int ccmd_default_error_report(const char* program_name, ccmd_formatter* formatter, ccmd_result* result)
{
    static const char* fmt_token_name[CCMD_ARGUMENT_INVALID + 1] = {
        "option",       // CCMD_ARGUMENT_OPTION
        "option",       // CCMD_ARGUMENT_OPTION_N_OR_MORE
        "positional",   // CCMD_ARGUMENT_POSITIONAL
        "subcommand",   // CCMD_ARGUMENT_SUBCOMMAND
        "<#INVALID>",   // CCMD_ARGUMENT_INVALID
    };

    // sort into error categories
    qsort(result->errors.data, result->error_count, sizeof(const ccmd_error), ccmd_qsort_error_comp);

    const int old_formatter_length = formatter->length;
    int category_progress[CCMD_ERROR_CATEGORY_COUNT][2] = { { 0, 0 } };
    for (int i = 0; i < result->error_count; ++i)
    {
        ++category_progress[CCMD_ERROR_KEY_CATEGORY(result->errors.data[i].key)][1];
    }

    for (int i = 0; i < result->error_count; ++i)
    {
        const ccmd_error* error = &result->errors.data[i];
        const ccmd_error_category category = CCMD_ERROR_KEY_CATEGORY(error->key);
        const ccmd_argument_type arg_type = CCMD_ERROR_KEY_ARG_TYPE(error->key);
        const int index_in_category = category_progress[category][0];
        const int last_index_in_category = category_progress[category][1] - 1;

        category_progress[category][0]++;

        switch (category)
        {
            case CCMD_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT:
            {
                if (index_in_category == 0)
                {
                    ccmd_fmt(formatter, "%s: error: the following arguments are required: ", program_name);
                }

                if (arg_type == CCMD_TOKEN_SHORT_OPTION || arg_type == CCMD_TOKEN_LONG_OPTION)
                {
                    ccmd_fmt_put_option_name(formatter, error->char8, error->str);
                }
                else
                {
                    ccmd_fmt_puts(formatter, error->str);
                }

                ccmd_fmt_puts(formatter, index_in_category < last_index_in_category ? ", " : "\n");
                break;
            }
            case CCMD_ERROR_CATEGORY_INVALID_NARGS:
            {
                ccmd_fmt(formatter, "%s: error: option ", program_name);
                ccmd_fmt_put_option_name(formatter, error->char8, error->str);

                if (arg_type == CCMD_ARGUMENT_OPTION_N_OR_MORE)
                {
                    ccmd_fmt(formatter, " expected at least %d argument", error->int32);
                }
                else
                {
                    ccmd_fmt(formatter, " expected %d argument", error->int32);
                }

                if (error->int32 > 1)
                {
                    ccmd_fmt_putc(formatter, 's'); // plural nargs
                }

                ccmd_fmt_putc(formatter, '\n');
                break;
            }
            case CCMD_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT:
            {
                ccmd_fmt(formatter, "%s: error; unrecognized %s: %s\n", program_name, fmt_token_name[arg_type], error->str);
                break;
            }
            case CCMD_ERROR_CATEGORY_INTERNAL:
            {
                ccmd_fmt(formatter, "%s: error: internal error - %s\n", program_name, error->str);
                break;
            }
            default:
            {
                fprintf(stderr, "%s, invalid error type: %d\n", program_name, arg_type);
                exit(1);
            }
        }
    }

    return formatter->length - old_formatter_length;
}

/*
 *****************************
 *
 * Generate a usage message
 * from the command spec
 *
 *****************************
 */
void ccmd_generate_usage(ccmd_formatter* usage_formatter, const int32_t command_count, const ccmd_command* const* commands)
{
    ccmd_fmt(usage_formatter, "usage: ");

    const ccmd_command* executed_command = commands[command_count - 1];
    int help_spacing = 0;

    for (int i = 0; i < command_count; ++i)
    {
        const ccmd_command* command = commands[i];

        if (command->name != NULL)
        {
            ccmd_fmt(usage_formatter, "%s ", command->name);
        }

        if (command->options.count > 0)
        {
            // Print out all the required options
            for (int opt_idx = 0; opt_idx < command->options.count; ++opt_idx)
            {
                if (command->options.data[opt_idx].required)
                {
                    const char* long_name = command->options.data[opt_idx].long_name;
                    const int nargs = command->options.data[opt_idx].nargs;
                    ccmd_fmt(usage_formatter, "--%s %s", long_name, nargs != 0 ? "ARGS " : "");
                }

                if (command == executed_command)
                {
                    // calculate max spacing for help strings
                    help_spacing = CPLATFORM_MAX(help_spacing, ccmd_option_display_length(&command->options.data[opt_idx]));
                }
            }

            ccmd_fmt(usage_formatter, "[options...] ");
        }

        if (command->positionals.count > 0)
        {
            for (int pos_idx = 0; pos_idx < command->positionals.count; ++pos_idx)
            {
                // print out positionals with specific amount of spacing, i.e
                // `program positional1 positional2 ...`
                ccmd_fmt(usage_formatter, "%s ", command->positionals.data[pos_idx].name);

                if (command == executed_command)
                {
                    // calculate max spacing for help strings
                    help_spacing = CPLATFORM_MAX(help_spacing, (int)strlen(command->positionals.data[pos_idx].name));
                }
            }
        }

        if (command == executed_command)
        {
            if (command->subcommands.count > 0)
            {
                ccmd_fmt(usage_formatter, "<command> ");
                for (int sc = 0; sc < command->subcommands.count; ++sc)
                {
                    // calculate max spacing for help strings
                    help_spacing = CPLATFORM_MAX(help_spacing, (int)strlen(command->subcommands.data[sc].name));
                }
            }
        }
    }

    if (executed_command->help != NULL)
    {
        ccmd_fmt(usage_formatter, "\n\n%s", executed_command->help);
    }

    help_spacing = CPLATFORM_MAX(CCMD_HELP_MIN_COLS, help_spacing + 4); // at least 4 spaces between the arg and the desc

    // output the positional args
    if (executed_command->positionals.count > 0)
    {
        ccmd_fmt(usage_formatter, "\n\nArguments:\n");

        // print out positionals with specific amount of spacing, i.e `positional1  help string`
        for (int pos_idx = 0; pos_idx < executed_command->positionals.count; ++pos_idx)
        {
            ccmd_fmt(usage_formatter, "  %s", executed_command->positionals.data[pos_idx].name);
            ccmd_fmt_spaces_arg(usage_formatter, help_spacing, executed_command->positionals.data[pos_idx].name);
            ccmd_fmt(usage_formatter, "%s\n", executed_command->positionals.data[pos_idx].help);
        }
    }

    ccmd_fmt(usage_formatter, "\nOptions:\n  -h, --help");
    ccmd_fmt_spaces_arg(usage_formatter, help_spacing, "-h, --help");
    ccmd_fmt(usage_formatter, "Returns this help message\n");

    if (executed_command->options.count > 0)
    {
        // print out options, i.e `-o, --option1  help string`
        for (int opt_idx = 0; opt_idx < executed_command->options.count; ++opt_idx)
        {
            ccmd_fmt(usage_formatter, "  ");

            // Write out short name
            if (executed_command->options.data[opt_idx].short_name != '\0')
            {
                ccmd_fmt(usage_formatter, "-%c, ", executed_command->options.data[opt_idx].short_name);
            }

            // Write long name
            ccmd_fmt(usage_formatter, "--%s", executed_command->options.data[opt_idx].long_name);
            ccmd_fmt_spaces_opt(usage_formatter, help_spacing, &executed_command->options.data[opt_idx]);
            // Write help string
            ccmd_fmt(usage_formatter, "%s\n", executed_command->options.data[opt_idx].help);
        }
    }

    // generate just a list of names for subcommands
    if (executed_command->subcommands.count > 0)
    {
        ccmd_fmt(usage_formatter, "\nCommands:\n  ");

        for (int cmd_idx = 0; cmd_idx < executed_command->subcommands.count; ++cmd_idx)
        {
            ccmd_fmt(usage_formatter, "%s", executed_command->subcommands.data[cmd_idx].name);

            if (cmd_idx < executed_command->subcommands.count - 1)
            {
                ccmd_fmt(usage_formatter, ", ");
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
ccmd_token ccmd_parse_element(const ccmd_parser* parser, const char* arg)
{
    if (arg == NULL)
    {
        return (ccmd_token) { .type = CCMD_TOKEN_INVALID };
    }

    int leading_dashes = 0;
    int length = 0;
    for (; arg[length] != '\0'; ++length)
    {
        if (arg[length] == '-' && length == leading_dashes)
        {
            ++leading_dashes;
        }
    }

    if (length == 0)
    {
        return (ccmd_token) { .type = CCMD_TOKEN_INVALID };
    }

    if (leading_dashes == 1)
    {
        return (ccmd_token) { .type = CCMD_TOKEN_SHORT_OPTION, .value = arg + 1, .length = 1 };
    }

    if (leading_dashes == 2)
    {
        // A solitary '--' argument indicates the command line should stop parsing
        const ccmd_token_type type = length == 2 ? CCMD_TOKEN_DELIMITER : CCMD_TOKEN_LONG_OPTION;
        return (ccmd_token) { .type = type, .value = arg + 2, .length = length - 2 };
    }

    const ccmd_command_result* command_result = parser->command_result;
    const ccmd_command* command_info = parser->command_infos[parser->program_result->commands_count - 1];
    const ccmd_command_result* subcommand = &parser->program_result->commands.data[parser->program_result->commands_count - 1];
    const bool all_positionals_parsed = command_result->positionals.count >= command_info->positionals.count;
    const bool has_parsed_subcommands = subcommand > parser->command_result;

    return (ccmd_token) {
        .type = (all_positionals_parsed && !has_parsed_subcommands)  ? CCMD_TOKEN_SUBCOMMAND : CCMD_TOKEN_POSITIONAL,
        .value = arg,
        .length = length
    };
}

bool ccmd_compare_option(const ccmd_token* element, const char expected_short_name, const char* expected_long_name)
{
    if (element->type != CCMD_TOKEN_SHORT_OPTION && element->type != CCMD_TOKEN_LONG_OPTION)
    {
        return false;
    }

    if (element->length == 1 && element->value[0] == expected_short_name)
    {
        return true;
    }

    return strncmp(expected_long_name, element->value, element->length) == 0;
}

int ccmd_find_option(const ccmd_command* command, const ccmd_token* element)
{
    for (int i = 0; i < command->options.count; ++i)
    {
        if (ccmd_compare_option(element, command->options.data[i].short_name, command->options.data[i].long_name))
        {
            return i;
        }
    }

    return -1;
}

ccmd_status ccmd_parse_command(const int argc, char* const* argv, ccmd_parser* parser)
{
    assert(parser->program_result->commands_count < parser->program_result->commands.count);

    // get command info
    const ccmd_command* command_info = parser->command_infos[parser->program_result->commands_count - 1];
    ccmd_command_result* command_result = parser->command_result;

    // setup parser
    const int max_errors = parser->program_result->errors.count;
    const ccmd_error* errors = parser->program_result->errors.data;
    for (int i = 0; i < command_info->positionals.count && parser->program_result->error_count < max_errors; ++i)
    {
        ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CCMD_ARGUMENT_POSITIONAL,
            '\0', command_info->positionals.data[i].name, 1
        );
    }
    for (int i = 0; i < command_info->options.count && parser->program_result->error_count < max_errors; ++i)
    {
        if (!command_info->options.data[i].required)
        {
            continue;
        }

        ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CCMD_ARGUMENT_OPTION,
            command_info->options.data[i].short_name,
            command_info->options.data[i].long_name,
            command_info->options.data[i].nargs
        );
    }

    // setup command defaults - name and run callback
    memset(command_result, 0, sizeof(ccmd_command_result));
    if (command_info->name != NULL)
    {
        command_result->name = command_info->name;
    }

    if (command_info->run != NULL)
    {
        command_result->run = command_info->run;
    }

    int nargs_parsed = 0;

    while (nargs_parsed < argc)
    {
        char* const* argv_slice = argv + nargs_parsed;
        const ccmd_token token = ccmd_parse_element(parser, argv[nargs_parsed++]);

        switch (token.type)
        {
            case CCMD_TOKEN_INVALID:
            {
                // very dodgy - something went terribly wrong
                ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_INTERNAL, CCMD_ARGUMENT_INVALID,
                    '\0', "invalid argument string detected", 0
                );
                return CCMD_STATUS_ERROR;
            }
            case CCMD_TOKEN_DELIMITER:
            {
                // detected ' -- ' : game over, man
                return CCMD_STATUS_SUCCESS;
            }
            case CCMD_TOKEN_SHORT_OPTION:
            case CCMD_TOKEN_LONG_OPTION:
            {
                // exit early and show help if this is the implicit -h/--help option
                if (ccmd_compare_option(&token, 'h', "help"))
                {
                    return CCMD_STATUS_HELP;
                }

                // find the given option and validate if exists
                const int option_index = ccmd_find_option(command_info, &token);

                if (option_index < 0)
                {
                    ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT, CCMD_ARGUMENT_OPTION,
                        '\0', token.value, 0
                    );
                    return CCMD_STATUS_ERROR;
                }

                // parse all the arguments for the option
                const ccmd_option* option_info = &command_info->options.data[option_index];

                // Remove from list of missing required options
                if (option_info->required)
                {
                    ccmd_remove_error(parser->program_result, CCMD_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CCMD_ARGUMENT_OPTION, option_info->short_name, option_info->long_name);
                }

                const bool is_n_or_more = option_info->nargs < 0;
                const int argc_remaining = argc - nargs_parsed;
                const int min_nargs = is_n_or_more ? option_info->nargs - CCMD_0_OR_MORE : option_info->nargs;
                const int max_nargs = is_n_or_more ? argc_remaining : option_info->nargs;
                const int option_args_begin = nargs_parsed;

                // special narg range [1, argc]
                for (int i = 0; i < CPLATFORM_MIN(max_nargs, argc_remaining); ++i)
                {
                    // we can't just verify argc we have to actually search through for the next '-/--'
                    if (*(argv[nargs_parsed]) == '-')
                    {
                        break;
                    }
                    ++nargs_parsed;
                }

                if (nargs_parsed - option_args_begin < min_nargs)
                {
                    ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_INVALID_NARGS, CCMD_ARGUMENT_OPTION_N_OR_MORE,
                        option_info->short_name,
                        option_info->long_name,
                        min_nargs
                    );
                    return CCMD_STATUS_ERROR;
                }

                if (!is_n_or_more && nargs_parsed < max_nargs)
                {
                    ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_INVALID_NARGS, CCMD_ARGUMENT_OPTION,
                        option_info->short_name,
                        option_info->long_name,
                        option_info->nargs
                    );
                    return CCMD_STATUS_ERROR;
                }

                // option parse success - add a new parsed one
                ccmd_parsed_args* option_result = add_option(parser);
                option_result->long_name = option_info->long_name;
                option_result->short_name = option_info->short_name;
                option_result->nargs = nargs_parsed - option_args_begin;
                option_result->args = option_info->nargs != 0 ? &(argv[option_args_begin]) : NULL;
                break;
            }
            case CCMD_TOKEN_POSITIONAL:
            {

                // just add this to the positional array
                const int position = parser->command_result->positionals.count++;
                if (parser->command_result->positionals.data == NULL)
                {
                    parser->command_result->positionals.data = argv_slice;
                }

                // mark required positional as parsed
                const char* positional_name = command_info->positionals.data[position].name;
                ccmd_remove_error(parser->program_result, CCMD_ERROR_CATEGORY_MISSING_REQUIRED_ARGUMENT, CCMD_ARGUMENT_POSITIONAL, '\0', positional_name);
                break;
            }
            case CCMD_TOKEN_SUBCOMMAND:
            {
                // if all the positionals have been parsed then this is either a subcommand or otherwise it's invalid
                const ccmd_command* subcommand_info = NULL;
                for (int i = 0; i < command_info->subcommands.count; ++i)
                {
                    if (strncmp(token.value, command_info->subcommands.data[i].name, token.length) == 0)
                    {
                        subcommand_info = &command_info->subcommands.data[i];
                        break;
                    }
                }

                // invalid - no such command
                if (subcommand_info == NULL)
                {
                    ccmd_add_error(parser->program_result, CCMD_ERROR_CATEGORY_UNRECOGNIZED_ARGUMENT, CCMD_ARGUMENT_SUBCOMMAND,
                        '\0', token.value, 0
                    );
                }

                // ensure all required options were found before moving to a subparser
                if (parser->program_result->error_count > 0)
                {
                    return CCMD_STATUS_ERROR;
                }

                // valid subcommand - recursively parse. Since positionals take precedence over subcommands this is fine
                assert(parser->program_result->commands_count < parser->program_result->commands.count);

                // setup the parser for the next recursive subcommand parse call
                parser->command_infos[parser->program_result->commands_count] = subcommand_info;
                parser->command_result = &parser->program_result->commands.data[parser->program_result->commands_count];
                ++parser->program_result->commands_count;

                // parse the subcommand
                const int subcommand_argc = argc - nargs_parsed;
                char* const* subcommand_argv = argv + nargs_parsed;
                return ccmd_parse_command(subcommand_argc, subcommand_argv, parser);
            }
        }
    }

    return parser->program_result->error_count > 0 ? CCMD_STATUS_ERROR : CCMD_STATUS_SUCCESS;
}

static int ccmd_count_subcommands(const ccmd_command* command)
{
    int count = 1 + command->subcommands.count; // self
    if (command->subcommands.count > 0)
    {
        for (int i = 0; i < command->subcommands.count; ++i)
        {
            count += ccmd_count_subcommands(&command->subcommands.data[i]);
        }
    }
    return count;
}

/*
 *****************************
 *
 * Public API implementations
 *
 *****************************
 */
ccmd_status ccmd_parse(ccmd_result* result, const int32_t argc, char* const* argv, const ccmd_command* cli)
{
    if (result->commands.data == NULL || result->commands.count <= 0)
    {
        fprintf(stderr, "the `result->commands` array view is NULL or invalid. This must be assigned before calling ccmd_parse\n");
        exit(1);
    }

    if (result->options.data == NULL || result->options.count <= 0)
    {
        fprintf(stderr, "the `result->options` array view is NULL or invalid. This must be assigned before calling ccmd_parse\n");
        exit(1);
    }

    // defaults for the result
    memset(result->program_name, 0, CCMD_PROGRAM_NAME_MAX);
    result->program_path = "";
    result->program_command = NULL;
    result->option_count = 0;
    result->commands_count = 0;
    result->error_count = 0;

    int subcommand_argc = argc;
    char* const* subcommand_argv = argv;

    // Check if empty args -- this should only occur with an adhoc command line, not one generated by the OS
    if (argc > 0)
    {
        assert(argv[0] != NULL);

        // Otherwise it's a normal command line
        result->program_path = argv[0];

        // adjust command args to skip program name
        subcommand_argc = argc - 1;
        subcommand_argv = argv + 1;
    }

    // use the explicitly-specified name if one was provided
    if (result->program_path != NULL)
    {
        // otherwise try and discover the program name from the program path if there are any args
        const int program_path_len = (int)strlen(result->program_path);

        // get the filename from the program path (i.e. program.exe)
        const char* exe_name = result->program_path;
        for (int i = 0; i < program_path_len; ++i)
        {
            if (result->program_path[i] != CPLATFORM_PATH_SEPARATOR || i >= program_path_len - 1)
            {
                continue;
            }

            exe_name = &result->program_path[i + 1];
        }

        // assign the program name and then try and remove the extension if one exists
CPLATFORM_PUSH_WARNING
        CPLATFORM_DISABLE_WARNING_GCC(-Wstringop-overflow)
        const int full_exe_name = CPLATFORM_MIN(strlen(exe_name), CCMD_PROGRAM_NAME_MAX);
        strncpy(result->program_name, exe_name, full_exe_name);
CPLATFORM_POP_WARNING

        // get the length of the default program name up to the extension and remove it, i.e. program.exe -> program
        // adjust the command line to exclude the program path
        int last_dot = full_exe_name;
        for (int i = 0; i < full_exe_name; ++i)
        {
            if (result->program_name[i] == '.')
            {
                last_dot = i;
            }
        }

        if (last_dot < CCMD_PROGRAM_NAME_MAX)
        {
            // zero-out the remaining path
            memset(result->program_name + last_dot, 0, CCMD_PROGRAM_NAME_MAX - last_dot);
        }
    }

    // Set default name for the program command, otherwise it will be assigned in the parse_command call if cli defines one
    ccmd_command_result* program_command = &result->commands.data[0];
    program_command->name = result->program_name;
    result->program_command = program_command;

    const int total_subcommand_count = ccmd_count_subcommands(cli);
    const ccmd_command** parsed_commands = CPLATFORM_ALLOCA_ARRAY(const ccmd_command*, sizeof(const ccmd_command*) * total_subcommand_count);
    parsed_commands[0] = cli;

    ccmd_status status = CCMD_STATUS_COUNT;
    if (result->errors.data == NULL)
    {
        // default error handling
        ccmd_error errors[64];
        CCMD_ARRAY_VIEW_INPLACE(result->errors, errors);

        status = ccmd_parse_command(subcommand_argc, subcommand_argv, &(ccmd_parser) {
            .command_result = &result->commands.data[result->commands_count++],
            .command_infos = parsed_commands,
            .program_result = result,
        });

        if (status == CCMD_STATUS_ERROR)
        {
            char error_string[4096];
            ccmd_formatter formatter = { .buffer_capacity = CCMD_ARRAY_SIZE(error_string), .buffer = error_string };
            ccmd_default_error_report(program_command->name, &formatter, result);
            fprintf(stderr, "%s\n", error_string);
        }
    }
    else
    {
        status = ccmd_parse_command(subcommand_argc, subcommand_argv, &(ccmd_parser) {
            .command_result = &result->commands.data[result->commands_count++],
            .command_infos = parsed_commands,
            .program_result = result
        });
    }

    // just always generate the default program help even if help wasn't requested if a usage buffer is assigned
    if (result->usage.data != NULL)
    {
        ccmd_formatter formatter = { .buffer_capacity = result->usage.count, .buffer = result->usage.data };
        ccmd_generate_usage(&formatter, result->commands_count, parsed_commands);
    }
    else if (status == CCMD_STATUS_HELP)
    {
        // otherwise do default usage handling if -h/--help were requested
        char usage_buffer[4096];
        ccmd_formatter formatter = { .buffer_capacity = CCMD_ARRAY_SIZE(usage_buffer), .buffer = usage_buffer };
        ccmd_generate_usage(&formatter, result->commands_count, parsed_commands);
        printf("%s\n", usage_buffer);
    }

    return status;
}

ccmd_status ccmd_run(const ccmd_result* program)
{
    const ccmd_command_result* cmd = &program->commands.data[program->commands_count - 1];

    if (cmd->run == NULL)
    {
        return CCMD_STATUS_SUCCESS;
    }

    return cmd->run(&program->commands.data[0], cmd);
}

ccmd_status ccmd_run_all(const ccmd_result* program)
{
    for (int i = 0; i < program->commands_count; ++i)
    {
        if (program->commands.data[i].run != NULL)
        {
            const ccmd_status status = program->commands.data[i].run(&program->commands.data[0], &program->commands.data[i]);
            if (status != CCMD_STATUS_SUCCESS)
            {
                return status;
            }
        }
    }

    return CCMD_STATUS_SUCCESS;
}

bool ccmd_has_option(const ccmd_command_result* command, const char* long_or_short_name)
{
    return ccmd_get_option(command, long_or_short_name) != NULL;
}

const ccmd_parsed_args* ccmd_get_option(const ccmd_command_result* command, const char* long_or_short_name)
{
    const int size = (int)strlen(long_or_short_name);

    if (size == 1)
    {
        // compare as short flag, i.e. -h
        for (int i = 0; i < command->options.count; ++i)
        {
            if (command->options.data[i].short_name == *long_or_short_name)
            {
                return &command->options.data[i];
            }
        }
    }
    else
    {
        // compare as long name, i.e. --help
        for (int i = 0; i < command->options.count; ++i)
        {
            if (strcmp(command->options.data[i].long_name, long_or_short_name) == 0)
            {
                return &command->options.data[i];
            }
        }
    }

    return NULL;
}

bool ccmd_has_positional(const ccmd_command_result* command, const int32_t position)
{
    return ccmd_get_positional(command, position) != NULL;
}

const char* ccmd_get_positional(const ccmd_command_result* command, const int32_t position)
{
    return position < command->positionals.count ? command->positionals.data[position] : NULL;
}