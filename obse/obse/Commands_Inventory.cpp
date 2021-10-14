#include "Commands_Inventory.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "Hooks_Gameplay.h"

#include "GameData.h"
#include "InternalSerialization.h"
#include "ArrayVar.h"
#include "GameMenus.h"
#include "InventoryReference.h"
#include "obse_common/SafeWrite.h"
#include "GameOSDepend.h"

static const _Cmd_Execute Cmd_AddItem_Execute = (_Cmd_Execute)0x00507320;
static const _Cmd_Execute Cmd_RemoveItem_Execute = (_Cmd_Execute)0x00513810;
static const _Cmd_Execute Cmd_EquipItem_Execute = (_Cmd_Execute)0x005162B0;
static const _Cmd_Execute Cmd_UnequipItem_Execute = (_Cmd_Execute)0x005164C0;


static void PrintItemType(TESForm * form)
{
	Console_Print("%s (%s)", GetFullName(form), GetObjectClassName(form));
}

// TODO: get rid of this duplicate code
// make an iterator, maybe?

typedef std::vector<ExtraContainerChanges::EntryData*> ExtraDataVec;
typedef std::map<TESForm*, UInt32> ExtraContainerMap;

class ExtraContainerInfo
{
public:
	ExtraContainerInfo(tList<ExtraContainerChanges::EntryData>* entryList): m_map(), m_vec()
	{
		m_vec.reserve(128);
		if (entryList) {
			entryList->Visit(*this);
		}
	}

	bool Accept(ExtraContainerChanges::EntryData* data)
	{
		if (data) {
			m_vec.push_back(data);
			m_map[data->type] = m_vec.size()-1;
		}
		return true;
	}

	bool IsValidContainerData(TESContainer::Data* containerData, SInt32& numObjects)
	{
		if (containerData) {
			numObjects = containerData->count;

			// make sure it isn't a leveled object
			if(Oblivion_DynamicCast(containerData->type, 0, RTTI_TESForm, RTTI_TESLevItem, 0))
				return false;

			ExtraContainerMap::iterator it = m_map.find(containerData->type);
			ExtraContainerMap::iterator itEnd = m_map.end();
			if (it != itEnd) {
				UInt32 index = (*it).second;
				ExtraContainerChanges::EntryData* extraData = m_vec[index];
				if(extraData)
				{
					numObjects += extraData->countDelta;
				}
				// let's clear this item from the vector
				// this way we don't bother to look for it in the second step
				m_vec[index] = NULL;
			}

			// is at least one still here?
			if(numObjects > 0)
			{
				//PrintItemType(containerData->type);
				return true;
			}
		}
		return false;
	}

	ExtraDataVec	m_vec;
	ExtraContainerMap m_map;
};

class ContainerCountIf
{
	ExtraContainerInfo& m_info;
public:
	ContainerCountIf(ExtraContainerInfo& info) : m_info(info) {}

	bool Accept(TESContainer::Data* containerData) const
	{
		SInt32 numObjects = 0; // not needed in this count
		return m_info.IsValidContainerData(containerData, numObjects);
	}
};

static bool Cmd_GetNumItems_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	EnterCriticalSection(g_extraListMutex);

	UInt32	count = 0;

	// get pointers
	ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(thisObj->baseExtraList.GetByType(kExtraData_ContainerChanges));

	// initialize a map of the ExtraData information
	// this will walk through the containerChanges once
	ExtraContainerInfo info(containerChanges ? containerChanges->data->objList : NULL);

	TESContainer* container = thisObj->GetContainer();

	// first walk the base container
	if(container) {
		ContainerVisitor visitContainer(&container->list);
		ContainerCountIf counter(info);
		count = visitContainer.CountIf(counter);
	}
	DEBUG_PRINT("GetNumItems %d", count);
	// now walk the remaining items
	if (containerChanges && containerChanges->data->objList) {
		for (tList<ExtraContainerChanges::EntryData>::Iterator iter = containerChanges->data->objList->Begin(); !iter.End(); ++iter) {
			if (*iter) {
				DEBUG_PRINT("%s  %d", GetFullName(iter->type), iter->countDelta);
				if (iter->countDelta > 0) count++;
			}
		}
	}
	DEBUG_PRINT("GetNumItems 1 %d", count);
	*result = count;

	LeaveCriticalSection(g_extraListMutex);

	return true;
}

static TESForm * GetItemByIdx(TESObjectREFR * thisObj, UInt32 objIdx, SInt32 * outNumItems)
{
	if(!thisObj) return NULL;

	UInt32	count = 0;

	// get pointers
	ExtraContainerChanges	* containerChanges = static_cast <ExtraContainerChanges *>(thisObj->baseExtraList.GetByType(kExtraData_ContainerChanges));

	// initialize a map of the ExtraData information
	// this will walk through the containerChanges once
	ExtraContainerInfo info(containerChanges ? containerChanges->data->objList : NULL);

	TESContainer			* container = NULL;
	TESForm	* baseForm = thisObj->GetBaseForm();
	if(baseForm)
	{
		container = (TESContainer *)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESContainer, 0);
	}

	// first walk the base container
	if(container)
	{
		for(TESContainer::Entry	* containerEntry = &container->list; containerEntry; containerEntry = containerEntry->next)
		{
			TESContainer::Data	* containerData = containerEntry->data;
			SInt32 numObjects = 0;
			if (info.IsValidContainerData(containerData, numObjects)) {
				if(count == objIdx)
				{
					if(outNumItems) *outNumItems = numObjects;
					return containerData->type;
				}
				count++;
			}
		}
	}

	// now walk the remaining items in the map

	ExtraDataVec::iterator itEnd = info.m_vec.end();
	ExtraDataVec::iterator it = info.m_vec.begin();
	while (it != itEnd) {
		ExtraContainerChanges::EntryData* extraData = (*it);
		if (extraData && (extraData->countDelta > 0)) {
			if(count == objIdx)
			{
				if(outNumItems) *outNumItems = extraData->countDelta;
				return extraData->type;
			}
			count++;
		}
		++it;
	}
	if(outNumItems) *outNumItems = 0;

	return NULL;
}

struct ContainerFormInfo
{
	ContainerFormInfo() : m_baseCount(0), m_entryData(NULL) {}
	ExtraContainerChanges::EntryData* m_entryData;
	SInt32	m_baseCount;
};

void GetContainerFormInfo(TESObjectREFR* thisObj, TESForm* formToFind, ContainerFormInfo& formInfo)
{
	if (!thisObj || !formToFind) return;
	TESContainer* container = (TESContainer *)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_TESContainer, 0);
	if (container) {
		TESContainer::Data* data = container->DataByType(formToFind);
		if (data) {
			formInfo.m_baseCount = data->count;
		}
	}

	ExtraContainerChanges	* containerChanges = static_cast <ExtraContainerChanges *>(thisObj->baseExtraList.GetByType(kExtraData_ContainerChanges));
	if (containerChanges) {
		formInfo.m_entryData = containerChanges->GetByType(formToFind);
	}
}

UInt32 GetContainerFormConfigCount(TESObjectREFR* thisObj, TESForm* formToFind)
{
	UInt32 nConfigs = 0;
	ContainerFormInfo formInfo;
	GetContainerFormInfo(thisObj, formToFind, formInfo);
	if (formInfo.m_baseCount > 0) nConfigs++;  //TODO nConfig should be increased by the basecount?

	if (formInfo.m_entryData) {
		//TODO use the countDelta?
		nConfigs += formInfo.m_entryData->extendData->Count();
	}

	return nConfigs;
}

ExtraDataList* GetNthContainerFormConfig(TESObjectREFR* thisObj, TESForm* formToFind, UInt32 index)
{
	// the 0 slot is always for the base configuration and is expected to have no overrides
	if (index == 0) return NULL;

	ContainerFormInfo formInfo;
	GetContainerFormInfo(thisObj, formToFind, formInfo);
	if (formInfo.m_entryData) {
		// adjust the index down by one as the base object has no overrides
		// and all of the others are assumed to be after it
		return  formInfo.m_entryData->extendData->GetNthItem(index - 1);

	}
	return NULL;
}

static bool Cmd_GetInventoryItemType_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	UInt32	objIdx = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &objIdx)) return true;

	EnterCriticalSection(g_extraListMutex);

	TESForm	* type = GetItemByIdx(thisObj, objIdx, NULL);
	if(type)
	{
		UInt32	id = type->refID;
		if (*g_bConsoleMode)
			Console_Print("GetInvObj %d >> %s %08x (%s)", objIdx, GetFullName(type), id, GetObjectClassName(type));

		*((UInt32 *)result) = id;
	}
	else
	{
		//Console_Print("couldn't get item");
	}

	LeaveCriticalSection(g_extraListMutex);

	return true;
}

enum
{
	kSlot_None = -1,
	// lower slots are TESBipedModelForm::kPart_*
	kSlot_RightRing = TESBipedModelForm::kPart_RightRing,
	kSlot_LeftRing,
	kSlot_Torch = TESBipedModelForm::kPart_Torch,	// 14
	kSlot_Weapon = TESBipedModelForm::kPart_Max,	// 16
	kSlot_Ammo,
	kSlot_RangedWeapon,
};

bool ItemSlotMatches(TESForm* pForm, UInt32 slot)
{
	bool bMatches = false;
	if (pForm) {
		if (slot == kSlot_Weapon && Oblivion_DynamicCast(pForm, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0) != 0) {
			bMatches = true;
		} else if (slot == kSlot_Ammo && Oblivion_DynamicCast(pForm, 0, RTTI_TESForm, RTTI_TESAmmo, 0) != 0) {
			bMatches = true;
		} else if (slot == kSlot_Torch) {
			TESObjectLIGH * pLight = OBLIVION_CAST(pForm, TESForm, TESObjectLIGH);
			if (pLight && pLight->IsCarriable()) {
				bMatches = true;
			}
		} else {
			TESBipedModelForm	* pBip = (TESBipedModelForm *)Oblivion_DynamicCast(pForm, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
			if (pBip) {
				// if slot is an official slot match it
				if (slot < TESBipedModelForm::kPart_Max) {
					UInt32 inMask = TESBipedModelForm::MaskForSlot(slot);
					if ((inMask & pBip->partMask) != 0) {
						bMatches = true;
					}
				}
				// if slot is an unoffical multi-slot - it must match exactly
				else if (pBip->GetSlot() == slot){
					bMatches = true;
				}
			}
		}
	}
	return bMatches;
}

static UInt32 GetItemSlot(TESForm* type)
{
	if (type) {
		TESBipedModelForm	* bip = (TESBipedModelForm *)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
		if (bip) {
			return bip->GetSlot();
		}
		else if (Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0) != 0) {
			return kSlot_Weapon;
		} else if (Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESAmmo, 0) != 0) {
			return kSlot_Ammo;
		} else {
			TESObjectLIGH* pLight = OBLIVION_CAST(type, TESForm, TESObjectLIGH);
			if (pLight && pLight->IsCarriable()) {
				return kSlot_Torch;
			}
		}
	}
	return kSlot_None;
}

static UInt32 GetItemSlotMask(TESForm * type)
{
	UInt32	result = 0;

	if(type)
	{
		TESBipedModelForm	* bip = (TESBipedModelForm *)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
		TESObjectWEAP		* weap = (TESObjectWEAP *)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0);

		if(bip)
		{
			result = bip->partMask;
		}
		else if(weap)
		{
			result = (weap->type == TESObjectWEAP::kType_Bow) ? (1 << kSlot_RangedWeapon) : (1 << kSlot_Weapon);
		}
		else if(Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESAmmo, 0))
		{
			result = (1 << kSlot_Ammo);
		}
	}

	return result;
}

class FoundEquipped
{
public:
	virtual bool Found(ExtraContainerChanges::EntryData* entryData, double *result, ExtraDataList* extendData) = 0;
};

static inline bool IsRingSlot(UInt32 slot)
{
	return slot == kSlot_RightRing || slot == kSlot_LeftRing;
}

static inline bool IsOppositeRing(UInt32 slot, UInt32 slotIdx) {
	return (slot == kSlot_RightRing && slotIdx == kSlot_LeftRing) || (slot == kSlot_LeftRing && slotIdx == kSlot_RightRing);
}

static bool FindEquipped(TESObjectREFR* thisObj, UInt32 slotIdx, FoundEquipped* foundEquippedFunctor, double* result) {
	ExtraContainerChanges* containerChanges = static_cast <ExtraContainerChanges*>(thisObj->baseExtraList.GetByType(kExtraData_ContainerChanges));
	if(containerChanges && containerChanges->data && containerChanges->data->objList){
		for(tList<ExtraContainerChanges::EntryData>::Iterator entry = containerChanges->data->objList->Begin(); !entry.End(); ++entry){
			// do the fast check first (an object must have extend data to be equipped)
			if(*entry && entry->extendData && entry->type){
				if (ItemSlotMatches(entry->type, slotIdx)) {
					// ok, it's the right type, now is it equipped?
					for(tList<ExtraDataList>::Iterator extend = entry->extendData->Begin(); !extend.End(); ++extend){
						if (*extend) {
							// handle rings
							bool bFound = false;
							if (IsRingSlot(slotIdx))
							{
								if (slotIdx == kSlot_LeftRing && extend->HasType(kExtraData_WornLeft))
								{
									bFound = true;
								}
								else if (slotIdx == kSlot_RightRing && extend->HasType(kExtraData_Worn))
								{
									bFound = true;
								}
							}
							else if (extend->HasType(kExtraData_Worn))
							{
								bFound = true;
							}

							if (bFound)
							{
								return foundEquippedFunctor->Found(entry.Get(), result, extend.Get());
							}
						}
					}
				}
			}
		}
	}
	return false;
}

static bool FindEquippedByMask(TESObjectREFR* thisObj, UInt32 targetMask, UInt32 targetData, FoundEquipped* foundEquippedFunctor, double* result)
{
	ExtraContainerChanges	* containerChanges = static_cast <ExtraContainerChanges *>(thisObj->baseExtraList.GetByType(kExtraData_ContainerChanges));
	if(containerChanges && containerChanges->data && containerChanges->data->objList) {
		for(tList<ExtraContainerChanges::EntryData>::Iterator  entry = containerChanges->data->objList->Begin(); !entry.End(); ++entry)	{
			// do the fast check first (an object must have extend data to be equipped)
			if(*entry && entry->extendData && entry->type) {
				// does it match the slot?
				UInt32	slotMask = GetItemSlotMask(entry->type);
				if((slotMask & targetMask) == targetData)	{
					for(tList<ExtraDataList>::Iterator  extend = entry->extendData->Begin(); !extend.End(); ++extend){
						if(*extend){
							// is it equipped?
							bool	isEquipped = false;

							isEquipped = extend->HasType(kExtraData_Worn);

							// special-case for left ring
							if(!isEquipped)
								isEquipped = (slotMask & (1 << TESBipedModelForm::kPart_LeftRing)) && extend->HasType(kExtraData_WornLeft);

							if(isEquipped)
							{
								return foundEquippedFunctor->Found(entry.Get(), result, extend.Get());
							}
						}
					}
				}
			}
		}
	}

	return false;
}

class feGetObject : public FoundEquipped
{
public:
	feGetObject() {}
	~feGetObject() {}
	virtual bool Found(ExtraContainerChanges::EntryData* entryData, double *result, ExtraDataList* extendData) {
		UInt32* refResult = (UInt32*) result;
		if (entryData) {
			// cool, we win
			*refResult = entryData->type->refID;
			//Console_Print("refID = %08X (%s)", *refResult, GetFullName(entry->data->type));
			return true;
		}
		return false;
	}
};

static bool Cmd_GetEquipmentSlotType_Execute(COMMAND_ARGS)
{
	UInt32	* refResult = (UInt32 *)result;

	*refResult = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	UInt32	slotIdx = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &slotIdx)) return true;
	feGetObject getObject;

	bool bFound = FindEquipped(thisObj, slotIdx, &getObject, result);
	if (IsConsoleMode())
	{
		if (!bFound)
			Console_Print("Nothing equipped in that slot.");
		else
		{
			TESForm* eqObj = LookupFormByID(*refResult);
			if (eqObj)
				Console_Print("GetEquippedObject %d >> %s (%08x)", slotIdx, GetFullName(eqObj), eqObj->refID);
		}
	}
	return true;
}

static bool Cmd_GetEquipmentSlotMask_Execute(COMMAND_ARGS)
{
	UInt32	* refResult = (UInt32 *)result;
	*refResult = 0;

	if(!thisObj) return true;

	UInt32	slotMask = 0;
	UInt32	slotData = 0;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &slotMask, &slotData)) return true;
	if(!slotData) slotData = slotMask;

	feGetObject	getObject;
	FindEquippedByMask(thisObj, slotMask, slotData, &getObject, result);

	return true;
}

enum {
	// basic values from 0 - 99
	kVal_Weight = 0,
	kVal_GoldValue,
	kVal_Health,
	kVal_Slot,
	kVal_Charge,
	kVal_QuestItem,
	kVal_Enchantment,
	kVal_Name,
	kVal_Model,
	kVal_Icon,
	kVal_Quality,
	kVal_CurHealth,
	// weapon values from 100 - 149
	kVal_Damage = 100,
	kVal_Reach,
	kVal_Speed,
	kVal_WeaponType,
	kVal_IgnoresResist,
	kVal_WeaponPoison,
	// for armor 150 - 199
	kVal_ArmorRating = 150,
	kVal_ArmorType,
	// for Soul Gems 200-209
	kVal_Soul = 200,
	kVal_SoulCapacity,
	// for ingredients 210 - 219
	kVal_Food = 210,
	// for Alchemy Items 220 - 229
	kVal_Poison = 220,

