add_executable(cli_example_basic basic.c)
target_link_libraries(cli_example_basic cli)
target_include_directories(cli_example_basic PRIVATE ${PROJECT_SOURCE_DIR}/source)

add_executable(cli_example_subcommands subcommands.c)
target_link_libraries(cli_example_subcommands cli)
target_include_directories(cli_example_subcommands PRIVATE ${PROJECT_SOURCE_DIR}/source)
