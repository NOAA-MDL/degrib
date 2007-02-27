AC_INIT([Conformal Map Transforms - ellipsoidal],[3.1],[albion.taylor@noaa.gov])
AC_CONFIG_SRCDIR([stlmbr.c])
AC_CONFIG_HEADERS([cmapf.h:cmapf.hin])
AC_PROG_CC()
AC_PROG_INSTALL()
AC_PROG_RANLIB()
AC_PROG_F77
AC_F77_WRAPPERS
AC_CHECK_PROG(AR,ar,ar,:)
AC_ARG_VAR([ARFLAGS])
AC_CHECK_FUNCS(strcasecmp)
AC_ARG_ENABLE(aixsize,
  [  --enable-aixsize=val     build val-bit (32 or 64, default 64)libraries for aix],
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
AC_OUTPUT(Makefile)