	// for biped model (clothing and armor) 230-239
	kVal_BipedMale = 230,
	kVal_BipedFemale,
	kVal_GroundMale,
	kVal_GroundFemale,
	kVal_IconMale,
	kVal_IconFemale,
	kVal_Playable,
	// for books
	kVal_BookIsScroll = 250,
	kVal_BookCantBeTaken,
	kVal_BookTeaches,
	kVal_ApparatusType = 260,
	kVal_TimeLeft
};

static bool GetWeaponBaseValue(TESForm * type, UInt32 valueType, double * result)
{
	TESObjectWEAP* weapon = (TESObjectWEAP*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0);
	TESAmmo* ammo = NULL;
	if (!weapon) {
		ammo = (TESAmmo*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESAmmo, 0);
		if (!ammo) {
			return false;
		}
	}

	switch (valueType) {
		case kVal_Reach:
		{
			if (weapon) {
					*result = weapon->reach;
					return true;
			}
			break;
		}

		case kVal_Speed:
		{
			if (weapon) {
				*result = weapon->speed;
				return true;
			} else if (ammo) {
				*result = ammo->speed;
				return true;
			}
			break;
		}

		case kVal_WeaponType:
		{
			if (weapon) {
				*result = weapon->type;
				return true;
			}
		}

		case kVal_IgnoresResist:
		{
			if (weapon) {
				*result = weapon->ignoreNormalWeaponResist;
				return true;
			} else if (ammo) {
				*result = ammo->ignoreNormalWeaponResist;
				return true;
			}
		}
	}
	return false;
}

static bool GetBookValue(TESObjectBOOK* book, UInt32 whichValue, double* result)
{
	if (!book || !result) return true;
	*result = 0;

	switch(whichValue) {
		case kVal_BookIsScroll:
			{
				*result = book->IsScroll() ? 1 : 0;
				break;
			}
		case kVal_BookCantBeTaken:
			{
				*result = book->CantBeTaken() ? 1 : 0;
				break;
			}
		case kVal_BookTeaches:
			{
				*result = book->Teaches();
				break;
			}
		default:
			break;
	}
	return true;
}

enum EMode {
	kGet = 0,
	kSet,
	kMod,
	kCompare,
	kCopy,
} ;

static bool GetBaseValue(TESForm * type, UInt32 valueType, double * result)
{
	if (!type || !result) return true;

	*result = 0;

	switch(valueType) {
		case kVal_Weight:
		{
			TESObjectAPPA * appa = (TESObjectAPPA*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectAPPA, 0);
			TESWeightForm* weightForm = (TESWeightForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESWeightForm, 0);
			if (weightForm) {
				*result = weightForm->weight;
				return true;
			}
			break;
		}
		case kVal_GoldValue:
		{
			switch (type->typeID)
			{
			case kFormType_Ingredient:
				{
					IngredientItem* ingredient = (IngredientItem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_IngredientItem, 0);
					if (ingredient) {
						*result = ingredient->value;
						return true;
					}
					break;
				}
			case kFormType_AlchemyItem:
				{
					AlchemyItem* alchItem = (AlchemyItem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
					if (alchItem) {
						*result = alchItem->GetGoldValue();
						return true;
					}
					break;
				}
			default:
				TESValueForm* valueForm = (TESValueForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESValueForm, 0);
				if (valueForm) {
					*result = valueForm->value;
					return true;
				}
			}
			break;
		}
		case kVal_Health:
		{
			TESHealthForm* healthForm = (TESHealthForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESHealthForm, 0);
			if (healthForm) {
				*result = healthForm->health;
				return true;
			}
			break;
		}

		case kVal_Slot:
		{
			UInt32 slot = GetItemSlot(type);
			if (slot != kSlot_None) {
				*result = slot;
				return true;
			}
			break;
		}

		case kVal_Charge:
		{
			TESEnchantableForm* enchantForm = (TESEnchantableForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESEnchantableForm, 0);
			if (enchantForm) {
				*result = enchantForm->enchantment;
				return true;
			}
			break;
		}

		case kVal_QuestItem :
		{
			*result = type->IsQuestItem() ? 1 : 0;
			return true;
		}

		case kVal_Enchantment:
		{
			TESEnchantableForm* enchantForm = (TESEnchantableForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESEnchantableForm, 0);
			if (enchantForm) {
				UInt32* refResult = (UInt32*)result;
				if (!enchantForm->enchantItem) return true;
				*refResult = enchantForm->enchantItem->refID;
				return true;
			}
			break;
		}

		case kVal_Damage:
		{
			TESAttackDamageForm* damageForm = (TESAttackDamageForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESAttackDamageForm, 0);
			if (damageForm) {
				*result = damageForm->damage;
				return true;
			}
			break;
		}

		case kVal_Reach:
		case kVal_Speed:
		case kVal_WeaponType:
		case kVal_IgnoresResist:
		{
			return GetWeaponBaseValue(type, valueType, result);
		}

		case kVal_ArmorRating:
		{
			TESObjectARMO* armor = (TESObjectARMO*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectARMO, 0);
			if (armor) {
				*result = armor->armorRating;
				return true;
			}
			break;
		}

		case kVal_ArmorType:
		{
			TESObjectARMO* armor = (TESObjectARMO*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectARMO, 0);
			if (armor) {
				*result = (armor->IsHeavyArmor()) ? 1 : 0;
				return true;
			}
			break;
		}

		case kVal_Soul:
		{
			TESSoulGem* soulGem = (TESSoulGem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESSoulGem, 0);
			if (soulGem) {
				*result = soulGem->soul;
				return true;
			}
			break;
		}

		case kVal_SoulCapacity:
		{
			TESSoulGem* soulGem = (TESSoulGem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESSoulGem, 0);
			if (soulGem) {
				*result = soulGem->capacity;
				return true;
			}
			break;
		}

		case kVal_Food :
		{
			IngredientItem* ingredient = (IngredientItem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_IngredientItem, 0);
			if (ingredient) {
				*result = ingredient->IsFood() ? 1 : 0;
				return true;
			} else {
				AlchemyItem* alchemyItem = (AlchemyItem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
				if (alchemyItem) {
					*result = alchemyItem->IsFood() ? 1 : 0;
					return true;
				}
			}
			break;
		}

		case kVal_Playable:
		{
			TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
			if (biped) {
				*result = biped->IsPlayable() ? 1 : 0;
				return true;
			}
			break;
		}

		case kVal_Poison :
		{
			AlchemyItem* alchemyItem = (AlchemyItem*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
			if (alchemyItem) {
				*result = alchemyItem->IsPoison() ? 1 : 0;
				return true;
			}
			break;
		}

		case kVal_BookIsScroll:
		case kVal_BookCantBeTaken:
		case kVal_BookTeaches:
		{
			TESObjectBOOK* book = (TESObjectBOOK*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectBOOK, 0);
			if (book) {
				return GetBookValue(book, valueType, result);
			}
			break;
		}

		case kVal_ApparatusType:
			{
				TESObjectAPPA* apparatus = (TESObjectAPPA*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESObjectAPPA, 0);
				if (apparatus) {
					*result = apparatus->appaType;
					return true;
				}
				break;
			}

		case kVal_Quality:
			{
				TESQualityForm* quality = (TESQualityForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESQualityForm, 0);
				if (quality) {
					*result = quality->quality;
					return true;
				}
				break;
			}
	}

	return true;
}

static bool GetObjectValue(COMMAND_ARGS, UInt32 valueType)
{
	*result = 0;
	TESForm* form = 0;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	bool bGotResult = GetBaseValue(form, valueType, result);
	return true;
}

static bool Cmd_GetObjectValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESForm* form = 0;
	UInt32	valueType = 0;

	ExtractArgs(PASS_EXTRACT_ARGS, &valueType, &form);
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	bool bGotResult = GetBaseValue(form, valueType, result);

	return true;
}

static bool Cmd_GetBaseObject_Execute(COMMAND_ARGS)
{
	UInt32	* refResult = (UInt32 *)result;
	*refResult = 0;

	if(!thisObj || !thisObj->baseForm) return true;

	*refResult = thisObj->baseForm->refID;
	if (IsConsoleMode())
		Console_Print("GetBaseObject >> %08x", *refResult);

	return true;
}

static bool Cmd_GetType_Execute(COMMAND_ARGS)
{
	*result= 0;
	TESForm* form = 0;

	if(!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form)) return true;
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	bool bDump = false;
	if (bDump) {
		DumpClass(form);
	}

	*result = form->typeID;

	return true;
}

static bool GetCurrentValue(BaseExtraList& extraList, UInt32 whichValue, double* result)
{
	UInt8 extraType = 0;	// set the extra type based on the value requested
	switch (whichValue) {
		case kVal_Health:
		case kVal_CurHealth:
			extraType = kExtraData_Health;
			break;
		case kVal_Charge:
			extraType = kExtraData_Charge;
			break;
		case kVal_WeaponPoison:
			extraType = kExtraData_Poison;
			break;
		case kVal_Soul:
			extraType = kExtraData_Soul;
			break;
		case kVal_TimeLeft:
			extraType = kExtraData_TimeLeft;
			break;
	}

	if (extraType != 0) {
		// now we need to see if there is an extra value form of the correct type
		BSExtraData* extraData = extraList.GetByType(extraType);
		if (extraData) {
			switch(whichValue) {
				case kVal_Health:
				case kVal_CurHealth:
				{
					ExtraHealth* xHealth = (ExtraHealth*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraHealth, 0);
					if (xHealth) {
						*result = xHealth->health;
						return true;
					}
					break;
				}
				case kVal_Charge:
				{
					ExtraCharge* xCharge = (ExtraCharge*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraCharge, 0);
					if (xCharge) {
						*result = xCharge->charge;
						return true;
					}
					break;
				}

				case kVal_WeaponPoison:
				{
					ExtraPoison* xPoison = (ExtraPoison*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraPoison, 0);
					if (xPoison) {
						UInt32* refResult = (UInt32*)result;
						*refResult = xPoison->poison->refID;
						return true;
					}
					break;
				}

				case kVal_Soul:
				{
					ExtraSoul* xSoul = (ExtraSoul*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraSoul, 0);
					if (xSoul) {
						*result = xSoul->soul;
						return true;
					}
					break;
				}

				case kVal_TimeLeft:
					{
						ExtraTimeLeft* xTime = OBLIVION_CAST(extraData, BSExtraData, ExtraTimeLeft);
						if (xTime) {
							*result = xTime->time;
							return true;
						}
						else {
							*result = -1.0;
						}
					}
					break;

				default:
					break;
			}
		}
	}
	return false;
}

static bool GetCurrentValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	if (!thisObj) return true;
	BaseExtraList& extraList = thisObj->baseExtraList;
	if (GetCurrentValue(extraList, whichValue, result)) {
		return true;
	} else {
		return GetBaseValue(thisObj->baseForm, whichValue, result);
	}
}

static bool Cmd_GetCurrentValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	UInt32 valueType = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &valueType)) return true;
	return GetCurrentValue_Execute(PASS_COMMAND_ARGS, valueType);
}

static bool Cmd_GetCurrentHealth_Execute(COMMAND_ARGS)
{
	return GetCurrentValue_Execute(PASS_COMMAND_ARGS, kVal_Health);
}

static bool Cmd_GetCurrentCharge_Execute(COMMAND_ARGS)
{
	return GetCurrentValue_Execute(PASS_COMMAND_ARGS, kVal_Charge);
}

static bool Cmd_GetCurrentSoulLevel_Execute(COMMAND_ARGS)
{
	return GetCurrentValue_Execute(PASS_COMMAND_ARGS, kVal_Soul);
}

class feGetCurrentValue : public FoundEquipped
{
	UInt32 m_valueType;
public:
	feGetCurrentValue(UInt32 valueType) : m_valueType(valueType) {}
	~feGetCurrentValue() {}
	bool Found(ExtraContainerChanges::EntryData* entryData, double *result, ExtraDataList* extendData) {
		UInt32* refResult = (UInt32*) result;
		if (extendData) {
			if (GetCurrentValue(*extendData, m_valueType, result)) {
				return true;
			} else {
				return GetBaseValue(entryData->type, m_valueType, result);
			}
		}
		return false;
	}
};

static bool GetEquippedCurrentValue_Execute(COMMAND_ARGS, UInt32 valueType)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	UInt32 slotIdx = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &slotIdx)) return true;

	feGetCurrentValue getCurrentValue(valueType);
	bool bFound = FindEquipped(thisObj, slotIdx, &getCurrentValue, result);
	return true;
}

static bool Cmd_GetEquippedCurrentValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if (!thisObj) return true;

	UInt32 valueType = 0;
	UInt32 slotIdx = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &valueType, &slotIdx)) return true;

	feGetCurrentValue getCurrentValue(valueType);
	bool bFound = FindEquipped(thisObj, slotIdx, &getCurrentValue, result);
	return true;
}

class feGetBaseValue : public FoundEquipped
{
	UInt32 m_valueType;
public:
	feGetBaseValue(UInt32 valueType) : m_valueType(valueType) {}
	~feGetBaseValue() {}
	bool Found(ExtraContainerChanges::EntryData* entryData, double *result, ExtraDataList* extendData) {
		UInt32* refResult = (UInt32*) result;
		if (entryData) {
			return GetBaseValue(entryData->type, m_valueType, result);
		}
		return false;
	}
};

static bool Cmd_GetEquippedObjectValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	UInt32 valueType = 0;
	UInt32 slotIdx = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &valueType, &slotIdx)) return true;

	feGetBaseValue getBaseValue(valueType);
	bool bFound = FindEquipped(thisObj, slotIdx, &getBaseValue, result);
	return true;
}

#if 0

class feSetCurrentValue : public FoundEquipped
{
	UInt32 m_valueType;
	float m_value;
public:
	feSetCurrentValue(UInt32 valueType, float value) : m_valueType(valueType), m_value(value) {}
	~feSetCurrentValue() {}
	bool Found(ExtraContainerChanges::EntryData* entryData, double *result) {
		if (m_valueType == kVal_Health) {
			// get the object's base health
			TESHealthForm* healthForm = (TESHealthForm*)Oblivion_DynamicCast(entryData->type, 0, RTTI_TESForm, RTTI_TESHealthForm, 0);
			if (healthForm) {
				UInt32 baseHealth = healthForm->health;
				// see if there is a current override
				BaseExtraList *baseExtraList = entryData->extendData->data;
				BSExtraData* extraData = baseExtraList->GetByType(kExtraData_Health);
				ExtraHealth* health = (ExtraHealth*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraHealth, 0);
				if (m_value == (float)baseHealth) {
					// putting back to base health
					if (health) {
						// we need to remove the health
						if (baseExtraList->Remove(health)) {
							FormHeap_Free(health);
							health = NULL;
						}
					}
				} else if (health) {
					// simply change the value
					health->health = m_value;
					// mark as modified?
				} else {
					health = ExtraHealth::Create();
					health->health = m_value;
					if (!baseExtraList->Add(health)) {
						FormHeap_Free(health);
					}
					bool bAddedType = baseExtraList->HasType(kExtraData_Health);
					//Console_Print("Health added: %d %08x %f", bAddedType, health, health->health);
					ExtraHealth* nuHealth = (ExtraHealth*)Oblivion_DynamicCast(baseExtraList->GetByType(kExtraData_Health), 0, RTTI_BSExtraData, RTTI_ExtraHealth, 0);
					//Console_Print("Health found: %d %08x %f", bAddedType, nuHealth, nuHealth->health);
				}
			}
		}
		return true;
	}
};

static bool Cmd_SetEquippedCurrentValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	UInt32 valueType = 0;
	float value = 0;
	UInt32 slotIdx = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &valueType, &value, &slotIdx)) return true;

	feSetCurrentValue setCurrentValue(valueType, value);
	bool bFound = FindEquipped(thisObj, slotIdx, &setCurrentValue, result);
	return true;
}

#endif

static const bool bClonedOnlyT = true;
static const bool bClonedOnlyF = false;
static const bool bModT = true;
static const bool bModF = false;
class MagicItem;

void ChangePathString(BSStringT& base, const char* change, bool bForMod)
{
	if (!bForMod) {
		base.Set(change);
	} else {
	}
}

class ChangeValueState
{
public:
	enum EValType {
		eInteger = 0,
		eFloat,
		eBool,
		eMagicItem,
		eString,
		eEffectSetting,
	};

	ChangeValueState(UInt32 whichVal, bool bForMod) :
		m_whichVal(whichVal), m_valType(ValTypeForVal(m_whichVal)), m_bForMod(bForMod),
		m_intVal(0), m_floatVal(0), m_magicItem(NULL), m_effectSetting(NULL) {
			memset(m_stringVal, 0, 256);
		}

	~ChangeValueState() {
	}

	static EValType ValTypeForVal(UInt32 whichVal) {
		switch(whichVal) {
			case kVal_Weight:
			case kVal_Reach:
			case kVal_Speed:
			case kVal_Quality:
				return eFloat;

			case kVal_GoldValue:
			case kVal_Health:
			case kVal_Slot:
			case kVal_Charge:
			case kVal_Damage:
			case kVal_WeaponType:
			case kVal_ArmorRating:
			case kVal_ArmorType:
			case kVal_Soul:
			case kVal_SoulCapacity:
			case kVal_ApparatusType:
			case kVal_BookTeaches:
				return eInteger;

			case kVal_QuestItem:
			case kVal_IgnoresResist:
			case kVal_Food:
			case kVal_Poison:
			case kVal_BookIsScroll:
			case kVal_Playable:
				return eBool;

			case kVal_Enchantment:
				return eMagicItem;

				return eEffectSetting;

			case kVal_Name:
				return eString;

			default:
				return eFloat;
		}
	}

