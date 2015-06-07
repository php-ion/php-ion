dnl config.m4 for extension ion

PHP_ARG_WITH(ion, for asynchronous IO notifications,
[  --with-ion             Include ION support])

CFLAGS="$CFLAGS -Wall -g3 -ggdb -O0"
AC_DEFINE(ION_DEBUG, 1, [Enable ION debug support])

if test "$PHP_ION" != "no"; then
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
    ],[
      AC_MSG_ERROR([wrong libevent version or lib not found])
    ],[
      -L$ION_DIR/lib $EXTRA_LIBS
    ])
    dnl PHP_SUBST(LIBEVENT_SHARED_LIBADD)
    case $build_os in
    darwin1*.*.*)
      AC_MSG_CHECKING([whether to compile for recent osx architectures])
      CFLAGS="$CFLAGS -arch i386 -arch x86_64 -mmacosx-version-min=10.5"
      AC_MSG_RESULT([yes])
      ;;
    darwin*)
      AC_MSG_CHECKING([whether to compile for every osx architecture ever])
      CFLAGS="$CFLAGS -arch i386 -arch x86_64 -arch ppc -arch ppc64"
      AC_MSG_RESULT([yes])
      ;;
    esac
    AC_CHECK_FUNCS(fork, AC_DEFINE(HAVE_FORK,1, [ ]),)
    AC_CHECK_FUNCS(kill, AC_DEFINE(HAVE_KILL,1, [ ]),)
    AC_CHECK_FUNCS(waitpid, [ AC_DEFINE(HAVE_WAITPID,1,[ ]) ],)
    AC_CHECK_FUNCS(inotify_init, [ AC_DEFINE(HAVE_INOTIFY,1, [ ]) ],)
    AC_CHECK_FUNCS(setproctitle, [ AC_DEFINE(HAVE_SETPROCTITLE,1,[ ]) ],)
    ion_src="framework.c ION/Data/LinkedList.c php_ion.c"
    PHP_NEW_EXTENSION(ion, $ion_src, $ext_shared,, $CFLAGS)

    PHP_SUBST(ION_SHARED_LIBADD)
fi
