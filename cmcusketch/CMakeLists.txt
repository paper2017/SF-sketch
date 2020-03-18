cmake_minimum_required(VERSION 3.1)
project(cmcuSketch)

include_directories("../include") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -O2")
set(CMAKE_C_FLAGS "-std=c99 -O2")

set(SOURCE_FILES BOBHash.cpp cmcusketch.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_executable(cmcusketch ${SOURCE_FILES} main.cpp)
