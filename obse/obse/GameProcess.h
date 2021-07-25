#pragma once

#include "obse/GameForms.h"
#include "obse/GameExtraData.h"
#include "obse/GameThreads.h"
#include "NiNodes.h"
#include "NiHavokCharacters.h"

// virtual function analysis
//	num	arg	ret	B			L			ML			MH			H
//	000	0		destructor	destructor	destructor	destructor	destructor
//	001	?		?			<-			<-			<-			<-
//	002	0	32	retn0		retn3		retn2		retn1		retn0
//	003	2	x	null		?			<-			?			?
//	004	1	x	null		update? checks gametime hour, handles sleeping
//										?			?			?
//	005	2	x	null		update? checks gametime hour, sets position
//										<-			<-			<-
//	006	2	8	retn0		update? checks gametime hour, day
//										<-			?			?
//	007	1	x	null		set curHour
//										<-			<-			<-
//	008	0	x	null		update curHour and curPackedDate to now
//										<-			<-			<-
//	009	0	b	false		returns something based on unk008 and unk004
//										<-			<-			<-
//	00A	0	f	retn0		get curHour
//										<-			<-			<-
//	00B	0	32	retn0		get curPackedDate
//										<-			<-			<-
//	00C	1	x	null		set curPackedDate
//										<-			<-			<-
//	00D	0	32	retn0		extract current day from curPackedDate
//										<-			<-			<-
//	00E	0	32	retn0		extract current month from curPackedDate
//										<-			<-			<-
//	00F	0	32	retn0		extract current year from curPackedDate
//										<-			<-			<-
//	010	1	x	null		<-			set unk090	<-			<-
//	011	0	32	retn0		<-			get unk090	<-			<-
//	012	1	x	null		<-			<-			<-			?
//	013	0	x	null		<-			<-			<-			?
//	014	0	32	retn0		<-			<-			set unk148	<-
//	015	1	x	null		<-			<-			get unk148	<-
//	016	2	x	null		?			<-			<-			<-
//	017	0	x	null		<-			<-			unk15C = 0	<-
//	018	1	x	null		<-			<-			<-			?
//	019	0	8	retn0		<-			<-			get unk0C8	<-
//	01A	1	x	null		<-			<-			set unk0C8	<-
//	01B	2	8	retn0		<-			<-			<-			?
//	01C	3	x	write to an UInt32[3] passed in as arg0
//										<-			<-			<-
//	01D	1	32	retn0		?			<-			<-			<-
//	01E	1	32	retn0		?			<-			<-			<-
//	01F	1	f	retn0		?			<-			<-			<-
//	020	2	x	null		arg0 is a TESBoundObject *, code handles TESNPC, TESCreature
//										<-			<-			<-
//	021	3	x	null		?			<-			<-			<-
//	022	0	8	retn0		<-			<-			<-			get unk244
//	023	1	x	null		<-			<-			<-			set unk244
//	024	1	x	null		<-			<-			<-			?
//	025	0	x	null		<-			<-			<-			unk1E8 = 0
//	026	0	x	null		decrements unk088 by frameTime
//										<-			<-			<-
//	027	0	f	retn0		get unk088	<-			<-			<-
//	028	1	x	null		set unk088	<-			<-			<-
//	029	0	x	null		<-			<-			<-			unk19C = 0
//	02A	4	32	retn0		<-			<-			<-			?
//	02B	0	8	retn0		<-			<-			get unk168	<-
//	02C	1	x	null		<-			<-			set unk168	<-

// ### added in 1.2 patch
//	02C	1	x	null

