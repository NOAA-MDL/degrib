#include "TextString.H"
#include "create_mosaic_wrap.H"
/*
0       255     239     231     None    __None__         # NOTE: Use '_' in
1	253	99	71	TS.W	Tsunami	Tsunami	Tsunami Warning
2	255	0	0	TO.W	Tornado	Tornado	Tornado Warning
3	255	20	147	EW.W	Wind	Wind	Extreme Wind Warning
4	255	165	0	SV.W	T-Storm	TStorm	Severe Thunderstorm Warning
5	139	0	0	FF.W	Flood	Flood	Flash Flood Warning
6	250	128	114		Emergency		Shelter In Place Warning
7	127	255	0		Emergency		Evacuation - Immediate
8	255	182	193		Emergency		Civil Danger Warning
9	75	0	130		Emergency		Nuclear Power Plant Warning
10	75	0	130		Emergency		Radiological Hazard Warning
11	75	0	130		Emergency		Hazardous Materials Warning
12	160	82	45		Fire		Fire Warning
13	255	182	193		Emergency		Civil Emergency Message
14	192	192	192		Emergency		Law Enforcement Warning
15	205	92	92	HF.W	Wind	Hurricane	Hurricane Force Wind Warning
16	205	92	92	HI.W	Tropical	Hurricane	Hurricane Warning
17	220	20	60	HU.W	Tropical	Hurricane	Hurricane Warning
18	220	20	60	HU.W	Tropical	Hurricane	Hurricane Warning
19	220	20	60		Tropical		Typhoon Warning
20	220	20	60	TY.W	Tropical	Typhoon	Typhoon Warning
21	255	165	0	MA.W	Marine	Marine	Special Marine Warning
22	255	165	0	MA.W	Marine	Marine	Marine Weather Statement (follow up to SMW)
23	255	69	0	BZ.W	Blizzard	Blizzard	Blizzard Warning
24	139	0	139	IS.W	Ice	Ice	Ice Storm Warning
25	178	34	34	TI.W	Tropical	TropStrm	Tropical Storm Warning
26	138	43	226	HS.W	Snow	Snow	Heavy Snow Warning
27	255	105	180	WS.W	Winter_Wx	WinterWx	Winter Storm Warning
28	218	165	32	HW.W	Wind	Wind	High Wind Warning
29	178	34	34	TR.W	Tropical	TropStrm	Tropical Storm Warning
30	178	34	34	TR.W	Tropical	TropStrm	Tropical Storm Warning
31	148	0	211	SR.W	Marine	Storm	Storm Warning
32	255	0	255	TS.A	Tsunami	Tsunami	Tsunami Watch
33	30	144	255		Avalanche		Avalanche Warning
34	139	69	19		Earthquake		Earthquake Warning
35	105	105	105		Volcano		Volcano Warning
36	34	139	34	CF.W	Flood	Flood	Coastal Flood Warning
37	34	139	34	LS.W	Flood	Flood	Lakeshore Flood Warning
38	0	255	0	FA.W	Flood	Flood	Flood Warning (areal)
39	0	255	0	FL.W	Flood	Flood	Flood Warning (for forecast points)
40	34	139	34	SU.W	Marine	Surf	High Surf Warning
41	135	206	235	IP.W	Ice	Sleet	Sleet Warning
42	0	139	139	LE.W	Snow	Snow	Lake Effect Snow Warning
43	199	21	133	EH.W	Heat	Heat	Excessive Heat Warning
44	255	228	196	DS.W	Dust	Dust	Dust Storm Warning
45	0	0	255	HZ.W	Cold 	Freeze	Hard Freeze Warning
46	255	215	0	TO.A	Tornado	Tornado	PDS - Tornado Watch
47	255	215	0	TO.A	Tornado	Tornado	Tornado Watch
48	219	112	147	SV.A	T-Storm	TStorm	PDS - Severe Thunderstorm Watch
49	219	112	147	SV.A	T-Storm	TStorm	Severe Thunderstorm Watch
50	50	205	50	FF.A	Flood	Flood	Flash Flood Watch
51	0	255	255	TO.W	Severe	Severe	Severe Weather Statement
52	0	255	255	SV.W	Severe	Severe	Severe Weather Statement
53	154	205	50	FF.W	Flood	Flood	Flash Flood Statement
54	221	160	221	GL.W	Marine	Gale	Gale Warning
55	0	255	0	FA.W	Flood	Flood	Flood Statement (follow-up to areal Flood Warning)
56	0	255	0	FL.W	Flood	Flood	Flood Statement (follow-up to point Flood Warning)
57	210	105	30		Tsunami		Tsunami Advisory
58	176	196	222	WC.W	Cold	WindChill	Wind Chill Warning
59	0	0	255	EC.W	Cold	Cold	Extreme Cold Warning
60	0	255	255	FZ.W	Cold	Freeze	Freeze Warning
61	255	20	147	FW.W	Red_Flag	RedFlag	Red Flag Warning
62	255	0	255	HU.A	Tropical	Hurricane	Hurricane Watch
63	255	0	255	HU.A	Tropical	Hurricane	Hurricane Watch
64	255	0	255	TY.A	Tropical	Typhoon	Typhoon Watch
65	255	0	255	HU.A	Tropical	Typhoon	Typhoon Watch
66	147	112	219		Tropical		Hurricane Local Statement
67	147	112	219		Tropical		Typhoon Local Statement
68	176	224	230	SB.Y	Snow	Blowing	Snow and Blowing Snow Advisory
69	106	90	205	ZR.Y	Ice	FrzRain	Freezing Rain Advisory
70	106	90	205		Ice		Freezing Drizzle Advisory
71	123	104	238	IP.Y	Ice	Sleet	Sleet Advisory
72	222	184	135	WW.Y	Winter_Wx	WinterWx	Winter Weather Advisory
73	72	209	204	LB.Y	Snow	Blowing	Lake Effect Snow and Blowing Snow Advisory
74	72	209	204	LE.Y	Snow	Snow	Lake Effect Snow Advisory
75	175	238	238	WC.Y	Cold	WindChill	Wind Chill Advisory
76	255	127	80	HT.Y	Heat	Heat	Heat Advisory
77	0	255	127	FA.Y	Flood	Flood	Urban and Small Stream Flood Advisory
78	0	255	127	FA.Y	Flood	Flood	Small Stream Flood Advisory
79	0	255	127	FA.Y	Flood	Flood	Arroyo and Small Stream Flood Advisory
80	0	255	127	FA.Y	Flood	Flood	Flood Advisory
81	153	50	204	HF.A	Wind	Hurricane	Hurricane Force Wind Watch
82	0	255	127	FA.Y	Flood	Flood	Hydrologic Advisory
83	124	252	0	LS.Y	Flood	Flood	Lakeshore Flood Advisory
84	238	130	238	SR.A	Marine	Storm	Storm Watch
85	255	192	203	GL.A	Marine	Gale	Gale Watch
86	124	252	0	CF.Y	Flood	Flood	Coastal Flood Advisory
87	72	61	139	SE.A	Marine	Seas	Hazardous Seas Watch
88	188	143	143	UP.A	Marine	Marine	Heavy Freezing Spray Watch
89	0	255	127	FL.Y	Flood	Flood	Flood Advisory (for forecast points)
90	186	85	211	SU.Y	Marine	Surf	High Surf Advisory
91	173	216	230	BS.Y	Blowing	Blowing	Blowing Snow Advisory
92	102	153	204	SN.Y	Snow	Snow	Snow Advisory
93	0	191	255	UP.W	Marine	Marine	Heavy Freezing Spray Warning
94	240	230	140	SM.Y	Smoke	Smoke	Dense Smoke Advisory
95	216	191	216	SW.Y	Marine	SmCraft	Small Craft Advisory For Hazadous Seas
96	216	191	216	RB.Y	Marine	SmCraft	Small Craft Advisory for Rough Bar
97	216	191	216	SI.Y	Marine	SmCraft	Small Craft Advisory for Winds
98	65	105	225	HZ.A	Cold	Freeze	Hard Freeze Watch
99	216	191	216	SC.Y	Marine	SmCraft	Small Craft Advisory
100	216	191	216	BW.Y	Marine	Marine	Brisk Wind Advisory
101	216	191	216	SE.W	Marine	Seas	Hazardous Seas Warning
102	112	128	144	FG.Y	Fog	Fog	Dense Fog Advisory
103	210	180	140	LW.Y	Wind	Wind	Lake Wind Advisory
104	210	180	140	WI.Y	Wind	Wind	Wind Advisory
105	189	183	107	DU.Y	Blowing	Blowing	Blowing Dust Advisory
106	100	149	237	FR.Y	Cold	Frost	Frost Advisory
107	169	169	169	AF.Y	Ash	Ashfall	Ashfall Advisory
108	0	128	128	ZF.Y	Ice	Ice	Freezing Fog Advisory
109	0	191	255	UP.Y	Marine	Marine	Freezing Spray Advisory
110	128	128	128	AS.Y	Air_Stag	AirStag.	Air Stagnation Advisory
111	165	42	42	LO.Y	Marine	Marine	Low Water Advisory
112	192	192	192		Emergency		Local Area Emergency
113	244	164	96		Avalanche		Avalanche Watch
114	173	255	47	BZ.A	Blizzard	Blizzard	Blizzard Watch
115	240	128	128	TI.A	Tropical	TropStrm	Tropical Storm Watch
116	240	128	128	TR.A	Tropical	TropStrm	Tropical Storm Watch
117	240	128	128	TR.A	Tropical	TropStrm	Tropical Storm Watch
118	255	160	122	HI.A	Tropical	Hurricane	Hurricane Watch
119	70	130	180	WS.A	Winter_Wx	WinterWx	Winter Storm Watch
120	102	205	170	CF.A	Flood	Flood	Coastal Flood Watch
121	102	205	170	LS.A	Flood	Flood	Lakeshore Flood Watch
122	46	139	87	FL.A	Flood	Flood	Flood Watch (for forecast points)
123	46	139	87	FA.A	Flood	Flood	Flood Watch (areal)
124	184	134	11	HW.A	Wind	Wind	High Wind Watch
125	128	0	0	EH.A	Heat	Heat	Excessive Heat Watch
126	0	0	255	EC.A	Cold	Cold	Extreme Cold Watch
127	95	158	160	WC.A	Cold	WindChill	Wind Chill Watch
128	72	209	204	LE.A	Snow	Snow	Lake Effect Snow Watch
129	65	105	225	FZ.A	Cold	Freeze	Freeze Watch
130	255	222	173	FW.A	Fire	Fire	Fire Weather Watch
131	233	150	122		Fire		Extreme Fire Danger
132	255	215	0		Emergency		Child Abduction Emergency
133	192	192	192		Emergency		911 Telephone Outage
134	107	142	35	CF.S	Flood	Flood	Coastal Flood Statement
135	107	142	35	LS.S	Flood	Flood	Lakeshore Flood Statement
136	255	228	181		Info		Special Weather Statement
137	255	239	213	MA.S	Marine	Marine	Marine Weather Statement
138	238	232	170		Info		Hazardous Weather Outlook
139	152	251	152		Info		Short Term Forecast
140	255	255	255		Info		Administrative Message
141	240	255	255		Info		Test


Comment Key:
//   - No number
///  - Repeated Hazard
//// - Aren't being plotted


July 2008    Added new list      Allard 

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
/*  if(index_value < 0)
       index_value = 0;
  if(index_value > 131)
       index_value = 0;
*/
   logDiag << "thishaz: " << thishaz << " index_value: " << index_value << std::endl; 

  return 0;
  
}

