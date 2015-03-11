cmake_minimum_required(VERSION 2.8.4)
project(sbunix C ASM)
include_directories(include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -std=c99 -D__thread= -fno-stack-protector -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -march=amdfam10 -g3")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")

# source for crt1.o
set(crt1_SRCS crt/crt1.c)

# Glob the libc sources
file(GLOB libc_SRCS         libc/*.c)

# obj's for libc/crt
add_library(c           STATIC ${libc_SRCS})
add_library(crt1_O      OBJECT ${crt1_SRCS})

# Grab all the directories in bin/
execute_process(COMMAND ls  ${PROJECT_SOURCE_DIR}/bin/
                OUTPUT_VARIABLE progdirs)

# Replace '\n' from ls with ';', the cmake seperator for lists
string(REPLACE "\n" ";" progdirs_list ${progdirs})

# Loop over the directories (which will be the executables)
foreach(progname  ${progdirs_list})
    # Sources
    file(GLOB   ${progname}_SRCS    bin/${progname}/*.c)

    # Objects
    add_library(${progname}_O   OBJECT  ${${progname}_SRCS})

    add_executable(${progname}
        $<TARGET_OBJECTS:crt1_O>
        $<TARGET_OBJECTS:${progname}_O>)

    # Link with "sblibc"
    target_link_libraries(${progname} c)
endforeach(progname)


# Glob kernel sources
file(GLOB kernel_SRCS       sys/*.c sys/*.s sys/*/*.c sys/*/*.s)

# kernel binary
add_executable(kernel ${kernel_SRCS})

SET_TARGET_PROPERTIES(kernel
  PROPERTIES
  LINK_FLAGS "-nostdlib -T ${PROJECT_SOURCE_DIR}/linker.script")

