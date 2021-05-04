add_library(cli STATIC config.h cli.h cli.c)
if (WIN32)
    set(padding_warnings
        /we4820         # warn about padding at end of structure
        /we4121         # warn about padding inserted between structure members for alignment purposes
    )
    target_compile_definitions(cli PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif ()
