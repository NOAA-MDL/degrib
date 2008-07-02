#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hazard.h"

typedef struct {
   char *abrev, *name;
   uChar number;
} HazTable;

enum {
   HAZ_FA, HAZ_BZ, HAZ_CF, HAZ_EH, HAZ_EC, HAZ_FW, HAZ_FF,
   HAZ_FZ, HAZ_GL, HAZ_HZ, HAZ_SE, HAZ_UP, HAZ_HW, HAZ_HF, HAZ_HU,
   HAZ_HI, HAZ_LE, HAZ_LS, HAZ_SV, HAZ_SR, HAZ_TO, HAZ_TR, HAZ_TI,
   HAZ_TS, HAZ_TY, HAZ_WC, HAZ_WS, HAZ_DS, HAZ_HS, HAZ_SU, HAZ_IS,
   HAZ_IP, HAZ_AS, HAZ_AF, HAZ_MH, HAZ_DU, HAZ_BS, HAZ_BW, HAZ_FG,
   HAZ_MF, HAZ_SM, HAZ_MS, HAZ_ZF, HAZ_ZR, HAZ_ZY, HAZ_FR, HAZ_HT,
   HAZ_LB, HAZ_LW, HAZ_LO, HAZ_SC, HAZ_SW, HAZ_RB, HAZ_SI, HAZ_SN,
   HAZ_SB, HAZ_WI, HAZ_WW, HAZ_MA, HAZ_NONE,
   HAZ_UNKNOWN
};

/* HAZ_EW, HAZ_FL */

