#!/bin/sh

if [[ -z $2 ]] || [[ ! -z $3 ]]
then
  echo "usage: $0 <version> <date>"
  echo "Recent Releases are:"
  grep "Release:" ../NEWS.txt
  exit 1
fi

version=$1
date=$2

#
# Update NEWS file (aka history.txt)
#
echo "...Updating NEWS (aka history.txt) file..."
echo '#' > ../NEWS.tmp
echo '#----- Release: Version' $version $date ----- >> ../NEWS.tmp
echo '#' >> ../NEWS.tmp
head -n3 ../NEWS.txt > ../NEWS.tmp2
test=`diff ../NEWS.tmp ../NEWS.tmp2`
if [[ -n "$test" ]]
then
  new=yes
  cat ../NEWS.txt >> ../NEWS.tmp
  mv ../NEWS.tmp ../NEWS.txt
  rm ../NEWS.tmp2
else
  rm ../NEWS.tmp
  rm ../NEWS.tmp2
fi

#
# Update configure.ac
#
if [[ -n "$new" ]]
then
  echo "...Updating configure.ac file..."
  # Determine old version number with no []
  ans=`grep AC_INIT configure.ac | cut -d, -f2 | cut -d[ -f2 | cut -d] -f1`
  # Determine old date
  oldDate=`grep PACKAGE_DATE configure.ac | cut -d"'" -f2 | cut -d"'" -f1`
  # Substitute the data.
  sed -e '/AC_INIT/ s/'${ans}'/'${version}'/' \
      -e '/PACKAGE_DATE/ s/'${oldDate}'/'${date}'/' configure.ac > configure.ac.tmp
#  head -n5 configure.ac > configure.ac.tmp
#  echo 'AC_INIT([rexout],['$version'],[arthur.taylor@noaa.gov])' >> configure.ac.tmp
#  echo "AC_SUBST(PACKAGE_DATE,'"$date"')" >> configure.ac.tmp
#  cat configure.ac | sed -n -e '8,$ p' >> configure.ac.tmp
  mv configure.ac.tmp configure.ac
  echo "...Re-creating configure..."
  autoconf
fi

#
# Reconfigure the program
#
echo "...Re-configuring the program..."
config-win.sh

#
# Rebuild the program and create the release
#
echo "...Rebuilding the program and creating the release..."
make clean
make release

echo "...Recommend: Type 'make distclean' prior to svn status..."
