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
    src/deps/http-parser/http_parser.c
    src/deps/multipart-parser-c/multipart_parser.c
    src/deps/websocket-parser/websocket_parser.c
    src/php_ion.c
    src/ion/ion_init.h
    src/ion/ion_errors.h
    src/ion/ion_strings.c
    src/ion/ion_memory.c
    src/ion/ion_crypto.h
    src/ion/ion_exceptions.c
    src/ion/ion_debug.c
    src/ion/ion_callback.c
    src/ion/ion_zend.c
    src/ion/ion_deferred_queue.c
    src/ion/ion_dns.h
    src/ion/ion_timers.h
    src/ion/ion_net.c
    src/ion/ion_promisor.c
    src/ion/ion_stream.c
    src/ion/ion_process.c
    src/ion/ion_fs.h
    src/ion/ion_fs.c
    src/ion/ion_http.c
    src/ion.h

    src/classes/ION/Debug.c
    src/classes/ION/Promise.c
    src/classes/ION/ResolvablePromise.c
    src/classes/ION/PromiseAggregator.c
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

    src/classes/ION/HTTP/Message.c
    src/classes/ION/HTTP.c
    src/classes/ION/URI.c
    src/classes/ION/HTTP/Request.c
    src/classes/ION/HTTP/Response.c
    src/classes/ION/HTTP/WebSocketParser.c
    src/classes/ION/HTTP/WebSocket/Frame.c src/classes/ION/EventAbstract.c src/classes/ION/TimerEvent.c src/ion/ion_event.c)

add_library(ion ${ION_SOURCE_FILES})
add_dependencies(ion external_ion)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/deps")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/deps/libevent")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/deps/libevent/include")
#include_directories("/Users/bzick/Dev/sources/php71-20170815-34008-ed27l6/php-7.1.8" " /Users/bzick/Dev/sources/php71-20170815-34008-ed27l6/php-7.1.8/main" " /Users/bzick/Dev/sources/php71-20170815-34008-ed27l6/php-7.1.8/Zend" " /Users/bzick/Dev/sources/php71-20170815-34008-ed27l6/php-7.1.8/TSRM")
include_directories("/opt/local/include/php72/php" "/opt/local/include/php72/php/main" "/opt/local/include/php72/php/Zend" "/opt/local/include/php72/php/TSRM"  )
#include_directories("/opt/local/include/php70/php" "/opt/local/include/php70/php/main" "/opt/local/include/php70/php/Zend" "/opt/local/include/php70/php/TSRM"  )
include_directories("/opt/local/var/macports/build/_opt_local_var_macports_sources_rsync.macports.org_macports_release_tarballs_ports_lang_php/php72/work/php-7.2.4/")
#include_directories("/opt/local/var/macports/build/_opt_local_var_macports_sources_rsync.macports.org_release_tarballs_ports_lang_php/php71/work/php-7.1.0RC3")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Libevent")
include_directories("/opt/local/include")
include_directories("/opt/local/include/openssl/")
#include_directories("/usr/local/opt/include")

