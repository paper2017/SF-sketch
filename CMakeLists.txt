cmake_minimum_required(VERSION 3.1)
project(sketch)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror -pedantic -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

OPTION(SF1 "slim-fat sketch v1" OFF)
OPTION(SF2 "slim-fat sketch v2" OFF)
OPTION(SF3 "slim-fat sketch v3" OFF)
OPTION(SF4 "slim-fat sketch v4" OFF)
OPTION(SF5 "slim-fat sketch v5" OFF)
OPTION(SF6 "slim-fat sketch v6" OFF)

IF(SF1)
    ADD_DEFINITIONS(-DSF1=1)
    set(SOURCE_FILES lib/BOBHash.c sf1sketch/sf1sketch.c sketch_config.c)
    add_executable(sf1sketch ${SOURCE_FILES} main.c)
ENDIF(SF1)

IF(SF2)
    ADD_DEFINITIONS(-DSF2=1)
    set(SOURCE_FILES lib/BOBHash.c sf2sketch/sf2sketch.c sketch_config.c)
    add_executable(sf2sketch ${SOURCE_FILES} main.c)
ENDIF(SF2)

IF(SF3)
    ADD_DEFINITIONS(-DSF3=1)
    set(SOURCE_FILES lib/BOBHash.c sf3sketch/sf3sketch.c sketch_config.c)
    add_executable(sf3sketch ${SOURCE_FILES} main.c)
ENDIF(SF3)

IF(SF4)
    ADD_DEFINITIONS(-DSF4=1)
    set(SOURCE_FILES lib/BOBHash.c sf4sketch/sf4sketch.c sketch_config.c)
    add_executable(sf4sketch ${SOURCE_FILES} main.c)
ENDIF(SF4)

IF(SF5)
    ADD_DEFINITIONS(-DSF5=1)
    set(SOURCE_FILES lib/BOBHash.c sf5sketch/sf5sketch.c sketch_config.c)
    add_executable(sf5sketch ${SOURCE_FILES} main.c)
ENDIF(SF5)

IF(SF6)
    ADD_DEFINITIONS(-DMC=1)
    set(SOURCE_FILES lib/BOBHash.c sf6sketch/sf6sketch.c sketch_config.c)
    add_executable(sf6sketch ${SOURCE_FILES} main.c)
ENDIF(MC)


#set(SOURCE_FILES lib/BOBHash.c sf6sketch/sf6sketch.c sf1sketch/sf1sketch.c sf2sketch/sf2sketch.c sf3sketch/sf3sketch.c  sf4sketch/sf4sketch.c  sf5sketch/sf5sketch.c sketch_config.c)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
#add_executable(sketch ${SOURCE_FILES} main.c)
