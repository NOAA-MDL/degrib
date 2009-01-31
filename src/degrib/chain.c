/*****************************************************************************
 * chain.c
 *
 * DESCRIPTION
 *    This file contains some primitives for chaining together a set of points
 * so that we can contour, and create "big polygons" from a grid.
 *
 * HISTORY
 *    6/2002 Arthur Taylor (MDL / RSIS): Created.
 *    9/2003 AAT: Introduced into "degrib" source.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "chain.h"
#include "myassert.h"
#include "myerror.h"
#include "myutil.h"
#include "tendian.h"

/*****************************************************************************
 * AddPoint() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Add a point to a given contour chain.  If f_head is true, it adds the
 * point at the head of the chain, otherwise it adds it to the tail.
 *
 * ARGUMENTS
 *     chain = The chain to add to. (Output)
 *      x, y = The point to add. (Input)
 *    f_head = True add to the head, false add to the tail. (Input)
 * f_compact = True if we should try to compact the chain. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   6/2002 Arthur Taylor (MDL/RSIS): Created.
 *   9/2003 AAT: Modified to reduce the number of co-linear points in chains.
 *
 * NOTES
 *****************************************************************************
 */
static void AddPoint (chainType *chain, float x, float y, char f_head,
                      char f_compact)
{
   chainNode *node;     /* The new point before it is put on the chain. */

   myAssert (chain != NULL);

   /* Check if there are any points on the chain. */
   if (chain->head == NULL) {
      node = (chainNode *) malloc (sizeof (chainNode));
      node->x = x;
      node->y = y;
      node->next = NULL;
      chain->head = node;
      chain->tail = node;
      chain->preTail = node;

   } else if (f_head) {
      /* compact the head part of the chain due to the new point. */
      if (f_compact && (chain->head != NULL)) {
         if (chain->head->next != NULL) {
            if ((chain->head->x == x) && (chain->head->next->x == x)) {
               chain->head->y = y;
               return;
            }
            if ((chain->head->y == y) && (chain->head->next->y == y)) {
               chain->head->x = x;
               return;
            }
         }
      }
      /* We now add at the head. */
      node = (chainNode *) malloc (sizeof (chainNode));
      node->x = x;
      node->y = y;
      node->next = chain->head;
      chain->head = node;
      if (chain->preTail == chain->tail) {
         chain->preTail = chain->head;
      }
   } else {
      if (f_compact && (chain->tail != NULL)) {
         if (chain->preTail != NULL) {
            if ((chain->tail->x == x) && (chain->preTail->x == x)) {
               chain->tail->y = y;
               return;
            }
            if ((chain->tail->y == y) && (chain->preTail->y == y)) {
               chain->tail->x = x;
               return;
            }
         }
      }
      /* We now add at the tail. */
      node = (chainNode *) malloc (sizeof (chainNode));
      node->x = x;
      node->y = y;
      node->next = NULL;
      chain->tail->next = node;
      chain->preTail = chain->tail;
      chain->tail = node;
   }
   return;
}

/*****************************************************************************
 * AddSeg() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Add a line segment to the contour chains.  Segments go from x1,y1 to
 * x2,y2, so we look for chains with tails that match the given "head", or
 * chains with heads that match the given tail.
 *
 * ARGUMENTS
 *    chList = The list of active chains we have created. (Output)
 *  numChain = The number of chains in chList. (Input/Output)
 *   finList = The list of closed completed chains. (Output)
 * numFinish = The number of chains in finList. (Input/output)
 *     value = The contour value of this segment. (Input)
 *    x1, y1 = The start (head) of this line segment. (Input)
 *    x2, y2 = The end (tail) of this line segment. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  6/2002 Arthur Taylor (MDL/RSIS): Created.
 * 10/2003 Arthur Taylor (MDL/RSIS): Modified for speed.
 *
 * NOTES
 *   Due to round-off error, we resort to epsilon checks to see if the points
 * are the same.  If we used integers instead of doubles this wouldn't be a
 * problem, but we would have to scale first, which could be inconvenient.
 *****************************************************************************
 */
static void AddSeg (chainType **chList, int *numChain, chainType **finList,
                    int *numFinish, double value, float x1, float y1,
                    float x2, float y2)
{
   int chTail = -1;     /* Add to this chain's tail */
   int chHead = -1;     /* Add to this chain's head */
   int cur;             /* Loop counter over number of chains. */

   myAssert (!((x1 == x2) && (y1 == y2)));

   /* Look for other chains starting or stopping at this segment. */
   for (cur = 0; cur < *numChain; cur++) {
      if (((*chList)[cur].tail->x == x1) && ((*chList)[cur].tail->y == y1)) {
         chTail = cur;
         if (chHead != -1)
            break;
      }
      if (((*chList)[cur].head->x == x2) && ((*chList)[cur].head->y == y2)) {
         chHead = cur;
         if (chTail != -1)
            break;
      }
   }

   if (chHead == -1) {
      if (chTail == -1) {
         /* Completely new segment. */
         cur = *numChain;
         *numChain += 1;
         *chList = (chainType *) realloc ((void *) *chList,
                                          *numChain * sizeof (chainType));
         (*chList)[cur].head = NULL;
         (*chList)[cur].tail = NULL;
         AddPoint ((*chList + cur), x1, y1, 1, 0);
         AddPoint ((*chList + cur), x2, y2, 0, 0);
      } else {
         /* Add x2, y2 to the tail of chTail. */
         AddPoint ((*chList + chTail), x2, y2, 0, 1);
      }
   } else if (chTail == -1) {
      /* Add p1 to the head of chHead. */
      AddPoint ((*chList + chHead), x1, y1, 1, 1);

   } else if (chHead != chTail) {
      /* We do not have a loop chain so join the two chains, with chTail
       * tail->next => chHead head. */
      (*chList)[chTail].tail->next = (*chList)[chHead].head;
      if ((*chList)[chHead].preTail == (*chList)[chHead].tail) {
         (*chList)[chTail].preTail = (*chList)[chTail].tail;
      } else {
         (*chList)[chTail].preTail = (*chList)[chHead].preTail;
      }
      (*chList)[chTail].tail = (*chList)[chHead].tail;
      /* Remove chHead. */
      for (cur = chHead + 1; cur < *numChain; cur++) {
         (*chList)[cur - 1] = (*chList)[cur];
      }
      *numChain = *numChain - 1;
   } else {
      /* Close the "loop" chain.  To do so, add x2,y2 to the tail. */
      AddPoint ((*chList + chTail), x2, y2, 0, 1);
      /* Move closed loop to "finList". */
      *numFinish += 1;
      *finList = (chainType *) realloc ((void *) *finList,
                                        *numFinish * sizeof (chainType));
      (*finList)[*numFinish - 1] = (*chList)[chTail];
      /* Remove chTail. */
      for (cur = chTail + 1; cur < *numChain; cur++) {
         (*chList)[cur - 1] = (*chList)[cur];
      }
      *numChain -= 1;
   }
}

