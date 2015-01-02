Installation:

./configure [CC=compiler name] [--enable-aix-64] [--prefix=DIR]
make
make install

where the optional configure parameters will select the C compiler, enable
64-bit arithmetic on AIX systems (if available) and change the installation
directory for the library to DIR/lib (default is DIR=/usr/local).

Options:  ./configure --enable-aix-64 will configure the Makefile to
create 64-bit code on Aix machines.



Usage of the Emapf Routines
----- -- --- ----- --------

The Emapf routines constitute aan upgrade of the Cmapf routines to accomodate
an Earth whose geoid is in the more accurate shape of an ellipsoid of
revolution.
Like the Cmapf routines, the Emapf routines form a library of C-language
functions which process geographic and dynamic data between grids and
geographical coordinates on a family of conformal map projections.  Data
characterizing the map projections and the grids on them is embedded in maparam
structures. Note: in all cases, North latitudes will be given as positive
degrees, South latitudes negative degrees, East longitudes positive degrees and
West longitudes negative degrees.


Initializing (configuring) the maparam structure:
-------------------------------------------------

Stage I - Selecting a Geoid:

NOTE:  It is VITAL to FIRST select a geoid.  If other routines are given
maparam structures with inconsistent geoids, they will return error
codes and invalid results.

The figure of the Earth (its "geoid") is generally considered to be an
ellipsoid, formed by rotating an ellipse, whose minor axis lies along the
Earth's axis, around that same axis. The equator is then a circle, whose radius
ARAD is half the major axis of the ellipse, and the half-length of the minor
axis, BRAD, may be thought of as the polar radius.

We can specify the geoid in one of two ways:

#include cmapf.h

maparam stcprm;
char fname[]="WGS84";
int rc;

rc = useGeoid(&stcprm,fname) {

will place in stcprm the dimensions and eccentricity of the WGS84 geiod,
i.e. the standard geoid established as the World Geodetic System in 1984,
currently the international standard geoid.  Other standard geoids are the
International, or HAYFORD, adopted internationally from 1924 to 1967, and
the IUGG of 1967, adopted from 1967 to 1984.  The name given must match
the name of one of the geoids known to useGeoid.  Case is ignored.  If the
specified geoid is not found, useGeoid returns 1, otherwise it returns 0.

lsGeoid();
will list on stdout the names of all geoids known to useGeoid.

Alternatively,

#include cmapf.h

maparam stcprm;

double arad = 6367.47,eccen=.08;
GspecType mode = AE;

rc = mkGeoid(&stcprm, mode, arad, eccen);

will place the dimensions and eccentricity for a geoid of the indicated size
in stcprm, where arad and eccen represent the equatorial radius in kilometers
and eccen represents the eccentricity.  Again, rc is zero if successful, one
if data are invalid.  A geoid is determined by the following two data:
Equatorial radius in km. (arad) and polar radius in km. (brad), or by either of
the radii and one of the following: flattening (The difference between arad and
brad, divided by arad) or the eccentricity of the ellipse with axes arad and
brad.  In most cases, geoids are defined by arad and the flattening (e.g. the
WGS84 ellipsoid by 6378.1370 and 1. / 298.257223563).

The geoid specification modes governing mkGeoid are:

AB: arguments arad, brad
AE: arguments arad, eccentricity
AF: arguments arad, flattening
BE: arguments brad, eccentricity
TST:  evaluates consistency of geoid data in stcprm, returning 0 if valid
      and 1 if invalid.  The other two arguments are unused.

Note: to specify the sphere using mode AF (flattening), it is useful to write
the flattening in the form 1./###.###, since geoid flattening is generally
published in that form.  Thus:

mkGeoid(&stcprm, AF, 6378.1234, 1. / 298.2);

Note: to obtain a sphere of radius R, use

mkGeoid(&stcprm, AE, R, 0.);

or

mkGeoid(&stcprm, AF, R, 0.);

Stage II - Selecting a Map Projection.

After selecting a geoid, a map projection may be selected.  To initialize a
maparam structure with a Conformal Polar Projection;

#include cmapf.h

maparam stcprm;
double reflon,tnglat;
int rc;
rc = stlmbr(& stcprm, tnglat, reflon);

will fill the maparam structure "stcprm" with default data specifying a Lambert
Conformal Projection on a cone tangent to the globe at latitude tnglat degrees.
If a cut is needed to lay the cone out flat on the plane, it will be made
opposite to (180 degrees away from) longitude reflon.  If tnglat = 90 or -90,
the map becomes a North or South Polar Stereographic Projection (and the value
of reflon is immaterial).  If tnglat=0., the map becomes a Mercator Projection.

For non-zero values of tnglat, between (but not including) -90 and +90, the
structure defines a Lambert Conformal projection tangent to the Earth at
"tnglat".  If the desired tangent latitude is not known, but the map legend
specifies two reference latitudes, the tangent latitude may be found by

maparam stcprm;
double reflat1,reflat2,tnglat;

tnglat = eqvlat(&stcprm, reflat1, reflat2);

Because this may be an elliptic geoid, eqvlat requires the relative sizes of
arad and brad (in stcprm) to obtain eqvlat.

Caution: this value is not the same as the mean latitude 0.5*(reflat1+reflat2).
Attempts to use the mean latitude will yield misleading results.

RETURN VALUES: stlmbr will return 0 if the geoid terms in stcprm are
consistent and 1 if not (perhaps because useGeoid or mkGeoid was not invoked).

Stage III - Laying out a grid on a selected map projection.

This stage configures the maparam structure with the information to locate the
points of the grid on the selected map projection.  Two alternative means are
provided: Either One-Point specification or Two-Point specification.

a) Two-Point Specification:

