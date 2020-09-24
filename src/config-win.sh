#!/bin/sh
./configure CFLAGS="-O3" TCL_PREFIX=../tcl853_include_win TCL_VERSION=85

echo ""
echo "NOTE: for MinGW we have the following hacks"
echo ""
echo "1) Error in /src/jpeg2000 :: "
echo "   - aclocal: macro '_LT_COMPILER_PIC' required but not defined"
echo "   - aclocal: macro '_LT_DECL_SED' required but not defined"
echo "Solution:"
echo "   $ cd /src/jpeg2000"
echo "   $ vi Makefile"
echo "   >> change 'run aclocal-1.6' to 'run aclocal-1.6a'"
echo "   $ make"
echo ""
echo "2) Error in /src/jpeg2000 :: "
echo "   - lib /OUT:.libs/libbase.lib  jas_cm.o ..."
echo "   - ../../../libtool: line 4401: lib: command not found"
echo "Solution:"
echo "   $ cp /src/jpeg2000/libtool.mingw /src/jpeg2000/libtool"
echo "   $ cd /src"
echo "   $ make"
echo ""
echo "3) Error in /src/libxml :: "
echo "   - ./libtool: line 1092: lib: command not found"
echo "Solution:"
echo "   $ cp /src/libxml/libtool.mingw /src/libxml/libtool"
echo "   $ make"

# ./configure CFLAGS="-O3" TCL_PREFIX=../tcl853_include_win TCL_VERSION=85 --with-halo

#./configure CFLAGS="-O3" TCL_PREFIX=c:/Tcl859 TCL_VERSION=85
# ./configure CFLAGS="-O3" TCL_PREFIX=c:/tcl832 TCL_VERSION=83 --with-halo
