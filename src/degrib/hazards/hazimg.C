#include "TextString.H"
#include "create_mosaic_wrap.H"
/*
0   255 239 231     None    __None__         # NOTE: Use '_' in
1   253  99  71     TS.W    Tsunami Warning
2   255   0   0     TO.W    Tornado Warning
3   255  20 147     EW.W    Extreme Wind Warning
4   255 165   0     SV.W    Severe Thunderstorm Warning
5   139   0   0     FF.W    Flash Flood Warning
6   250 128 114             Shelter In Place Warning
7   127 255   0             Evacuation - Immediate
8   255 182 193             Civil Danger Warning
9    75   0 130             Nuclear Power Plant Warning
10   75   0 130             Radiological Hazard Warning
11   75   0 130             Hazardous Materials Warning
12  160  82  45             Fire Warning
13  255 182 193             Civil Emergency Message
14  192 192 192             Law Enforcement Warning
15  205  92  92     HF.W    Hurricane Force Wind Warning
16  205  92  92     HI.W    Inland Hurricane Warning
17  220  20  60     HU.W    Hurricane Warning
18  220  20  60     HU.W    Hurricane Warning
19  220  20  60             Typhoon Warning
20  220  20  60     TY.W    Typhoon Warning
21  255 165   0     MA.W    Special Marine Warning
22  255 165   0     MA.W    Marine Weather Statement (follow up to SMW)
23  255  69   0     BZ.W    Blizzard Warning
24  139   0 139     IS.W    Ice Storm Warning
25  178  34  34     TI.W    Inland Tropical Storm Warning
26  138  43 226     HS.W    Heavy Snow Warning
27  255 105 180     WS.W    Winter Storm Warning
28  218 165  32     HW.W    High Wind Warning
29  178  34  34     TR.W    Tropical Storm Warning
30  178  34  34     TR.W    Tropical Storm Warning
31  148   0 211     SR.W    Storm Warning
32  255   0 255     TS.A    Tsunami Watch
33   30 144 255             Avalanche Warning
34  139  69  19             Earthquake Warning
35  105 105 105             Volcano Warning
36   34 139  34     CF.W    Coastal Flood Warning
37   34 139  34     LS.W    Lakeshore Flood Warning
38    0 255   0     FA.W    Flood Warning (areal)
39    0 255   0     FL.W    Flood Warning (for forecast points)
40   34 139  34     SU.W    High Surf Warning
41  135 206 235     IP.W    Sleet Warning
42    0 139 139     LE.W    Lake Effect Snow Warning
43  199  21 133     EH.W    Excessive Heat Warning
44  255 228 196     DS.W    Dust Storm Warning
45  255 215   0     TO.A    PDS - Tornado Watch
46  255 255   0     TO.A    Tornado Watch
47  148   0 211     SV.A    PDS - Severe Thunderstorm Watch
48  219 112 147     SV.A    Severe Thunderstorm Watch
49   50 205  50     FF.A    Flash Flood Watch
50    0 255 255     TO.W    Severe Weather Statement
51    0 255 255     SV.W    Severe Weather Statement
52  154 205  50     FF.W    Flash Flood Statement
53  221 160 221     GL.W    Gale Warning
54    0 255   0     FA.W    Flood Statement (follow-up to areal Flood Warning)
55    0 255   0     FL.W    Flood Statement (follow-up to point Flood Warning)
56  210 105  30             Tsunami Advisory
57  176 196 222     WC.W    Wind Chill Warning
58    0   0 255     EC.W    Extreme Cold Warning
59    0   0 255     HZ.W    Hard Freeze Warning
60    0 255 255     FZ.W    Freeze Warning
61  255  20 147     FW.W    Red Flag Warning
62  255   0 255     HU.A    Hurricane Watch
63  255   0 255     HU.A    Hurricane Watch
64  255   0 255     TY.A    Typhoon Watch
65  255   0 255     HU.A    Typhoon Watch
66  147 112 219             Hurricane Local Statement
67  147 112 219             Typhoon Local Statement
68  176 224 230     SB.Y    Snow and Blowing Snow Advisory
69  106  90 205     ZR.Y    Freezing Rain Advisory
70  106  90 205             Freezing Drizzle Advisory
71  123 104 238     IP.Y    Sleet Advisory
72  222 184 135     WW.Y    Winter Weather Advisory
73   72 209 204     LB.Y    Lake Effect Snow and Blowing Snow Advisory
74   72 209 204     LE.Y    Lake Effect Snow Advisory
75  175 238 238     WC.Y    Wind Chill Advisory
76  255 127  80     HT.Y    Heat Advisory
77    0 255 127     FA.Y    Urban and Small Stream Flood Advisory
78    0 255 127     FA.Y    Small Stream Flood Advisory
79    0 255 127     FA.Y    Arroyo and Small Stream Flood Advisory
80    0 255 127     FA.Y    Flood Advisory
81    0 255 127     FA.Y    Hydrologic Advisory
82  124 252   0     LS.Y    Lakeshore Flood Advisory
83  124 252   0     CF.Y    Coastal Flood Advisory
84    0 255 127     FL.Y    Flood Advisory (for forecast points)
85  186  85 211     SU.Y    High Surf Advisory
86  173 216 230     BS.Y    Blowing Snow Advisory
87  102 153 204     SN.Y    Snow Advisory
88    0 191 255     UP.W    Heavy Freezing Spray Warning
89  240 230 140     SM.Y    Dense Smoke Advisory
90  216 191 216     SW.Y    Small Craft Advisory For Hazadous Seas
91  216 191 216     RB.Y    Small Craft Advisory for Rough Bar
92  216 191 216     SI.Y    Small Craft Advisory for Winds
93  216 191 216     SC.Y    Small Craft Advisory
94  216 191 216     BW.Y    Brisk Wind Advisory
95  216 191 216     SE.W    Hazardous Seas Warning
96  112 128 144     FG.Y    Dense Fog Advisory
97  210 180 140     LW.Y    Lake Wind Advisory
98  210 180 140     WI.Y    Wind Advisory
99  189 183 107     DU.Y    Blowing Dust Advisory
100 100 149 237     FR.Y    Frost Advisory
101 169 169 169     AF.Y    Ashfall Advisory
102   0 128 128     ZF.Y    Freezing Fog Advisory
103   0 191 255     UP.Y    Freezing Spray Advisory
104 128 128 128     AS.Y    Air Stagnation Advisory
105 165  42  42     LO.Y    Low Water Advisory
106 192 192 192             Local Area Emergency
107 244 164  96             Avalanche Watch
108 173 255  47     BZ.A    Blizzard Watch
109 240 128 128     TI.A    Inland Tropical Storm Watch
110 240 128 128     TR.A    Tropical Storm Watch
111 240 128 128     TR.A    Tropical Storm Watch
112 255 160 122     HI.A    Inland Hurricane Watch
113  70 130 180     WS.A    Winter Storm Watch
114 102 205 170     CF.A    Coastal Flood Watch
115 102 205 170     LS.A    Lakeshore Flood Watch
116  46 139  87     FL.A    Flood Watch (for forecast points)
117  46 139  87     FA.A    Flood Watch (areal)
118 184 134  11     HW.A    High Wind Watch
119 128   0   0     EH.A    Excessive Heat Watch
120   0   0 255     EC.A    Extreme Cold Watch
121  95 158 160     WC.A    Wind Chill Watch
122  72 209 204     LE.A    Lake Effect Snow Watch
123  65 105 225     FZ.A    Freeze Watch
124 255 222 173     FW.A    Fire Weather Watch
125 233 150 122             Extreme Fire Danger
126 255 215   0             Child Abduction Emergency
127 192 192 192             911 Telephone Outage
128 107 142  35     CF.S    Coastal Flood Statement
129 107 142  35     LS.S    Lakeshore Flood Statement
130 255 228 181             Special Weather Statement
131 255 239 213     MA.S    Marine Weather Statement
132 238 232 170             Hazardous Weather Outlook
133 152 251 152             Short Term Forecast
134 255 255 255             Administrative Message
135 240 255 255             Test

*/

