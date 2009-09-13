#!/bin/sh

sftp tayloraa@slosh.nws.noaa.gov <<EOF
cd /www/html/degrib2
lcd webDir
put degrib.txt
put history.txt
put tkdegrib.txt
cd download
lcd download
put degrib-all.exe
put degrib-all.tar.gz
put degrib-bin.exe
put degrib-bin.tar.gz
put degrib-install.exe
put degrib-src.tar
put degrib-src.tar.gz
put version.txt
cd archive
lcd archive
put degrib-*.tar.gz
exit 
EOF
