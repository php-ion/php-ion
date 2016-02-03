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
        src/ion_skiplist_config.h
        src/deps/skiplist/skiplist.c
        src/deps/http-parser/http_parser.c
        src/deps/multipart-parser-c/multipart_parser.c
        src/deps/websocket-parser/websocket_parser.c
        src/php_ion.c
        src/core/init.h
        src/core/errors.h
        src/core/strings.c
        src/core/memory.c
        src/core/zts.h
        src/core/crypto.h
        src/core/exceptions.c
        src/core/debug.c
        src/core/callback.c
        src/core/engine.c
        src/core/linkedlist.c
        src/core/net.c
        src/core/promisor.c
        src/core/stream.c
        src/core/http.c
        src/ion_core.h

        src/classes/ION/Debug.c
        src/classes/ION/Promise.c
        src/classes/ION/ResolvablePromise.c
        src/classes/ION/Deferred.c
        src/classes/ION/Sequence.c
        src/classes/ION.c
        src/classes/ION/DNS.c
        src/classes/ION/Listener.c
        src/classes/ION/Stream.c
        src/classes/ION/Process.c
        src/classes/ION/FS.c
        src/classes/ION/Crypto.c

        src/classes/ION/Stream/StorageAbstract.c
        src/classes/ION/Stream/Storage.c
        src/classes/ION/Stream/Server.c
        src/classes/ION/Stream/Client.c

        src/classes/ION/HTTP/Message.c
        src/classes/ION/HTTP.c
        src/classes/ION/URI.c
        src/classes/ION/HTTP/Request.c
        src/classes/ION/HTTP/Response.c
        src/classes/ION/HTTP/WebSocket/Frame.c
        )

add_library(ion ${ION_SOURCE_FILES})
add_dependencies(ion external_ion)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("/opt/local/include/php70/php" "/opt/local/include/php70/php/main" "/opt/local/include/php70/php/Zend" "/opt/local/include/php70/php/TSRM"  )
include_directories("/opt/local/var/macports/build/_opt_local_var_macports_sources_rsync.macports.org_release_tarballs_ports_lang_php/php70/work/php-7.0.0")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Libevent")
include_directories("/opt/local/include")

