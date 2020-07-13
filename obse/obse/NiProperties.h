#pragma once

#include "obse/NiNodes.h"

/**** properties
 *	name													id	sizeid2	
 *	NiTexturingProperty										06	30
 *	NiVertexColorProperty									07	1C
 *	NiWireframeProperty										08	1C
 *	NiZBufferProperty										09	1C
 *	NiMaterialProperty										02	5C
 *	NiAlphaProperty											00	1C
 *	NiStencilProperty										05	24
 *	NiRendererSpecificProperty								03	18
 *	NiShadeProperty											04	1C
 *		BSShaderProperty									04	6C
 *			SkyShaderProperty								04	8C	0B
 *			ParticleShaderProperty							04	128	0E
 *			BSShaderLightingProperty						04	9C	01
 *				DistantLODShaderProperty					04	AC	02
 *				TallGrassShaderProperty						04	B0	04
 *				BSShaderPPLightingProperty					04	F0	05
 *					SpeedTreeShaderPPLightingProperty		04	F4	07
 *						SpeedTreeBranchShaderProperty		04	F4	07
 *					Lighting30ShaderProperty				04	108	0A
 *					HairShaderProperty						04	170	06
 *				SpeedTreeShaderLightingProperty				04	A8	01
 *					SpeedTreeLeafShaderProperty				04	B0	09
 *					SpeedTreeFrondShaderProperty			missing yet present in rtti?!
 *				GeometryDecalShaderProperty					04	9C	03
 *			PrecipitationShaderProperty						04	AC	0F
 *			BoltShaderProperty								04	19C	0D
 *			WaterShaderProperty								04	88	0C
 *	NiSpecularProperty										0C	1C
 *	NiFogProperty											01	2C
 *		BSFogProperty										01	34
 *	NiDitherProperty										0B	1C
 */

// 018
class NiProperty : public NiObjectNET
{
public:
	NiProperty();
	~NiProperty();

	virtual UInt32	GetPropertyType(void);
	virtual void	Update(float unk0);
};

// 030 (id 06)
class NiTexturingProperty : public NiProperty
{
public:
	NiTexturingProperty();
	~NiTexturingProperty();

	enum { kType = 0x06 };

	enum
	{
		// ------------xxx-	apply mode

		kTexFlags_ApplyMode_Shift =		1,
		kTexFlags_ApplyMode_Mask =		0x0007,

		kTexFlags_ApplyMode_Replace =	0 << kTexFlags_ApplyMode_Shift,
		kTexFlags_ApplyMode_Decal =		1 << kTexFlags_ApplyMode_Shift,
		kTexFlags_ApplyMode_Modulate =	2 << kTexFlags_ApplyMode_Shift,
		kTexFlags_ApplyMode_Hilight =	3 << kTexFlags_ApplyMode_Shift,
		kTexFlags_ApplyMode_Hilight2 =	4 << kTexFlags_ApplyMode_Shift,
	};

	// 10
	class Map
	{
	public:
		virtual void	Destructor(bool arg);
		virtual void	Load(NiStream * stream);
		virtual void	Save(NiStream * stream);

		enum
		{
			// --xx------------	clamp mode
			// ----xxxx--------	filter mode
			// --------xxxxxxxx	texture coord idx

			kClampMode_WrapT =	1 << 12,
			kClampMode_WrapS =	2 << 12,

			kFilterMode_Nearest =			0x00 << 8,
			kFilterMode_Bilerp =			0x01 << 8,
			kFilterMode_Trilerp =			0x02 << 8,
			kFilterMode_NearestMipNearest =	0x03 << 8,
			kFilterMode_NearestMipLerp =	0x04 << 8,
			kFilterMode_BilerpMipNearest =	0x05 << 8,
			kFilterMode_Mask =				0x0F << 8,

			kTexcoord_Mask =				0xFF << 0
		};

//		void	** _vtbl;		// 00
		UInt16	mode;			// 04
		UInt8	pad06[2];		// 06
		void	* m_spTexture;	// 08
		UInt32	unk0C;			// 0C
	};

