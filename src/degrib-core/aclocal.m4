# DEGRIB aclocal.m4 : version 20070627

#####
# Set @DWORDS_BIGENDIAN@ variable.
# May want to use CPPFLAGS instead.
#####
AC_DEFUN([SET_BIGENDIAN],
[
  AC_C_BIGENDIAN([AC_SUBST([DWORDS_BIGENDIAN],"-DWORDS_BIGENDIAN")])
])

#####
# Set @DSIZEOF_LONG_INT@ variable.
# May want to use CPPFLAGS instead.
#####
AC_DEFUN([SET_SIZEOF_LONGINT],
[
  # For some reason on cygwin/mingw system there is an extra carriage return
  # in ac_cv_sizeof_long_int?
  case $host in
    *-*-cygwin*|*-*-mingw*)
      AC_SUBST([DSIZEOF_LONG_INT],"-DSIZEOF_LONG_INT=4");;
    *)
      AC_CHECK_SIZEOF([long int])
      AC_SUBST([DSIZEOF_LONG_INT],"-DSIZEOF_LONG_INT=${ac_cv_sizeof_long_int}");;
  esac
])

#####
# Set @ANSIFLAG@ variable.
# Set @C99FLAG@ variable.
# Adjusts: CFLAGS (preset output variable)
#####
AC_DEFUN([SET_ANSIFLAG],
[
  AS_IF([test "$ac_cv_c_compiler_gnu" = yes],
    [
     CFLAGS="${CFLAGS} -Wall"
     # -ansi is not allowed in linux if myutil.c::MyStat uses "S_IFDIR", etc.
     ANSIFLAG="-pedantic"
     C99FLAG="-pedantic"
     case $host in
       *-*-cygwin*|*-*-mingw*)
         ANSIFLAG="-ansi -pedantic"
         C99FLAG="-std=c99 -pedantic";;
     esac
    ],[
     ANSIFLAG=""
     C99FLAG=""
     case $host in
       *-*-aix*)
         CFLAGS="${CFLAGS} -qcpluscmt"
         C99FLAG="-qlanglvl=stdc99";;
       *-*-hp*)
         CFLAGS="${CFLAGS} -Aa"
         C99FLAG="-c99";;
     esac
    ])
  AC_SUBST([ANSIFLAG])
  AC_SUBST([C99FLAG])
])

#####
# Set @DWINDOWS@ variable.
#####
AC_DEFUN([SET_DWINDOWS],
[
  DWINDOWS=""
  case $host in
    *-*-cygwin*|*-*-mingw*)
      DWINDOWS="-D_WINDOWS_";;
  esac
  AC_SUBST([DWINDOWS])
])

#####
# Set @DYNAMIC_LIB@ variable.
# may not properly handle the hpgcc case of -ldld
#####
AC_DEFUN([SET_DYNAMIC_LIB],
[
  DYNAMIC_LIB="-ldl"
  case $host in
    *-*-cygwin*|*-*-mingw*)
      DYNAMIC_LIB="";;
  esac
  AC_SUBST([DYNAMIC_LIB])
])

#####
# Adds the flags to CFLAGS to compile using signed char.
# Needs: AC_CANONICAL_HOST
# Adjusts: CFLAGS (preset output variable)
#####
AC_DEFUN([AAT_MYSIGN],
[
  AC_C_CHAR_UNSIGNED()
  AS_IF([test x$at_c_cv_char_unsigned = xyes],
    [AS_IF([test "$ac_cv_c_compiler_gnu" = yes],
      [CFLAGS="${CFLAGS} -fsigned-char"],
      [case $host in
         *-*-aix*) CFLAGS="${CFLAGS} -qchars=signed";;
         *-*-hp*)  CFLAGS="${CFLAGS} -signed";;
         esac
      ])
    ])
])

#####
# Provides --with-cygwin option (Default is --without-cygwin)
# Adjusts: CFLAGS (preset output variable)
#####
AC_DEFUN([OPT_CYGWIN],
[
  AC_ARG_WITH([cygwin],
    [AS_HELP_STRING([--with-cygwin],[Create executables which use cygwin1.dll])],
    [],
    [with_cygwin=no])
  AS_IF([test "x$with_cygwin" == xno],
    [case "$build" in
       *-*-cygwin*)
         AS_IF([test "$ac_cv_c_compiler_gnu" = yes],
               [CFLAGS="${CFLAGS} -mno-cygwin"]);;
       esac
    ])
])

