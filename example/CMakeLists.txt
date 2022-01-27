add_executable(ccmd_example_basic basic.c)
target_link_libraries(ccmd_example_basic ccmd)
target_include_directories(ccmd_example_basic PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(ccmd_example_subcommands subcommands.c)
target_link_libraries(ccmd_example_subcommands ccmd)
target_include_directories(ccmd_example_subcommands PRIVATE ${PROJECT_SOURCE_DIR})
