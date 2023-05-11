#include "obse/GameExtraData.h"
#include "obse/GameAPI.h"
#include "obse/GameObjects.h"
#include "obse/Script.h"
#include "obse/Hooks_Gameplay.h"
#include <obse\Settings.h>

typedef ExtraContainerChanges* (* _GetOrCreateExtraContainerChanges)(TESObjectREFR* refr);
_GetOrCreateExtraContainerChanges GetOrCreateExtraContainerChanges = (_GetOrCreateExtraContainerChanges)0x00485E00;

ExtraContainerChanges::EntryData * ExtraContainerChanges::GetByType(TESForm * type)
{
	ExtraContainerChanges::EntryData	* result = NULL;

	if(data)	{
		for(tList<EntryData>::Iterator entry = data->objList->Begin(); !entry.End(); ++entry)		{
			if(*entry && (entry->type == type))
			{
				result = entry.Get();
				break;
			}
		}
	}

	return result;
}
/*
ExtraContainerChanges::EntryExtendData* ExtraContainerChanges::EntryExtendData::Create(ExtraDataList* list)
{
	EntryExtendData* extend = FORM_HEAP_ALLOCATE(EntryExtendData);
	extend->data = list;
	extend->next = NULL;
	return extend;
}

bool ExtraContainerChanges::Entry::Remove(EntryExtendData* toRemove, bool bFree)
{
	if (data) {
		ExtraCount* xCount = toRemove->data ? (ExtraCount*)toRemove->data->GetByType(kExtraData_Count) : NULL;
		SInt16 count = xCount ? xCount->count : 1;
		EntryExtendData* prev = NULL;
		for (EntryExtendData* cur = data->extendData; cur; cur = cur->next) {
			if (cur == toRemove) {
				if (prev)
					prev->next = cur->next;
				else
					data->extendData = cur->next;
				data->countDelta -= count;
				if (bFree) {
					if (toRemove->data) {
						toRemove->data->Destroy(FreeRef);
					}
					FormHeap_Free(toRemove);
				}
				return true;
			}
			prev = cur;
		}
	}
	ASSERT(false);
	return false;
}

bool ExtraContainerChanges::Entry::Add(EntryExtendData* toAdd)
{
	if (data && data->Add(toAdd)) {
		return true;
	}
	ASSERT(false);
	return false;
}

ExtraContainerChanges::EntryExtendData* ExtraContainerChanges::EntryData::Add(ExtraContainerChanges::EntryExtendData* newData)
{
	newData->next = extendData;	
	extendData = newData;

	// update the count, if no ExtraCount it is 1
	ExtraCount* xCount = (newData && newData->data) ? (ExtraCount*)newData->data->GetByType(kExtraData_Count) : NULL;
	countDelta += xCount ? xCount->count : 1;

	return extendData;
}


ExtraContainerChanges::EntryExtendData* ExtraContainerChanges::Add(TESForm* form, ExtraDataList* dataList)
{
	if (!data) {
		// wtf
		_WARNING("ExtraContainerChanges::Add() encountered ExtraContainerChanges with NULL data");
		return NULL;
	}

	if (!data->objList) {
		data->objList = Entry::Create();
	}

	// try to locate the form
	EntryData* found = NULL;
	for (Entry* cur = data->objList; cur; cur = cur->next) {
		if (cur->data && cur->data->type == form) {
			found = cur->data;
			break;
		}
	}

	if (!found) {
		// add it to the list with a count delta of 0
		Entry* en = Entry::Create();
		found = EntryData::Create(0, form);
		en->data = found;
		en->next = data->objList;
		data->objList = en;
	}

	return found->Add(dataList);
}

ExtraContainerChanges::Entry* ExtraContainerChanges::Entry::Create()
{
	Entry* entry = FORM_HEAP_ALLOCATE(Entry);
	entry->data = NULL;
	entry->next = NULL;
	return entry;
}
*/

void ExtraContainerChanges::EntryData::Add(ExtraDataList* newList)
{
	if(extendData == nullptr) extendData = (tList<ExtraDataList>*) tList<ExtraDataList>::Create();
	extendData->AddAt(newList,0);
}


void ExtraContainerChanges::EntryData::Cleanup()
{
	ThisStdCall(0x00484660, this);

}

UInt32 GetCountForExtraDataList(ExtraDataList* list)
{
	if (!list)
		return 1;

	ExtraCount* xCount = (ExtraCount*)list->GetByType(kExtraData_Count);
	return xCount ? xCount->count : 1;
}

