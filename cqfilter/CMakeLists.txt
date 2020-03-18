cmake_minimum_required(VERSION 3.1)
project(cqfilter)

include_directories("../include") 
set(CMAKE_C_FLAGS "-std=gnu11 -msse4.1 -O2 -Wall -pedantic -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls")
#set(CMAKE_C_FLAGS "-std=c99 -O2 -Wall -Werror -pedantic  -Wmissing-declarations -Wredundant-decls")

set(SOURCE_FILES gqf.c gqf_file.c hashutil.c partitioned_counter.c BOBHash.c)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_executable(cqfilter ${SOURCE_FILES} main.c)

target_link_libraries(cqfilter -lm)