//	02D	1	x	null		?			<-			<-			<-
//	02E	1	x	null		get unk01D	<-			<-			<-
//	02F	0	8	retn1		set unk01D	<-			<-			<-
//	030	1	x	null		<-			<-			set unk0D0	set unk0D0, unk1FC &= ~0x0340
//	031	1	8	retn1		<-			<-			get unk0D0	<-
//	032	0	32	retn0		get unk02C	<-			<-			<-
//	033	0	x	null		set unk02C, some bookkeeping
//										<-			?			<-
//	034	0	32	retn0		<-			<-			<-			<-
//	035	1	x	null		<-			<-			<-			<-
//	036	1	x	null		<-			<-			set unk044	<-
//	037	0	32	retn0		<-			<-			get unk048	<-
//	038	1	x	null		<-			<-			set unk048	<-
//	039	1	32	retn0		<-			<-			?			<-
//	03A	1	32	retn0		<-			<-			?			<-
//	03B	1	32	retn0		<-			<-			?			<-
//	03C	1	32	retn0		<-			<-			?			<-
//	03D	1	32	retn0		<-			<-			?			<-
//	03E	0	32	retn0		get unk038	<-			<-			<-
//	03F	1	x	null		set unk038	<-			<-			<-
//	040	2	8	retn0		<-			<-			?			<-
//	041	1	8	retn0		<-			<-			?			<-
//	042	1	8	retn0		<-			<-			?			<-
//	043	1	8	retn0		<-			<-			?			<-
//	044	1	8	retn0		<-			<-			?			<-
//	045	1	32	retn0		<-			<-			?			<-
//	046	1	32	retn0		<-			<-			?			<-
//	047	1	32	retn0		<-			<-			?			<-
//	048	1	32	retn0		<-			<-			?			<-
//	049	1	32	retn0		<-			<-			?			<-
//	04A	1	32	retn0		<-			<-			?			<-
//	04B	1	32	retn0		<-			<-			?			<-
//	04C	1	32	retn0		<-			<-			?			<-
//	04D	0	8	retn0		<-			<-			get unk0F4	<-
//	04E	0	8	retn0		<-			<-			get unk0F5	<-
//	04F	0	8	retn0		get unk084	<-			<-			<-
//	050	1	x	null		set unk084	<-			<-			<-
//	051	0	8	retn0		get unk01E	<-			<-			<-
//	052	1	x	null		set unk01E	<-			<-			<-
//	053	4	x	null		<-			<-			?			<-
//	054	1	x	null		set unk024	<-			<-			<-
//	055	2	x	null		<-			<-			<-			?
//	056	0	f	retn0		<-			<-			<-			get unk248
//	057	1	x	null		<-			<-			<-			add arg to unk248
//	058	1	x	null		set unk028	<-			<-			<-
//	059	0	f	retn0		get unk028	<-			<-			<-
//	05A	1	x	null		<-			<-			<-			?
//	05B	2	f	retn0		checks health and fatigue of arg1
//										<-			<-			<-
//	05C	0	32	retn0		<-			<-			get unk0C0	<-
//	05D	1	x	null		<-			<-			?			<-
//	05E	1	x	set unk004	<-			<-			?			<-
//	05F	0	32	get unk004	<-			<-			?			<-
//	060	0	ptr	get unk008	<-			<-			?			<-
//	061	2	x	increment unk004 by arg1
//							does base plus other stuff
//										<-			?			<-
//	062	1	32?	*arg0 = 0; return arg0?
//							<-			<-			?			<-
//	063	1	x	decrements reference count of argument
//							<-			<-			?			<-
//	064	1	x	null		<-			<-			?			<-
//	065	4	x	null		?			<-			?			?
//	066	4	x	null		?			<-			?			<-
//	067	1	x	null		?			<-			?			?
//	068	5	x	null		<-			<-			<-			?
//	069	1	x	null		<-			<-			<-			?
//	06A	1	x	null		<-			<-			<-			?
//	06B	1	x	null		<-			<-			?			<-
//	06C	1	8	retn1		<-			<-			?			<-
//	06D	0	32	retn0		<-			<-			<-			get unk288
//	06E	1	x	null		<-			<-			<-			?
//	06F	1	x	null		<-			<-			<-			set unk204
//	070	2	8	retn0		<-			<-			<-			?
//	071	1	32	retn0		<-			<-			<-			?
//	072	0	8	retn0		<-			<-			<-			get unk24C
//	073	1	x	null		<-			<-			<-			set unk24C
//	074	2	x	null		?			<-			<-			?
//	075	2	f	retn0		<-			<-			<-			?
//	076	0	32	retn0		<-			<-			<-			get unk254
//	077	1	x	null		<-			<-			<-			set unk254
//	078	2	x	null		<-			<-			<-			?
//	079	0	f	retn0		<-			<-			<-			get unk204
//	07A	0	x	null		<-			<-			<-			unk204 -= frameTime
//	07B	1	x	null		<-			<-			<-			set unk238
//	07C	0	f	retn20		<-			<-			<-			get unk238
//	07D	1	x	null		<-			<-			<-			<-
//	07E	0	8	retn0		<-			<-			<-			<-
//	07F	0	8	retn0		<-			<-			<-			get unk228
//	080	1	x	null		<-			<-			<-			set unk228
//	081	0	f	retn0		<-			<-			<-			get unk21C
//	082	1	x	null		<-			<-			<-			set unk21C
//	083	0	8	retn0		get unk1F	<-			<-			<-
//	084	1	x	null		set unk1F	<-			<-			<-
//	085	2	32	retn0		<-			<-			<-			?
//	086	0	x	null		<-			<-			<-			?
//	087	0	f	retn0		<-			<-			<-			get unk230
//	088	1	x	null		<-			<-			<-			set unk230
//	089	10	8	retn0		creates a new package
//										<-			?			?
//	08A	2	x	null		<-			<-			<-			?
//	08B	3	x	null		<-			<-			<-			?
//	08C	1	x	null		<-			<-			<-			set unk274
//	08D	2	x	null		<-			<-			<-			?
//	08E	1	x	null		<-			<-			<-			?
//	08F	3	8	retn0		<-			<-			<-			?
//	090	0	b	retn0		<-			<-			!!unkC0		<-
//	091	0	8	retn0		get unk20	<-			<-			<-
//	092	1	x	null		set unk20	<-			<-			<-
//	093	0	8	retn0		<-			<-			<-			get unk2B8
//	094	1	x	null		<-			<-			<-			set unk2B8
//	095	1	x	null		<-			<-			<-			set unk2B9
//	096	0	8	retn0		<-			<-			<-			get unk2B9
//	097	0	32	retn0		<-			<-			<-			get unk2B4
//	098	1	x	null		<-			<-			<-			set unk2B4
//	099	3	32	retn-1		does stuff with containers
//										?			<-			?
//	09A	3	f	retn0		does stuff with containers
//										?			<-			?
//	09B	3	x	null		?			<-			<-			?
//	09C	3	x	null		?			<-			<-			?
//	09D	3	x	null		<-			?			<-			?
//	09E	3	x	null		<-			?			<-			?
//	09F	3	x	null		<-			<-			<-			? same as A0
//	0A0	3	x	null		<-			<-			<-			? same as 9F
//	0A1	3	x	null		?			?			<-			?
//	0A2	3	x	null		?			?			<-			?
//	0A3	0	x	null		<-			<-			<-			unk294 = -1
//	0A4	0	x	null		<-			<-			<-			<-
//	0A5	0	32	retn0		<-			<-			get unk174	<-
//	0A6	0	x	null		<-			<-			<-			<-
//	0A7	3	x	null		<-			<-			?			<-
//	0A8	4	x	null		<-			<-			?			<-
//	0A9	0	32	retn0		<-			<-			get unk144	<-
//	0AA	1	x	null		<-			<-			set unk144	<-
//	0AB	0	32	retn0		<-			<-			get unk178	<-
//	0AC	1	x	null		<-			<-			set unk178	<-
//	0AD	0	8	retn0		<-			<-			get unk14C	<-
//	0AE	1	x	null		<-			<-			set unk14C	<-
//	0AF	0	16	retn0		<-			<-			<-			get unk1FC
//	0B0	2	x	null		<-			<-			<-			?
//	0B1	1	x	null		<-			<-			<-			?
//	0B2	4	x	null		<-			<-			<-			?
//	0B3	0	32	retn-1		<-			<-			<-			get unk1F4
//	0B4	0	32	retn0		<-			<-			<-			get unk1F8
//	0B5	2	x	null		<-			<-			<-			?
//	0B6	0	8	retn1		<-			<-			<-			?
//	0B7	1	8	retn0		<-			<-			<-			?
//	0B8	0	32	retn0		<-			<-			get unk11C	<-
//	0B9	1	x	null		<-			<-			set unk11C	<-	
//	0BA	1	x	null		<-			<-			<-			<-
//	0BB	5	x	null		<-			<-			?			<-
//	0BC	1	x	null		<-			<-			?			<-
//	0BD	1	x	null		<-			<-			?			<-
//	0BE	0	8	retn0		<-			<-			get unk114	<-
//	0BF	1	x	null		<-			<-			set unk114	<-
//	0C0	0	8	retn0		<-			<-			get unk115	<-
//	0C1	1	x	null		<-			<-			set unk115	<-
//	0C2	3	x	null		<-			<-			<-			set unk20C
//	0C3	0	32	?			<-			<-			<-			get &unk20C
//	0C4	0	x	null		<-			<-			<-			unk1A8 = 0
//	0C5	1	x	null		<-			<-			<-			?
//	0C6	1	x	null		<-			<-			set unk16C	<-
//	0C7	0	8	retn0		<-			<-			get unk16C	<-
//	0C8	0	f	retn0		<-			<-			get unk0F8	<-
//	0C9	1	x	null		<-			<-			set unk0F8	<-
//	0CA	1	x	null		?			<-			<-			<-
//	0CB	1	x	null		?			<-			<-			<-
//	0CC	0	32	retn0		<-			<-			<-			get unk218
//	0CD	1	x	null		<-			<-			<-			set unk218
//	0CE	1	32	retn0		<-			<-			<-			get unk220[arg]
//	0CF	2	x	null		<-			<-			<-			set unk220[arg]
//	0D0	1	x	null		<-			<-			<-			?
//	0D1	1	x	null		<-			<-			<-			? torch-related
//	0D2	0	8	retn0		<-			<-			<-			get unk278
//	0D3	0	x	null		<-			<-			<-			unk278 = 0
//	0D4	1	x	null		<-			<-			<-			?
//	0D5	0	x	null		<-			<-			<-			? sets unk2B0
//	0D6	0	f	retn0		<-			<-			<-			get unk2AC
//	0D7	0	f	retn0		<-			<-			<-			get unk22C
//	0D8	1	x	null		<-			<-			<-			set unk22C
//	0D9	1	8	retn0		<-			<-			<-			?
//	0DA	0	32	retn0		<-			<-			get unk11D	<-
//	0DB	4	x	null		<-			<-			?			<-
//	0DC	0	32	retn0		<-			<-			?			<-
//	0DD	0	32	retn0		<-			<-			get unk120	<-
//	0DE	0	32	retn0		<-			<-			get unk124	<-
//	0DF	0	32	retn0		<-			<-			get &unk128	<-
//	0E0	1	8	retn0		<-			<-			?			<-
//	0E1	0	8	retn0		<-			<-			<-			get unk25C
//	0E2	1	x	null		<-			<-			<-			set unk25C
//	0E3	0	8	retn0		get unk01C	<-			<-			<-
//	0E4	1	x	null		set unk01C	<-			<-			<-
//	0E5	0	8	retn0		<-			<-			get unk180	<-
//	0E6	1	x	null		<-			<-			set unk180	<-
//	0E7	2	8	retn0		?			<-			<-			<-
//	0E8	0	32	retn0		<-			<-			get unk184	<-
//	0E9	1	x	null		<-			<-			set unk184 with refcounting
//																<-
//	0EA	2	f	retn100		<-			<-			?			<-
//	0EB	1	32	retn0		<-			<-			<-			?
//	0EC	2	32	retn0		<-			<-			<-			<-
//	0ED	0	32	retn0		get unk030	<-			<-			<-
//	0EE	1	x	null		set unk030	<-			<-			<-
//	0EF	1	x	null		?			<-			?			<-
//	0F0	1	32	retn0		<-			<-			?			<-
//	0F1	1	x	null		<-			<-			set unk138	<-
//	0F2	1	x	null		<-			<-			set unk13C	<-
//	0F3	0	32	retn0		<-			<-			get unk13C	<-
//	0F4	1	x	null		<-			<-			set unk140	<-
//	0F5	0	32	retn0		<-			<-			get unk140	<-
//	0F6	6	8	retn1		?			<-			?			<-
//	0F7	1	x	null		<-			<-			<-			set unk260
//	0F8	0	f	retn0		<-			<-			<-			get unk260
//	0F9	3	x	null		<-			<-			?			<-
//	0FA	1	x	null		<-			<-			?			<-
//	0FB	?	32	GetSaveSize	*			*			*			*
//	0FC	?	?	SaveGame	*			*			*			*
//	0FD	?	?	LoadGame	*			*			*			*
//	0FE	3	?	?			?			<-			?			?
//	0FF	3	?	?			<-			<-			?			<-
//	100	2	?	?			?			?			?			?
//	101	?	?	purecall	?			<-			?			?		creates pathing object
//	102	0	32	purecall	get unk034	<-			<-			<-
//	103	0	32	purecall	stuff with unk034
//										<-			<-			<-
//	104	?	?	purecall	?			<-			<-			<-
//	105	?	?	purecall	?			<-			<-			<-
//	106	0	32	retn0		<-			<-			get unk150	<-
//	107	1	x	null		<-			<-			set unk150	<-
//	108	0	8	retn0		<-			<-			get unk160	<-
//	109	1	x	null		<-			<-			set unk160	<-
//	10A	4	x	null		<-			<-			?			<-
//	10B	0	f	retn1		<-			<-			get unk154	<-
//	10C	1	x	null		<-			<-			set unk154	<-
//	10D	0	f	retn0		<-			<-			get unk158	<-
//	10E	1	x	null		<-			<-			set unk158	<-
//	10F	1	x	null		<-			<-			<-			?
//	110	0	32	retn0		<-			<-			<-			get unk274
//	111	1	x	null		<-			<-			<-			unk274 += arg
//	112	2	8	retn0		<-			<-			?			<-
//	113	0	x	null		<-			<-			unk161 = 1	<-
//	114	2	32	retn0		<-			<-			<-			?
//	115	1	x	null		<-			<-			<-			set unk29C
//	116	0	32	retn0		<-			<-			<-			get unk29C
//	117	1	x	null		<-			<-			<-			set unk2A0
//	118	0	32	retn0		<-			<-			<-			get unk2A0
//	119	1	f	retn0		<-			?			<-			<-
//	11A	0	32	retn0		<-			<-			get unk188	<-
//	11B	1	x	null		<-			<-			set unk188	<-
//	11C	1	x	null		<-			<-			set unk16D	<-	
//	11D	0	8	retn0		<-			<-			get unk16D	<-
//	11E	0	32	retn0		<-			<-			<-			get unk2BC
//	11F	1	x	null		<-			<-			<-			?
//	120	1	x	null		<-			<-			<-			?
//	121	1	x	null		<-			<-			<-			?
//	122	1	x	null		<-			<-			<-			?
//	123	1	x	null		<-			<-			<-			?
//	124	2	x	null		<-			<-			<-			?
//	125	0	x	null		<-			<-			<-			?
//	126	0	x	null		<-			<-			<-			?
//	127	0	x	null		<-			<-			<-			?
//	128	0	x	null		<-			<-			<-			?
//	129	0	x	null		<-			<-			<-			?
//	12A	0	x	null		<-			<-			<-			unk2B0 -= frameTime
//	12B	0	f	retn0		<-			<-			<-			get unk2B0
//	12C	0	x	null		<-			<-			<-			?
//	12D	1	x	null		<-			<-			<-			?
//	12E	0	8	retn0		<-			<-			<-			?
//	12F	0	8	retn0		<-			<-			<-			?
//	130	0	8	retn0		<-			<-			<-			!unk2DC[4]
//	131	1	32	retn0		<-			<-			<-			get unk2C8[arg]
//	132	0	32	retn0		<-			<-			<-			?
//	133	0	32	retn0		<-			<-			<-			returns highest index set in unk2DC
//	134	0	str	retn ""		<-			<-			<-			returns default, action, script, combat, dialog, or none based on Fn133
//	135	1	x	null		<-			<-			<-			set unk2E8
//	136	0	8	retn0		<-			<-			<-			get unk2E8
//	137	0	32	retn0		<-			<-			<-			get unk2E4
//	138	1	8	retn0		<-			<-			<-			<-
//	139	1	x	null		<-			<-			set unk16B	<-
//	13A	0	8	retn0		<-			<-			get unk16B	<-
//	13B	0	f	retn0		get unk08C	<-			<-			<-
//	13C	1	x	null		set unk08C	<-			<-			<-
//	13D	1	x	null		<-			<-			?			<-
//	13E	1	x	null		<-			<-			?			<-
//	13F	1	x	null		<-			<-			?			<-
//	140	0	32	retn0		<-			<-			get unk170	<-