	// 28
	class BumpMap : public Map
	{
		float	m_fLumaScale;	// 10
		float	m_fLumaOffset;	// 14
		float	m_fBumpMat00;	// 18
		float	m_fBumpMat01;	// 1C
		float	m_fBumpMat10;	// 20
		float	m_fBumpMat11;	// 24
	};

	// 14
	class ShaderMap : public Map
	{
		UInt32	m_uiID;	// 10
	};

	enum
	{
		kMap_Base = 0,
		kMap_Dark,
		kMap_Detail,
		kMap_Gloss,
		kMap_Glow,
		kMap_Bump,
	};

	UInt16					m_texFlags;		// 018
	UInt8					pad01A[2];		// 01A
	NiTArray <Map *>		m_maps;			// 01C
	NiTArray <ShaderMap *>	* m_shaderMaps;	// 02C
};

// 01C (id 07)
class NiVertexColorProperty : public NiProperty
{
public:
	NiVertexColorProperty();
	~NiVertexColorProperty();

	enum { kType = 0x07 };

	enum
	{
		// ----------xx----	src mode
		// ------------x---	lighting mode

		kSrcMode_Ignore =	0 << 4,
		kSrcMode_Emissive =	1 << 4,
		kSrcMode_AmbDiff =	2 << 4,
		kSrcMode_Mask =		3 << 4,

		kLightingMode_Emissive =				0 << 3,
		kLightingMode_EmissiveAmbientDiffuse =	1 << 3,
		kLightingMode_Mask =					1 << 3,
	};

	UInt16	flags;		// 018 - init'd to 8
	UInt8	pad01A[2];	// 01A
};

// 01C (id 08)
class NiWireframeProperty : public NiProperty
{
public:
	NiWireframeProperty();
	~NiWireframeProperty();

	enum { kType = 0x08 };

	UInt16	m_bWireframe;	// 018 - init'd to 0
	UInt8	pad01A[2];		// 01A
};

// 01C (id 09)
class NiZBufferProperty : public NiProperty
{
public:
	NiZBufferProperty();
	~NiZBufferProperty();

	enum { kType = 0x09 };

	enum
	{
		kZTest =					1 << 0,
		kZWrite =					1 << 1,

		kZFunction_Always =			0x00 << 2,
		kZFunction_Less =			0x01 << 2,
		kZFunction_Equal =			0x02 << 2,
		kZFunction_LessEqual =		0x03 << 2,
		kZFunction_Greater =		0x04 << 2,
		kZFunction_NotEqual =		0x05 << 2,
		kZFunction_GreaterEqual =	0x06 << 2,
		kZFunction_Never =			0x07 << 2,
		kZFunction_Mask =			0x0F << 2,
	};

	UInt16	flags;		// 018 - init'd to F
	UInt8	pad01A[2];	// 01A
};

// 5C (id 02)
class NiMaterialProperty : public NiProperty
{
public:
	NiMaterialProperty();
	~NiMaterialProperty();

	enum { kType = 0x02 };

	UInt32	unk018;		// 018
	NiColor	m_amb;		// 01C
	NiColor	m_diff;		// 028
	NiColor	m_spec;		// 034
	NiColor	m_emit;		// 040
	float	m_fShine;	// 04C
	float	m_fAlpha;	// 050
	void	* unk054;	// 054 - freed by NiRenderState
	UInt32	unk058;		// 058
};

// 1C (id 00)
class NiAlphaProperty : public NiProperty
{
public:
	NiAlphaProperty();
	~NiAlphaProperty();

	enum { kType = 0x00 };

	enum
	{
		// ---------------x	alpha
		// -----------xxxx-	srcblend
		// -------xxxx-----	dstblend
		// ------x---------	alpha test
		// ---xxx----------	alpha test mode
		// --x-------------	no sort

		kAlpha =			1 << 0,

		kBlend_One =		0,
		kBlend_Zero,
		kBlend_SrcColor,
		kBlend_InvSrcColor,
		kBlend_DstColor,
		kBlend_InvDstColor,
		kBlend_SrcAlpha,
		kBlend_InvSrcAlpha,
		kBlend_DstAlpha,
		kBlend_InvDstAlpha,
		kBlend_SrcAlphaSat,
		kBlend_Mask =		0x0F,

