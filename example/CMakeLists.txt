add_executable(cli_example cmd.c)
target_link_libraries(cli_example cli)
target_include_directories(cli_example PRIVATE ${PROJECT_SOURCE_DIR}/source)