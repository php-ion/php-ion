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
        src/config.h
        src/php_ion.c
        src/pion/exceptions.c
        src/pion/debug.c
        src/pion/callback.c
        src/pion/engine.c
        src/pion/linkedlist.c
        src/pion/net.c
        src/pion.c
        src/ION/Debug.c
        src/externals/SkipList/skiplist.c
        src/ION/Promise.c
        src/ION/ResolvablePromise.c
        src/ION/Deferred.c
        src/ION/PromiseMap.c
        src/ION.c src/pion/promisor.c)

#src/ION/Deferred.c
#src/ION/Data/SkipList.c
#src/ION/Data/LinkedList.c
#src/ION/Promise/Result.c
#src/ION/Promise.c
#src/ION/ResolvablePromise.c
#src/ION.c
#src/ION/DNS.c
#src/ION/Process.c
#src/ION/Stream.c
#src/ION/Promise/Map.c src/ION/Promise/Map.h

add_library(ion ${ION_SOURCE_FILES})
add_dependencies(ion external_ion)
include_directories("/opt/local/include" "/opt/local/include/php70/php" "/opt/local/include/php70/php/main" "/opt/local/include/php70/php/Zend" "/opt/local/include/php70/php/TSRM" "/opt/local/var/macports/build/_opt_local_var_macports_sources_rsync.macports.org_release_tarballs_ports_devel_libevent/libevent/work/libevent-2.0.22-stable/")