/* See: http://products.weather.gov/PDD/HazardGrid0608.pdf */
HazTable HazCode[] = {
/* Watch section */
   /* 0 */ {"FA", "Areal Flood", HAZ_FA},
   /* 1 */ {"BZ", "Blizzard", HAZ_BZ},
   /* 2 */ {"CF", "Coastal Flood", HAZ_CF},
   /* 3 */ {"EH", "Excessive Heat", HAZ_EH},
   /* 4 */ {"EC", "Extreme Cold", HAZ_EC}, /* Excessive Cold */
   /* 5 */ {"FW", "Fire Weather", HAZ_FW},
   /* 6 */ {"FF", "Flash Flood", HAZ_FF},
   /* 7 */ {"FZ", "Freeze", HAZ_FZ},
   /* 8 */ {"GL", "Gale", HAZ_GL},
   /* 9 */ {"HZ", "Hard Freeze", HAZ_HZ},
   /* 10 */ {"SE", "Hazardous Seas", HAZ_SE},
   /* 11 */ {"UP", "Heavy Freezing Spray", HAZ_UP},
   /* 12 */ {"HW", "High Wind", HAZ_HW},
   /* 13 */ {"HF", "Hurricane Force", HAZ_HF},
   /* 14 */ {"HU", "Hurricane", HAZ_HU},
   /* 15 */ {"HI", "Hurricane Wind", HAZ_HI},  /* Inland Hurricane */
   /* 16 */ {"LE", "Lake Effect Snow", HAZ_LE},
   /* 17 */ {"LS", "Lakeshore Flood", HAZ_LS},
   /* 18 */ {"SV", "Severe Thunderstorm", HAZ_SV},
   /* 19 */ {"SR", "Storm", HAZ_SR},
   /* 20 */ {"TO", "Tornado", HAZ_TO},
   /* 21 */ {"TR", "Tropical Storm", HAZ_TR},
   /* 22 */ {"TI", "Tropical Storm Wind", HAZ_TI}, /* Inland Tropical Storm */
   /* 23 */ {"TS", "Tsunami", HAZ_TS},
   /* 24 */ {"TY", "Typhoon", HAZ_TY},
   /* 25 */ {"WC", "Wind Chill", HAZ_WC},
   /* 26 */ {"WS", "Winter Storm", HAZ_WS},
/* Warning section */
   /* BZ, CF */
   /* 27 */ {"DS", "Dust Storm", HAZ_DS},
   /* EH, EC, FZ, GL, HZ, SE, UP */
   /* 28 */ {"HS", "Heavy Snow", HAZ_HS},
   /* 29 */ {"SU", "High Surf", HAZ_SU},
   /* HW, HF, HU, HI */
   /* 30 */ {"IS", "Ice Storm", HAZ_IS},
   /* LE, LS, FW */
   /* 31 */ {"IP", "Sleet", HAZ_IP},
   /* SR, TR, TI, TS, TY, WC, WS */
/* Advisory section */
   /* 32 */ {"AS", "Air Stagnation", HAZ_AS},
   /* 33 */ {"AF", "Ashfall", HAZ_AF},  /* Duplicate 1 */
   /* 34 */ {"MH", "Ashfall", HAZ_MH},  /* Duplicate 1 */
   /* 35 */ {"DU", "Blowing Dust", HAZ_DU},
   /* 36 */ {"BS", "Blowing Snow", HAZ_BS},
   /* 37 */ {"BW", "Brisk Wind", HAZ_BW}, /* Blowing Wind */
   /* CF */
   /* 38 */ {"FG", "Dense Fog", HAZ_FG}, /* Duplicate 2 */
   /* 39 */ {"MF", "Dense Fog", HAZ_MF}, /* Duplicate 2 */
   /* 40 */ {"SM", "Dense Smoke", HAZ_SM}, /* Duplicate 3 */
   /* 41 */ {"MS", "Dense Smoke", HAZ_MS}, /* Duplicate 3 */
   /* 42 */ {"ZF", "Freezing Fog", HAZ_ZF},
   /* 43 */ {"ZR", "Freezing Rain", HAZ_ZR},
   /* UP */
   /* 45 */ {"ZY", "Heavy Freezing Spray", HAZ_ZY},
   /* 46 */ {"FR", "Frost", HAZ_FR},
   /* 47 */ {"HT", "Heat", HAZ_HT},
   /* SU, LE */
   /* 48 */ {"LB", "Lake Effect Snow and Blowing Snow", HAZ_LB},
   /* 49 */ {"LW", "Lake Wind", HAZ_LW},
   /* LS */
   /* 50 */ {"LO", "Low Water", HAZ_LO},
   /* IP */
   /* 51 */ {"SC", "Small Craft", HAZ_SC},
   /* 52 */ {"SW", "Small Craft for Hazardous Seas", HAZ_SW},
   /* 53 */ {"RB", "Small Craft for Rough Bar", HAZ_RB},
   /* 54 */ {"SI", "Small Craft for Winds", HAZ_SI},
   /* 55 */ {"SN", "Snow", HAZ_SN},
   /* 56 */ {"SB", "Snow and Blowing Snow", HAZ_SB},
   /* TS */
   /* 57 */ {"WI", "Wind", HAZ_WI},
   /* 58 */ {"WW", "Winter Weather", HAZ_WW},
/* Extra Based on GFE documentation */
   /* 59 */ {"MA", "Special Marine", HAZ_MA},
   /* 60 */ /* {"EW", "Excessive Wind", HAZ_EW}, */
   /* 61 */ /* {"FL", "Flood", HAZ_FL}, */
};

enum {
   SIG_A, SIG_S, SIG_Y, SIG_W, SIG_NONE
};


void FreeHazardString (HazardStringType * haz)
{
   int j;               /* Used to free all the english words. */

   for (j = 0; j < NUM_HAZARD_WORD; j++) {
      free (haz->english[j]);
   }
}

