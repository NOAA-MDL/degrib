#ifndef DRAW_H
#define DRAW_H

#include "meta.h"
#include "userparse.h"

int drawGrib (const char *Filename, double *grib_Data, const char *mapIniFile,
              const char *mapIniOptions, gdsType * gds, double Min,
              double Max, sChar f_missing, double missing,
              grib_MetaData * meta, userType * usr);

#endif
