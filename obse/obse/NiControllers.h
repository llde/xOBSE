#pragma once

#include "obse/NiNodes.h"

/**** controllers
 *	NiTimeController
 *		BSDoorHavokController
 *		BSPlayerDistanceCheckController
 *		NiD3DController
 *		NiControllerManager
 *		NiInterpController
 *			NiSingleInterpController
 *				NiTransformController
 *				NiPSysModifierCtlr
 *					NiPSysEmitterCtlr
 *					NiPSysModifierBoolCtlr
 *						NiPSysModifierActiveCtlr
 *					NiPSysModifierFloatCtlr
 *						NiPSysInitialRotSpeedVarCtlr
 *						NiPSysInitialRotSpeedCtlr
 *						NiPSysInitialRotAngleVarCtlr
 *						NiPSysInitialRotAngleCtlr
 *						NiPSysGravityStrengthCtlr
 *						NiPSysFieldMaxDistanceCtlr
 *						NiPSysFieldMagnitudeCtlr
 *						NiPSysFieldAttenuationCtlr
 *						NiPSysEmitterSpeedCtlr
 *						NiPSysEmitterPlanarAngleVarCtlr
 *						NiPSysEmitterPlanarAngleCtlr
 *						NiPSysEmitterLifeSpanCtlr
 *						NiPSysEmitterInitialRadiusCtlr
 *						NiPSysEmitterDeclinationVarCtlr
 *						NiPSysEmitterDeclinationCtlr
 *						NiPSysAirFieldSpreadCtlr
 *						NiPSysAirFieldInheritVelocityCtlr
 *						NiPSysAirFieldAirFrictionCtlr
 *				NiFloatInterpController
 *					NiFlipController
 *					NiAlphaController
 *					NiTextureTransformController
 *					NiLightDimmerController
 *				NiBoolInterpController
 *					NiVisController
 *				NiPoint3InterpController
 *					NiMaterialColorController
 *					NiLightColorController
 *				NiExtraDataController
 *					NiFloatsExtraDataPoint3Controller
 *					NiFloatsExtraDataController
 *					NiFloatExtraDataController
 *					NiColorExtraDataController
 *			NiMultiTargetTransformController
 *			NiGeomMorpherController
 *		bhkBlendController
 *		bhkForceController
 *		NiBSBoneLODController
 *		NiUVController
 *		NiPathController
 *		NiLookAtController
 *		NiKeyframeManager
 *		NiBoneLODController
 *		NiPSysUpdateCtlr
 *		NiPSysResetOnLoopCtlr
 *		NiFloatController
 *			NiRollController
 */

class NiInterpolator;
class NiDefaultAVObjectPalette;

// 03C
class NiTimeController : public NiObject
{
public:
	NiTimeController();
	~NiTimeController();

	virtual void		Start(float time);
	virtual void		Stop(void);
	virtual void		Update(float time) = 0;
	virtual void		SetTarget(NiObjectNET * node);
	virtual bool		Unk_17(void);
	virtual bool		Unk_18(void);
	virtual void		Unk_19(void);
	virtual bool		Unk_1A(void);
	virtual bool		Unk_1B(void);
	virtual bool		Unk_1C(void) = 0;

	enum
	{
		kAnimType_AppTime =		0 << 0,
		kAnimType_AppInit =		1 << 0,

		kCycleType_Loop =		0 << 1,
		kCycleType_Reverse =	1 << 1,
		kCycleType_Clamp =		2 << 1,

		kActive =				1 << 3,
		
		kPlayBackwards =		1 << 4,
	};

	UInt16				flags;			// 008
	UInt8				pad00A[2];		// 00A
	float				m_fFrequency;	// 00C
	float				m_fPhase;		// 010
	float				m_fLoKeyTime;	// 014
	float				m_fHiKeyTime;	// 018
	float				m_fStartTime;	// 01C
	float				m_fLastTime;	// 020
	float				unk024;			// 024 - updated in Unk_19
	float				unk028;			// 028
	UInt8				unk02C;			// 02C
	UInt8				pad02D[3];		// 02D
	NiObjectNET			* m_pTarget;	// 030 - type is a (wrong) guess; NET has a back controller pointer, but many children assume larger objects
	NiObject			* next;			// 034 - singly linked list of controllers
	UInt8				unk038;			// 038
	UInt8				unk039[3];		// 039
};