/* Based on the method used in "matchHazImageCodes()" */
static int HazardRank (uChar haz, uChar sig)
{
   if ((haz == HAZ_HF) && (sig == SIG_W))
      return 15;
   if ((haz == HAZ_HI) && (sig == SIG_W))
      return 16;
   if ((haz == HAZ_HU) && (sig == SIG_W))
      return 17;
   if ((haz == HAZ_TY) && (sig == SIG_W))
      return 20;
   if ((haz == HAZ_BZ) && (sig == SIG_W))
      return 23;
   if ((haz == HAZ_IS) && (sig == SIG_W))
      return 24;
   if ((haz == HAZ_TI) && (sig == SIG_W))
      return 25;
   if ((haz == HAZ_HS) && (sig == SIG_W))
      return 26;
   if ((haz == HAZ_WS) && (sig == SIG_W))
      return 27;
   if ((haz == HAZ_HW) && (sig == SIG_W))
      return 28;
   if ((haz == HAZ_TR) && (sig == SIG_W))
      return 29;
   if ((haz == HAZ_SR) && (sig == SIG_W))
      return 31;
   if ((haz == HAZ_CF) && (sig == SIG_W))
      return 36;
   if ((haz == HAZ_LS) && (sig == SIG_W))
      return 37;
   if ((haz == HAZ_SU) && (sig == SIG_W))
      return 40;
   if ((haz == HAZ_IP) && (sig == SIG_W))
      return 41;
   if ((haz == HAZ_LE) && (sig == SIG_W))
      return 42;
   if ((haz == HAZ_EH) && (sig == SIG_W))
      return 43;
   if ((haz == HAZ_DS) && (sig == SIG_W))
      return 44;
   if ((haz == HAZ_TO) && (sig == SIG_A))
      return 46;
   if ((haz == HAZ_SV) && (sig == SIG_A))
      return 48;
   if ((haz == HAZ_FF) && (sig == SIG_A))
      return 49;
   if ((haz == HAZ_GL) && (sig == SIG_W))
      return 53;
   if ((haz == HAZ_WC) && (sig == SIG_W))
      return 57;
   if ((haz == HAZ_EC) && (sig == SIG_W))
      return 58;
   if ((haz == HAZ_HZ) && (sig == SIG_W))
      return 59;
   if ((haz == HAZ_FZ) && (sig == SIG_W))
      return 60;
   if ((haz == HAZ_FW) && (sig == SIG_W))
      return 61;
   if ((haz == HAZ_HU) && (sig == SIG_A))
      return 62;
   if ((haz == HAZ_TY) && (sig == SIG_A))
      return 64;
   if ((haz == HAZ_SB) && (sig == SIG_Y))
      return 68;
   if ((haz == HAZ_ZR) && (sig == SIG_Y))
      return 69;
   if ((haz == HAZ_IP) && (sig == SIG_Y))
      return 71;
   if ((haz == HAZ_WW) && (sig == SIG_Y))
      return 72;
   if ((haz == HAZ_LB) && (sig == SIG_Y))
      return 73;
   if ((haz == HAZ_LE) && (sig == SIG_Y))
      return 74;
   if ((haz == HAZ_WC) && (sig == SIG_Y))
      return 75;
   if ((haz == HAZ_HT) && (sig == SIG_Y))
      return 76;
/*
   if ((haz == HAZ_FA) && (sig == SIG_Y))
      return 77;
*/
   if ((haz == HAZ_LS) && (sig == SIG_Y))
      return 82;
   if ((haz == HAZ_CF) && (sig == SIG_Y))
      return 83;
   if ((haz == HAZ_SU) && (sig == SIG_Y))
      return 85;
   if ((haz == HAZ_BS) && (sig == SIG_Y))
      return 86;
   if ((haz == HAZ_SN) && (sig == SIG_Y))
      return 87;
   if ((haz == HAZ_UP) && (sig == SIG_W))
      return 88;
   if ((haz == HAZ_SM) && (sig == SIG_Y))
      return 89;
   if ((haz == HAZ_SW) && (sig == SIG_Y))
      return 90;
   if ((haz == HAZ_RB) && (sig == SIG_Y))
      return 91;
   if ((haz == HAZ_SI) && (sig == SIG_Y))
      return 92;
   if ((haz == HAZ_SC) && (sig == SIG_Y))
      return 93;
   if ((haz == HAZ_BW) && (sig == SIG_Y))
      return 94;
   if ((haz == HAZ_SE) && (sig == SIG_W))
      return 95;
   if ((haz == HAZ_FG) && (sig == SIG_Y))
      return 96;
   if ((haz == HAZ_LW) && (sig == SIG_Y))
      return 97;
   if ((haz == HAZ_WI) && (sig == SIG_Y))
      return 98;
   if ((haz == HAZ_DU) && (sig == SIG_Y))
      return 99;
   if ((haz == HAZ_FR) && (sig == SIG_Y))
      return 100;
   if ((haz == HAZ_AF) && (sig == SIG_Y))
      return 101;
   if ((haz == HAZ_ZF) && (sig == SIG_Y))
      return 102;
   if ((haz == HAZ_UP) && (sig == SIG_Y))
      return 103;
   if ((haz == HAZ_AS) && (sig == SIG_Y))
      return 104;
   if ((haz == HAZ_LO) && (sig == SIG_Y))
      return 105;
   if ((haz == HAZ_BZ) && (sig == SIG_A))
      return 108;
   if ((haz == HAZ_TI) && (sig == SIG_A))
      return 109;
   if ((haz == HAZ_TR) && (sig == SIG_A))
      return 110;
   if ((haz == HAZ_HI) && (sig == SIG_A))
      return 112;
   if ((haz == HAZ_WS) && (sig == SIG_A))
      return 113;
   if ((haz == HAZ_CF) && (sig == SIG_A))
      return 114;
   if ((haz == HAZ_LS) && (sig == SIG_A))
      return 115;
   if ((haz == HAZ_FA) && (sig == SIG_A))
      return 117;
   if ((haz == HAZ_HW) && (sig == SIG_A))
      return 118;
   if ((haz == HAZ_EH) && (sig == SIG_A))
      return 119;
   if ((haz == HAZ_EC) && (sig == SIG_A))
      return 120;
   if ((haz == HAZ_WC) && (sig == SIG_A))
      return 121;
   if ((haz == HAZ_LE) && (sig == SIG_A))
      return 122;
   if ((haz == HAZ_FZ) && (sig == SIG_A))
      return 123;
   if ((haz == HAZ_FW) && (sig == SIG_A))
      return 124;
/*
   if ((haz == HAZ_CF) && (sig == SIG_S))
      return 128;
   if ((haz == HAZ_LS) && (sig == SIG_S))
      return 129;
   if ((haz == HAZ_MA) && (sig == SIG_S))
      return 131;
*/
   return 9999;
}

