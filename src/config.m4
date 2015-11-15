# +----------------------------------------------------------------------+
# | PHP Version 7                                                        |
# +----------------------------------------------------------------------+
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

# --with-ion
if test "$PHP_ION" != "no"; then
    AC_DEFINE_UNQUOTED(ION_VERSION, "$VERSION", [Current git version])

    # search libevent headers
    SEARCH_PATH="/usr/local /usr /opt/local"
    SEARCH_FOR="/include/event.h"
    if test -r $PHP_ION/; then
      ION_DIR=$PHP_ION
      AC_MSG_RESULT(Using $ION_DIR)
    else
      AC_MSG_CHECKING([for libevent files in default path])
      for i in $SEARCH_PATH ; do
        if test -r $i/$SEARCH_FOR; then
          ION_DIR=$i
          AC_MSG_RESULT(found in $i)
        fi
      done
    fi

    if test -z "$ION_DIR"; then
        AC_MSG_RESULT([not found])
        AC_MSG_ERROR([Please reinstall the libevent distribution])
    fi

    # --enable-ion-debug
    if test "$PHP_ION_DEBUG" != "no"; then
        AC_DEFINE(ION_DEBUG, 1, [enable ION debug mode])
        CFLAGS="$CFLAGS -Wall -g3 -ggdb -O0 -std=c99"
    fi

    # --enable-ion-coverage
    if test "$PHP_ION_COVERAGE" != "no"; then
        AC_DEFINE(ION_COVERAGE, 1, [enable ION code coverage])
        CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
        LDFLAGS="$LDFLAGS -fprofile-arcs -ftest-coverage"
    fi


    AC_MSG_RESULT(checking compiler flags: $CFLAGS)
    PHP_ADD_INCLUDE($ION_DIR/include)

    EXTRA_LIBS="-lm"
    AC_CHECK_LIB(dl, dlopen, [
       EXTRA_LIBS="$EXTRA_LIBS -ldl"
    ])

    LIBNAME=event
    LIBSYMBOL=event_base_loop

    PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
    [
      PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ION_DIR/lib, ION_SHARED_LIBADD)
      AC_DEFINE(HAVE_LIBEVENT, 1, [ libevent ])
      AC_DEFINE([ION_EVENT_ENGINE], ["libevent"], [ Event engine used ])
    ],[
      AC_MSG_ERROR([wrong libevent version or lib not found])
    ],[
      -L$ION_DIR/lib $EXTRA_LIBS
    ])
    # PHP_SUBST(LIBEVENT_SHARED_LIBADD)
    case $build_os in
    darwin1*.*.*)
      AC_MSG_CHECKING([whether to compile for recent osx architectures])
      CFLAGS="$CFLAGS -arch x86_64 -mmacosx-version-min=10.5"
      AC_MSG_RESULT([yes])
      ;;
    darwin*)
      AC_MSG_CHECKING([whether to compile for every osx architecture ever])
      CFLAGS="$CFLAGS -arch x86_64 -arch ppc -arch ppc64"
      AC_MSG_RESULT([yes])
      ;;
    esac
    AC_CHECK_FUNCS(fork, AC_DEFINE(HAVE_FORK,1, [ ]),)
    AC_CHECK_FUNCS(kill, AC_DEFINE(HAVE_KILL,1, [ ]),)
    AC_CHECK_FUNCS(waitpid, [ AC_DEFINE(HAVE_WAITPID,1,[ ]) ],)
    AC_CHECK_FUNCS(inotify_init, [ AC_DEFINE(HAVE_INOTIFY,1, [ ]) ],)
    ion_src="php_ion.c
    pion/debug.c
    pion/exceptions.c
    pion/callback.c
    pion/engine.c
    pion/promisor.c
    pion/net.c
    pion.c

    ION/Debug.c
    ION/Promise.c
    ION/ResolvablePromise.c
    ION/Deferred.c
    ION/Sequence.c
    ION.c
    ION/DNS.c
    ION/Listener.c
    ION/Stream.c
    ION/Process.c
    "

    PHP_NEW_EXTENSION(ion, $ion_src, $ext_shared,, "$CFLAGS -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1")

    PHP_SUBST(ION_SHARED_LIBADD)
fi
