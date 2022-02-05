#include "Commands_MiscReference.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION
#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameProcess.h"
#include "GameTasks.h"
#include "InternalSerialization.h"
#include "Hooks_Script.h"
#include "Hooks_Gameplay.h"
#include "StringVar.h"
#include "ArrayVar.h"
#include "NiExtraData.h"
#include "InventoryReference.h"
#include "NiObjects.h"
#include "obse_common/SafeWrite.h"

// link two persistent door refs, add to LowPathWorld. Fails if either already has ExtraTeleport
typedef void (__cdecl * _LinkDoors)(TESObjectREFR* door1, TESObjectREFR* door2);

// Remove ExtraTeleport from door and its linked door, and removes from LowPathWorld.
typedef void (__cdecl * _RemoveExtraTeleportFromDoorRef)(TESObjectREFR* doorRef);

static const _LinkDoors LinkDoors = (_LinkDoors)0x004B80E0;
static const _RemoveExtraTeleportFromDoorRef RemoveExtraTeleportFromDoorRef = (_RemoveExtraTeleportFromDoorRef)0x004B6D50;
static LowPathWorld** g_LowPathWorld = (LowPathWorld**)0x00B3BE00;


static const _Cmd_Execute Cmd_Activate_Execute = (_Cmd_Execute)0x00507650;

// global pathing objects, multi-threaded pathing system, make life easier.
class LowPathWorldPtr
{
public:
	LowPathWorldPtr() { EnterCriticalSection (g_pathingMutex); }
	~LowPathWorldPtr() {LeaveCriticalSection (g_pathingMutex); }

	LowPathWorld* operator->() { return *g_LowPathWorld; }
};

static bool Cmd_GetTravelHorse_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32*)result;
	if (!thisObj) return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_TravelHorse);
	if (xData) {
		ExtraTravelHorse* xHorse = (ExtraTravelHorse*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraTravelHorse, 0);
		if (xHorse) {
			*refResult = xHorse->horseRef->refID;
		}
	}
	return true;
}

static bool Cmd_SetTravelHorse_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj) return true;
	TESObjectREFR* objectRef = NULL;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &objectRef))
		return true;
	if (!thisObj || thisObj->typeID != kFormType_ACHR) return true;	//only NPCs may have travel horses

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_TravelHorse);

	if (xData) {
		ExtraTravelHorse* xHorse = (ExtraTravelHorse*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraTravelHorse, 0);
		if (xHorse) {
			*refResult = xHorse->horseRef->refID;
			xHorse->horseRef = objectRef;
		}
	}
	else		//add
	{
		ExtraTravelHorse* xHorse = ExtraTravelHorse::Create();
		xHorse->horseRef = objectRef;
		thisObj->baseExtraList.Add(xHorse);
	}

	return true;
}

static bool Cmd_GetOpenKey_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj)	return true;
	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Lock);
	if (!xData)
		return true;

	ExtraLock* xLock = (ExtraLock*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraLock, 0);
	if (xLock && xLock->data && xLock->data->key)
		*refResult = xLock->data->key->refID;

	return true;
}

static bool Cmd_SetOpenKey_Execute(COMMAND_ARGS)
{
	TESForm* form;
	*result = 0;

	if (!thisObj)
		return true;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	if (!form)
		return true;

	TESKey* key = (TESKey*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESKey, 0);
	if (!key)
		return true;

	ExtraLock* xLock = NULL;
	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Lock);
	if (!xData)			//create and add to baseExtraList
	{
		xLock = ExtraLock::Create();
		if (!thisObj->baseExtraList.Add(xLock))
		{
			FormHeap_Free(xLock->data);
			FormHeap_Free(xLock);
			return true;
		}
	}
	else
		xLock = (ExtraLock*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraLock, 0);

	if (xLock)
	{
		xLock->data->key = key;
		*result = 1;
	}

	return true;
}

static TESForm* GetOwner(BaseExtraList* xDataList)
{
	BSExtraData* xData = xDataList->GetByType(kExtraData_Ownership);
	ExtraOwnership* xOwner = NULL;
	TESForm* owner = NULL;

	if (xData)
	{
		xOwner = (ExtraOwnership*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraOwnership, 0);
		if (xOwner)
			owner = xOwner->owner;
	}

	return owner;
}

static UInt32 GetOwningFactionRequiredRank(BaseExtraList* xDataList)
{
	BSExtraData* xData = xDataList->GetByType(kExtraData_Rank);
	if (xData)
	{
		ExtraRank* xRank = (ExtraRank*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraRank, 0);
		if (xRank)
			return xRank->rank;
	}

	return 0;
}

static SInt32 SetOwningFactionRequiredRank(BaseExtraList* xDataList, UInt32 rank)
{
	BSExtraData* xData = xDataList->GetByType(kExtraData_Rank);
    TESFaction* fact = NULL;
	if (xData)
	{
		ExtraRank* xRank = (ExtraRank*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraRank, 0);
		if (xRank){
            UInt32 old_Rank = xRank->rank;
            xRank->rank = rank;
			return old_Rank;
        }
	}
	else{
        xData = xDataList->GetByType(kExtraData_Ownership);
        if (xData){
            xOwner = (ExtraOwnership*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraOwnership, 0);
            if (xOwner){
                fact = (TESFaction*) Oblivion_DynamicCast(xOwner->owner, 0, RTTI_TESForm, RTTI_TESFaction, 0);
            }
        }
        if(fact){
            ExtraRank* newRank = ExtraRank::Create(rank);
            xDataList->Add(newRank);
        }
    }

	return -1;
}

static bool Cmd_GetParentCellOwner_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj)
		return true;
    if(!thisObj->parentCell) return true;
	TESForm* owner = GetOwner(&(thisObj->parentCell->extraData));
	if (owner)
		*refResult = owner->refID;

	return true;
}

static bool Cmd_GetOwner_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj)
		return true;

	TESForm* owner = GetOwner(&(thisObj->baseExtraList));
	if (owner)
		*refResult = owner->refID;

	return true;
}

static bool Cmd_GetOwningFactionRequiredRank_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj)
		return true;

	*result = GetOwningFactionRequiredRank(&(thisObj->baseExtraList));

	return true;
}

static bool Cmd_GetParentCellOwningFactionRequiredRank_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj)
		return true;

	*result = GetOwningFactionRequiredRank(&(thisObj->parentCell->extraData));

	return true;
}

static bool Cmd_SetOwningFactionRequiredRank_Execute(COMMAND_ARGS)
{
	*result = 0;
    UInt32 rank = 0;
	if (!thisObj)
		return true;
    if(!ExtractArgs(PASS_EXTRACT_ARGS, &rank) ) return true;

	*result = SetOwningFactionRequiredRank(&(thisObj->baseExtraList), rank);

	return true;
}

static bool Cmd_SetParentCellOwningFactionRequiredRank_Execute(COMMAND_ARGS)
{
	*result = 0;
    UInt32 rank = 0;
	if (!thisObj)
		return true;

    if(!ExtractArgs(PASS_EXTRACT_ARGS, &rank) ) return true;

	*result = SetOwningFactionRequiredRank(&(thisObj->parentCell->extraData), rank);

	return true;
}

static SInt8 IsOffLimits(BaseExtraList* xDataList, TESNPC* actor)
{
	SInt8 offLimits = -1;					//return -1 if ownership is ambiguous

	BSExtraData* xData;
	TESForm* owner = GetOwner(xDataList);
	if (owner)
	{
		if (owner->typeID == kFormType_NPC)
		{
			if (owner->refID == actor->refID)			//owned by this actor
				offLimits = 0;
			else
			{
				xData = xDataList->GetByType(kExtraData_Global);
				if (xData)
				{
					ExtraGlobal* global = (ExtraGlobal*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraGlobal, 0);
					if (global->globalVar->data)
						offLimits = 0;
					else
						offLimits = 1;
				}
				else
					offLimits = 1;
			}
		}
		else if (owner->typeID == kFormType_Faction)
		{
			TESFaction* owningFaction = (TESFaction*)Oblivion_DynamicCast(owner, 0, RTTI_TESForm, RTTI_TESFaction, 0);
			if (owningFaction && !(owningFaction->IsEvil()))		//no crime to steal from evil factions
			{
				xData = xDataList->GetByType(kExtraData_Rank);
				SInt8 reqRank = 0;
				if (xData)					// ExtraRank only present if required rank > 0
				{
					ExtraRank* rank = (ExtraRank*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraRank, 0);
					reqRank = rank->rank;
				}
				if (actor->actorBaseData.GetFactionRank((TESFaction*)owner) >= reqRank)
					offLimits = 0;
				else
					offLimits = 1;
			}
		}
	}
	return offLimits;
}

static bool Cmd_IsOffLimits_Execute(COMMAND_ARGS)
{
	TESNPC* actor = NULL;
	*result = 0;

	if (!thisObj)
		return true;
	else if (!ExtractArgs(PASS_EXTRACT_ARGS, &actor))
		return true;

	if (!actor || actor == (*g_thePlayer)->baseForm)	// if actor arg omitted use player
	{
		// let the game do the work if it's the player
		*result = CALL_MEMBER_FN(thisObj, IsOffLimitsToPlayer)() ? 1 : 0;
		return true;
	}

	TESObjectREFR* refObj = thisObj;
	if (refObj->parentCell && !refObj->parentCell->IsInterior() && refObj->baseForm->typeID == kFormType_Door)
	{
		// ownership data for doors in exteriors stored on linked door
		BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Teleport);
		if (xData)
		{
			ExtraTeleport* xTele = (ExtraTeleport*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraTeleport, 0);
			if (xTele)
				refObj = xTele->data->linkedDoor;
		}
	}

	if (!refObj)
		return true;

	SInt8 offLimits = IsOffLimits(&refObj->baseExtraList, actor);
	if (offLimits != -1)
		*result = offLimits;
	else
	{
		offLimits = IsOffLimits(&refObj->parentCell->extraData, actor);
		if (offLimits == 1)
			*result = 1;
	}

	return true;
}