		kSrcBlendShift =	1,
		kDstBlendShift =	5,

		kAlphaTest =		1 << 9,

		kTestMode_Always =			0 << 10,
		kTestMode_Less =			1 << 10,
		kTestMode_Equal =			2 << 10,
		kTestMode_LessEqual =		3 << 10,
		kTestMode_Greater =			4 << 10,
		kTestMode_NotEqual =		5 << 10,
		kTestMode_GreaterEqual =	6 << 10,
		kTestMode_Never =			7 << 10,

		kNoSort =			1 << 13,
	};

	UInt16	flags;			// 018
	UInt8	alphaTestRef;	// 01A
	UInt8	unk01B;			// 01B
};

// 24 (id 05)
class NiStencilProperty : public NiProperty
{
public:
	NiStencilProperty();
	~NiStencilProperty();

	enum { kType = 0x05 };

	enum
	{
		// ---------------x	enable
		// ------------xxx-	fail action
		// ---------xxx----	zfail action
		// ------xxx-------	pass action
		// ----xx----------	draw mode
		// -xxx------------	test mode

		kEnable =	1 << 0,

		kTestMode_Never =			0 << 12,
		kTestMode_Less =			1 << 12,
		kTestMode_Equal =			2 << 12,
		kTestMode_LessEqual =		3 << 12,
		kTestMode_Greater =			4 << 12,
		kTestMode_NotEqual =		5 << 12,
		kTestMode_GreaterEqual =	6 << 12,
		kTestMode_Always =			7 << 12,
		kTestMode_Mask =			7 << 12,

		kAction_Keep =	0,
		kAction_Zero,
		kAction_Replace,
		kAction_Increment,
		kAction_Decrement,
		kAction_Invert,
		kAction_Mask =	7,

		kFailActionShift =	1,
		kZFailActionShift =	4,
		kPassActionShift =	7,

		kDrawMode_Auto =	0 << 10,
		kDrawMode_CCW =		1 << 10,
		kDrawMode_CW =		2 << 10,
		kDrawMode_Both =	3 << 10,
		kDrawMode_Mask =	3 << 10,
	};

	UInt16	flags;		// 018
	UInt32	m_uiRef;	// 01C
	UInt32	m_uiMask;	// 020
};

// 18 (id 03)
class NiRendererSpecificProperty : public NiProperty
{
public:
	NiRendererSpecificProperty();
	~NiRendererSpecificProperty();

	enum { kType = 0x03 };

	// no idea where the data is in this case, supposed to have derived classes?
};

// 1C (id 04)
class NiShadeProperty : public NiProperty
{
public:
	NiShadeProperty();
	~NiShadeProperty();

	enum { kType = 0x04 };

	virtual UInt32 Unk_15(void);	// returns 0xFFFFFFFF

	enum
	{
		kSmooth = 1
	};

	UInt16	flags;		// 018
	UInt8	pad01A[2];	// 01A
};

STATIC_ASSERT(sizeof(NiShadeProperty) == 0x1C);

// 6C (id 04)
class BSShaderProperty : public NiShadeProperty
{
public:
	BSShaderProperty();
	~BSShaderProperty();

	enum { kType = 0x04 };

	class RenderPass
	{
	public:
		RenderPass();
		~RenderPass();

		UInt32	unk00;		// 00 - vtbl?
		UInt16	type;		// 04 - see 007B4920
		UInt8	isFPass;	// 06
	};

	virtual UInt32	Unk_15(void);		// returns 0 - may be a 'get type'
	virtual bool	Unk_16(UInt32 arg);	// returns 0
	virtual void *	Unk_17(UInt32 arg0, UInt32 arg1);
	virtual UInt16	Unk_18(UInt32 arg);	// returns 1
	virtual void *	Unk_19(void);		// returns &unk58