int matchHazImageCodes(TextString);

int makeHazImageCodes(TextString thishaz, 
                      float& index_value )
{

  //Mike
/*  logDiag << "In makeHazImageCoded. thishaz1: " << thishaz1 << "  thishaz2: " << thishaz2
            << "thishaz3: " << thishaz3 << "  Intensity1: " << intensity1
            << "  Intensity2: " << intensity2 << "  Intensity3: "
            << intensity3 << std::endl; 
*/

  int index_valuenew =9999;
  index_valuenew=matchHazImageCodes(thishaz);
 
  if(index_valuenew < index_value)
       index_value = index_valuenew;
  

//Below because hazards WFO's make-up that are unofficial make it through and have random values and are plotted as deafult white vs the none "peach" color.  This numbere, 131, may have to be adjusted as more hazards are added. 0 should always be the lowest end.
  if(index_value < 0)
       index_value = 0;
  if(index_value > 131)
       index_value = 0;

   logDiag << "thishaz: " << thishaz << " index_value: " << index_value << std::endl; 

  return 0;
  
}

int matchHazImageCodes(TextString thishaz)
{

//  if (thishaz == "TS")
//    if (intensity == "W")
//        return 1;
//  if (thishaz == "TO")
//    if (intensity == "W")
//        return 2;
//  if (thishaz == "EW")
//    if (intensity == "W")
//        return 3;
//  if (thishaz == "SV")
//    if (intensity == "W")
//        return 4;
//  if (thishaz == "FF")
//    if (intensity == "W")
//        return 5;
  if (thishaz == "HF.W")
        return 15;
  if (thishaz == "HI.W")
        return 16;
  if (thishaz == "HU.W")
        return 17;
//  if (thishaz == "HU")
//    if (intensity == "W")
//        return 18;
  if (thishaz == "TY.W")
        return 20;
//  if (thishaz == "MA")
//    if (intensity == "W")
//        return 21;
//  if (thishaz == "MA")
//    if (intensity == "W")
//        return 22;
  if (thishaz == "BZ.W")
        return 23;
  if (thishaz == "IS.W")
        return 24;
  if (thishaz == "TI.W")
        return 25;
  if (thishaz == "HS.W")
        return 26;
  if (thishaz == "WS.W")
        return 27;  
  if (thishaz == "HW.W")
        return 28;
  if (thishaz == "TR.W")
        return 29;
//  if (thishaz == "TR")
//    if (intensity == "W")
//        return 30;
  if (thishaz == "SR.W")
        return 31;
//  if (thishaz == "TS")
//    if (intensity == "A")
//        return 32;
  if (thishaz == "CF.W")
        return 36;
  if (thishaz == "LS.W")
        return 37;
//  if (thishaz == "FA")
//    if (intensity == "W")
//        return 38;
//  if (thishaz == "FL")
//    if (intensity == "W")
//        return 39;
  if (thishaz == "SU.W")
        return 40;
  if (thishaz == "IP.W")
        return 41;
  if (thishaz == "LE.W")
        return 42;
  if (thishaz == "EH.W")
        return 43;
  if (thishaz == "DS.W")
        return 44;
//  if (thishaz == "TO")
//    if (intensity == "A")
//       return 45;
  if (thishaz == "TO.A")
        return 46;
//  if (thishaz == "SV")
//    if (intensity == "A")
//        return 47;
  if (thishaz == "SV.A")
        return 48;
  if (thishaz == "FF.A")
        return 49;
//  if (thishaz == "TO")
//    if (intensity == "W")
//        return 50;
//  if (thishaz == "SV")
//    if (intensity == "W")
//        return 51;
//  if (thishaz == "FF")
//    if (intensity == "W")
//        return 52;
  if (thishaz == "GL.W")
        return 53;
//  if (thishaz == "FA")
//    if (intensity == "W")
//        return 54;
//  if (thishaz == "FL")
//    if (intensity == "W")
//        return 55;
  if (thishaz == "WC.W")
        return 57;
  if (thishaz == "EC.W")
        return 58;
  if (thishaz == "HZ.W")
        return 59; 
  if (thishaz == "FZ.W")
        return 60;
  if (thishaz == "FW.W")
        return 61;
  if (thishaz == "HU.A")
        return 62;
//  if (thishaz == "HU")
//    if (intensity == "A")
//        return 63;
  if (thishaz == "TY.A")
        return 64;
//  if (thishaz == "HU")
//    if (intensity == "A")
//        return 65;
  if (thishaz == "SB.Y")
        return 68;
  if (thishaz == "ZR.Y")
        return 69;
  if (thishaz == "IP.Y")
        return 71;
  if (thishaz == "WW.Y")
        return 72;
  if (thishaz == "LB.Y")
        return 73;
  if (thishaz == "LE.Y")
        return 74;
  if (thishaz == "WC.Y")
        return 75;
  if (thishaz == "HT.Y")
        return 76;
  if (thishaz == "FA.Y")
        return 77;
//  if (thishaz == "FA")
//    if (intensity == "Y")
//        return 78;
//  if (thishaz == "FA")
//    if (intensity == "Y")
//        return 79;
//  if (thishaz == "FA")
//    if (intensity == "Y")
//        return 80;
//  if (thishaz == "FA")
//    if (intensity == "Y")
//        return 81;
  if (thishaz == "LS.Y")
        return 82;
  if (thishaz == "CF.Y")
        return 83;
//  if (thishaz == "FL")
//    if (intensity == "Y")
//        return 84;
  if (thishaz == "SU.Y")
        return 85;
  if (thishaz == "BS.Y")
        return 86;
  if (thishaz == "SN.Y")
        return 87;
  if (thishaz == "UP.W")
        return 88;
  if (thishaz == "SM.Y")
        return 89;
  if (thishaz == "SW.Y")
        return 90;
  if (thishaz == "RB.Y")
        return 91;
   if (thishaz == "SI.Y")
        return 92;
  if (thishaz == "SC.Y")
        return 93;
  if (thishaz == "BW.Y")
        return 94;
  if (thishaz == "SE.W")
        return 95;
  if (thishaz == "FG.Y")
        return 96;
  if (thishaz == "LW.Y")
        return 97;
  if (thishaz == "WI.Y")
        return 98;
  if (thishaz == "DU.Y")
        return 99;
  if (thishaz == "FR.Y")
        return 100;
  if (thishaz == "AF.Y")
        return 101;
  if (thishaz == "ZF.Y")
        return 102;
  if (thishaz == "UP.Y")
        return 103;
  if (thishaz == "AS.Y")
        return 104;
  if (thishaz == "LO.Y")
        return 105;
  if (thishaz == "BZ.A")
        return 108;
  if (thishaz == "TI.A")
        return 109;
  if (thishaz == "TR.A")
        return 110;
//  if (thishaz == "TR")
//    if (intensity == "A")
//        return 111;
  if (thishaz == "HI.A")
        return 112;
  if (thishaz == "WS.A")
        return 113;
  if (thishaz == "CF.A")
        return 114;
  if (thishaz == "LS.A")
        return 115;
//  if (thishaz == "FL")
//    if (intensity == "A")
//        return 116;
  if (thishaz == "FA.A")
        return 117;
  if (thishaz == "HW.A")
        return 118;
  if (thishaz == "EH.A")
        return 119;
  if (thishaz == "EC.A")
        return 120;
  if (thishaz == "WC.A")
        return 121;
  if (thishaz == "LE.A")
        return 122;
  if (thishaz == "FZ.A")
        return 123;
  if (thishaz == "FW.A")
        return 124;
  if (thishaz == "CF.S")
        return 128;
  if (thishaz == "LS.S")
        return 129;
  if (thishaz == "MA.S")
        return 131;
  else{
      logDebug << "thishaz" << thishaz << std::endl;
  }; 
};
