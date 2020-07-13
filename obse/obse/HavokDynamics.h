#pragma once

#include "HavokBase.h"
#include "HavokTypes.h"
#include "Utilities.h"

class hkSimulationIsland;
class hkRigidBody;
class hkPhantom;
class hkSimulation;
class hkWorldMaintenanceManager;
class hkWorldOperationQueue;
class hkBroadPhase;
class hkTypedBroadPhaseDispatcher;
class hkEntityEntityBroadPhaseListener;
class hkProcessCollisionInput;
class hkPhantomBroadPhaseListener;
class hkEntityListener;
class hkPhantomListener;
class hkConstraintListener;
class hkWorldDeletionListener;
class hkIslandActivationListener;
class hkWorldPostSimulationListener;
class hkWorldPostIntegrateListener;
class hkWorldPostCollideListener;
class hkIslandPostIntegrateListener;
class hkIslandPostCollideListener;
class hkCollisionListener;
class hkCollisionFilter;
class hkCollisionDispatcher;
class hkActionListener;
class hkWorldMemoryAvailableWatchDog;
class hkWorldRayCastInput;
class hkWorldRayCastOutput;

class hkWorldCinfo : public hkReferencedObject
{
public:
	UInt32		m_pad08[(0x10 - 0x08) >> 2];	// 08
	hkVector4	m_gravity;						// 10
};

// 2C0?
class hkWorld : public hkReferencedObject
{
public:
	// we probably don't want to mess around with these directly, but it's good to know
	// for reading other code that accesses hkWorld

	hkSimulation						* m_simulation;					// 008
	UInt32								m_unk00C[(0x020 - 0x00C) >> 2];	// 00C

	hkVector4							m_gravity;						// 020
	hkSimulationIsland					* m_fixedIsland;				// 030
	hkRigidBody							* m_fixedRigidBody;				// 034

	hkArray <hkSimulationIsland *>		m_activeSimulationIslands;		// 038
	hkArray <hkSimulationIsland *>		m_inactiveSimulationIslands;	// 044
	hkArray <hkSimulationIsland *>		m_dirtySimulationIslands;		// 050
	hkWorldMaintenanceManager			* m_maintenanceMgr;				// 05C
	hkWorldMemoryAvailableWatchDog		* m_memoryWatchDog;				// 060 - unconfirmed
	hkBroadPhase						* m_broadPhase;					// 064
	hkTypedBroadPhaseDispatcher			* m_broadPhaseDispatcher;		// 068
	hkPhantomBroadPhaseListener			* m_phantomBroadPhaseListener;	// 06C
	hkEntityEntityBroadPhaseListener	* m_entityEntityBroadPhaseListener;	// 070
																		// no m_multithreadedSimulationJobData
	hkProcessCollisionInput				* m_collisionInput;				// 074
	hkCollisionFilter					* m_collisionFilter;			// 078
	hkCollisionDispatcher				* m_collisionDispatcher;		// 07C
	hkWorldOperationQueue				* m_pendingOperations;			// 080
	int									m_pendingOperationsCount;		// 084 - unconfirmed
	int									m_lockCount;					// 088
	int									m_lockCountForPhantoms;			// 08C
	UInt8								m_blockExecutingPendingOperations;	// 090
	UInt8								m_criticalOperationsAllowed;	// 091
	UInt8								m_pad092[2];					// 092
	UInt32								m_unk094[(0x0A0 - 0x094) >> 2];	// 094

	CRITICAL_SECTION					* m_unk0A0;						// 0A0
	UInt8								m_unk0A4;						// 0A4
	UInt8								m_unk0A5[3];					// 0A5
	UInt32								m_unk0A8[(0x0B8 - 0x0A8) >> 2];	// 0A8

	hkArray <hkPhantom *>				m_phantoms;						// 0B8
	hkArray <hkActionListener *>		m_actionListeners;				// 0C4
	hkArray <hkEntityListener *>		m_entityListeners;				// 0D0
	hkArray <hkPhantomListener *>		m_phantomListeners;				// 0DC
	hkArray <hkConstraintListener *>	m_constraintListeners;			// 0E8
	hkArray <hkWorldDeletionListener *>	m_worldDeletionListeners;		// 0F4
	hkArray <hkIslandActivationListener *>
										m_islandActivationListeners;	// 100
	hkArray <hkWorldPostSimulationListener *>
										m_worldPostSimulationListeners;	// 10C
	hkArray <hkWorldPostIntegrateListener *>
										m_worldPostIntegrateListeners;	// 118
	hkArray <hkWorldPostCollideListener *>
										m_worldPostCollideListeners;	// 124
	hkArray <hkIslandPostIntegrateListener *>
										m_islandPostIntegrateListeners;	// 130
	hkArray <hkIslandPostCollideListener *>
										m_islandPostCollideListeners;	// 13C - unconfirmed
	hkArray <hkCollisionListener *>		m_collisionListeners;			// 148
	hkReferencedObject					* m_unk154;						// 154

	MEMBER_FN_PREFIX(hkWorld);
	DEFINE_MEMBER_FN(castRay, void, 0x008987E0, const hkWorldRayCastInput * input, hkWorldRayCastOutput * output);
};

STATIC_ASSERT(sizeof(CRITICAL_SECTION) == 0x18);
STATIC_ASSERT(offsetof(hkWorld, m_phantoms) == 0x0B8);
STATIC_ASSERT(offsetof(hkWorld, m_collisionListeners) == 0x148);

class hkWorldObject : public hkReferencedObject
{
public:
	//
};

class hkPhantomListener
{
public:
	virtual void * Destroy(bool bFreeMem);
	virtual UInt32 Unk_01(UInt32 arg0);
	virtual UInt32 Unk_02(UInt32 arg0);
	virtual UInt32 Unk_03(UInt32 arg0);
	virtual UInt32 Unk_04(UInt32 arg0);

	hkPhantomListener();
	~hkPhantomListener();

	// void ** vtbl
};

class hkEntityListener
{
public:
	virtual void * Destroy(bool bFreeMem);
	virtual UInt32 Unk_01(UInt32 arg0);
	virtual UInt32 Unk_02(UInt32 arg0);
	virtual UInt32 Unk_03(UInt32 arg0);
	virtual UInt32 Unk_04(UInt32 arg0);

	hkEntityListener();
	~hkEntityListener();

	// void ** vtbl
};