	enum
	{
		// -------- -------- -------- -------x	specular
		// -------- -------- -------- ------x-	skinned
		// -------- -------- -------- -----x--	low detail
		// -------- -------- -------- ----x---	multi texture
		// -------- -------- -------- ---x----	multi specular
		// -------- -------- -------- x-------	envmap reflection
		// -------- -------- -------x --------	alpha base texture
		// xxxx---- -------- -------- --------	scenegraph

		kSpecular =			0x00000001,
		kSkinned =			0x00000002,
		kLowDetail =		0x00000004,
		kMultiTexture =		0x00000008,
		kMultiSpecular =	0x00000010,
		kEnvmapReflection =	0x00000080,
		kAlphaBaseTexture =	0x00000100,

		kScenegraph_Shift =	28,
		kScenegraph_Mask =	0x0F,
	};

	UInt32	passInfo;						// 01C
	float	alpha;							// 020 - init'd to 1
	UInt32	lastRenderPassState;			// 024 - init'd to 0
	NiTPointerList <RenderPass>	passes;		// 028
	NiTPointerList <RenderPass>	unk38;		// 038
	NiTPointerList <RenderPass>	unk48;		// 048
	NiTPointerList <RenderPass>	unk58;		// 058
	UInt32	unk068;							// 068
};

// 8C (id 04 subid 0B)
class SkyShaderProperty : public BSShaderProperty
{
public:
	SkyShaderProperty();
	~SkyShaderProperty();

	float	unk06C;			// 06C
	float	unk070;			// 070
	float	unk074;			// 074
	float	unk078;			// 078
	NiNode	* blendTexture;	// 07C
	float	blendValue;		// 080
	UInt16	unk084;			// 084
	UInt8	pad086[2];		// 086
	UInt32	soType;			// 088 - init'd to 8
};

// 128 (id 04 subid 0E)
class ParticleShaderProperty : public BSShaderProperty
{
public:
	ParticleShaderProperty();
	~ParticleShaderProperty();
	
	void	* unk06C;	// 06C
	UInt32	unk070;		// 070

	UInt32	pad074;		// 074 - ?

	UInt8	unk078;		// 078
	UInt8	pad079[3];	// 079
	UInt32	unk07C;		// 07C
	float	unk080;		// 080
	float	unk084;		// 084
	float	unk088;		// 088
	float	unk08C;		// 08C
	float	unk090;		// 090
	float	unk094;		// 094
	float	unk098;		// 098
	float	unk09C;		// 09C
	float	unk0A0;		// 0A0
	float	unk0A4;		// 0A4
	float	unk0A8;		// 0A8

	UInt32	pad0AC[(0xB8 - 0xAC) >> 2];	// 0AC - ?

	float	unk0B8;		// 0B8
	float	unk0BC;		// 0BC
	float	unk0C0;		// 0C0
	float	unk0C4;		// 0C4
	float	unk0C8;		// 0C8
	float	unk0CC;		// 0CC
	float	unk0D0;		// 0D0
	float	unk0D4;		// 0D4
	float	unk0D8;		// 0D8
	float	unk0DC;		// 0DC
	float	unk0E0;		// 0E0
	float	unk0E4;		// 0E4

	UInt32	pad0E8[(0xF8 - 0xE8) >> 2];	// 0E8 - ?

	float	unk0F8;		// 0F8

	UInt32	pad0FC[(0x10C - 0x0FC) >> 2];	// 0FC - ?

	UInt32	unk10C;		// 10C
	NiTArray <NiAVObject *>	unk110;	// 110
	UInt32	unk120;		// 120
	float	unk124;		// 124
};

class ShadowSceneLight;
class DECAL_DATA;

// 9C (id 04 subid 01)
class BSShaderLightingProperty : public BSShaderProperty
{
public:
	BSShaderLightingProperty();
	~BSShaderLightingProperty();

	NiTPointerList <ShadowSceneLight *>	lights;	// 06C
	UInt32	unk07C;								// 07C
	NiTPointerList <DECAL_DATA *>		decals;	// 080
	UInt32	unk090;								// 090
	float	unk094;								// 094
	UInt32	refID;								// 098
};

