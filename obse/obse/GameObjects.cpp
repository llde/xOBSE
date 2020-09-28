#include "obse/GameObjects.h"
#include "obse/GameAPI.h"
#include "obse/GameData.h"
#include "obse/NiObjects.h"
#include "obse/GameTasks.h"
#include "obse/Tasks.h"
#include "obse_common/SafeWrite.h"

typedef Sky * (* _Sky_GetSingleton)(void);

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

PlayerCharacter ** g_thePlayer = (PlayerCharacter **)0x00AEAAE4;

static const UInt32	s_Actor_EquipItem =					0x005E6380;
static const UInt32	s_Actor_GetBaseActorValue =			0x005E4D30;

static const UInt32	s_PlayerCharacter_SetActiveSpell =	0x00650C30;

static const _Sky_GetSingleton	Sky_GetSingleton = (_Sky_GetSingleton)0x00537420;
static const UInt32				s_Sky_RefreshClimate = 0x00537C00;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2

PlayerCharacter ** g_thePlayer = (PlayerCharacter **)0x00B333C4;

static const UInt32	s_Actor_EquipItem =					0x005FACE0;
static const UInt32	s_Actor_GetBaseActorValue =			0x005F1750;

static const UInt32	s_PlayerCharacter_SetActiveSpell =	0x006641B0;

static const _Sky_GetSingleton	Sky_GetSingleton = (_Sky_GetSingleton)0x00542F10;
static const UInt32				s_Sky_RefreshClimate = 0x00543270;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

PlayerCharacter **  g_thePlayer =				(PlayerCharacter **)0x00B333C4;
static UInt8*		g_bUpdatePlayerModel =		(UInt8*)0x00B33D80;	// this is set to true when player confirms change of race in RaceSexMenu -
																	// IF requires change of skeleton - and back to false when model updated
static NiObject **	g_3rdPersonCameraNode =				(NiObject**)0x00B3BB10;
static NiObject **	g_1stPersonCameraBipedNode =		(NiObject**)0x00B3BB14;
static NiObject **	g_1stPersonCameraNode =				(NiObject**)0x00B3BB0C;

static const UInt32 s_TESObjectREFR_Set3D =				0x004E0F80;	// void : (const char*)
static const UInt32 s_Character_Set3D =					0x0060E430;

static const UInt32	s_Actor_EquipItem =					0x005FAEA0;
static const UInt32	s_Actor_GetBaseActorValue =			0x005F1910;

static const UInt32	s_PlayerCharacter_SetActiveSpell =	0x00664700;
static const UInt32 s_PlayerCharacter_GenerateNiNode =	0x00659F30;	// NiNode* : (void)

typedef void (* _UpdatePlayerHead)(void);
static const _UpdatePlayerHead UpdatePlayerHead = (_UpdatePlayerHead)0x005C2F20;

static const _Sky_GetSingleton	Sky_GetSingleton =		(_Sky_GetSingleton)0x00542EA0;
static const UInt32				s_Sky_RefreshClimate = 0x00543200;

#else

#error unsupported version of oblivion

#endif

void Actor::EquipItem(TESForm * objType, UInt32 unk1, BaseExtraList* itemExtraList, UInt32 unk3, bool lockEquip)
{
	ThisStdCall(s_Actor_EquipItem, this, objType, unk1, itemExtraList, unk3, lockEquip);
}

void Actor::UnequipItem(TESForm* objType, UInt32 unk1, BaseExtraList* itemExtraList, UInt32 unk3, bool lockUnequip, UInt32 unk5)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x005F2E70, this, objType, unk1, itemExtraList, unk3, lockUnequip, unk5);
#else
#error unsupported oblivion version
#endif
}

UInt32 Actor::GetBaseActorValue(UInt32 value)
{
	return ThisStdCall(s_Actor_GetBaseActorValue, this, value);
}

EquippedItemsList Actor::GetEquippedItems()
{
	EquippedItemsList itemList;

	ExtraContainerChanges	* xChanges = static_cast <ExtraContainerChanges *>(baseExtraList.GetByType(kExtraData_ContainerChanges));
	if(xChanges && xChanges->data && xChanges->data->objList)
		for(ExtraContainerChanges::Entry * entry = xChanges->data->objList; entry; entry = entry->next)
			if(entry->data && entry->data->extendData && entry->data->type)
				for(ExtraContainerChanges::EntryExtendData * extend = entry->data->extendData; extend; extend = extend->next)
					if(extend->data && (extend->data->HasType(kExtraData_Worn) || extend->data->HasType(kExtraData_WornLeft)))
						itemList.push_back(entry->data->type);

	return itemList;
}

