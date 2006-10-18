AC_DEFUN([ADT_SPLIT],
[
dnl  #ADT_SPLIT(FULL_PATH,PREFIX,LAST_PART)
  AS_IF([ test "$2" != ""],
    [ AS_IF([ test "$2" = "at_temp" || test "${$2}" = ""], 
      [ $2=${$1%/*}]) 
    ] )
  if test "$3" != ""
  then
    AS_IF( [ test "$3" = "at_temp" || test "${$3}" = ""],
      [ $3=${$1##*/} ] )
  fi
])

AC_DEFUN([ADT_SHLIB],
  [ if test "$ac_cv_c_compiler_gnu" = yes; then
      case "$host" in
      *-*-aix*)
        LD_FLAGS="${LD_FLAGS} -Wl,-brtl";;
      esac
    else
      case "$host" in
      *-*-aix*)
        LD_FLAGS="${LD_FLAGS} -brtl";;
      esac
    fi]
)

AC_DEFUN([ADT_MYSIGN],
[ 
  AS_IF([test x$at_c_cv_char_unsigned = xyes],
        [
          AS_IF([test "$ac_compiler_gnu" = yes],
                [ CSGFLAGS="-fsigned-char"],
                [case $host in
                   *-*-aix*) CSGFLAGS="-qchars=signed";;
                   *-*-hp*)  CSGFLAGS="-signed";;
                 esac 
                ])
        ])
])

AC_DEFUN([AT_TCL_VERSION],
[ if test "$TCL_VERSION" = ""
  then
    AC_MSG_CHECKING([Tcl/Tk version])
    if test "$TCL_BINARY" = "" || test "$TCL_BINARY" = "not_found"
    then
      TCL_VERSION="not_found"
    else
      TCL_VERSION=`echo puts \\$tcl_version | $TCL_BINARY`
    fi
    AC_MSG_RESULT([$TCL_VERSION])
  fi
])

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
      ADT_SHLIB
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

AC_DEFUN([ADT_SPECFLAGS],
[
  if test "$ac_cv_c_compiler_gnu" = yes; then
#    CFLAGS="${CFLAGS} -Wall -pedantic"
    CFLAGS="${CFLAGS} -Wall"
# -ansi is not allowed in linux if myutil.c::MyStat uses "S_IFDIR", etc.
    ANSIFLAG="-pedantic"
    case $host in
      *-*-cygwin*|*-*-mingw*)
# -ansi seems to be needed for mingw to link with gd?
        ANSIFLAG="-ansi -pedantic"
        ;;
    esac
  else
    ANSIFLAG=""
    case $host in
      *-*-aix*) CFLAGS="${CFLAGS} -qcpluscmt -qmaxmem=8192";;
      *-*-hp*) CFLAGS="${CFLAGS} -Aa";;
    esac
  fi
  AC_SUBST([ANSIFLAG])
  if test "$ac_cv_f77_compiler_gnu" = no; then
    case $host in
      *-*-aix*)
      FFLAGS="${FFLAGS} -qstrict -qfixed -qarch=auto -qintlog -qintsize=4"
      FFLAGS="${FFLAGS} -qmaxmem=8192  -qrealsize=4";;
    esac
  fi
])

AC_DEFUN([AAT_JPEG2000_LIBTOOL_FIX],
[
  JPEG2000_LIBTOOL_FIX=""
  case $host in
    *-*-cygwin*|*-*-mingw*)
      JPEG2000_LIBTOOL_FIX="cp ./jpeg2000/libtool.mingw ./jpeg2000/libtool"
      ;;
  esac
  AC_SUBST([JPEG2000_LIBTOOL_FIX])
]
)

AC_DEFUN([AAT_DWINDOWS],
[
  DWINDOWS=""
  case $host in
    *-*-cygwin*|*-*-mingw*)
#      MNOCYGWIN="-mno-cygwin"
      DWINDOWS="-D_WINDOWS_"
#      AC_DEFINE(_WINDOWS_)
      ;;
  esac
  AC_SUBST([DWINDOWS])
]
)

AC_DEFUN([AAT_XLIBS],
[
  if test "$X_LIBS" != ""
  then
    X_LIBS="${X_LIBS} -lX11"
  fi
]
)

AC_DEFUN([AAT_DYNAMIC_LIB],
[
  DYNAMIC_LIB="-ldl"
  case $host in
    *-*-cygwin*|*-*-mingw*)
      DYNAMIC_LIB=""
      ;;
  esac
  AC_SUBST([DYNAMIC_LIB])
]
)

AC_DEFUN([AAT_DBADTCLSSH],
[
  DBAD_TCLSSH=""
  case "$host" in
  *-*-aix*)
    DBAD_TCLSSH="-D_BAD_TCL_SSH_EXIT";;
  esac
  AC_SUBST([DBAD_TCLSSH])
]
)

AC_DEFUN([ADT_AIXSIZE],[

AC_ARG_ENABLE(aixsize,
  [AS_HELP_STRING([--enable-aixsize=val],
         [build val-bit (32 or 64, default 64) libraries for aix])],
   [at_c_savcflags=${CFLAGS}
    at_c_savarflags=${ARFLAGS}
    case ${enableval}x in
    32x)
      if test "$ac_compiler_gnu" = yes; then
        CFLAGS="${CFLAGS} -maix32"
      else
        CFLAGS="${CFLAGS} -q32"
      fi
      FFLAGS="${FFLAGS} -q32"
      ARFLAGS="${ARFLAGS} -X32";;
    64x)
      if test "$ac_compiler_gnu" = yes; then
        CFLAGS="${CFLAGS} -maix64"
      else
        CFLAGS="${CFLAGS} -q64"
      fi
      FFLAGS="${FFLAGS} -q64"
      ARFLAGS="${ARFLAGS} -X64";;
    *)
      AC_MSG_WARN([Invalid aixsize value- ${enableval}])
    esac
    ],
    )
])