// AC (id 04 subid 02)
class DistantLODShaderProperty : public BSShaderLightingProperty
{
public:
	DistantLODShaderProperty();
	~DistantLODShaderProperty();

	virtual UInt32	Unk_1A(void);		// return unk0A4
	virtual void	Unk_1B(UInt32 arg);	// set unk0A4
	virtual UInt32	Unk_1C(void);		// return ((unk0A4 == 1) || (unk0A4 != 3)) ? 3 : 0
	virtual void	Unk_1D(UInt32 arg);	// if(!arg) unk0A4 = 3; else if((arg > 0) && (arg <= 3)) unk0A4 = 1;

	UInt32	unk09C;	// 09C
	UInt32	unk0A0;	// 0A0
	UInt32	unk0A4;	// 0A4
	UInt32	unk0A8;	// 0A8
};

// B0 (id 04 subid 04)
class TallGrassShaderProperty : public BSShaderLightingProperty
{
public:
	TallGrassShaderProperty();
	~TallGrassShaderProperty();

	// same as DistanceLODShaderProperty only with unk0A8
	virtual UInt32	Unk_1A(void);
	virtual void	Unk_1B(UInt32 arg);
	virtual UInt32	Unk_1C(void);
	virtual void	Unk_1D(UInt32 arg);

	UInt32		unk09C;		// 09C
	NiObject	* unk0A0;	// 0A0
	NiObject	* unk0A4;	// 0A4
	UInt32		unk0A8;		// 0A8
	UInt32		unk0AC;		// 0AC
};

// F0 (id 04 subid 05)
class BSShaderPPLightingProperty : public BSShaderLightingProperty
{
public:
	BSShaderPPLightingProperty();
	~BSShaderPPLightingProperty();

	virtual void *	Unk_1A(void * arg);
	virtual void	Unk_1B(void * arg0, UInt32 arg1, void * arg2);
	virtual UInt32	Unk_1C(void);	// same as DistanceLODShaderProperty 1A-1D with unkDC
	virtual void	Unk_1D(UInt32 arg);
	virtual UInt32	Unk_1E(void);
	virtual void	Unk_1F(UInt32 arg);
	virtual void	Unk_20(UInt32 arg0, UInt32 arg1);
	virtual void	Unk_21(void);
	virtual void	Unk_22(void);
	virtual void	Unk_23(void);
	virtual void	Unk_24(void);
	virtual void	Unk_25(void);
	virtual void	Unk_26(void);
	virtual void	Unk_27(void);

	enum
	{
		// -------- -------- -------- -------x	specular
		// -------- -------- -------- ------x-	skinned
		// -------- -------- -------- -----x--	low detail
		// -------- -------- -------- ----x---	multi texture
		// -------- -------- -------- ---x----	multi specular
		// -------- -------- -------- x-------	envmap reflection
		// -------- -------- -------x --------	alpha base texture
		// -------- -------- -----x-- --------	facegen blend
		// -------- -------- -x------ --------	landscape texturing
		// -------- -------- x------- --------	simple refraction
		// -------- -------x -------- --------	complex refraction
		// xxxx---- -------- -------- --------	scenegraph

		kFacegenBlend =			0x00000400,
		kLandscapeTexturing =	0x00004000,
		kSimpleRefraction =		0x00008000,
		kComplexRefraction =	0x00010000,
	};

	// 1E = no clamp

	struct TexEffectData
	{
		UInt32	unk00[(0x0C - 0x00) >> 2];	// 00

		float	fillR;			// 0C
		float	fillG;			// 10
		float	fillB;			// 14
		float	fillA;			// 18
		float	edgeR;			// 1C
		float	edgeG;			// 20
		float	edgeB;			// 24
		float	edgeA;			// 28

		UInt32	unk2C[(0x54 - 0x2C) >> 2];	// 2C

		float	edgeFalloff;	// 54
	};