void Actor::UnequipAllItems()
{
	ExtraContainerChanges* xChanges = static_cast <ExtraContainerChanges*> (baseExtraList.GetByType (kExtraData_ContainerChanges));
	if (xChanges && xChanges->data && xChanges->data->objList)
		for (ExtraContainerChanges::Entry* entry = xChanges->data->objList; entry; entry = entry->next)
			if (entry->data && entry->data->extendData && entry->data->type && entry->data->type->IsQuestItem())
				for (ExtraContainerChanges::EntryExtendData* extend = entry->data->extendData; extend; extend = extend->next)
					if (extend->data && extend->data->IsWorn())
						UnequipItem (entry->data->type, 1, extend->data, 0, false, 0);
}

ExtraContainerDataList	Actor::GetEquippedEntryDataList()
{
	ExtraContainerDataList itemList;

	ExtraContainerChanges	* xChanges = static_cast <ExtraContainerChanges *>(baseExtraList.GetByType(kExtraData_ContainerChanges));
	if(xChanges && xChanges->data && xChanges->data->objList)
		for(ExtraContainerChanges::Entry * entry = xChanges->data->objList; entry; entry = entry->next)
			if(entry->data && entry->data->extendData && entry->data->type)
				for(ExtraContainerChanges::EntryExtendData * extend = entry->data->extendData; extend; extend = extend->next)
					if(extend->data && (extend->data->HasType(kExtraData_Worn) || extend->data->HasType(kExtraData_WornLeft)))
						itemList.push_back(entry->data);

	return itemList;
}

bool PlayerCharacter::SetActiveSpell(MagicItem * item)
{
	return ThisStdCall(s_PlayerCharacter_SetActiveSpell, this, item) != 0;
}

void PlayerCharacter::TogglePOV(bool bFirstPerson)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	ThisStdCall(0x00655560, this, bFirstPerson);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	ThisStdCall(0x0066C040, this, bFirstPerson);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x0066C580, this, bFirstPerson);
#else
#error unsupported Oblivion version
#endif
}

void PlayerCharacter::SetBirthSign(BirthSign* birthSign)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x0066A400, this, birthSign);
#else
#error unsupported oblivion version
#endif
}

float GetGameSettingFloat(char* settingName)
{
	float fVal = 1.0;
	SettingInfo* setting = 0;
	if (GetGameSetting(settingName, &setting)) {
		fVal = setting->f;
	}
	return fVal;
}

float PlayerCharacter::ExperienceNeeded(UInt32 skill, UInt32 atLevel)
{
	if (atLevel == 0) atLevel = 1;
	float fSkillUseMajorMult = GetGameSettingFloat("fSkillUseMajorMult");
	float fSkillUseMinorMult = GetGameSettingFloat("fSkillUseMinorMult");
	float fSkillUseSpecMult = GetGameSettingFloat("fSkillUseSpecMult");
	float fSkillUseFactor = GetGameSettingFloat("fSkillUseFactor");
	float fSkillUseExp = GetGameSettingFloat("fSkillUseExp");

	TESClass* pClass = GetPlayerClass();

	TESSkill* pSkill = TESSkill::SkillForActorVal(skill);
	float fSkillUseMult = (pClass->IsMajorSkill(skill)) ? fSkillUseMajorMult : fSkillUseMinorMult;
	float fSkillSpecMult = (pClass->specialization == pSkill->specialization) ? fSkillUseSpecMult : 1.0;

	float expNeeded = pow(fSkillUseFactor * atLevel, fSkillUseExp) * fSkillUseMult * fSkillSpecMult;
	return expNeeded;
}

void PlayerCharacter::ChangeExperience(UInt32 valSkill, UInt32 whichUse, float howManyTimes)
{
	if (howManyTimes > 0) {
		ModExperience(valSkill, whichUse, howManyTimes);
	} else if (howManyTimes < 0) {
		TESSkill *skill = TESSkill::SkillForActorVal(valSkill);
		if (skill) {
			float expPerUse = (whichUse == 1) ? skill->useValue1 : skill->useValue0;
			float expChange = howManyTimes * expPerUse;
			ChangeExperience(valSkill, expChange);
		}
	}
}

