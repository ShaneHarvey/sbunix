cmake_minimum_required(VERSION 2.8.4)
project(sbush)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(STRING_SRCS
    lib/string.c)

FILE(GLOB SBUSH_SRCS bin/sbush/*.c)

include_directories(include)

add_library(string_lib OBJECT ${STRING_SCRS})

add_executable(sbush
    ${SBUSH_SRCS}
    $<TARGET_OBJECTS:string_lib>)