	UInt32			pad09C;					// 09C
	UInt32			pad0A0;					// 0A0
	float			envmapLOD;				// 0A4
	float			unk0A8;					// 0A8
	float			unk0AC;					// 0AC
	float			unk0B0;					// 0B0
	float			unk0B4;					// 0B4
	UInt16			numLandscapeTextures;	// 0B8 - init'd to 2
	UInt8			pad0BA[2];				// 0BA
	NiObject		** diffuse;				// 0BC - arrays for multitexturing
	NiObject		** normal;				// 0C0
	NiObject		** glowMap;				// 0C4
	NiObject		** unk0C8;				// 0C8
	UInt32			pad0CC;					// 0CC
	UInt8			* specEnable;			// 0D0
	UInt32			unk0D4;					// 0D4
	UInt32			unk0D8;					// 0D8
	UInt32			unk0DC;					// 0DC - init'd to 1
	TexEffectData	* spTexEffectData;		// 0E0
	UInt8			unk0E4;					// 0E4
	UInt8			pad0E5[3];				// 0E5
	float			refractionPower;		// 0E8
	UInt32			refractionPeriod;		// 0EC
};

// F4 (id 04 subid 05)
class SpeedTreeShaderPPLightingProperty : public BSShaderPPLightingProperty
{
public:
	SpeedTreeShaderPPLightingProperty();
	~SpeedTreeShaderPPLightingProperty();

	struct Str0F0
	{
		UInt32	pad00[2];	// 00
		UInt32	unk08;		// 08
		UInt16	unk0C;		// 0C
	};

	virtual Str0F0 *	Unk_28(void);	// return unk0F0
	virtual UInt32		Unk_29(void);	// return unk0F0 ? unk0F0[8] : 0
	virtual UInt16		Unk_2A(void);	// return unk0F0 ? unk0F0[0x0C] : 0
	virtual void		Unk_2B(UInt32 arg0, UInt16 arg1);	// if(unk0F0) { unk0F0->unk08 = arg0; unk0F0->unk0C = arg1; }
	virtual void		Unk_2C(void);

	Str0F0	* unk0F0;	// 0F0
};

// F4 (id 04 subid 05)
class SpeedTreeBranchShaderProperty : public SpeedTreeShaderPPLightingProperty
{
public:
	SpeedTreeBranchShaderProperty();
	~SpeedTreeBranchShaderProperty();
};

// 108 (id 04 subid 0A)
class Lighting30ShaderProperty : public BSShaderPPLightingProperty
{
public:
	float	unk0F0;	// 0F0
	float	unk0F4;	// 0F4
	float	unk0F8;	// 0F8
	float	unk0FC;	// 0FC

	UInt32	pad100;	// 100

	UInt32	unk104;	// 104
};

// 170 (id 04 subid 06)
class HairShaderProperty : public BSShaderPPLightingProperty
{
public:
	enum
	{
		// -------- -------- -------- -------x	specular
		// -------- -------- -------- ------x-	skinned
		// -------- -------- -------- -----x--	low detail
		// -------- -------- -------- ----x---	multi texture
		// -------- -------- -------- ---x----	multi specular
		// -------- -------- -------- x-------	envmap reflection
		// -------- -------- -------x --------	alpha base texture
		// -------- -------- -----x-- --------	primary light is point
		// -------- -------- ----x--- --------	second light
		// -------- -------- ---x---- --------	third light
		// xxxx---- -------- -------- --------	scenegraph

		kPrimaryLightIsPoint =	0x00000400,	// overrides? facegenblend
		kSecondLight =			0x00000800,
		kThirdLight =			0x00001000,
	};

	NiObject	* unk0F0;		// 0F0
	NiObject	* unk0F4;		// 0F4
	UInt32		unk0F8;			// 0F8
	UInt32		unk0FC;			// 0FC
	UInt32		unk100;			// 100
	UInt32		unk104;			// 104
	UInt32		unk108;			// 108
	UInt32		unk10C;			// 10C
	UInt32		unk110;			// 110
	UInt32		unk114;			// 114
	UInt32		unk118;			// 118
	float		unk11C;			// 11C
	float		unk120;			// 120
	float		unk124;			// 124
	float		unk128;			// 128
	float		unk12C;			// 12C
	float		unk130;			// 130
	float		unk134;			// 134
	float		unk138;			// 138
	float		unk13C;			// 13C
	float		unk140;			// 140
	float		unk144;			// 144
	float		unk148;			// 148
	float		unk14C;			// 14C
	float		unk150;			// 150
	float		unk154;			// 154
	float		unk158;			// 158
	float		unk15C;			// 15C
	float		unk160;			// 160
	float		unk164;			// 164
	NiObject	* heightMap;	// 168
	UInt32		unk16C;			// 16C
};