void PlayerCharacter::ChangeExperience(UInt32 valSkill, float expChange)
{
	float &curExperience = skillExp[valSkill - kActorVal_Armorer];
	float curSkillLevel = GetBaseActorValue(valSkill);
	float skillLevel = curSkillLevel;

	if (expChange > 0) {
		curExperience += expChange;
		float expNeeded = ExperienceNeeded(valSkill, curSkillLevel);
		while (expNeeded < curExperience) {
			curExperience -= expNeeded;
			skillLevel++;
			expNeeded = ExperienceNeeded(valSkill, skillLevel);
		}
	}
	else {
		expChange = -expChange; // reverse the sign
		while (curExperience < expChange  && skillLevel > 1) {
			// we have to reduce the skill by at least a level
			// calculate the experience that was needed by the previous level

			// decrement the skill count
			--skillLevel;

			// calculate the amount of experience needed for this particular skill level
			// and increment the current experience by that much
			curExperience += ExperienceNeeded(valSkill, skillLevel);
		}

		// decrement the current experience
		if (expChange > curExperience && skillLevel == 1) {
			curExperience = 0;
		} else {
			curExperience -= expChange;
		}
	}
	if (skillLevel != curSkillLevel) {
		SetActorValue(valSkill, skillLevel);
		SInt32 delta = skillLevel - curSkillLevel;
		ModSkillAdvanceCount(valSkill, delta);
		if (GetPlayerClass()->IsMajorSkill(valSkill)) {
			ModMajorSkillAdvanceCount(delta);
		}
	}
}

UInt32 PlayerCharacter::ModSkillAdvanceCount(UInt32 valSkill, SInt32 mod) {
	if (IsSkill(valSkill)) {
		UInt32& adv = skillAdv[valSkill - kActorVal_Armorer];
		SInt32 adjusted = adv + mod; // is this going to work?
		if (adjusted < 0) {
			adv = 0;
		} else {
			adv = adjusted; // is this going to work?
		}
		return adv;
	}
	return 0;
}

TESClass* PlayerCharacter::GetPlayerClass() const
{
	TESNPC* pNPC = OBLIVION_CAST(this->baseForm, TESForm, TESNPC);
	return pNPC->npcClass;
}

class UsedPowerFinder
{
	SpellItem	* m_toFind;
public:
	UsedPowerFinder(SpellItem* power) : m_toFind(power) { }

	bool Accept(const Actor::PowerListData* data)
	{
		return (data->power == m_toFind);
	}
};

bool Actor::CanCastGreaterPower(SpellItem* power)
{
	Visitor<PowerListEntry, PowerListData> visitor(&greaterPowerList);
	if (visitor.Find(UsedPowerFinder(power)))
		return false;
	else
		return true;
}

void Actor::SetCanUseGreaterPower(SpellItem* power, bool bAllowUse, float timer)
{
	Visitor<PowerListEntry, PowerListData> visitor(&greaterPowerList);
	const PowerListEntry* foundEntry = visitor.Find(UsedPowerFinder(power));

	if (bAllowUse && foundEntry)
	{
		visitor.Remove(foundEntry->data);
	}
	else if (!bAllowUse && !foundEntry)
	{
		PowerListData* nuData = (PowerListData*)FormHeap_Allocate(sizeof(PowerListData));

		if (timer == -1)
		{
			TESGlobal* timeScaleGlob = (*g_dataHandler)->GetGlobalVarByName("TimeScale", strlen("TimeScale"));
			if (!timeScaleGlob)			// what?
				nuData->timer = 2800;
			else
				nuData->timer = 3600 / timeScaleGlob->data * 24;
		}
		else
			nuData->timer = timer;

		nuData->power = power;

		PowerListEntry* nuEntry = (PowerListEntry*)FormHeap_Allocate(sizeof(PowerListEntry));
		nuEntry->data = nuData;
		nuEntry->next = NULL;

		visitor.Append(nuEntry);
	}
	else if (!bAllowUse && foundEntry && timer != -1)		// change timer on power already in list
	{
		foundEntry->data->timer = timer;
	}
}

void Actor::PowerListEntry::Delete()
{
	FormHeap_Free(data);
	FormHeap_Free(this);
}

void Actor::PowerListEntry::DeleteHead(PowerListEntry* replaceWith)
{
	FormHeap_Free(data);
	if (replaceWith)
	{
		data = replaceWith->data;
		next = replaceWith->next;
		FormHeap_Free(replaceWith);
	}
	else
		memset(this, 0, sizeof(PowerListEntry));
}

Sky * Sky::GetSingleton(void)
{
	return Sky_GetSingleton();
}

void Sky::RefreshClimate(TESClimate * climate, UInt32 unk1)
{
	ThisStdCall(s_Sky_RefreshClimate, this, climate, unk1);
}

