/*
 *  config.h
 *  ccmd
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#pragma once


//#include <stddef.h>

/*
 **********************
 *
 * # Build type setup
 *
 **********************
 */
#ifndef CCMD_DEBUG
    #define CCMD_DEBUG 0
#endif // CCMD_DEBUG
#ifndef CCMD_RELEASE
    #define CCMD_RELEASE 0
#endif // CCMD_RELEASE

/*
 ******************************************
 *
 * # String expansion and token pasting
 *
 ******************************************
 */
#define CCMD_EXPAND(x) x
#define CCMD_STRINGIFY(x) #x
#define CCMD_CONCAT_BASE(x, y) x##y
#define CCMD_CONCAT(x, y) CCMD_CONCAT_BASE(x, y)

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
        #define CCMD_OS_IOS 1
        #define CCMD_OS_NAME_STRING "iOS Simulator"
    #elif TARGET_OS_IPHONE == 1
        #define CCMD_OS_IOS 1
        #define CCMD_OS_NAME_STRING "iOS"
    #elif TARGET_OS_MAC == 1
        #define CCMD_OS_MACOS 1
        #define CCMD_OS_NAME_STRING "MacOS"
    #endif // TARGET_*
#elif defined(__WIN32__) || defined(__WINDOWS__) || defined(_WIN64) || defined(_WIN32) || defined(_WINDOWS) || defined(__TOS_WIN__)
    #define CCMD_OS_WINDOWS 1
    #define CCMD_OS_NAME_STRING "Windows"
#elif defined(__linux__) || defined(__linux) || defined(linux_generic)
    #define CCMD_OS_LINUX 1
    #define CCMD_OS_NAME_STRING "Linux"
#elif defined(__ANDROID__)
    #define CCMD_OS_ANDROID 1
    #define CCMD_ANDROID_API_LEVEL __ANDROID_API__
    #define CCMD_OS_NAME_STRING "Android"
#endif // defined(<platform>)

#if CCMD_OS_LINUX == 1 || CCMD_OS_MACOS == 1 || CCMD_OS_IOS == 1 || CCMD_OS_ANDROID == 1
    #define CCMD_OS_UNIX 1
#else
    #define CCMD_OS_UNIX 0
#endif //

#ifndef CCMD_OS_MACOS
    #define CCMD_OS_MACOS 0
#endif // CCMD_OS_MACOS
#ifndef CCMD_OS_IOS
    #define CCMD_OS_IOS 0
#endif // CCMD_OS_IOS
#ifndef CCMD_OS_ANDROID
    #define CCMD_OS_ANDROID 0
#endif // CCMD_OS_ANDROID
#ifndef CCMD_OS_WINDOWS
    #define CCMD_OS_WINDOWS 0
#endif // CCMD_OS_WINDOWS
#ifndef CCMD_OS_LINUX
    #define CCMD_OS_LINUX 0
#endif // CCMD_OS_LINUX

#ifndef CCMD_OS_NAME_STRING
    #define CCMD_OS_NAME_STRING "UNKNOWN_OS"
#endif // ifndef CCMD_OS_NAME_STRING

/*
 ******************************
 *
 * # Compiler detection
 *
 ******************************
 */
#if defined(__clang__)
    #define CCMD_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define CCMD_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define CCMD_COMPILER_MSVC 1
#endif // defined(_MSC_VER)

#ifndef CCMD_COMPILER_CLANG
    #define CCMD_COMPILER_CLANG 0
#endif // CCMD_COMPILER_CLANG
#ifndef CCMD_COMPILER_GCC
    #define CCMD_COMPILER_GCC 0
#endif // CCMD_COMPILER_GCC
#ifndef CCMD_COMPILER_MSVC
    #define CCMD_COMPILER_MSVC 0
#endif // CCMD_COMPILER_MSVC

#if CCMD_COMPILER_CLANG == 0  && CCMD_COMPILER_GCC == 0 && CCMD_COMPILER_MSVC == 0
    #define CCMD_COMPILER_UNKNOWN 1
#else
    #define CCMD_COMPILER_UNKNOWN 0
#endif // CCMD_COMPILER_CLANG == 0 || CCMD_COMPILER_GCC == 0 || CCMD_COMPILER_MSVC == 0

