/*
 *  config.h
 *  cli
 *
 *  Copyright (c) 2021 Jacob Milligan. All rights reserved.
 */

#pragma once

#define CLI_OS_MACOS 0
#define CLI_OS_WINDOWS 0
#define CLI_OS_LINUX 0

#if defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC == 1
        #undef CLI_OS_MACOS
        #define CLI_OS_MACOS 1
        #define CLI_OS_NAME_STRING "MacOS"
    #endif // TARGET_*
#elif defined(__WIN32__) || defined(__WINDOWS__) || defined(_WIN64) || defined(_WIN32) || defined(_WINDOWS) || defined(__TOS_WIN__)
    #undef CLI_OS_WINDOWS
    #define CLI_OS_WINDOWS 1
    #define CLI_OS_NAME_STRING "Windows"
#elif defined(__linux__) || defined(__linux) || defined(linux_generic)
    #undef CLI_OS_LINUX
    #define CLI_OS_LINUX 1
    #define CLI_OS_NAME_STRING "Linux"
#endif // defined(<platform>)

#if CLI_OS_LINUX == 1 || CLI_OS_MACOS == 1
    #define CLI_OS_UNIX 1
#else
    #define CLI_OS_UNIX 0
#endif // CLI_OS_*

#define CLI_COMPILER_CLANG 0
#define CLI_COMPILER_GCC 0
#define CLI_COMPILER_MSVC 0
#define CLI_COMPILER_UNKNOWN 1

#if defined(__clang__)
    #undef CLI_COMPILER_CLANG
    #define CLI_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #undef CLI_COMPILER_GCC
    #define CLI_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #undef CLI_COMPILER_MSVC
    #define CLI_COMPILER_MSVC 1
#endif // defined(_MSC_VER)

#if CLI_COMPILER_CLANG == 1 || CLI_COMPILER_GCC == 1 || CLI_COMPILER_MSVC == 1
    #undef CLI_COMPILER_UNKNOWN
    #define CLI_COMPILER_UNKNOWN 0
#endif // CLI_COMPILER_CLANG == 1 || CLI_COMPILER_GCC == 1 || CLI_COMPILER_MSVC == 1

/*
 * Processor architecture - only works for x86_64 detection.
 * see: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
 */
#if defined(__x86_64__) || defined(_M_X64)
    #define CLI_ARCH_64BIT 1
    #define CLI_ARCH_BITS 64
#else
    #define CLI_ARCH_32BIT 1
    #define CLI_ARCH_BITS 32
#endif // Processor arch

#define CLI_ROUND_UP(SIZE, ALIGNMENT) ((SIZE + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

// Must be a macro because even a FORCE_INLINE function may possibly free the memory
#if CLI_OS_WINDOWS == 1
    #define CLI_ALLOCA(SIZE, ALIGNMENT) (_alloca(CLI_ROUND_UP(SIZE, ALIGNMENT)))
#else
    #define CLI_ALLOCA(SIZE, ALIGNMENT) (alloca(CLI_ROUND_UP(SIZE, ALIGNMENT)))
#endif // CLI_OS_WINDOWS == 1

#ifdef __cplusplus
    #define CLI_ALLOCA_ARRAY(T, SIZE) (static_cast<T*>(CLI_ALLOCA(sizeof(T) * SIZE, alignof(T))))
#else
    #define CLI_ALLOCA_ARRAY(T, SIZE) ((T*)CLI_ALLOCA(sizeof(T) * SIZE, _Alignof(T)))
#endif // __cplusplus