static bool Cmd_IsLoadDoor_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj)
		return true;

	if (thisObj->baseExtraList.GetByType(kExtraData_Teleport) ||
		thisObj->baseExtraList.GetByType(kExtraData_RandomTeleportMarker))
		*result = 1;

	return true;
}

static bool Cmd_GetLinkedDoor_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Teleport);
	if (xData)
	{
		ExtraTeleport* xTele = (ExtraTeleport*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraTeleport, 0);
		if (xTele && xTele->data && xTele->data->linkedDoor)
			*refResult = xTele->data->linkedDoor->refID;
	}

	return true;
}

static bool Cmd_GetTeleportCell_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Teleport);
	if (xData)
	{
		ExtraTeleport* xTele = (ExtraTeleport*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraTeleport, 0);

		// parentCell will be null if linked door's cell is not currently loaded (e.g. most exterior cells)
		if (xTele && xTele->data && xTele->data->linkedDoor && xTele->data->linkedDoor->parentCell)
			*refResult = xTele->data->linkedDoor->parentCell->refID;
	}

	return true;
}

enum {
	kTeleport_X,
	kTeleport_Y,
	kTeleport_Z,
	kTeleport_Rot,
};

static const float kRadToDegree = 57.29577951f;
static const float kDegreeToRad = 0.01745329252f;

static bool GetTeleportInfo(COMMAND_ARGS, UInt32 which)
{
	*result = 0;

	if (!thisObj || thisObj->baseForm->typeID != kFormType_Door)
		return true;

	ExtraTeleport* tele = (ExtraTeleport*)thisObj->baseExtraList.GetByType(kExtraData_Teleport);
	if (tele && tele->data)
	{
		switch (which)
		{
		case kTeleport_X:
			*result = tele->data->x;
			break;
		case kTeleport_Y:
			*result = tele->data->y;
			break;
		case kTeleport_Z:
			*result = tele->data->z;
			break;
		case kTeleport_Rot:
			*result = tele->data->zRot * kRadToDegree;
			break;
		}
	}

	return true;
}

static bool Cmd_GetDoorTeleportX_Execute(COMMAND_ARGS)
{
	return GetTeleportInfo(PASS_COMMAND_ARGS, kTeleport_X);
}

static bool Cmd_GetDoorTeleportY_Execute(COMMAND_ARGS)
{
	return GetTeleportInfo(PASS_COMMAND_ARGS, kTeleport_Y);
}

static bool Cmd_GetDoorTeleportZ_Execute(COMMAND_ARGS)
{
	return GetTeleportInfo(PASS_COMMAND_ARGS, kTeleport_Z);
}

static bool Cmd_GetDoorTeleportRot_Execute(COMMAND_ARGS)
{
	return GetTeleportInfo(PASS_COMMAND_ARGS, kTeleport_Rot);
}

static bool Cmd_SetDoorTeleport_Execute(COMMAND_ARGS)
{
	// linkedDoor x y z (rot). if omitted, coords/rot taken from linked ref

	*result = 0;
	if (!thisObj || thisObj->baseForm->typeID != kFormType_Door)
		return true;

	TESObjectREFR* linkedDoor = NULL;
	float x = 999;
	float y = 999;
	float z = 999;
	float rot = 999;
	UInt32 bTemp = 0;	// script passes 1 to prevent change being saved in savegame

	if (thisObj->baseExtraList.HasType(kExtraData_RandomTeleportMarker))
		return true;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &linkedDoor, &x, &y, &z, &rot, &bTemp) && linkedDoor && linkedDoor->IsPersistent())	// ###TODO: necessary for linkedref to be door?
	{
		ExtraTeleport* tele = (ExtraTeleport*)thisObj->baseExtraList.GetByType(kExtraData_Teleport);
		if (!tele)
		{
			tele = ExtraTeleport::Create();
			thisObj->baseExtraList.Add(tele);
		}

		tele->data->linkedDoor = linkedDoor;
		if (x == 999 && y == 999 && z == 999)
		{
			x = linkedDoor->posX;
			y = linkedDoor->posY;
			z = linkedDoor->posZ;
		}

		if (rot == 999)
			rot = linkedDoor->rotZ;
		else
			rot *= kDegreeToRad;

		tele->data->x = x;
		tele->data->y = y;
		tele->data->z = z;
		tele->data->zRot = rot;

		// save the changes
		if (!bTemp) {
			thisObj->MarkAsModified(TESObjectREFR::kChanged_DoorExtraTeleport);
		}

		*result = 1;
	}

	return true;
}

static bool Cmd_LinkToDoor_Execute(COMMAND_ARGS)
{
	TESObjectREFR* door1 = thisObj, * door2 = NULL;

	// avoid very bad things
	if (ExtractArgs(PASS_EXTRACT_ARGS, &door2)
		&& door1 && door2
		&& door1 != door2
		&& door1->IsPersistent()
		&& door2->IsPersistent()
		&& OBLIVION_CAST(door1->baseForm, TESForm, TESObjectDOOR)
		&& OBLIVION_CAST(door2->baseForm, TESForm, TESObjectDOOR)
		)
	{
		ExtraTeleport::Data* data1 = door1->GetExtraTeleportData();
		ExtraTeleport::Data* data2 = door2->GetExtraTeleportData();

		// ###TODO: the #if-ed out stuff below works, mostly. Problem is that the game ignores a CHANGE_EXTRA_TELEPORT flag if the teleport was removed
		// So after changing the destination of a door, exiting, and reloading, the door previously linked to that door will once again be linked to it
		// For the time being, only allow linking two doors which have no ExtraTeleport already
		if (data1 || data2)
			return true;
#if 0
		if (data1 && data2 && data1->linkedDoor == door1 && data2->linkedDoor == door2)
		{
			// doors are already linked.
			*result = 1.0;
			return true;
		}

		// try to preserve the positioning of the actor when coming through the door, if an ExtraTeleport already exists
		Vector3 pos1, pos2;
		float rot1, rot2;

		if (data1)
		{
			data1->linkedDoor->MarkAsModified (TESObjectREFR::kChanged_DoorExtraTeleport);
			data1 = data1->linkedDoor->GetExtraTeleportData();
			pos2 = Vector3 (data1->x, data1->y, data1->z);
			rot2 = data1->zRot;
			RemoveExtraTeleportFromDoorRef (door1);
		}

		if (data2)
		{
			data2->linkedDoor->MarkAsModified (TESObjectREFR::kChanged_DoorExtraTeleport);
			data2 = data2->linkedDoor->GetExtraTeleportData();
			pos1 = Vector3 (data2->x, data2->y, data2->z);
			rot1 = data2->zRot;
			RemoveExtraTeleportFromDoorRef (door2);
		}
#endif

		LinkDoors (door1, door2);

#if 0
		if (data1)
		{
			data1 = door1->GetExtraTeleportData();
			data1->x = pos1.x;
			data1->y = pos1.y;
			data1->z = pos1.z;
			data1->zRot = rot1;
		}

		if (data2)
		{
			data2 = door2->GetExtraTeleportData();
			data2->x = pos2.x;
			data2->y = pos2.y;
			data2->z = pos2.z;
			data2->zRot = rot2;
		}
#endif

		*result = 1.0;
	}

	return true;
}

struct CellScanInfo
{
	const	TESObjectCELL::ObjectListEntry *	prev;	//last ref returned to script
	const	TESObjectCELL * curCell;					//cell currently being scanned
	const	TESObjectCELL * cell;						//player's current cell
	const	TESWorldSpace * world;
	SInt32	curX;										//offsets of curCell from player's cell
	SInt32	curY;
	UInt8	formType;									//form type to scan for
	UInt8	cellDepth;									//depth of adjacent cells to scan
	bool	includeTakenRefs;
	bool	includeDeletedRefs;

	CellScanInfo()
	{	}

	CellScanInfo(UInt8 _cellDepth, UInt8 _formType, bool _includeTaken, bool _includeDeleted ,TESObjectCELL* _cell)
					:	cellDepth(_cellDepth), formType(_formType), includeTakenRefs(_includeTaken), includeDeletedRefs(_includeDeleted) , prev(NULL), cell(_cell)
	{
		world = cell->worldSpace;

		if (world && cellDepth)		//exterior, cell depth > 0
		{
			curX = cell->coords->x - cellDepth;
			curY = cell->coords->y - cellDepth;
			curCell = world->LookupCell(curX, curY);
		}
		else
		{
			cellDepth = 0;
			curCell = cell;
			curX = cell->coords->x;
			curY = cell->coords->y;
		}
	}

	bool NextCell()		//advance to next cell in area
	{
		if (!world || !cellDepth)
		{
			curCell = NULL;
			return false;
		}

		do
		{
			if (curX - cell->coords->x == cellDepth)
			{
				if (curY - cell->coords->y == cellDepth)
				{
					curCell = NULL;
					return false;
				}
				else
				{
					curY++;
					curX -= cellDepth * 2;
					curCell = world->LookupCell(curX, curY);
				}
			}
			else
			{
				curX++;
				curCell = world->LookupCell(curX, curY);
			}
		}while (!curCell);

		return true;
	}

	void FirstCell()	//init curCell to point to first valid cell
	{
		if (!curCell)
			NextCell();
	}
};

class RefMatcherDeleted {
public:
	bool Accept(const TESObjectREFR* refr) {
		return refr->IsDeleted();
	}
};

class RefMatcherAnyForm
{
	bool m_includeTaken;
public:
	RefMatcherAnyForm(bool includeTaken) : m_includeTaken(includeTaken)
		{ }

	bool Accept(const TESObjectREFR* refr)
	{
		if (m_includeTaken || !(refr->IsTaken()))
			return true;
		else
			return false;
	}
};

class RefMatcherFormType
{
	UInt32 m_formType;
	bool m_includeTaken;
	bool m_includedDeleted;
public:
	RefMatcherFormType(UInt32 formType, bool includeTaken, bool includeDeleted) : m_formType(formType), m_includeTaken(includeTaken), m_includedDeleted(includeDeleted)
		{ }