/*****************************************************************************
 * AddTrip() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Add a line segment made of 3 points to the contour chains.  Segments go
 * from x1,y1 to x0,y0, to x2,y2, so we look for chains with tails that match
 * the given "head" (x1,y1), or chains with heads that match the given "tail"
 * (x2,y2).
 *   This is based on AddSeg().
 *
 * ARGUMENTS
 *    chList = The list of chains we have created. (Output)
 *  numChain = The number of chains in chList. (Input/Output)
 *   finList = The list of closed completed chains. (Output)
 * numFinish = The number of chains in finList. (Input/output)
 *     value = The contour value of this segment. (Input)
 *    x1, y1 = The start (head) of this line segment. (Input)
 *    x0, y0 = The middle point of the segment. (Input)
 *    x2, y2 = The end (tail) of this line segment. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  9/2003 Arthur Taylor (MDL/RSIS): Created.
 * 10/2003 Arthur Taylor (MDL/RSIS): Modified for speed.
 *
 * NOTES
 *   Due to round-off error, we resort to epsilon checks to see if the points
 * are the same.  If we used integers instead of doubles this wouldn't be a
 * problem, but we would have to scale first, which could be inconvenient.
 *****************************************************************************
 */
static void AddTrip (chainType **chList, int *numChain,
                     chainType **finList, int *numFinish, double value,
                     float x1, float y1, float x0, float y0, float x2,
                     float y2)
{
   int chTail = -1;     /* Add to this chain's tail */
   int chHead = -1;     /* Add to this chain's head */
   int cur;             /* Loop counter over number of chains. */

   myAssert (!((x1 == x2) && (y1 == y2)));

   /* Check if middle point equals an end point. */
   myAssert (!(((x1 == x0) && (y1 == y0)) || ((x2 == x0) && (y2 == y0))));
   /* Check if points are co-linear. */
   myAssert (!(((x1 == x0) && (x2 == x0)) || ((y1 == y0) && (y2 == y0))));
   /* If either of the above assertions fail we should call AddSeg(). */

   /* Look for other chains starting or stopping at this segment. */
   for (cur = 0; cur < *numChain; cur++) {
      if (((*chList)[cur].tail->x == x1) && ((*chList)[cur].tail->y == y1)) {
         chTail = cur;
         if (chHead != -1)
            break;
      }
      if (((*chList)[cur].head->x == x2) && ((*chList)[cur].head->y == y2)) {
         chHead = cur;
         if (chTail != -1)
            break;
      }
   }

   if (chHead == -1) {
      if (chTail == -1) {
         /* Completely new segment. */
         cur = *numChain;
         *numChain += 1;
         *chList = (chainType *) realloc ((void *) *chList,
                                          *numChain * sizeof (chainType));
         (*chList)[cur].head = NULL;
         (*chList)[cur].tail = NULL;
         AddPoint ((*chList + cur), x1, y1, 1, 0);
         AddPoint ((*chList + cur), x0, y0, 0, 0);
         AddPoint ((*chList + cur), x2, y2, 0, 0);
      } else {
         /* Add x2, y2 to the tail of chTail. */
         AddPoint ((*chList + chTail), x0, y0, 0, 1);
         AddPoint ((*chList + chTail), x2, y2, 0, 0);
      }
   } else if (chTail == -1) {
      /* Add p1 to the head of chHead. */
      AddPoint ((*chList + chHead), x0, y0, 1, 1);
      AddPoint ((*chList + chHead), x1, y1, 1, 0);

   } else if (chHead != chTail) {
      /* We do not have a loop chain so join the two chains, with chTail
       * tail->next => chHead head. */
      AddPoint ((*chList + chTail), x0, y0, 0, 1);
      (*chList)[chTail].tail->next = (*chList)[chHead].head;
      if ((*chList)[chHead].preTail == (*chList)[chHead].tail) {
         (*chList)[chTail].preTail = (*chList)[chTail].tail;
      } else {
         (*chList)[chTail].preTail = (*chList)[chHead].preTail;
      }
      (*chList)[chTail].tail = (*chList)[chHead].tail;
      /* Remove chHead. */
      for (cur = chHead + 1; cur < *numChain; cur++) {
         (*chList)[cur - 1] = (*chList)[cur];
      }
      *numChain = *numChain - 1;
   } else {
      /* Close the "loop" chain.  To do so, add x2,y2 to the tail. */
      AddPoint ((*chList + chTail), x0, y0, 0, 1);
      AddPoint ((*chList + chTail), x2, y2, 0, 0);
      /* Move closed loop to "finList". */
      *numFinish += 1;
      *finList = (chainType *) realloc ((void *) *finList,
                                        *numFinish * sizeof (chainType));
      (*finList)[*numFinish - 1] = (*chList)[chTail];
      /* Remove chTail. */
      for (cur = chTail + 1; cur < *numChain; cur++) {
         (*chList)[cur - 1] = (*chList)[cur];
      }
      *numChain -= 1;
   }
}

/*****************************************************************************
 * AddPoly() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Add a line segment to the polygon type which is a combination of contour
 * chains.  We find the chains that have this value, and then call AddSeg to
 * add the segment to it.
 *
 * ARGUMENTS
 * numPoly = The number of polygons created so far. (Input/Output)
 *    poly = The collection of contour chains. (Input/Output)
 *  f_3pnt = True if x0,y0 is valid. (Input)
 *   value = The contour value of this segment. (Input)
 *  x1, y1 = The start (head) of this line segment. (Input)
 *  x0, y0 = The middle point of the segment. (Input)
 *  x2, y2 = The end (tail) of this line segment. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   -1 if called with invalid arguments.
 *    0 otherwise.
 *
 * HISTORY
 * 10/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *   Due to round-off error, we resort to epsilon checks to see if the points
 * are the same.
 *****************************************************************************
 */
