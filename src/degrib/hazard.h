#ifndef HAZARD_H
#define HAZARD_H

#include "meta.h"

void FreeHazardString (HazardStringType * haz);

int ParseHazardString (HazardStringType * haz, char *data, int simpleVer);

void PrintHazardString (HazardStringType * haz);

#endif
