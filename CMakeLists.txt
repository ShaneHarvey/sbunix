cmake_minimum_required(VERSION 2.8.4)
project(sbush)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -std=c99 -D__thread= -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -march=amdfam10 -g3 -fno-stack-protector -nostdlib")

# Glob the libc sources
FILE(GLOB libc_SRCS libc/*.c)

# sbush, the shell sources
FILE(GLOB sbush_SRCS bin/sbush/*.c)

# source for crt1.o
set(crt1_SRCS crt/crt1.c)

include_directories(include)

add_library(c STATIC ${libc_SRCS})

add_library(crt1_O OBJECT ${crt1_SRCS})

add_library(sbush_O OBJECT ${sbush_SRCS})

add_executable(sbush
    $<TARGET_OBJECTS:crt1_O>
    $<TARGET_OBJECTS:sbush_O>)

target_link_libraries(sbush c)
# This didn't work, why not?
# target_link_libraries(sbush ${c})