static void AddPoly (int *numPoly, polyType **poly, sChar f_3pnt,
                     double value, float x1, float y1, float x0, float y0,
                     float x2, float y2)
{
   static double epsilon = .0000000001; /* Used in comparisons to avoid round 
                                         * off errors. */
   static double NegEps = -.0000000001; /* Used in comparisons to avoid round 
                                         * off errors. */
   double delt;         /* Difference between value and current chain. */
   int i;               /* Used to do an insertion sort. */
   int low, high, mid;  /* Indicies to assist in binary search. */
   sChar f_found;       /* Flag indicating if we found it. */

   myAssert (!((x1 == x2) && (y1 == y2)));

   /* Look for value in poly. */
   myAssert (*numPoly >= 0);
   low = 0;
   high = *numPoly - 1;
   mid = 0;
   f_found = 0;
   while (low <= high) {
      mid = (low + high) / 2;
      delt = (*poly)[mid].value - value;
      if (delt > epsilon) {
         /* value < poly[mid].value */
         high = mid - 1;
      } else if (delt < NegEps) {
         /* value > poly[mid].value */
         low = mid + 1;
      } else {
         /* equal. */
         f_found = 1;
         break;
      }
   }
   if (!f_found) {
      /* Figure out where to insert.  mid is either just too big or just too
       * small.  We want it just too big. */
      if (mid < *numPoly) {
         if (((*poly)[mid].value - value) < epsilon) {
            mid++;
            /* Don't have to worry about mid == *numPoly since we do
             * numPoly++ next. */
         }
      }
      /* Insert new segment at mid. */
      (*numPoly)++;
      *poly = (polyType *) realloc ((void *) *poly,
                                    *numPoly * sizeof (polyType));
      /* Make room. */
      for (i = *numPoly - 1; i > mid; i--) {
         (*poly)[i] = (*poly)[i - 1];
      }
      (*poly)[mid].value = value;
      (*poly)[mid].numAct = 0;
      (*poly)[mid].numFin = 0;
      (*poly)[mid].actList = NULL;
      (*poly)[mid].finList = NULL;
      (*poly)[mid].crossLink = NULL;
   }
   if (f_3pnt) {
      /* Check if middle point equals an end point. */
      myAssert (!(((x1 == x0) && (y1 == y0)) || ((x2 == x0) && (y2 == y0))));
      /* Check if points are co-linear. */
      myAssert (!(((x1 == x0) && (x2 == x0)) || ((y1 == y0) && (y2 == y0))));
      /* If either of the above assertions fail we should call AddSeg(). */

      AddTrip (&((*poly)[mid].actList), &((*poly)[mid].numAct),
               &((*poly)[mid].finList), &((*poly)[mid].numFin),
               value, x1, y1, x0, y0, x2, y2);
      return;
   } else {
      AddSeg (&((*poly)[mid].actList), &((*poly)[mid].numAct),
              &((*poly)[mid].finList), &((*poly)[mid].numFin),
              value, x1, y1, x2, y2);
      return;
   }
}

/*****************************************************************************
 * NewPolys() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Initialize the poly and numPoly data.
 *
 * ARGUMENTS
 *    poly = The collection of contour chains. (Output)
 * numPoly = The number of polygons created so far. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 * 10/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void NewPolys (polyType **poly, int *numPoly)
{
   *poly = NULL;
   *numPoly = 0;
}

/*****************************************************************************
 * CompactPolys() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Reduce redundant points in the chains.  The process of creating "big
 * polygons" from a grid ended up with lines like: (0,0) (0,0.5) (0,1.5) (0,2)
 * instead of (0,0) (0,2).
 *
 * ARGUMENTS
 *    poly = The collection of contour chains. (Output)
 * numPoly = The number of polygons created so far. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  9/2003 Arthur Taylor (MDL/RSIS): Created.
 * 10/2003 AAT: Modified to use PolyType.
 *
 * NOTES
 * 1) Even though AddPoint is now reducing the number of co-linear points in
 *    chains, when we join 2 chains together, there could be co-linear points
 *    created.  This routine will reduce those.
 *****************************************************************************
 */
#ifdef CHAIN_DEBUG
void CompactPolys (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   chainNode *last;     /* The last node. */
   chainNode *cur;      /* The current node. */

   for (i = 0; i < numPoly; i++) {
      myAssert (poly[i].numAct == 0);
      for (j = 0; j < poly[i].numFin; j++) {
         /* Check colinear preTail, (tail == head), head->next */
         last = poly[i].finList[j].preTail;
         cur = poly[i].finList[j].head;
         if (((last->x == cur->x) && (cur->next->x == cur->x)) ||
             ((last->y == cur->y) && (cur->next->y == cur->y))) {
            poly[i].finList[j].head = poly[i].finList[j].head->next;
            free (cur);
            cur = poly[i].finList[j].head;
            poly[i].finList[j].tail->x = poly[i].finList[j].head->x;
            poly[i].finList[j].tail->y = poly[i].finList[j].head->y;
         }
         while (cur != NULL) {
            last = cur;
            cur = cur->next;
            if (cur != NULL) {
               if (cur->next != NULL) {
                  if (((last->x == cur->x) && (cur->next->x == cur->x)) ||
                      ((last->y == cur->y) && (cur->next->y == cur->y))) {
                     last->next = cur->next;
                     free (cur);
                     /* Make cur = last so next time through cur is
                      * cur->next. */
                     cur = last;
                  }
               } else {
                  poly[i].finList[j].tail = cur;
                  poly[i].finList[j].preTail = last;
               }
            }
         }
      }
   }
}
#endif

/*****************************************************************************
 * gribCompactPolys() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Reduce redundant points in the chains.  The process of creating "big
 * polygons" from a grid ended up with lines like: (0,0) (0,0.5) (0,1.5) (0,2)
 * instead of (0,0) (0,2).
 *   In addition, go through the chains, and remove any chains associated
 * with missing data using the GRIB schemes for defining missing data.
 *
 * ARGUMENTS
 *       poly = The collection of contour chains. (Input/Output)
 *    numPoly = The number of polygons created so far. (Input)
 * f_nMissing = True if we do not want missing values in the .shp file. (In)
 *     attrib = Tells what type of missing values we used. (Input)
 *   polyData = The values associated with each polygon. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 * 10/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void gribCompactPolys (polyType *poly, int *numPoly, sChar f_nMissing,
                       gridAttribType *attrib, double **polyData)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains, and used to
                         * shift polygons up when a poly is removed. */
   chainNode *last;     /* The last node. */
   chainNode *cur;      /* The current node. */
   chainNode *cur2;     /* The next node on the chain. */

   for (i = 0; i < *numPoly; i++) {
      if ((!f_nMissing) || (attrib->f_miss == 0) ||
          ((poly[i].value != attrib->missPri) &&
           ((attrib->f_miss != 2) || (poly[i].value != attrib->missSec)))) {
         myAssert (poly[i].numAct == 0);
         for (j = 0; j < poly[i].numFin; j++) {
            /* Check colinear preTail, (tail == head), head->next */
            last = poly[i].finList[j].preTail;
            cur = poly[i].finList[j].head;
            if (((last->x == cur->x) && (cur->next->x == cur->x)) ||
                ((last->y == cur->y) && (cur->next->y == cur->y))) {
               poly[i].finList[j].head = poly[i].finList[j].head->next;
               free (cur);
               cur = poly[i].finList[j].head;
               poly[i].finList[j].tail->x = poly[i].finList[j].head->x;
               poly[i].finList[j].tail->y = poly[i].finList[j].head->y;
            }
            while (cur != NULL) {
               last = cur;
               cur = cur->next;
               if (cur != NULL) {
                  if (cur->next != NULL) {
                     if (((last->x == cur->x) && (cur->next->x == cur->x)) ||
                         ((last->y == cur->y) && (cur->next->y == cur->y))) {
                        last->next = cur->next;
                        free (cur);
                        /* Make cur = last so next time through cur is
                         * cur->next. */
                        cur = last;
                     }
                  } else {
                     poly[i].finList[j].tail = cur;
                     poly[i].finList[j].preTail = last;
                  }
               }
            }
         }
      } else {
         /* Free this poly. */
         myAssert (poly[i].numAct == 0);
         free (poly[i].actList);
         poly[i].actList = NULL;
         for (j = 0; j < poly[i].numFin; j++) {
            cur = poly[i].finList[j].head;
            while (cur != NULL) {
               cur2 = cur->next;
               free (cur);
               cur = cur2;
            }
         }
         free (poly[i].finList);
         poly[i].finList = NULL;
         if (poly[i].crossLink != NULL) {
            free (poly[i].crossLink);
         }
         /* shift remaining polys up. */
         for (j = i + 1; j < *numPoly; j++) {
            poly[j - 1] = poly[j];
         }
         /* Adjust indexes to reflect the shift. */
         i--;
         *numPoly -= 1;
      }
   }
   /* Extract out the poly values and store in polyData. */
   *polyData = (double *) malloc (*numPoly * sizeof (double));
   for (i = 0; i < *numPoly; i++) {
      (*polyData)[i] = poly[i].value;
   }
}