//	141	1+	?	x			?			<-			?			?
//	142	1	x	x			null		<-			?			?
//	143	1	x	x			null		<-			?			?
//	144	1	?	x			?			?			?			<-
//	145	1	?	x			?			<-			?			?
//	146	2?	?	x			?			<-			?			?
//	147	1	?	x			?			<-			<-			?
//	148	1	?	x			?			<-			?			?
//	149	1	x	x			null		<-			<-			?
//	14A	1	?	x			?			<-			<-			?
//	14B	1	b	x			?			<-			<-			?
//	14C	1	x	x			null		<-			?			<-
//	14D	1	?	x			?			<-			?			?
//	14E	1	?	x			calls Fn61	<-			?			<-
//	14F	1	?	x			calls Fn65	<-			<-			?
//	150	1	x	x			null		<-			<-			?
//	151	1	?	x			calls Fn61	<-			? same 153	?
//	152	1	?	x			calls Fn61	<-			?			?	
//	153	1	?	x			calls Fn61	<-			? same 151	<-
//	154	2	?	x			?			<-			?			<-
//	155	1	?	x			?			<-			?			?
//	156	1	?	x			?			<-			?			<-
//	157	1	b	x			?			<-			?			<-
//	158	1	b	x			?			<-			?			<-
//	159	1	?	x			?			<-			?			?

//	15A	2	8	x			x			?			?			<-

//	15B	1	x	x			x			x			null		set unk25D
//	15C	?	?	x			x			x			?			<-
//	15D	?	?	x			x			x			?			?
//	15E	?	?	x			x			x			? same 15F	? same 15F
//	15F	?	?	x			x			x			? same 15E	? same 15E
//	160	?	?	x			x			x			?			?
//	161	?	?	x			x			x			?			<-
//	162	?	?	x			x			x			?			?
//	163	?	?	x			x			x			?			<-
//	164	1	?	x			x			x			null		?

class Actor;
class TESObjectREFR;
class ActiveEffect;
class NiObject;
class BSBound;
class PathingData;

// size?
// This is used all over the game code to manage actors and occassionally other objects.
class ActorProcessManager
{
public:
	ActorProcessManager();
	~ActorProcessManager();

	struct ActorList {
		tList<Actor>			head;
		tList<Actor>::_Node		* tail;
	};

	ActorList				middleHighActors;		// 00
	ActorList				lowActors0C;			// 0C significantly smaller list than lowActors18. 
													//	  Plausible: actors currently scheduled for low processing
	ActorList				lowActors18;			// 18
	float					unk24;					// 24
	UInt32					unk28;					// 28
	void					* unk2C;				// 2C
	UInt32					unk30;					// 30
	void					* unk34;				// 34
	UInt32					unk38[4];				// 38
	tList<BSTempEffect>		tempEffects;			// 48
	UInt32					unk4C[4];				// 4C
	tList<Actor>			highActors;				// 5C
	Actor					* actor64;				// 64
	tList<Actor>::_Node		* unkNodes[3];			// 68 ##TODO: which lists do these belong to
	UInt32					unk74;					// 74 Possibly not a member. Definitely no more members following this.
};

extern ActorProcessManager * g_actorProcessManager;

// 14
class PathLow
{
public:
	PathLow();
	~PathLow();

	enum {
		kPathType_Low,
		kPathType_MiddleHigh,
		kPathType_High
	};

	virtual void	Destroy(bool bFreeMem) = 0;
	virtual UInt32	GetType(void) = 0;				// return one of kPathType_XXX
	virtual void	Unk_02(UInt32 arg0) = 0;
	virtual bool	Unk_03(void) = 0;
	virtual void	Unk_04(void) = 0;				// move to next path step?
	virtual void	Unk_05(Actor* actor, float* arg1, UInt32 arg2) = 0;	// arg1 looks like ptr to 3D vec. Build path?
	virtual bool	Unk_06(Actor* actor) = 0;		// update path?
	virtual void	Unk_07(UInt32 arg0, float arg1) = 0;
	virtual bool	Unk_08(Actor* actor) = 0;
	virtual void	Unk_09(Actor* actor, float* arg1) = 0;
	virtual void	Unk_0A(UInt32 arg0) = 0;
	virtual bool	GetPathingFailed(void) = 0;
	virtual void	SetPathingFailed(bool bSet) = 0;

//	void	** _vtbl;				// 00
	UInt32	unk04;					// 04
	UInt32	unk08;					// 08
	float	pathPointReachDistance;	// 0C
	UInt8	unk10;					// 10
	UInt8	pad11[3];				// 11
};

// 1C
class PathMiddleHigh : public PathLow
{
public:
	PathMiddleHigh();
	~PathMiddleHigh();

	UInt32	unk14;	// 14
	UInt32	unk18;	// 18
};