	bool Accept(const TESObjectREFR* refr)
	{
		if (!m_includedDeleted && refr->IsDeleted())
			return false;
		else if (!m_includeTaken && refr->IsTaken())
			return false;
		else if (refr->baseForm->typeID == m_formType && refr->baseForm->refID != 7)	//exclude player for kFormType_NPC
			return true;
		else
			return false;
	}
};

class RefMatcherActor
{
	bool m_includedDeleted;
public:
	RefMatcherActor(bool includedDeleted) : m_includedDeleted(includedDeleted)
		{ }

	bool Accept(const TESObjectREFR* refr)
	{
		if (!m_includedDeleted && refr->IsDeleted())
			return false;
		else if (refr->baseForm->typeID == kFormType_Creature)
			return true;
		else if (refr->baseForm->typeID == kFormType_NPC && refr->baseForm->refID != 7) //exclude the player
			return true;
		else
			return false;
	}
};

class RefMatcherMapMarker
{
	bool m_includedDeleted;

public:
	RefMatcherMapMarker(bool includedDeleted) : m_includedDeleted(includedDeleted) {}

	bool Accept(const TESObjectREFR* refr) {
		return ((m_includedDeleted || !refr->IsDeleted()) && refr->baseForm->refID == kFormID_MapMarker);
	}
};

class RefMatcherItem
{
	bool m_includeTaken;
	bool m_includedDeleted;
public:
	RefMatcherItem(bool includeTaken, bool includeDeleted) : m_includeTaken(includeTaken), m_includedDeleted(includeDeleted)
		{ }

	bool Accept(const TESObjectREFR* refr)
	{
		if (!m_includedDeleted && refr->IsDeleted())
			return false;
		else if (!m_includeTaken && refr->IsTaken())
			return false;

		switch (refr->baseForm->typeID)
		{
			case kFormType_Apparatus:
			case kFormType_Armor:
			case kFormType_Book:
			case kFormType_Clothing:
			case kFormType_Ingredient:
			case kFormType_Misc:
			case kFormType_Weapon:
			case kFormType_Ammo:
			case kFormType_SoulGem:
			case kFormType_Key:
			case kFormType_AlchemyItem:
			case kFormType_SigilStone:
				return true;

			case kFormType_Light:
				TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(refr->baseForm, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
				if (light)
					if (light->IsCarriable())
						return true;
		}
		return false;
	}
};

class ProjectileFinder
{
	Actor		* m_owningActor;
	MagicCaster	* m_magicCaster;
	TESForm		* m_form;
	MagicItem	* m_magicItem;
	UInt32		m_type;
public:
	float curLifetime;

	enum {
		kType_Any,
		kType_Arrow,
		kType_Magic
	};

	ProjectileFinder(UInt32 type, Actor* actor, TESForm* formToMatch)
		: m_type(type), m_owningActor(actor), m_form(formToMatch), m_magicItem(NULL)
	{
		if (m_form)
			m_magicItem = OBLIVION_CAST(m_magicItem, TESForm, MagicItem);

		m_magicCaster = OBLIVION_CAST(m_owningActor, Actor, MagicCaster);
	}

	bool Accept(const TESObjectREFR* refr)
	{
		if (refr->baseForm->typeID != kFormType_Ammo || refr->IsDeleted())
			return false;

		MagicProjectile* magic = OBLIVION_CAST(refr, TESObjectREFR, MagicProjectile);
		ArrowProjectile* arrow = OBLIVION_CAST(refr, TESObjectREFR, ArrowProjectile);

		switch (m_type)
		{
		case kType_Arrow:
			if (m_form && refr->baseForm != m_form)
				return false;

			if (!arrow)
				return false;
			else if (arrow->shooter != m_owningActor)
				return false;
			else
			{
				curLifetime = arrow->elapsedTime;
				return true;
			}

		case kType_Magic:
			if (!magic)
				return false;
			else if (magic->caster != m_magicCaster)
				return false;
			else if (m_magicItem && magic->magicItem != m_magicItem)
				return false;
			else
			{
				curLifetime = magic->elapsedTime;
				return true;
			}
		case kType_Any:
			if (!m_owningActor)
				return true;

			if (arrow && arrow->shooter == m_owningActor)
			{
				curLifetime = arrow->elapsedTime;
				return true;
			}

			if (magic && magic->caster == m_magicCaster)
			{
				curLifetime = magic->elapsedTime;
				return true;
			}

			return false;
		default:		// uh-oh
			return false;
		}
	}
};

static const TESObjectCELL::ObjectListEntry* GetCellRefEntry(CellListVisitor visitor, UInt32 formType, const TESObjectCELL::ObjectListEntry* prev, bool includeTaken, bool includeDeleted, ProjectileFinder* projFinder = NULL)
{
	const TESObjectCELL::ObjectListEntry* entry = NULL;
	switch(formType)
	{
	case 0:		//Any type
		entry = visitor.Find(RefMatcherAnyForm(includeTaken), prev);
		break;
	case 69:	//Actor
		entry = visitor.Find(RefMatcherActor(includeDeleted), prev);
		break;
	case 70:	//Inventory Item
		entry = visitor.Find(RefMatcherItem(includeTaken,includeDeleted), prev);
		break;
	case 71:	//Owned Projectile
		if (projFinder)
			entry = visitor.Find(*projFinder, prev);
		break;
	case 72:	// map marker
		entry = visitor.Find(RefMatcherMapMarker(includeDeleted), prev);
		break;
	case 90:
		entry = visitor.Find(RefMatcherDeleted(), prev);
		break;
	default:
		entry = visitor.Find(RefMatcherFormType(formType, includeTaken, includeDeleted), prev);
	}

	return entry;
}

static TESObjectREFR* CellScan(Script* scriptObj, TESObjectCELL* cellToScan = NULL, UInt32 formType = 0, UInt32 cellDepth = 0, bool getFirst = false, bool includeTaken = false, bool includeDeleted = false ,ProjectileFinder* projFinder = NULL)
{
	static std::map<UInt32, CellScanInfo> scanScripts;
	UInt32 idx = scriptObj->refID;

	if (getFirst)
		scanScripts.erase(idx);

	if (scanScripts.find(idx) == scanScripts.end())
	{
		scanScripts[idx] = CellScanInfo(cellDepth, formType, includeTaken, includeDeleted , cellToScan);
		scanScripts[idx].FirstCell();
	}

	CellScanInfo* info = &(scanScripts[idx]);

	bool bContinue = true;
	while (bContinue)
	{
		info->prev = GetCellRefEntry(CellListVisitor(&info->curCell->objectList), info->formType, info->prev, info->includeTakenRefs, info->includeDeletedRefs , projFinder);
		if (!info->prev || !info->prev->refr)				//no ref found
		{
			if (!info->NextCell())			//check next cell if possible
				bContinue = false;
		}
		else if (!(*g_ioManager)->IsInQueue(info->prev->refr))	// don't include newly-queued refs
			bContinue = false;			//found a ref
	}

	if (info->prev)
		return info->prev->refr;
	else
	{
		scanScripts.erase(idx);
		return NULL;
	}
}

static bool GetFirstRef_Execute(COMMAND_ARGS, bool bUsePlayerCell = true)
{
	UInt32 formType = 0;
	UInt32 cellDepth = -1;
	UInt32 bIncludeTakenRefs = 0;
	UInt32 bIncludeDeletedRefs = 0;
	UInt32* refResult = (UInt32*)result;
	TESObjectCELL* cell = NULL;
	*refResult = 0;

	PlayerCharacter* pc = *g_thePlayer;
	if (!pc)
		return true;						//avoid crash when these functions called in main menu before parentCell instantiated

	if (bUsePlayerCell)
	{
		if (ExtractArgs(PASS_EXTRACT_ARGS, &formType, &cellDepth, &bIncludeTakenRefs, &bIncludeDeletedRefs))
			cell = pc->parentCell;
		else
			return true;
	}
	else
		if (!ExtractArgs(PASS_EXTRACT_ARGS, &cell, &formType, &cellDepth, &bIncludeTakenRefs, &bIncludeDeletedRefs))
			return true;

	if (!cell)
		return true;

	if (cellDepth == -1)
		cellDepth = 0;

	TESObjectREFR* refr = CellScan(scriptObj, cell, formType, cellDepth, true, bIncludeTakenRefs ? true : false, bIncludeDeletedRefs ? true : false);
	if (refr)
		*refResult = refr->refID;

	if (IsConsoleMode())
		Console_Print("GetFirstRef >> %08x", *refResult);

	return true;
}

static bool Cmd_GetFirstRef_Execute(COMMAND_ARGS)
{
	GetFirstRef_Execute(PASS_COMMAND_ARGS, true);
	return true;
}

static bool Cmd_GetFirstRefInCell_Execute(COMMAND_ARGS)
{
	GetFirstRef_Execute(PASS_COMMAND_ARGS, false);
	return true;
}

static bool Cmd_GetNextRef_Execute(COMMAND_ARGS)
{
	PlayerCharacter* pc = *g_thePlayer;
	if (!pc || !(pc->parentCell))
		return true;						//avoid crash when these functions called in main menu before parentCell instantiated

	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	TESObjectREFR* refr = CellScan(scriptObj);
	if (refr)
		*refResult = refr->refID;

	return true;
}

static bool GetNumRefs_Execute(COMMAND_ARGS, bool bUsePlayerCell = true)
{
	*result = 0;
	UInt32 formType = 0;
	UInt32 cellDepth = -1;
	UInt32 includeTakenRefs = 0;
	UInt32 includeDeletedRefs = 0;

	PlayerCharacter* pc = *g_thePlayer;
	if (!pc || !(pc->parentCell))
		return true;						//avoid crash when these functions called in main menu before parentCell instantiated

	TESObjectCELL* cell = NULL;
	if (bUsePlayerCell)
		if (ExtractArgs(PASS_EXTRACT_ARGS, &formType, &cellDepth, &includeTakenRefs, &includeDeletedRefs))
			cell = pc->parentCell;
		else
			return true;
	else
		if (!ExtractArgs(PASS_EXTRACT_ARGS, &cell, &formType, &cellDepth, &includeTakenRefs, &includeDeletedRefs))
			return true;

	if (!cell)
		return true;

	bool bIncludeTakenRefs = includeTakenRefs ? true : false;
	bool bIncludeDeltedRefs = includeDeletedRefs ? true : false;
	if (cellDepth == -1)
		cellDepth = 0;

	CellScanInfo info(cellDepth, formType, bIncludeTakenRefs, bIncludeDeltedRefs,cell);
	info.FirstCell();

	while (info.curCell)
	{
		CellListVisitor visitor(&info.curCell->objectList);
		switch (formType)
		{
		case 0:
			*result += visitor.CountIf(RefMatcherAnyForm(bIncludeTakenRefs));
			break;
		case 69:
			*result += visitor.CountIf(RefMatcherActor(bIncludeDeltedRefs));
			break;
		case 70:
			*result += visitor.CountIf(RefMatcherItem(bIncludeTakenRefs, bIncludeDeltedRefs));
			break;
		case 72:
			*result += visitor.CountIf(RefMatcherMapMarker(bIncludeDeltedRefs));
			break;
		case 90:
			*result += visitor.CountIf(RefMatcherDeleted());
			break;
		default:
			*result += visitor.CountIf(RefMatcherFormType(formType, bIncludeTakenRefs, bIncludeDeltedRefs));
		}
		info.NextCell();
	}

	return true;
}

static bool Cmd_GetNumRefs_Execute(COMMAND_ARGS)
{
	GetNumRefs_Execute(PASS_COMMAND_ARGS, true);
	return true;
}

static bool Cmd_GetNumRefsInCell_Execute(COMMAND_ARGS)
{
	GetNumRefs_Execute(PASS_COMMAND_ARGS, false);
	return true;
}

static bool Cmd_IsPersistent_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (thisObj)
		*result = (thisObj->flags & thisObj->kFlags_Persistent) ? 1 : 0;