#####
# Provides --with-memwatch option (Default is --without-memwatch)
# Assumes $(top_srcdir) is set in the Makefile.in
# Set @MEM_CLEAN@ variable
# Set @MEM_DEF@ variable
# Set @MEM_NAME@ variable
# Set @MEM_LIBDEP@ variable
# Set @MEM_STDINC@ variable
# Set @MEM_STDLIB@ variable
#####
AC_DEFUN([OPT_MEMWATCH],
[
  AC_ARG_WITH([memwatch],
    [AS_HELP_STRING([--with-memwatch], [enable memory watching library.])],
    [],
    [with_memwatch=no])
  AS_IF([test "x$with_memwatch" != xno],
    [
      AC_SUBST([MEM_CLEAN],"libmemwatch-clean")
      AC_SUBST([MEM_DEF],"-DMEMWATCH -DMEMWATCH_STDIO")
  # Single quotes are the key to the following.
      AC_SUBST([MEM_NAME],'memwatch-2.71')
      AC_SUBST([MEM_LIBDEP],'$(top_srcdir)/memwatch-2.71/libmemwatch.a')
      AC_SUBST([MEM_STDINC],'-I$(top_srcdir)/memwatch-2.71')
      AC_SUBST([MEM_STDLIB],'-L$(top_srcdir)/memwatch-2.71 -lmemwatch')
    ])
])

#####
# Provides --without-strip Option (Default is --with-strip)
# Make sure you call AC_SUBST([STRIP])
#####
AC_DEFUN([OPT_NOSTRIP],
[
  AC_ARG_WITH([strip],
    [AS_HELP_STRING([--without-strip], [disable symbol stripping in binaries.])],
    [],
    [with_strip=yes])
  STRIP=:
    AS_IF([test "x$with_strip" != xno],
      [
        AC_PATH_PROG([STRIP], [strip],[:])
      ])
])

#####
# Provides --enable-aixsize option (32 or 64 bit compiling (default 64) for AIX
# Adjusts: CFLAGS (preset output variable)
# Adjusts: FFLAGS (preset output variable)
# Make sure you call AC_SUBST(ARFLAGS)
#####
AC_DEFUN([OPT_AIXSIZE],
[
  AC_ARG_ENABLE(aixsize,
    [AS_HELP_STRING([--enable-aixsize=val], [build val-bit (32 or 64, default 64) libraries for aix])],
    [case ${enableval}x in
       32x)
         AS_IF([test "$ac_cv_c_compiler_gnu" = yes],
           [CFLAGS="${CFLAGS} -maix32"],
           [CFLAGS="${CFLAGS} -q32"])
         FFLAGS="${FFLAGS} -q32"
         ARFLAGS="${ARFLAGS} -X32";;
       64x)
         AS_IF([test "$ac_cv_c_compiler_gnu" = yes],
           [CFLAGS="${CFLAGS} -maix64"],
           [CFLAGS="${CFLAGS} -q64"])
         FFLAGS="${FFLAGS} -q64"
         ARFLAGS="${ARFLAGS} -X64";;
       *)
         AC_MSG_WARN([Invalid aixsize value- ${enableval}])
       esac
    ],)
])

#####
# Splits a full_path into component parts.
# IF "Prefix" == at_temp, then set at_temp to stuff before last / in FULL_PATH
#####
AC_DEFUN([ADT_SPLIT],
[
dnl  #ADT_SPLIT(FULL_PATH,PREFIX,LAST_PART)
  AS_IF([test "$2" != ""],
    [AS_IF([test "$2" = "at_temp" || test "${$2}" = ""],
      [$2=${$1%/*}])
    ])
  if test "$3" != ""
  then
    AS_IF( [ test "$3" = "at_temp" || test "${$3}" = ""],
      [ $3=${$1##*/} ] )
  fi
])

#####
# Attempt to set up Tcl/Tk libraries on the current system.
# Possible adjustment to TCL_PREFIX
# Possible adjustment to TK_PREFIX
# Calls AC_SUBST([TK_LIBS])
# Calls AC_SUBST([TCL_LIBS])
# Make sure you call AC_SUBST(LD_FLAGS)
#####
AC_DEFUN([AT_PROG_TCL],
[ if test "$TCL_PREFIX" = ""
  then
    AC_MSG_CHECKING([Tcl/Tk directories])
    AC_PATH_PROG([TCL_BINARY],[tclsh],[not_found],[$PATH:/usr/bin:/usr/local/bin:/usrx/local/bin:/usr/local/tcltk/bin])
    AC_PATH_PROG([TK_BINARY],[wish],[not_found],[$PATH:/usr/bin:/usr/local/bin:/usrx/local/bin:/usr/local/tcltk/bin])
    if test "$TCL_BINARY" != "not_found"
    then
      ADT_SPLIT([TCL_BINARY],[at_temp])
      ADT_SPLIT([at_temp],[TCL_PREFIX])
      ADT_SPLIT([TK_BINARY],[at_temp])
      ADT_SPLIT([at_temp],[TK_PREFIX])
    else
      TK_PREFIX="not_found"
      TCL_PREFIX="not_found"
    fi
    AC_MSG_RESULT([ TCL_PREFIX = $TCL_PREFIX, TK_PREFIX = $TK_PREFIX])
    AC_MSG_CHECKING([Tcl/Tk Libraries])
    tk_lib_dota=not_found
    tk_lib_stubs=not_found
    tk_lib_dotso=not_found
    for adtval in $TK_PREFIX/lib/libtk*
    do
     case "$adtval" in
     *stub*)
       tk_lib_stubs=$adtval;;
      *.a)
       tk_lib_dota=$adtval;;
      *.so)
       tk_lib_dotso=$adtval;;
      esac
    done
    tcl_lib_dota="not_found"
    tcl_lib_stubs="not_found"
    tcl_lib_dotso="not_found"
    for adtval in $TCL_PREFIX/lib/libtcl*
    do
     case "$adtval" in
     *stub*)
       tcl_lib_stubs=$adtval;;
      *.a)
       tcl_lib_dota=$adtval;;
      *.so)
       tcl_lib_dotso=$adtval;;
      esac
    done
    if test $tcl_lib_dota != not_found
    then ADT_SPLIT([tcl_lib_dota],[at_temp])
      at_temp2=${tcl_lib_dota%*.a}
    elif test $tcl_lib_dotso != not_found
    then ADT_SPLIT([tcl_lib_dotso],[at_temp])
      at_temp2=${tcl_lib_dotso%*.so}