ScriptEventList* TESObjectREFR::GetEventList() const
{
	BSExtraData* xData = baseExtraList.GetByType(kExtraData_Script);
	if (xData)
	{
		ExtraScript* xScript = (ExtraScript*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraScript, 0);
		if (xScript)
			return xScript->eventList;
	}

	return 0;
}

TESForm* TESObjectREFR::GetInventoryItem(UInt32 itemIndex, bool bGetWares)
{
	//if getWares == true, looks up info in g_DataHandler->unkCDC

	ExtraContainerChanges::EntryData* data;

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	data = (ExtraContainerChanges::EntryData*)ThisStdCall(0x4CEB10, this, itemIndex, bGetWares);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	data = (ExtraContainerChanges::EntryData*)ThisStdCall(0x4D88E0, this, itemIndex, bGetWares);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	data = (ExtraContainerChanges::EntryData*)ThisStdCall(0x4D88F0, this, itemIndex, bGetWares);
#else
	#error unsupported Oblivion version
#endif

	if (data)
		return data->type;
	else
		return NULL;
}

typedef void (*_DisableRef)(TESObjectREFR *refr);
typedef void (*_EnableRef)(TESObjectREFR *refr);

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
const _DisableRef DisableRef = (_DisableRef)0x004F16A0;
const _EnableRef EnableRef = (_EnableRef)0x004F0650;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
const _DisableRef DisableRef = (_DisableRef)0x004FBC80;
const _EnableRef EnableRef = (_EnableRef)0x004FA5F0;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
const _DisableRef DisableRef = (_DisableRef)0x004FBB30;
const _EnableRef EnableRef = (_EnableRef)0x004FA540;
#else
#error unsupported version of oblivion
#endif

void TESObjectREFR::Disable()
{
	DisableRef(this);
}

void TESObjectREFR::Enable()
{
	EnableRef(this);
}

bool TESObjectREFR::RunScripts()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
#error unsupported Oblivion version
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
#error unsupported Oblivion version
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x004D7190, this) ? true : false;
#else
#error unsupported Oblivion version
#endif
}

bool TESObjectREFR::IsDeleted() const
{
	if (flags & kFlags_Deleted)
	{
		//DEBUG_PRINT("Ref %08x is flagged as deleted", refID);
		return true;
	}

	return false;
}

ExtraTeleport::Data* TESObjectREFR::GetExtraTeleportData()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return (ExtraTeleport::Data*)ThisStdCall(0x004D7630, this);
#else
#error unsupported Oblivion version
#endif
}

TESPackage* Actor::GetCurrentPackage()
{
	TESPackage* pkg = NULL;

	if (process)
	{
		pkg = process->GetCurrentPackage();
		// Game code for GetIsCurrentPackage checks flag bit 0xB on pkg; if not set it looks for an ExtraPackage in extra data list
		// ###TODO: figure out what that flag bit is
		if (pkg && (pkg->packageFlags & TESPackage::kPackageFlag_Unk11) == 0)
		{
			ExtraPackage* xPkg = (ExtraPackage*)baseExtraList.GetByType(kExtraData_Package);
			if (xPkg && xPkg->package)
				pkg = xPkg->package;
		}
	}

	return pkg;
}

bool TESObjectREFR::GetTeleportCellName(BSStringT* outName)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ExtraTeleport* xTele = (ExtraTeleport*)baseExtraList.GetByType(kExtraData_Teleport);
	if (xTele && xTele->data && xTele->data->linkedDoor) {
		return ThisStdCall(0x004DE8D0, xTele->data->linkedDoor, outName) ? true : false;
	}
#endif

	outName->Set("");
	return false;
}

void PlayerCharacter::UpdateHead(void)
{
	UpdatePlayerHead();
}

