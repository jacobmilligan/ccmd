cmake_minimum_required(VERSION 3.19)
project(cli C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(source/cli)
add_subdirectory(example)
