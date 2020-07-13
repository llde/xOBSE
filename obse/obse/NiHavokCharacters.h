#pragma once

/**** havok character control classes
 *	bhkCharacterProxy
 *		bhkCharacterListenerArrow - no NiRTTI
 *		bhkCharacterListenerSpell - no NiRTTI
 *		bhkCharacterController - no NiRTTI
 */

#include "obse/NiHavok.h"
#include "obse/HavokCollision.h"
#include "obse/HavokCharacters.h"

// 1D0
class bhkCharacterPointCollector : public hkAllCdPointCollector
{	
public:
	bhkCharacterPointCollector();
	~bhkCharacterPointCollector();

	UInt32	unk1A0[(0x1D0 - 0x1A0) >> 2];	// 1A0
};

STATIC_ASSERT(sizeof(bhkCharacterPointCollector) == 0x1D0);

// 1E0
class bhkCharacterProxy : public bhkRefObject
{
public:
	bhkCharacterProxy();
	~bhkCharacterProxy();

	virtual void Unk_15(void);
	virtual void Unk_16(void);
	virtual void Unk_17(void);
	virtual void Unk_18(void);
	virtual void Unk_19(void);
	virtual void Unk_1A(void);
	virtual void Unk_1B(void);
	virtual void Unk_1C(void);
	virtual void Unk_1D(void);
	virtual void Unk_1E(void);
	virtual void Unk_1F(void);

	//ahkCharacterProxy	*						// 008
	UInt32						unk00C;			// 00C
	bhkCharacterPointCollector	pointCollecter;	// 010
};

STATIC_ASSERT(sizeof(bhkCharacterProxy) == 0x1E0);

// 68+
class bhkCharacterListener : public hkCharacterProxyListener
{
public:
	bhkCharacterListener();
	~bhkCharacterListener();

	UInt32				flags004;						// 004 if either bit 8 or 9 set, no fall damage
	UInt32				unk008[(0x024 - 0x008) >> 2];	// 008
	UInt32				groundSurfaceMaterial;			// 24 one of bhkShape::kMaterial_XXX
	UInt32				unk028[(0x068 - 0x028) >> 2];	// 28
};

STATIC_ASSERT(sizeof(bhkCharacterListener) == 0x68);

//3E0
class bhkCharacterController : public bhkCharacterProxy
{
public:
	bhkCharacterController();
	~bhkCharacterController();

	// bases
	hkCharacterContext		ctx;							// 1E0
	bhkCharacterListener	listener;						// 1F0

	UInt32					unk258[(0x2D8 - 0x258) >> 2];	// 258
	float					timeDelta2D8;					// 2D8 added to timeInAir each tick while in air
															//     and to fallDamageTimer after timeInAir exceeds threshold for damage
															//     possibly a general time delta not specific to hkState_InAir
	UInt32					unk2DC;							// 2DC
	float					velX, velY, velZ;				// 2E0 These are updated each tick from ahkCharacterController.
															//     Modifying them directly generally won't produce expected results.
	UInt32					unk2EC[(0x320 - 0x2EC) >> 2];	// 2F0
	float					fallDamageTimer;				// 320
	float					timeInAir;						// 324
	UInt32					unk328[(0x3E0 - 0x328) >> 2];	// 328
};

STATIC_ASSERT(offsetof(bhkCharacterController, ctx) == 0x1E0);
STATIC_ASSERT(offsetof(bhkCharacterController, listener) == 0x1F0);
STATIC_ASSERT(offsetof(bhkCharacterController, unk258) == 0x258);
STATIC_ASSERT(offsetof(bhkCharacterController, fallDamageTimer) == 0x320);
STATIC_ASSERT(offsetof(bhkCharacterController, timeInAir) == 0x324);
STATIC_ASSERT(sizeof(bhkCharacterController) == 0x3E0);
