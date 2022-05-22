cmake_minimum_required(VERSION 3.19)
project(ccmd C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)


add_library(ccmd STATIC ccmd.h ccmd.c)
if (WIN32)
    set(padding_warnings
            /we4820         # warn about padding at end of structure
            /we4121         # warn about padding inserted between structure members for alignment purposes
            )
    target_compile_definitions(ccmd PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif ()

if (DEFINED CCMD_BUILD_EXAMPLES)
    add_subdirectory(example)
endif ()
