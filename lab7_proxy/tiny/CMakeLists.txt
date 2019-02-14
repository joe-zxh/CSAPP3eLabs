cmake_minimum_required(VERSION 3.9)
project(tiny C)

set(CMAKE_C_STANDARD 11)

add_executable(tiny tiny.c csapp.h csapp.c)

find_package(Threads REQUIRED)
target_link_libraries(tiny Threads::Threads)