static int HazTable1 (HazardStringType * haz)
{
   int minVal;
   int i;
   int ans;

   minVal = 9999;
   for (i = 0; i < haz->numValid; i++) {
      ans = HazardRank (haz->haz[i], haz->sig[i]);
      if (minVal > ans)
         minVal = ans;
   }
   if (minVal > 131)
      minVal = 0;
   return minVal;
}

static void InitHazardString (HazardStringType * haz)
{
   int i;               /* Used to traverse all the words. */

   haz->numValid = 0;
   haz->SimpleCode = 0;
   for (i = 0; i < NUM_HAZARD_WORD; i++) {
      haz->haz[i] = HAZ_NONE;
      haz->sig[i] = SIG_NONE;
      haz->english[i] = NULL;
   }
}

static void Hazard2English (HazardStringType * haz)
{
   int i;               /* Loop counter over number of words. */
   char buffer[400];    /* Temporary storage as we build up the phrase. */

   for (i = 0; i < haz->numValid; i++) {
      buffer[0] = '\0';
      if (haz->haz[i] == HAZ_NONE) {
         strcat (buffer, "<None>");
      } else {
         strcat (buffer, HazCode[haz->haz[i]].name);
         switch (haz->sig[i]) {
            case SIG_A:
               strcat (buffer, " Watch");
               break;
            case SIG_S:
               strcat (buffer, " Statement");
               break;
            case SIG_Y:
               strcat (buffer, " Advisory");
               break;
            case SIG_W:
               strcat (buffer, " Warning");
               break;
         }
      }
      haz->english[i] = (char *) malloc ((strlen (buffer) + 1) *
                                          sizeof (char));
      strcpy (haz->english[i], buffer);
   }
}