// A8+ (id 04 subid 01)
class SpeedTreeShaderLightingProperty : public BSShaderLightingProperty
{
public:
	SpeedTreeShaderLightingProperty();
	~SpeedTreeShaderLightingProperty();

	virtual void *	Unk_1A(void);
	virtual UInt32	Unk_1B(void);			// same as DistanceLODShaderProperty 1A-1D with unk0A4
	virtual void	Unk_1C(UInt32 arg);
	virtual UInt32	Unk_1D(void);
	virtual void	Unk_1E(UInt32 arg);
	virtual void	Unk_1F(NiObject * obj);	// set unk09C, destroy old object
	virtual UInt32	Unk_20(void);			// get unk0A0
	virtual void	Unk_21(UInt32 arg);		// set unk0A0
	virtual UInt32	Unk_22(void);			// return 0 or 3 based on unk0A0
	virtual void	Unk_23(UInt32 arg);		// set unk0A0 to 1 or 3 based on arg

	struct Str0A4	// same as SpeedTreeShaderPPLightingProperty::Str0F0?
	{
		UInt32	pad00[2];	// 00
		UInt32	unk08;		// 08
		UInt16	unk0C;		// 0C
	};
	
	NiObject	* unk09C;	// 09C
	UInt32		unk0A0;		// 0A0
	Str0A4		* unk0A4;	// 0A4
};

class NiAdditionalGeometryData;

// B0 (id 04 subid 09)
class SpeedTreeLeafShaderProperty : public SpeedTreeShaderLightingProperty
{
public:
	SpeedTreeLeafShaderProperty();
	~SpeedTreeLeafShaderProperty();

	virtual SpeedTreeLeafShaderProperty *	Unk_24(void);	// allocate a new SpeedTreeLeafShaderProperty
	virtual NiAdditionalGeometryData *		Unk_25(void);	// allocate a new NiAdditionalGeometryData
	virtual UInt16							Unk_26(void);	// return unk0AC
	virtual void *							Unk_27(void);	// return unk0A8
	virtual UInt32							Unk_28(void);	// return unk0A8 ? unk0A8[8] : 0
	virtual void							Unk_29(void);	// call a fn on unk0A8 if it exists
	
	void	* unk0A8;	// 0A8
	UInt16	unk0AC;		// 0AC
	UInt8	unk0AE[2];	// 0AE
};

// 9C (id 04 subid 03)
class GeometryDecalShaderProperty : public BSShaderLightingProperty
{
public:
	GeometryDecalShaderProperty();
	~GeometryDecalShaderProperty();

	// 09C
};

// AC (id 04 subid 0F)
class PrecipitationShaderProperty : public BSShaderProperty
{
public:
	PrecipitationShaderProperty();
	~PrecipitationShaderProperty();

	// same as DistanceLODShaderProperty only with unk0A0
	virtual UInt32	Unk_1A(void);
	virtual void	Unk_1B(UInt32 arg);
	virtual UInt32	Unk_1C(void);
	virtual void	Unk_1D(UInt32 arg);

	UInt32		unk06C;	// 06C
	UInt32		pad070;	// 070
	float		unk074;	// 074
	UInt32		unk078;	// 078
	UInt32		unk07C;	// 07C
	UInt32		unk080;	// 080
	UInt32		unk084;	// 084
	UInt32		unk088;	// 088
	UInt32		unk08C;	// 08C
	UInt32		unk090;	// 090
	UInt32		unk094;	// 094
	UInt32		unk098;	// 098
	NiObject *	unk09C;	// 09C
	UInt32		unk0A0;	// 0A0
	UInt32		unk0A4;	// 0A4 - init'd to 1
	UInt32		unk0A8;	// 0A8
};

