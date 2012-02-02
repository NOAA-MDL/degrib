To make a distribution as of 9/12/2009:

cd distrib
./distrib.tcl 1.94 9/13/2009
./nsi.tcl 1.94 9/13/2009
cp degrib-install.exe ./webDir/download
./transfer.sh

On SLOSH... 
cd /www/html/degrib2/download
cp degrib-install.exe ./archive/degrib-install-197a.exe

The ./distrib.tcl guidance at the end is:
Recommend in degrib/src:
   A) svn update
   B) autoconf.sh
   C) config-win.sh
   D) make
Then rerun the distrib.tcl again

Add the full download URL to the Release notes

When finished, remember to tag it using:
svn copy -r747 http://slosh.nws.noaa.gov/svn/degrib/degrib
  http://slosh.nws.noaa.gov/svn/degrib/tags/20080724-1.92-747
  -m 'Tagging release 1.92 (revision 747)'

---------------------

To make a distribution:

From cygwin the following will create all the source distributions but not
affect the test suites:
  cd ~/prj/ndfd/degrib/distrib
  ./distrib.tcl 0.85

From cygwin the following will create all the test suites but not affect the
distributions.  (You may want to make sure it only grabs the files you want)

  First use e:/prj/ndfd/degrib/ans/clean.tcl to clean the text files of CR - LF.
    Easier for "diff" to handle LF.  clean.tcl automatically cleans all
    .txt .ave .prj and .hdr files in e:/prj/ndfd/degrib/ans/

  Then
    cd ~/prj/ndfd/degrib/distrib
    ./disttest.tcl

-----------

Test distribution via:
  cd ~/prj/ndfd/degrib.web/htdocs/download/archive
  cp <>.tar.gz ~/prj/ndfd/degrib.dst
  cd ~/prj/ndfd/degrib.dst
  tar -xzf <>.tar.gz
  cd ./degrib/src/grib2unpacker
  make
  cd ../cmapf-c
  make
  cd ../degrib
  make
  make install
Test program?
Use Sync to see if all files that you want are there.

-----------

Port distribution to other platforms.
  bd11-nhdw
  rainbow
  asp.ncep.noaa.gov
and run tests.

-----------

Get registred users using :
cd ~/prj/ndfd/degrib.web/getreg.tcl