// 4C
class PathHigh : public PathMiddleHigh
{
public:
	PathHigh();
	~PathHigh();

	enum {
		kFlag_PathingFailed				= 1 << 3,
		kFlag_Turning					= 1 << 5,
	};

	// ###TODO: size?
	struct AvoidanceInfo {
		enum {
			kStatus_NONE,
			kStatus_CheckingObstacle,
			kStatus_AvoidingObstacle,
			kStatus_AnglingLeft,
			kStatus_AnglingRight,
			kStatus_Waiting,
			kStatus_ForcingNewPath,
			kStatus_FAILED
		};

		UInt32		unk00;
		UInt32		unk04;
		UInt32		unk08;
		UInt8		status;
		UInt8		unk0D[3];
		UInt32		unk10;
		float		unk14;
		float		waitTime;
		float		anglingTime;
		// ...?
	};


	float		unk1C;				// 1C
	float		unk20;				// 20
	UInt32		unk24;				// 24
	UInt32		unk28;				// 28
	UInt8		flags;				// 2C
	UInt8		unk2D[3];
	AvoidanceInfo	* avoidanceInfo;	// 30
	UInt32		unk34;				// 34
	UInt32		unk38;				// 38
	float		unk3C[3];			// 3C
	UInt32		unk48;				// 48
};

STATIC_ASSERT(sizeof(PathHigh) == 0x4C);

// 48
class PathBuilder : public BackgroundLoader
{
public:
	PathBuilder();
	~PathBuilder();

	typedef NiTPointerMap<PathingData>	ActorPathingMap;	// key is Actor*

	ActorPathingMap		actorPathingMap_10;		// 10	paths not started yet?
	ActorPathingMap		actorPathingMap_20;		// 20	paths in progress?
	ActorPathingMap		completedPaths;			// 30
	UInt32				unk40;					// 40
	UInt32				unk44;					// 44

	static PathBuilder * GetSingleton();
};

// 1C0
class CombatController : public TESPackage
{
public:
	CombatController();
	~CombatController();

	struct TargetInfo {
		Actor	* target;
		UInt32	unk04;		// looks like priority. seen values from 0-200
	};

	struct TargetList {
		TargetInfo	* info;
		TargetList	* next;
	};

	struct AvailableSpellExtraInfo {	// for item-based spells, e.g. potions and scrolls
		void		* unk00;
		UInt32		quantity;
		TESForm		* source;	// e.g. for EnchantmentItem on a scroll, a TESObjectBOOK*
		void		* unk0C;	// looks like pointer into owner's ExtraContainerChanges list for this item
	};

	struct AvailableSpellInfo {
		MagicItem				* item;
		AvailableSpellExtraInfo	* extraInfo;
	};

	struct AvailableSpellList {		// all the spells/magic items the actor has available. includes spells resolved from leveled lists
		AvailableSpellInfo	* info;
		AvailableSpellList	* next;
	};

	struct SelectedSpellInfo {
		MagicItem			* item;
		AvailableSpellExtraInfo	* availableInfo;	// for scrolls, potions, points to the entry to this (if any) in available spells list
													// is NULL for spells
	};

	struct AlliesList {
		Actor		* ally;
		AlliesList	* next;
	};

	Actor				* actor;					// 03C
	TargetList			* targets;					// 040
	float				unk044;						// 044
	UInt32				unk048;						// 048
	UInt32				unk04C;						// 04C
	UInt32				unk050;						// 050
	float				unk054;						// 054
	float				unk058;						// 058
	AvailableSpellList	* rangedSpells;				// 05C
	AvailableSpellList	* meleeSpells;				// 060
	AvailableSpellList	* restoreSpells;			// 064
	AvailableSpellList	* buffSpells;				// 068 anything that doesn't fit in the other 3 categories
	UInt32				unk06C[4];					// 06C
	SelectedSpellInfo	* selectedMeleeSpell;		// 07C
	SelectedSpellInfo	* selectedRangedSpell;		// 080
	SelectedSpellInfo	* selectedRestoreSpell;		// 084
	UInt32				unk088;						// 088
	UInt32				unk08C;						// 08C
	SelectedSpellInfo	* selectedBuffSpell;		// 090
	SelectedSpellInfo	* selectedBoundArmorSpell;	// 094
	SelectedSpellInfo	* selectedBoundWeaponSpell; // 098
	SelectedSpellInfo	* selectedSummonSpell;		// 09C
	UInt32				unk0A0[(0x15C-0x0A0) >> 2];	// 0A0
	AlliesList			allies;						// 15C
	UInt32				unk164[(0x1C0-0x164) >> 2];	// 164
};

#if OBLIVION
STATIC_ASSERT(sizeof(CombatController) == 0x1C0);
#endif

// 004+
class BaseProcess
{
public:
	enum
	{
		//	RefIDIdx	combatController;
	};

	enum
	{
		kMovementFlag_Sneaking =	0x00000400,	// overridden by kMovementFlag_Swimming
		kMovementFlag_Swimming =	0x00000800
	};

	BaseProcess();
	~BaseProcess();

	virtual void	Destructor(void);
	virtual void	Unk_01(void) = 0;
	virtual UInt32	GetProcessLevel(void) = 0;	// 0 - high, 3 - low
	virtual void	Unk_03(Actor* act, UInt32 arg1) = 0;
	virtual void	ManagePackProcedure(Actor* arg0) = 0;
	virtual void	Unk_05(UInt32 arg0, UInt32 arg1) = 0;
	virtual bool	Unk_06(UInt32 arg0, UInt32 arg1) = 0;
	virtual void	SetCurHour(float hour) = 0;
	virtual void	Unk_08(void) = 0;
	virtual bool	Unk_09(void) = 0;
	virtual float	GetCurHour(void) = 0;
	virtual UInt32	GetCurPackedDate(void) = 0;
	virtual void	SetCurPackedDate(UInt32 arg0) = 0;
	virtual UInt32	GetCurDay(void) = 0;
	virtual UInt32	GetCurMonth(void) = 0;
	virtual UInt32	GetCurYear(void) = 0;
	virtual void	Unk_10(UInt32 arg0) = 0;
	virtual UInt32	Unk_11(void) = 0;
	virtual void	Unk_12(UInt32 arg0) = 0;
	virtual void	Unk_13(void) = 0;
	virtual UInt32	Unk_14(void) = 0;
	virtual void	Unk_15(UInt32 arg0) = 0;
	virtual void	Unk_16(UInt32 arg0, UInt32 arg1) = 0;
	virtual void	Unk_17(void) = 0;
	virtual void	Unk_18(UInt32 arg0) = 0;
	virtual bool	Unk_19(void) = 0;
	virtual void	Unk_1A(UInt32 arg0) = 0;
	virtual bool	Unk_1B(UInt32 arg0, UInt32 arg1) = 0;
	virtual void	Unk_1C(UInt32 arg0, UInt32 arg1, UInt32 arg2) = 0;
	virtual UInt32	Unk_1D(UInt32 arg0) = 0;
	virtual UInt32	Unk_1E(UInt32 arg0) = 0;
	virtual float	Unk_1F(UInt32 arg0) = 0;
	virtual void	Unk_20(UInt32 arg0, UInt32 arg1) = 0;
	virtual void	Unk_21(UInt32 arg0, UInt32 arg1, UInt32 arg2) = 0;
	virtual bool	Unk_22(void) = 0;
	virtual void	Unk_23(UInt32 arg0) = 0;
	virtual void	Unk_24(UInt32 arg0) = 0;
	virtual void	Unk_25(void) = 0;
	virtual void	UpdateUnk088(void) = 0;
	virtual float	GetUnk088(void) = 0;
	virtual void	SetUnk088(float arg0) = 0;
	virtual void	Unk_29(void) = 0;

	// returns HighProcess::DetectionList::Data* associated with detectedActor. detectionState is one of kDetectionState_XXX enum
	// ###TODO: move some of this stuff out of HighProcess class def so we can forward declare it
	virtual void *  SetDetected(Actor* detectedActor, UInt8 detectionState, bool hasLOS, SInt32 detectionLevel) = 0;
	virtual bool	Unk_2B(void) = 0;
	virtual void	Unk_2C(UInt32 arg0) = 0;

	virtual void	Unk_2C_1_2(void) = 0;	// added in 1.2 patch

