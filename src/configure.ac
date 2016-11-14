#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# DEGRIB configure.ac : version 20070705

AC_PREREQ(2.59)
AC_INIT([degrib],[2.08],[arthur.taylor@noaa.gov])
AC_SUBST(PACKAGE_DATE,'20161110')
AC_CONFIG_SRCDIR([Makefile.in])
AC_CONFIG_HEADERS([gd/config.h:gd/scripts/config.hin])
AC_CANONICAL_HOST()

# Checks for programs.
AC_PROG_CC()
AC_PROG_INSTALL()
AC_PROG_RANLIB()
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
  *-*-darwin*) STRIP_FLAGS="-SxX";;
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

# Set @DWINDOWS@ variable (drawshp doesn't need X or Tcl/Tk but does need this)
SET_DWINDOWS()

# Set @DYNAMIC_LIB@ variable.
SET_DYNAMIC_LIB()

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

# Provides --with-halo option (Default is --without-halo)
OPT_HALO()

# ................................
# ....Graphics Display configs....
# ................................

# Provides X linker flags to X_LIBS
# Probably should chek into X_CFLAGS
AC_PATH_XTRA()
if test "$X_LIBS" == ""
then
  case $host in
    *-*-aix*) X_LIBS="-L/usr/lib -lX11 -lXext";;
  esac
else
  X_LIBS="${X_LIBS} -lX11 -lXext"
fi

case $host in
  *-*-cygwin*|*-*-mingw*) X_LIBS="";;
esac

# ................................
# ....Tcl/Tk configurations   ....
# ................................

AC_ARG_VAR([TCL_PREFIX],[Base Directory for TCL/TK])
AC_ARG_VAR([TK_PREFIX],[Base Directory for TCL/TK])
AC_ARG_VAR([TCL_VERSION],[Current version of Tcl/Tk (8.3, 8.4, or 8.5)])

# Attempt to set up Tcl/Tk libraries on the current system.
# Possible adjustment to TCL_PREFIX
# Possible adjustment to TK_PREFIX
# Calls AC_SUBST([TK_LIBS])
# Calls AC_SUBST([TCL_LIBS])
# Make sure you call AC_SUBST(LD_FLAGS)
AT_PROG_TCL()

# Provides the --with-badtclssh option (Default is --without-badtclssh)
# This option allows for the bad exit With Tcl/Tk and SSH on NCEP's AIX
OPT_DBADTCLSSH()

# Provides the --with-tkputs option (Default is --without-tkputs)
# This option allows for puts in MS-Windows from a Tk app (ie creates a console)
OPT_TKPUTS()

# Set @TCL_VERSION@ variable (current version of Tcl/Tk)
# Not needed since TK_LIBS and TCL_LIBS are already set.
AC_ARG_VAR([TCL_VERSION],[Version of TCL/TK])
SET_TCL_VERSION()

SET_MINIZIP32()
SET_DLLEXT()

# ................................
# ....Local modifications     ....
# ................................
AC_ARG_WITH([zlib],
  [AS_HELP_STRING([--with-zlib], [Path to the include directory for zlib])],
  [AC_SUBST([ZLIB_PATH],$with_zlib)],
  [AC_SUBST([ZLIB_PATH],../zlib)])

AC_ARG_WITH([libpng],
  [AS_HELP_STRING([--with-libpng], [Path to the include directory for libpng])],
  [AC_SUBST([LIBPNG_PATH],$with_libpng)],
  [AC_SUBST([LIBPNG_PATH],../libpng)])

AC_ARG_WITH([libaat],
  [AS_HELP_STRING([--with-libaat], [Path to the include directory for libaat])],
  [AC_SUBST([LIBAAT_PATH],$with_libaat)],
  [AC_SUBST([LIBAAT_PATH],../libaat)])

AC_ARG_WITH([jpeg2000],
  [AS_HELP_STRING([--with-jpeg2000], [Path to the include directory for jpeg2000])],
  [AC_SUBST([JPEG2000_PATH],$with_jpeg2000)],
  [AC_SUBST([JPEG2000_PATH],../jpeg2000/src/libjasper/include)])

# Provides --with-debug option
AC_ARG_WITH([debug],
  [AS_HELP_STRING([--with-debug],[set debug defines (default=no)])],
  [],
  [with_debug=no])
AS_IF([test "x$with_debug" != xno],
  [
    AC_SUBST(DDEBUG,["-DDEBUG"])
  ])

# Provides --with-extradebug option
AC_ARG_WITH([extradebug],
  [AS_HELP_STRING([--with-extradebug],[set extra debug defines (default=no)])],
  [],
  [with_extradebug=no])
AS_IF([test "x$with_extradebug" != xno],
  [
    AC_SUBST(PNGDEBUG,["-g -DPNG_DEBUG=5"])
    AC_SUBST(ZLIB_DEBUG,["-g -DDEBUG"])
  ])


# Handle JPEG2000 Libtool fix
JPEG2000_LIBTOOL_FIX=""
case $host in
  *-*-cygwin*|*-*-mingw*)
    JPEG2000_LIBTOOL_FIX="cp ./jpeg2000/libtool.mingw ./jpeg2000/libtool";;
  esac
AC_SUBST([JPEG2000_LIBTOOL_FIX])

# (For AIX only) Set the max dynamic memory data size to 0x50000000
case $host in
  *-*-aix*) LD_FLAGS="${LD_FLAGS} -bmaxdata:0x50000000";;
esac

# Perform "Substs"
AC_SUBST([STRIP])
AC_SUBST([STRIP_FLAGS])
AC_SUBST([ARFLAGS])
AC_SUBST([LD_FLAGS])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([zlib/Makefile:zlib/scripts/Makefile.in])
AC_CONFIG_FILES([zlib/contrib/minizip/Makefile])
AC_CONFIG_FILES([libpng/Makefile:libpng/scripts/Makefile.in])
AC_CONFIG_FILES([libaat/Makefile])
AC_CONFIG_FILES([mdl_g2c/Makefile])
AC_CONFIG_FILES([gd/Makefile:gd/scripts/Makefile.in])
AC_CONFIG_FILES([degrib/Makefile])
AC_CONFIG_FILES([xyz2shp/Makefile])
AC_OUTPUT()
