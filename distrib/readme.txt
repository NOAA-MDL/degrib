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
