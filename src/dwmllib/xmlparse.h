#ifndef XMLPARSE_H
#define XMLPARSE_H
#include "genprobe.h"
#include "type.h"
#include "sector.h"




int XMLParse (uChar f_XML, size_t numPnts, Point * pnts, PntSectInfo *pntInfo,
              sChar f_pntType, char **labels, size_t numInFiles,
              char **inFiles, uChar f_fileType, sChar f_interp, sChar f_unit,
              double majEarth, double minEarth, sChar f_icon,
              sChar f_SimpleVer, sChar f_valTime, double startTime,
              double endTime, size_t numNdfdVars, uChar *ndfdVars,
              char *f_inTypes, char *gribFilter, size_t numSector,
              char ** sector, sChar f_ndfdConven);
#endif
