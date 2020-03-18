cmake_minimum_required(VERSION 3.1)
project(sfsketch)

#aux_source_directory(. DIR_SRCS)

include_directories("../include") 
set(CMAKE_C_FLAGS "-std=c11 -msse4.1 -O2 -Wall -pedantic -Wredundant-decls")
#set(CMAKE_C_FLAGS "-std=c11 -msse4.1 -O2 -Wall -Werror -pedantic -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls")
#set(CMAKE_C_FLAGS "-std=c99 -O2 -Wall -Werror -pedantic  -Wmissing-declarations -Wredundant-decls")

set(SOURCE_FILES sketch.c util.c BOBHash.c)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
#add_subdirectory(util)
add_executable(sfsketch ${SOURCE_FILES} main.c)