/*****************************************************************************
 * FreePolys() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free up the memory associated with the poly structure.
 *
 * ARGUMENTS
 *    poly = The collection of contour chains. (Output)
 * numPoly = The number of polygons created so far. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 * 10/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void FreePolys (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   chainNode *cur;      /* The current node (to free). */
   chainNode *cur2;     /* The next node on the chain. */

   for (i = 0; i < numPoly; i++) {
      for (j = 0; j < poly[i].numAct; j++) {
         cur = poly[i].actList[j].head;
         while (cur != NULL) {
            cur2 = cur->next;
            free (cur);
            cur = cur2;
         }
      }
      free (poly[i].actList);
      for (j = 0; j < poly[i].numFin; j++) {
         cur = poly[i].finList[j].head;
         while (cur != NULL) {
            cur2 = cur->next;
            free (cur);
            cur = cur2;
         }
      }
      free (poly[i].finList);
      if (poly[i].crossLink != NULL) {
         free (poly[i].crossLink);
      }
   }
   free (poly);
}

/*****************************************************************************
 * DESCRIPTION
 *    This section deals specifically with the creating "big polygons" from a
 * grid.  Algorithm is based on the observation that the four cells can come
 * together in 15 different ways:
 *   AAAA AAAB         AABA AABB AABC
 *   ABAA ABAB ABAC    ABBA ABBB ABBC    ABCA ABCB ABCC ABCD
 *
 *   After observing this, each of the individual 15 cases was handled in
 * create AddCell().  Grid2BigPoly() (the driver for AddCell) is just looping
 * over the grid calling AddCell() repeatedly.  Grid2BigPoly() had to be a
 * little careful when dealing with the edges and sides, but could still call
 * AddCell().
 *
 * HISTORY
 *    9/2003 AAT: Created.
 *
 * NOTES
 *****************************************************************************
 */

/*****************************************************************************
 * CellsEqual() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To check if two cells are equal.
 *
 * ARGUMENTS
 * f_Valid1 = If p1 is valid or not.  False for edges and corners. (Input)
 *       p1 = First value to compare. (Input)
 * f_Valid2 = If p2 is valid or not.  False for edges and corners. (Input)
 *       p2 = Second value to compare. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *    1 if p1 == p2, or both are invalid.
 *    0 otherwise.
 *
 * HISTORY
 *   9/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int CellsEqual (int f_Valid1, double p1, int f_Valid2, double p2)
{
   if (f_Valid1 && f_Valid2) {
      return (p1 == p2);
   } else {
      return (f_Valid1 == f_Valid2);
   }
}

/*****************************************************************************
 * AddCell() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To add an intersection of 4 cells to the chain list.
 *
 * ARGUMENTS
 * numPoly = The number of polygon's in poly. (Output)
 *    poly = The set of polygon chains to add to. (Output)
 *    x, y = Location of the intersection in question. (Input)
 *  flag_A = If A is valid or not.  False for edges and corners. (Input)
 *       A = The upper left corner. (Input)
 *  flag_B = If B is valid or not.  False for edges and corners. (Input)
 *       B = The upper right corner. (Input)
 *  flag_C = If C is valid or not.  False for edges and corners. (Input)
 *       C = The upper left corner. (Input)
 *  flag_D = If D is valid or not.  False for edges and corners. (Input)
 *       D = The upper right corner. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2003 Arthur Taylor (MDL/RSIS): Created.
 *  10/2003 AAT: Modified to use polyType.
 *
 * NOTES
 * 1) Assumes scan mode of 0100.  If that is incorrect, polygons could be
 *    inside out... So to switch from 0100 to 0000, reverse all chains.
 * 2) By observing that certain intersections can only have a corner or
 *    edge on one side or none at all, one could removes some of the
 *    "if (flag_A)" statements.
 *****************************************************************************
 */
