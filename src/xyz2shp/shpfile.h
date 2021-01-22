#ifndef SHPFILE_H
#define SHPFILE_H

#ifndef LATLON_TYPE
#define LATLON_TYPE
typedef struct {
   double lat, lon;
} LatLon;
#endif

int shpCreatePnt(const char *Filename, const LatLon *dp, size_t numDP);
int shpCreatePrj(const char *Filename, const char *gcs, const char *datum,
                 const char *spheroid, double A, double B);

#endif