	virtual void	Unk_2D(UInt32 arg0) = 0;
	virtual void	Unk_2E(UInt32 arg0) = 0;
	virtual bool	Unk_2F(void) = 0;
	virtual void	Unk_30(UInt32 arg0) = 0;
	virtual bool	Unk_31(void) = 0;
	virtual TESObjectREFR *	GetUnk02C(void) = 0;
	virtual void	SetUnk02C(TESObjectREFR * obj) = 0;
	virtual UInt32	Unk_34(void) = 0;
	virtual void	Unk_35(UInt32 arg0) = 0;
	virtual void	Unk_36(UInt32 arg0) = 0;
	virtual UInt32	Unk_37(void) = 0;
	virtual void	Unk_38(UInt32 arg0) = 0;
	virtual UInt32	Unk_39(UInt32 arg0) = 0;
	virtual UInt32	Unk_3A(UInt32 arg0) = 0;
	virtual UInt32	Unk_3B(UInt32 arg0) = 0;
	virtual ExtraContainerChanges::EntryData* GetEquippedWeaponData(bool arg0) = 0;
	virtual UInt32	Unk_3D(UInt32 arg0) = 0;
	virtual ExtraContainerChanges::EntryData* GetEquippedAmmoData(bool arg0) = 0;
	virtual void	Unk_3F(UInt32 arg0) = 0;
	virtual bool	Unk_40(UInt32 arg0, UInt32 arg1) = 0;
	virtual bool	Unk_41(UInt32 arg0) = 0;
	virtual bool	Unk_42(UInt32 arg0) = 0;
	virtual bool	Unk_43(UInt32 arg0) = 0;
	virtual bool	Unk_44(UInt32 arg0) = 0;
	virtual UInt32	Unk_45(UInt32 arg0) = 0;
	virtual UInt32	Unk_46(UInt32 arg0) = 0;
	virtual UInt32	Unk_47(UInt32 arg0) = 0;
	virtual UInt32	Unk_48(UInt32 arg0) = 0;
	virtual UInt32	Unk_49(UInt32 arg0) = 0;
	virtual UInt32	Unk_4A(UInt32 arg0) = 0;
	virtual UInt32	Unk_4B(UInt32 arg0) = 0;
	virtual UInt32	Unk_4C(UInt32 arg0) = 0;
	virtual bool	Unk_4D(void) = 0;
	virtual bool	Unk_4E(void) = 0;
	virtual UInt8	GetUnk084(void) = 0;
	virtual void	SetUnk084(UInt8 arg0) = 0;
	virtual bool	GetUnk01E(void) = 0;
	virtual void	SetUnk01E(UInt32 arg0) = 0;
	virtual void	Unk_53(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3) = 0;
	virtual void	SetUnk024(UInt32 arg) = 0;
	virtual void	Unk_55(void) = 0;
	virtual void	Unk_56(void) = 0;
	virtual void	Unk_57(void) = 0;
	virtual void	GetUnk028(float arg) = 0;
	virtual float	SetUnk028(void) = 0;
	virtual void	Unk_5A(void) = 0;
	virtual void	Unk_5B(void) = 0;
	virtual void	Unk_5C(void) = 0;
	virtual void	SetCurrentPackage(TESPackage* package) = 0;
	virtual void	SetCurrentPackProcedure(TESPackage::eProcedure proceudre) = 0;  //Return eProceudre
	virtual TESPackage::eProcedure	GetCurrentPackProcedure(void) = 0;
	virtual TESPackage * GetCurrentPackage(void) = 0;	// returns MiddleHighProcess::pkg0C0 if not NULL, else BaseProcess::package
	virtual void	Unk_61(UInt32 arg0) = 0;			// marks ScriptEventList::kEvent_OnPackageDone
	virtual bhkCharacterProxy **	GetCharProxy(bhkCharacterProxy ** characterProxy) = 0;	// increfs and returns the proxy (or sets to NULL)
	virtual void	Unk_63(void * obj) = 0;
	virtual void	Unk_64(void) = 0;
	virtual void	Unk_65(void) = 0;
	virtual void	Unk_66(void) = 0;
	virtual void	Unk_67(void) = 0;
	virtual void	SayTopic(Character* speaker, TESTopic* topic, bool bForceSubtitles, UInt32 unk3, UInt32 unk4) = 0; // unks: 0, 0
	virtual void	Unk_69(void) = 0;	
	virtual void	Unk_6A(void) = 0;
	virtual void	Unk_6B(void) = 0;
	virtual void	Unk_6C(void) = 0;
	virtual void	Unk_6D(void) = 0;
	virtual void	Unk_6E(void) = 0;
	virtual void	Unk_6F(void) = 0;
	virtual void	Unk_70(void) = 0;
	virtual bool	GetLOS(UInt32 arg0, TESObjectREFR* target) = 0;	// arg0 unused
	virtual void	Unk_72(void) = 0;
	virtual void	Unk_73(void) = 0;
	virtual void	Unk_74(void) = 0;
	virtual void	Unk_75(void) = 0;
	virtual void	Unk_76(void) = 0;
	virtual void	Unk_77(void) = 0;
	virtual void	Unk_78(void) = 0;
	virtual void	Unk_79(void) = 0;
	virtual void	Unk_7A(void) = 0;
	virtual void	Unk_7B(void) = 0;
	virtual void	Unk_7C(void) = 0;
	virtual void	Unk_7D(void) = 0;
	virtual void	Unk_7E(void) = 0;
	virtual void	Unk_7F(void) = 0;
	virtual void	Unk_80(void) = 0;
	virtual void	Unk_81(void) = 0;
	virtual void	Unk_82(void) = 0;
	virtual UInt8	GetIsAlerted(void) = 0;
	virtual void	SetIsAlerted(UInt8 arg) = 0;
	virtual void	Unk_85(void) = 0;
	virtual void	Unk_86(void) = 0;
	virtual void	Unk_87(void) = 0;
	virtual void	Unk_88(void) = 0;
	virtual void	Unk_89(void) = 0;
	virtual void	Unk_8A(void) = 0;
	virtual void	Unk_8B(void) = 0;
	virtual void	Unk_8C(void) = 0;
	virtual void	Unk_8D(void) = 0;
	virtual void	Unk_8E(void) = 0;
	virtual void	Unk_8F(void) = 0;
	virtual void	Unk_90(void) = 0;
	virtual UInt8	GetUnk020(void) = 0;
	virtual void	SetUnk020(UInt8 arg) = 0;
	virtual void	Unk_93(void) = 0;
	virtual void	Unk_94(void) = 0;
	virtual void	Unk_95(void) = 0;
	virtual void	Unk_96(void) = 0;
	virtual void	Unk_97(void) = 0;
	virtual void	Unk_98(void) = 0;
	virtual void	Unk_99(void) = 0;
	virtual void	Unk_9A(void) = 0;
	virtual void	Unk_9B(void) = 0;
	virtual void	Unk_9C(void) = 0;
	virtual void	Unk_9D(void) = 0;
	virtual void	Unk_9E(void) = 0;
	virtual void	Unk_9F(void) = 0;
	virtual void	Unk_A0(void) = 0;
	virtual void	Unk_A1(void) = 0;
	virtual void	Unk_A2(void) = 0;
	virtual void	Unk_A3(void) = 0;
	virtual void	Unk_A4(void) = 0;
	virtual void	Unk_A5(void) = 0;
	virtual void	Unk_A6(void) = 0;
	virtual void	Unk_A7(void) = 0;
	virtual void	Unk_A8(void) = 0;
	virtual void	Unk_A9(void) = 0;
	virtual MagicItem	* GetQueuedMagicItem(void) = 0;	// returns MiddleHighProcess+144
	virtual void	Unk_AB(void) = 0;
	virtual void	Unk_AC(void) = 0;
	virtual void	Unk_AD(void) = 0;
	virtual void	Unk_AE(void) = 0;
	virtual UInt32	GetMovementFlags(void) = 0;
	virtual void	Unk_B0(void) = 0;
	virtual void	Unk_B1(void) = 0;
	virtual void	Unk_B2(void) = 0;
	virtual void	Unk_B3(void) = 0;
	virtual void	Unk_B4(void) = 0;
	virtual void	Unk_B5(void) = 0;

	// action is one of kAction_XXX. Returns action, return value probably unused.
	virtual UInt16	SetCurrentAction(UInt16 action, BSAnimGroupSequence* sequence) = 0;
	virtual void	Unk_B7(void) = 0;
/*	virtual void	Unk_B8(void) = 0;   //OR set this as GetKnockedState
	virtual UInt8	GetKnockedState(void) = 0; //And this as SetKnockedState(UInt8 knockedState) TODO investigate
	*/
	virtual void	GetKnockedState(void) = 0;  //JROush DB report this as HasFatigue/SetHasFatigue
	virtual UInt8	SetKnockedState(UInt8 knockedState)  = 0;
    