static void AddCell (int *numPoly, polyType **poly, float x, float y,
                     int flag_A, double A, int flag_B, double B, int flag_C,
                     double C, int flag_D, double D)
{
   if (CellsEqual (flag_A, A, flag_B, B)) {
      if (CellsEqual (flag_A, A, flag_C, C)) {
         if (CellsEqual (flag_A, A, flag_D, D)) {
            /* AAAA */
         } else {
            /* AAAB */
            if (flag_A) {
               AddPoly (numPoly, poly, 1, A, x + 0.5, y, x, y, x, y - 0.5);
            }
            if (flag_D) {
               AddPoly (numPoly, poly, 1, D, x, y - 0.5, x, y, x + 0.5, y);
            }
         }
      } else if (CellsEqual (flag_A, A, flag_D, D)) {
         /* AABA */
         if (flag_A) {
            AddPoly (numPoly, poly, 1, A, x, y - 0.5, x, y, x - 0.5, y);
         }
         if (flag_C) {
            AddPoly (numPoly, poly, 1, C, x - 0.5, y, x, y, x, y - 0.5);
         }
      } else if (CellsEqual (flag_C, C, flag_D, D)) {
         /* AABB */
         if (flag_A) {
            AddPoly (numPoly, poly, 0, A, x + 0.5, y, -1, -1, x - 0.5, y);
         }
         if (flag_C) {
            AddPoly (numPoly, poly, 0, C, x - 0.5, y, -1, -1, x + 0.5, y);
         }
      } else {
         /* AABC */
         if (flag_A) {
            AddPoly (numPoly, poly, 0, A, x + 0.5, y, -1, -1, x - 0.5, y);
         }
         if (flag_C) {
            AddPoly (numPoly, poly, 1, C, x - 0.5, y, x, y, x, y - 0.5);
         }
         if (flag_D) {
            AddPoly (numPoly, poly, 1, D, x, y - 0.5, x, y, x + 0.5, y);
         }
      }
   } else if (CellsEqual (flag_A, A, flag_C, C)) {
      if (CellsEqual (flag_A, A, flag_D, D)) {
         /* ABAA */
         if (flag_A) {
            AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x + 0.5, y);
         }
         if (flag_B) {
            AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
         }
      } else if (CellsEqual (flag_B, B, flag_D, D)) {
         /* ABAB */
         if (flag_A) {
            AddPoly (numPoly, poly, 0, A, x, y + 0.5, -1, -1, x, y - 0.5);
         }
         if (flag_B) {
            AddPoly (numPoly, poly, 0, B, x, y - 0.5, -1, -1, x, y + 0.5);
         }
      } else {
         /* ABAC */
         if (flag_A) {
            AddPoly (numPoly, poly, 0, A, x, y + 0.5, -1, -1, x, y - 0.5);
         }
         if (flag_B) {
            AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
         }
         if (flag_D) {
            AddPoly (numPoly, poly, 1, D, x, y - 0.5, x, y, x + 0.5, y);
         }
      }
   } else if (CellsEqual (flag_B, B, flag_C, C)) {
      if (CellsEqual (flag_A, A, flag_D, D)) {
         /* ABBA */
         if (flag_A) {
            AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
            AddPoly (numPoly, poly, 1, A, x, y - 0.5, x, y, x + 0.5, y);
         }
         if (flag_B) {
            AddPoly (numPoly, poly, 1, B, x - 0.5, y, x, y, x, y - 0.5);
            AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
         }
      } else if (CellsEqual (flag_B, B, flag_D, D)) {
         /* ABBB */
         if (flag_A) {
            AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
         }
         if (flag_B) {
            AddPoly (numPoly, poly, 1, B, x - 0.5, y, x, y, x, y + 0.5);
         }
      } else {
         /* ABBC */
         if (flag_A) {
            AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
         }
         if (flag_B) {
            AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
            AddPoly (numPoly, poly, 1, B, x - 0.5, y, x, y, x, y - 0.5);
         }
         if (flag_D) {
            AddPoly (numPoly, poly, 1, D, x, y - 0.5, x, y, x + 0.5, y);
         }
      }
   } else if (CellsEqual (flag_A, A, flag_D, D)) {
      /* ABCA */
      if (flag_A) {
         AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
         AddPoly (numPoly, poly, 1, A, x, y - 0.5, x, y, x + 0.5, y);
      }
      if (flag_B) {
         AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
      }
      if (flag_C) {
         AddPoly (numPoly, poly, 1, C, x - 0.5, y, x, y, x, y - 0.5);
      }
   } else if (CellsEqual (flag_B, B, flag_D, D)) {
      /* ABCB */
      if (flag_A) {
         AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
      }
      if (flag_B) {
         AddPoly (numPoly, poly, 0, B, x, y - 0.5, -1, -1, x, y + 0.5);
      }
      if (flag_C) {
         AddPoly (numPoly, poly, 1, C, x - 0.5, y, x, y, x, y - 0.5);
      }
   } else if (CellsEqual (flag_C, C, flag_D, D)) {
      /* ABCC */
      if (flag_A) {
         AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
      }
      if (flag_B) {
         AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
      }
      if (flag_C) {
         AddPoly (numPoly, poly, 0, C, x - 0.5, y, -1, -1, x + 0.5, y);
      }
   } else {
      /* ABCD */
      if (flag_A) {
         AddPoly (numPoly, poly, 1, A, x, y + 0.5, x, y, x - 0.5, y);
      }
      if (flag_B) {
         AddPoly (numPoly, poly, 1, B, x + 0.5, y, x, y, x, y + 0.5);
      }
      if (flag_C) {
         AddPoly (numPoly, poly, 1, C, x - 0.5, y, x, y, x, y - 0.5);
      }
      if (flag_D) {
         AddPoly (numPoly, poly, 1, D, x, y - 0.5, x, y, x + 0.5, y);
      }
   }
}

/*****************************************************************************
 * Grid2BigPoly() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To create a set of chains which represent the polygons which are
 * obtained when one merges the same valued cells in a grid.
 *
 * ARGUMENTS
 *    poly = The collection of contour chains. (Output)
 * numPoly = The number of polygons created so far. (Output)
 *      Nx = The x dimmension of Data. (Input)
 *      Ny = The y dimmension of Data. (Input)
 *    Data = The grid of data values to merge together. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   -1 if Nx or Ny is 0.
 *    0 otherwise.
 *
 * HISTORY
 *   9/2003 Arthur Taylor (MDL/RSIS): Created.
 *  10/2003 AAT: Modified to use PolyType.
 *
 * NOTES
 *   Assumes scan mode of 0100.  If that is incorrect, polygons could be
 * inside out... To switch from 0100 to 0000, you have to reverse all chains.
 *****************************************************************************
 */
int Grid2BigPoly (polyType **poly, int *numPoly, int Nx, int Ny, double *Data)
{
   int x, y;            /* The current cell intersection to add. */

   if ((Nx == 0) || (Ny == 0)) {
      return -1;
   }
   /* Do the interior points first. */
   if ((Ny > 2) && (Nx > 2)) {
      for (y = 1; y < Ny; y++) {
         for (x = 1; x < Nx; x++) {
            AddCell (numPoly, poly, x, y, 1, Data[(x - 1) + y * Nx],
                     1, Data[x + y * Nx], 1, Data[(x - 1) + (y - 1) * Nx],
                     1, Data[x + (y - 1) * Nx]);
         }
      }
   }
   for (x = 1; x < Nx; x++) {
      /* Do the y == 0 edge. */
      AddCell (numPoly, poly, x, 0, 1, Data[(x - 1) + 0 * Nx],
               1, Data[x + 0 * Nx], 0, 9999, 0, 9999);
      /* Do the y == Ny edge. */
      AddCell (numPoly, poly, x, Ny, 0, 9999, 0, 9999,
               1, Data[(x - 1) + (Ny - 1) * Nx], 1, Data[x + (Ny - 1) * Nx]);
   }
   for (y = 1; y < Ny; y++) {
      /* Do the x == 0 edge. */
      AddCell (numPoly, poly, 0, y, 0, 9999, 1, Data[0 + y * Nx], 0, 9999,
               1, Data[0 + (y - 1) * Nx]);
      /* Do the x == Nx edge. */
      AddCell (numPoly, poly, Nx, y, 1, Data[(Nx - 1) + y * Nx], 0, 9999,
               1, Data[(Nx - 1) + (y - 1) * Nx], 0, 9999);
   }
   /* Do the x == 0, y == 0 corner. */
   AddCell (numPoly, poly, 0, 0, 0, 9999, 1, Data[0 + 0 * Nx], 0, 9999,
            0, 9999);
   /* Do the x == Nx, y == 0 corner. */
   AddCell (numPoly, poly, Nx, 0, 1, Data[(Nx - 1) + 0 * Nx], 0, 9999,
            0, 9999, 0, 9999);
   /* Do the x == 0, y == Ny corner. */
   AddCell (numPoly, poly, 0, Ny, 0, 9999, 0, 9999, 0, 9999,
            1, Data[0 + (Ny - 1) * Nx]);
   /* Do the x == Nx, y == Ny corner. */
   AddCell (numPoly, poly, Nx, Ny, 0, 9999, 0, 9999,
            1, Data[(Nx - 1) + (Ny - 1) * Nx], 0, 9999);
   return 0;
}

