# Tcl package index file, version 1.0
# This file is generated by the "pkg_mkIndex" command
# and sourced either when an application starts up or
# by a "package unknown" script.  It invokes the
# "package ifneeded" command to set up package-related
# information so that packages will be loaded automatically
# in response to "package require" commands.  When this
# script is sourced, the variable $dir must contain the
# full path name of this file's directory.

global tcl_platform
if {$tcl_platform(platform)=="windows"} {
  package ifneeded grib2 1.0 \
    [list tclPkgSetup $dir grib2 1.0 {{libdegrib85.dll load { \
       Grib2Init \
  }}}]
} else {
  package ifneeded grib2 1.0 \
    [list tclPkgSetup $dir grib2 1.0 {{libdegrib8.4.so load { \
       Grib2Init \
  }}}]
}
