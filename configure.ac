dnl REQUIRE AUTOCONF 2.50 OR HIGHER...
AC_PREREQ(2.63) 

dnl
dnl Process this file with autoconf to produce a configure script.
dnl
AC_INIT(
  m4_esyscmd_s([sed -ne 's/.*program_name.*"\(.*\)"\;/\1/p' src/version.c]), 
  m4_esyscmd_s([sed -ne 's/.*version_string.*"\(.*\)"\;/\1/p' src/version.c]),
) 
AC_CONFIG_AUX_DIR(utils)  
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS(include/config.h)

dnl canonicalize the host
AC_CANONICAL_HOST

DATE=`${CONFIG_SHELL} ${srcdir}/utils/mkstamp`
VERSION=`sed -ne 's/.*version_string.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
PROGRAM=`sed -ne 's/.*program_name.*"\(.*\)"\;/\1/p' src/version.c`
AUTHOR=`sed -ne 's/.*author_name.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
EMAIL=`sed -ne 's/.*email_address.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
YEARS=`sed -ne 's/.*years.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
PLATFORM="${host_vendor}-${host_cpu}-${host_os}"
AC_SUBST(DATE)
AC_SUBST(PROGRAM)
AC_SUBST(AUTHOR)
AC_SUBST(EMAIL)
AC_SUBST(PLATFORM)
 
dnl Checks for programs.
AC_PROG_CC 

WARN_CFLAGS="-W -Wall "
AC_SUBST(WARN_CFLAGS)


AM_PROG_LIBTOOL 

dnl
dnl Program support
dnl
AC_PATH_PROG( PERL, perl, false )
AC_PROG_SHELL
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_C_INLINE
AC_CHECK_TYPE(int8_t,   char)
AC_CHECK_TYPE(int16_t,  short)
AC_CHECK_TYPE(int32_t,  int)
AC_CHECK_TYPE(int64_t,  long long)
AC_CHECK_TYPE(uint8_t,  unsigned char)
AC_CHECK_TYPE(uint16_t, unsigned short)
AC_CHECK_TYPE(uint32_t, unsigned int)
AC_CHECK_TYPE(uint64_t, unsigned long long)
AC_CHECK_TYPE(u_int32_t,unsigned int)
AC_CHECK_TYPE(ssize_t,  int)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(limits.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(sys/select.h)
AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADERS(sys/time.h sys/times.h)
AC_CHECK_HEADERS(sys/resource.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(arpa/inet.h)
AC_CHECK_HEADERS(netinet/in.h)
AC_CHECK_HEADERS(netdb.h)
AC_CHECK_HEADERS(pthread.h) 
AC_CHECK_HEADERS(string.h) 
AC_CHECK_HEADERS(strings.h) 
AC_CHECK_HEADERS(sched.h) 
AC_CHECK_HEADERS(openssl/e_os.h)
AC_CHECK_HEADERS(openssl/e_os2.h)

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl
dnl Return type of signal-handlers
dnl
AC_TYPE_SIGNAL

dnl
dnl Checks for library functions.
dnl
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(strchr memcpy strncpy strstr strlen)
AC_CHECK_FUNCS(strncasecmp strncmp)
AC_CHECK_FUNCS(socket, , AC_CHECK_LIB(socket, socket))
AC_CHECK_FUNCS(gethostbyname, , AC_CHECK_LIB(nsl, gethostbyname))
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(rand_r)
AC_CHECK_FUNCS(localtime_r)
AC_CHECK_FUNCS(gmtime_r)
AC_CHECK_FUNCS(getipnodebyname)
AC_CHECK_FUNCS(freehostent)
AC_CHECK_FUNCS(getopt_long)
AC_CHECK_FUNCS(poll)

dnl
dnl Check for socket library
dnl
AC_CHECK_LIB(socket, socket)

dnl
dnl update dates and versioning in doc
dnl
AC_OUTPUT_COMMANDS([
  for file in doc/ajping.1 ;
  do
    rm -f $file
    sed -e "s|\$_VERSION|$VERSION|g" \
        -e "s|\$_PROGRAM|$PROGRAM|g" \
        -e "s|\$_DATE|$DATE|g" \
        -e "s|\$_AUTHOR|$AUTHOR|g" \
        -e "s|\$_YEARS|$YEARS|g" \
        -e "s|\$_EMAIL|$EMAIL|g" \
        < $file.in > $file
  done
], [ VERSION=$VERSION DATE="$DATE" PROGRAM="$PROGRAM" AUTHOR="$AUTHOR" EMAIL="$EMAIL" ])

AC_OUTPUT(Makefile src/Makefile include/Makefile include/joedog/Makefile doc/Makefile utils/Makefile)

