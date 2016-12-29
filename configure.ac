AC_INIT(src/main.c)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_PROG_MAKE_SET

AM_SANITY_CHECK
AM_INIT_AUTOMAKE(somamdd, 0.1)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_CC
AC_ISC_POSIX
AC_PROG_LIBTOOL

AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS(unistd.h stdlib.h stdio.h string.h sys/types.h sys/stat.h dirent.h signal.h sys/wait.h time.h pwd.h fcntl.h sys/param.h sys/socket.h netinet/in.h arpa/inet.h netdb.h termios.h sched.h errno.h)

AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_SIGNAL

AC_FUNC_VPRINTF

AC_CHECK_HEADER(pthread.h,, AC_MSG_ERROR(pthread is required to compile soma without libsm))

AC_CHECK_LIB(pthread,pthread_mutex_init,, AC_MSG_ERROR(pthread is required to compile soma))

AC_CHECK_HEADER(dlfcn.h)
AC_CHECK_LIB(dl,dlopen)

LDFLAGS="$LDFLAGS -lpthread"

AC_PATH_PROG(pkgconfigpath, pkg-config)
AC_MSG_CHECKING(for openssl through pkg-config)
if test "$pkgconfigpath" != "" && \
   $pkgconfigpath openssl --exists; then
  CFLAGS="$CFLAGS `$pkgconfigpath openssl --cflags` -DSOMA_USE_OPENSSL"
  LIBS="$LIBS `$pkgconfigpath openssl --libs`"
  AC_MSG_RESULT(yes)
else
  AC_MSG_ERROR(Failed to found ssl library)
fi

AC_PATH_PROG(somaconfigpath, soma-config)
AC_MSG_CHECKING(for soma through soma-config)
if test "$somaconfigpath" != ""; then
  MAJOR=`$somaconfigpath --version | cut -d\. -f1`
  MINOR=`$somaconfigpath --version | cut -d\. -f2`

  if test "$MAJOR" = "2"; then
    CFLAGS="$CFLAGS `$somaconfigpath --cflags`"
    LIBS="$LIBS `$somaconfigpath --libs` -lsoma"
    AC_MSG_RESULT(yes)
  
  else
    AC_MSG_ERROR(Soma must be 2.x release!);
  fi
else
  AC_MSG_ERROR(No soma-config founded!);
fi

CFLAGS="$CFLAGS -Wall"

AC_OUTPUT([
Makefile
confuse/Makefile
src/Makefile
])