// Clang and GCC shared definitions
#if CCMD_COMPILER_CLANG == 1 || CCMD_COMPILER_GCC == 1
    #define CCMD_PACKED(n)                       __attribute__((packed, aligned(n)))
    #define CCMD_FUNCTION_NAME                   __PRETTY_FUNCTION__
    #define CCMD_FORCE_INLINE inline             __attribute__((always_inline))
    #define CCMD_PRINTFLIKE(fmt, firstvararg)    __attribute__((__format__ (__printf__, fmt, firstvararg)))
    #define CCMD_LIKELY(statement)               __builtin_expect((statement), 1)
    #define CCMD_UNLIKELY(statement)             __builtin_expect((statement), 0)
    #define CCMD_EXPORT_SYMBOL                   __attribute__ ((visibility("default")))
#endif // CCMD_COMPILER_CLANG || CCMD_COMPILER_GCC

// Compiler-specific non-shared definitions
#if CCMD_COMPILER_CLANG == 1
    #define CCMD_PUSH_WARNING                _Pragma("clang diagnostic push")
    #define CCMD_POP_WARNING                 _Pragma("clang diagnostic pop")
    #define CCMD_DISABLE_WARNING_CLANG(W)    _Pragma(CCMD_STRINGIFY(clang diagnostic ignored W))
#elif CCMD_COMPILER_GCC == 1
    #define CCMD_PUSH_WARNING                _Pragma("GCC diagnostic push")
    #define CCMD_POP_WARNING                 _Pragma("GCC diagnostic pop")
    #define CCMD_DISABLE_WARNING_GCC(W)      _Pragma(CCMD_STRINGIFY(GCC diagnostic ignored # W))
#elif CCMD_COMPILER_MSVC == 1
    // Displays type information, calling signature etc. much like __PRETTY_FUNCTION__
    // see: https://msdn.microsoft.com/en-us/library/b0084kay.aspx
    #define CCMD_FUNCTION_NAME                   __FUNCSIG__
    #define CCMD_FORCE_INLINE                    __forceinline
    #define CCMD_PRINTFLIKE(fmt, firstvararg)
    #define CCMD_PUSH_WARNING                    __pragma(warning( push ))
    #define CCMD_DISABLE_WARNING_MSVC(w)         __pragma(warning( disable: w ))
    #define CCMD_POP_WARNING                     __pragma(warning( pop ))
    #define CCMD_DISABLE_WARNING_CLANG(w)
    #define CCMD_LIKELY(statement)               (statement)
    #define CCMD_UNLIKELY(statement)             (statement)
    #if _WIN64
    #else
        #define CCMD_ARCH_32BIT
    #endif // _WIN64
#endif // CCMD_COMPILER_*

#ifndef CCMD_DISABLE_WARNING_CLANG
    #define CCMD_DISABLE_WARNING_CLANG(W)
#endif // CCMD_DISABLE_WARNING_CLANG
#ifndef CCMD_DISABLE_WARNING_GCC
    #define CCMD_DISABLE_WARNING_GCC(W)
#endif // CCMD_DISABLE_WARNING_GCC
#ifndef CCMD_DISABLE_WARNING_MSVC
    #define CCMD_DISABLE_WARNING_MSVC(W)
#endif // CCMD_DISABLE_WARNING_MSVC

// Define dllexport/import for windows if compiler is MSVC or Clang (supported in clang since at least v3.5)
#if CCMD_OS_WINDOWS == 1 && CCMD_COMPILER_GCC == 0
    #ifdef CCMD_DLL
        #undef CCMD_EXPORT_SYMBOL
        #undef CCMD_IMPORT_SYMBOL
        #define CCMD_EXPORT_SYMBOL   __declspec(dllexport)
        #define CCMD_IMPORT_SYMBOL   __declspec(dllimport)
    #endif // CCMD_DLL
#endif

#ifndef CCMD_EXPORT_SYMBOL
    #define CCMD_EXPORT_SYMBOL
#endif // CCMD_EXPORT_SYMBOL
#ifndef CCMD_IMPORT_SYMBOL
    #define CCMD_IMPORT_SYMBOL
#endif // CCMD_IMPORT_SYMBOL

#if defined(__OBJC__)
    #define CCMD_OBJC_RELEASE(object) [object release], (object) = nil