bool PlayerCharacter::SetSkeletonPath(const char* newPath)
{
	if (!isThirdPerson) {
		// ###TODO: enable in first person
		return false;
	}

//	if (!(*g_FileFinder)->FindFile(newPath, 0, 0, -1)) return false;  //This doesn't seem to work

	// store parent of current niNode
	NiNode* niParent = (NiNode*)(niNode->m_parent);

	// set niNode to NULL via BASE CLASS Set3D() method
	ThisStdCall(s_TESObjectREFR_Set3D, this, NULL);
	// modify model path
	if (newPath) {
		TESNPC* base = OBLIVION_CAST(baseForm, TESForm, TESNPC);
		base->model.SetPath(newPath);
	}
	// create new NiNode, add to parent
	*(g_bUpdatePlayerModel) = 1;
	NiNode* newNode = (NiNode*)ThisStdCall(s_PlayerCharacter_GenerateNiNode, this);
	niParent->AddObject(newNode, 1);
	*(g_bUpdatePlayerModel) = 0;
	newNode->SetName("Player");
	// get and store camera node
	// ### TODO: pretty this up
	UInt32 vtbl = *((UInt32*)newNode);
	UInt32 vfunc = *((UInt32*)(vtbl + 0x58));
	NiObject* cameraNode = (NiObject*)ThisStdCall(vfunc, newNode, "Camera01");
	*g_3rdPersonCameraNode = cameraNode;

	cameraNode = (NiObject*)ThisStdCall(vfunc, (NiNode*)this->firstPersonNiNode, "Camera01");
	*g_1stPersonCameraNode = cameraNode;
	Unk_52();
	// Update face/head/hair
	UpdateHead();
	return true;
}

bool TESObjectREFR::Update3D()
{
	if (this == *g_thePlayer) {
		TESModel* model = OBLIVION_CAST(baseForm, TESForm, TESModel);
		return (*g_thePlayer)->SetSkeletonPath(model->GetModelPath());
	}

	Set3D(NULL);
	ModelLoader::GetSingleton()->QueueReference(this, 1);
	return true;
}

TESObjectREFR* TESObjectREFR::Create(bool bTemp)
{
	TESObjectREFR* refr = (TESObjectREFR*)FormHeap_Allocate(sizeof(TESObjectREFR));
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x004D9A70, refr);
#else
#error unsupported Oblivion version
#endif
	if (bTemp)
		refr->MarkAsTemporary();

	return refr;
}

TESContainer* TESObjectREFR::GetContainer()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return (TESContainer*)ThisStdCall(0x004D6D40, this);
#else
#error unsupported Oblivion version
#endif
}

bool TESObjectREFR::IsMapMarker()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const TESForm** g_MapMarkerForm = (const TESForm**)0x00B35EA8;
#else
#error unsupported Oblivion version
#endif

	if (baseForm && *g_MapMarkerForm) {
		return baseForm->refID == (*g_MapMarkerForm)->refID;
	}
	else {
		return false;
	}
}

float TESObjectREFR::GetDistance(TESObjectREFR* other, bool bIncludeDisabled)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	float result;
	ThisStdCall(0x004D7E90, this, other, bIncludeDisabled);
	__asm { fstp	[result] }
#else
#error unsupported Oblivion version
#endif
	return result;
}

bool Actor::IsObjectEquipped(TESForm* object)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x004D8880, this, object) ? true : false;
#else
#error unsupported Oblivion version
#endif
}

float Actor::GetAVModifier(eAVModifier mod, UInt32 avCode)
{
	float result = 0;

	if (mod == kAVModifier_Invalid)
		return result;

	if (PlayerCharacter* pc = OBLIVION_CAST(this, Actor, PlayerCharacter)) {
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
		ThisStdCall(0x0065D270, pc, mod, avCode);
		__asm { fstp	[result] }
#else
#error unsupported Oblivion version
#endif
	}
	else {		// non-player actors
		switch (mod)
		{
		case kAVModifier_Max:
			if (MiddleLowProcess* midproc = OBLIVION_CAST(process, BaseProcess, MiddleLowProcess)) {
				result = midproc->maxAVModifiers.GetAV(avCode);
			}
			break;
		case kAVModifier_Offset:
			result = avModifiers.GetAV(avCode);
			break;
		case kAVModifier_Damage:
			if (LowProcess* proc = OBLIVION_CAST(process, BaseProcess, LowProcess))	{
				result = proc->avDamageModifiers.GetAV(avCode);
			}
			break;
		default:
			break;
		}
	}

	return result;
}

float Actor::GetCalculatedBaseAV(UInt32 avCode)
{
	float result = 0;

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x005EAD00, this, avCode);
	__asm { fstp [result] }
	return result;
#else
#error unsupported Oblivion version
#endif
}

bool Actor::IsAlerted()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x005E0E30, this) ? true : false;
#else
#error unsupported Oblivion version
#endif
}

void Actor::SetAlerted(bool bAlerted)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x005E0E10, this, bAlerted);
#else
#error unsupported Oblivion version
#endif
}

void Actor::EvaluatePackage()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00601B80, this);
#else
#error unsupported Oblivion version
#endif
}

MagicItem* PlayerCharacter::GetActiveMagicItem()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return (MagicItem*)ThisStdCall(0x0065D4A0, this);
#else
#error unsupported Oblivion version
#endif
}