double x1,y1,lat1,lon1, x2,y2,lat2,lon2;
stcm2p(&stcprm, x1,y1,lat1,lon1, x2,y2,lat2,lon2);

causes stcprm to be configured so that the grid point with grid coordinates
x1,y1 will be located on the map at latitude lat1, longitude lon1, while
x2,y2 is located on the map at lat2, lon2.  The two points must be different.
This specification locates all other grid points, assumed arranged on the
projection with equal spacing in the x- and y- directions.

The two points will often be opposite corners of the intended grid, but many
times a different selection will be preferred.  For example, in a Mercator
projection, it will often be better to select two points of the same latitude
or the same longitude, to ensure exact N-S orientation of the grid.
Alternatively, several subgrids of an overall grid may be guaranteed
compatibility by using the same two points of the overall grid, even if neither
of the points lie in the subgrid at all.

b) One-Point Specification:

double x1,y1,lat1,lon1, scalat,scalon, gsize,orient;
stcm1p(&stcprm, x1,y1,lat1,lon1, scalat,scalon, gsize,orient);

causes stcprm to be configured so that the grid point with grid coordinates
x1,y1 will be located on the map at latitude lat1, longitude lon1.  The
remaining configuration information is provided at the scaling point whose
latitude and longitude are scalat, scalon.  At that point, and incidentally
every on the latitude circle scalat, the size of a grid cell is gsize in both
the x- and y- directions.  Finally, at the scaling point, the y-lines of the
grid make an angle of "orient" degrees clockwise (East) of the scalon meridian.

The x1,y1 coordinates can be thought of as "pinning" that grid point to the map
at lat1,lon1, the gsize value can be thought of as "zooming" the grid in and
out until the desired size is attained, and the "orient" specification can be
thought of as rotating the grid about the pinned point until the grid has the
wanted orientation.

Frequently, a grid will be published as having an "lov" value of a longitude
intended to be vertical; for such grids, scalon=lov and orient=0.

Using the configured maparams:
-----------------------------

Coordinate transforms:
----------------------

maparam stcprm;
double x,y,lat,lon;

cll2xy(&stcprm, lat,lon, &x,&y);

will return grid coordinates x,y according to the grid configured as stcprm,
for the point whose latitude and longitude are lat,lon.  Longitude values are
adjusted to fit between reflon-180. and reflon+180., so that nearby points on
the globe remain nearby points on the map, unless they straddle the "cut"  at
longitude reflon+180., and find themselves on opposite edges of an unrolled
cylinder.

