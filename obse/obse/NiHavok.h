#pragma once

/**** havok
 *	bhkRefObject
 *		bhkSerializable
 *			bhkWorld - NiRTTI is wrong here
 *				bhkWorldM
 *			bhkAction
 *				bhkUnaryAction
 *					bhkMouseSpringAction
 *					bhkMotorAction
 *				bhkBinaryAction
 *					bhkSpringAction
 *					bhkAngularDashpotAction
 *					bhkDashpotAction
 */

#include "obse/HavokBase.h"
#include "obse/HavokTypes.h"
#include "obse/NiNodes.h"

class hkWorld;

// 00C
class bhkRefObject : public NiObject
{
public:
	bhkRefObject();
	~bhkRefObject();

	virtual void	SetObj(hkReferencedObject * obj);
	virtual void	UpdateRefcount(bool incRef);	// inc/decref depending on arg

	hkReferencedObject	* hkObj;	// 008
};

// 010+
class bhkSerializable : public bhkRefObject
{
public:
	bhkSerializable();
	~bhkSerializable();

	virtual void		Unk_15(UInt32 arg);
	virtual hkWorld *	GetWorld(void);
	virtual bool		Unk_17(UInt32 arg);
	virtual bool		Unk_18(void);
	virtual void		FreeData(bool del);	// free hkData
	virtual UInt32		Unk_1A(void);
	virtual void		LoadHavokData(NiStream * stream);	// called from bhkSerializable::Load after primary load is done
	virtual void		Unk_1C(void) = 0;	// create object
	virtual void *		CreateHavokData(UInt8 * arg) = 0;	// create Cinfo, return hkData
	virtual void		Unk_1E(void);		// destroy object
	virtual void		Unk_1F(void);

	void	* hkData;	// 00C - stores hkConstraintData (descriptor used to build hkObj)
};

// 090
class bhkWorld : public bhkSerializable
{
public:
	bhkWorld();
	~bhkWorld();

	virtual void		Unk_20(void);	// reports some statistics (num raycasts)
	virtual void		Unk_21(void);
	virtual void		Unk_22(void);	// pick object? builds raycaster
	virtual void		Unk_23(void);
	virtual void		Unk_24(void);
	virtual void		Unk_25(void);
	virtual void		Unk_26(void);

	// ahkWorld * hkObj		// 008

	void		* unk010;	// 010 - 0x108 byte object
	NiRefObject	* visDebug;	// 014
	UInt8		unk018;		// 018
	UInt8		enabled;	// 019
	UInt8		unk01A;		// 01A
	UInt8		pad01B;		// 01B
	UInt32		unk01C;		// 01C
	UInt32		unk020;		// 020
	UInt32		unk024;		// 024
	void		* unk028;	// 028 - 0x2EE0 byte buffer
	UInt32		unk02C;		// 02C
	void		* unk030;	// 030 - 0x320 byte buffer
	UInt32		unk034;		// 034
	void		* unk038;	// 038 - 0x190 byte buffer
	UInt32		unk03C;		// 03C
	void		* unk040;	// 040 - 0x320 byte buffer
	UInt32		unk044;		// 044
	void		* unk048;	// 048 - 0x2EE0 byte buffer
	UInt32		unk04C;		// 04C
	NiVector4	unk050;		// 050
	NiRefObject	** unk060;	// 060 - simple array
	UInt32		unk064;		// 064 - num elements in 060
	UInt32		unk068;		// 068
	void		* unk06C;	// 06C
	UInt32		unk070;		// 070
	UInt32		unk074;		// 074
	UInt32		pad078[(0x90 - 0x78) >> 2];	// 078
};

STATIC_ASSERT(sizeof(bhkWorld) == 0x90);

// 0C0
class bhkWorldM : public bhkWorld
{
public:
	bhkWorldM();
	~bhkWorldM();

	float		unk080;			// 080 - probably another hkVector4
	float		unk084;			// 084
	float		unk088;			// 088
	UInt32		pad08C;			// 08C
	hkVector4	borderSize;		// 090
	hkVector4	worldTotalSize;	// 0A0
};

// 010
class bhkAction : public bhkSerializable
{
public:
	bhkAction();
	~bhkAction();
};

// 010
class bhkUnaryAction : public bhkAction
{
public:
	bhkUnaryAction();
	~bhkUnaryAction();
};

// 010
class bhkMouseSpringAction : public bhkUnaryAction
{
public:
	bhkMouseSpringAction();
	~bhkMouseSpringAction();

	// damping, elasticity, object damping, max force stored on object
};

// 010
class bhkMotorAction : public bhkUnaryAction
{
public:
	bhkMotorAction();
	~bhkMotorAction();

	// spin rate, gain stored on object
};

// 010
class bhkBinaryAction : public bhkAction
{
public:
	bhkBinaryAction();
	~bhkBinaryAction();

	virtual void	Unk_20(void);
	virtual void	Unk_21(void);
	virtual void	Unk_22(void);
};

// 010
class bhkSpringAction : public bhkBinaryAction
{
public:
	bhkSpringAction();
	~bhkSpringAction();
};

// 010
class bhkAngularDashpotAction : public bhkBinaryAction
{
public:
	bhkAngularDashpotAction();
	~bhkAngularDashpotAction();
};

// 010
class bhkDashpotAction : public bhkBinaryAction
{
public:
	bhkDashpotAction();
	~bhkDashpotAction();
};