	virtual void	Unk_BA(void) = 0;
/*	virtual void	Unk_BB(void) = 0;

	// arg3 is a multiplier, arg4 appears to be base force to apply
	virtual void	KnockbackActor(Actor* target, float arg1, float arg2, float arg3, float arg4) = 0;
While OBSE and TESR define this order, VTBL analysis suggest that Unk_BB is actually KnockbackActor
TODO test and control. Seems correct for MiddleHighProcess
	*/
	virtual void	KnockbackActor(Actor* target, float arg1, float arg2, float arg3, float arg4) = 0;
	virtual void	Unk_BC(void) = 0;   //Seems to be actually be some stuff about skeleton nodes and animations. At least for MiddleHighProcess
	virtual void	Unk_BD(void) = 0;
	virtual UInt8	GetCombatMode(void) = 0;
	virtual UInt8	SetCombatMode(UInt8 mode) = 0;
	virtual UInt8	GetWeaponOut(void) = 0;
	virtual UInt8	SetWeaponOut(UInt8 out) = 0;
	virtual void	Unk_C2(void) = 0;
	virtual void *	Unk_C3(void) = 0;	// returns some pointer
	virtual void	Unk_C4(void) = 0;
	virtual void	Unk_C5(void) = 0;
	virtual void	Unk_C6(void) = 0;
	virtual void	Unk_C7(void) = 0;
	virtual void	Unk_C8(void) = 0;
	virtual void	Unk_C9(void) = 0;
	virtual void	Unk_CA(void) = 0;
	virtual void	Unk_CB(void) = 0;
	virtual void	Unk_CC(void) = 0;
	virtual void	Unk_CD(void) = 0;
	virtual void	Unk_CE(void) = 0;
	virtual void	Unk_CF(void) = 0;
	virtual void	Unk_D0(void) = 0;
	virtual void	Unk_D1(void) = 0;
	virtual void	Unk_D2(void) = 0;
	virtual void	Unk_D3(void) = 0;
	virtual void	Unk_D4(void) = 0;
	virtual void	Unk_D5(void) = 0;
	virtual void	Unk_D6(void) = 0;
	virtual void	Unk_D7(void) = 0;
	virtual void	Unk_D8(void) = 0;
	virtual void	Unk_D9(void) = 0;
	virtual void	Unk_DA(void) = 0;
	virtual UInt8	GetSitSleepState(void) = 0;
	virtual void	Unk_DC(void) = 0;
	virtual TESFurniture*	GetFurniture(void) = 0;
	virtual void	Unk_DE(void) = 0;
	virtual void	Unk_DF(void) = 0;
	virtual UInt32	GetUnk0D4(void) = 0;
	virtual void	Unk_E1(void) = 0;
	virtual void	Unk_E2(void) = 0;
	virtual UInt8	GetUnk01C(void) = 0;
	virtual void	SetUnk01C(UInt8 arg) = 0;
	virtual void	Unk_E5(void) = 0;
	virtual void	Unk_E6(void) = 0;
	virtual void	Unk_E7(void) = 0;
	virtual void	Unk_E8(void) = 0;
	virtual void	Unk_E9(void) = 0;
	virtual float	GetLightAmount(Actor * actor, UInt32 unk1) = 0;
	virtual void	Unk_EB(void) = 0;
	virtual void	* GetDetectionState(Actor* target) = 0;	// returns HighProcess::DetectionList::Data *
	virtual void	Unk_ED(void) = 0;
	virtual void	Unk_EE(void) = 0;
	virtual void	Unk_EF(void) = 0;
	virtual void	Unk_F0(void) = 0;
	virtual void	Unk_F1(void) = 0;
	virtual void	Unk_F2(void) = 0;
	virtual void	Unk_F3(void) = 0;
	virtual void	Unk_F4(void) = 0;
	virtual void	Unk_F5(void) = 0;
	virtual void	Unk_F6(void) = 0;
	virtual void	Unk_F7(void) = 0;
	virtual void	Unk_F8(void) = 0;
	virtual void	Unk_F9(void) = 0;
	virtual void	Unk_FA(void) = 0;
	virtual void	Unk_FB(void) = 0;
	virtual void	Unk_FC(void) = 0;
	virtual void	Unk_FD(void) = 0;
	virtual void	Unk_FE(void) = 0;
	virtual void	Unk_FF(void) = 0;
	virtual void	Unk_100(void) = 0;
	virtual void	Unk_101(void) = 0;
	virtual PathLow * CreatePath(void) = 0;
	virtual PathLow	* GetCurrentPath(void) = 0;
	virtual void	Unk_104(void) = 0;
	virtual void	Unk_105(void) = 0;
	virtual void	Unk_106(void) = 0;
	virtual void	Unk_107(void) = 0;
	virtual void	Unk_108(void) = 0;
	virtual void	Unk_109(void) = 0;
	virtual void	Unk_10A(void) = 0;
	virtual void	Unk_10B(void) = 0;
	virtual void	Unk_10C(void) = 0;
	virtual void	Unk_10D(void) = 0;
	virtual void	Unk_10E(void) = 0;
	virtual void	Unk_10F(void) = 0;
	virtual void	Unk_110(void) = 0;
	virtual void	Unk_111(void) = 0;
	virtual void	Unk_112(void) = 0;
	virtual void	Unk_113(void) = 0;
	virtual void	Unk_114(void) = 0;
	virtual void	Unk_115(void) = 0;
	virtual void	Unk_116(void) = 0;
	virtual void	Unk_117(void) = 0;
	virtual void	Unk_118(void) = 0;
	virtual void	Unk_119(void) = 0;
	virtual void	Unk_11A(void) = 0;
	virtual void	Unk_11B(void) = 0;
	virtual void	Unk_11C(void) = 0;
	virtual void	Unk_11D(void) = 0;
	virtual void	Unk_11E(void) = 0;
	virtual void	Unk_11F(void) = 0;
	virtual void	Unk_120(void) = 0;
	virtual void	Unk_121(void) = 0;
	virtual void	Unk_122(void) = 0;
	virtual void	Unk_123(void) = 0;
	virtual void	Unk_124(void) = 0;
	virtual void	Unk_125(void) = 0;
	virtual void	Unk_126(void) = 0;
	virtual void	Unk_127(void) = 0;
	virtual void	Unk_128(void) = 0;
	virtual void	Unk_129(void) = 0;
	virtual void	Unk_12A(void) = 0;
	virtual void	Unk_12B(void) = 0;
	virtual void	Unk_12C(void) = 0;
	virtual void	Unk_12D(void) = 0;
	virtual void	Unk_12E(void) = 0;
	virtual void	Unk_12F(void) = 0;
	virtual void	Unk_130(void) = 0;
	virtual void	Unk_131(void) = 0;

	// action type is one of HighProcess::kActionType_XXX
	virtual TESObjectREFR* GetActionTarget(UInt32 actionType) = 0;	
	virtual void	Unk_133(void) = 0;
	virtual const char *	Unk_134(void) = 0;
	virtual void	Unk_135(void) = 0;
	virtual void	Unk_136(void) = 0;
	virtual void	Unk_137(void) = 0;
	virtual void	Unk_138(void) = 0;
	virtual void	Unk_139(void) = 0;
	virtual void	Unk_13A(void) = 0;
	virtual float	GetUnk08C(void) = 0;
	virtual void	SetUnk08C(float arg) = 0;
	virtual void	Unk_13D(void) = 0;
	virtual void	Unk_13E(void) = 0;
	virtual void	Unk_13F(void) = 0;
	virtual void	Unk_140(void) = 0;

//	void					** _vtbl;				// 000
	TESPackage::eProcedure	editorPackProcedure;	// 004
	TESPackage				* editorPackage;		// 008
};

// 90
class LowProcess : public BaseProcess
{
public:
	enum
	{
		//	BaseProcess flags

		//	
	};

	LowProcess();
	~LowProcess();

	virtual void	Unk_141(void) = 0;
	virtual void	Unk_142(void) = 0;
	virtual void	Unk_143(void) = 0;
	virtual void	Unk_144(void) = 0;
	virtual void	Alarm(Actor* act) = 0;
	virtual void	Unk_146(void) = 0;
	virtual void	Unk_147(void) = 0;
	virtual void	Unk_148(void) = 0;
	virtual void	Unk_149(void) = 0;
	virtual void	Unk_14A(void) = 0;
	virtual void	Unk_14B(void) = 0;
	virtual void	Unk_14C(void) = 0;
	virtual void	Unk_14D(void) = 0;
	virtual void	Unk_14E(void) = 0;
	virtual void	Unk_14F(void) = 0;
	virtual void	Unk_150(void) = 0;
	virtual void	Unk_151(void) = 0;
	virtual void	Unk_152(void) = 0;
	virtual void	Unk_153(void) = 0;
	virtual void	Unk_154(void) = 0;
	virtual void	Unk_155(void) = 0;
	virtual void	Unk_156(void) = 0;
	virtual UInt8	MountHorse(Actor* act) = 0;
	virtual UInt8	DismoutHorse(Actor* act) = 0;
	virtual void	Unk_159(void) = 0;

