add_executable(cli_example_basic basic.c)
target_link_libraries(cli_example_basic cli)
target_include_directories(cli_example_basic PRIVATE ${PROJECT_SOURCE_DIR}/source)

add_executable(cli_example_advanced advanced.c)
target_link_libraries(cli_example_advanced cli)
target_include_directories(cli_example_advanced PRIVATE ${PROJECT_SOURCE_DIR}/source)

add_executable(cli_example_cpp cmd.cpp)
target_link_libraries(cli_example_cpp cli)
target_include_directories(cli_example_cpp PRIVATE ${PROJECT_SOURCE_DIR}/source)