	return true;
}

//SetPersistent not exposed because changes are not saved
static bool Cmd_SetPersistent_Execute(COMMAND_ARGS)
{
	UInt32 persistent = 0;
	*result = 0;

	ExtractArgs(PASS_EXTRACT_ARGS, &persistent);
	if (thisObj)
	{
		if (persistent)
			thisObj->flags |= thisObj->kFlags_Persistent;
		else
			thisObj->flags &= ~thisObj->kFlags_Persistent;
	}

	return true;
}

static bool Cmd_GetNumChildRefs_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_EnableStateChildren);
	if (!xData)
		return true;

	ExtraEnableStateChildren* xKids = (ExtraEnableStateChildren*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraEnableStateChildren, 0);
	if (xKids)
		*result = EnableStateChildrenVisitor(&xKids->childList).Count();

	return true;
}

static bool Cmd_GetNthChildRef_Execute(COMMAND_ARGS)
{
	UInt32 idx = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &idx))
		return true;

	else if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_EnableStateChildren);
	if (!xData)
		return true;

	ExtraEnableStateChildren* xKids = (ExtraEnableStateChildren*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraEnableStateChildren, 0);
	if (xKids)
	{
		TESObjectREFR* kid = EnableStateChildrenVisitor(&xKids->childList).GetNthInfo(idx);
		if (kid)
			*refResult = kid->refID;
	}

	return true;
}

static bool Cmd_SetScaleEX_Execute(COMMAND_ARGS)
{
	float newScale = 0;
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &newScale))
		return true;
	else if (!thisObj)
		return true;

	thisObj->scale = newScale;
	thisObj->MarkAsModified(thisObj->kChanged_Scale);
	*result = 1;

	return true;
}

static bool Cmd_IsActivatable_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj)
		return true;

	UInt32 type = thisObj->baseForm->typeID;
	if (type >= kFormType_Activator && type <= kFormType_Ingredient)
		*result = 1;
	else if (type == kFormType_Light)
		*result = 1;
	else if (type == kFormType_Misc)
		*result = 1;
	else if (type >= kFormType_Flora && type <= kFormType_Creature)
		*result = 1;
	else if (type >= kFormType_SoulGem && type <= kFormType_AlchemyItem)
		*result = 1;
	else if (type == kFormType_SigilStone)
		*result = 1;

	return true;
}

static bool Cmd_IsHarvested_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (!thisObj)
		return true;
	else if (thisObj->baseForm->typeID != kFormType_Flora)
		return true;

	if (thisObj->flags & TESFlora::kFloraFlags_Harvested)
		*result = 1;

	return true;
}

static bool Cmd_SetHarvested_Execute(COMMAND_ARGS)
{
	UInt32 bHarvested = 0;
	if (!thisObj || thisObj->baseForm->typeID != kFormType_Flora)
		return true;
	else if (!ExtractArgs(PASS_EXTRACT_ARGS, &bHarvested))
		return true;

	if (bHarvested){
		thisObj->flags |= TESFlora::kFloraFlags_Harvested;
		thisObj->MarkAsModified(TESFlora::kModified_Empty);
	}
	else {
		thisObj->flags &= ~TESFlora::kFloraFlags_Harvested;
		thisObj->ClearModified(TESFlora::kModified_Empty);
	}
	return true;
}

static bool Cmd_HasBeenPickedUp_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (thisObj && thisObj->IsTaken())
		*result = 1;

	return true;
}

static bool Cmd_SetHasBeenPickedUp_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 bPickedUp = 0;

	if (!thisObj)
		return true;
	else if (!ExtractArgs(PASS_EXTRACT_ARGS, &bPickedUp))
		return true;

	thisObj->SetTaken(bPickedUp ? true : false);
	return true;
}

UInt32 GetProjectileType(TESObjectREFR* mob)
{
	if (mob && mob->baseForm->typeID == kFormType_Ammo)
	{
		ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(mob, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (arrow)
			return kProjectileType_Arrow;

		MagicBallProjectile* ball = (MagicBallProjectile*)Oblivion_DynamicCast(mob, 0, RTTI_TESObjectREFR, RTTI_MagicBallProjectile, 0);
		if (ball)
			return kProjectileType_Ball;

		MagicBoltProjectile* bolt = (MagicBoltProjectile*)Oblivion_DynamicCast(mob, 0, RTTI_TESObjectREFR, RTTI_MagicBoltProjectile, 0);
		if (bolt)
			return kProjectileType_Bolt;

		MagicFogProjectile* fog = (MagicFogProjectile*)Oblivion_DynamicCast(mob, 0, RTTI_TESObjectREFR, RTTI_MagicFogProjectile, 0);
		if (fog)
			return kProjectileType_Fog;
	}

	return -1;
}

static bool Cmd_GetProjectileType_Execute(COMMAND_ARGS)
{
	*result = -1;

	if (thisObj)
		*result = GetProjectileType(thisObj);

	return true;
}

static bool Cmd_GetMagicProjectileSpell_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		MagicProjectile* mag = (MagicProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicProjectile, 0);
		if (mag && mag->magicItem)
		{
			SpellItem* spell = (SpellItem*)Oblivion_DynamicCast(mag->magicItem, 0, RTTI_MagicItem, RTTI_SpellItem, 0);
			if (spell)
				*refResult = spell->refID;
			else	//try as enchantment
			{
				EnchantmentItem* enchItem = (EnchantmentItem*)Oblivion_DynamicCast(mag->magicItem, 0, RTTI_MagicItem, RTTI_EnchantmentItem, 0);
				if (enchItem)
					*refResult = enchItem->refID;
			}
		}
	}

	return true;
}

static bool Cmd_GetArrowProjectileEnchantment_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (arrow && arrow->arrowEnch)
			*refResult = arrow->arrowEnch->refID;
	}

	return true;
}

static bool Cmd_GetArrowProjectileBowEnchantment_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (arrow && arrow->bowEnch)
			*refResult = arrow->bowEnch->refID;
	}

	return true;
}

static bool Cmd_GetArrowProjectilePoison_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (arrow && arrow->poison)
			*refResult = arrow->poison->refID;
	}

	return true;
}

static bool Cmd_GetProjectileSource_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		MagicProjectile* mag = (MagicProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicProjectile, 0);
		if (mag)
		{
			if (mag->caster)
			{
				Actor* caster = (Actor*)Oblivion_DynamicCast(mag->caster, 0, RTTI_MagicCaster, RTTI_Actor, 0);
				if (caster)
					*refResult = caster->refID;
				else
				{
					NonActorMagicCaster* caster = (NonActorMagicCaster*)Oblivion_DynamicCast(mag->caster, 0, RTTI_MagicCaster, RTTI_NonActorMagicCaster, 0);
					if (caster && caster->caster)
						*refResult = caster->caster->refID;
				}
			}
		}
		else
		{
			ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
			if (arrow && arrow->shooter)
				*refResult = arrow->shooter->refID;
		}
	}

	return true;
}

static bool Cmd_SetMagicProjectileSpell_Execute(COMMAND_ARGS)
{
	if (!thisObj)
		return true;

	SpellItem* spell = NULL;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &spell))
		return true;

	MagicProjectile* mag = (MagicProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicProjectile, 0);
	if (mag && spell)
		mag->magicItem = &(spell->magicItem);

	return true;
}

static bool Cmd_SetPlayerProjectile_Execute(COMMAND_ARGS)
{
	if (!thisObj)
		return true;

	MagicProjectile* mag = (MagicProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicProjectile, 0);
	if (mag)
	{
		MagicCaster* caster = (MagicCaster*)Oblivion_DynamicCast(*g_thePlayer, 0, RTTI_Actor, RTTI_MagicCaster, 0);
		if (caster)
			mag->caster = caster;
	}
	else
	{
		ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (arrow)
			arrow->shooter = *g_thePlayer;
	}

	return true;
}