	float	unk00C;				// 00C - initialized to -1
	float	unk010;				// 010
	float	curHour;			// 014 - initialized to -1
	UInt32	curPackedDate;		// 018 - (year << 13) | (month << 9) | day
	UInt8	unk01C;				// 01C
	UInt8	unk01D;				// 01D
	UInt8	unk01E;				// 01E
	UInt8	isAlerted;				// 01F
	UInt8	unk020;				// 020
	UInt8	pad021[3];			// 021
	TESForm * usedItem;			// 024 for idles like reading book, making potions, etc
	float	unk028;				// 028
	Actor*  follow;				// 02C
	TESObjectREFR   * unk030;	// 030 seen XMarkerHeading refs here
	PathLow	* pathing;			// 034
	UInt32	unk038;				// 038
	UInt32	unk03C;				// 03C
	UInt32	unk040;				// 040
	UInt32	unk044;				// 044
	UInt32	unk048;				// 048
	UInt32	unk04C;				// 04C
	UInt32	unk050;				// 050
	UInt32	unk054;				// 054
	UInt32	unk058;				// 058
	UInt32	unk05C;				// 05C
	UInt32	unk060;				// 060
	UInt32	unk064;				// 064
	UInt32	unk068;				// 068
	UInt32	unk06C;				// 06C
	ActorValues	avDamageModifiers;	// 070
	UInt8	unk084;				// 084
	UInt8	pad085[3];			// 085
	float	unk088;				// 088 - counter in seconds
	float	unk08C;				// 08C
};

STATIC_ASSERT(sizeof(LowProcess) == 0x90);

// A8
class MiddleLowProcess : public LowProcess
{
public:
	MiddleLowProcess();
	~MiddleLowProcess();

	virtual void	Unk_15A(void) = 0;

	UInt32			unk090;				// 090
	ActorValues		maxAVModifiers;		// 094
};

STATIC_ASSERT(sizeof(MiddleLowProcess) == 0xA8);

class bhkCharacterController;

// DC
class ActorAnimData
{
public:
	ActorAnimData();
	~ActorAnimData();

	UInt32				unk00;			//00
	NiNode				* niNode04;		//04 seen BSFadeNode for 3rd Person, NiNode for 1st
	NiNode				* niNode08;		//08
	UInt32				unk0C[6];		//0C
	NiNode				* niNodes24[5];	//24
	UInt32				unk38[24];		//38
	NiControllerManager	* manager;		//98
	NiTPointerMap<UInt32> * map9C;		//9C NiTPointerMap<AnimSequenceBase>
	BSAnimGroupSequence	* animSequences[5]; //A0
	UInt32				unkB4;			//B4
	UInt32				unkB8;			//B8
	float				unkBC;			//BC
	float				unkC0;			//C0
	float				unkC4;			//C4
	UInt32				unkC8[3];		//C8
	UInt32				unkD4;			//D4
	void				* unkD8;		//D8 looks like struct with idle anim transform data

	bool FindAnimInRange(UInt32 lowBound, UInt32 highBound = -1);
	bool PathsInclude(const char* subString);
};

// 18C
class MiddleHighProcess : public MiddleLowProcess
{
public:
	MiddleHighProcess();
	~MiddleHighProcess();

	virtual void	Unk_15B(void) = 0;
	virtual void	Unk_15C(void) = 0;
	virtual void	Unk_15D(void) = 0;
	virtual void	Unk_15E(void) = 0;
	virtual void	Unk_15F(void) = 0;
	virtual void	Dialogue(Actor* act) = 0;
	virtual void	RemoveWornItems(Actor* act, UInt8 arg2, UInt32 arg3) = 0;
	virtual void	Travel(Actor* act, UInt8 arg2, float arg3, int arg4) = 0;
	virtual void	Unk_163(void) = 0;
	virtual void	Unk_164(void) = 0;

	// 10
	struct Unk128
	{
		UInt32	unk0;	// 0
		UInt32	unk4;	// 4
		UInt32	unk8;	// 8
		UInt16	unkC;	// C
		UInt8	unkE;	// E
		UInt8	padF;	// F
	};

	// 8
	struct EffectListNode
	{
		ActiveEffect	* effect;	// 0
		EffectListNode	* next;		// 4
	};

	UInt32				unk0A8;		// 0A8
	UInt32				unk0AC;		// 0AC
	UInt32				unk0B0;		// 0B0
	UInt32				unk0B4;		// 0B4
	UInt32				unk0B8;		// 0B8
	float				unk0BC;		// 0BC
	TESPackage			* currentPackage;	// 0C0 if null, editorPackage applies
	UInt32				unk0C4;		// 0C4
	UInt8				unk0C8;		// 0C8
	UInt8				pad0C9[3];	// 0C9
	TESPackage::eProcedure	currentPackProcedure;		// 0CC
	UInt8				unk0D0;		// 0D0
	UInt8				pad0D0[15];	// 0D1 - never initialized
	UInt32				unk0E0;		// 0E0
	ExtraContainerChanges::EntryData* equippedWeaponData;		// 0E4
	ExtraContainerChanges::EntryData* equippedLightData;		// 0E8
	ExtraContainerChanges::EntryData* equippedAmmoData;			// 0EC
	ExtraContainerChanges::EntryData* equippedShieldData;		// 0F0
	UInt8				unk0F4;		// 0F4
	UInt8				unk0F5;		// 0F5
	UInt8				pad0F6[2];	// 0F6
	float				unk0F8;		// 0F8
	UInt32				unk0FC;		// 0FC
	UInt32				unk100;		// 100
	UInt32				unk104;		// 104
	UInt32				unk108;		// 108
	UInt32				unk10C;		// 10C
	UInt32				unk110;		// 110
	UInt8				unk114;		// 114
	UInt8				unk115;		// 115
	UInt8				pad116[2];	// 116
	bhkCharacterProxy	* charProxy;	// 118 - seen bhkCharacterProxy
	SInt8				knockedState;	// 11C
	UInt8				sleepState;		// 11D
	UInt8				pad11E;		// 11E
	UInt8				pad11F;		// 11F
	TESObjectREFR		* furniture;	// 120 Furniture ref NPC is sitting on (may be used for other things)
	UInt8				unk124;		// 124 - init'd to 0x7F
	Unk128				unk128;		// 128
	UInt16				unk138;		// 138
	UInt8				pad13A[2];	// 13A
	UInt32				unk13C;		// 13C
	UInt32				unk140;		// 140
	MagicItem			* queuedMagicItem;	// 144 set before calling sub_69AF30 after Addspell cmd, unset upon return
	UInt32				unk148;		// 148
	UInt8				unk14C;		// 14C looks like true if casting, or possibly a casting state
	UInt8				pad14D[3];	// 14D
	UInt32				unk150;		// 150
	float				actorAlpha;	// 154 valid values 0-1
	float				unk158;		// 158
	NiExtraData			* unk15C;	// 15C seen BSFaceGenAnimationData*, reset when modifying character face
	UInt8				unk160;		// 160
	UInt8				unk161;		// 161
	UInt8				pad162[2];	// 162
	UInt32				unk164;		// 164
	UInt8				unk168;		// 168
	UInt8				unk169;		// 169
	UInt8				unk16A;		// 16A
	UInt8				unk16B;		// 16B
	UInt8				unk16C;		// 16C
	UInt8				unk16D;		// 16D
	UInt8				pad16E[2];	// 16E
	UInt32				unk170;		// 170
	EffectListNode		* effectList;	// 174
	UInt32				unk178;		// 178
	ActorAnimData		* animData;	// 17C
	UInt8				unk180;		// 180
	UInt8				pad181[3];	// 181
	NiObject			* unk184;	// 184 - seen BSShaderPPLightingProperty
	BSBound				* boundingBox;	// 188

	bhkCharacterController* GetCharacterController();
};

STATIC_ASSERT(sizeof(MiddleHighProcess) == 0x18C);

// 2EC
class HighProcess : public MiddleHighProcess
{
public:
	HighProcess();
	~HighProcess();

	enum
	{
		kActionType_Default = 0,
		kActionType_Action,
		kActionType_Script,
		kActionType_Combat,
		kActionType_Dialog,

