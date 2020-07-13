#pragma once

#include "obse/NiNodes.h"

/*** extra data
 *	name								unk13	unk14
 *	NiExtraData							1		1
 *		TESObjectExtraData				1		1
 *		BSFaceGenAnimationData			1		1
 *		BSFaceGenModelExtraData			1		1
 *		BSFaceGenBaseMorphExtraData		1		1
 *		DebugTextExtraData				1		1
 *		NiStringExtraData				1		1
 *		NiFloatExtraData				1		1
 *			FadeNodeMaxAlphaExtraData	1		1
 *		BSFurnitureMarker				1		1
 *		NiBinaryExtraData				1		1
 *		BSBound							1		1
 *		NiSCMExtraData					0		0
 *		NiTextKeyExtraData				1		1
 *		NiVertWeightsExtraData			1		1
 *		bhkExtraData					1		1
 *		PArrayPoint - does not exist
 *		NiIntegerExtraData				1		1
 *			BSXFlags					1		1
 *		NiFloatsExtraData				1		1
 *		NiColorExtraData				1		1
 *		NiVectorExtraData				1		1
 *		NiSwitchStringExtraData			1		1
 *		NiStringsExtraData				1		1
 *		NiIntegersExtraData				1		1
 *		NiBooleanExtraData				1		1
 */

class TESObjectREFR;

// 0C
class NiExtraData : public NiObject
{
public:
	NiExtraData();
	~NiExtraData();

	virtual	bool	Unk_13(void);
	virtual bool	Unk_14(void);

	char	* m_pcName;	// 08
};

// 10
class TESObjectExtraData : public NiExtraData
{
public:
	TESObjectExtraData();
	~TESObjectExtraData();

	TESObjectREFR	* refr;	// 0C
};

// ? - these classes do not belong in this file
class BSFaceGenKeyframe
{
public:
	BSFaceGenKeyframe();
	~BSFaceGenKeyframe();

	virtual void	Destroy(bool free);
	virtual UInt32	GetUnk04(void);
	virtual bool	SetUnk04(UInt32 arg);	// returns if the value changed
	virtual float	GetUnk08(void);
	virtual bool	SetUnk08(float arg);	// returns if the value changed
	virtual bool	Unk_05(void * arg);
	virtual bool	Unk_06(void * arg0, void * arg1, float arg2);
	virtual	bool	Unk_07(void * arg0, float arg1, UInt8 arg2, UInt32 arg3);	// args wrong
	virtual bool	Unk_08(void);	// args wrong
	virtual BSFaceGenKeyframe *	Unk_09(void);	// clone?
	virtual void	Unk_0A(void);	// args wrong
	virtual bool	Unk_0B(void * arg);
	virtual float	Unk_0C(void);
	virtual bool	Unk_0D(void);
	virtual void	Reset(void);	// clear unk0C table
	virtual bool	Unk_0F(float arg0, void * arg1);
	virtual bool	Unk_10(void);
	virtual bool	Unk_11(void);
	virtual float	GetWeight(UInt32 idx);
	virtual bool	Unk_13(UInt32 idx, float arg);
	virtual UInt32	GetNumWeights(void);
	virtual bool	Unk_15(UInt32 idx);
	virtual void	Resize(UInt32 count, bool mode);

//	void	** vtbl;	// 00
	UInt32	unk04;		// 04
	float	unk08;		// 08
};

// 14
class BSFaceGenKeyframeMultiple : public BSFaceGenKeyframe
{
public:
	BSFaceGenKeyframeMultiple();
	~BSFaceGenKeyframeMultiple();

	float	* weight;	// 0C
	UInt32	numWeights;	// 10
};

// 1E0
class BSFaceGenAnimationData : public NiExtraData
{
public:
	BSFaceGenAnimationData();
	~BSFaceGenAnimationData();