	// used by
	UInt32 WhichValue() { return m_whichVal; }
	EValType ValueType() { return m_valType; }

	bool UseFloat() const { return m_valType == eFloat; }
	bool UseInteger() const { return m_valType == eInteger || m_valType == eBool; }
	bool UseMagicItem() const { return m_valType == eMagicItem; }
	bool UseString() const { return m_valType == eString; }
	bool UseEffectSetting() const { return m_valType == eEffectSetting; }

	EValType UseValueType() {
		if (m_valType == eInteger && m_bForMod) {
			return eFloat;
		} else {
			return m_valType;
		}
	}

	float* FloatPtr() { return &m_floatVal; }
	UInt32* UInt32Ptr() { return &m_intVal; }
	MagicItem** MagicItemPtr() { return &m_magicItem; }
	char** StringPtr() { return (char**)&m_stringVal; }
	EffectSetting** EffectSettingPtr() { return &m_effectSetting; }

	float FloatVal() const { return m_floatVal; }
	UInt32 UInt32Val() const { return m_intVal; }
	bool BoolVal() const { return m_intVal != 0; }
	MagicItem* MagicItemVal() const { return m_magicItem; }
	const char* StringVal() const { return m_stringVal; }
	EffectSetting* EffectSettingVal() const { return m_effectSetting; }

	bool ForMod() const { return m_bForMod; }

private:
	char	m_stringVal[256];
	UInt32 m_whichVal;
	EValType m_valType;
	float	m_floatVal;
	UInt32	m_intVal;
	MagicItem* m_magicItem;
	EffectSetting* m_effectSetting;
	bool m_bForMod;
};

bool ChangeWeaponBaseValue(TESForm* form, ChangeValueState& state, double* result /*bool bMod, TESForm* form, UInt32 valueType, float value, MagicItem* magicItem, double* result*/)
{
	if (!form || !result) return false;
	*result = 0;
	TESObjectWEAP* weapon = (TESObjectWEAP*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0);
	TESAmmo* ammo = NULL;
	if (!weapon) {
		ammo = (TESAmmo*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESAmmo, 0);
		if (!ammo) {
			return false;
		}
	}

	bool bMod = state.ForMod();
	UInt32 intVal = 0;
	float floatVal = 0.0;
	bool bVal = false;
	MagicItem* magicItem = NULL;
	const char* string = NULL;

	switch(state.UseValueType()) {
		case ChangeValueState::eInteger:
			intVal = state.UInt32Val();
			break;
		case ChangeValueState::eFloat:
			floatVal = state.FloatVal();
			break;
		case ChangeValueState::eBool:
			bVal = state.BoolVal();
			break;
		case ChangeValueState::eMagicItem:
			magicItem = state.MagicItemVal();
			break;
		case ChangeValueState::eString:
			string = state.StringVal();
			break;
		default:
			return false;
	}

	switch (state.WhichValue()) {
		case kVal_Reach:
		{
			if (weapon) {
				weapon->reach = SafeChangeFloat(weapon->reach, floatVal, bMod, false);
				return true;
			}
			break;
		}

		case kVal_Speed:
		{
			float& speedToSet = (weapon) ? weapon->speed : ammo->speed;
			speedToSet = SafeChangeFloat(speedToSet, floatVal, bMod, false);
			return true;
		}

		case kVal_IgnoresResist:
		{
			UInt32 & ignoreResistToSet = (weapon) ? weapon->ignoreNormalWeaponResist : ammo->ignoreNormalWeaponResist;
			ignoreResistToSet = (bVal) ? 1 : 0;
			return true;
		}

		// changing the weapon type is not supported
		case kVal_WeaponType:
		{
			if (weapon) {
				weapon->type = intVal;
				return true;
			}
			break;
		}

		default:
			return false;
	}
	return false;
}

bool ChangeObjectBaseValue(TESForm* form, ChangeValueState& state, double* result)
{
	if (!form) return false;

	*result = 0;

	bool bMod = state.ForMod();
	UInt32 intVal = 0;
	float floatVal = 0.0;
	bool bVal = false;
	MagicItem* magicItem = NULL;
	const char* string = NULL;
	EffectSetting* effectSetting = NULL;

	switch(state.UseValueType()) {
		case ChangeValueState::eInteger:
			intVal = state.UInt32Val();
			break;
		case ChangeValueState::eFloat:
			floatVal = state.FloatVal();
			break;
		case ChangeValueState::eBool:
			bVal = state.BoolVal();
			break;
		case ChangeValueState::eMagicItem:
			magicItem = state.MagicItemVal();
			break;
		case ChangeValueState::eString:
			string = state.StringVal();
			break;
		case ChangeValueState::eEffectSetting:
			effectSetting = state.EffectSettingVal();
		default:
			return false;
	}

	UInt32 whichValue = state.WhichValue();

	switch(whichValue) {
		case kVal_Weight:
			{
				TESWeightForm* weightForm = (TESWeightForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESWeightForm, 0);
				if (weightForm) {
					weightForm->weight = SafeChangeFloat(weightForm->weight, floatVal, bMod, false);
					return true;
				}
				break;
			}
		case kVal_GoldValue:
			{
				switch (form->typeID)
				{
				case kFormType_Ingredient:
					{
						IngredientItem* ingredient = (IngredientItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_IngredientItem, 0);
						if (ingredient) {
							ingredient->value = (bMod) ? SafeModUInt32(ingredient->value, floatVal) : intVal;
							form->MarkAsModified(TESForm::kModified_GoldValue);
							return true;
						}
						break;
					}
				case kFormType_AlchemyItem:
					{
						AlchemyItem* alchItem = (AlchemyItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
						if (alchItem) {
							alchItem->SetGoldValue((bMod) ? SafeModUInt32(alchItem->GetGoldValue(), floatVal) : intVal);
							form->MarkAsModified(TESForm::kModified_GoldValue);
							return true;
						}
						break;
					}
				default:
					TESValueForm* valueForm = (TESValueForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESValueForm, 0);
					if (valueForm) {
						valueForm->value = (bMod) ? SafeModUInt32(valueForm->value, floatVal) : intVal;
						form->MarkAsModified(TESForm::kModified_GoldValue);
						return true;
					}
					break;
				}
			}
		case kVal_Health:
			{
				TESHealthForm* healthForm = (TESHealthForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESHealthForm, 0);
				if (healthForm) {
					healthForm->health = (bMod) ? SafeModUInt32(healthForm->health, floatVal) : intVal;
					return true;
				}
				break;
			}
		case kVal_Slot:
			{
				TESBipedModelForm* bipedForm = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
				if (bipedForm) {
					bipedForm->SetSlot(intVal);
				}
				break;
			}
		case kVal_Charge:
		{
			TESEnchantableForm* enchantForm = (TESEnchantableForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESEnchantableForm, 0);
			if (enchantForm) {
				enchantForm->enchantment = (bMod) ? SafeModUInt32(enchantForm->enchantment, floatVal) : intVal;
				return true;
			}
			break;
		}

		case kVal_QuestItem :
		{
			form->SetQuestItem(bVal);
			return true;
		}

		case kVal_Enchantment:
		{
			EnchantmentItem* enchantItem = NULL;
			if (magicItem) {
				enchantItem = (EnchantmentItem*)Oblivion_DynamicCast(magicItem, 0, RTTI_MagicItem, RTTI_EnchantmentItem, 0);
			}

			TESEnchantableForm* enchantForm = (TESEnchantableForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESEnchantableForm, 0);

			if (enchantForm) {
				EnchantmentItem* oldItem = enchantForm->enchantItem;

				if (state.ForMod()) {
					// the bForMod is a remove call
					enchantForm->enchantItem = 0;
					UInt32* refResult = (UInt32*)result;
				} else if (enchantItem && enchantItem->MatchesType(form)) {
					enchantForm->enchantItem = enchantItem;
				}
				if (oldItem) {
					UInt32* refResult = (UInt32*)result;
					*refResult = oldItem->refID;
				}
				return true;
			}
			break;
		}

		case kVal_Damage:
		{
			TESAttackDamageForm* damageForm = (TESAttackDamageForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESAttackDamageForm, 0);
			if (damageForm) {
				damageForm->damage = (bMod) ? SafeModUInt32(damageForm->damage, floatVal) : intVal;
				return true;
			}
			break;
		}

		case kVal_Reach:
		case kVal_Speed:
		case kVal_WeaponType:
		case kVal_IgnoresResist:
		{
			return ChangeWeaponBaseValue(form, state, result);
			break;
		}

		case kVal_ArmorRating:
		{
			TESObjectARMO* armor = (TESObjectARMO*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectARMO, 0);
			if (armor) {
				armor->armorRating = (bMod) ? SafeModUInt32(armor->armorRating, floatVal) : intVal;
				return true;
			}
			break;
		}

		case kVal_ArmorType:
		{
			TESObjectARMO* armor = (TESObjectARMO*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectARMO, 0);
			if (armor) {
				bool bSetHeavyArmor = (intVal != 0);
				armor->SetHeavyArmor(bSetHeavyArmor);
				return true;
			}
			break;
		}
		case kVal_Soul:
		{
			TESSoulGem* soulGem = (TESSoulGem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESSoulGem, 0);
			if (soulGem && intVal >= TESSoulGem::kSoul_None && intVal <= TESSoulGem::kSoul_Grand ) {
				soulGem->soul = intVal;
				return true;
			}
			break;
		}

		case kVal_SoulCapacity:
		{
			TESSoulGem* soulGem = (TESSoulGem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESSoulGem, 0);
			if (soulGem && intVal >= TESSoulGem::kSoul_None && intVal <= TESSoulGem::kSoul_Grand ) {
				soulGem->capacity = intVal;
				return true;
			}
			break;
		}
		case kVal_Food :
		{
			IngredientItem* ingredient = (IngredientItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_IngredientItem, 0);
			if (ingredient) {
				ingredient->SetIsFood(bVal);
				return true;
			} else {
				AlchemyItem* alchemyItem = (AlchemyItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
				if (alchemyItem) {
					alchemyItem->SetIsFood(bVal);
					return true;
				}
			}
			break;
		}

		case kVal_Playable:
		{
			TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
			if (biped) {
				biped->SetPlayable(bVal);
				return true;
			}
			break;
		}

		case kVal_Name:
		{
			TESFullName* name = (TESFullName*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFullName, 0);
			if (name) {
				name->name.Set(string);
				return true;
			}
			break;
		}

		case kVal_BookIsScroll:
		{
			TESObjectBOOK* book = (TESObjectBOOK*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectBOOK, 0);
			if (book) {
				book->SetIsScroll(bVal);
				return true;
			}
			break;
		}

		case kVal_BookCantBeTaken:
		{
			TESObjectBOOK* book = (TESObjectBOOK*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectBOOK, 0);
			if (book) {
				book->SetCantBeTaken(bVal);
				return true;
			}
			break;
		}

		case kVal_BookTeaches:
		{
			TESObjectBOOK* book = (TESObjectBOOK*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectBOOK, 0);
			if (book) {
				book->SetTeaches(intVal);
				return true;
			}
			break;
		}

		case kVal_Quality:
		{
			TESQualityForm* quality = (TESQualityForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESQualityForm, 0);
			if (quality) {
				quality->quality = SafeChangeFloat(quality->quality, floatVal, bMod, false);
				return true;
			}
			break;
		}

		case kVal_ApparatusType:
			{
				TESObjectAPPA* apparatus = (TESObjectAPPA*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectAPPA, 0);
				if (apparatus && intVal <= TESObjectAPPA::eApparatus_Retort) {
					apparatus->appaType = intVal;
					return true;
				}
				break;
			}
		case kVal_Poison:
		default:
			return false;
	}
	return false;
}

//class feSetObjectValue : public FoundEquipped
//{
//	UInt32 m_valueType;
//	float m_value;
//	bool m_bMod;
//public:
//	feSetObjectValue(UInt32 valueType, float value, bool bMod) : m_valueType(valueType), m_value(value), m_bMod(bMod) {}
//	~feSetObjectValue() {}
//	bool Found(ExtraContainerChanges::EntryData* entryData, double *result) {
//		bool bChanged = ChangeObjectBaseValue(m_bMod, bClonedOnlyF, entryData->type, m_valueType, m_value, NULL, result);
//		return true;
//	}
//
//};
//
//static bool Cmd_SetEquippedObjectValue_Execute(COMMAND_ARGS)
//{
//	*result = 0;
//
//	// easy out if we don't have an object
//	if(!thisObj) return true;
//
//	UInt32 valueType = 0;
//	float value = 0;
//	UInt32 slotIdx = 0;
//	if(!ExtractArgs(PASS_EXTRACT_ARGS, &valueType, &value, &slotIdx)) return true;
//
//	feSetObjectValue setObjectValue(valueType, value, false);
//	return FindEquipped(thisObj, slotIdx, &setObjectValue, result);
//}
//
//static bool Cmd_ModEquippedObjectValue_Execute(COMMAND_ARGS)
//{
//	*result = 0;
//
//	// easy out if we don't have an object
//	if(!thisObj) return true;
//
//	UInt32 valueType = 0;
//	float value = 0;
//	UInt32 slotIdx = 0;
//	if(!ExtractArgs(PASS_EXTRACT_ARGS, &valueType, &value, &slotIdx)) return true;
//
//	feSetObjectValue setObjectValue(valueType, value, true);
//	return FindEquipped(thisObj, slotIdx, &setObjectValue, result);
//}

static bool ChangeCurrentHealth(TESForm* baseForm, BaseExtraList* baseExtraList, ChangeValueState& state, double* result)
{
	*result = 0;
	if (!baseForm || !baseExtraList || !result) return false;
	UInt32 whichVal = state.WhichValue();
	if (!(whichVal == kVal_Health || whichVal == kVal_CurHealth)) return false;

	TESHealthForm* healthForm = (TESHealthForm*)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESHealthForm, 0);
	if (healthForm) {
		// see if there is a current override
		BSExtraData* extraData = baseExtraList->GetByType(kExtraData_Health);
		ExtraHealth* xHealth = (ExtraHealth*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraHealth, 0);

		float baseHealth =  healthForm->health;
		float nuHealthVal = 0;
		if (state.ForMod()) {
			float modBaseHealth = (xHealth) ? xHealth->health : baseHealth;
			nuHealthVal = SafeChangeFloat(modBaseHealth, state.FloatVal(), true, false);
		} else if (whichVal == kVal_CurHealth) {
			nuHealthVal = state.FloatVal();
		} else {
			nuHealthVal = state.UInt32Val();
		}

		if (nuHealthVal == baseHealth) {
			// putting back to base health
			if (xHealth) {
				// we need to remove the health
				if (baseExtraList->Remove(xHealth)) {
					FormHeap_Free(xHealth);
					xHealth = NULL;
				}
			}
		} else if (xHealth) {
			// simply change the value
			xHealth->health = nuHealthVal;
		} else {
			xHealth = ExtraHealth::Create();
			xHealth->health = (float)nuHealthVal;
			if (!baseExtraList->Add(xHealth)) {
				FormHeap_Free(xHealth);
			}
		}
	}
	return true;
}

static bool ChangeCurrentCharge(TESForm* baseForm, BaseExtraList* baseExtraList, ChangeValueState& state, double* result)
{
	*result = 0;
	if (!baseForm || !baseExtraList || !result) return false;
	if (state.WhichValue() != kVal_Charge) return false;

	TESEnchantableForm* enchantable = (TESEnchantableForm*)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESEnchantableForm, 0);
	if (enchantable) {
		// see if there is a current override
		BSExtraData* extraData = baseExtraList->GetByType(kExtraData_Charge);
		ExtraCharge* xCharge = (ExtraCharge*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraCharge, 0);

		float baseCharge = enchantable->enchantment;
		float nuChargeVal = 0;
		if (state.ForMod()) {
			float modBaseCharge = (xCharge) ? xCharge->charge : baseCharge;
			nuChargeVal = SafeChangeFloat(modBaseCharge, state.FloatVal(), true, false);
		} else {
			nuChargeVal = (float)state.UInt32Val();
		}

		if (nuChargeVal == baseCharge) {
			// fully charged
			if (xCharge) {
				// we need to remove the extra charge
				if (baseExtraList->Remove(xCharge)) {
					FormHeap_Free(xCharge);
					xCharge = NULL;
				}
			}
		} else if (xCharge) {
			// simply change the value
			xCharge->charge = nuChargeVal;
		} else {
			xCharge = ExtraCharge::Create();
			xCharge->charge = nuChargeVal;
			if (!baseExtraList->Add(xCharge)) {
				FormHeap_Free(xCharge);
			}
		}
	}
	return true;
}

static bool ChangeCurrentPoison(TESForm* baseForm, BaseExtraList* baseExtraList, ChangeValueState& state, double* result)
{
	*result = 0;
	if (!baseForm || !baseExtraList || !result) return false;
	if (state.WhichValue() != kVal_WeaponPoison) return false;

	TESObjectWEAP* weapon = (TESObjectWEAP*)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0);
	if (weapon) {
		// see if there is a current override
		BSExtraData* extraData = baseExtraList->GetByType(kExtraData_Poison);
		ExtraPoison* xPoison = (ExtraPoison*)Oblivion_DynamicCast(extraData, 0, RTTI_BSExtraData, RTTI_ExtraPoison, 0);
		AlchemyItem* oldPoison = (xPoison) ? xPoison->poison : NULL;
		AlchemyItem* nuPoison = (AlchemyItem*)Oblivion_DynamicCast(state.MagicItemVal(), 0, RTTI_MagicItem, RTTI_AlchemyItem, 0);

		if (state.ForMod()) {
			// we need to remove the poison
			if (xPoison) {
				baseExtraList->Remove(xPoison);
				FormHeap_Free(xPoison);
				xPoison = NULL;
			}
		} else if (nuPoison) {
			if (xPoison) {
				xPoison->poison = nuPoison;
			} else {
				xPoison = ExtraPoison::Create();
				xPoison->poison = nuPoison;
				if (!baseExtraList->Add(xPoison)) {
					FormHeap_Free(xPoison);
					xPoison = NULL;
				}
			}
		}
		if (oldPoison) {
			UInt32* refResult = (UInt32*)result;
			*refResult = oldPoison->refID;
		}
	}
	return true;
}