//NULL for projectile source = BAD.
static bool Cmd_ClearProjectileSource_Execute(COMMAND_ARGS)
{
/*	if (thisObj)
	{
		MobileObject* mob = (MobileObject*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MobileObject, 0);
		if (mob)
		{
			MagicProjectileData* data = NULL;
			UInt32 projType = GetProjectileData(mob, true, &data);
			if (projType == kProjectileType_Arrow)
			{
				ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(mob, 0, RTTI_MobileObject, RTTI_ArrowProjectile, 0);
				if (arrow)
					arrow->shooter = NULL;
			}
			else if (projType != -1)
			{
				if (data)
					data->caster = NULL;
			}
		}
	}
*/
	return true;
}

static bool Cmd_GetRefCount_Execute(COMMAND_ARGS)
{
	*result = 1;
	if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Count);
	if (xData)
	{
		ExtraCount* xCount = (ExtraCount*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraCount, 0);
		if (xCount)
			*result = xCount->count;
	}
	return true;
}

static bool Cmd_SetRefCount_Execute(COMMAND_ARGS)
{
	if (thisObj) {
		InventoryReference* iref = InventoryReference::GetForRefID(thisObj->refID);
		if (iref && iref->GetContainer()) {
			_WARNING("SetRefCount cannot be called on an inventory reference");
			return true;
		}
	}

	UInt32 newCount = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &newCount))
		return true;
	else if (!thisObj || newCount > 32767 || newCount < 1)
		return true;

	ExtraCount* xCount = NULL;
	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Count);
	if (xData)
		xCount = (ExtraCount*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraCount, 0);
	else
	{
		xCount = ExtraCount::Create();
		xCount->count = newCount;
		thisObj->baseExtraList.Add(xCount);
	}

	if (xCount)
		xCount->count = newCount;

	return true;
}

static bool GetSound_Execute(COMMAND_ARGS, UInt32 whichSound)
{
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (whichSound > 2)
		return false;

	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
		return false;

	form = form->TryGetREFRParent();
	if (!form)
	{
		if (thisObj)
			form = thisObj->baseForm;
		else
			return false;
	}

	switch (form->typeID)
	{
	case kFormType_Door:
		{
			TESObjectDOOR* door = (TESObjectDOOR*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectDOOR, 0);
			if (door->animSounds[whichSound])
				*refResult = door->animSounds[whichSound]->refID;
		}
		break;
	case kFormType_Container:
		{
			if (whichSound == 2)
				return false;
			TESObjectCONT* cont = (TESObjectCONT*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectCONT, 0);
			if (cont->animSounds[whichSound])
				*refResult = cont->animSounds[whichSound]->refID;
		}
		break;
	case kFormType_Activator:
		{
			if (whichSound != 2)
				return false;
			TESObjectACTI* acti = (TESObjectACTI*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectACTI, 0);

			if (acti->loopSound)
				*refResult = acti->loopSound->refID;
		}
		break;
	case kFormType_Light:
		{
			if (whichSound != 2)
				return false;
			TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);

			if (light->loopSound)
				*refResult = light->loopSound->refID;
		}
		break;
	default:
		return false;
	}

	return true;
}

static bool Cmd_GetOpenSound_Execute(COMMAND_ARGS)
{
	*result = 0;
	GetSound_Execute(PASS_COMMAND_ARGS, 0);
	return true;
}

static bool Cmd_GetCloseSound_Execute(COMMAND_ARGS)
{
	*result = 0;
	GetSound_Execute(PASS_COMMAND_ARGS, 1);
	return true;
}

static bool Cmd_GetLoopSound_Execute(COMMAND_ARGS)
{
	*result = 0;
	GetSound_Execute(PASS_COMMAND_ARGS, 2);
	return true;
}

static bool SetSound_Execute(COMMAND_ARGS, UInt32 whichSound)
{
	TESForm* form = NULL;
	TESSound* sound = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (whichSound > 2)
		return false;

	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &form))
		return false;

	form = form->TryGetREFRParent();
	if (!form)
	{
		if (thisObj)
			form = thisObj->baseForm;
		else
			return false;
	}

	switch (form->typeID)
	{
	case kFormType_Door:
		{
			TESObjectDOOR* door = (TESObjectDOOR*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectDOOR, 0);
			door->animSounds[whichSound] = sound;
		}
		break;
	case kFormType_Container:
		{
			if (whichSound == 2)
				return false;
			TESObjectCONT* cont = (TESObjectCONT*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectCONT, 0);
			cont->animSounds[whichSound] = sound;
		}
		break;
	case kFormType_Activator:
		{
			if (whichSound != 2)
				return false;
			TESObjectACTI* acti = (TESObjectACTI*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectACTI, 0);
			acti->loopSound = sound;
		}
		break;
	case kFormType_Light:
		{
			if (whichSound != 2)
				return false;
			TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
			light->loopSound = sound;
		}
		break;
	default:
		return false;
	}

	return true;
}

static bool Cmd_SetOpenSound_Execute(COMMAND_ARGS)
{
	*result = 0;
	SetSound_Execute(PASS_COMMAND_ARGS, 0);
	return true;
}

static bool Cmd_SetCloseSound_Execute(COMMAND_ARGS)
{
	*result = 0;
	SetSound_Execute(PASS_COMMAND_ARGS, 1);
	return true;
}

static bool Cmd_SetLoopSound_Execute(COMMAND_ARGS)
{
	*result = 0;
	SetSound_Execute(PASS_COMMAND_ARGS, 2);
	return true;
}

static bool Cmd_DeleteReference_Execute(COMMAND_ARGS)
{
	*result = 0;

	// Don't delete temp refs or non-dynamic refs
	if (thisObj && thisObj->GetModIndex() == 0xFF && !(thisObj->flags & 0x4000))
	{
		// don't delete objects in inventories
		// references must be disabled before deletion
		if (!contObj && thisObj->IsDisabled())
		{
			IOManager* ioMan = IOManager::GetSingleton();
			if (ioMan)
			{
				QueueRefForDeletion(thisObj);
				*result = 1;
			}
		}
	}

	return true;
}

enum ProjectileDataType {
	kProjectile_Speed,
	kProjectile_Distance,
	kProjectile_Time,
	kProjectile_Source
};

static bool GetProjectileValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	if (!thisObj)
		return true;

	MagicProjectile* mag = (MagicProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicProjectile, 0);
	if (mag)
	{
		switch (whichValue)
		{
		case kProjectile_Speed:
			*result = mag->speed;
			return true;
		case kProjectile_Distance:
			*result = mag->distanceTraveled;
			return true;
		case kProjectile_Time:
			*result= mag->elapsedTime;
			return true;
		default:
			return true;
		}
	}
	else
	{
		ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (arrow)
		{
			switch (whichValue)
			{
			case kProjectile_Speed:
				*result = arrow->speed;
				return true;
			case kProjectile_Time:
				*result = arrow->elapsedTime;
				return true;
			case kProjectile_Distance:
				// TODO: decode ArrowProjectile class to expose this
				return true;
			default:
				return true;
			}
		}
	}

	return true;
}

static bool Cmd_GetProjectileSpeed_Execute(COMMAND_ARGS)
{
	return GetProjectileValue_Execute(PASS_COMMAND_ARGS, kProjectile_Speed);
}

static bool Cmd_GetProjectileDistanceTraveled_Execute(COMMAND_ARGS)
{
	return GetProjectileValue_Execute(PASS_COMMAND_ARGS, kProjectile_Distance);
}

static bool Cmd_GetProjectileLifetime_Execute(COMMAND_ARGS)
{
	return GetProjectileValue_Execute(PASS_COMMAND_ARGS, kProjectile_Time);
}

enum ArrowAdditionType{
	kArrow_BowEnchantment,
	kArrow_Enchantment,
	kArrow_Poison
};

static bool SetArrowProjectileValue(COMMAND_ARGS, UInt32 whichValue)
{
	EnchantmentItem* enchItem = NULL;
	AlchemyItem* alchItem = NULL;
	TESForm* formArg = NULL;
	*result = 0;

	if (!thisObj)
		return true;

	bool bExtracted = false;
	ArrowProjectile* arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
	if (arrow)
	{
		if (whichValue < kArrow_Poison)
		{
			bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &formArg);
			enchItem = (EnchantmentItem*)Oblivion_DynamicCast(formArg, 0, RTTI_TESForm, RTTI_EnchantmentItem, 0);
		}
		else
		{
			bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &formArg);
			alchItem = (AlchemyItem*)Oblivion_DynamicCast(formArg, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
		}

		if (bExtracted)
		{
			switch (whichValue)
			{
			case kArrow_Poison:
				arrow->poison = alchItem;
				break;
			case kArrow_Enchantment:
				arrow->arrowEnch = enchItem;
				break;
			case kArrow_BowEnchantment:
				arrow->bowEnch = enchItem;
				break;
			}
		}
	}

	return true;
}

static bool Cmd_SetArrowProjectileEnchantment_Execute(COMMAND_ARGS)
{
	return SetArrowProjectileValue(PASS_COMMAND_ARGS, kArrow_Enchantment);
}

static bool Cmd_SetArrowProjectileBowEnchantment_Execute(COMMAND_ARGS)
{
	return SetArrowProjectileValue(PASS_COMMAND_ARGS, kArrow_BowEnchantment);
}

static bool Cmd_SetArrowProjectilePoison_Execute(COMMAND_ARGS)
{
	return SetArrowProjectileValue(PASS_COMMAND_ARGS, kArrow_Poison);
}

static bool SetProjectileValue(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	if (!thisObj)
		return true;

	Actor* actor = NULL;
	float newValue = 0;

	ArrowProjectile* arrow = NULL;
	MagicProjectile* data = NULL;
	UInt32 projType = GetProjectileType(thisObj);
	if (projType == kProjectileType_Arrow)
	{
		arrow = (ArrowProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_ArrowProjectile, 0);
		if (!arrow)
			return true;
	}
	else if (projType != -1)
	{
		data = (MagicProjectile*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicProjectile, 0);
		if (!data)
			return true;
	}

	switch (whichValue)
	{
	case kProjectile_Speed:
		if (ExtractArgs(PASS_EXTRACT_ARGS, &newValue))
		{
			if (projType == kProjectileType_Arrow)
				arrow->speed = newValue;
			else
				data->speed = newValue;
		}
		break;
	case kProjectile_Source:
		if (ExtractArgs(PASS_EXTRACT_ARGS, &actor))
		{
			if (projType == kProjectileType_Arrow)
				arrow->shooter = actor;
			else
				data->caster = (MagicCaster*)Oblivion_DynamicCast(actor, 0, RTTI_Actor, RTTI_MagicCaster, 0);
		}
		break;
	}

	return true;
}