	virtual float	Unk_15(UInt32 arg);	// max = 0x0C
	virtual void	Unk_16(UInt32 arg0, float arg1);	// arg0 max = 0x0C
	virtual float	Unk_17(UInt32 arg);	// max = 0x10
	virtual bool	Unk_18(NiMatrix33 * arg);
	virtual void	Unk_19(UInt32 arg0, float arg1);	// arg0 max = 0x10
	virtual float	Unk_1A(UInt32 arg);	// max = 0x0F
	virtual void	Unk_1B(UInt32 arg0, float arg1);	// arg0 max = 0x0F
	virtual float	Unk_1C(UInt32 arg);	// max = 0
	virtual void	Unk_1D(UInt32 arg0, float arg1);	// arg0 max = 0
	virtual void	Unk_1E(float arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5);
	virtual void	Unk_1F(void);
	virtual void	Unk_20(void);
	virtual void	Unk_21(void);
	virtual void	Unk_22(void);
	virtual UInt8	Unk_23(void);	// return unk1D7
	virtual UInt8	Unk_24(void);	// return unk1D8
	virtual void	Unk_25(UInt8 arg);	// unk1D8 = arg
	virtual UInt8	Unk_26(void);	// return unk1DA
	virtual void	Unk_27(void);
	virtual void	Unk_28(void);
	virtual void	Unk_29(void);
	virtual void	Unk_2A(void);
	virtual void	Unk_2B(void);
	virtual void	Unk_2C(void);
	virtual void	Unk_2D(void);
	virtual void	Unk_2E(void);
	virtual bool	Unk_2F(UInt32 arg);	// 0 = unk0C4, 2 = unk068, 3 = unk120
	virtual void	Unk_30(UInt32 arg);	// 0 = unk0E8, 1 = unk030, 2 = unk08C, 3 = unk144
	virtual void	Unk_31(void);
	virtual void	Unk_32(void);
	virtual void	Unk_33(void);
	virtual void	Unk_34(void);
	virtual void	Unk_35(void);
	virtual void	Unk_36(void);
	virtual void	Unk_37(void);

	typedef NiTPointerList <BSFaceGenKeyframe *>	KeyframeList;

	UInt32	unk00C;	// 00C
	BSFaceGenKeyframeMultiple	unk010;	// 010
	KeyframeList				unk024;	// 024
	BSFaceGenKeyframeMultiple	unk034;	// 034
	BSFaceGenKeyframeMultiple	unk048;	// 048
	KeyframeList				unk05C;	// 05C
	BSFaceGenKeyframeMultiple	unk06C;	// 06C
	KeyframeList				unk080;	// 080
	BSFaceGenKeyframeMultiple	unk090;	// 090
	BSFaceGenKeyframeMultiple	unk0A4;	// 0A4
	KeyframeList				unk0B8;	// 0B8
	BSFaceGenKeyframeMultiple	unk0C8;	// 0C8
	KeyframeList				unk0DC;	// 0DC
	BSFaceGenKeyframeMultiple	unk0EC;	// 0EC
	BSFaceGenKeyframeMultiple	unk100;	// 100
	KeyframeList				unk114;	// 114
	BSFaceGenKeyframeMultiple	unk124;	// 124
	KeyframeList				unk138;	// 138
	BSFaceGenKeyframeMultiple	unk148;	// 148
	BSFaceGenKeyframeMultiple	unk15C;	// 15C
	UInt32	unk170;		// 170
	UInt32	unk174;		// 174
	UInt32	unk178;		// 178
	UInt32	unk17C;		// 17C
	float	unk180;		// 180
	float	unk184;		// 184
	float	unk188;		// 188
	float	unk18C;		// 18C
	float	unk190;		// 190
	UInt32	unk194;		// 194
	UInt8	unk198;		// 198
	UInt8	pad199[3];	// 199
	float	unk19C;		// 19C
	float	unk1A0;		// 1A0
	float	unk1A4;		// 1A4
	float	unk1A8;		// 1A8
	float	unk1AC;		// 1AC
	float	unk1B0;		// 1B0
	float	unk1B4;		// 1B4
	float	unk1B8;		// 1B8
	float	unk1BC;		// 1BC
	UInt32	unk1C0;		// 1C0
	float	unk1C4;		// 1C4
	float	unk1C8;		// 1C8
	UInt32	pad1CC[(0x1D4 - 0x1CC) >> 2];	// 1CC
	UInt8	unk1D4;		// 1D4
	UInt8	unk1D5;		// 1D5
	UInt8	pad1D6;		// 1D6
	UInt8	unk1D7;		// 1D7
	UInt8	unk1D8;		// 1D8
	UInt8	unk1D9;		// 1D9
	UInt8	unk1DA;		// 1DA
	UInt8	unk1DB;		// 1DB
	float	unk1DC;		// 1DC
};

STATIC_ASSERT(sizeof(BSFaceGenAnimationData) == 0x1E0);

// 10
class BSFaceGenModelExtraData : public NiExtraData
{
public:
	BSFaceGenModelExtraData();
	~BSFaceGenModelExtraData();

	virtual void *	GetUnk0C(void);

	void	* unk0C;	// 0C
};

// 18
class BSFaceGenBaseMorphExtraData : public NiExtraData
{
public:
	BSFaceGenBaseMorphExtraData();
	~BSFaceGenBaseMorphExtraData();

	void	* unk0C;	// 0C
	UInt32	unk10;		// 10
	UInt32	unk14;		// 14 - count
};

// 20
class DebugTextExtraData : public NiExtraData
{
public:
	DebugTextExtraData();
	~DebugTextExtraData();

	UInt32	unk0C;	// 0C
	BSStringT	data;	// 10
	UInt32	unk18;	// 18
	UInt32	unk1C;	// 1C
};

