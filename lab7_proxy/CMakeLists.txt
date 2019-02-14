cmake_minimum_required(VERSION 3.9)
project(lab7_proxy C)

set(CMAKE_C_STANDARD 11)

add_executable(lab7_proxy proxy.c csapp.h csapp.c)

find_package(Threads REQUIRED)
target_link_libraries(lab7_proxy Threads::Threads)