#else
    #define CCMD_OBJC_RELEASE
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
    #define CCMD_ARCH_64BIT 1
    #define CCMD_ARCH_BITS 64
#else
    #define CCMD_ARCH_32BIT 1
    #define CCMD_ARCH_BITS 32
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
#if !defined(CCMD_BIG_ENDIAN) && !defined(CCMD_LITTLE_ENDIAN)
    #define CCMD_LITTLE_ENDIAN
#endif // CCMD_LITTLE_ENDIAN


/*
 **************************************************
 *
 * # Assertion configuration
 * allow user to force assertions on if needed
 *
 **************************************************
 */
#if CCMD_FORCE_ASSERTIONS_ENABLED == 1
    #define CCMD_ENABLE_ASSERTIONS CCMD_FORCE_ASSERTIONS_ENABLED
#else
    #if CCMD_DEBUG == 1
        #define CCMD_ENABLE_ASSERTIONS 1
    #else
        #define CCMD_ENABLE_ASSERTIONS 0
    #endif // CCMD_DEBUG == 1
#endif // !defined(CCMD_FORCE_ASSERTIONS_ENABLED)

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
#define CCMD_BEGIN_MACRO_BLOCK do {
#define CCMD_END_MACRO_BLOCK } while (false)
#define CCMD_EXPLICIT_SCOPE(x) do { x } while (false)

#define CCMD_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define CCMD_MAX(X, Y) ((X) >= (Y) ? (X) : (Y))

#define CCMD_ROUND_UP(SIZE, ALIGNMENT) (((SIZE) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#define CCMD_UNUSED(x) (void)(x)

// Allows using NOLINT for clang-tidy inside macros
#define CCMD_NOLINT(...) __VA_ARGS__ // NOLINT

#if CCMD_OS_WINDOWS == 1
    #define CCMD_PATH_SEPARATOR '\\'
#else
    #define CCMD_PATH_SEPARATOR '/'
#endif // CCMD_OS_WINDOWS == 1

// Macros for working with explicit struct padding
#ifdef __cplusplus
    #define CCMD_PAD(N) char CCMD_CONCAT(padding, __LINE__)[N] { 0 }
#else
    #define CCMD_PAD(N) char CCMD_CONCAT(padding, __LINE__)[N]
#endif // __cplusplus

#define CCMD_DISABLE_PADDING_WARNINGS CCMD_DISABLE_WARNING_MSVC(4121 4820)

/*
 **********************************************************************************
 *
 * # CCMD_ALLOCA & CCMD_ALLOCA_ARRAY
 *
 * Portable alloca implementations - CCMD_ALLOCA_ARRAY
 * calls CCMD_ALLOCA with size==sizeof(T)*count and alignment==alignof(T)
 *
 **********************************************************************************
 */
// Must be a macro because even a FORCE_INLINE function may possibly free the memory
#if CCMD_OS_WINDOWS == 1
    #define CCMD_ALLOCA(SIZE, ALIGNMENT) (_alloca(CCMD_ROUND_UP(SIZE, ALIGNMENT)))
#else
    #define CCMD_ALLOCA(SIZE, ALIGNMENT) (alloca(CCMD_ROUND_UP(SIZE, ALIGNMENT)))
#endif // CCMD_OS_WINDOWS == 1

#ifdef __cplusplus
    #define CCMD_ALLOCA_ARRAY(T, SIZE) (static_cast<T*>(CCMD_ALLOCA(sizeof(T) * SIZE, alignof(T))))
#else
    #define CCMD_ALLOCA_ARRAY(T, SIZE) ((T*)CCMD_ALLOCA(sizeof(T) * (SIZE), _Alignof(T)))
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
 * # CCMD_MOVE
 *
 * this is a replacement for std::forward used to avoid having to include <utility> (which also includes <type_traits>)
 */
#define CCMD_FORWARD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace ccmd {
    #ifndef CCMD_STATIC_ARRAY_LENGTH_INLINE
        #define CCMD_STATIC_ARRAY_LENGTH_INLINE
        template <typename T, int Size>
        inline constexpr int static_array_length(T(&)[Size])
        {
            return Size;
        }
    #endif // CCMD_STATIC_ARRAY_LENGTH_INLINE
} // namespace ccmd
#endif