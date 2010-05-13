/*****************************************************************************
 * split.h
 *
 * DESCRIPTION
 *    This file contains the code needed to split the GRIB file into its
 * component messages.
 *
 * HISTORY
 *   5/2010 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef SPLIT_H
#define SPLIT_H

int GRIB2Split (char *filename, int msgNum, int curMsg);

#endif