/*****************************************************************************
 * ConvertChain2LtLn() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Convert the chain of grid cells into a chain of lat/lon points paying
 * attention to where the dateline falls.  This allows us to split polygons at
 * the dateline.
 *   Took advantage of the fact that the polyType structure has a "finList"
 * and an "actList".  By the time the data has gone through gribCompactPolys,
 * the "finList" has data, and the actList doesn't.  This procedure converts
 * the "finList" points, and stores them on the actList.
 *
 * ARGUMENTS
 *    poly = The collection of contour chains for the Big Polygons. (Input)
 * numPoly = The number of polygons created. (Input)
 *     map = Holds the current map projection info to compute from. (Input)
 * LatLon_Decimal = How many decimals to round the lat/lon's to. (Input)   
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  11/2003 Arthur Taylor (MDL/RSIS): Created.
 *   3/2004 AAT: Updated the dateline split method to get the extra points
 *               that are actually on the dateline.
 *
 * NOTES
 *****************************************************************************
 */
void ConvertChain2LtLn (polyType *poly, int numPoly, myMaparam *map,
                        sChar LatLon_Decimal)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   chainNode *cur;      /* The current node to convert. */
   chainNode *cur2;     /* Used to help free the finList nodes as we go. */
   float x = 0, y = 0;  /* Used to assist in giving grid cell resolution to
                         * the lines. */
   double lat, lon;     /* The location of the current point. */
   double oldLat = 0;   /* The old lat. */
   double oldLon = 0;   /* The old lon. */
   int splitIndex;      /* -1 if not on split polygon, otherwise index to
                         * split polygon. */
   int f_first;         /* true if this is the first node in the chain. */
   double newLat;       /* latitude where line segment crosses the dateline */

   for (i = 0; i < numPoly; i++) {
      myAssert (poly[i].numAct == 0);
      poly[i].numAct = poly[i].numFin;
      poly[i].actList = (chainType *) realloc ((void *) poly[i].actList,
                                               poly[i].numFin *
                                               sizeof (chainType));
      for (j = 0; j < poly[i].numFin; j++) {
         cur = poly[i].finList[j].head;
         f_first = 1;
         poly[i].actList[j].head = NULL;
         poly[i].actList[j].tail = NULL;
         splitIndex = -1;

         if (cur != NULL) {
            x = cur->x;
            y = cur->y;
            oldLat = 0;
            oldLon = 0;
         }
         while (cur != NULL) {
            /* The + .5 is because the chain algorithm put the values in the
             * CENTER of a grid that starts at 0,0.  The map projection was
             * set up so that the values are on latice points, on a grid that
             * starts at 1,1. (see Grid2BigPoly()) */
            if (f_first) {
               x = cur->x;
               y = cur->y;
               myCxy2ll (map, x + .5, y + .5, &lat, &lon);
               lat = myRound (lat, LatLon_Decimal);
               lon = myRound (lon, LatLon_Decimal);
               AddPoint (&(poly[i].actList[j]), lon, lat, 0, 0);
               oldLat = lat;
               oldLon = lon;
            } else {
               /* The following is so that we don't jump over latice points
                * when walking along the edge. */
               while (x != cur->x) {
                  if (x > cur->x) {
                     x--;
                  } else {
                     x++;
                  }
                  myCxy2ll (map, x + .5, y + .5, &lat, &lon);
                  lat = myRound (lat, LatLon_Decimal);
                  lon = myRound (lon, LatLon_Decimal);
                  if (fabs (oldLon - lon) > 180) {
                     DateLineLat (oldLon, oldLat, lon, lat, &newLat);
                     newLat = myRound (newLat, LatLon_Decimal);
                     if (splitIndex == -1) {
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[j]), -180, newLat, 0,
                                     0);
                        } else {
                           AddPoint (&(poly[i].actList[j]), 180, newLat, 0,
                                     0);
                        }
                        /* Make room for new (split off) chain */
                        splitIndex = poly[i].numAct;
                        poly[i].numAct++;
                        poly[i].actList = (chainType *)
                              realloc (poly[i].actList, poly[i].numAct *
                                       sizeof (chainType));
                        poly[i].actList[splitIndex].head = NULL;
                        poly[i].actList[splitIndex].tail = NULL;
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[splitIndex]), 180,
                                     newLat, 0, 0);
                        } else {
                           AddPoint (&(poly[i].actList[splitIndex]), -180,
                                     newLat, 0, 0);
                        }
                     } else {
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[splitIndex]), -180,
                                     newLat, 0, 0);
                        } else {
                           AddPoint (&(poly[i].actList[splitIndex]), 180,
                                     newLat, 0, 0);
                        }
                        /* Add the first point on the splitIndex list to
                         * close the loop. */
                        AddPoint (&(poly[i].actList[splitIndex]),
                                  poly[i].actList[splitIndex].head->x,
                                  poly[i].actList[splitIndex].head->y, 0, 0);
                        splitIndex = -1;
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[j]), 180, newLat, 0,
                                     0);
                        } else {
                           AddPoint (&(poly[i].actList[j]), -180, newLat, 0,
                                     0);
                        }
                     }
                  }
                  if (splitIndex != -1) {
                     AddPoint (&(poly[i].actList[splitIndex]), lon, lat, 0,
                               0);
                  } else {
                     AddPoint (&(poly[i].actList[j]), lon, lat, 0, 0);
                  }
                  oldLat = lat;
                  oldLon = lon;
               }
               while (y != cur->y) {
                  if (y > cur->y) {
                     y--;
                  } else {
                     y++;
                  }
                  myCxy2ll (map, x + .5, y + .5, &lat, &lon);
                  lat = myRound (lat, LatLon_Decimal);
                  lon = myRound (lon, LatLon_Decimal);
                  if (fabs (oldLon - lon) > 180) {
                     DateLineLat (oldLon, oldLat, lon, lat, &newLat);
                     newLat = myRound (newLat, LatLon_Decimal);
                     if (splitIndex == -1) {
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[j]), -180, newLat, 0,
                                     0);
                        } else {
                           AddPoint (&(poly[i].actList[j]), 180, newLat, 0,
                                     0);
                        }
                        /* Make room for new (split off) chain */
                        splitIndex = poly[i].numAct;
                        poly[i].numAct++;
                        poly[i].actList = (chainType *)
                              realloc (poly[i].actList, poly[i].numAct *
                                       sizeof (chainType));
                        poly[i].actList[splitIndex].head = NULL;
                        poly[i].actList[splitIndex].tail = NULL;
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[splitIndex]), 180,
                                     newLat, 0, 0);
                        } else {
                           AddPoint (&(poly[i].actList[splitIndex]), -180,
                                     newLat, 0, 0);
                        }
                     } else {
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[splitIndex]), -180,
                                     newLat, 0, 0);
                        } else {
                           AddPoint (&(poly[i].actList[splitIndex]), 180,
                                     newLat, 0, 0);
                        }
                        /* Add the first point on the splitIndex list to
                         * close the loop. */
                        AddPoint (&(poly[i].actList[splitIndex]),
                                  poly[i].actList[splitIndex].head->x,
                                  poly[i].actList[splitIndex].head->y, 0, 0);
                        splitIndex = -1;
                        if (oldLon < 0) {
                           AddPoint (&(poly[i].actList[j]), 180, newLat, 0,
                                     0);
                        } else {
                           AddPoint (&(poly[i].actList[j]), -180, newLat, 0,
                                     0);
                        }
                     }
                  }
                  if (splitIndex != -1) {
                     AddPoint (&(poly[i].actList[splitIndex]), lon, lat, 0,
                               0);
                  } else {
                     AddPoint (&(poly[i].actList[j]), lon, lat, 0, 0);
                  }
                  oldLat = lat;
                  oldLon = lon;
               }
            }
            /* Free up data on the "finList" as we go. */
            cur2 = cur->next;
            free (cur);
            cur = cur2;
            f_first = 0;
         }
      }
      /* Free up the "finList" as we go. */
      free (poly[i].finList);
      poly[i].finList = NULL;
      poly[i].numFin = 0;
   }
}

