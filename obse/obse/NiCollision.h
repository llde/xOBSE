#pragma once

#include "obse/NiNodes.h"

/****
 *	NiCollisionObject
 *		NiCollisionData
 *		bhkNiCollisionObject
 *			bhkCollisionObject
 *				bhkBlendCollisionObject
 *					WeaponObject
 *					bhkBlendCollisionObjectAddRotation
 *			bhkPCollisionObject
 *				bhkSPCollisionObject
 */

class NiBoundingVolume;

// C?
class NiCollisionObject : public NiObject
{
public:
	NiCollisionObject();
	~NiCollisionObject();

	virtual void	Attach(NiAVObject * obj);
	virtual void	Unk_14(void) = 0;
	virtual void	Unk_15(void) = 0;
	virtual void	LoadBoundingVolume(UInt32 arg) = 0;
	virtual void	Unk_17(UInt32 version, UInt32 arg1) = 0;
	virtual void	Unk_18(void) = 0;

	NiAVObject	* m_pkSceneObject;	// 008
};

// 50
class NiCollisionData : public NiCollisionObject
{
public:
	NiCollisionData();
	~NiCollisionData();

	enum
	{
		kPropagate_OnSuccess = 0,
		kPropagate_OnFailure,
		kPropagate_Always,
		kPropagate_Never
	};

	enum
	{
		kMode_OBB = 0,
		kMode_Triangles,
		kMode_AlternateBoundingVolume,
		kMode_NoTest,
		kMode_NiBound,	// aka sphere tests
	};

	NiVector3	m_kLocalVelocity;	// 00C
	NiVector3	m_kWorldVelocity;	// 018
	UInt32		m_ePropagationMode;	// 024 - init'd to 2
	UInt32		m_eCollisionMode;	// 028 - init'd to 3
	NiBoundingVolume	* m_pkModelABV;	// 02C
	NiBoundingVolume	* m_pkWorldABV;	// 030
	NiObject	* unk034;	// 034
	UInt32		unk038;		// 038
	UInt32		unk03C;		// 03C
	void		* unk040;	// 040
	void		* unk044;	// 044
	UInt8		unk048;		// 048
	UInt8		unk049;		// 049
	UInt8		pad04A[2];	// 04A
	UInt16		unk04C;		// 04C
	UInt8		unk04E;		// 04E
	UInt8		pad04F;		// 04F
};

// 14
class bhkNiCollisionObject : public NiCollisionObject
{
public:
	bhkNiCollisionObject();
	~bhkNiCollisionObject();

	virtual void	Unk_19(void) = 0;
	virtual void	Unk_1A(void) = 0;
	virtual void	Unk_1B(void) = 0;
	virtual void	Unk_1C(void) = 0;
	virtual bool	IsKeyframed(void) = 0;
	virtual void	Unk_1E(void * arg) = 0;
	virtual void	Unk_1F(void * arg) = 0;

	enum
	{
		kFlag_Active =		1 << 0,
		kFlag_Reset =		1 << 1,
		kFlag_Notify =		1 << 2,
		kFlag_SetLocal =	1 << 3,
	};

	UInt16	flags;				// 00C
	UInt8	pad00E[2];			// 00E
	void	* bhkWorldObject;	// 010
};

// 14
class bhkCollisionObject : public bhkNiCollisionObject
{
public:
	bhkCollisionObject();
	~bhkCollisionObject();

	enum
	{
		kFlag_UseVel1 =	1 << 5,	// both must be on for bUseVel to show as true
		kFlag_UseVel2 =	1 << 6,
	};
};

// 28
class bhkBlendCollisionObject : public bhkCollisionObject
{
public:
	bhkBlendCollisionObject();
	~bhkBlendCollisionObject();

	enum
	{
		kFlag_BlendPos =	1 << 8,
		kFlag_AlwaysBlend =	1 << 9,
	};

	float	fHeirGain;		// 014
	float	fVelGain;		// 018
	UInt32	unk01C;			// 01C
	void	* storedWorld;	// 020
	UInt32	pad024;			// 024
};

// 28
class WeaponObject : public bhkBlendCollisionObject
{
public:
	WeaponObject();
	~WeaponObject();
};

// 4C
class bhkBlendCollisionObjectAddRotation : public bhkBlendCollisionObject
{
public:
	bhkBlendCollisionObjectAddRotation();
	~bhkBlendCollisionObjectAddRotation();

	NiMatrix33	rot;	// 028
};

// 14
class bhkPCollisionObject : public bhkNiCollisionObject
{
public:
	bhkPCollisionObject();
	~bhkPCollisionObject();
};

// 14
class bhkSPCollisionObject : public bhkPCollisionObject
{
public:
	bhkSPCollisionObject();
	~bhkSPCollisionObject();
};