		kActionType_Max
	};

	enum {
		kDetectionState_Lost = 0,
		kDetectionState_Unseen,
		kDetectionState_Noticed,
		kDetectionState_Seen,

		kDetectionState_Max
	};

	// 8
	struct DetectionList
	{
		struct Data
		{
			Actor			* actor;
			UInt8			detectionState;
			UInt8			pad04[3];
			UInt8			hasLOS;
			UInt8			pad08[3];
			SInt32			detectionLevel;
		};

		Data			* data;
		DetectionList	* next;

		Data* Info() const { return data; }
		DetectionList* Next() const { return next; }
	};

	typedef Visitor<DetectionList, DetectionList::Data> DetectionListVisitor;

	// this appears to be a common linked list class
	// 4
	struct Node190
	{
		void	* data;
		Node190	* next;
	};

	// 10
	struct Unk1BC
	{
		UInt32	unk0;
		UInt32	unk4;
		UInt32	unk8;
		UInt32	unkC;
	};

	// C
	struct Unk20C
	{
		UInt32	unk0;
		UInt32	unk4;
		UInt32	unk8;
	};

	// C
	struct Unk27C
	{
		UInt32	unk0;
		UInt32	unk4;
		UInt32	unk8;
	};

	enum{
		kAction_None			= -1,
		kAction_EquipWeapon,
		kAction_UnequipWeapon,
		kAction_Attack,
		kAction_AttackFollowThrough,
		kAction_AttackBow,
		kAction_AttackBowArrowAttached,
		kAction_Block,
		kAction_Recoil,
		kAction_Stagger,
		kAction_Dodge,
		kAction_LowerBodyAnim,
		kAction_SpecialIdle,
		kAction_ScriptAnimation,
	};

	enum
	{
		kMovement_Forward =		0x0001,
		kMovement_Backward =	0x0002,
		kMovement_Left =		0x0004,
		kMovement_Right =		0x0008,
		kMovement_TurnLeft =	0x0010,
		kMovement_TurnRight =	0x0020,

		kMovement_Walk =		0x0100,
		kMovement_Run =			0x0200,
		kMovement_Sneak =		0x0400,	// overridden by kMovementFlag_Swimming
		kMovement_Swim =		0x0800,
		kMovement_Jump =		0x1000, //Jump and above appear not to be used.
		kMovement_Fly =			0x2000,
		kMovement_Fall =		0x4000,
		kMovement_Slide =		0x8000,
	};

	DetectionList	* detectionList;	// 18C
	Node190	unk190;		// 190
	UInt32	unk198;		// 198
	float	unk19C;		// 19C - idle chatter comment timer
	UInt8	unk1A0;		// 1A0
	UInt8	pad1A1[3];	// 1A1
	UInt32	unk1A4;		// 1A4
	UInt32	unk1A8;		// 1A8
	UInt32	unk1AC;		// 1AC
	UInt32	unk1B0;		// 1B0 - uninitialized
	UInt32	unk1B4;		// 1B4 - uninitialized
	UInt32	unk1B8;		// 1B8
	Unk1BC	unk1BC;		// 1BC
	UInt32	unk1CC;		// 1CC - uninitialized
	UInt8	unk1D0;		// 1D0
	UInt8	unk1D1;		// 1D1
	UInt32	unk1D4;		// 1D4
	UInt32	unk1D8;		// 1D8
	UInt32	unk1DC;		// 1DC
	UInt32	unk1E0;		// 1E0
	UInt8	unk1E4;		// 1E4
	UInt8	pad1E5[3];	// 1E5
	UInt32	unk1E8;		// 1E8
	UInt32	unk1EC;		// 1EC
	UInt32	unk1F0;		// 1F0
	SInt16	currentAction;	// 1F4 - related to 1F8. returned by vtbl +2D0
	UInt8	pad1F6[2];	// 1F6
	UInt32	unk1F8;		// 1F8 - related to 1F4. Sometimes a BSAnimGroupSequence*
	UInt16	movementFlags;	// 1FC - returned by vtbl + 2C0
	UInt8	pad1FE[2];	// 1FE
	UInt32	unk200;		// 200
	float	unk204;		// 204
	UInt16	unk208;		// 208
	UInt8	pad20A[2];	// 20A
	Unk20C	unk20C;		// 20C
	UInt32	unk218;		// 218
	float	unk21C;		// 21C
	UInt32	unk220[2];	// 220
	UInt8	unk228;		// 228
	UInt8	pad229[3];	// 229
	float	unk22C;		// 22C
	float	unk230;		// 230 - initialized to ((rand() % 5000) * .001) + 10
	UInt32	unk234;		// 234 - not initialized
	float	swimBreath;		// 238 - initialized to 0x41A00000
	UInt8	unk23C;		// 23C
	UInt8	unk23D[3];	// 23D
	UInt32	unk240;		// 240
	UInt8	unk244;		// 244
	UInt8	unk245[3];	// 245
	float	unk248;		// 248
	UInt8	unk24C;		// 24C
	UInt8	pad24D[3];	// 24D
	UInt32	unk250;		// 250
	UInt32	unk254;		// 254
	UInt32	unk258;		// 258
	UInt8	unk25C;		// 25C
	UInt8	unk25D;		// 25D
	UInt8	pad25E[2];	// 25E
	float	unk260;		// 260
	UInt32	unk264;		// 264
	NiObject	* unk268;	// 268 - decref'able pointer
	float	unk26C;		// 26C
	UInt32	unk270;		// 270
	UInt32	unk274;		// 274
	UInt8	unk278;		// 278
	UInt8	pad279[3];	// 279
	Unk27C	unk27C;		// 27C
	UInt32	unk288;		// 288
	UInt32	unk28C;		// 28C
	UInt8	unk290;		// 290
	UInt8	pad291[3];	// 291
	float	unk294;		// 294
	UInt32	unk298;		// 298
	UInt32	unk29C;		// 29C
	UInt32	unk2A0;		// 2A0
	UInt32	unk2A4;		// 2A4
	UInt8	unk2A8;		// 2A8
	UInt8	unk2A9;		// 2A9
	UInt8	pad2AA[2];	// 2AA
	float	unk2AC;		// 2AC
	float	unk2B0;		// 2B0
	UInt32	unk2B4;		// 2B4
	UInt8	unk2B8;		// 2B8
	UInt8	unk2B9;		// 2B9
	UInt8	pad2BA[2];	// 2BA
	UInt32	unk2BC;		// 2BC
	UInt32	unk2C0;		// 2C0
	UInt32	unk2C4;		// 2C4
	TESObjectREFR	* actionTarget[kActionType_Max];	// 2C8
	UInt8			actionActive[kActionType_Max];		// 2DC - true if the specified action is running
	UInt8	pad2E1[3];	// 2E1
	TESObjectREFR	* unk2E4;	// 2E4
	UInt8	unk2E8;		// 2E8
	UInt8	pad2E9[3];	// 2E9

	bool IsAttacking()
		{	return currentAction >= kAction_Attack && currentAction <= kAction_AttackBowArrowAttached;	}
	bool IsBlocking()
		{	return currentAction == kAction_Block;	}
	bool IsRecoiling()
		{	return currentAction == kAction_Recoil;	}
	bool IsStaggered()
		{	return currentAction == kAction_Stagger;	}
	bool IsDodging()
		{	return currentAction == kAction_Dodge;	}
	bool IsMovementFlagSet(UInt32 flag)
		{	return (movementFlags & flag) == flag;	}
};

STATIC_ASSERT(sizeof(HighProcess) == 0x2EC);

// 14
class AStarWorldNode
{
public:
	AStarWorldNode();
	~AStarWorldNode();

	UInt32			id;					// 00
	TESObjectREFR	* fromDoorRef;		// 04
	TESForm			* from;				// 08 TESObjectCELL, TESWorldSpace, TESSubspace refr
	TESObjectREFR	* toDoorRef;		// 0C
	TESForm			* to;				// 10 as with from

	typedef tList<AStarWorldNode>	List;
};

// A "low" path is one that traverses a graph where each node is a cell, worldspace, or subspace, with edges defined by load doors
// Used whenever an actor's destination is outside of its current cell/space, regardless of actor's process level
typedef NiTPointerMap <AStarWorldNode::List>	AStarCellNodeMap;		// key is TESForm* (cell, worldspace, subspace)
typedef NiTPointerMap <AStarCellNodeMap>		LowPathWorld;			// key is TESForm* (cell, worldspace, subspace)

typedef NiTListBase<AStarWorldNode>	AStarWorldNodeList;