// 19C (type 04 subid 0D)
class BoltShaderProperty : public BSShaderProperty
{
public:
	BoltShaderProperty();
	~BoltShaderProperty();

	void	* unk06C;	// 06C
	float	unk070;		// 070
	float	unk074;		// 074
	UInt32	unk078;		// 078
	float	unk07C;		// 07C
	UInt32	unk080;		// 080 - init'd to 1
	UInt32	unk084;		// 084
	UInt32	unk088;		// 088
	UInt32	unk08C;		// 08C
	float	unk090;		// 090
	float	unk094[40];	// 094 - init'd to random values from 0-1
	UInt32	unk134;		// 134 - init'd to 1
	float	unk138;		// 138 - init'd to 2048
	float	unk13C;		// 13C - init'd to 100
	float	unk140;		// 140 - init'd to 3
	float	unk144;		// 144 - init'd to 3
	float	unk148;		// 148
	UInt32	unk14C;		// 14C
	float	unk150;		// 150
	float	unk154;		// 154
	UInt32	unk158;		// 158
	float	unk15C;		// 15C
	float	unk160;		// 160
	float	unk164;		// 164
	float	unk168;		// 168
	float	unk16C;		// 16C
	float	unk170;		// 170
	float	unk174;		// 174
	float	unk178;		// 178
	float	unk17C;		// 17C
	UInt8	unk180;		// 180
	UInt8	unk181;		// 181 - init'd to 1
	UInt8	unk182;		// 182 - init'd to 1
	UInt8	unk183;		// 183 - init'd to 1
	UInt32	unk184;		// 184
	UInt32	pad188;		// 188
	float	unk18C;		// 18C
	UInt32	unk190;		// 190
	UInt32	unk194;		// 194
	float	unk198;		// 198 - init'd to 1
};

// 88 (type 04 subid 0D)
class WaterShaderProperty : public BSShaderProperty
{
public:
	WaterShaderProperty();
	~WaterShaderProperty();

	UInt32	unk06C;		// 06C
	UInt8	unk070;		// 070
	UInt8	unk071;		// 071
	UInt8	unk072;		// 072
	UInt8	pad073;		// 073
	UInt32	unk074;		// 074
	UInt32	unk078;		// 078
	float	unk07C;		// 07C - init'd to 1
	float	unk080;		// 080
	UInt8	unk084;		// 084
	UInt8	unk085;		// 085
	UInt8	pad086[2];	// 086
};

// 1C (type 0C)
class NiSpecularProperty : public NiProperty
{
public:
	NiSpecularProperty();
	~NiSpecularProperty();

	enum
	{
		kEnable =	1 << 0,
	};

	UInt16	flags;		// 018
	UInt8	pad01A[2];	// 01A
};

// 2C (type 01)
class NiFogProperty : public NiProperty
{
public:
	NiFogProperty();
	~NiFogProperty();

	enum
	{
		// ---------------x	enable
		// -------------xx-	function

		kEnable =	1 << 0,

		kFunction_ZLinear =			0 << 1,
		kFunction_RangeSquared =	1 << 1,
		kFunction_VertexAlpha =		2 << 1,
		kFunction_Mask =			3 << 1,
	};

	UInt16	flags;		// 018
	UInt8	pad01A[2];	// 01A
	float	depth;		// 01C
	NiColor	color;		// 020
};

// 34 (type 01)
class BSFogProperty : public NiFogProperty
{
public:
	BSFogProperty();
	~BSFogProperty();

	float	fogStart;	// 02C
	float	fogEnd;		// 030
};

// 1C (type 0B)
class NiDitherProperty : public NiProperty
{
public:
	NiDitherProperty();
	~NiDitherProperty();

	enum
	{
		kEnable =	1 << 0,
	};

	UInt16	flags;		// 018
	UInt8	pad01A[2];	// 01A
};