int matchHazImageCodes(TextString thishaz)
{

  if (thishaz == "TS.W")
        return 1;
////  if (thishaz == "TO.W")
////        return 2;
////  if (thishaz == "EW.W")
////        return 3;
////  if (thishaz == "SV.W")
////        return 4;
////  if (thishaz == "FF.W")
////        return 5;
//  if (thishaz == "")
//        return 6;
//  if (thishaz == "")
//        return 7;
//  if (thishaz == "")
//        return 8;
//  if (thishaz == "")
//        return 9;
//  if (thishaz == "")
//        return 10;
//  if (thishaz == "")
//        return 11;
//  if (thishaz == "")
//        return 12;
//  if (thishaz == "")
//        return 13;
//  if (thishaz == "")
//        return 14;
  if (thishaz == "HF.W")
        return 15;
  if (thishaz == "HI.W")
        return 16;
  if (thishaz == "HU.W")
        return 17;
///  if (thishaz == "HU.W")
///        return 18;
//  if (thishaz == "")
//        return 19;
  if (thishaz == "TY.W")
        return 20;
////  if (thishaz == "MA.W")
////        return 21;
///  if (thishaz == "MA.W")
///        return 22;
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
///  if (thishaz == "TR.W")
///        return 30;
  if (thishaz == "SR.W")
        return 31;
  if (thishaz == "TS.A")
        return 32;
//  if (thishaz == "")
//        return 33;
//  if (thishaz == "")
//        return 34;
//  if (thishaz == "")
//        return 35;
  if (thishaz == "CF.W")
        return 36;
  if (thishaz == "LS.W")
        return 37;
////  if (thishaz == "FA.W")
////        return 38;
////  if (thishaz == "FL.W")
////        return 39;
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
  if (thishaz == "HZ.W")
        return 45;
  if (thishaz == "TO.A")
        return 46;
///  if (thishaz == "TO.A")
///        return 47;
  if (thishaz == "SV.A")
        return 48;
///  if (thishaz == "SV.A")
///        return 49;
  if (thishaz == "FF.A")
        return 50;
///  if (thishaz == "TO.W")
///        return 51;
///  if (thishaz == "SV.W")
///        return 52;
///  if (thishaz == "FF.W")
///        return 53;
  if (thishaz == "GL.W")
        return 54;
///  if (thishaz == "FA.W")
///        return 55;
///  if (thishaz == "FL.W")
///        return 56;
//  if (thishaz == "")
//        return 57;
  if (thishaz == "WC.W")
        return 58;
  if (thishaz == "EC.W")
        return 59;
  if (thishaz == "FZ.W")
        return 60;
  if (thishaz == "FW.W")
        return 61;
  if (thishaz == "HU.A")
        return 62;
///  if (thishaz == "HU.A")
///        return 63;
  if (thishaz == "TY.A")
        return 64;
///  if (thishaz == "HU.A")
///        return 65;
//  if (thishaz == "")
//        return 66;
//  if (thishaz == "")
//        return 67;
  if (thishaz == "SB.Y")
        return 68;
  if (thishaz == "ZR.Y")
        return 69;
//  if (thishaz == "")
//        return 70;
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
////  if (thishaz == "FA.Y")
////        return 77;
///  if (thishaz == "FA.Y")
///        return 78;
///  if (thishaz == "FA.Y")
///        return 79;
///  if (thishaz == "FA.Y")
///        return 80;
  if (thishaz == "HF.A")
        return 81;
///  if (thishaz == "FA.Y")
///        return 82;
  if (thishaz == "LS.Y")
        return 83;
  if (thishaz == "SR.A")
        return 84;
  if (thishaz == "GL.A")
        return 85;
  if (thishaz == "CF.Y")
        return 86;
  if (thishaz == "SE.A")
        return 87;
  if (thishaz == "UP.A")
        return 88;
////  if (thishaz == "FL.Y")
////        return 89;
  if (thishaz == "SU.Y")
        return 90;
  if (thishaz == "BS.Y")
        return 91;
  if (thishaz == "SN.Y")
        return 92;
  if (thishaz == "UP.W")
        return 93;
  if (thishaz == "SM.Y")
        return 94;
  if (thishaz == "SW.Y")
        return 95;
  if (thishaz == "RB.Y")
        return 96;
  if (thishaz == "SI.Y")
        return 97;
  if (thishaz == "HZ.A")
        return 98;
  if (thishaz == "SC.Y")
        return 99;
  if (thishaz == "BW.Y")
        return 100;
  if (thishaz == "SE.W")
        return 101;
  if (thishaz == "FG.Y")
        return 102;
  if (thishaz == "LW.Y")
        return 103;
  if (thishaz == "WI.Y")
        return 104;
  if (thishaz == "DU.Y")
        return 105;
  if (thishaz == "FR.Y")
        return 106;
  if (thishaz == "AF.Y")
        return 107;
  if (thishaz == "ZF.Y")
        return 108;
  if (thishaz == "UP.Y")
        return 109;
  if (thishaz == "AS.Y")
        return 110;
  if (thishaz == "LO.Y")
        return 111;
//  if (thishaz == "")
//        return 112;
//  if (thishaz == "")
//        return 113;
  if (thishaz == "BZ.A")
        return 114;
  if (thishaz == "TI.A")
        return 115;
  if (thishaz == "TR.A")
        return 116;
///  if (thishaz == "TR.A")
///        return 117;
  if (thishaz == "HI.A")
        return 118;
  if (thishaz == "WS.A")
        return 119;
  if (thishaz == "CF.A")
        return 120;
  if (thishaz == "LS.A")
        return 121;
////  if (thishaz == "FL.A")
////        return 122;
  if (thishaz == "FA.A")
        return 123;
  if (thishaz == "HW.A")
        return 124;
  if (thishaz == "EH.A")
        return 125;
  if (thishaz == "EC.A")
        return 126;
  if (thishaz == "WC.A")
        return 127;
  if (thishaz == "LE.A")
        return 128;
  if (thishaz == "FZ.A")
        return 129;
  if (thishaz == "FW.A")
        return 130;
//  if (thishaz == "")
//        return 131;
//  if (thishaz == "")
//        return 132;
//  if (thishaz == "")
//        return 133;
  if (thishaz == "CF.S")
        return 134;
////  if (thishaz == "LS.S")
////        return 135;
//  if (thishaz == "")
//        return 136;
////  if (thishaz == "MA.S")
////        return 137;
//  if (thishaz == "")
//        return 138;
//  if (thishaz == "")
//        return 139;
//  if (thishaz == "")
//        return 140;
//  if (thishaz == "")
//        return 141;

  else{
      logDebug << "thishaz" << thishaz << std::endl;
  }; 
        return 9999;

};