// 040
class BSDoorHavokController : public NiTimeController
{
public:
	BSDoorHavokController();
	~BSDoorHavokController();

	UInt8	unk03C;		// 03C - Update skips once if nonzero, then resets back to zero (wtf)
	UInt8	unk03D[3];	// 03D
};

// NiD3DController - unref'd

// 080
class NiControllerManager : public NiTimeController
{
public:
	NiControllerManager();
	~NiControllerManager();

	NiTArray <NiControllerSequence *>	sequences;			// 03C - NiTArray <NiPointer <NiControllerSequence> > >
	NiControllerSequence	** unk04C;
	UInt32	unk050;
	UInt32	unk054;
	NiTStringPointerMap <NiControllerSequence *>	sequenceMap;	// 058
	UInt32	pad06C;	// 06C
	UInt32	pad070;	// 070
	UInt32	pad074;	// 074
	UInt32	pad078;	// 078
	NiDefaultAVObjectPalette	* objectPalette;	// 07C
};

// 03C
class NiInterpController : public NiTimeController
{
public:
	NiInterpController();
	~NiInterpController();

	virtual UInt16	GetNumInterpolators(void) = 0;	// 1D
	virtual char *	GetInterpolatorName(UInt16 idx) = 0;
	virtual UInt16	GetInterpolatorIdx(char * name) = 0;
	virtual NiInterpolator *	GetInterpolator(UInt16 idx) = 0;
	virtual void	SetInterpolator(NiInterpolator * _interpolator, UInt16 idx) = 0;
	virtual void	Unk_22(void);
	virtual char *	GetName(void);
	virtual void	Unk_24(UInt16 idx) = 0;	// create interpolators
	virtual void	Unk_25(void) = 0;
	virtual void	Unk_26(UInt16 idx) = 0;	// create blend interpolators
	virtual void	Unk_27(void) = 0;
	virtual void	Unk_28(void * arg, UInt16 idx) = 0;

	enum
	{
		kIsManagerControlled =	1 << 5,
	};
};

// 040
class NiSingleInterpController : public NiInterpController
{
public:
	NiSingleInterpController();
	~NiSingleInterpController();

	virtual bool	Unk_29(void * arg) = 0;

	NiInterpolator	* interpolator;	// 03C
};

// 040
class NiTransformController : public NiSingleInterpController
{
public:
	NiTransformController();
	~NiTransformController();
};

// 048+
class NiPSysModifierCtlr : public NiSingleInterpController
{
public:
	NiPSysModifierCtlr();
	~NiPSysModifierCtlr();

	char	* modifierName;	// 040
	void	* unk044;		// 044 - cached from an NiTMapBase lookup on m_pTarget
};

// 064 (type 02)
class NiPSysEmitterCtlr : public NiPSysModifierCtlr
{
public:
	NiPSysEmitterCtlr();
	~NiPSysEmitterCtlr();

	virtual bool	Unk_29(void * arg);
	virtual void	Unk_2A(void);
	virtual void	Unk_2B(void);

	// interpolators
	// 0: BirthRate
	// 1: EmitterActive

	NiRefObject	* unk048;		// 048
	// 04C
	float	unk050;				// 050 - set to -inf when stopped
	UInt8	unk054;				// 054
	UInt8	pad055[3];			// 055
	UInt32	unk058;				// 058
	UInt32	unk05C;				// 05C
	UInt32	unk060;				// 060
};

// 048
class NiPSysModifierBoolCtlr : public NiPSysModifierCtlr
{
public:
	NiPSysModifierBoolCtlr();
	~NiPSysModifierBoolCtlr();

	virtual void	Unk_2C(void) = 0;
};

// 048
class NiPSysModifierActiveCtlr : public NiPSysModifierBoolCtlr
{
public:
	NiPSysModifierActiveCtlr();
	~NiPSysModifierActiveCtlr();
};

// 048
class NiPSysModifierFloatCtlr : public NiPSysModifierCtlr
{
public:
	NiPSysModifierFloatCtlr();
	~NiPSysModifierFloatCtlr();

	virtual void	Unk_2C(void) = 0;
};

// these are all the same except for vtbl (usually changing which interpolator is spawned)
#define FLOAT_CTLR(name)	class name : public NiPSysModifierFloatCtlr { public : name(); ~name(); };

