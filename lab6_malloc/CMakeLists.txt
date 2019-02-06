cmake_minimum_required(VERSION 3.9)
project(lab6malloc C)

set(CMAKE_C_STANDARD 99)

add_executable(lab6malloc mdriver.c memlib.h memlib.c mm.h mm.c fsecs.h fsecs.c
        config.h
        ftimer.h ftimer.c
        fcyc.h fcyc.c
        clock.h clock.c)