int ParseHazardString (HazardStringType * haz, char *data, int simpleVer)
{
   char *start;         /* Where current phrase starts. */
   char *end;
   char *ptr;
   uChar word = 0;      /* Which word in sentence (# of ^ seen) */
   char f_continue;
   char f_found;
   int i;

   InitHazardString (haz);
   /* Handle 'None case. */
   if (strcmp (data, "<None>") == 0) {
      haz->numValid = 1;
      Hazard2English (haz);
      if (simpleVer == 1) {
         haz->SimpleCode = HazTable1 (haz);
      }
      return 0;
   }
   start = data;
   f_continue = 1;
   do {
      if ((end = strchr (start, '^')) != NULL) {
         *end = '\0';
      } else {
         f_continue = 0;
      }
      if ((ptr = strchr (start, '.')) == NULL) {
         if (f_continue) {
            *end = '^';
         }
         haz->numValid = 1;
         haz->english[0] = (char *) malloc ((strlen (data) + 1) *
                                             sizeof (char));
         strcpy (haz->english[0], data);
         /* For unknown hazards, f_valid = 0, so parseGrid sets it to missing */
         printf ("Unknown Hazard '%s' (Treating as missing)\n", data);
         if (simpleVer == 1) {
            haz->SimpleCode = 0;
         }
         return 1;
      }
      *ptr = '\0';
      f_found = 0;
      for (i = 0; i < (sizeof (HazCode) / sizeof (HazTable)); i++) {
         if (strcmp (start, HazCode[i].abrev) == 0) {
            f_found = 1;
            haz->haz[word] = HazCode[i].number;
            break;
         }
      }
      *ptr = '.';
      if (! f_found) {
         if (f_continue) {
            *end = '^';
         }
         haz->numValid = 1;
         haz->english[0] = (char *) malloc ((strlen (data) + 1) *
                                             sizeof (char));
         strcpy (haz->english[0], data);
         /* For unknown hazards, f_valid = 0, so parseGrid sets it to missing */
         printf ("Unknown Hazard '%s' (Treating as missing)\n", data);
         if (simpleVer == 1) {
            haz->SimpleCode = 0;
         }
         return 1;
      }
      switch (ptr[1]) {
         case 'A':
            haz->sig[word] = SIG_A;
            break;
         case 'S':
            haz->sig[word] = SIG_S;
            break;
         case 'Y':
            haz->sig[word] = SIG_Y;
            break;
         case 'W':
            haz->sig[word] = SIG_W;
            break;
         default:
            if (f_continue) {
               *end = '^';
            }
            haz->numValid = 1;
            haz->english[0] = (char *) malloc ((strlen (data) + 1) *
                                             sizeof (char));
            strcpy (haz->english[0], data);
            if (simpleVer == 1) {
               haz->SimpleCode = 0;
            }
            /* For unknown hazards, f_valid = 0, so parseGrid sets it to missing */
            printf ("Unknown Significance '%s' (Treating as missing)\n", data);
            return 1;
      }
      word++;
      if (f_continue) {
         *end = '^';
         start = end + 1;
      }
   } while (f_continue);

   haz->numValid = word;
   Hazard2English (haz);
   if (simpleVer == 1) {
      haz->SimpleCode = HazTable1 (haz);
   }
   return 0;
}
