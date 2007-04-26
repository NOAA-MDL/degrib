#ifndef SOLAR_H
#define SOLAR_H

#include "type.h"

int sunTime (sInt4 year, int mon, int day, double lat, double lon,
             int f_sunrise, int *hh, int *mm, int *ss);

int isNightPeriod (double time, double lat, double lon, int zone);

#endif