void ExtraContainerChanges::Cleanup()
{
	if (data) {
		for (tList<ExtraContainerChanges::EntryData>::Iterator cur = data->objList->Begin(); !cur.End(); ++cur) {
			if (*cur) {
//				DEBUG_PRINT("cur %0X", *cur);
				// make sure we don't have any NULL ExtraDataList's in extend data, game will choke when saving
				tList<ExtraDataList>::_Node* prev = NULL;
				if (cur->extendData == NULL  /*|| cur->extendData->IsEmpty()*/) continue;
				for (tList<ExtraDataList>::Iterator xtendData = cur->extendData->Begin(); !xtendData.End();) {
			//		DEBUG_PRINT("xtend %0X", *xtendData);
					if (!(*xtendData)) {
						//Node is null remove
						DEBUG_PRINT("Removing ExtraDataList Node %0X", xtendData);
						ExtraDataList* toDelete = xtendData.RemoveMe();
						if(toDelete) FormHeap_Free(toDelete);
//						++xtendData;
					}
					else {
						++xtendData;
					}
				}
				cur->Cleanup();
				DEBUG_PRINT("After cleanup  %0X  %s", cur->extendData, GetFullName(cur->type));
			}
		}
	}
}

void ExtraContainerChanges::DebugDump()
{
	_MESSAGE("Dumping ExtraContainerChanges");
	gLog.Indent();

	if (data && data->objList)	{
		for (tList<ExtraContainerChanges::EntryData>::Iterator entry = data->objList->Begin(); !entry.End(); ++entry){
			if (!*entry) {
				_MESSAGE("No data!");
				continue;
			}

			_MESSAGE("Type: %s CountDelta: %d [%08X]", GetFullName(entry->type), entry->countDelta, *entry);
			gLog.Indent();
			if (!entry->extendData)
				_MESSAGE("* No extend data *");
			else {
				for(tList<ExtraDataList>::Iterator extendData = entry->extendData->Begin(); !extendData.End(); ++extendData){
					_MESSAGE("Extend Data: [%08X]", extendData);
					gLog.Indent();
					if (*extendData) {
						extendData->DebugDump();
						ExtraCount* xCount = (ExtraCount*)extendData->GetByType(kExtraData_Count);
						if (xCount) {
							_MESSAGE("ExtraCount value : %d", xCount->count);
						}
					}
					else
						_MESSAGE("NULL");

					gLog.Outdent();
				}
			}
			gLog.Outdent();
		}
	}
	gLog.Outdent();
}

ExtraContainerChanges* ExtraContainerChanges::GetForRef(TESObjectREFR* refr)
{
	GetOrCreateExtraContainerChanges(refr); //TODO GetOrCreateExtraContainerChanges should return the pointer
	return (ExtraContainerChanges*)refr->baseExtraList.GetByType(kExtraData_ContainerChanges);
}

ExtraDataList* ExtraContainerChanges::SetEquipped(TESForm* obj, bool bEquipped)
{
	return (ExtraDataList*)ThisStdCall(0x00485FA0, this, obj, bEquipped);

}

UInt32 ExtraContainerChanges::GetAllEquipped(std::vector<EntryData*>& outEntryData, std::vector<ExtraDataList*>& outExtendData)
{
	if (data) {
		for (tList<ExtraContainerChanges::EntryData>::Iterator entry = data->objList->Begin(); !entry.End(); ++entry) {
			if (*entry && entry->extendData) {
				for (tList<ExtraDataList>::Iterator extend = entry->extendData->Begin(); !extend.End(); ++extend) {
					if (*extend) {
						if (extend->IsWorn()) {
							outEntryData.push_back(entry.Get());
							outExtendData.push_back(extend.Get());
						}
					}
				}
			}
		}
	}

	return outEntryData.size();
}