cxy2ll(&stcprm, x,y, &lat, &lon);

will return latitude and longitude parameters according to the grid configured
as stcprm, for the point whose grid coordinates are x,y.  Longitude values will
be returned in the range from -180. to +180.
If the point x,y corresponds to the North or South Pole, where longitude is
undefined, the returned longitude willl be 180. for the North Pole, 0. for the
South Pole, for compatibility with the WMO standard for reporting winds there.
(In an earlier version of CMAPF, the value of reflon was returned here.)

Wind Vector transforms:
-----------------------

maparam stcprm;
double x,y,lat,lon;
double ue,vn,ug,vg;

cc2gll(&stcprm, lat,lon, ue,vn, &ug,&vg);
and
cg2cll(&stcprm, lat,lon, ug,vg, &ue,&vn);

will convert the East- and North- components ue,vn (Cartographic components) of
the wind to and from components ug,vg of the wind in the x- and y- directions
of the grid (Grid components).  The first transfers from cartographic
components to grid components, the second does the reverse. In both cases, the
wind is that observed at the specified cartographic coordinates lat,lon.
At the poles (lat=90. or lat= -90.), the direction of "North", which determines
the meaning of ue and vn, is based on the nominal value of longitude. The
direction of North will agree with that perceived by an observer stationed a
short distance away on the lon meridian.  Thus, if lat,lon = 90.,-70., "North"
will be asssigned to the direction of the 110E meridian.

cc2gxy(&stcprm, x,y, ue,vn, &ug,&vg);
and
cg2cxy(&stcprm, x,y, ug,vg, &ue,&vn);

will convert the East- and North- components ue,vn (Cartographic components) of
the wind to and from components ug,vg of the wind in the x- and y- directions
of the grid (Grid components).  The first transfers from cartographic
components to grid components, the second does the reverse. In both cases, the
wind is that observed at the specified grid coordinates x,y.
If x,y refers to the North or to the South pole, the orientation of "North" is
defined according to World Meteorological Organization (WMO) code 878.  The
direction of "North" is that of the Greenwich meridian, which is the equivalent
of using lat=90.,lon=180. for the North Pole, and lat=-90.,lon=0. for the South
Pole.


cw2gll(&stcprm, lat,lon, ue,vn, &ug,&vg);
cg2wll(&stcprm, lat,lon, ug,vg, &ue,&vn);
cw2gxy(&stcprm, x,y, ue,vn, &ug,&vg);
and
cg2wxy(&stcprm, x,y, ug,vg, &ue,&vn);

perform the same services as the above transforms, except that within 1 degree
(111km or 60nm) or either pole, ue and vn are interpreted according to WMO code
878 above.  In an earlier version of CMAPF, this was the definition of the
c2g and g2c routines, above.

Gridsize Evaluation:
--------------------

maparam stcprm;
double x,y,lat,lon,gsize;

gsize = cgszll(&stcprm, lat,lon);
and
gsize = cgszxy(&stcprm, x,y);

return the size, in kilometers, of a grid cell at the indicated point.

Curvature Vector:
-----------------
maparam stcprm;
double x,y,lat,lon,gx,gy;

ccrvxy (&stcprm, x,y, &gx,&gy);

ccrvll (&stcprm, lat,lon, &gx,&gy);

return, in &gx and &gy, the logarithmic gradient of the scale.  This vector
represents the curvature on the Earth of a straight line segment on the map.
Units are in radians per kilometer on the Earth's surface.  See the paper for
details on usage in equations.

North Polar Vector evaluations:
-------------------------------
maparam stcprm;
double x,y,lat,lon,enx,eny,enz;

cpolxy (&stcprm, x,y, &enx,&eny,&enz);

cpolll (&stcprm, lat,lon, &enx,&eny,&enz);


return, in enx, eny, and enz the components of the direction of the Earth's
axis.  components enx and eny provide the local direction of North in grid
coordinates, while enz is proportional to the local Coriolis parameter.
