#ifndef PACK_H
#define PACK_H

#include "meta.h"
#include "degrib2.h"

int WriteGrib2Record (grib_MetaData *meta, double *Grib_Data,
                      sInt4 grib_DataLen, IS_dataType *is, sChar f_unit,
                      char ** cPack, sInt4 *c_len, uChar f_stdout);

#endif