FLOAT_CTLR(NiPSysInitialRotSpeedVarCtlr);
FLOAT_CTLR(NiPSysInitialRotSpeedCtlr);
FLOAT_CTLR(NiPSysInitialRotAngleVarCtlr);
FLOAT_CTLR(NiPSysInitialRotAngleCtlr);
FLOAT_CTLR(NiPSysGravityStrengthCtlr);
FLOAT_CTLR(NiPSysFieldMaxDistanceCtlr);
FLOAT_CTLR(NiPSysFieldMagnitudeCtlr);
FLOAT_CTLR(NiPSysFieldAttenuationCtlr);
FLOAT_CTLR(NiPSysEmitterSpeedCtlr);
FLOAT_CTLR(NiPSysEmitterPlanarAngleVarCtlr);
FLOAT_CTLR(NiPSysEmitterPlanarAngleCtlr);
FLOAT_CTLR(NiPSysEmitterLifeSpanCtlr);
FLOAT_CTLR(NiPSysEmitterInitialRadiusCtlr);
FLOAT_CTLR(NiPSysEmitterDeclinationVarCtlr);
FLOAT_CTLR(NiPSysEmitterDeclinationCtlr);
FLOAT_CTLR(NiPSysAirFieldSpreadCtlr);
FLOAT_CTLR(NiPSysAirFieldInheritVelocityCtlr);
FLOAT_CTLR(NiPSysAirFieldAirFrictionCtlr);

#undef FLOAT_CTLR

// 040
class NiFloatInterpController : public NiSingleInterpController
{
public:
	NiFloatInterpController();
	~NiFloatInterpController();

	virtual void	Unk_2A(float * out) = 0;
};

// 05C
class NiFlipController : public NiFloatInterpController
{
public:
	NiFlipController();
	~NiFlipController();

	NiTArray <NiTexture *>	textures;	// 040
	UInt32	texIdx;	// 050
	UInt32	unk054;	// 054
	char	* name;	// 058
};

// 040
class NiAlphaController : public NiFloatInterpController
{
public:
	NiAlphaController();
	~NiAlphaController();
};

// 058
class NiTextureTransformController : public NiFloatInterpController
{
public:
	NiTextureTransformController();
	~NiTextureTransformController();

	enum
	{
		kTarget_Base = 0,
		kTarget_Dark,
		kTarget_Detail,
		kTarget_Gloss,
		kTarget_Glow,
		kTarget_Bump,
		kTarget_Normal,
		kTarget_Unknown2,
		kTarget_Decal0,
		kTarget_Decal1,
		kTarget_Decal2,
		kTarget_Decal3,
	};

	enum
	{
		kTransform_TranslateU = 0,
		kTransform_TranslateV,
		kTransform_Rotate,
		kTransform_ScaleU,
		kTransform_ScaleV,
	};

	UInt32	unk040;			// 040
	void	* unk044;		// 044
	UInt8	unk048;			// 048
	UInt8	pad049[3];		// 049
	UInt32	targetType;		// 04C
	UInt32	transformType;	// 050
	char	* name;			// 054
};

// 040
class NiLightDimmerController : public NiFloatInterpController
{
public:
	NiLightDimmerController();
	~NiLightDimmerController();
};

// 040
class NiBoolInterpController : public NiSingleInterpController
{
public:
	NiBoolInterpController();
	~NiBoolInterpController();

	virtual void	Unk_2A(UInt8 * out) = 0;
};

// 040
class NiVisController : public NiBoolInterpController
{
public:
	NiVisController();
	~NiVisController();
};

// 040+
class NiPoint3InterpController : public NiSingleInterpController
{
public:
	NiPoint3InterpController();
	~NiPoint3InterpController();

	virtual void	Unk_2A(NiVector3 * out) = 0;
};

// 044
class NiMaterialColorController : public NiPoint3InterpController
{
public:
	NiMaterialColorController();
	~NiMaterialColorController();

	enum
	{
		// -------------xxx target

		kFlag_Ambient = 0,
		kFlag_Diffuse,
		kFlag_Specular,
		kFlag_Selfillum,
	};

	UInt16	flags;		// 040
	UInt8	pad042[2];	// 042
};

// 044
class NiLightColorController : public NiPoint3InterpController
{
public:
	NiLightColorController();
	~NiLightColorController();

	enum
	{
		// ---------------x amb/diffuse

		kFlag_Diffuse = 0,
		kFlag_Ambient,
	};