static bool Cmd_SetCurrentHealth_Execute(COMMAND_ARGS)
{
	*result = 0;

	// easy out if we don't have an object
	if(!thisObj) return true;

	float value = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &value)) return true;

	ChangeValueState state(kVal_CurHealth, bModF);
	*state.FloatPtr() = value;
	return ChangeCurrentHealth(thisObj->baseForm, &thisObj->baseExtraList, state, result);
}

static bool Cmd_SetCurrentSoulLevel_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (!thisObj) return true;
	TESSoulGem* soulGem = (TESSoulGem*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESSoulGem, 0);
	// this needs to be a soul gem and to be a soul gem type without a soul normally in it
	if (!soulGem || soulGem->soul != 0) return true;

	UInt32 soulLevel = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &soulLevel)) return true;
	if (soulLevel > TESSoulGem::kSoul_Grand) return true;
	if (soulLevel > soulGem->capacity) soulLevel = soulGem->capacity;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Soul);
	ExtraSoul* xSoul = (ExtraSoul*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraSoul, 0);

	if (xSoul) {
		xSoul->soul = soulLevel; // see if this works with soulLevel = 0.
	} else if (soulLevel != 0) {
		// need to add a new ExtraSoul
		xSoul = ExtraSoul::Create();
		xSoul->soul = soulLevel;
		thisObj->baseExtraList.Add(xSoul);
	}
	return true;
}

static bool ChangeObjectValue_Execute(COMMAND_ARGS, ChangeValueState& state)
{
	*result = 0;
	TESForm* form = NULL;

	switch(state.UseValueType()) {
		case ChangeValueState::eFloat:
			ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, state.FloatPtr(), &form);
			break;
		case ChangeValueState::eInteger:
		case ChangeValueState::eBool:
			ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, state.UInt32Ptr(), &form);
			break;

		case ChangeValueState::eMagicItem:
			ExtractArgs(PASS_EXTRACT_ARGS, state.MagicItemPtr(), &form);
			break;
		case ChangeValueState::eString:
			ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, state.StringPtr(), &form);
			break;
		default:
			return true;
	}

	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	bool bChanged = ChangeObjectBaseValue(form, state, result);
	return true;
}

class feChangeCurrentValue : public FoundEquipped
{
	ChangeValueState& m_cvs;
public:
	feChangeCurrentValue(ChangeValueState& cvs) : m_cvs(cvs) {}
	bool Found(ExtraContainerChanges::EntryData* entryData, double *result, ExtraDataList* extendData) {
		switch(m_cvs.WhichValue()) {
			case kVal_Health:
			case kVal_CurHealth:
				return ChangeCurrentHealth(entryData->type, extendData, m_cvs, result);
			case kVal_Charge:
				return ChangeCurrentCharge(entryData->type, extendData, m_cvs, result);
			case kVal_WeaponPoison:
				return ChangeCurrentPoison(entryData->type, extendData, m_cvs, result);
			default:
				return false;
		}
	}
};

static bool ChangeEquippedCurrentValue_Execute(COMMAND_ARGS, ChangeValueState& state)
{
	*result = 0;

	UInt32 slotIdx;
	bool bArgsExtracted = false;

	switch(state.UseValueType()) {
		case ChangeValueState::eFloat:
			bArgsExtracted = ExtractArgs(PASS_EXTRACT_ARGS, state.FloatPtr(), &slotIdx);
			break;
		case ChangeValueState::eInteger:
		case ChangeValueState::eBool:
			bArgsExtracted = ExtractArgs(PASS_EXTRACT_ARGS, state.UInt32Ptr(), &slotIdx);
			break;

		case ChangeValueState::eMagicItem:
			bArgsExtracted = ExtractArgs(PASS_EXTRACT_ARGS, state.MagicItemPtr(), &slotIdx);
			break;
		case ChangeValueState::eString:
			bArgsExtracted = ExtractArgs(PASS_EXTRACT_ARGS, state.StringPtr(), &slotIdx);
			break;
		default:
			return true;
	}

	if (!thisObj || !bArgsExtracted) return true;

	feChangeCurrentValue changeCurrentVal(state);
	bool bFound = FindEquipped(thisObj, slotIdx, &changeCurrentVal, result);
	return true;
}

// Weight
static bool Cmd_GetWeight_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Weight);
}

static bool Cmd_SetWeight_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Weight, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModWeight_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Weight, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// Gold Value
static bool Cmd_GetGoldValue_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_GoldValue);
}

static bool Cmd_SetGoldValue_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_GoldValue, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModGoldValue_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_GoldValue, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// Health
static bool Cmd_GetObjectHealth_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Health);
}

static bool Cmd_SetObjectHealth_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Health, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModObjectHealth_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Health, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// Equipment Slot
static bool Cmd_GetEquipmentSlot_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Slot);
}

static bool Cmd_SetEquipmentSlot_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Slot, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// EnchantmentCharge
static bool Cmd_GetObjectCharge_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Charge);
}

static bool Cmd_SetObjectCharge_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Charge, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModObjectCharge_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Charge, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// QuestItem
static bool Cmd_IsQuestItem_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_QuestItem);
}

static bool Cmd_SetQuestItem_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_QuestItem, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// Enchantment
static bool Cmd_GetEnchantment_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Enchantment);
}

static bool Cmd_RemoveEnchantment_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	ExtractArgs(PASS_EXTRACT_ARGS, &form);
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	ChangeValueState state(kVal_Enchantment, bModT);
	bool bChanged = ChangeObjectBaseValue(form, state, result);
	return true;
}

static bool Cmd_SetEnchantment_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Enchantment, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// AttackDamage
static bool Cmd_GetAttackDamage_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Damage);
}

static bool Cmd_SetAttackDamage_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Damage, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModAttackDamage_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Damage, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// WeaponReach
static bool Cmd_GetWeaponReach_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Reach);
}

static bool Cmd_SetWeaponReach_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Reach, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModWeaponReach_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Reach, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// WeaponSpeed
static bool Cmd_GetWeaponSpeed_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Speed);
}

static bool Cmd_SetWeaponSpeed_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Speed, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModWeaponSpeed_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Speed, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// WeaponType
static bool Cmd_GetWeaponType_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_WeaponType);
}

static bool Cmd_SetWeaponType_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_WeaponType, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// IgnoresResistance
static bool Cmd_GetIgnoresResistance_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_IgnoresResist);
}

static bool Cmd_SetIgnoresResistance_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_IgnoresResist, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// ArmorRating
static bool Cmd_GetArmorAR_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_ArmorRating);
}

static bool Cmd_SetArmorAR_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_ArmorRating, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModArmorAR_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_ArmorRating, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// ArmorType
static bool Cmd_GetArmorType_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_ArmorType);
}

static bool Cmd_SetArmorType_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_ArmorType, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// SoulLevel
static bool Cmd_GetSoulLevel_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Soul);
}

static bool Cmd_SetSoulLevel_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Soul, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// SoulGemCapacity
static bool Cmd_GetSoulGemCapacity_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_SoulCapacity);
}

static bool Cmd_SetSoulGemCapacity_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_SoulCapacity, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// IsFood
static bool Cmd_IsFood_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Food);
}

// SetIsFood
static bool Cmd_SetIsFood_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Food, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// IsPosion
static bool Cmd_IsPoison_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Poison);
}

// SetIsFood
static bool Cmd_SetIsPosion_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Poison, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// IsPlayable
static bool Cmd_IsPlayable_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Playable);
}

static bool Cmd_IsPlayable2_Execute(COMMAND_ARGS)
{
	//version which only returns false if "unplayable" flag is present and set
	*result = 1;
	TESForm* form = NULL;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		form = form->TryGetREFRParent();
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;
	}
	if (form)
	{
		TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
		if (biped)
			*result = biped->IsPlayable() ? 1 : 0;
	}
	return true;
}

// SetIsFood
static bool Cmd_SetIsPlayable_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Playable, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// SetName
static bool Cmd_SetName_Execute(COMMAND_ARGS)
{
	if (!result) return true;

	TESForm* form = NULL;
	char	string[256];

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &string, &form);
	if (!form)
	{
		if (!thisObj)
			return true;

		// don't use baseform, it may be a mapmarker. GetFullName() will use base if appropriate
		form = thisObj;

		// if we are changing the name of a mapmarker and the mapmenu is open, update map menu
		if (thisObj->baseForm && thisObj->baseForm->refID == kFormID_MapMarker)
		{
			MapMenu* mapMenu = (MapMenu*)GetMenuByType(kMenuType_Map);
			if (mapMenu)
				mapMenu->UpdateMarkerName(thisObj, string);
		}
	}

	TESFullName* name = form->GetFullName();
	if (name) {
		name->name.Set(string);
		return true;
	}
	return true;
}

static bool Cmd_CompareName_Execute(COMMAND_ARGS)
{
	if (!result) return true;

	TESForm* form = NULL;
	char	textArg[256];

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &textArg, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	TESFullName* name = (TESFullName*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFullName, 0);
	if (name) {
		bool bFound = name->name.Includes(textArg);
		*result = (bFound) ? 1 : 0;
	}
	return true;
}

static bool Cmd_CopyName_Execute(COMMAND_ARGS)
{
	if (!result) return true;

	TESForm* srcForm = NULL;
	TESForm* targetForm = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &srcForm, &targetForm);
	srcForm = srcForm->TryGetREFRParent();
	targetForm = targetForm->TryGetREFRParent();

	if (!srcForm) return true;
	if (!targetForm) {
		if (!thisObj) return true;
		targetForm = thisObj->baseForm;
	}

	TESFullName* targetName = (TESFullName*)Oblivion_DynamicCast(targetForm, 0, RTTI_TESForm, RTTI_TESFullName, 0);

	if (!targetName) return true;

	targetName->name.Set(GetFullName(srcForm));
	return true;
}

static bool Cmd_ModName_Execute(COMMAND_ARGS)
{
	if (!result) return true;

	TESForm* form = NULL;
	char	string[256];

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &string, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	TESFullName* name = (TESFullName*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFullName, 0);
	if (name) {
		// we expect textArg to be in the following format: "toReplace|replaceWith"
		std::string strTextArg(string);
		// look and see if the input has the pipe character
		std::string::size_type pipePos = strTextArg.find(GetSeparatorChar(scriptObj));
		if (pipePos != std::string::npos) {
			// we found the pipe
			// now look for the replacement string
			std::string toReplace(strTextArg.substr(0, pipePos));
			name->name.Replace(toReplace.c_str(), &strTextArg[pipePos+1]);
		}
		return true;
	}
	return true;
}

static bool Cmd_AppendToName_Execute(COMMAND_ARGS)
{
	if (!result) return true;

	TESForm* form = NULL;
	char	string[256];

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &string, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	TESFullName* name = (TESFullName*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFullName, 0);
	if (name) {
		name->name.Append(string);
	}
	return true;
}

static BSStringT* PathStringFromForm(TESForm* form, UInt32 whichValue, EMode mode)
{
	switch(whichValue) {
		case kVal_Model:
		{
			TESModel* model = (TESModel*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESModel, 0);
			if (model && (form->SupportsSimpleModel() || mode == kCompare || mode == kGet)) {
				return &model->nifPath;
			}
			break;
		}
		case kVal_Icon:
		{
			TESIcon* icon = (TESIcon*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESIcon, 0);
			if (icon) {
				return &icon->ddsPath;
			}
			break;
		}

		case kVal_BipedMale:
		case kVal_BipedFemale:
		{
			TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
			if (biped) {
				TESModel& model = biped->bipedModel[whichValue - kVal_BipedMale];
				return &model.nifPath;
			}
			break;
		}

		case kVal_GroundMale:
		case kVal_GroundFemale:
		{
			TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
			if (biped) {
				TESModel& model = biped->groundModel[whichValue - kVal_GroundMale];
				return &(model.nifPath);
			}
			break;
		}

		case kVal_IconMale:
		case kVal_IconFemale:
		{
			TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
			if (biped) {
				TESIcon& icon = biped->icon[whichValue - kVal_IconMale];
				return &icon.ddsPath;
			}
			break;
		}
	}
	return NULL;
}

static bool PathFunc_Execute(COMMAND_ARGS, UInt32 whichValue, EMode mode)
{
	*result = 0;

	TESForm* targetForm = NULL;
	TESForm* srcForm = NULL;
	char textArg[256] = { 0 };

	bool bExtracted = false;
	if (mode == kCopy) {
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &srcForm, &targetForm);
		if (!srcForm) return true;
		srcForm = srcForm->TryGetREFRParent();
	} else if (mode == kGet) {
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &targetForm);
	}
	else {
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &textArg, &targetForm);
		if (textArg[0] == '\0') return true;
	}

	targetForm = targetForm->TryGetREFRParent();
	if (!targetForm) {
		if (!thisObj) return true;
		targetForm = thisObj->baseForm;
	}

	BSStringT* theString = PathStringFromForm(targetForm, whichValue, mode);
	if (theString != NULL) {
		switch(mode) {
			case kSet:
				{
					theString->Set(textArg);
					break;
				}
			case kMod:
				{
					// we expect textArg to be in the following format: "toReplace|replaceWith"
					std::string strTextArg(textArg);
					// look and see if the input has the pipe character
					std::string::size_type pipePos = strTextArg.find(GetSeparatorChar(scriptObj));
					if (pipePos != std::string::npos) {
						// we found the pipe
						// now look for the replacement string
						std::string toReplace(strTextArg.substr(0, pipePos));
						theString->Replace(toReplace.c_str(), &strTextArg[pipePos+1]);
					}
					break;
				}

			case kCompare:
				{
					bool bFound = theString->Includes(textArg);
					*result = bFound ? 1 : 0;
					break;
				}
			case kCopy:
				{
					BSStringT* srcString = PathStringFromForm(srcForm, whichValue, mode);
					if (srcString) {
						theString->Set(srcString->m_data);
					}
					break;
				}

			case kGet:
				{
					std::string sFilePath;
					if (whichValue == kVal_Icon)
						sFilePath = std::string("data\\textures\\menus\\icons\\") + std::string(theString->m_data);
					else
						sFilePath = std::string("data\\meshes\\") + std::string(theString->m_data);

					*result = (*g_FileFinder)->FindFile(sFilePath.c_str(), 0, 0, -1) ? 1 : 0;

					break;
				}
			default:
				break;
		}
	}
	return true;
}

// SetModelPath
static bool Cmd_SetModelPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Model, kSet);
}

// ModModelPath
static bool Cmd_ModModelPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Model, kMod);
}

// CompareModelPath
static bool Cmd_CompareModelPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Model, kCompare);
}

// CopyModelPath
static bool Cmd_CopyModelPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Model, kCopy);
}

// SetIconPath
static bool Cmd_SetIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Icon, kSet);
}

// ModIconPath
static bool Cmd_ModIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Icon, kMod);
}

// CompareIconPath
static bool Cmd_CompareIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Icon, kCompare);
}

// CopyIconPath
static bool Cmd_CopyIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Icon, kCopy);
}

// SetMaleBipedPath
static bool Cmd_SetMaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedMale, kSet);
}

// ModMaleBipedPath
static bool Cmd_ModMaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedMale, kMod);
}

// CompareMaleBipedPath
static bool Cmd_CompareMaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedMale, kCompare);
}

// CopyMaleBipedPath
static bool Cmd_CopyMaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedMale, kCopy);
}

// SetFemaleBipedPath
static bool Cmd_SetFemaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedFemale, kSet);
}

// ModFemaleBipedPath
static bool Cmd_ModFemaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedFemale, kMod);
}

// CompareFemaleBipedPath
static bool Cmd_CompareFemaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedFemale, kCompare);
}

// CopyFemaleBipedPath
static bool Cmd_CopyFemaleBipedPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_BipedFemale, kCopy);
}

// SetMaleGroundPath
static bool Cmd_SetMaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundMale, kSet);
}

// ModMaleGroundPath
static bool Cmd_ModMaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundMale, kMod);
}

// CompareMaleGroundPath
static bool Cmd_CompareMaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundMale, kCompare);
}

// CopyMaleGroundPath
static bool Cmd_CopyMaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundMale, kCopy);
}

// SetFemaleGroundPath
static bool Cmd_SetFemaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundFemale, kSet);
}

// ModFemaleGroundPath
static bool Cmd_ModFemaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundFemale, kMod);
}

// CompareFemaleGroundPath
static bool Cmd_CompareFemaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundFemale, kCompare);
}

// CopyFemaleGroundPath
static bool Cmd_CopyFemaleGroundPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_GroundFemale, kCopy);
}

// SetMaleIconPath
static bool Cmd_SetMaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconMale, kSet);
}

// ModMaleIconPath
static bool Cmd_ModMaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconMale, kMod);
}

// CompareMaleIconPath
static bool Cmd_CompareMaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconMale, kCompare);
}

// CopyMaleIconPath
static bool Cmd_CopyMaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconMale, kCopy);
}

// SetFemaleIconPath
static bool Cmd_SetFemaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconFemale, kSet);
}

// ModFemaleIconPath
static bool Cmd_ModFemaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconFemale, kMod);
}

