#!/bin/sh
./configure CC=cc CFLAGS="-O" --with-aixsize=32 TCL_PREFIX=/mdl2/save/bin/tcltk TCL_VERSION=8.5
echo "This may not create tcldegrib/tkdegrib because those libraries may have been built in 64 bit mode"
