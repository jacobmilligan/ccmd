add_library(cli STATIC config.h cli.h cli.c)
if (WIN32)
    target_compile_definitions(cli PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif ()
