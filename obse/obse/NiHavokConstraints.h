#pragma once

/**** havok constraints
 *	bhkConstraint
 *		bhkLimitedHingeConstraint
 *		bhkMalleableConstraint
 *		bhkBreakableConstraint
 *		bhkWheelConstraint
 *		bhkStiffSpringConstraint
 *		bhkRagdollConstraint
 *		bhkPrismaticConstraint
 *		bhkHingeConstraint
 *		bhkBallAndSocketConstraint
 *		bhkGenericConstraint
 *			bhkFixedConstraint
 *		bhkPointToPathConstraint - unref'd
 *		bhkPoweredHingeConstraint - unref'd
 */

#include "obse/NiHavok.h"

class hkEntity;

// 010
class bhkConstraint : public bhkSerializable
{
public:
	bhkConstraint();
	~bhkConstraint();

	// hkObj is a hkConstraintInstance

	virtual void	SetEntity1(hkEntity * arg);	// update hkObj->entities[0]
	virtual void	SetEntity2(hkEntity * arg);	// update hkObj->entities[1]
	virtual hkEntity *	GetEntity1(void);		// get hkObj->entities[0]
	virtual hkEntity *	GetEntity2(void);		// get hkObj->entities[1]
	virtual bool	Unk_Fn24(void * arg1, UInt32 arg2);
};

// 010
class bhkLimitedHingeConstraint : public bhkConstraint
{
public:
	bhkLimitedHingeConstraint();
	~bhkLimitedHingeConstraint();
};

// 010
class bhkMalleableConstraint : public bhkConstraint
{
public:
	bhkMalleableConstraint();
	~bhkMalleableConstraint();
};

// 010
class bhkBreakableConstraint : public bhkConstraint
{
public:
	bhkBreakableConstraint();
	~bhkBreakableConstraint();
};

// 010
class bhkWheelConstraint : public bhkConstraint
{
public:
	bhkWheelConstraint();
	~bhkWheelConstraint();
};

// 010
class bhkStiffSpringConstraint : public bhkConstraint
{
public:
	bhkStiffSpringConstraint();
	~bhkStiffSpringConstraint();
};

// 010
class bhkRagdollConstraint : public bhkConstraint
{
public:
	bhkRagdollConstraint();
	~bhkRagdollConstraint();
};

// 010
class bhkPrismaticConstraint : public bhkConstraint
{
public:
	bhkPrismaticConstraint();
	~bhkPrismaticConstraint();
};

// 010
class bhkHingeConstraint : public bhkConstraint
{
public:
	bhkHingeConstraint();
	~bhkHingeConstraint();
};

// 010
class bhkBallAndSocketConstraint : public bhkConstraint
{
public:
	bhkBallAndSocketConstraint();
	~bhkBallAndSocketConstraint();
};

// 010
class bhkGenericConstraint : public bhkConstraint
{
public:
	bhkGenericConstraint();
	~bhkGenericConstraint();
};

// bhkPointToPathConstraint - unref'd
// bhkPoweredHingeConstraint - unref'd