// CompareFemaleIconPath
static bool Cmd_CompareFemaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconFemale, kCompare);
}

// CopyFemaleIconPath
static bool Cmd_CopyFemaleIconPath_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_IconFemale, kCopy);
}

// Equipped Current Health
static bool Cmd_GetEquippedCurrentHealth_Execute(COMMAND_ARGS)
{
	return GetEquippedCurrentValue_Execute(PASS_COMMAND_ARGS, kVal_Health);
}

static bool Cmd_SetEquippedCurrentHealth_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Health, bModF);
	return ChangeEquippedCurrentValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModEquippedCurrentHealth_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Health, bModT);
	return ChangeEquippedCurrentValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_GetEquippedCurrentCharge_Execute(COMMAND_ARGS)
{
	return GetEquippedCurrentValue_Execute(PASS_COMMAND_ARGS, kVal_Charge);
}

static bool Cmd_SetEquippedCurrentCharge_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Charge, bModF);
	return ChangeEquippedCurrentValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModEquippedCurrentCharge_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Charge, bModT);
	return ChangeEquippedCurrentValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_GetEquippedWeaponPoison_Execute(COMMAND_ARGS)
{
	feGetCurrentValue getCurrentValue(kVal_WeaponPoison);
	bool bFound = FindEquipped(thisObj, kSlot_Weapon, &getCurrentValue, result);
	return true;
}

static bool Cmd_SetEquippedWeaponPoison_Execute(COMMAND_ARGS)
{
	if (!thisObj) return true;

	ChangeValueState state(kVal_WeaponPoison, bModF);
	bool bArgsExtracted = ExtractArgs(PASS_EXTRACT_ARGS, state.MagicItemPtr());
	if (!bArgsExtracted) return true;

	feChangeCurrentValue changeCurrentVal(state);
	bool bFound = FindEquipped(thisObj, kSlot_Weapon, &changeCurrentVal, result);
	return true;
}

static bool Cmd_RemoveEquippedWeaponPoison_Execute(COMMAND_ARGS)
{
	if (!thisObj) return true;

	ChangeValueState state(kVal_WeaponPoison, bModT);
	feChangeCurrentValue changeCurrentVal(state);
	bool bFound = FindEquipped(thisObj, kSlot_Weapon, &changeCurrentVal, result);
	return true;
}

static bool Cmd_GetBookCantBeTaken_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_BookCantBeTaken);
}

static bool Cmd_SetBookCantBeTaken_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_BookCantBeTaken, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_GetBookIsScroll_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_BookIsScroll);
}

static bool Cmd_SetBookIsScroll_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_BookIsScroll, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_GetBookSkillTaught_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_BookTeaches);
}

static bool Cmd_SetBookSkillTaught_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_BookTeaches, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

//static bool Cmd_SetBookSkillTaughtC_Execute(COMMAND_ARGS)
//{
//	ChangeValueState state(kVal_BookTeaches, bModF);
//	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
//}

static bool Cmd_GetApparatusType_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_ApparatusType);
}

static bool Cmd_SetApparatusType_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_ApparatusType, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_GetQuality_Execute(COMMAND_ARGS)
{
	return GetObjectValue(PASS_COMMAND_ARGS, kVal_Quality);
}

static bool Cmd_SetQuality_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Quality, bModF);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

static bool Cmd_ModQuality_Execute(COMMAND_ARGS)
{
	ChangeValueState state(kVal_Quality, bModT);
	return ChangeObjectValue_Execute(PASS_COMMAND_ARGS, state);
}

// Is XXX functions
static bool IsType_Execute(COMMAND_ARGS, UInt8 typeID)
{
	*result = 0;
	TESForm* form = 0;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	if (form->typeID == typeID) {
		*result = 1;
	}
	return true;
}

static bool Cmd_IsWeapon_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Weapon);
}

static bool Cmd_IsAmmo_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Ammo);
}

static bool Cmd_IsAmmo_Eval(COMMAND_ARGS_EVAL)
{
	Console_Print("IsAmmo args: %08x, %08x, %08x", thisObj, arg1, arg2);
	*result = 1;
	return true;
}

static bool Cmd_IsArmor_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Armor);
}

static bool Cmd_IsClothing_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Clothing);
}

static bool Cmd_IsBook_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Book);
}

static bool Cmd_IsIngredient_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Ingredient);
}

static bool Cmd_IsContainer_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Container);
}

static bool Cmd_IsKey_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Key);
}

static bool Cmd_IsAlchemyItem_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_AlchemyItem);
}

static bool Cmd_IsApparatus_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Apparatus);
}

static bool Cmd_IsSoulGem_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_SoulGem);
}

static bool Cmd_IsSigilStone_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_SigilStone);
}

static bool Cmd_IsDoor_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Door);
}

static bool Cmd_IsActivator_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Activator);
}

static bool Cmd_IsLight_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Light);
}

static bool Cmd_IsMiscItem_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Misc);
}

static bool Cmd_IsFurniture_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Furniture);
}

static bool Cmd_IsFlora_Execute(COMMAND_ARGS)
{
	return IsType_Execute(PASS_COMMAND_ARGS, kFormType_Flora);
}

static bool Cmd_IsClonedForm_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	*result = IsClonedForm(form->refID) ? 1 : 0;
	return true;
}

static bool Cmd_CloneForm_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* form = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	TESForm* clonedForm = CloneForm(form);
	if (clonedForm) {
		*refResult = clonedForm->refID;
	}

	return true;
}

static bool Cmd_CompareNames_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	TESForm* base = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &base);
	form = form->TryGetREFRParent();
	base = base->TryGetREFRParent();

	if (!form)
		return true;
	if (!base)
	{
		if (!thisObj)
			return true;
		base = thisObj->baseForm;
	}

	TESFullName* first = (TESFullName*)Oblivion_DynamicCast(base, 0, RTTI_TESForm, RTTI_TESFullName, 0);
	TESFullName* second = (TESFullName*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFullName, 0);
	if (first && second)
		*result = first->name.Compare(second->name);

	return true;
}

static bool Cmd_GetContainerRespawns_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectCONT* container = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &container);
	if (!container) {
		if (!thisObj) return true;
		container = (TESObjectCONT*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESObjectCONT, 0);
		if (!container) return true;
	}

	*result = container->IsRespawning() ? 1 : 0;
	return true;
}

static bool Cmd_SetContainerRespawns_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 respawns = 0;
	TESObjectCONT* container = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &respawns, &container);
	if (!container) {
		if (!thisObj) return true;
		container = (TESObjectCONT*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESObjectCONT, 0);
		if (!container) return true;
	}
	container->SetRespawning(respawns != 0);
	return true;
}

static bool Cmd_IsLightCarriable_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = 0;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		form = form->TryGetREFRParent();
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;
	}
	if (form)
	{
		TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
		if (light)
			*result = light->IsCarriable() ? 1 : 0;
	}

	return true;
}

static bool Cmd_GetLightRadius_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = 0;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		form = form->TryGetREFRParent();
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;
	}
	if (form)
	{
		TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
		if (light)
			*result = light->GetRadius();
	}

	return true;
}

static bool Cmd_SetLightRadius_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 newRadius = 0;
	*result = 0;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &newRadius, &form))
	{
		form = form->TryGetREFRParent();
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;
	}
	if (form)
	{
		TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
		if (light)
		{
			light->SetRadius(newRadius);
			*result = 1;
		}
	}

	return true;
}

static bool Cmd_HasName_Execute(COMMAND_ARGS)
{
	TESForm* form = 0;
	*result = 0;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	form = form->TryGetREFRParent();
	if (!form)
		form = thisObj;

	if (form) {
		TESFullName* fullname = form->GetFullName();
		*result = (fullname && fullname->name.m_data && fullname->name.m_dataLen) ? 1 : 0;
	}

	return true;
}

static bool Cmd_AddItemNS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_AddItem_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);
	return true;
}

static bool Cmd_RemoveItemNS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_RemoveItem_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);
	return true;
}

static bool Cmd_EquipItemNS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_EquipItem_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);
	return true;
}

static bool Cmd_UnequipItemNS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_UnequipItem_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);
	return true;
}

class EffectItemValueCounter
{
public:
	UInt32 m_value;
	enum {
		kEffectCode_WAWA = 0x41574157,
		kEffectCode_NEYE = 0x4559454E,
		kEffectCode_WABR = 0x52424157,
	};

	EffectItemValueCounter() : m_value(0)
		{ }

	bool Accept(EffectItem* eff)
	{
		switch (eff->effectCode)
		{
		case kEffectCode_WAWA:
		case kEffectCode_WABR:
		case kEffectCode_NEYE:
			m_value += eff->setting->barterFactor * 5;
			break;
		default:
			m_value += eff->setting->barterFactor * eff->magnitude;
		}
		return true;
	}
};

static bool Cmd_GetFullGoldValue_Execute(COMMAND_ARGS)
//includes enchantment value in calculation
{
	TESForm* form = NULL;
	*result = 0;
	UInt32 baseVal = 0;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	form = form->TryGetREFRParent();
	if (!form)
		if (thisObj)
			form = thisObj->baseForm;

	if (!form)
		return true;

	switch (form->typeID)
	{
	case kFormType_Ingredient:
	{
		IngredientItem* ingredient = (IngredientItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_IngredientItem, 0);
		if (ingredient)
			baseVal = ingredient->value;
		break;
	}
	case kFormType_AlchemyItem:
	{
		AlchemyItem* alchItem = (AlchemyItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_AlchemyItem, 0);
		if (alchItem)
			baseVal = alchItem->GetGoldValue();
		break;
	}
	default:
		TESValueForm* valueForm = (TESValueForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESValueForm, 0);
		if (valueForm)
			baseVal = valueForm->value;
	}

	*result = baseVal;
	EnchantmentItem* enchItem = NULL;
	TESEnchantableForm* enchantForm = (TESEnchantableForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESEnchantableForm, 0);
	if (enchantForm)
		enchItem = enchantForm->enchantItem;
	if (!enchItem)		//not enchanted, so we're done
		return true;

	UInt32 enchCost = 0;
	if (!enchItem->IsAutoCalc()) {
		enchCost = enchItem->cost;
	} else {
		enchCost = enchItem->magicItem.list.GetMagickaCost();
	}

	switch (form->typeID)
	{
	case kFormType_Weapon:
	case kFormType_Ammo:
		enchCost = 0.4 * (enchCost + enchantForm->enchantment);
		break;
	case kFormType_Book:
		enchCost /= 2;
		break;
	case kFormType_Clothing:
	case kFormType_Armor:
		{
			MagicItem* magicItem = (MagicItem*)Oblivion_DynamicCast(enchItem, 0, RTTI_TESForm, RTTI_MagicItem, 0);
			if (!magicItem)		//not enchanted (how'd we get here then?)
				return true;
			EffectItemValueCounter vCounter;
			EffectItemVisitor(&(magicItem->list.effectList)).Visit(vCounter);
			enchCost = vCounter.m_value;
		}
		break;
	default:		//what?
		break;
	}

	*result = baseVal + enchCost;
	return true;
}

static bool Cmd_GetHotKeyItem_Execute(COMMAND_ARGS)
{
	UInt32 whichKey = 0;
	UInt32 * refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &whichKey))
		return true;

	if (whichKey && whichKey <= 8)
	{
		NiTPointerList <TESForm> * quickKey = &g_quickKeyList[--whichKey];
		if (quickKey->start && quickKey->start->data)
		{
			*refResult = quickKey->start->data->refID;
			if (IsConsoleMode())
				Console_Print("GetHotKeyItem %d >> %s (%08x)", ++whichKey, GetFullName(quickKey->start->data), *refResult);
		}
	}

	return true;
}

class ExtraContainerChangesEntryFinder {
public:
	TESForm* m_whichForm;

	ExtraContainerChangesEntryFinder ( TESForm* whichForm ) : m_whichForm(whichForm)
		{ }

	bool Accept(ExtraContainerChanges::EntryData* entryData)
	{
		return entryData->type == m_whichForm;
	}
};


static void _ClearHotKey ( UInt32 whichKey ) {
	DEBUG_PRINT("%u", whichKey);
	if (whichKey > 7)
		return;

	//remove ExtraQuickKey from container changes
	ExtraContainerChanges* xChanges = static_cast <ExtraContainerChanges*>((*g_thePlayer)->baseExtraList.GetByType(kExtraData_ContainerChanges));
	DEBUG_PRINT("Mortacci");
	if (xChanges && xChanges->data && xChanges->data->objList) {
		DEBUG_PRINT("Mortacci 2");
		for (tList<ExtraContainerChanges::EntryData>::Iterator xData = xChanges->data->objList->Begin(); !xData.End(); ++xData) {
			if (*xData && xData->extendData &&  !xData->extendData->IsEmpty()) {
				tList<ExtraDataList>::_Node* rem = xData->extendData->Head();
				if (!rem || !rem->item) {
					_MESSAGE("[WARNING]  Element  is invalid");
					continue;
				}
				ExtraQuickKey* qKey = (ExtraQuickKey*)rem->item->GetByType(kExtraData_QuickKey);//TODO this check only the first extradatalist, but more may be present in the EntryData 
				if (qKey && qKey->keyID == whichKey || whichKey == UInt32(-1)) {
					rem->item->Remove(qKey);  //merge the Ge
					FormHeap_Free(qKey);
				}

			}
		}
	}
	//clear quickkey
	DEBUG_PRINT("Mortacci 3");
	NiTPointerList <TESForm> * quickKey = &g_quickKeyList[whichKey];
	if (quickKey->start && quickKey->start->data) {
		if (xChanges && xChanges->data && xChanges->data->objList) {
			DEBUG_PRINT("Mortacci 4");
			for (tList<ExtraContainerChanges::EntryData>::Iterator xData = xChanges->data->objList->Begin(); !xData.End(); ++xData) {
				if (*xData && xData->type == quickKey->start->data && xData->extendData && !xData->extendData->IsEmpty()) {
					tList<ExtraDataList>::_Node* rem = xData->extendData->Head();
					if (!rem || !rem->item) {
						_MESSAGE("[WARNING]  Element  is invalid");
						continue;
					}
					ExtraQuickKey* qKey = (ExtraQuickKey*)rem->item->GetByType(kExtraData_QuickKey);//TODO this check only the first extradatalist, but more may be present in the EntryData 
					if (qKey && qKey->keyID > 7) {
						rem->item->Remove(qKey);  //merge the Ge
						FormHeap_Free(qKey);
					}

				}
			}
		}
		DEBUG_PRINT("Mortacci 5");

		quickKey->FreeNode(quickKey->start);
		quickKey->numItems = 0;
		quickKey->start = NULL;
		quickKey->end = NULL;
	}
}
static bool Cmd_ClearHotKey_Execute(COMMAND_ARGS)
{
	UInt32 whichKey = 0;
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &whichKey))
		return true;

	whichKey -= 1;
	_ClearHotKey ( whichKey );
	return true;
}

class InvDumper
{
public:
	bool Accept(ExtraContainerChanges::EntryData* entryData)
	{
		DEBUG_PRINT("%08x -> %s", entryData->type, GetFullName(entryData->type));
		return true;
	}
};

static bool Cmd_SetHotKeyItem_Execute(COMMAND_ARGS)
{
	TESForm* qkForm = NULL;
	UInt32 whichKey = 0;
	*result = 0;

	if(!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &whichKey, &qkForm))
		return true;

	whichKey -= 1;
	if (whichKey > 7)
		return true;
	_ClearHotKey ( whichKey );

	if (!qkForm)
		return true;

	ExtraContainerChanges* xChanges = static_cast <ExtraContainerChanges *>((*g_thePlayer)->baseExtraList.GetByType(kExtraData_ContainerChanges));
	DEBUG_PRINT("Mortacci   %u  %0X", whichKey, qkForm);
	if (xChanges && xChanges->data && xChanges->data->objList) {
		DEBUG_PRINT("Mortacci 2");
		for (tList<ExtraContainerChanges::EntryData>::Iterator xData = xChanges->data->objList->Begin(); !xData.End(); ++xData) {
			if (*xData && xData->type == qkForm && xData->extendData && !xData->extendData->IsEmpty()) {
				tList<ExtraDataList>::_Node* rem = xData->extendData->Head();
				if (!rem || !rem->item) {
					_MESSAGE("[WARNING]  Element  is invalid");
					continue;
				}
				ExtraQuickKey* qKey = (ExtraQuickKey*)rem->item->GetByType(kExtraData_QuickKey);//TODO this check only the first extradatalist, but more may be present in the EntryData 
				if (qKey) {
					rem->item->Remove(qKey);  //merge the Ge
					FormHeap_Free(qKey);
				}

			}
		}
	}
	DEBUG_PRINT("Mortacci 3");
	NiTPointerList <TESForm> * quickKey = &g_quickKeyList[whichKey];
	NiTPointerList <TESForm>::Node * qkNode = quickKey->start;
	if (!quickKey->numItems || !qkNode)	// quick key not yet assigned
	{
		qkNode = quickKey->AllocateNode();
		quickKey->start = qkNode;
		quickKey->end = qkNode;
		quickKey->numItems = 1;
	}

	qkNode->next = NULL;
	qkNode->prev = NULL;
	qkNode->data = qkForm;
	DEBUG_PRINT("Mortacci 4");
	ExtraQuickKey* xQKey = NULL;

	if (qkForm->typeID != kFormType_Spell)	//add ExtraQuickKey to new item's extra data list
	{
		if (xChanges && xChanges->data && xChanges->data->objList)		//look up form in player's inventory
		{
			DEBUG_PRINT("Mortacci 5");
			ExtraContainerChangesEntryFinder finder(qkForm);
			ExtraContainerChanges::EntryData* xEntry = xChanges->data->objList->Find(finder);
			if (xEntry)	{
				if (!xEntry->extendData){
					xEntry->extendData = (tList<ExtraDataList>*) tList<ExtraDataList>::Create();
				}
				if (xEntry->extendData->IsEmpty())
					xEntry->extendData->AddAt(ExtraDataList::Create(),0);
				if (!xQKey) {
					xQKey = ExtraQuickKey::Create();
					xQKey->keyID = whichKey;
				}

				xEntry->extendData->GetNthItem(0)->Add(xQKey);
			}
			else
				Console_Print("SetHotKeyItem >> Item not found in inventory");
		}
	}
	DEBUG_PRINT("Mortacci 6");
	return true;
}

