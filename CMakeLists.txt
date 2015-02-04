cmake_minimum_required(VERSION 2.8.4)
project(sbush)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SBUSH_SRCS
    bin/hello/hello.c
    bin/sbush/sbush.c)

include_directories(include)

add_executable(sbush ${SBUSH_SRCS})