#pragma once

#include "obse/HavokTypes.h"

class hkShape;

// 04
class hkBroadPhaseHandle
{
public:
	hkBroadPhaseHandle();
	~hkBroadPhaseHandle();

	UInt32	m_id;		// 00
};

// 0C
class hkTypedBroadPhaseHandle : public hkBroadPhaseHandle
{
public:
	hkTypedBroadPhaseHandle();
	~hkTypedBroadPhaseHandle();

	SInt8	m_type;					// 04
	SInt8	m_ownerOffset;			// 05
	UInt16	m_objectQualityType;	// 06
	UInt32	m_collisionFilterInfo;	// 08
};

// 10
class hkCdBody
{
public:
	hkShape		* m_shape;	// 00
	UInt32		m_shapeKey;	// 04
	void		* m_motion;	// 08 - either hkTransform or hkMotionState
	hkCdBody	* m_parent;	// 0C
};

// 24
class hkCollidable : public hkCdBody
{
public:
	UInt8					m_ownerOffset;				// 10
	UInt8					pad11[3];					// 11
	hkTypedBroadPhaseHandle	m_broadPhaseHandle;			// 14
	float					m_allowedPenetrationDepth;	// 20
};

// 28
struct hkCdPoint
{
	hkContactPoint	m_contact;		// 00
	hkCdBody		* m_cdBodyA;	// 20
	hkCdBody		* m_cdBodyB;	// 24
};

// 30
struct hkRootCdPoint
{
	hkContactPoint	m_contact;				// 00
	hkCollidable	* m_rootCollidableA;	// 20
	hkShapeKey		m_shapeKeyA;			// 24
	hkCollidable	* m_rootCollidableB;	// 28
	hkShapeKey		m_shapeKeyB;			// 2C
};

// 08
class hkCdPointCollector
{
public:
	hkCdPointCollector();
	~hkCdPointCollector();

	virtual void	Destroy(bool freeMem);
	virtual void	AddCdPoint(const hkCdPoint & point) = 0;

//	void	** m_vtbl;				// 00
	float	m_earlyOutDistance;		// 04
};

// 1A0
class hkAllCdPointCollector : public hkCdPointCollector
{
public:
	hkAllCdPointCollector();
	~hkAllCdPointCollector();

	virtual void	AddCdPoint(const hkCdPoint & point);	// overridden here (mainly so we can STATIC_ASSERT)

	hkInplaceArray <hkRootCdPoint, 8>	m_hits;	// 08 (data at 20 due to alignment)
};

STATIC_ASSERT(sizeof(hkAllCdPointCollector) == 0x1A0);

class hkWorldRayCastInput
{
public:
	hkWorldRayCastInput()	{ }
	~hkWorldRayCastInput()	{ }

	hkVector4	m_from;
	hkVector4	m_to;
	bool		m_enableShapeCollectionFilter;
	UInt32		m_filterInfo;
};

class hkShapeRayCastOutput
{
public:
	hkShapeRayCastOutput()	:m_hitFraction(1) { }
	~hkShapeRayCastOutput()	{ }

	hkVector4	m_normal;
	UInt32		m_shapeKey;
	float		m_hitFraction;
};

class hkWorldRayCastOutput : public hkShapeRayCastOutput
{
public:
	hkWorldRayCastOutput()	{ }
	~hkWorldRayCastOutput()	{ }

	hkCollidable	* m_rootCollidable;
};