static bool Cmd_IsModelPathValid_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Model, kGet);
}

static bool Cmd_IsIconPathValid_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Icon, kGet);
}

static bool IsBipedPathValid_Execute(COMMAND_ARGS, bool checkIcon)
{
	enum {
		kPath_Male,
		kPath_Female,
		kPath_MaleGround,
		kPath_FemaleGround
	};

	*result = 0;
	UInt32 whichPath = 0;
	TESForm* form = NULL;
	BSStringT* filePath = NULL;
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &whichPath, &form))
		return true;

	if (!form)
	{
		if (thisObj)
			form = thisObj->baseForm;
		else
			return true;
	}

	TESBipedModelForm* bip = (TESBipedModelForm *)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
	if (bip)
	{
		switch(whichPath)
		{
		case kPath_MaleGround:
			if (!checkIcon)
				filePath = PathStringFromForm(form, kVal_GroundMale, kGet);
			break;
		case kPath_FemaleGround:
			if (!checkIcon)
				filePath = PathStringFromForm(form, kVal_GroundFemale, kGet);
			break;
		case kPath_Male:
			if (checkIcon)
				filePath = PathStringFromForm(form, kVal_IconMale, kGet);
			else
				filePath = PathStringFromForm(form, kVal_BipedMale, kGet);
			break;
		case kPath_Female:
			if (checkIcon)
				filePath = PathStringFromForm(form, kVal_IconFemale, kGet);
			else
				filePath = PathStringFromForm(form, kVal_BipedFemale, kGet);
			break;
		default:
			break;
		}

		if (filePath && filePath->m_data)
		{
			std::string sFilePath;
			if (checkIcon)
				sFilePath = std::string("data\\textures\\menus\\icons\\") + std::string(filePath->m_data);
			else
				sFilePath = std::string("data\\meshes\\") + std::string(filePath->m_data);

			*result = (*g_FileFinder)->FindFile(sFilePath.c_str(), 0, 0, -1) ? 1 : 0;
		}
	}

	return true;
}

static bool Cmd_IsBipedModelPathValid_Execute(COMMAND_ARGS)
{
	return IsBipedPathValid_Execute(PASS_COMMAND_ARGS, false);
}

static bool Cmd_IsBipedIconPathValid_Execute(COMMAND_ARGS)
{
	return IsBipedPathValid_Execute(PASS_COMMAND_ARGS, true);
}

static bool Cmd_FileExists_Execute(COMMAND_ARGS)
{
	char filePath[512] = { 0 };
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &filePath))
		*result = (*g_FileFinder)->FindFile(filePath, 0, 0, -1) ? 1 : 0;

	return true;
}

static bool Cmd_SetNameEx_Execute(COMMAND_ARGS)
{
	if (!result) return true;

	TESForm* form = NULL;
	char	newName[kMaxMessageLength];

	if(!ExtractFormatStringArgs(0, newName, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetNameEx.numParams, &form))
		return true;

	if (!form && thisObj)
	{
		form = thisObj;

		// if we are changing the name of a mapmarker and the mapmenu is open, update map menu
		if (thisObj->baseForm && thisObj->baseForm->refID == kFormID_MapMarker)
		{
			MapMenu* mapMenu = (MapMenu*)GetMenuByType(kMenuType_Map);
			if (mapMenu)
				mapMenu->UpdateMarkerName(thisObj, newName);
		}
	}

	if (!form)
		return true;

	TESFullName* name = form->GetFullName();
	if (name)
		name->name.Set(newName);

	return true;
}

static bool BipedModelFlagFunc_Execute(COMMAND_ARGS, UInt32 flag, UInt32 mode)
{
	TESForm* form = NULL;
	UInt32 bToggleOn = 1;
	*result = 0;

	bool bExtracted = false;
	if (mode == kGet)
		bExtracted = ExtractArgs(PASS_EXTRACT_ARGS, &form);
	else if (mode == kSet)
		bExtracted = ExtractArgs(PASS_EXTRACT_ARGS, &bToggleOn, &form);

	if (!bExtracted)
		return true;
	else if (!form)
	{
		if (thisObj)
			form = thisObj->baseForm;
		else
			return true;
	}

	TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
	if (biped)
	{
		if (mode == kGet)
			*result = (biped->flags & flag) == flag;
		else
		{
			if (bToggleOn)
				biped->flags |= flag;
			else
				biped->flags &= ~flag;
		}
	}

	return true;
}

static bool Cmd_GetHidesRings_Execute(COMMAND_ARGS)
{
	return BipedModelFlagFunc_Execute(PASS_COMMAND_ARGS, 1 << TESBipedModelForm::kFlags_HidesRings, kGet);
}

static bool Cmd_GetHidesAmulet_Execute(COMMAND_ARGS)
{
	return BipedModelFlagFunc_Execute(PASS_COMMAND_ARGS, 1 << TESBipedModelForm::kFlags_HidesAmulets, kGet);
}

static bool Cmd_SetHidesRings_Execute(COMMAND_ARGS)
{
	return BipedModelFlagFunc_Execute(PASS_COMMAND_ARGS, 1 << TESBipedModelForm::kFlags_HidesRings, kSet);
}

static bool Cmd_SetHidesAmulet_Execute(COMMAND_ARGS)
{
	return BipedModelFlagFunc_Execute(PASS_COMMAND_ARGS, 1 << TESBipedModelForm::kFlags_HidesAmulets, kSet);
}

static bool Cmd_GetBipedSlotMask_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = -1;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &form))
	{
		if (!form && thisObj)
			form = thisObj->baseForm;

		TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
		if (biped)
			*result = biped->partMask;
	}

	return true;
}

static bool Cmd_SetBipedSlotMask_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 mask = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &mask, &form))
	{
		if (!form && thisObj)
			form = thisObj->baseForm;

		TESBipedModelForm* biped = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
		if (biped)
			biped->partMask = mask;
	}

	return true;
}

static bool FormMatchesTypes(UInt32 types[], UInt32 numTypes, TESForm* form)
{
	if (!form)
		return false;
	else if (!numTypes)
		return true;

	for (UInt32 i = 0; i < numTypes; i++)
	{
		if (form->typeID == types[i])
			return true;
	}

	return false;
}

static bool Cmd_GetItems_Execute(COMMAND_ARGS)
{
	// returns an array containing all items in calling object's inventory
	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetItems must be called within an OBSE expression.");
		return true;
	}

	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	UInt32 t[10] = { 0 };	// list of types to match, if omitted match all types
	if (!thisObj || !ExtractArgs(PASS_EXTRACT_ARGS, &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], &t[6], &t[7], &t[8], &t[9]))
		return true;

	UInt32 numTypes = 0;	// how many types are specified?
	while (t[numTypes] && numTypes < 10)
		numTypes++;

	UInt32	count = 0;

	// get pointers
	ExtraContainerChanges	* containerChanges = static_cast <ExtraContainerChanges *>(thisObj->baseExtraList.GetByType(kExtraData_ContainerChanges));

	// initialize a map of the ExtraData information
	// this will walk through the containerChanges once
	ExtraContainerInfo info(containerChanges ? containerChanges->data->objList : NULL);

	TESContainer* container = NULL;
	TESForm* baseForm = thisObj->GetBaseForm();
	if(baseForm)
		container = (TESContainer *)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESContainer, 0);

	// first walk the base container
	if(container)
	{
		for(TESContainer::Entry	* containerEntry = &container->list; containerEntry; containerEntry = containerEntry->next)
		{
			TESContainer::Data	* containerData = containerEntry->data;
			SInt32 numObjects = 0;
			if (info.IsValidContainerData(containerData, numObjects) && FormMatchesTypes(t, numTypes, containerData->type))
			{
				g_ArrayMap.SetElementFormID(arrID, count, containerData->type ? containerData->type->refID : 0);
				count++;
			}
		}
	}

	// now walk the remaining items in the map
	ExtraDataVec::iterator itEnd = info.m_vec.end();
	ExtraDataVec::iterator it = info.m_vec.begin();
	while (it != itEnd) {
		ExtraContainerChanges::EntryData* extraData = (*it);
		if (extraData && (extraData->countDelta > 0) && FormMatchesTypes(t, numTypes, extraData->type))
		{
			g_ArrayMap.SetElementFormID(arrID, count, extraData->type ? extraData->type->refID : 0);
			count++;
		}
		++it;
	}

#if _DEBUG && 0
	_MESSAGE("Inventory contents for %s", GetFullName(thisObj));
	g_ArrayMap.DumpArray(arrID);
#endif

	return true;
}

static bool Cmd_GetBaseItems_Execute(COMMAND_ARGS)
{
	// returns an array of arrays:
	// BaseItems[0]["item"] := form
	// BaseItems[0]["count"] := quantity

	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetBaseItems must be called within an OBSE expression.");
		return true;
	}

	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	TESForm* form = NULL;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		if (!form && thisObj)
			form = thisObj->baseForm;
	}

	TESContainer* cont = OBLIVION_CAST(form, TESForm, TESContainer);
	if (cont)
	{
		UInt32 idx = 0;
		for (TESContainer::Entry* entry = &cont->list; entry && entry->data; entry = entry->next)
		{
			ArrayID innerArr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
			g_ArrayMap.SetElementFormID(innerArr, "item", entry->data->type ? entry->data->type->refID : 0);
			g_ArrayMap.SetElementNumber(innerArr, "count", (double)entry->data->count);
			g_ArrayMap.SetElementArray(arr, idx, innerArr);
			idx++;
		}
	}

	return true;
}

static bool ChangeCurrentCharge(COMMAND_ARGS, bool bMod)
{
	float val = 0;
	if (thisObj && ExtractArgs(PASS_EXTRACT_ARGS, &val))
	{
		TESEnchantableForm* ench = OBLIVION_CAST(thisObj->baseForm, TESForm, TESEnchantableForm);
		if (ench)
		{
			ExtraCharge* xCharge = (ExtraCharge*)thisObj->baseExtraList.GetByType(kExtraData_Charge);
			float curCharge = xCharge ? xCharge->charge : ench->enchantment;
			float nuCharge = bMod ? val + curCharge : val;

			if (nuCharge == ench->enchantment)
			{
				if (xCharge)
				{
					thisObj->baseExtraList.Remove(xCharge);
					FormHeap_Free(xCharge);
					*result = 1;
				}
			}
			else if (nuCharge <= ench->enchantment && nuCharge >= 0)
			{
				if (xCharge)
				{
					xCharge->charge = nuCharge;
					*result = 1;
				}
				else
				{
					xCharge = ExtraCharge::Create();
					xCharge->charge = nuCharge;
					if (thisObj->baseExtraList.Add(xCharge))
						*result = 1;
					else
						FormHeap_Free(xCharge);
				}
			}
		}
	}

	return true;
}

static bool Cmd_SetCurrentCharge_Execute(COMMAND_ARGS)
{
	return ChangeCurrentCharge(PASS_COMMAND_ARGS, false);
}

static bool Cmd_ModCurrentCharge_Execute(COMMAND_ARGS)
{
	return ChangeCurrentCharge(PASS_COMMAND_ARGS, true);
}

static bool Cmd_EquipItem2_Execute(COMMAND_ARGS)
{
	// forces onEquip block to run

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor) {
		ExtraContainerDataList  preList = actor->GetEquippedEntryDataList();
		Cmd_EquipItem_Execute(PASS_COMMAND_ARGS);
		ExtraContainerDataList postList = actor->GetEquippedEntryDataList();

		// what was equipped (if anything)?
		for (UInt32 i = 0; i < postList.size(); i++) {
			if (std::find(preList.begin(), preList.end(), postList[i]) == preList.end()) {
				ExtraContainerChanges::EntryData* data = postList[i];
				if (data->extendData && data->extendData->GetNthItem(0)) {
					// mark the event
					data->extendData->GetNthItem(0)->MarkScriptEvent(ScriptEventList::kEvent_OnEquip, actor);
				}
				break;
			}
		}
	}

	return true;
}

static bool Cmd_EquipItem2NS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_EquipItem2_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);

	return true;
}

static bool Cmd_EquipMe_Execute(COMMAND_ARGS)
{
	Actor* owner = OBLIVION_CAST(contObj, TESObjectREFR, Actor);
	if (thisObj) {
		if (owner) {
			owner->EquipItem(thisObj->baseForm, 1, &thisObj->baseExtraList, 1, false);
		}
		else {	// inventory reference?
			InventoryReference* iref = InventoryReference::GetForRefID(thisObj->refID);
			if (iref) {
				iref->SetEquipped(true);
			}
		}
	}

	return true;
}

static bool Cmd_UnequipMe_Execute(COMMAND_ARGS)
{
	Actor* owner = OBLIVION_CAST(contObj, TESObjectREFR, Actor);
	_MESSAGE("%0X   %0X   %0X", thisObj, contObj, owner);
	if (thisObj) {
		if (owner) {
			owner->UnequipItem(thisObj->baseForm, 1, &thisObj->baseExtraList, 0, false, 0);
		}
		else {
			InventoryReference* iref = InventoryReference::GetForRefID(thisObj->refID);
			if (iref) {
				iref->SetEquipped(false);
			}
		}
	}

	return true;
}

static bool Cmd_IsEquipped_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (thisObj) {
		*result = thisObj->baseExtraList.HasType(kExtraData_Worn) ? 1.0 : 0.0;
	}

	return true;
}

static bool OverrideGameSounds_Execute(Cmd_Execute cmd, COMMAND_ARGS)
{
	static const UInt16 s_overwrittenInstructions = 0x0675;	// jnz [rel8]
	static const UInt32 s_patchAddr = 0x005E96E7l;			// in fn char* GetItemUpDownSound(TESForm* item, bool bUpSound, arg2)

	// nop out a jnz rel8 at beginning of fn to make it always return NULL (indicating no sound for specified item)
	SafeWrite16(s_patchAddr, 0x9090);
	// invoke the cmd
	bool cmdResult = cmd(PASS_COMMAND_ARGS);
	// restore overwritten instructions
	SafeWrite16(s_patchAddr, s_overwrittenInstructions);

	return cmdResult;
}

static bool Cmd_EquipItemSilent_Execute(COMMAND_ARGS)
{
	return OverrideGameSounds_Execute(Cmd_EquipItemNS_Execute, PASS_COMMAND_ARGS);
}

static bool Cmd_UnequipItemSilent_Execute(COMMAND_ARGS)
{
	return OverrideGameSounds_Execute(Cmd_UnequipItemNS_Execute, PASS_COMMAND_ARGS);
}

static bool Cmd_GetEquippedTorchTimeLeft_Execute(COMMAND_ARGS)
{
	*result = -1.0;
	feGetCurrentValue getCurrentVal(kVal_TimeLeft);
	FindEquipped(thisObj, kSlot_Torch, &getCurrentVal, result);
	return true;
}

static bool Cmd_SetGoldValue_T_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	UInt32 val = 0;
	TESForm* form = NULL;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &val, &form)) {
		if (!form && thisObj) {
			form = thisObj->baseForm;
		}

		if (form) {
			switch (form->typeID) {
				case kFormType_Ingredient:
					{
						IngredientItem* ingred = OBLIVION_CAST(form, TESForm, IngredientItem);
						if (ingred) {
							ingred->value = val;
							*result = 1.0;
						}
					}
					break;
				case kFormType_AlchemyItem:
					{
						AlchemyItem* alch = OBLIVION_CAST(form, TESForm, AlchemyItem);
						if (alch) {
							alch->SetGoldValue(val);
							*result = 1.0;
						}
					}
					break;
				default:
					{
						TESValueForm* valueForm = OBLIVION_CAST(form, TESForm, TESValueForm);
						if (valueForm) {
							valueForm->value = val;
							*result = 1.0;
						}
					}
			}
		}
	}

	return true;
}

#endif