	UInt16	flags;		// 040
	UInt8	pad042[2];	// 042
};

// 048
class NiExtraDataController : public NiSingleInterpController
{
public:
	NiExtraDataController();
	~NiExtraDataController();

	char		* extraDataName;	// 040
	NiRefObject	* extraData;		// 044
};

// 050
class NiFloatsExtraDataPoint3Controller : public NiExtraDataController
{
public:
	NiFloatsExtraDataPoint3Controller();
	~NiFloatsExtraDataPoint3Controller();

	UInt32	m_iFloatsExtraDataIndex;	// 048
	char	* name;						// 04C
};

// 050
class NiFloatsExtraDataController : public NiExtraDataController
{
public:
	NiFloatsExtraDataController();
	~NiFloatsExtraDataController();

	UInt32	m_iFloatsExtraDataIndex;	// 048
	char	* name;						// 04C
};

// 048
class NiFloatExtraDataController : public NiExtraDataController
{
public:
	NiFloatExtraDataController();
	~NiFloatExtraDataController();
};

// 048
class NiColorExtraDataController : public NiExtraDataController
{
public:
	NiColorExtraDataController();
	~NiColorExtraDataController();
};

// 048
class NiMultiTargetTransformController : public NiInterpController
{
public:
	NiMultiTargetTransformController();
	~NiMultiTargetTransformController();

	void		* interpolators;	// 03C - spaced every 0x30 bytes? must be a linear array
	NiObject	** targets;			// 040
	UInt16		m_usNumInterps;		// 044
	UInt8		pad046[2];			// 046
};

// 05C
class NiGeomMorpherController : public NiInterpController
{
public:
	NiGeomMorpherController();
	~NiGeomMorpherController();

	struct Str050
	{
		UInt32	unk00;				// 00
		UInt32	unk04;				// 04
		UInt16	numInterpolators;	// 08
		UInt8	pad0A[2];			// 0A
		UInt32	unk0C;				// 0C
		UInt32	unk10;				// 10
	};

	virtual void *	Unk_29(void);
	virtual void	Unk_2A(void);

	UInt16				flags;				// 03C
	UInt8				pad03E[2];			// 03E
	NiTArray <float>	unk040;				// 040
	Str050				* interpolatorInfo;	// 050
	NiInterpolator		** interpolators;	// 054
	UInt8				unk058;				// 058
	UInt8				unk059;				// 059
	UInt8				alwaysUpdate;		// 05A
	UInt8				unk05B;				// 05B
};

// 064
class bhkBlendController : public NiTimeController
{
public:
	bhkBlendController();
	~bhkBlendController();

	// ???
	struct BLENDKEY
	{
		//
	};

	UInt32			unk03C;	// 03C
	NiTLargeArray <BLENDKEY>	unk040;	// 040
	float			unk058;	// 058
	float			unk05C;	// 05C
	UInt32			unk060;	// 060
};

// 070? might be another derived class, lots of pad
class bhkForceController : public NiTimeController
{
public:
	bhkForceController();
	~bhkForceController();

	bool	Unk_1D(void);

	UInt32		pad03C;		// 03C
	NiVector4	unk040;		// 040
	float		unk050;		// 050
	UInt32		pad054[(0x070 - 0x054) >> 2];	// 054
};

// 054
class NiBSBoneLODController : public NiTimeController
{
public:
	NiBSBoneLODController();
	~NiBSBoneLODController();

	bool	Unk_1D(void);

	UInt32	m_iLOD;			// 03C - init'd to -1
	UInt32	m_uiNumLODs;	// 040
	NiTArray <NiTSet <NiNode *> *>	unk044;	// 044
};

// 03C
class NiPSysUpdateCtlr : public NiTimeController
{
public:
	NiPSysUpdateCtlr();
	~NiPSysUpdateCtlr();
};

// 040
class NiPSysResetOnLoopCtlr : public NiTimeController
{
public:
	NiPSysResetOnLoopCtlr();
	~NiPSysResetOnLoopCtlr();

	float	unk03C;	// 03C
};

// 044
class NiFloatController : public NiTimeController
{
public:
	NiFloatController();
	~NiFloatController();

	UInt32		unk03C;		// 03C
	NiRefObject	* unk040;	// 040
};

// 044
class NiRollController : public NiFloatController
{
public:
	NiRollController();
	~NiRollController();
};
