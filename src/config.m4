# +----------------------------------------------------------------------+
# | PHP Version 7                                                        |
# +----------------------------------------------------------------------+
# | Author: Ivan Shalganov <ivan@shalganov.me>                           |
# +----------------------------------------------------------------------+


VERSION=`git describe --tags --long`

PHP_ARG_WITH(ion, for asynchronous IO notifications $VERSION,
[  --with-ion              Include ION support])

PHP_ARG_ENABLE(ion-debug, whether to enable debug for ION,
[  --enable-ion-debug      Enable ION debug], no, no)

PHP_ARG_ENABLE(ion-coverage, whether to enable code coverage for ION,
[  --enable-ion-coverage   Enable code coverage for ION], no, no)

# --enable-ion-debug
if test "$PHP_ION_DEBUG" != "no"; then
    AC_DEFINE(ION_DEBUG, 1, [enable ION debug mode])
fi

# --enable-ion-coverage
if test "$PHP_ION_COVERAGE" != "no"; then
    AC_DEFINE(ION_COVERAGE, 1, [enable ION code coverage])
fi

# --with-ion
if test "$PHP_ION" != "no"; then
    AC_DEFINE_UNQUOTED(ION_VERSION, "$VERSION", [Current git version])

    EVENT_DIR="deps/libevent"
    PHP_ADD_INCLUDE($EVENT_DIR/include)
    PHP_ADD_INCLUDE($EVENT_DIR)
    AC_DEFINE(HAVE_LIBEVENT, 1, ['libevent"])
    AC_DEFINE([ION_EVENT_ENGINE], ["libevent"], [ Event engine used ])

    PHP_ADD_LIBRARY_WITH_PATH(event, $EVENT_DIR, ION_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(event_openssl, $EVENT_DIR, ION_SHARED_LIBADD)
    if test "$enable_maintainer_zts" = "yes"; then
        AC_DEFINE(HAVE_THREADING, 1, ['zts"])
        PHP_ADD_LIBRARY_WITH_PATH(event_pthreads, $EVENT_DIR, ION_SHARED_LIBADD)
    fi

    AC_CHECK_FUNCS(fork, AC_DEFINE(HAVE_FORK,1, [ ]),)
    AC_CHECK_FUNCS(kill, AC_DEFINE(HAVE_KILL,1, [ ]),)
    AC_CHECK_FUNCS(waitpid, [ AC_DEFINE(HAVE_WAITPID,1,[ ]) ],)
    AC_CHECK_FUNCS(inotify_init, [ AC_DEFINE(HAVE_INOTIFY,1, [ ]) ],)
    AC_CHECK_FUNCS(kqueue, [ AC_DEFINE(HAVE_KQUEUE,1, [ ]) ],)

    PHP_SUBST(ION_SHARED_LIBADD)

    ion_src="php_ion.c
    deps/http-parser/http_parser.c
    deps/multipart-parser-c/multipart_parser.c
    deps/websocket-parser/websocket_parser.c
    ion/ion_debug.c
    ion/ion_strings.c
    ion/ion_memory.c
    ion/ion_exceptions.c
    ion/ion_callback.c
    ion/ion_zend.c
    ion/ion_promisor.c
    ion/ion_event.c
    ion/ion_net.c
    ion/ion_stream.c
    ion/ion_deferred_queue.c
    ion/ion_process.c
    ion/ion_fs.c
    ion/ion_http.c
    classes/ION/Debug.c
    classes/ION/Promise.c
    classes/ION/ResolvablePromise.c
    classes/ION/Deferred.c
    classes/ION/Sequence.c
    classes/ION.c
    classes/ION/EventAbstract.c
    classes/ION/TimerEvent.c
    classes/ION/Crypto.c
    classes/ION/DNS.c
    classes/ION/FS.c
    classes/ION/Listener.c
    classes/ION/Stream.c
    classes/ION/Process.c
    classes/ION/Process/Exec.c
    classes/ION/Process/IPC.c
    classes/ION/Process/IPC/Message.c
    classes/ION/Process/ChildProcess.c
    classes/ION/URI.c
    classes/ION/HTTP/Message.c
    classes/ION/HTTP/Request.c
    classes/ION/HTTP/Response.c
    classes/ION/HTTP/WebSocket/Frame.c
    classes/ION/HTTP/WebSocketParser.c
    classes/ION/HTTP.c"

    PHP_NEW_EXTENSION(ion, $ion_src, $ext_shared,, "$CFLAGS -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 -DSKIPLIST_LOCAL_INCLUDE=\"<ion_skiplist_config.h>\" ")
fi
