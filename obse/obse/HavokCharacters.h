#pragma once

#include "obse/HavokBase.h"
#include "obse/HavokCollision.h"
#include "obse/HavokDynamics.h"

class hkCharacterProxy;
class hkCharacterState;

enum // hkCharacterStateType
{
	kHKState_OnGround = 0,
	kHKState_Jumping,
	kHKState_InAir,
	kHKState_Climbing,
	kHKState_Flying,
	kHKState_Swimming,
	kHKState_Projectile,	// user state 0
	kHKState_UserState1,
	kHKState_UserState2,	// 8
	kHKState_UserState3,
	kHKState_UserState4,
	kHKState_UserState5,

	kHKState_Max			// C
};

// 3C
class hkCharacterStateManager : public hkReferencedObject
{
public:
	hkCharacterStateManager();
	~hkCharacterStateManager();

	hkCharacterState	* m_registeredState[kHKState_Max];
};

// 10
class hkCharacterContext : public hkReferencedObject
{
public:
	hkCharacterStateManager	* stateMgr;	// 08
	UInt32					hkState;	// 0C - hkCharacterStateType

	bool IsOnGround()
		{	return (hkState == kHKState_OnGround);	}
	bool IsJumping()
		{	return (hkState == kHKState_Jumping);	}
	bool IsInAir()
		{	return (hkState == kHKState_InAir);	}
	bool IsFlying()
		{	return (hkState == kHKState_Flying);	}
};

// 4
class hkCharacterProxyListener
{
	virtual void	Destroy(bool freeMem);
	virtual void	ProcessConstraintsCallback(void * arg0, void * arg1);
	virtual void	ContactPointAddedCallback(hkRootCdPoint * point);
	virtual void	ContactPointRemovedCallback(hkRootCdPoint * point);
	virtual void	CharacterInteractionCallback(hkCharacterProxy * proxy, hkCharacterProxy * otherProxy, hkContactPoint * contact);
	virtual void	ObjectInteractionCallback(hkCharacterProxy * proxy, void * arg1, void * arg2);

//	void	** _vtbl;	// 0
};

// B0
class hkCharacterProxy : public hkReferencedObject
{
public:
	virtual void Unk_02(UInt32 arg0) = 0;
	virtual void Unk_03(UInt32 arg0, UInt32 arg1, UInt32 arg2) = 0;

	hkCharacterProxy();
	~hkCharacterProxy();

	// bases
	hkEntityListener		entityListener;				// 08
	hkPhantomListener		phantomListener;			// 0C

	hkVector4				velocity;					// 10
	hkVector4				unk20;						// 20 pos?
	void					* unk30;					// 30
	UInt32					unk34;						// 34
	UInt32					unk38;						// 38
	UInt32					unk3C;						// 3C
	hkVector4				unk40;						// 40
	UInt32					unk50[(0x74 - 0x50) >> 2];	// 50
	float					unk74[3][4];				// 74 each init'ed to { 0.0, 0.0, -0.0 }
	float					unkA4;						// A4 cosine of some angle passed in c'tor arg
	UInt32					unkA8;						// A8
	UInt32					unkAC;						// AC
};

// C0
class ahkCharacterProxy : public hkCharacterProxy
{
public:
	ahkCharacterProxy();
	~ahkCharacterProxy();

	UInt32		unkBO[4];			// B0
};

STATIC_ASSERT(sizeof(ahkCharacterProxy) == 0xC0);
