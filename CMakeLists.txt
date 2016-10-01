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
    src/ion_skiplist_config.h
    src/deps/skiplist/skiplist.c
    src/deps/http-parser/http_parser.c
    src/deps/multipart-parser-c/multipart_parser.c
    src/deps/websocket-parser/websocket_parser.c
    src/php_ion.c
    src/ion/init.h
    src/ion/errors.h
    src/ion/strings.c
    src/ion/memory.c
    src/ion/crypto.h
    src/ion/exceptions.c
    src/ion/debug.c
    src/ion/callback.c
    src/ion/engine.c
    src/ion/deferred_queue.c
    src/ion/dns.h
    src/ion/timers.h
    src/ion/net.c
    src/ion/promisor.c
    src/ion/stream.c
    src/ion/process.c
    src/ion/fs.h
    src/ion/fs.c
    src/ion/http.c
    src/ion.h

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
    src/classes/ION/Process/Exec.c
    src/classes/ION/Process/IPC.c
    src/classes/ION/Process/IPC/Message.c
    src/classes/ION/Process/ChildProcess.c

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
    src/classes/ION/HTTP/WebSocketParser.c
    src/classes/ION/HTTP/WebSocket/Frame.c
 )

add_library(ion ${ION_SOURCE_FILES})
add_dependencies(ion external_ion)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("/opt/local/include/php70/php" "/opt/local/include/php70/php/main" "/opt/local/include/php70/php/Zend" "/opt/local/include/php70/php/TSRM"  )
include_directories("/opt/local/var/macports/build/_opt_local_var_macports_sources_rsync.macports.org_release_tarballs_ports_lang_php/php70/work/php-7.0.0")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Libevent")
include_directories("/opt/local/include")