// 10
class NiStringExtraData : public NiExtraData
{
public:
	NiStringExtraData();
	~NiStringExtraData();

	char	* m_pString;	// 0C
};

// 10
class NiFloatExtraData : public NiExtraData
{
public:
	NiFloatExtraData();
	~NiFloatExtraData();

	float	m_fValue;	// 0C
};

// 10
class FadeNodeMaxAlphaExtraData : public NiFloatExtraData
{
public:
	FadeNodeMaxAlphaExtraData();
	~FadeNodeMaxAlphaExtraData();
};

// 10
class FurnitureMark
{
public:
	FurnitureMark();
	~FurnitureMark();

	NiVector3	pos;	// 00
	SInt16	heading;	// 0C - heading * 1000
	UInt8	number;		// 0E
	UInt8	pad0F;		// 0F
};

// 1C
class BSFurnitureMarker : public NiExtraData
{
public:
	BSFurnitureMarker();
	~BSFurnitureMarker();

	NiTArray <FurnitureMark>	marks;	// 0C
};

// 14
class NiBinaryExtraData : public NiExtraData
{
public:
	NiBinaryExtraData();
	~NiBinaryExtraData();

	void	* data;		// 0C
	UInt32	m_uiSize;	// 10
};

// 24
class BSBound : public NiExtraData
{
public:
	BSBound();
	~BSBound();

	NiVector3	center;		// 0C
	NiVector3	extents;	// 18
};

// 24
class NiSCMExtraData : public NiExtraData
{
public:
	NiSCMExtraData();
	~NiSCMExtraData();

	UInt32	unk0C;		// 0C - size of unk1C? (8 byte chunks)
	UInt32	unk10;		// 10 - size of unk20? (8 byte chunks)
	UInt32	unk14;		// 14
	UInt32	unk18;		// 18
	void	* unk1C;	// 1C
	void	* unk20;	// 20
};

// 14
class NiTextKeyExtraData : public NiExtraData
{
public:
	NiTextKeyExtraData();
	~NiTextKeyExtraData();

	struct Data
	{
		float	time;
		char	* text;
	};

	UInt32	m_uiNumKeys;	// 0C
	Data	* keys;			// 10
};

// 0C
class NiVertWeightsExtraData : public NiExtraData
{
public:
	NiVertWeightsExtraData();
	~NiVertWeightsExtraData();
};

// 24
class bhkExtraData : public NiExtraData
{
public:
	bhkExtraData();
	~bhkExtraData();

	NiTLargeArray <NiTimeController *>	timeControllers;	// 0C
};

// 10
class NiIntegerExtraData : public NiExtraData
{
public:
	NiIntegerExtraData();
	~NiIntegerExtraData();

	UInt32	m_iValue;	// 0C
};

// 10
class BSXFlags : public NiIntegerExtraData
{
public:
	BSXFlags();
	~BSXFlags();

	enum
	{
		kFlag_Animated =	1 << 0,
		kFlag_Havok =		1 << 1,
		kFlag_Ragdoll =		1 << 2,
		kFlag_Complex =		1 << 3,
		kFlag_Flame =		1 << 4,
	};
};

// 14
class NiFloatsExtraData : public NiExtraData
{
public:
	NiFloatsExtraData();
	~NiFloatsExtraData();

	UInt32	m_uiSize;		// 0C
	float	* m_pfValue;	// 10
};

// 1C
class NiColorExtraData : public NiExtraData
{
public:
	NiColorExtraData();
	~NiColorExtraData();

	NiColorAlpha	color;	// 0C
};

// 1C
class NiVectorExtraData : public NiExtraData
{
public:
	NiVectorExtraData();
	~NiVectorExtraData();

	NiVector4	data;	// 0C
};

STATIC_ASSERT(sizeof(NiVectorExtraData) == 0x1C);

// 18
class NiSwitchStringExtraData : public NiExtraData
{
public:
	NiSwitchStringExtraData();
	~NiSwitchStringExtraData();

	UInt32	m_uiSize;		// 0C
	char	** m_ppcValue;	// 10
	UInt32	m_iIndex;		// 14
};

// 14
class NiStringsExtraData : public NiExtraData
{
public:
	NiStringsExtraData();
	~NiStringsExtraData();

	UInt32	m_uiSize;		// 0C
	char	** m_ppcValue;	// 10
};

// 14
class NiIntegersExtraData : public NiExtraData
{
public:
	NiIntegersExtraData();
	~NiIntegersExtraData();

	UInt32	m_uiSize;		// 0C
	UInt32	* m_piValue;	// 10
};

// 10
class NiBooleanExtraData : public NiExtraData
{
public:
	NiBooleanExtraData();
	~NiBooleanExtraData();

	UInt8	m_bValue;	// 0C
	UInt8	pad0D[3];	// 0D
};
