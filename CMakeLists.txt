cmake_minimum_required(VERSION 3.16)

project(neurobit)

enable_language(C)
set(CMAKE_C_STANDARD 99)

add_executable(neurobit_test neurobit.h test.c)
target_link_libraries(neurobit_test m)
