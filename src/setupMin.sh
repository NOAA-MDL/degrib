#!/bin/sh
echo 'first try to do make -f makefile.win'
echo "if that doesn't work then try this"
mkdir ./jpeg2000/src/libjasper/base/.deps
cp ./jpeg2000/src/libjasper/base/_deps/mingw/* ./jpeg2000/src/libjasper/base/.deps
mkdir ./jpeg2000/src/libjasper/jpc/.deps
cp ./jpeg2000/src/libjasper/jpc/_deps/mingw/* ./jpeg2000/src/libjasper/jpc/.deps
cp ./jpeg2000/libtool.mingw ./jpeg2000/libtool
mkdir ./libxml/.deps
cp ./libxml/_deps/mingw/* ./libxml/.deps

echo 'libxml appears to have a time dependence on its files'
echo 'so you may need the distribution copy in order to get the right times'