static bool Cmd_SetProjectileSource_Execute(COMMAND_ARGS)
{
	return SetProjectileValue(PASS_COMMAND_ARGS, kProjectile_Source);
}

static bool Cmd_SetProjectileSpeed_Execute(COMMAND_ARGS)
{
	return SetProjectileValue(PASS_COMMAND_ARGS, kProjectile_Speed);
}

enum MapMarkerType{
	kMapMarker_Visible,
	kMapMarker_CanTravel,
	kMapMarker_Type
};

enum MapMarkerMode {
	kSet,
	kGet
};

static bool MapMarkerCommand_Execute(COMMAND_ARGS, UInt32 mode, UInt32 attribute)
{
	UInt32 intArg = 0;
	*result = 0;

	if (!thisObj)
		return true;
	else if (mode == kSet && !ExtractArgs(PASS_EXTRACT_ARGS, &intArg))
		return true;

	ExtraMapMarker* mapMarker = (ExtraMapMarker*)thisObj->baseExtraList.GetByType(kExtraData_MapMarker);
	if (mapMarker && mapMarker->data)
	{
		switch (mode)
		{
		case kSet:
			switch (attribute)
			{
			case kMapMarker_Visible:
				mapMarker->SetVisible(intArg ? true : false);
				break;
			case kMapMarker_CanTravel:
				mapMarker->SetCanTravelTo(intArg ? true : false);
				break;
			case kMapMarker_Type:
				mapMarker->data->type = intArg;
				break;
			default:
				return true;

				thisObj->MarkAsModified(TESObjectREFR::kChanged_MapMarkerFlags);
			}

			break;
		case kGet:
			switch (attribute)
			{
			case kMapMarker_Visible:
				*result = mapMarker->IsVisible() ? 1 : 0;
				break;
			case kMapMarker_CanTravel:
				*result = mapMarker->CanTravelTo() ? 1 : 0;
				break;
			case kMapMarker_Type:
				*result = mapMarker->data->type;
				break;
			}
		}
	}

	return true;
}

static bool Cmd_IsMapMarkerVisible_Execute(COMMAND_ARGS)
{
	return MapMarkerCommand_Execute(PASS_COMMAND_ARGS, kGet, kMapMarker_Visible);
}

static bool Cmd_SetMapMarkerVisible_Execute(COMMAND_ARGS)
{
	return MapMarkerCommand_Execute(PASS_COMMAND_ARGS, kSet, kMapMarker_Visible);
}

static bool Cmd_CanTravelToMapMarker_Execute(COMMAND_ARGS)
{
	return MapMarkerCommand_Execute(PASS_COMMAND_ARGS, kGet, kMapMarker_CanTravel);
}

static bool Cmd_SetCanTravelToMapMarker_Execute(COMMAND_ARGS)
{
	return MapMarkerCommand_Execute(PASS_COMMAND_ARGS, kSet, kMapMarker_CanTravel);
}

static bool Cmd_GetMapMarkerType_Execute(COMMAND_ARGS)
{
	return MapMarkerCommand_Execute(PASS_COMMAND_ARGS, kGet, kMapMarker_Type);
}

static bool Cmd_SetMapMarkerType_Execute(COMMAND_ARGS)
{
	return MapMarkerCommand_Execute(PASS_COMMAND_ARGS, kSet, kMapMarker_Type);
}

static bool Cmd_SetBaseForm_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = 0;

	if (contObj)			// object is in a container
		return true;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form) && thisObj)
	{
		if (!thisObj->IsActor() && form && form->typeID == thisObj->baseForm->typeID)
		{
			*result = 1;
			thisObj->baseForm = form;
		}
	}

	return true;
}

static bool Cmd_GetParentCell_Execute(COMMAND_ARGS)
{
	UInt32	* refResult = (UInt32 *)result;
	*refResult = 0;

	if(!thisObj) return true;

	TESForm	* currentCell = (TESForm *)Oblivion_DynamicCast(thisObj->parentCell, 0, RTTI_TESObjectCELL, RTTI_TESForm, 0);

	if(currentCell) {
		//Console_Print("Cell: %08x", currentCell->refID);
		*refResult = currentCell->refID;
	}

	return true;
}

static bool Cmd_GetParentWorldspace_Execute(COMMAND_ARGS)
{
	*result = 0;

	if(!thisObj || !thisObj->parentCell || !thisObj->parentCell->worldSpace) return true;

	UInt32	* refResult = (UInt32 *)result;
	*refResult = thisObj->parentCell->worldSpace->refID;

	return true;
}

static bool Cmd_GetProjectile_Execute(COMMAND_ARGS)
{
	UInt32 projType = 0;
	TESForm* formToMatch = NULL;
	float minLifetime = 9999.0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;
	else if (!(*g_thePlayer))
		return true;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &projType, &minLifetime, &formToMatch))
	{
		ProjectileFinder finder(projType, actor, formToMatch);
		TESObjectCELL* pcCell = (*g_thePlayer)->parentCell;
		TESObjectREFR* curRef = CellScan(scriptObj, pcCell, 71, 3, true, false, &finder);
		TESObjectREFR* foundRef = NULL;
		while (curRef)
		{
			if (finder.curLifetime <= minLifetime)
			{
				foundRef = curRef;
				minLifetime = finder.curLifetime;
				if (minLifetime == 0)
					break;
			}
			curRef = CellScan(scriptObj, pcCell, 71, 3, false, false, &finder);
		}

		if (foundRef)
		{
			*refResult = foundRef->refID;
			if (IsConsoleMode())
				Console_Print("GetProjectile >> (%08x)", *refResult);
		}
	}

	return true;
}

static bool Cmd_Activate2_Execute(COMMAND_ARGS)
{
	// ###TODO: make this fix bug in which calling Activate on an unscripted object prevents
	// subsequent default activation of that object

	ResetActivationRecurseDepth();
	return Cmd_Activate_Execute(PASS_COMMAND_ARGS);
}

static bool Cmd_IsRefDeleted_Execute(COMMAND_ARGS)
{
	TESObjectREFR* refr = NULL;
	*result = 1;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &refr) && refr && !refr->IsDeleted())
		*result = 0;

	return true;
}

static bool Cmd_GetTeleportCellName_Execute(COMMAND_ARGS)
{
	// is useful for doors to exterior cells, which may not be loaded in memory

	// a little hacky, but I like keeping constructors for game types undefined to avoid
	// accidentally creating them.
	char dummy[sizeof(BSStringT)] = { 0 };
	BSStringT* strName = (BSStringT*)dummy;
	strName->Set("");

	if (thisObj) {
		thisObj->GetTeleportCellName(strName);
	}

	AssignToStringVar(PASS_COMMAND_ARGS, strName->m_data);

	// free mem
	strName->Set(NULL);
	return true;
}

