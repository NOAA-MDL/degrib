#!/bin/sh
@echo "Remember to remove mingw from your path"
./configure CFLAGS="-O3" --with-cygwin