# Deal with Tcl shared libraries
      if test "$ac_cv_c_compiler_gnu" = yes; then
        case "$host" in
          *-*-aix*) LD_FLAGS="${LD_FLAGS} -Wl,-brtl";;
        esac
      else
        case "$host" in
          *-*-aix*) LD_FLAGS="${LD_FLAGS} -brtl";;
        esac
      fi
    fi
    TCL_LIBS="-L$at_temp -l${at_temp2##*/lib}"
    if test $tcl_lib_stubs != not_found
    then
       at_temp2=${tcl_lib_stubs%*.a}
    fi
    TCL_LIBS="${TCL_LIBS} -l${at_temp2##*/lib}"
    if test $tk_lib_dota != not_found
    then ADT_SPLIT([tk_lib_dota],[at_temp])
      at_temp2=${tk_lib_dota%*.a}
    elif test $tk_lib_dotso != not_found
    then ADT_SPLIT([tk_lib_dotso],[at_temp])
      at_temp2=${tk_lib_dotso%*.so}
    fi
    TK_LIBS="-L$at_temp -l${at_temp2##*/lib}"
    if test $tk_lib_stubs != not_found
    then
       at_temp2=${tk_lib_stubs%*.a}
    fi
    TK_LIBS="${TK_LIBS} -l${at_temp2##*/lib}"
    AC_SUBST([TK_LIBS])
    AC_SUBST([TCL_LIBS])
    AC_MSG_RESULT([TCL_LIB = "$TCL_LIBS", TK_LIBS = "$TK_LIBS"])
  else
    if test "$TK_PREFIX" = ""
    then
      TK_PREFIX=$TCL_PREFIX
    fi
    TCL_LIBS="-L$TCL_PREFIX/lib -ltcl$TCL_VERSION"
    TK_LIBS="-L$TK_PREFIX/lib -ltk$TCL_VERSION"
    AC_SUBST([TK_LIBS])
    AC_SUBST([TCL_LIBS])
  fi
])

#####
# Provides the --with-badtclssh option (Default is --without-badtclssh)
# This option allows for the bad exit With Tcl/Tk and SSH on NCEP's AIX
#####
AC_DEFUN([OPT_DBADTCLSSH],
[
  AC_ARG_WITH([badtclssh],
    [AS_HELP_STRING([--with-badtclssh], [enable special exit for Tcl/Tk.])],
    [],
    [with_badtclssh=no])
  DBAD_TCLSSH=""
  AS_IF([test "x$with_badtclssh" != xno],
    [case "$host" in
       *-*-aix*)
         DBAD_TCLSSH="-D_BAD_TCL_SSH_EXIT";;
       esac
    ])
  AC_SUBST([DBAD_TCLSSH])
])

#####
# Provides the --with-tkputs option (Default is --without-tkputs)
# This option allows for puts in MS-Windows from a Tk app (ie creates a console)
#####
AC_DEFUN([OPT_TKPUTS],
[
  AC_ARG_WITH([tkputs],
    [AS_HELP_STRING([--with-tkputs], [enable puts from a Tk app (ie create a console window)])],
    [],
    [with_tkputs=no])
  TCL_LDFLAGS=""
  TK_LDFLAGS=""
  case "$host" in
    *-*-cygwin*|*-*-mingw*)
      TCL_LDFLAGS="-mconsole"
      AS_IF([test "x$with_tkputs" != xno],
        [TK_LDFLAGS="-mconsole"],
        [TK_LDFLAGS="-mwindows"]);;
    esac
  AC_SUBST([TCL_LDFLAGS])
  AC_SUBST([TK_LDFLAGS])
])

#####
# Set @TCL_VERSION@ variable (current version of Tcl/Tk)
# Not needed since TK_LIBS and TCL_LIBS are already set.
#####
#AC_DEFUN([SET_TCL_VERSION],
#[if test "$TCL_VERSION" = ""
# then
#  AC_MSG_CHECKING([Tcl/Tk version])
#  if test "$TCL_BINARY" = "" || test "$TCL_BINARY" = "not_found"
#  then
#    TCL_VERSION="not_found"
#  else
#    TCL_VERSION=`echo puts \\$tcl_version | $TCL_BINARY`
#  fi
#  AC_MSG_RESULT([$TCL_VERSION])
# fi
#])






