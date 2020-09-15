#!/bin/bash

wget https://eternallybored.org/misc/wget/1.20.3/32/wget.exe

# Test ...
./wget.exe "https://graphical.weather.gov/xml/gribcut.php?var=apt&lat1=30&lon1=-100&lat2=50&lon2=-80" -O a