static const UInt32 s_ExtraHealthSize = 0x10;
static const UInt32 s_ExtraHealthVtbl = 0x00A35848;
static const UInt32 s_ExtraChargeSize = 0x10;
static const UInt32 s_ExtraChargeVtbl = 0x00A3586C;
static const UInt32 s_ExtraUsesSize = 0x10;
static const UInt32 s_ExtraUsesVtbl = 0x00A35854;
static const UInt32 s_ExtraPoisonSize = 0x10;
static const UInt32 s_ExtraPoisonVtbl = 0x00A359E0;
static const UInt32 s_ExtraTravelHorseSize = 0x10;
static const UInt32 s_ExtraTravelHorseVtbl = 0x00A359BC;
static const UInt32 s_ExtraLockSize = 0x10;
static const UInt32 s_ExtraLockVtbl = 0x00A357B8;
static const UInt32 s_ExtraSoulSize = 0x10;
static const UInt32 s_ExtraSoulVtbl = 0x00A35878;
static const UInt32 s_ExtraCountSize = 0x10;
static const UInt32 s_ExtraCountVtbl = 0x00A3583C;
static const UInt32 s_ExtraQuickKeySize = 0x10;
static const UInt32 s_ExtraQuickKeyVtbl = 0x00A358E4;
static const UInt32 s_ExtraDataListVtbl = 0x00A350F8;
static const UInt32 s_ExtraTeleportVtbl = 0x00A357DC;
static const UInt32 s_ExtraTeleportSize = 0x10;
static const UInt32 s_ExtraContainerChangesSize = 0x10;
static const UInt32 s_ExtraContainerChangesVtbl = 0x00A35800;
static const UInt32 s_ExtraScriptSize = 0x14;
static const UInt32 s_ExtraScriptVtbl = 0x00A35884;
static const UInt32 s_ExtraOwnershipSize = 0x10;
static const UInt32 s_ExtraOwnershipVtbl = 0x00A35818;
static const UInt32 s_ExtraCannotWearVtbl = 0x00A358CC;
static const UInt32 s_ExtraCannotWearSize = 0x0C;
static const UInt32 s_ExtraRankVtbl = 0x00A35830;
static const UInt32 s_ExtraRankSize = 0x10;


// static
BSExtraData* BSExtraData::Create(UInt8 xType, UInt32 size, UInt32 vtbl)
{
	void* memory = FormHeap_Allocate(size);
	memset(memory, 0, size);
	((UInt32*)memory)[0] = vtbl;
	BSExtraData* xData = (BSExtraData*)memory;
	xData->type = xType;
	return xData;
}

// static
ExtraHealth* ExtraHealth::Create()
{
	ExtraHealth* xHealth = (ExtraHealth*)BSExtraData::Create(kExtraData_Health, s_ExtraHealthSize, s_ExtraHealthVtbl);
	return xHealth;
}

// static
ExtraCharge* ExtraCharge::Create()
{
	ExtraCharge* xCharge = (ExtraCharge*)BSExtraData::Create(kExtraData_Charge, s_ExtraChargeSize, s_ExtraChargeVtbl);
	return xCharge;
}

// static
ExtraUses* ExtraUses::Create()
{
	ExtraUses* xUses = (ExtraUses*)BSExtraData::Create(kExtraData_Uses, s_ExtraUsesSize, s_ExtraUsesVtbl);
	return xUses;
}

// static
ExtraPoison* ExtraPoison::Create()
{
	ExtraPoison* xPoison = (ExtraPoison*)BSExtraData::Create(kExtraData_Poison, s_ExtraPoisonSize, s_ExtraPoisonVtbl);
	return xPoison;
}

ExtraTravelHorse* ExtraTravelHorse::Create()
{
	ExtraTravelHorse* xHorse = (ExtraTravelHorse*)BSExtraData::Create(kExtraData_TravelHorse, s_ExtraTravelHorseSize,
																	  s_ExtraTravelHorseVtbl);
	return xHorse;
}

ExtraWaterHeight* ExtraWaterHeight::Create(float height)
{
	ExtraWaterHeight* xHeight = (ExtraWaterHeight*)FormHeap_Allocate(sizeof(ExtraWaterHeight));
	ThisStdCall(0x0041D920, xHeight, height);		// constructor

	return xHeight;
}

ExtraLock* ExtraLock::Create()
{
	ExtraLock* xLock = (ExtraLock*)BSExtraData::Create(kExtraData_Lock, s_ExtraLockSize, s_ExtraLockVtbl);
	ExtraLock::Data* lockData = (ExtraLock::Data*)FormHeap_Allocate(sizeof(ExtraLock::Data));
	memset(lockData, 0, sizeof(ExtraLock::Data));
	xLock->data = lockData;
	return xLock;
}

ExtraSoul* ExtraSoul::Create()
{
	ExtraSoul* xSoul = (ExtraSoul*)BSExtraData::Create(kExtraData_Soul, s_ExtraSoulSize, s_ExtraSoulVtbl);
	memset(xSoul->padding, 0, 3);
	return xSoul;
}

ExtraScript* ExtraScript::Create(Script* scr)
{
	ExtraScript* xScript = (ExtraScript*)BSExtraData::Create(kExtraData_Script, s_ExtraScriptSize, s_ExtraScriptVtbl);
	xScript->script = scr;
	xScript->eventList = scr ? scr->CreateEventList() : NULL;
	return xScript;
}

ExtraOwnership* ExtraOwnership::Create(TESForm* _owner)
{
	ExtraOwnership* xOwner = (ExtraOwnership*)BSExtraData::Create(kExtraData_Ownership, s_ExtraOwnershipSize, s_ExtraOwnershipVtbl);
	xOwner->owner = _owner;
	return xOwner;
}