static bool Cmd_Update3D_Execute(COMMAND_ARGS)
{
	if (thisObj) {
		if (thisObj->Update3D()) {
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_GetBoundingBox_Execute(COMMAND_ARGS)
{
	UInt32 arrID = 0;
	if (thisObj) {
		MobileObject* mob = OBLIVION_CAST(thisObj, TESObjectREFR, MobileObject);
		if (mob) {
			MiddleHighProcess* proc = OBLIVION_CAST(mob->process, BaseProcess, MiddleHighProcess);
			if (proc) {
				BSBound* bound = proc->boundingBox;
				if (bound) {
					arrID = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());

					ArrayID centerID = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
					g_ArrayMap.SetElementNumber(centerID, "x", bound->center.x);
					g_ArrayMap.SetElementNumber(centerID, "y", bound->center.y);
					g_ArrayMap.SetElementNumber(centerID, "z", bound->center.z);

					ArrayID extentID = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
					g_ArrayMap.SetElementNumber(extentID, "x", bound->extents.x);
					g_ArrayMap.SetElementNumber(extentID, "y", bound->extents.y);
					g_ArrayMap.SetElementNumber(extentID, "z", bound->extents.z);

					g_ArrayMap.SetElementArray(arrID, "center", centerID);
					g_ArrayMap.SetElementArray(arrID, "extent", extentID);
				}
			}
		}
	}

	*result = arrID;
	return true;
}

static bool Cmd_SetOwner_Execute(COMMAND_ARGS)
{
	bool bChanged = false;
	if (thisObj) {
		TESForm* owner = NULL;
		if (ExtractArgs(PASS_EXTRACT_ARGS, &owner)) {
			ExtraOwnership* xOwner = (ExtraOwnership*)thisObj->baseExtraList.GetByType(kExtraData_Ownership);
			if (xOwner) {
				bChanged = true;
				if (owner) {
					xOwner->owner = owner;
				}
				else {
					thisObj->baseExtraList.Remove(xOwner);
					FormHeap_Free(xOwner);
				}
			}
			else if (owner) {
				bChanged = true;
				xOwner = ExtraOwnership::Create(owner);
				thisObj->baseExtraList.Add(xOwner);
			}	// else don't change anything
		}
	}

	if (bChanged) {
		*result = 1;
	}

	return true;
}

static bool Cmd_GetBoundingRadius_Execute(COMMAND_ARGS)
{
	if (thisObj && thisObj->niNode) {
		*result = thisObj->niNode->m_kWorldBound.radius;
	}
	else {
		*result = -1.0;
	}

	if (IsConsoleMode()) {
		Console_Print("GetBoundingRadius >> %.2f", *result);
	}

	return true;
}

struct Cmd_OverrideChangeFlag_Info {
	const Cmd_Execute	cmd;
	UInt32				patchLoc;
	UInt8				changeFlag;
};

static const Cmd_OverrideChangeFlag_Info s_tempCommandInfos[3] = {
	{	(const Cmd_Execute)0x00508FC0,	0x004D8A4B,	0x04	},
	{	(const Cmd_Execute)0x005075A0,	0x00507610,	0x80	},
	{	(const Cmd_Execute)0x00507620,	0x00507637, 0x80	},
};



static bool Cmd_OverrideChangeFlag_Execute(COMMAND_ARGS, UInt32 index)
{
	// calls a vanilla script function, preventing the form from being marked as modified by this particular call
	ASSERT(index < SIZEOF_ARRAY(s_tempCommandInfos, Cmd_OverrideChangeFlag_Info));
	const Cmd_OverrideChangeFlag_Info* info = &s_tempCommandInfos[index];

	// replace the change flag with zero
	SafeWrite8(info->patchLoc, 0);
	// invoke the cmd
	bool bResult = info->cmd(PASS_COMMAND_ARGS);
	// restore change flag
	SafeWrite8(info->patchLoc, info->changeFlag);
	return bResult;
}

static bool Cmd_SetPos_T_Execute(COMMAND_ARGS)
{
	return Cmd_OverrideChangeFlag_Execute(PASS_COMMAND_ARGS, 0);
}

static bool Cmd_SetOwnership_T_Execute(COMMAND_ARGS)
{
	return Cmd_OverrideChangeFlag_Execute(PASS_COMMAND_ARGS, 1);
}

static bool Cmd_ClearOwnership_T_Execute(COMMAND_ARGS)
{
	return Cmd_OverrideChangeFlag_Execute(PASS_COMMAND_ARGS, 2);
}

class EffectShaderFinder
{
	TESEffectShader		* m_shader;
	TESObjectREFR		* m_refr;
public:
	EffectShaderFinder(TESObjectREFR* refr, TESEffectShader* shader) : m_refr(refr), m_shader(shader) { }

	bool Accept(BSTempEffect* effect)
	{
		MagicShaderHitEffect* mgsh = OBLIVION_CAST(effect, BSTempEffect, MagicShaderHitEffect);
		if (mgsh && mgsh->target == m_refr && mgsh->effectShader == m_shader) {
			return true;
		}
		else {
			return false;
		}
	}
};

static bool Cmd_HasEffectShader_Execute(COMMAND_ARGS)
{
	TESEffectShader * shader = NULL;
	*result = 0.0;

	if (thisObj && ExtractArgs(PASS_EXTRACT_ARGS, &shader) && shader) {
		EffectShaderFinder finder(thisObj, shader);
		*result = g_actorProcessManager->tempEffects.CountIf(finder);
	}

	return true;
}

static bool Cmd_IsInOblivion_Execute(COMMAND_ARGS)
{
	*result = 0.0;

	if (thisObj && thisObj->parentCell) {
		if (thisObj->parentCell->IsOblivionInterior()) {
			*result = 1.0;
		}
		else if (thisObj->parentCell->worldSpace && thisObj->parentCell->worldSpace->IsOblivionWorld()) {
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_GetTimeLeft_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	if (thisObj) {
		ExtraTimeLeft* xTime = (ExtraTimeLeft*)thisObj->baseExtraList.GetByType(kExtraData_TimeLeft);
		if (xTime) {
			*result = xTime->time;
		}
		else {
			// return base light duration
			TESObjectLIGH* light = OBLIVION_CAST(thisObj->baseForm, TESForm, TESObjectLIGH);
			if (light) {
				*result = light->time;
			}
		}
	}

	if (IsConsoleMode()) {
		Console_Print("GetTimeLeft >> %.2f", *result);
	}

	return true;
}

void SetExtraTimeLeft(BaseExtraList* list, float time)
{
	ThisStdCall(0x0041EDF0, list, time);

}

static bool Cmd_SetTimeLeft_Execute(COMMAND_ARGS)
{
	if (thisObj && thisObj->baseForm->typeID == kFormType_Light) {
		float time = 0.0;
		if (ExtractArgs(PASS_EXTRACT_ARGS, &time)) {
			SetExtraTimeLeft(&thisObj->baseExtraList, time);
		}
	}

	return true;
}

#endif

CommandInfo kCommandInfo_GetTravelHorse =
{
	"GetTravelHorse",
	"",
	0,
	"returns the travel horse of the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetTravelHorse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetTravelHorse =
{
	"SetTravelHorse",
	"",
	0,
	"sets the travel horse of the reference",
	1,
	1,
	kParams_OneObjectRef,
	HANDLER(Cmd_SetTravelHorse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetOpenKey =
{
	"GetOpenKey",
	"GetKey",
	0,
	"returns the key used to unlock the calling object",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetOpenKey_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOpenKey =
{
	"SetOpenKey",
	"SetKey",
	0,
	"sets the key used to unlock the calling object",
	1,
	1,
	kParams_OneInventoryObject,
	HANDLER(Cmd_SetOpenKey_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetParentCellOwner =
{
	"GetParentCellOwner",
	"GetCellOwner",
	0,
	"returns the owner of the calling reference's cell",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetParentCellOwner_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetOwner =
{
	"GetOwner",
	"",
	0,
	"returns the owner of the calling reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetOwner_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneActor[1] =
{
	{	"actor reference",	 kParamType_Actor,	0	},
};

CommandInfo kCommandInfo_GetOwningFactionRequiredRank =
{
	"GetOwningFactionRequiredRank",
	"GetOwningFactionRank",
	0,
	"returns the required rank for ownership of the calling reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetOwningFactionRequiredRank_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetParentCellOwningFactionRequiredRank =
{
	"GetParentCellOwningFactionRequiredRank",
	"GetCellFactionRank",
	0,
	"returns the required rank for ownership of the calling reference's cell",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetParentCellOwningFactionRequiredRank_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
CommandInfo kCommandInfo_SetOwningFactionRequiredRank =
{
	"SetOwningFactionRequiredRank",
	"SetOwningFactionRank",
	0,
	"sets the required rank for ownership of the calling reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetOwningFactionRequiredRank_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
CommandInfo kCommandInfo_SetParentCellOwningFactionRequiredRank =
{
	"SetParentCellOwningFactionRequiredRank",
	"SetCellFactionRank",
	0,
	"sets the required rank for ownership of the calling reference's cell",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetParentCellOwningFactionRequiredRank_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneNPC[1] =
{
	{	"NPC",	kParamType_NPC,	0	},
};

CommandInfo kCommandInfo_IsOffLimits =
{
	"IsOffLimits", "IsIllegal",
	0,
	"returns true if activating the calling reference would result in a crime for the actor",
	1,
	1,
	kParams_OneOptionalNPC,
	HANDLER(Cmd_IsOffLimits_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsLoadDoor =
{
	"IsLoadDoor", "",
	0,
	"returns 1 if the calling reference is a load door",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsLoadDoor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetLinkedDoor =
{
	"GetLinkedDoor", "",
	0,
	"returns the door to which the calling reference is linked",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetLinkedDoor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTeleportCell =
{
	"GetTeleportCell", "",
	0,
	"returns the cell to which the calling door reference teleports",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetTeleportCell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetFirstRef[4] =
{
	{	"form type",			kParamType_Integer,	1	},
	{	"cell depth",			kParamType_Integer,	1	},
	{	"include taken refs",	kParamType_Integer,	1	},
	{	"include deleted refs",	kParamType_Integer,	1	},
};

CommandInfo kCommandInfo_GetFirstRef =
{
	"GetFirstRef", "",
	0,
	"returns the first reference of the specified type in the current cell",
	0,
	4,
	kParams_GetFirstRef,
	HANDLER(Cmd_GetFirstRef_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNextRef =
{
	"GetNextRef", "",
	0,
	"returns the next reference of a given type in the  current cell",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetNextRef_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumRefs =
{
	"GetNumRefs", "",
	0,
	"returns the number of references of a given type in the current cell",
	0,
	4,
	kParams_GetFirstRef,
	HANDLER(Cmd_GetNumRefs_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetFirstRefInCell[5] =
{
	{	"cell",					kParamType_Cell,	0	},
	{	"form type",			kParamType_Integer,	1	},
	{	"cell depth",			kParamType_Integer,	1	},
	{	"include taken refs",	kParamType_Integer,	1	},
	{	"include deleted refs",	kParamType_Integer,	1	},
};

CommandInfo kCommandInfo_GetFirstRefInCell =
{
	"GetFirstRefInCell", "",
	0,
	"returns the first reference of the specified type in the specified cell",
	0,
	5,
	kParams_GetFirstRefInCell,
	HANDLER(Cmd_GetFirstRefInCell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumRefsInCell =
{
	"GetNumRefsInCell", "",
	0,
	"returns the number of references of a given type in the specified cell",
	0,
	5,
	kParams_GetFirstRefInCell,
	HANDLER(Cmd_GetNumRefsInCell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsPersistent =
{
	"IsPersistent", "",
	0,
	"returns true if the reference is persistent",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsPersistent_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetPersistent =
{
	"SetPersistent", "",
	0,
	"sets the persistence of the calling reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetPersistent_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumChildRefs =
{
	"GetNumChildRefs", "", 0,
	"returns the number of enable state children for the calling reference",
	1, 0, NULL,
	HANDLER(Cmd_GetNumChildRefs_Execute),
	Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_GetNthChildRef =
{
	"GetNthChildRef", "", 0,
	"returns the nth enable state child for the calling reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthChildRef_Execute),
	Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_SetScaleEX =
{
	"SetScaleEX", "", 0,
	"sets scale of the calling reference above or below limits of setScale",
	1,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_SetScaleEX_Execute),
	Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_IsActivatable =
{
	"IsActivatable", "",
	0,
	"returns 1 if the calling reference can be activated",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsActivatable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsHarvested =
{
	"IsHarvested", "",
	0,
	"returns 1 if the calling flora reference has been harvested",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsHarvested_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetHarvested =
{
	"SetHarvested", "",
	0,
	"sets the harvested flag on the calling flora reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetHarvested_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_HasBeenPickedUp =
{
	"HasBeenPickedUp",
	"IsTaken",
	0,
	"returns 1 if the calling reference has been placed in an inventory",
	1,
	0,
	NULL,
	HANDLER(Cmd_HasBeenPickedUp_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetProjectileType =
{
	"GetProjectileType",
	"",
	0,
	"returns the type of the calling projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetProjectileType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicProjectileSpell =
{
	"GetMagicProjectileSpell",
	"GetMPSpell",
	0,
	"returns the spell associated with the projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetMagicProjectileSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetArrowProjectileEnchantment =
{
	"GetArrowProjectileEnchantment",
	"GetAPEnch",
	0,
	"returns the enchantment on the calling arrow projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetArrowProjectileEnchantment_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetArrowProjectileBowEnchantment =
{
	"GetArrowProjectileBowEnchantment",
	"GetAPBowEnch",
	0,
	"returns the bow enchantment on the calling arrow projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetArrowProjectileBowEnchantment_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetArrowProjectilePoison =
{
	"GetArrowProjectilePoison",
	"GetAPPoison",
	0,
	"returns the poison on the calling arrow projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetArrowProjectilePoison_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetProjectileSource =
{
	"GetProjectileSource",
	"",
	0,
	"returns the source of the calling projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetProjectileSource_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicProjectileSpell =
{
	"SetMagicProjectileSpell",
	"SetMPSpell",
	0,
	"sets the spell associated with the calling magic projectile",
	1,
	1,
	kParams_OneSpellItem,
	HANDLER(Cmd_SetMagicProjectileSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

/*
CommandInfo kCommandInfo_SetProjectileSource =
{
	"SetProjectileSource",
	"",
	0,
	"sets the source of the calling projectile to the specified actor",
	1,
	1,
	kParams_OneActor,
	HANDLER(Cmd_SetProjectileSource_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
*/

CommandInfo kCommandInfo_ClearProjectileSource =
{
	"ClearProjectileSource",
	"",
	0,
	"removes information about the source of the calling projectile",
	1,
	0,
	NULL,
	HANDLER(Cmd_ClearProjectileSource_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetRefCount =
{
	"GetRefCount",
	"",
	0,
	"returns the count in a stacked reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetRefCount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetRefCount =
{
	"SetRefCount",
	"",
	0,
	"sets the count on a stacked reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetRefCount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetSound[2] =
{
	{	"sound",	kParamType_Sound,			0	},
	{	"object",	kParamType_InventoryObject,	1	},
};

CommandInfo kCommandInfo_GetOpenSound =
{
	"GetOpenSound",
	"",
	0,
	"returns the open sound for a container or door",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetOpenSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCloseSound =
{
	"GetCloseSound",
	"",
	0,
	"returns the close sound for a container or door",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetCloseSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetLoopSound =
{
	"GetLoopSound",
	"",
	0,
	"returns the loop sound for an object",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetLoopSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOpenSound =
{
	"SetOpenSound",
	"",
	0,
	"sets the open sound for a container or door",
	0,
	2,
	kParams_SetSound,
	HANDLER(Cmd_SetOpenSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCloseSound =
{
	"SetCloseSound",
	"",
	0,
	"sets the close sound for a container or door",
	0,
	2,
	kParams_SetSound,
	HANDLER(Cmd_SetCloseSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetLoopSound =
{
	"SetLoopSound",
	"",
	0,
	"sets the looping sound for an object",
	0,
	2,
	kParams_SetSound,
	HANDLER(Cmd_SetLoopSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(SetPlayerProjectile,
			   sets the player as the source of the projectile,
			   1,
			   0,
			   NULL);

CommandInfo kCommandInfo_SetHasBeenPickedUp =
{
	"SetHasBeenPickedUp",
	"SetTaken",
	0,
	"toggles the 'taken' flag on a reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetHasBeenPickedUp_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(DeleteReference,
			   deletes a dynamic reference,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(SetProjectileSource,
			   sets the source of a projectile,
			   1,
			   1,
			   kParams_OneActorRef);

static ParamInfo kParams_OneOptionalMagicItem[1] =
{
	{	"enchantment",	kParamType_MagicItem,	1	},
};

DEFINE_COMMAND(SetArrowProjectileEnchantment,
			   sets the enchantment on an arrow projectile,
			   1,
			   1,
			   kParams_OneOptionalMagicItem);

DEFINE_COMMAND(SetArrowProjectileBowEnchantment,
			   sets the bow enchantment on an arrow projectile,
			   1,
			   1,
			   kParams_OneOptionalMagicItem);

DEFINE_COMMAND(SetArrowProjectilePoison,
			   sets the poison on an arrow projectile,
			   1,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetProjectileSpeed,
			   returns the speed of a projectile,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(GetProjectileDistanceTraveled,
			   returns the distance traveled by a projectile,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(GetProjectileLifetime,
			   returns the length of time a projectile has existed,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(SetProjectileSpeed,
			   sets the speed of a projectile - this may not work as expected,
			   1,
			   1,
			   kParams_OneFloat);

DEFINE_COMMAND(IsMapMarkerVisible,
			   returns true if the map marker is visible on the map,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(SetMapMarkerVisible,
			   toggles the visible flag on a map marker,
			   1,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(CanTravelToMapMarker,
			   returns true if the map marker can be traveled to,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(SetCanTravelToMapMarker,
			   toggles the can travel flag on the map marker,
			   1,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetMapMarkerType,
			   returns the type of the map marker,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(SetMapMarkerType,
			   sets the type of the map marker,
			   1,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(SetBaseForm,
			   sets the baseform of a reference to another form of the same type,
			   1,
			   1,
			   kParams_OneInventoryObject);

static ParamInfo kParams_GetProjectile[3] =
{
	{	"type",			kParamType_Integer,			1	},
	{	"maxLifetime",	kParamType_Float,			1	},
	{	"formToMatch",	kParamType_InventoryObject,	1	},
};

DEFINE_COMMAND(GetProjectile,
			   returns the most recent projectile fired by the calling actor,
			   1,
			   3,
			   kParams_GetProjectile);

static ParamInfo kParams_Activate[] =
{
	{	"ObjectReferenceID",	kParamType_ObjectRef, 1 },
	{	"Integer",				kParamType_Integer,	  1 },
};

DEFINE_COMMAND(Activate2,
			   activates the calling object circumventing hard-coded limit on recursive Activate calls,
			   1, 2, kParams_Activate);

DEFINE_COMMAND(IsRefDeleted, returns true if the calling object has been deleted by a plugin, 0, 1, kParams_OneObjectRef);

CommandInfo kCommandInfo_GetParentCell =
{
	"GetParentCell",
	"gcel",
	0,
	"returns a ref to the cell the object is in",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetParentCell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetParentWorldspace =
{
	"GetParentWorldspace",
	"",
	0,
	"returns a ref to the worldspace the object is in",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetParentWorldspace_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetDoorTeleportX, returns x-coord to which the door teleports, 1, 0, NULL);
DEFINE_COMMAND(GetDoorTeleportY, returns y-coord to which the door teleports, 1, 0, NULL);
DEFINE_COMMAND(GetDoorTeleportZ, returns z-coord to which the door teleports, 1, 0, NULL);
DEFINE_COMMAND(GetDoorTeleportRot, returns z rotation to which the door teleports, 1, 0, NULL);

static ParamInfo kParams_SetDoorTeleport[6] =
{
	{	"linkedDoor",	kParamType_ObjectRef,	0	},
	{	"x",			kParamType_Float,		1	},
	{	"y",			kParamType_Float,		1	},
	{	"z",			kParamType_Float,		1	},
	{	"rot",			kParamType_Float,		1	},
	{	"temporary",	kParamType_Integer,		1	},
};

DEFINE_COMMAND(SetDoorTeleport, sets the linked door and coordinates to which the door teleports, 0, 6, kParams_SetDoorTeleport);
DEFINE_COMMAND(GetTeleportCellName, returns the name of the cell to which a door teleports, 1, 0, NULL);
DEFINE_COMMAND(Update3D, updates the visual representation of the calling reference, 1, 0, NULL);

DEFINE_COMMAND(GetBoundingBox, returns a stringmap representing the calling objects axis-aligned bounding box, 1, 0, NULL);

static ParamInfo kParams_OneOptionalOwner[1] =
{
	{	"owner",	kParamType_Owner,	1	},
};

DEFINE_COMMAND(SetOwner, sets the owner of the calling reference to the specified base actor or faction. If the argument is omitted all ownership will be removed, 1, 1, kParams_OneOptionalOwner);

DEFINE_COMMAND(GetBoundingRadius, returns the radius of the objects bounding sphere, 1, 0, NULL);

static ParamInfo kParams_SetPos[2] = {
	{	"axis",	kParamType_Axis,	0	},
	{	"pos",	kParamType_Float,	0	},
};

DEFINE_COMMAND(SetPos_T, like setPos but change in position not saved, 1, 2, kParams_SetPos);
DEFINE_COMMAND(SetOwnership_T, like SetOwnership but change not saved, 1, 1, kParams_OneOptionalOwner);
DEFINE_COMMAND(ClearOwnership_T, like ClearOwnership but change not saved, 1, 0, NULL);

static ParamInfo kParams_OneEffectShader[1] =
{
	{ "effectShader",	kParamType_EffectShader,	0	},
};

DEFINE_COMMAND(HasEffectShader, returns 1 if the reference is playing the effect shader, 1, 1, kParams_OneEffectShader);
DEFINE_COMMAND(IsInOblivion, returns 1 if the reference is in Oblivion, 1, 0, NULL);

DEFINE_COMMAND(GetTimeLeft, returns the time left for the light reference, 1, 0, NULL);
DEFINE_COMMAND(SetTimeLeft, sets the time left for the light reference, 1, 1, kParams_OneFloat);

DEFINE_COMMAND(LinkToDoor, links the caling door reference with the specified door reference, 1, 1, kParams_OneObjectRef);
