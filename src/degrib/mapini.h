#ifndef MAPINI_H
#define MAPINI_H

#include "type.h"

typedef struct {            
   short int r, g, b, alpha;
   float value;
   char f_null;
   int gdIndex;
} colorType;

typedef struct {
   char *name;
   colorType fg;
   colorType bg;
   colorType textC;
   int x, y;
   int f_font; /* 1=tiny, 2=small, 3=medium, 4=large, 5=giant. */
} textType;

typedef struct {
   unsigned char f_valid;
   colorType fg;
   colorType bg;
   int x, y;
   char decimal; /* how many decimals to print with. */
   char style;  /* 0 Range... 1 GE this value... 2 LE this value. */
   colorType textC;
   char pixelSize;
   int f_large;
   sChar f_vert;  /* Legend is vertical or horizontal. */
   int wid, hei;
} legendType;

typedef struct {
   double lat1, lon1, lat2, lon2;
   char *file;
   unsigned char f_flag; /* Have lat1,lon1,lat2,lon2 been set? */
} zoomType;

typedef struct {
   int type;
   double majEarth, minEarth;
   double lat1, lon1;
   unsigned char f_flag1; /* Have type,majEarth,minEarth,lat1,lon1 been set?*/
   double mesh;
   double orientLon;
   double meshLat;
   double scaleLat1;
   double scaleLat2;
   unsigned char f_flag2; /* Have mesh,orientLon,meshLat,scaleLat1,scaleLat2 been set?*/
} projectType;

typedef struct {
   char *filename;
   int *active;
   size_t numActive;
   char f_valid;
} outputType;

typedef struct {
   int X0, Y0;
   int X1, Y1;
   int X_Size;
   int Y_Size;
   colorType bg;
   int f_zoom; /* 0 no zoom, 1 lat/lon,lat/lon */
   zoomType zoom;
} frameType;

typedef struct {
   int X_Size;
   int Y_Size;
   int *active;
   size_t numActive;
   frameType *frame;
   int numFrame;
   unsigned char f_flag; /* Have X_Size,Y_Size been set? */
   int numOutputs;
   outputType *outputs;
} allOutputType;

/* plotting methods. */
enum {INVALID, SINGLE_SYMBOL, GRADUATED, TEXT, DB2, PNG, GRID, INFO, LATTICE};
/* supported shape file types. */
enum {INVALID_SHP, POLYGON, POINT, MEMORY, VOID};

typedef struct {
   colorType fg;
   colorType out;
   char *mark;
   char f_mark; /* 0 use mark, 1 use dot, 2 use value, 3 pixel, 4 dot3, 5 dot2 */
                /* dot is 5 x 5, dot3 is 3 x 3, dot2 is 2 x 2, pixel is 1x1 */
   double Max, Min;
   char f_maxInc, f_minInc; /* Whether to include or not the min/max value */
   char decimal; /* how many decimals to print with. */
   char thick;   /* how thick to make the outline. */
} SymbolType;

typedef struct {
   size_t numSymbol;
   SymbolType *symbol;
   char *field;
} graduatedType;

typedef struct {
/* colorIndex should be (value - min) / (max - min) * numColors. */
   colorType *colors;
   size_t numColors;
   double min, max;
   char f_missing;   /* Has a missing value. */
   colorType missColor;
   double missValue;
   char decimal;
   char thick;   /* how thick to make the outline. 0 means none */
   colorType outline;
   float labInterval;
   float labMax, labMin;
   int numLab;
   float *labRay;
   int *labJust;
   char **label;
} rampType;

typedef struct {
   char *field;
} dbf2Type;

typedef struct {
   char f_valid;
   colorType fg;
   double spacing; /* in degrees. */
   int style; /* 1,2,3 */
   char labelSite[6];
} latticeType;

typedef struct {
   char *filename;
   unsigned char type;
   unsigned char shpType;
   char *matchField;
   size_t numMatchVal;
   char **matchVal;
   size_t numPnt;
   LatLon *pnt;
   SymbolType single;
   graduatedType grad;
   rampType ramp;
   textType title;
   latticeType lattice;
   legendType legend;
   dbf2Type db2;
   int frameNum;
   double * gridData;
   sInt4 gridNx, gridNy;
   projectType gridProj;
   int legendFrameNum;
} layerType;

typedef struct {
   colorType bg;
   int numLayer;
   layerType *layers;
} allLayerType;

typedef struct {
   zoomType zoom;
   projectType proj;
   allOutputType out;
   allLayerType all;
} mapIniType;

void InitMapIni (mapIniType *mapIni);

void FreeMapIni (mapIniType *mapIni);

void InitLayer (layerType * layer);

int ParseMapIniFile (mapIniType * usr, const char * options, FILE *fp);

int ValidateMapIni (mapIniType *mapIni);

int SaveMapIniFile (mapIniType *mapIni, char *filename);

int ReadShpBounds (char *filename, double *minX, double *minY, double *maxX,
                   double *maxY);

int freeIniSymbol (SymbolType *symbol);

int ParseSymbol (SymbolType * symbol, char *value);

#endif
