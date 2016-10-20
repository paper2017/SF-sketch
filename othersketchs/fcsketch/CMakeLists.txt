cmake_minimum_required(VERSION 3.1)
project(fcsketch)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror -pedantic -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls")

set(SOURCE_FILES sketch_config.c sketch.c BOBHash.c)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_executable(fcsketch ${SOURCE_FILES} main.c)