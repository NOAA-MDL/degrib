#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libaat], [1.0], [arthur.taylor@noaa.gov])
AC_CONFIG_SRCDIR([allocSprintf.c])
# AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROG(AR,ar,ar,:)

MEMWATCH_NAME="memwatch-2.71"
AC_SUBST([MEMWATCH_NAME])

AC_ARG_WITH([memwatch],
  [AS_HELP_STRING([--with-memwatch], [enable memory watching library.])],
  [],
  [with_memwatch=no])
AS_IF([test "x$with_memwatch" != xno],
  [
    AC_SUBST([MEM_CLEAN],"memwatch-clean")
    AC_SUBST([MEM_DISTCLEAN],"memwatch-distclean")
    AC_SUBST([MEM_DEF],"-DMEMWATCH -DMEMWATCH_STDIO")
# Single quotes are the key to the following.
    AC_SUBST([MEM_LIBDEP],'$(MEMWATCH_DIR)/libmemwatch.a')
    AC_SUBST([MEM_STDINC],'-I$(MEMWATCH_DIR)')
    AC_SUBST([MEM_STDLIB],'-L$(MEMWATCH_DIR) -lmemwatch')
  ])

AC_ARG_VAR([MEMWATCH_DIR],
           [Specify the location of memwatch library from libaat perspective])

# Provides --with-debug option
AC_ARG_WITH([debug],
  [AS_HELP_STRING([--with-debug],[set debug defines (default=no)])],
  [],
  [with_debug=no])
AS_IF([test "x$with_debug" != xno],
  [
    AC_SUBST(DDEBUG,["-DDEBUG"])
  ])

AS_IF([test x$at_c_cv_char_unsigned = xyes],
[
  AS_IF([test "$ac_compiler_gnu" = yes],
        [ CFLAGS="${CFLAGS} -fsigned-char"],
        [case $host in
          *-*-aix*) CFLAGS="${CFLAGS} -qchars=signed";;
          *-*-hp*)  CFLAGS="${CFLAGS} -signed";;
          esac
        ])
])

AC_ARG_VAR([ARFLAGS])
AC_ARG_ENABLE(aixsize,
  [  --enable-aixsize=val    build 32bit or 64bit (default 64) libraries for aix],
   [if test "$enableval" = 32; then
      if test "$ac_compiler_gnu" = yes; then
        CFLAGS="${CFLAGS} -maix32"
      else
        CFLAGS="${CFLAGS} -q32"
      fi
      FFLAGS="${FFLAGS} -q32"
      ARFLAGS="${ARFLAGS} -X32"
    else
      if test "$ac_compiler_gnu" = yes; then
        CFLAGS="${CFLAGS} -maix64"
      else
        CFLAGS="${CFLAGS} -q64"
      fi
      FFLAGS="${FFLAGS} -q64"
      ARFLAGS="${ARFLAGS} -X64"
    fi
    ],
    )
AC_SUBST(ARFLAGS,)

AC_C_BIGENDIAN([AC_SUBST(DWORDS_BIGENDIAN,"-DWORDS_BIGENDIAN")])

# For some reason on cygwin/mingw system there is an extra carriage return
# in ac_cv_sizeof_long_int?
AC_LONG_64_BITS
AC_CHECK_SIZEOF([long int])
AS_IF([test "x${LONG_64_BITS}" = "x"],
    [AC_SUBST(DSIZEOF_LONG_INT,"-DSIZEOF_LONG_INT=4")],
    [AC_SUBST(DSIZEOF_LONG_INT,"-DSIZEOF_LONG_INT=${ac_cv_sizeof_long_int}")])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor memset sqrt strchr strrchr strspn strtol])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