ExtraRank* ExtraRank::Create(UInt32 rank)
{
	ExtraRank* xRank = (ExtraRank*)BSExtraData::Create(kExtraData_Rank, s_ExtraRankSize, s_ExtraRankVtbl);
	xRank->rank = rank;
	return xRank;
}


ExtraCount* ExtraCount::Create()
{
	ExtraCount* xCount = (ExtraCount*)BSExtraData::Create(kExtraData_Count, s_ExtraCountSize, s_ExtraCountVtbl);
	return xCount;
}

ExtraCount* ExtraCount::Create(SInt32 count)
{
	ExtraCount* xCount = Create();
	xCount->count = count;
	return xCount;
}

ExtraContainerChanges* ExtraContainerChanges::Create()
{
	ExtraContainerChanges* xChanges = (ExtraContainerChanges*)BSExtraData::Create(kExtraData_ContainerChanges, s_ExtraContainerChangesSize, s_ExtraContainerChangesVtbl);
	xChanges->data = NULL;
	return xChanges;
}

ExtraContainerChanges::Data* ExtraContainerChanges::Data::Create(TESObjectREFR* owner)
{
	Data* data = (Data*)FormHeap_Allocate(sizeof(Data));
	data->owner = owner;
	data->objList = NULL;
	data->totalWeight = -1.0;
	data->armorWeight = -1.0;
	
	return data;
}

void ExtraContainerChanges::Data::RunScripts()
{
	ThisStdCall(0x0048E060, this);

}

ExtraContainerChanges::EntryData* ExtraContainerChanges::EntryData::Create(SInt32 countDelta, TESForm* type)
{
	EntryData* ed = (EntryData*)FormHeap_Allocate(sizeof(EntryData));
	ed->countDelta = countDelta;
	ed->type = type;
	ed->extendData = NULL;

	return ed;
}

ExtraQuickKey* ExtraQuickKey::Create()
{
	ExtraQuickKey* xQuickKey = (ExtraQuickKey*)BSExtraData::Create(kExtraData_QuickKey, s_ExtraQuickKeySize, s_ExtraQuickKeyVtbl);
	memset(xQuickKey->pad, 0, 3);
	return xQuickKey;
}

ExtraDataList* ExtraDataList::Create()
{
	ExtraDataList* xData = (ExtraDataList*)FormHeap_Allocate(sizeof(ExtraDataList));
	memset(xData, 0, sizeof(ExtraDataList));
	((UInt32*)xData)[0] = s_ExtraDataListVtbl;
	return xData;
}

ExtraTeleport* ExtraTeleport::Create()
{
	ExtraTeleport* tele = (ExtraTeleport*)BSExtraData::Create(kExtraData_Teleport, s_ExtraTeleportSize, s_ExtraTeleportVtbl);
	
	// create data
	ExtraTeleport::Data* data = (ExtraTeleport::Data*)FormHeap_Allocate(sizeof(ExtraTeleport::Data));
	data->linkedDoor = NULL;
	data->yRot = -0.0;
	data->xRot = 0.0;
	data->x = 0.0;
	data->y = 0.0;
	data->z = 0.0;
	data->zRot = 0.0;

	tele->data = data;
	return tele;
}

ExtraCannotWear* ExtraCannotWear::Create()
{
	return (ExtraCannotWear*)BSExtraData::Create(kExtraData_CannotWear, s_ExtraCannotWearSize, s_ExtraCannotWearVtbl);
}

ExtraTimeLeft* ExtraTimeLeft::Create(float timeLeft)
{
	ExtraTimeLeft* xTime = (ExtraTimeLeft*)FormHeap_Allocate(sizeof(ExtraTimeLeft));

	ThisStdCall(0x00429EC0, xTime, timeLeft);		// constructor


	return xTime;
}

ExtraCellWaterType* ExtraCellWaterType::Create(TESWaterForm* water)
{
	static const UInt32 s_ctrAddr = 0x0041D9A0;


	ExtraCellWaterType* xWater = (ExtraCellWaterType*)FormHeap_Allocate(sizeof(ExtraCellWaterType));
	ThisStdCall(s_ctrAddr, xWater);
	xWater->waterType = water;
	return xWater;
}

ExtraCellMusicType* ExtraCellMusicType::Create(UInt32 _musicType)
{
	static const UInt32 s_ctrAddr = 0x0041D960;

	ASSERT(_musicType < kMusicType_MAX);
	ExtraCellMusicType* xMusic = (ExtraCellMusicType*)FormHeap_Allocate(sizeof(ExtraCellMusicType));
	ThisStdCall(s_ctrAddr, xMusic, _musicType);
	return xMusic;
}
