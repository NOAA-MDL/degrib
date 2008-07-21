#ifndef HAZARD_H
#define HAZARD_H

#include "meta.h"

void FreeHazardString (HazardStringType * haz);

void ParseHazardString (HazardStringType * haz, char *data, int simpleVer);

void PrintHazardString (HazardStringType * haz);

#endif
