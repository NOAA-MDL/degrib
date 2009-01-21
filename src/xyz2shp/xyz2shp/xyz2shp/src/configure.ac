#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# DEGRIB configure.ac : version 20070709

AC_PREREQ(2.59)
AC_INIT([xyz2shp],[0.5],[arthur.taylor@noaa.gov])
AC_SUBST(PACKAGE_DATE,'20080915')
AC_CONFIG_SRCDIR([Makefile.in])
AC_CANONICAL_HOST()

# Checks for programs.
AC_PROG_CC
#AC_PROG_INSTALL()
AC_PROG_RANLIB
AC_CHECK_PROG(AR,ar,ar,:)
AC_PROG_MAKE_SET

# Set @DWORDS_BIGENDIAN@ variable.
# May want to use CPPFLAGS instead.
SET_BIGENDIAN()

# Provides --enable-aixsize option (32 or 64 bit compiling (default 64) for AIX
# Make sure you call AC_SUBST(ARFLAGS)
OPT_AIXSIZE()

case "$host" in
  *-*-aix*) STRIP_FLAGS="-X32_64";;
  *) STRIP_FLAGS="-s";;
esac

# Set @DSIZEOF_LONG_INT@ variable.
# May want to use CPPFLAGS instead.
# This should be after "OPT_AIXSIZE"
SET_SIZEOF_LONGINT()

# Set @ANSIFLAG@ variable.
# Set @C99FLAG@ variable.
# Also sets -Wall in CFLAGS.
SET_ANSIFLAG()

# Adds the flags to CFLAGS to compile using signed char.
AAT_MYSIGN()

# Provides --with-cygwin option (Default is --without-cygwin)
OPT_CYGWIN()

# Provides --without-strip Option (Default is --with-strip)
# Make sure you call AC_SUBST(STRIP)
OPT_NOSTRIP()

# Provides --with-memwatch option (Default is --without-memwatch)
# Assumes $(top_srcdir) is set in the Makefile.in
# Set @MEM_CLEAN@ variable
# Set @MEM_DEF@ variable
# Set @MEM_NAME@ variable
# Set @MEM_LIBDEP@ variable
# Set @MEM_STDINC@ variable
# Set @MEM_STDLIB@ variable
OPT_MEMWATCH()

# Provides --with-debug option
AC_ARG_WITH([debug],
  [AS_HELP_STRING([--with-debug],[set debug defines (default=no)])],
  [],
  [with_debug=no])
AS_IF([test "x$with_debug" != xno],
  [
    AC_SUBST(DDEBUG,["-DDEBUG"])
  ])

# Perform "Substs"
AC_SUBST([STRIP])
AC_SUBST([STRIP_FLAGS])
AC_SUBST([ARFLAGS])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libaat/Makefile])
AC_CONFIG_FILES([xyz2shp/Makefile])
AC_OUTPUT()
