cmake_minimum_required(VERSION 2.8.4)
project(sbunix C ASM)
include_directories(include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -std=c99 -D__thread= -fno-stack-protector -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -march=amdfam10 -g3")
# source for crt1.o
set(crt1_SRCS crt/crt1.c)

# Glob the libc sources
file(GLOB libc_SRCS         libc/*.c)

# sbush, the shell sources
file(GLOB sbush_SRCS        bin/sbush/*.c)

file(GLOB alloctest_SRCS    bin/alloctest/*.c)

# Glob kernel sources
file(GLOB kernel_SRCS       sys/*.c sys/*.s)

# obj's for libc/crt
add_library(c           STATIC ${libc_SRCS})
add_library(crt1_O      OBJECT ${crt1_SRCS})

# obj's for executables
add_library(sbush_O     OBJECT ${sbush_SRCS})
add_library(alloctest_O OBJECT ${alloctest_SRCS})

add_executable(sbush
    $<TARGET_OBJECTS:crt1_O>
    $<TARGET_OBJECTS:sbush_O>)

target_link_libraries(sbush c)

add_executable(alloctest
    $<TARGET_OBJECTS:crt1_O>
    $<TARGET_OBJECTS:alloctest_O>)

target_link_libraries(alloctest c)

# kernel binary
add_executable(kernel ${kernel_SRCS})

SET_TARGET_PROPERTIES(kernel
  PROPERTIES 
  LINK_FLAGS "-nostdlib -o kernel -T ${PROJECT_SOURCE_DIR}/linker.script")

