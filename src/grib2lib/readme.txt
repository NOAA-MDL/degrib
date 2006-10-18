Modifications:
1) ./g2libc-1.0/gridtemplates.h :: Handle a negative longitude:
             /* 3.30: Lambert Conformal */
//       {30, 22, 0, {1,1,4,1,4,1,4,4,4,-4,4,1,-4,4,4,4,1,1,-4,-4,-4,4} },
         {30, 22, 0, {1,1,4,1,4,1,4,4,4,-4,-4,1,-4,4,4,4,1,1,-4,-4,-4,4} },

2) ./grib2unpacker/rdieee.f -> ./grib2unpacker/rdi3e.f
   There were two procedure named "rdieee" (one in the C version).  To merge
   the libraries, I needed 1, so I relabeled the FORTRAN one to rdi3e

3) ./grib2unpacker/unpk_grib2.f -> ./grib2unpacker/unpk_g2mdl.f
   I relabeled unpk_grib2() to unpk_g2mdl() so that the API I created
   wouldn't be confused with the original one.

4) ./g2libc-1.0/grib2.h
   Use of //.  A minor issue but if someone includes it, they have to have
   their compiler set to not squak at the //.  Since this is the main header
   file and is intended for outsiders to include, I'd recommend changing all
   the // in this one file to /*, and have done so.

5) ./grib2packer/pack_gp.f -> ./grib2packer/pack_gp2.f 
   I relabeled pack_gp() to pack_gp2() to avoid conflict with steve's library.

6) ./grib2packer/reduce.f -> ./grib2packer/reduce2.f
   I relabeled reduce() to reduce2() to avoid conflict with steve's library.

------------------------------
Notes:
Problems with "seekgb"
   1) Assumes the user knows, or can make a reasonable guess as to the total
      size of the compacted GRIB2 message without ever having looked at the
      message.
   2) Allocates and frees memory which contains the whole message.
   3) Reads the whole message in.
   2 & 3) are inefficient since the driver most likely has to do the same
      thing shortly to uncompress the data, so we end up with 2 file I/Os and
      2 memory allocations.
   Answer: I would recommend something closer to "ReadSECT0()" in degrib2.c

Problems with "grib_mod"
   1) Forces one to actually use fortran 90.

NCEP treats all data internally as floats.  Technically this could lose some
   precision when dealing with very large integers.

New section 5 templates don't state whether the original data is an integer or
   a float data (I assume float)

------------------------------
API Issues:
1) MDL Returns IS5(12), IS5(24) and IS5(28) as a truncated integer of the
   actual data.  NCEP Returned the IEEE version.  I prefer the IEEE version,
   but maintained the MDL version of the API

2) The API uses Big Endian 4 byte ints to pass the data in and out.  NCEP uses
   unsigned 1 byte ints so they don't have to worry about "Endian" issues.
   The result is on a Little Endian machine, the driver flips the bytes, then
   to call NCEP routines the bytes are flipped a second time.

   Recommended Solution: Have an API which accepts 1 byte ints, and have it
   create 4 byte ints to call the MDL FORTRAN routines with.  This would
   result in the fewest number of byte flippings.

3) The API assumed that Local use data would be MDL packed.  If the local use
   data is not MDL packed, it is uncertain how to return it.
   Possible Answer 1: Return it in the IS2 array.
   Possible Answer 2: Provide a different API to unpack MDL section 2 data,
      and don't bother uncompressing it in the Main API.  Return the data as
      1 byte integers.

4) The API returns either Integer arrays or Float Arrays.  NCEP is treating
   everything internally as floats.  Should the API be simplified to just
   return floats?

5) API does not request which subgrid someone is interested in.  NCEP's code
   would work faster for subgrids if this was passsed to it.
