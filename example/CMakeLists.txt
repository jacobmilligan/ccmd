add_executable(cli_example cmd.c)
target_link_libraries(cli_example cli)
target_include_directories(cli_example PRIVATE ${PROJECT_SOURCE_DIR}/source)

add_executable(cli_example_cpp cmd.cpp)
target_link_libraries(cli_example_cpp cli)
target_include_directories(cli_example_cpp PRIVATE ${PROJECT_SOURCE_DIR}/source)