CommandInfo kCommandInfo_GetNumItems =
{
	"GetNumItems",
	"gni",
	0,
	"return the number of items in an object's inventory",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetNumItems_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetInventoryItemType =
{
	"GetInventoryItemType",
	"giit",
	0,
	"returns a ref to the type of the inventory item",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetInventoryItemType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetInventoryObject =
{
	"GetInventoryObject",
	"GetInvObj",
	0,
	"returns the base object for the item in the specified inventory index",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetInventoryItemType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetEquipmentSlotType =
{
	"GetEquipmentSlotType",
	"gest",
	0,
	"returns a ref to the type of the specified equipment slot",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetEquipmentSlotType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetEquippedObject =
{
	"GetEquippedObject",
	"GetEqObj",
	0,
	"returns the base object of the item in the specified equipment slot",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetEquipmentSlotType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetEquipmentSlotMask[2] =
{
	{	"int", kParamType_Integer, 0 },
	{	"int (optional)", kParamType_Integer, 1 },
};

CommandInfo kCommandInfo_GetEquipmentSlotMask =
{
	"GetEquipmentSlotMask",
	"",
	0,
	"returns a ref to the type of the specified equipment slot, selected by bitfield comparison",
	1,
	2,
	kParams_GetEquipmentSlotMask,
	HANDLER(Cmd_GetEquipmentSlotMask_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetObjectValue[2] =
{
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_InventoryObject, 1 },
};

CommandInfo kCommandInfo_GetObjectValue =
{
	"GetObjectValue",
	"GetOV",
	0,
	"returns the specified value of the base object passed in",
	0,
	2,
	kParams_GetObjectValue,
	HANDLER(Cmd_GetObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCurrentValue =
{
	"GetCurrentValue",
	"GetCV",
	0,
	"returns the current specified value of the reference",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetCurrentValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetBaseObject =
{
	"GetBaseObject",
	"GetBO",
	0,
	"returns the base object type of the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetBaseObject_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetType =
{
	"GetObjectType",
	"GetOT",
	0,
	"returns the type id of the specified object or the reference on which it was called",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsWeapon =
{
	"IsWeapon",
	"IsWEAP",
	0,
	"returns 1 if the specified base object is a weapon and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsWeapon_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsAmmo =
{
	"IsAmmo",
	"IsAmmo",
	0,
	"returns 1 if the specified base object is ammo and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsAmmo_Execute),
	Cmd_Default_Parse,
	HANDLER_EVAL(Cmd_IsAmmo_Eval),
	1
};

CommandInfo kCommandInfo_IsArmor =
{
	"IsArmor",
	"IsARMO",
	0,
	"returns 1 if the specified base object is armor and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsArmor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsClothing =
{
	"IsClothing",
	"IsClot",
	0,
	"returns 1 if the specified base object is clothing and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsClothing_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsBook =
{
	"IsBook",
	"IsBook",
	0,
	"returns 1 if the specified base object is a book and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsBook_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsIngredient =
{
	"IsIngredient",
	"IsIngred",
	0,
	"returns 1 if the specified base object is an ingredient and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsIngredient_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsContainer =
{
	"IsContainer",
	"IsCont",
	0,
	"returns 1 if the specified base object is a container and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsContainer_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsKey =
{
	"IsKey",
	"IsKey",
	0,
	"returns 1 if the specified base object is a key and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsKey_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsAlchemyItem =
{
	"IsAlchemyItem",
	"IsPotion",
	0,
	"returns 1 if the specified base object is an alchemy item and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsAlchemyItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsApparatus =
{
	"IsApparatus",
	"IsAPPA",
	0,
	"returns 1 if the specified base object is an alchemy item and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsApparatus_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsSoulGem =
{
	"IsSoulGem",
	"IsGem",
	0,
	"returns 1 if the specified base object is a soul gem and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsSoulGem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsSigilStone =
{
	"IsSigilStone",
	"IsSigil",
	0,
	"returns 1 if the specified base object is a sigil stone and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsSigilStone_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsDoor =
{
	"IsDoor",
	"IsDoor",
	0,
	"returns 1 if the specified base object is a door and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsDoor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsActivator =
{
	"IsActivator",
	"IsACTI",
	0,
	"returns 1 if the specified base object is an activator and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsActivator_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsLight =
{
	"IsLight",
	"IsLIGH",
	0,
	"returns 1 if the specified base object is a light and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsLight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMiscItem =
{
	"IsMiscItem",
	"IsMisc",
	0,
	"returns 1 if the specified base object is a misc item and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsMiscItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsFurniture =
{
	"IsFurniture",
	"IsFurn",
	0,
	"returns 1 if the specified base object is furniture and 0 if it is not",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsFurniture_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetECV[2] =
{
	{	"value", kParamType_Integer, 0 },
	{	"slot", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_GetEquippedCurrentValue =
{
	"GetEquippedCurrentValue",
	"GetECV",
	0,
	"returns the current specified value for the given equipment slot",
	1,
	2,
	kParams_GetECV,
	HANDLER(Cmd_GetEquippedCurrentValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetEquippedObjectValue =
{
	"GetEquippedObjectValue",
	"GetEOV",
	0,
	"returns the specified base object value for the given equipment slot",
	1,
	2,
	kParams_GetECV,
	HANDLER(Cmd_GetEquippedObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

#if 0
static ParamInfo kParams_SetECV[3] =
{
	{	"which value", kParamType_Integer, 0 },
	{	"float", kParamType_Float, 0 },
	{	"slot", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_SetEquippedCurrentValue =
{
	"SetEquippedCurrentValue",
	"SetECV",
	0,
	"sets the specified current value on the given equipment slot",
	1,
	3,
	kParams_SetECV,
	HANDLER(Cmd_SetEquippedCurrentValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetEquippedObjectValue =
{
	"SetEquippedObjectValue",
	"SetEOV",
	0,
	"sets the specified base value on the given equipment slot",
	1,
	3,
	kParams_SetECV,
	HANDLER(Cmd_SetEquippedObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetModOV[4] =
{
	{	"which value", kParamType_Integer, 0 },
	{	"value", kParamType_Float, 0 },
	{	"magic item", kParamType_MagicItem, 0 },
	{	"type", kParamType_InventoryObject, 1}
};

CommandInfo kCommandInfo_SetObjectValue =
{
	"SetObjectValue",
	"SetOV",
	0,
	"sets the specified base value on the calling reference or passed object",
	0,
	4,
	kParams_SetModOV,
	HANDLER(Cmd_SetObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModObjectValue =
{
	"ModObjectValue",
	"ModOV",
	0,
	"sets the specified base value on the calling reference or passed object",
	0,
	4,
	kParams_SetModOV,
	HANDLER(Cmd_ModObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_TestSetObjectValue =
{
	"TestSetObjectValue",
	"TestSetOV",
	0,
	"sets the specified base value on the calling reference or passed object",
	0,
	4,
	kParams_SetModOV,
	HANDLER(Cmd_TestSetObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_TestModObjectValue =
{
	"TestModObjectValue",
	"TestModOV",
	0,
	"sets the specified base value on the calling reference or passed object",
	0,
	4,
	kParams_SetModOV,
	HANDLER(Cmd_TestModObjectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
#endif

CommandInfo kCommandInfo_SetCurrentHealth =
{
	"SetCurrentHealth",
	"SetCHealth",
	0,
	"sets the current health to the specified value",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_SetCurrentHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetModObjectFloat[2] =
{
	{	"value", kParamType_Float, 0 },
	{	"type", kParamType_InventoryObject, 1}
};

static ParamInfo kParams_SetObjectInteger[2] =
{
	{	"value", kParamType_Integer, 0 },
	{	"type", kParamType_InventoryObject, 1}
};

static ParamInfo kParams_SetObjectMagicItem[2] =
{
	{	"value", kParamType_MagicItem, 0 },
	{	"type", kParamType_InventoryObject, 1}
};

static ParamInfo kParams_SetObjectString[2] =
{
	{	"value", kParamType_String, 0 },
	{	"type", kParamType_InventoryObject, 1}
};

static ParamInfo kParams_CopyObjectPath[2] =
{
	{	"src", kParamType_InventoryObject, 0},
	{	"dest", kParamType_InventoryObject, 1}
};

static ParamInfo kParams_SetEquippedInt[2] =
{
	{	"value", kParamType_Integer, 0 },
	{	"slot", kParamType_Integer, 0}
};

static ParamInfo kParams_ModEquippedFloat[2] =
{
	{	"value", kParamType_Float, 0 },
	{	"slot", kParamType_Integer, 0}
};

// Weight
CommandInfo kCommandInfo_GetWeight =
{
	"GetWeight",
	"",
	0,
	"returns the specified value of the base object passed in",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetWeight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeight =
{
	"SetWeight",
	"",
	0,
	"Set the weight of the specified object or object reference.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_SetWeight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModWeight =
{
	"ModWeight",
	"",
	0,
	"Modifies the weight of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModWeight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Gold value
CommandInfo kCommandInfo_GetGoldValue =
{
	"GetGoldValue",
	"",
	0,
	"returns the fold value of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetGoldValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetGoldValue =
{
	"SetGoldValue",
	"",
	0,
	"Sets the weight of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetGoldValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModGoldValue =
{
	"ModGoldValue",
	"",
	0,
	"Modifies the weight of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModGoldValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Health
CommandInfo kCommandInfo_GetObjectHealth =
{
	"GetObjectHealth",
	"",
	0,
	"returns the health of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetObjectHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetObjectHealth =
{
	"SetObjectHealth",
	"",
	0,
	"Sets the health of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetObjectHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModObjectHealth =
{
	"ModObjectHealth",
	"",
	0,
	"Modifies the health of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModObjectHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// GetCurrentHealth
// SetCurrentHealth
// ModCurrentHealth

// EquipmentSlot
CommandInfo kCommandInfo_GetEquipmentSlot =
{
	"GetEquipmentSlot",
	"",
	0,
	"returns the equipment slot of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetEquipmentSlot_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetEquipmentSlot =
{
	"SetEquipmentSlot",
	"",
	0,
	"Sets the health of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetEquipmentSlot_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Enchantment Charge
CommandInfo kCommandInfo_GetObjectCharge =
{
	"GetObjectCharge",
	"",
	0,
	"returns the enchantment charge of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetObjectCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetObjectCharge =
{
	"SetObjectCharge",
	"",
	0,
	"Sets the enchantment charge of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetObjectCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModObjectCharge =
{
	"ModObjectCharge",
	"",
	0,
	"Modifies the enchantment charge of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModObjectCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

//	GetCurrentEnchantentCharge
//	SetCurrentEnchantmentCharge	int
//	ModCurrentEnchantmentCharge	int

// Quest Item
CommandInfo kCommandInfo_IsQuestItem =
{
	"IsQuestItem",
	"",
	0,
	"returns 1 if the object or calling reference is a quest item",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsQuestItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetQuestItem =
{
	"SetQuestItem",
	"",
	0,
	"Sets whether the specified object or object reference is a quest item.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetQuestItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Enchantment
CommandInfo kCommandInfo_GetEnchantment =
{
	"GetEnchantment",
	"",
	0,
	"returns any enchantment of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetEnchantment_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetEnchantment =
{
	"SetEnchantment",
	"",
	0,
	"Sets the enchantment of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectMagicItem,
	HANDLER(Cmd_SetEnchantment_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveEnchantment =
{
	"RemoveEnchantment",
	"",
	0,
	"removes and returns the enchantment on the calling object or reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_RemoveEnchantment_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Attack Damage
CommandInfo kCommandInfo_GetAttackDamage =
{
	"GetAttackDamage",
	"",
	0,
	"returns the attack damage of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetAttackDamage_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetAttackDamage =
{
	"SetAttackDamage",
	"",
	0,
	"Sets the attack damage of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetAttackDamage_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModAttackDamage =
{
	"ModAttackDamage",
	"",
	0,
	"Modifies the attack damage of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModAttackDamage_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Weapon Reach
CommandInfo kCommandInfo_GetWeaponReach =
{
	"GetWeaponReach",
	"",
	0,
	"returns the weapon reach of the specified object or object reference.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetWeaponReach_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeaponReach =
{
	"SetWeaponReach",
	"",
	0,
	"Set the weapon reach of the specified object or object reference.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_SetWeaponReach_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModWeaponReach =
{
	"ModWeaponReach",
	"",
	0,
	"Modifies the weapon reach of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModWeaponReach_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Weapon Speed
CommandInfo kCommandInfo_GetWeaponSpeed =
{
	"GetWeaponSpeed",
	"",
	0,
	"returns the weapon speed of the object or reference.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetWeaponSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeaponSpeed =
{
	"SetWeaponSpeed",
	"",
	0,
	"Set the weapon speed of the specified object or object reference.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_SetWeaponSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModWeaponSpeed =
{
	"ModWeaponSpeed",
	"",
	0,
	"Modifies the weapon speed of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModWeaponSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Weapon Type
CommandInfo kCommandInfo_GetWeaponType =
{
	"GetWeaponType",
	"",
	0,
	"returns the weapon type of the object or calling reference.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetWeaponType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetWeaponType =
{
	"SetWeaponType",
	"",
	0,
	"Sets the weapon type of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetWeaponType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Ignores Resistance
CommandInfo kCommandInfo_GetIgnoresResistance =
{
	"GetIgnoresResistance",
	"",
	0,
	"returns 1 if the object or calling reference ignores weapon resistance.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetIgnoresResistance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetIgnoresResistance =
{
	"SetIgnoresResistance",
	"",
	0,
	"Sets the ingores Resistance of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetIgnoresResistance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// GetCurrentWeaponPoision
// SetCurrentWeaponPoison ref

// Armor Rating
CommandInfo kCommandInfo_GetArmorAR =
{
	"GetArmorAR",
	"",
	0,
	"returns the armor rating of the object or calling reference",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetArmorAR_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetArmorAR =
{
	"SetArmorAR",
	"",
	0,
	"Sets the armor rating of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetArmorAR_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModArmorAR =
{
	"ModArmorAR",
	"",
	0,
	"Modifies the armor rating of the specified object or object reference by the passed amount.",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModArmorAR_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Armor Type
CommandInfo kCommandInfo_GetArmorType =
{
	"GetArmorType",
	"",
	0,
	"returns the armor type of the object or calling reference.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetArmorType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetArmorType =
{
	"SetArmorType",
	"",
	0,
	"Sets the armor type of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetArmorType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Soul Level
CommandInfo kCommandInfo_SoulLevel =
{
	"GetSoulLevel",
	"",
	0,
	"returns the soul level of the object or calling reference.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetSoulLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetSoulLevel =
{
	"SetSoulLevel",
	"",
	0,
	"Sets the soul level of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetSoulLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SoulGem Capacity
CommandInfo kCommandInfo_GetSoulGemCapacity =
{
	"GetSoulGemCapacity",
	"",
	0,
	"returns the soul gem capacity of the object or calling reference.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetSoulGemCapacity_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetSoulGemCapacity =
{
	"SetSoulGemCapacity",
	"",
	0,
	"Sets the soul gem capacity of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetSoulGemCapacity_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsFood =
{
	"IsFood",
	"",
	0,
	"returns 1 if specified object or object reference is food.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsFood_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Set is Food
CommandInfo kCommandInfo_SetIsFood =
{
	"SetIsFood",
	"",
	0,
	"Sets the is food of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetIsFood_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsPlayable =
{
	"IsPlayable",
	"",
	0,
	"returns 1 if specified clothing or armor is playable.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsPlayable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsPlayable2 =
{
	"IsPlayable2",
	"",
	0,
	"returns 1 unless the object is clothing marked as unplayable",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsPlayable2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// Set is Food
CommandInfo kCommandInfo_SetPlayable =
{
	"SetIsPlayable",
	"",
	0,
	"sets whether the specified clothing or armor is playable.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetIsPlayable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsPoison =
{
	"IsPoison",
	"",
	0,
	"returns 1 if specified object or object reference is poison.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsPoison_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetName
CommandInfo kCommandInfo_SetName =
{
	"SetName",
	"",
	0,
	"Sets the name of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareName
CommandInfo kCommandInfo_CompareName =
{
	"CompareName",
	"NameIncludes",
	0,
	"returns 1 if the name of the specified object or object reference includes the passed string.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyName
CommandInfo kCommandInfo_CopyName =
{
	"CopyName",
	"",
	0,
	"copies the name of one object to another",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModName
CommandInfo kCommandInfo_ModName =
{
	"ModName",
	"",
	0,
	"Modifies the name of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// AppendToName
CommandInfo kCommandInfo_AppendToName =
{
	"AppendToName",
	"",
	0,
	"appends a string to the name of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_AppendToName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetModelPath
CommandInfo kCommandInfo_SetModelPath =
{
	"SetModelPath",
	"",
	0,
	"Sets the model path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetModelPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModModelPath
CommandInfo kCommandInfo_ModModelPath =
{
	"ModModelPath",
	"",
	0,
	"Modifies the model path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModModelPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareModelPath
CommandInfo kCommandInfo_CompareModelPath =
{
	"CompareModelPath",
	"ModelPathIncludes",
	0,
	"Returns 1 if the given string is part of the model path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareModelPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyModelPath
CommandInfo kCommandInfo_CopyModelPath =
{
	"CopyModelPath",
	"",
	0,
	"copies the model path from one object to another",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyModelPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetIconPath
CommandInfo kCommandInfo_SetIconPath =
{
	"SetIconPath",
	"",
	0,
	"Sets the icon path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModIconPath
CommandInfo kCommandInfo_ModIconPath =
{
	"ModIconPath",
	"",
	0,
	"Modifies the icon path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareIconPath
CommandInfo kCommandInfo_CompareIconPath =
{
	"CompareIconPath",
	"IconPathIncludes",
	0,
	"Returns 1 if the given string is part of the icon path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyIconPath
CommandInfo kCommandInfo_CopyIconPath =
{
	"CopyIconPath",
	"",
	0,
	"copies the icon path from one object to another",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetMaleBipedPath
CommandInfo kCommandInfo_SetMaleBipedPath =
{
	"SetMaleBipedPath",
	"",
	0,
	"Sets the male biped path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetMaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModMaleBipedPath
CommandInfo kCommandInfo_ModMaleBipedPath =
{
	"ModMaleBipedPath",
	"",
	0,
	"Modifies the male biped path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModMaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareMaleBipedPath
CommandInfo kCommandInfo_CompareMaleBipedPath =
{
	"CompareMaleBipedPath",
	"MaleBipedPathIncludes",
	0,
	"Returns 1 if the given string is part of the MaleBiped path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareMaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyMaleBipedPath
CommandInfo kCommandInfo_CopyMaleBipedPath =
{
	"CopyMaleBipedPath",
	"",
	0,
	"",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyMaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetFemaleBipedPath
CommandInfo kCommandInfo_SetFemaleBipedPath =
{
	"SetFemaleBipedPath",
	"",
	0,
	"Sets the female biped path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetFemaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModFemaleBipedPath
CommandInfo kCommandInfo_ModFemaleBipedPath =
{
	"ModFemaleBipedPath",
	"",
	0,
	"Modifies the Female biped path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModFemaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareFemaleBipedPath
CommandInfo kCommandInfo_CompareFemaleBipedPath =
{
	"CompareFemaleBipedPath",
	"FemaleBipedPathIncludes",
	0,
	"Returns 1 if the given string is part of the FemaleBiped path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareFemaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyFemaleBipedPath
CommandInfo kCommandInfo_CopyFemaleBipedPath =
{
	"CopyFemaleBipedPath",
	"",
	0,
	"",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyFemaleBipedPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetMaleGroundPath
CommandInfo kCommandInfo_SetMaleGroundPath =
{
	"SetMaleGroundPath",
	"",
	0,
	"Sets the male ground path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetMaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModMaleGroundPath
CommandInfo kCommandInfo_ModMaleGroundPath =
{
	"ModMaleGroundPath",
	"",
	0,
	"Modifies the male Ground path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModMaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareMaleGroundPath
CommandInfo kCommandInfo_CompareMaleGroundPath =
{
	"CompareMaleGroundPath",
	"MaleGroundPathIncludes",
	0,
	"Returns 1 if the given string is part of the MaleGround path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareMaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyMaleGroundPath
CommandInfo kCommandInfo_CopyMaleGroundPath =
{
	"CopyMaleGroundPath",
	"",
	0,
	"",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyMaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetFemaleGroundPath
CommandInfo kCommandInfo_SetFemaleGroundPath =
{
	"SetFemaleGroundPath",
	"",
	0,
	"Sets the female ground path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetFemaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModFemaleGroundPath
CommandInfo kCommandInfo_ModFemaleGroundPath =
{
	"ModFemaleGroundPath",
	"",
	0,
	"Modifies the Female Ground path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModFemaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareFemaleGroundPath
CommandInfo kCommandInfo_CompareFemaleGroundPath =
{
	"CompareFemaleGroundPath",
	"FemaleGroundPathIncludes",
	0,
	"Returns 1 if the given string is part of the FemaleGround path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareFemaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyFemaleGroundPath
CommandInfo kCommandInfo_CopyFemaleGroundPath =
{
	"CopyFemaleGroundPath",
	"",
	0,
	"",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyFemaleGroundPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetMaleIconPath
CommandInfo kCommandInfo_SetMaleIconPath =
{
	"SetMaleIconPath",
	"",
	0,
	"Sets the male icon path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetMaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModMaleIconPath
CommandInfo kCommandInfo_ModMaleIconPath =
{
	"ModMaleIconPath",
	"",
	0,
	"Modifies the male Icon path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModMaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareMaleIconPath
CommandInfo kCommandInfo_CompareMaleIconPath =
{
	"CompareMaleIconPath",
	"MaleIconPathIncludes",
	0,
	"Returns 1 if the given string is part of the MaleIcon path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareMaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyMaleIconPath
CommandInfo kCommandInfo_CopyMaleIconPath =
{
	"CopyMaleIconPath",
	"",
	0,
	"",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyMaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetFemaleIconPath
CommandInfo kCommandInfo_SetFemaleIconPath =
{
	"SetFemaleIconPath",
	"",
	0,
	"Sets the female icon path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_SetFemaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModFemaleIconPath
CommandInfo kCommandInfo_ModFemaleIconPath =
{
	"ModFemaleIconPath",
	"",
	0,
	"Modifies the Female Icon path of the specified object or object reference.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_ModFemaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CompareFemaleIconPath
CommandInfo kCommandInfo_CompareFemaleIconPath =
{
	"CompareFemaleIconPath",
	"FemaleIconPathIncludes",
	0,
	"Returns 1 if the given string is part of the FemaleIcon path.",
	0,
	2,
	kParams_SetObjectString,
	HANDLER(Cmd_CompareFemaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CopyFemaleIconPath
CommandInfo kCommandInfo_CopyFemaleIconPath =
{
	"CopyFemaleIconPath",
	"",
	0,
	"",
	0,
	2,
	kParams_CopyObjectPath,
	HANDLER(Cmd_CopyFemaleIconPath_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// GetEquippedCurrentHealth
CommandInfo kCommandInfo_GetEquippedCurrentHealth =
{
	"GetEquippedCurrentHealth",
	"GetECHealth",
	0,
	"returns the current health of the actor's specified equipment slot",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetEquippedCurrentHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetEquippedCurrentHealth
CommandInfo kCommandInfo_SetEquippedCurrentHealth =
{
	"SetEquippedCurrentHealth",
	"SetECHealth",
	0,
	"Sets the health of the item in the specified equipment slot",
	1,
	2,
	kParams_SetEquippedInt,
	HANDLER(Cmd_SetEquippedCurrentHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModEquippedCurrentHealth
CommandInfo kCommandInfo_ModEquippedCurrentHealth =
{
	"ModEquippedCurrentHealth",
	"ModECHealth",
	0,
	"modifies the health of the item in the specified equipment slot",
	1,
	2,
	kParams_ModEquippedFloat,
	HANDLER(Cmd_ModEquippedCurrentHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// GetEquippedCurrentCharge
CommandInfo kCommandInfo_GetEquippedCurrentCharge =
{
	"GetEquippedCurrentCharge",
	"GetECCharge",
	0,
	"returns the current charge of the actor's specified equipment slot",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetEquippedCurrentCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// SetEquippedCurrentCharge
CommandInfo kCommandInfo_SetEquippedCurrentCharge =
{
	"SetEquippedCurrentCharge",
	"SetECCharge",
	0,
	"Sets the charge of the item in the specified equipment slot",
	1,
	2,
	kParams_SetEquippedInt,
	HANDLER(Cmd_SetEquippedCurrentCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// ModEquippedCurrentCharge
CommandInfo kCommandInfo_ModEquippedCurrentCharge =
{
	"ModEquippedCurrentCharge",
	"ModECCharge",
	0,
	"modifies the charge of the item in the specified equipment slot",
	1,
	2,
	kParams_ModEquippedFloat,
	HANDLER(Cmd_ModEquippedCurrentCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// GetEquippedWeaponPoison
CommandInfo kCommandInfo_GetEquippedWeaponPoison =
{
	"GetEquippedWeaponPoison",
	"GetEWPoison",
	0,
	"gets the poison on the equipped weapon.",
	1,
	0,
	0,
	HANDLER(Cmd_GetEquippedWeaponPoison_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveEquippedWeaponPoison =
{
	"RemoveEquippedWeaponPoison",
	"RemEWPoison",
	0,
	"removes the poison on the equipped weapon and returns it.",
	1,
	0,
	0,
	HANDLER(Cmd_RemoveEquippedWeaponPoison_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

ParamInfo kParamInfo_OneMagicItem[1] =
{
	{	"poison", kParamType_MagicItem, 0 },
};

// SetEquippedWeaponPoison
CommandInfo kCommandInfo_SetEquippedWeaponPoison =
{
	"SetEquippedWeaponPoison",
	"SetEWPoison",
	0,
	"Sets the poison on the equipped weapon.",
	1,
	1,
	kParamInfo_OneMagicItem,
	HANDLER(Cmd_SetEquippedWeaponPoison_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// IsClonedForm
CommandInfo kCommandInfo_IsClonedForm =
{
	"IsClonedForm",
	"IsCloned",
	0,
	"returns if the specified form is a cloned object.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsClonedForm_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

// CloneForm
CommandInfo kCommandInfo_CloneForm =
{
	"CloneForm",
	"",
	0,
	"Clones the specified base object.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_CloneForm_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCurrentHealth =
{
	"GetCurrentHealth",
	"",
	0,
	"returns the current health of the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetCurrentHealth_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCurrentCharge =
{
	"GetCurrentCharge",
	"",
	0,
	"returns the current charge of the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetCurrentCharge_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCurrentSoulLevel =
{
	"GetCurrentSoulLevel",
	"",
	0,
	"returns the current soul level of the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetCurrentSoulLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetBookCantBeTaken =
{
	"GetBookCantBeTaken",
	"",
	0,
	"returns 1 if the calling book or passed reference can't be taken",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetBookCantBeTaken_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetBookCantBeTaken =
{
	"SetBookCantBeTaken",
	"",
	0,
	"Sets whether the book can't be taken.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetBookCantBeTaken_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetBookIsScroll =
{
	"GetBookIsScroll",
	"IsScroll",
	0,
	"returns 1 if the calling book or passed reference is a scroll.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetBookIsScroll_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetBookIsScroll =
{
	"SetBookIsScroll",
	"",
	0,
	"Sets whether the book is a scroll.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetBookIsScroll_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetBookSkillTaught =
{
	"GetBookSkillTaught",
	"",
	0,
	"returns the skill the calling book or passed reference teaches.",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetBookSkillTaught_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetBookSkillTaught =
{
	"SetBookSkillTaught",
	"",
	0,
	"Sets the skill the book teaches.",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetBookSkillTaught_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

//CommandInfo kCommandInfo_SetBookSkillTaughtC =
//{
//	"SetBookSkillTaughtC",
//	"",
//	0,
//	"Sets the skill the book teaches.",
//	0,
//	2,
//	kParams_SetObjectInteger,
//	HANDLER(Cmd_SetBookSkillTaughtC_Execute),
//	Cmd_Default_Parse,
//	NULL,
//	0
//};

CommandInfo kCommandInfo_GetApparatusType =
{
	"GetApparatusType",
	"GetAPPAType",
	0,
	"returns the type of the referenced or passed alchemy apparatus",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetApparatusType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetQuality =
{
	"GetQuality",
	"",
	0,
	"returns the quality of the referenced or passed object",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetQuality_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetApparatusType =
{
	"SetApparatusType",
	"SetAPPAType",
	0,
	"Sets the type of the alchemy apparatus",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetApparatusType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetQuality =
{
	"SetQuality",
	"",
	0,
	"Sets the quality of the object",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_SetQuality_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModQuality =
{
	"ModQuality",
	"",
	0,
	"modifies the quality of the object",
	0,
	2,
	kParams_SetModObjectFloat,
	HANDLER(Cmd_ModQuality_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_CompareNames[2] =
{
	{	"compare to",	kParamType_InventoryObject,	0	},
	{	"base object",	kParamType_InventoryObject, 1	},
};

CommandInfo kCommandInfo_CompareNames =
{
	"CompareNames",
	"CompNames",
	0,
	"returns -1 if first < second, 1 if first > second, 0 if equal",
	0,
	2,
	kParams_CompareNames,
	HANDLER(Cmd_CompareNames_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneOptionalContainer[1] =
{
	{	"container",	kParamType_Container, 1	},
};

CommandInfo kCommandInfo_GetContainerRespawns =
{
	"GetContainerRespawns",
	"IsUnsafeContainer",
	0,
	"returns 1 if the container respawns",
	0,
	1,
	kParams_OneOptionalContainer,
	HANDLER(Cmd_GetContainerRespawns_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetContainerRespawns[2] =
{
	{	"respawns",		kParamType_Integer,		  0 },
	{	"container",	kParamType_Container, 1	},
};

CommandInfo kCommandInfo_SetContainerRespawns =
{
	"SetContainerRespawns",
	"SetUnsafeContainer",
	0,
	"sets whether the container respawns",
	0,
	2,
	kParams_SetContainerRespawns,
	HANDLER(Cmd_SetContainerRespawns_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsLightCarriable =
{
	"IsLightCarriable", "CanCarry",
	0,
	"returns true if the light can be placed in an inventory",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsLightCarriable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetLightRadius =
{
	"GetLightRadius", "GetRadius",
	0,
	"returns the radius of the light",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetLightRadius_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetLightRadius =
{
	"SetLightRadius", "SetRadius",
	0,
	"sets the radius of the light",
	0,
	2,
	kParams_SetObjectInteger,
	HANDLER(Cmd_SetLightRadius_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCurrentSoulLevel =
{
	"SetCurrentSoulLevel", "",
	0,
	"sets the soul level of the current reference",
	1, 1, kParams_OneInt,
	HANDLER(Cmd_SetCurrentSoulLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_HasName =
{
	"HasName",
	"",
	0,
	"returns 1 if the object has a name with 1 or more characters",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_HasName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_AddItem[2] =
{
	{	"item",		kParamType_InventoryObject,	0	},
	{	"quantity",	kParamType_Integer,			0	},
};

CommandInfo kCommandInfo_AddItemNS =
{
	"AddItemNS",
	"",
	0,
	"version of AddItem which doesn't generate UI messages",
	1,
	2,
	kParams_AddItem,
	HANDLER(Cmd_AddItemNS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveItemNS =
{
	"RemoveItemNS",
	"",
	0,
	"version of RemoveItem which doesn't generate UI messages",
	1,
	2,
	kParams_AddItem,
	HANDLER(Cmd_RemoveItemNS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_EquipItem[2] =
{
	{	"item",		kParamType_InventoryObject,	0	},
	{	"lockEquip",kParamType_Integer,			1	},
};

CommandInfo kCommandInfo_EquipItemNS =
{
	"EquipItemNS",
	"",
	0,
	"version of EquipItem which doesn't generate UI messages",
	1,
	2,
	kParams_EquipItem,
	HANDLER(Cmd_EquipItemNS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_UnequipItemNS =
{
	"UnequipItemNS",
	"",
	0,
	"version of UnequipItem which doesn't generate UI messages",
	1,
	2,
	kParams_EquipItem,
	HANDLER(Cmd_UnequipItemNS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetFullGoldValue =
{
	"GetFullGoldValue",
	"",
	0,
	"returns gold value of item including enchantment value",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetFullGoldValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetHotKeyItem =
{
	"GetHotKeyItem", "",
	0,
	"returns the item or spell bound to the specified hotkey",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetHotKeyItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ClearHotKey =
{
	"ClearHotKey", "ClearHotKeyItem",
	0,
	"clears the item or spell associated with the specified hotkey",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_ClearHotKey_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetHotKeyItem[2] =
{
	{	"hotkey",	kParamType_Integer,			0	},
	{	"item",		kParamType_InventoryObject,	0	},
};

CommandInfo kCommandInfo_SetHotKeyItem =
{
	"SetHotKeyItem", "",
	0,
	"sets the spell or item associated with a hotkey",
	0,
	2,
	kParams_SetHotKeyItem,
	HANDLER(Cmd_SetHotKeyItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(IsModelPathValid,
			   "returns true if the object's model path is valid",
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(IsIconPathValid,
			   "returns true if the object's icon path is valid",
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(IsBipedModelPathValid,
			   "returns true if the biped model path is valid",
			   0,
			   2,
			   kParams_GetObjectValue);

DEFINE_COMMAND(IsBipedIconPathValid,
			   "returns true if the biped icon path is valid",
			   0,
			   2,
			   kParams_GetObjectValue);

DEFINE_COMMAND(FileExists,
			   "returns true if the specified file exists",
			   0,
			   1,
			   kParams_OneString);

static ParamInfo kParams_SetNameEx[22] =
{
	FORMAT_STRING_PARAMS,
	{"inventory object", kParamType_InventoryObject,	1	},
};

DEFINE_COMMAND(SetNameEx,
			   "sets the name of the object based on the format string",
			   0,
			   22,
			   kParams_SetNameEx);

DEFINE_COMMAND(GetHidesAmulet,
			   returns true if the clothing hides amulets,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetHidesRings,
			   returns true if the clothing hides rings,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(SetHidesAmulet,
			   toggles the hides amulets flag,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInventoryObject);

DEFINE_COMMAND(SetHidesRings,
			   toggles the hides rings flag,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInventoryObject);

DEFINE_COMMAND(IsFlora,
			   returns true if the object is a harvestable plant,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetBipedSlotMask,
			   returns the slot mask associated with a wearable item,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(SetBipedSlotMask,
			   sets the slots used by a wearable item,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInventoryObject);

static ParamInfo kParams_GetItems[] =
{
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
	{	"formType",	kParamType_Integer,	1	},
};

DEFINE_COMMAND(GetItems, returns an array containing calling objects inventory items, 1, 10, kParams_GetItems);

DEFINE_COMMAND(GetBaseItems, returns an array containing the inventory of a base container, 0, 1, kParams_OneOptionalActorBase);

DEFINE_COMMAND(SetCurrentCharge, sets the charge of the calling object, 1, 1, kParams_OneFloat);
DEFINE_COMMAND(ModCurrentCharge, mods the charge of the calling object, 1, 1, kParams_OneFloat);

DEFINE_COMMAND(EquipItem2, equips and runs onequip block, 1, 2, kParams_EquipItem);
DEFINE_COMMAND(EquipItem2NS, equips and runs onequip block without spam, 1, 2, kParams_EquipItem)
DEFINE_COMMAND(EquipMe, equips the calling object on its owner, 1, 0, NULL);
DEFINE_COMMAND(UnequipMe, unequips the calling object on its owner, 1, 0, NULL);

DEFINE_COMMAND(IsEquipped, returns 1 if the calling object is currently being worn, 1, 0, NULL);

DEFINE_COMMAND(EquipItemSilent, equips the item without messages or sounds, 1, 2, kParams_EquipItem);
DEFINE_COMMAND(UnequipItemSilent, unequips the item without messages or soudns, 1, 2, kParams_EquipItem);

DEFINE_COMMAND(GetEquippedTorchTimeLeft, returns the time left for the equipped torch, 1, 0, NULL);
DEFINE_COMMAND(SetGoldValue_T, sets the value of the item without saving the change in the savegame, 0, 2, kParams_SetObjectInteger);