#pragma once

/**** havok world objects
 *	bhkWorldObject
 *		bhkPhantom
 *			bhkShapePhantom
 *				bhkSimpleShapePhantom
 *				bhkCachingShapePhantom
 *			bhkAabbPhantom
 *				bhkAvoidBox
 *		bhkEntity
 *			bhkRigidBody
 *				bhkRigidBodyT
 */

#include "obse/NiHavok.h"

// 010
class bhkWorldObject : public bhkSerializable
{
public:
	bhkWorldObject();
	~bhkWorldObject();

	virtual void		Unk_20(void) = 0;
	virtual void		Unk_21(void);
	virtual NiNode *	Unk_22(NiNode * node);
};

// 014
class bhkPhantom : public bhkWorldObject
{
public:
	bhkPhantom();
	~bhkPhantom();

	UInt8	unk010;		// 010
	UInt8	pad011[3];	// 011
};

// 014
class bhkShapePhantom : public bhkPhantom
{
public:
	bhkShapePhantom();
	~bhkShapePhantom();
};

// 014
class bhkSimpleShapePhantom : public bhkShapePhantom
{
public:
	bhkSimpleShapePhantom();
	~bhkSimpleShapePhantom();
};

// 014
class bhkCachingShapePhantom : public bhkShapePhantom
{
public:
	bhkCachingShapePhantom();
	~bhkCachingShapePhantom();
};

// 014
class bhkAabbPhantom : public bhkPhantom
{
public:
	bhkAabbPhantom();
	~bhkAabbPhantom();
};

// 014
class bhkAvoidBox : public bhkAabbPhantom
{
public:
	bhkAvoidBox();
	~bhkAvoidBox();
};

// 010
class bhkEntity : public bhkWorldObject
{
public:
	bhkEntity();
	~bhkEntity();
};

// 01C
class bhkRigidBody : public bhkEntity
{
public:
	bhkRigidBody();
	~bhkRigidBody();

	struct Node
	{
		NiRefObject	* obj;
		Node		* next;
	};

	Node		listRoot;	// 010
	void		* unk018;	// 018
};

// 050
class bhkRigidBodyT : public bhkRigidBody
{
public:
	bhkRigidBodyT();
	~bhkRigidBodyT();

	virtual void	Unk_23(hkVector4 * arg);
	virtual void	Unk_24(hkVector4 * arg);
	virtual void	Unk_25(hkVector4 * arg);
	virtual void	Unk_26(hkVector4 * arg);
	virtual void	Unk_27(UInt32 arg);
	virtual void	Unk_28(hkVector4 * arg1, hkVector4 * arg2);
	virtual void	Unk_29(hkVector4 * arg);
	virtual void	Unk_2A(hkVector4 * arg);
	virtual void	Unk_2B(hkTransform * arg);	// arg type may be wrong
	virtual void	Unk_2C(hkVector4 * arg);

	UInt32			pad01C;		// 01C
	hkQuaternion	localRot;	// 020
	hkVector4		localPos;	// 030
	UInt32			unk040[4];	// 040
};