/*****************************************************************************
 * CreateBigPolyShp() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Create the .shp and .shx files for the Big Polygons.
 *
 * ARGUMENTS
 * filename = Name of file to save to. (Output)
 *     poly = The collection of contour chains for the Big Polygons. (Input)
 *  numPoly = The number of polygons created. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 *
 * HISTORY
 *  10/2003 Arthur Taylor (MDL/RSIS): Created.
 *  10/2003 AAT: Fixed the grid offset by adding .5
 *  10/2003 AAT: Fixed the initialization of polygon bounds
 *          Shifted "if (numPnts == 0)" to before "numPnts++".
 *  10/2003 Matthew T. Kallio (matt@wunderground.com):
 *          FWRITE_LIT (&parts, sizeof (sInt4), numParts, sfp);
 *       => FWRITE_LIT (parts, sizeof (sInt4), numParts, sfp);
 *  12/2003 AAT Added "ConvertChain2LtLn" before this, so now uses actList
 *          instead of finList
 *
 * NOTES
 *****************************************************************************
 */
int CreateBigPolyShp (char *filename, polyType *poly, int numPoly)
{
   FILE *sfp;           /* The open file pointer for .shp */
   FILE *xfp;           /* The open file pointer for .shx. */
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[] = {
      0., 0., 0., 0., 0., 0., 0., 0.
   };                   /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   sInt4 dataType = 5;  /* Polygon shp type. */
   chainNode *cur;      /* The current node (to print). */
   double PolyBound[] = {
      0., 0., 0., 0.
   };                   /* Used for holding the polygon bounds. */
   sInt4 offset;        /* Where we are in the .shp file. */
   sInt4 startOffset;   /* Start of a polygon record. */
   sInt4 *shxData;      /* start of each polygon for use in the .shx file. */
   sInt4 curRec[2];     /* rec number, and content length. */
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   double lat, lon;     /* The location of the current point. */
   sInt4 *parts = NULL; /* The start index of each part in the polygon. */
   sInt4 numParts;      /* Number of parts in the current polygon. */
   sInt4 numPnts;       /* Number of points in current polygon. */

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }

   /* Start Writing header in first 100 bytes. */
   Head1[0] = 9994;     /* ArcView identifier. */
   memset ((Head1 + 1), 0, 5 * sizeof (sInt4)); /* set 5 unused to 0 */
   Head1[6] = 0;        /* Compute .shp file size (in 2 byte words) later. */
   FWRITE_BIG (Head1, sizeof (sInt4), 7, sfp);
   Head2[0] = 1000;     /* ArcView version identifier. */
   Head2[1] = dataType; /* Signal that these are polygon data. */
   FWRITE_LIT (Head2, sizeof (sInt4), 2, sfp);
   /* Write out initial guess for bounds... Need to revisit. */
   FWRITE_LIT (Bounds, sizeof (double), 8, sfp);

   /* Start Writing data. */
   curRec[1] = 0;       /* need to compute content length in (2 byte word) */
   offset = 100;
   shxData = (sInt4 *) malloc (numPoly * sizeof (sInt4));
   for (i = 0; i < numPoly; i++) {
      myAssert (poly[i].numFin == 0);
      startOffset = offset;
      /* Record number. */
      curRec[0] = i + 1;
      shxData[curRec[0] - 1] = startOffset;
      /* Write record header. */
      FWRITE_BIG (curRec, sizeof (sInt4), 2, sfp);
      /* Write the data type. */
      FWRITE_LIT (&dataType, sizeof (sInt4), 1, sfp);
      /* Write polygons bounds */
      FWRITE_LIT (PolyBound, sizeof (double), 4, sfp);
      /* Write out the Polygon Specs. */
      numParts = poly[i].numAct;
      numPnts = 0;
      parts = (sInt4 *) realloc ((void *) parts, numParts * sizeof (sInt4));
      FWRITE_LIT (&numParts, sizeof (sInt4), 1, sfp);
      FWRITE_LIT (&numPnts, sizeof (sInt4), 1, sfp);
      FWRITE_LIT (parts, sizeof (sInt4), numParts, sfp);
      offset += (5 * sizeof (sInt4) + 4 * sizeof (double) +
                 numParts * sizeof (sInt4));
      for (j = 0; j < poly[i].numAct; j++) {
         parts[j] = numPnts;
         cur = poly[i].actList[j].head;
         while (cur != NULL) {
            lon = cur->x;
            lat = cur->y;
            /* Update PolyBound. */
            if (numPnts == 0) {
               PolyBound[0] = PolyBound[2] = lon;
               PolyBound[1] = PolyBound[3] = lat;
            } else {
               if (lon < PolyBound[0]) {
                  PolyBound[0] = lon;
               } else if (lon > PolyBound[2]) {
                  PolyBound[2] = lon;
               }
               if (lat < PolyBound[1]) {
                  PolyBound[1] = lat;
               } else if (lat > PolyBound[3]) {
                  PolyBound[3] = lat;
               }
            }
            FWRITE_LIT (&lon, sizeof (double), 1, sfp);
            FWRITE_LIT (&lat, sizeof (double), 1, sfp);
            offset += 2 * sizeof (double);
            numPnts++;
            cur = cur->next;
         }
      }
      /* Seek the start of the poly record, and jump over the record # */
      fseek (sfp, startOffset + 4, SEEK_SET);
      /* Write content length in (2 byte words) */
      curRec[1] = (offset - (startOffset + 2 * sizeof (sInt4))) / 2;
      FWRITE_BIG (&(curRec[1]), sizeof (sInt4), 1, sfp);
      /* Jump over "Write the data type". */
      fseek (sfp, sizeof (sInt4), SEEK_CUR);
      /* Write polygons bounds */
      FWRITE_LIT (PolyBound, sizeof (double), 4, sfp);
      /* Jump over numParts. */
      fseek (sfp, sizeof (sInt4), SEEK_CUR);
      FWRITE_LIT (&numPnts, sizeof (sInt4), 1, sfp);
      FWRITE_LIT (parts, sizeof (sInt4), numParts, sfp);
      fseek (sfp, offset, SEEK_SET);

      /* Update Bounds of all data. */
      if (curRec[0] == 1) {
         Bounds[0] = PolyBound[0];
         Bounds[1] = PolyBound[1];
         Bounds[2] = PolyBound[2];
         Bounds[3] = PolyBound[3];
      } else {
         if (Bounds[0] > PolyBound[0])
            Bounds[0] = PolyBound[0];
         if (Bounds[1] > PolyBound[1])
            Bounds[1] = PolyBound[1];
         if (Bounds[2] < PolyBound[2])
            Bounds[2] = PolyBound[2];
         if (Bounds[3] < PolyBound[3])
            Bounds[3] = PolyBound[3];
      }
   }
   free (parts);
   /* Store the updated Bounds. */
   fseek (sfp, 36, SEEK_SET);
   /* FWRITE = 4 since we are only updating 4 bounds (not 8). */
   FWRITE_LIT (Bounds, sizeof (double), 4, sfp);
   /* Store the updated file length (in 2 byte words). */
   Head1[6] = offset / 2;
   fseek (sfp, 24, SEEK_SET);
   FWRITE_BIG (&(Head1[6]), sizeof (sInt4), 1, sfp);
   fflush (sfp);
   fclose (sfp);

   filename[strlen (filename) - 1] = 'x';
   if ((xfp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   /* Write ArcView header. */
   Head1[6] = (100 + 8 * numPoly) / 2; /* shx file size (in words). */
   FWRITE_BIG (Head1, sizeof (sInt4), 7, xfp);
   FWRITE_LIT (Head2, sizeof (sInt4), 2, xfp);
   FWRITE_LIT (Bounds, sizeof (double), 8, xfp);
   for (i = 0; i < numPoly; i++) {
      /* Offset in 2 bytes words for current record. */
      curRec[0] = shxData[i] / 2;
      /* Compute record content length in 2 byte words. */
      if (i < numPoly - 1) {
         curRec[1] = (shxData[i + 1] - (shxData[i] + 2 * sizeof (sInt4))) / 2;
      } else {
         curRec[1] = (offset - (shxData[i] + 2 * sizeof (sInt4))) / 2;
      }
      FWRITE_BIG (curRec, sizeof (sInt4), 2, xfp);
   }
   fflush (xfp);
   fclose (xfp);
   free (shxData);
   return 0;
}

#ifdef CHAIN_DEBUG
/*****************************************************************************
 * PrintPolys() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Used during testing / debugging to print the chain of contours.
 *
 * ARGUMENTS
 *    poly = The collection of contour chains. (Output)
 * numPoly = The number of polygons created so far. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  6/2002 Arthur Taylor (MDL/RSIS): Created.
 * 10/2003 AAT: Modified to use PolyType.
 *
 * NOTES
 *****************************************************************************
 */
void PrintPolys (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   int cnt;             /* Check if we have > 100 nodes. */
   chainNode *node;     /* The current node (to print). */

   for (i = 0; i < numPoly; i++) {
      myAssert (poly[i].numAct == 0);
      for (j = 0; j < poly[i].numFin; j++) {
         printf ("%d (%f) :: ", j, poly[i].finList[j].value);
         node = poly[i].finList[j].head;
         cnt = 0;
         while (node != NULL) {
            cnt++;
            printf ("(%f %f) ,", node->x, node->y);
            if (node == poly[i].finList[j].tail) {
               printf (":: NULL? ");
            }
            node = node->next;
            if (cnt == 100)
               return;
         }
         printf ("<check> \n");
      }
   }
}

/*****************************************************************************
 * main() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To test the Grid2BigPoly() procedure.
 *
 * ARGUMENTS
 * argc = The number of command line arguments. (Input)
 * argv = The values of the command line arguments. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  9/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int main (int argc, char **argv)
{
   double Data[16];
   int Nx = 4, Ny = 4;
   polyType *poly;
   int numPoly;
   int i, x, y;

   /* Set up some test data. */
   for (i = 0; i < Nx * Ny; i++) {
      Data[i] = 0;
   }
   x = 0;
   y = 0;
   Data[x + y * Nx] = 1;
   y = 1;
   Data[x + y * Nx] = 1;
   y = 2;
   Data[x + y * Nx] = 1;
   x = 1;
   y = 2;
   Data[x + y * Nx] = 1;
   y = 3;
   Data[x + y * Nx] = 1;
   x = 2;
   y = 3;
   Data[x + y * Nx] = 1;
   x = 3;
   y = 3;
   Data[x + y * Nx] = 1;
   y = 2;
   Data[x + y * Nx] = 1;
   y = 1;
   Data[x + y * Nx] = 1;
   y = 0;
   Data[x + y * Nx] = 1;
   x = 2;
   y = 0;
   Data[x + y * Nx] = 1;
   for (y = Ny - 1; y >= 0; y--) {
      for (x = 0; x < Nx; x++) {
         printf ("%3.1f ", Data[x + y * Nx]);
      }
      printf ("\n");
   }

   NewPolys (&poly, &numPoly);
   Grid2BigPoly (&poly, &numPoly, Nx, Ny, Data);

   CompactPolys (poly, numPoly);
   printf ("---------\n");
   PrintPolys (poly, numPoly);

   FreePolys (poly, numPoly);
   return 0;
}
#endif
