cmake_minimum_required(VERSION 3.2)
project(ion)

include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FindPkgConfig)

set(CMAKE_SOURCE_DIR ./src)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g3 -ggdb -O0")


add_custom_command(
   OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/ion.so
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
   COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/src/Makefile
   COMMENT "Running io makefile"
)

add_custom_target(
   external_ion ALL
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/ion.so
)

set(ION_SOURCE_FILES
    src/php_ion.c
    src/pion/exceptions.c
    src/pion/debug.c
    src/pion/callback.c
    src/pion/engine.c
    src/pion/linkedlist.c
    src/pion/deferred.h
    src/pion.c
    src/ION/Data/LinkedList.c
    src/externals/SkipList/skiplist.c
    src/ION/Data/SkipList.c
    src/ION/Deferred.c
    src/ION.c
    src/ION/Process.c
)

add_library(ion ${ION_SOURCE_FILES})
add_dependencies(ion external_ion)
include_directories("/opt/local/include" "/opt/local/include/php56/php" "/opt/local/include/php56/php/main" "/opt/local/include/php56/php/Zend" "/opt/local/include/php56/php/TSRM")