#!/bin/sh

echo "When first setting up in mingw, you may need to"
echo "use tar -xzf libxml.tar.gz to get the correct timestamps"
echo "on the libxml files"

echo ""
echo "After that, make -f makefile.win should work the first time"
echo ""
echo "to get make -f makefile.win to work without having to do:"
echo "  make -f makefile.win clean"
echo "the second time, do the following:"
echo ""
echo "cp ./jpeg2000/src/libjasper/base/_deps/mingw/* ./jpeg2000/src/libjasper/base/.deps"
echo "cp ./jpeg2000/src/libjasper/jpc/_deps/mingw/* ./jpeg2000/src/libjasper/jpc/.deps"
echo "cp ./jpeg2000/libtool.mingw ./jpeg2000/libtool"
echo ""
echo "cp ./libxml/_deps/mingw/* ./libxml/.deps"
echo ""

# cp ./jpeg2000/src/libjasper/base/_deps/mingw/* ./jpeg2000/src/libjasper/base/.deps
# cp ./jpeg2000/src/libjasper/jpc/_deps/mingw/* ./jpeg2000/src/libjasper/jpc/.deps
# cp ./jpeg2000/libtool.mingw ./jpeg2000/libtool
# cp ./libxml/_deps/mingw/* ./libxml/.deps

