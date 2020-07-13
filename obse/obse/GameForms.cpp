#include "obse/GameForms.h"
#include "obse/GameData.h"
#include <map>
#include <functional>
#include <ctime>			//for time()
#include <string>			//for strcpy_s()
#include "InternalSerialization.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
static const UInt32 TESCreature_vtbl = 0x00A1FFEC;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
static const UInt32 TESCreature_vtbl = 0x00A5340C;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static const UInt32 TESCreature_vtbl = 0x00A5324C;

#else
#error unsupported Oblivion version
#endif

static const UInt32 kSEFF = Swap32('SEFF');

enum {
	kSlot_Head =		0x1 << TESBipedModelForm::kPart_Head,
	kSlot_Hair =		0x1 << TESBipedModelForm::kPart_Hair,
	kSlot_UpperBody =	0x1 << TESBipedModelForm::kPart_UpperBody,
	kSlot_LowerBody =	0x1 << TESBipedModelForm::kPart_LowerBody,
	kSlot_Hand =		0x1 << TESBipedModelForm::kPart_Hand,
	kSlot_Foot =		0x1 << TESBipedModelForm::kPart_Foot,
	kSlot_RightRing =	0x1 << TESBipedModelForm::kPart_RightRing,
	kSlot_LeftRing =	0x1 << TESBipedModelForm::kPart_LeftRing,
	kSlot_Amulet =		0x1 << TESBipedModelForm::kPart_Amulet,
	kSlot_Weapon =		0x1 << TESBipedModelForm::kPart_Weapon,
	kSlot_BackWeapon =	0x1 << TESBipedModelForm::kPart_BackWeapon,
	kSlot_SideWeapon =	0x1 << TESBipedModelForm::kPart_SideWeapon,
	kSlot_Quiver =		0x1 << TESBipedModelForm::kPart_Quiver,
	kSlot_Shield =		0x1 << TESBipedModelForm::kPart_Shield,
	kSlot_Torch =		0x1 << TESBipedModelForm::kPart_Torch,
	kSlot_Tail =		0x1 << TESBipedModelForm::kPart_Tail,
	kSlot_UpperLower =			kSlot_UpperBody | kSlot_LowerBody,
	kSlot_UpperLowerFoot =		kSlot_UpperLower | kSlot_Foot,
	kSlot_UpperLowerHandFoot =	kSlot_UpperLowerFoot | kSlot_Hand,
	kSlot_UpperLowerHand =		kSlot_UpperLower | kSlot_Hand,
	kSlot_BothRings =			kSlot_RightRing | kSlot_LeftRing,
	kSlot_UpperHand =			kSlot_UpperBody | kSlot_Hand,

	kSlot_None = 0,
};

bool TESForm::IsQuestItem() const
{
	return (flags & kFormFlags_QuestItem) != 0;
}

bool IsClonedForm(UInt32 formID) {
	return (formID & 0xff000000) == 0xff000000;
}

bool TESForm::IsCloned() const
{
	return IsClonedForm(refID);
}

UInt8 TESForm::GetModIndex()
{
	return (refID >> 24);
}

bool TESForm::SupportsSimpleModel() const
{
	switch (typeID) {
		case kFormType_LeveledCreature:
			return false;

		case kFormType_Creature:
		case kFormType_NPC:
		case kFormType_Activator:
		case kFormType_Apparatus:
		case kFormType_Armor:
		case kFormType_Book:
		case kFormType_Clothing:
		case kFormType_Container:
		case kFormType_Door:
		case kFormType_Ingredient:
		case kFormType_Light:
		case kFormType_Misc:
		case kFormType_Stat:
		case kFormType_Grass:
		case kFormType_Tree:
		case kFormType_Flora:
		case kFormType_Furniture:
		case kFormType_Weapon:
		case kFormType_Ammo:
		case kFormType_SoulGem:
		case kFormType_Key:
		case kFormType_AlchemyItem:
		case kFormType_SigilStone:
		case kFormType_LeveledItem:
		case kFormType_ANIO:
			return true;
		default:
			return false;
	}
}

TESForm * TESForm::TryGetREFRParent(void)
{
	TESForm			* result = this;

	if(result)
	{
		TESObjectREFR	* refr = (TESObjectREFR *)Oblivion_DynamicCast(this, 0, RTTI_TESForm, RTTI_TESObjectREFR, 0);

		if(refr && refr->baseForm)
			result = refr->baseForm;
	}

	return result;
}

enum {
	kBit_HeavyArmor = 0x1 << TESBipedModelForm::kFlags_HeavyArmor,
	kBit_NotPlayable = 0x1 << TESBipedModelForm::kFlags_NotPlayable,
};

UInt32 TESBipedModelForm::SlotForMask(UInt32 mask)
{
	switch (mask) {
		case kSlot_Head: return kPart_Head;
		case kSlot_Hair: return kPart_Hair;
		case kSlot_UpperBody: return kPart_UpperBody;
		case kSlot_LowerBody: return kPart_LowerBody;
		case kSlot_Hand: return kPart_Hand;
		case kSlot_Foot: return kPart_Foot;
		case kSlot_RightRing: return kPart_RightRing;
		case kSlot_LeftRing: return kPart_LeftRing;
		case kSlot_Amulet: return kPart_Amulet;
		case kSlot_Weapon: return kPart_Weapon;
		case kSlot_BackWeapon: return kPart_BackWeapon;
		case kSlot_SideWeapon: return kPart_SideWeapon;
		case kSlot_Quiver: return kPart_Quiver;
		case kSlot_Shield: return kPart_Shield;
		case kSlot_Torch: return kPart_Torch;
		case kSlot_Tail: return kPart_Tail;
		// combinations
		case kSlot_UpperLower: return 18;
		case kSlot_UpperLowerFoot: return 19;
		case kSlot_UpperLowerHandFoot: return 20;
		case kSlot_UpperLowerHand: return 21;
		case kSlot_BothRings: return kPart_RightRing;
		case kSlot_UpperHand: return 22;
		case kSlot_None: return 255;
		default: return 0;
	}
}

UInt32 TESBipedModelForm::MaskForSlot(UInt32 slot)
{
	UInt32 mask = 0;
	switch(slot) {
		case kPart_Head: mask = kSlot_Head; break;
		case kPart_Hair: mask = kSlot_Hair; break;
		case kPart_UpperBody: mask = kSlot_UpperBody; break;
		case kPart_LowerBody: mask = kSlot_LowerBody; break;
		case kPart_Hand: mask = kSlot_Hand; break;
		case kPart_Foot: mask = kSlot_Foot; break;
		case kPart_RightRing: mask = kSlot_BothRings; break;
		case kPart_LeftRing: mask = kSlot_BothRings; break;
		case kPart_Amulet: mask = kSlot_Amulet; break;
		case kPart_Weapon: mask = kSlot_Weapon; break;
		case kPart_BackWeapon: mask = kSlot_BackWeapon; break;
		case kPart_SideWeapon: mask = kSlot_SideWeapon; break;
		case kPart_Quiver: mask = kSlot_Quiver; break;
		case kPart_Shield: mask = kSlot_Shield; break;
		case kPart_Torch: mask = kSlot_Torch; break;
		case kPart_Tail: mask = kSlot_Tail; break;
		case 18: mask = kSlot_UpperLower; break;
		case 19: mask = kSlot_UpperLowerFoot; break;
		case 20: mask = kSlot_UpperLowerHandFoot; break;
		case 21: mask = kSlot_UpperLowerHand; break;
		case 22: mask = kSlot_UpperHand; break;
		case 255: mask = kSlot_None; break;
		default: break;
	}
	return mask;
}

UInt32 TESBipedModelForm::GetSlot() const {
	return SlotForMask(partMask);
}

void TESBipedModelForm::SetSlot(UInt32 slot)
{
	partMask = MaskForSlot(slot);
}

bool TESBipedModelForm::IsPlayable() const
{
	return (flags & kBit_NotPlayable) == 0;
}

void TESBipedModelForm::SetPlayable(bool bPlayable)
{
	// flag is not playable, so if bPlayable clear flag
	if (bPlayable) {
		flags &= ~kBit_NotPlayable;
	} else {
		flags |= kBit_NotPlayable;
	}
}

void  TESBipedModelForm::SetPath(const char* newPath, UInt32 whichPath, bool bFemalePath)
{
	BSStringT* toSet = NULL;

	switch (whichPath)
	{
	case kPath_Biped:
		toSet = &bipedModel[bFemalePath ? 1 : 0].nifPath;
		break;
	case kPath_Ground:
		toSet = &groundModel[bFemalePath ? 1 : 0].nifPath;
		break;
	case kPath_Icon:
		toSet = &icon[bFemalePath ? 1 : 0].ddsPath;
		break;
	}

	if (toSet)
		toSet->Set(newPath);
}

const char* TESBipedModelForm::GetPath(UInt32 whichPath, bool bFemalePath)
{
	BSStringT* pathStr = NULL;

	switch (whichPath)
	{
	case kPath_Biped:
		pathStr = &bipedModel[bFemalePath ? 1 : 0].nifPath;
		break;
	case kPath_Ground:
		pathStr = &groundModel[bFemalePath ? 1 : 0].nifPath;
		break;
	case kPath_Icon:
		pathStr = &icon[bFemalePath ? 1 : 0].ddsPath;
		break;
	}

	if (pathStr)
		return pathStr->m_data;
	else
		return "";
}

bool TESObjectARMO::IsHeavyArmor() const
{
	return (bipedModel.flags & kBit_HeavyArmor) != 0;
}

void TESObjectARMO::SetHeavyArmor(bool bHeavyArmor) {
	if (bHeavyArmor) {
		bipedModel.flags |= kBit_HeavyArmor;
	} else {
		bipedModel.flags &= ~kBit_HeavyArmor;
	}
}

MagicItem::EType MagicItem::Type() const
{
	void* pVoid = (void*)this;
	if (Oblivion_DynamicCast(pVoid, 0, RTTI_MagicItem, RTTI_SpellItem, 0)) {
		return kType_Spell;
	} else if (Oblivion_DynamicCast(pVoid, 0, RTTI_MagicItem, RTTI_EnchantmentItem, 0)) {
		return kType_Enchantment;
	} else if (Oblivion_DynamicCast(pVoid, 0, RTTI_MagicItem, RTTI_AlchemyItem, 0)) {
		return kType_Alchemy;
	} else if (Oblivion_DynamicCast(pVoid, 0, RTTI_MagicItem, RTTI_IngredientItem, 0)) {
		return kType_Ingredient;
	}
	return kType_None;
}

bool IngredientItem::_IsFlagSet(UInt32 mask) const
{
	return (ingredFlags & mask) != 0;
}

void IngredientItem::_SetFlag(UInt32 flag, bool bSet)
{
	if (bSet) {
		ingredFlags |= flag;
	} else {
		ingredFlags &= ~flag;
	}
}

bool EnchantmentItem::MatchesType(TESForm* form)
{
	switch(enchantType) {
		case kEnchant_Scroll:
			{
				if ((TESObjectBOOK*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectBOOK, 0) != NULL) {
					return true;
				}
			}
			break;
		case kEnchant_Staff:
			{
				TESObjectWEAP* weapon = (TESObjectWEAP*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0);
				if (weapon && weapon->type == TESObjectWEAP::kType_Staff) return true;
			}
			break;

		case kEnchant_Weapon:
			{
				TESObjectWEAP* weapon = (TESObjectWEAP*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectWEAP, 0);
				if (weapon && weapon->type != TESObjectWEAP::kType_Staff) return true;
				TESAmmo* ammo = (TESAmmo*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESAmmo, 0);
				if (ammo)
					return true;
			}
			break;
		case kEnchant_Apparel:
			{
				if (Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectCLOT, 0) != NULL ||
					Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectARMO, 0) != NULL) {
					return true;
				}
			}
			break;
	}
	return false;
}

bool EnchantmentItem::IsAutoCalc() const
{
	return (flags040 & kEnchant_NoAutoCalc) == 0;
}

void EnchantmentItem::SetAutoCalc(bool bAutoCalc) {
	if (bAutoCalc) {
		flags040 &= ~kEnchant_NoAutoCalc;
	} else {
		flags040 |= kEnchant_NoAutoCalc;
	}
}

bool SpellItem::IsAutoCalc() const
{
	return (spellFlags & kFlag_NoAutoCalc) == 0;
}

void SpellItem::SetAutoCalc(bool bAutoCalc) {
	if (bAutoCalc) {
		spellFlags &= ~kFlag_NoAutoCalc;
	} else {
		spellFlags |= kFlag_NoAutoCalc;
	}
}

bool SpellItem::TouchExplodesWithNoTarget() const
{
	return (spellFlags & kFlag_TouchExplodesWithNoTarget) != 0;
}

void SpellItem::SetTouchExplodes(bool bExplodesWithNoTarget)
{
	if (bExplodesWithNoTarget) {
		spellFlags |= kFlag_TouchExplodesWithNoTarget;
	} else {
		spellFlags &= ~kFlag_TouchExplodesWithNoTarget;
	}
}

UInt32 SpellItem::GetSchool() const
{
	EffectItem* highest = magicItem.list.ItemWithHighestMagickaCost();
	if (!highest) return 0;
	return (highest->IsScriptedEffect()) ? highest->ScriptEffectSchool() : highest->setting->school;
}

static float SkillFactor(TESForm* actorCasting, UInt32 school);

UInt32 SpellItem::GetMagickaCost(TESForm *form) const
{
	Actor* actor = OBLIVION_CAST(form, TESForm, Actor);
	EffectItemList* effList = OBLIVION_CAST(this, SpellItem, EffectItemList);
	return effList->GetMagickaCost(actor);	// ok to pass NULL for actor
}

static float GetSpellLevelMin(short whichLevel)
{
	float fLevel = 0.0;
	char* settingString = NULL;
	switch(whichLevel) {
		case SpellItem::kLevel_Apprentice:
			settingString = "fMagicSpellLevelApprenticeMin";
			break;
		case SpellItem::kLevel_Journeyman:
			settingString = "fMagicSpellLevelApprenticeMin";
			break;
		case SpellItem::kLevel_Expert:
			settingString = "fMagicSpellLevelApprenticeMin";
			break;
		case SpellItem::kLevel_Master:
			settingString = "fMagicSpellLevelApprenticeMin";
			break;
		case SpellItem::kLevel_Novice:
		default:
			return fLevel;
	}
	SettingInfo* setting = NULL;
	if (GetGameSetting(settingString, &setting)) {
		fLevel = setting->f;
	}
	return fLevel;
}

UInt32 SpellItem::GetMasteryLevel() const
{
	bool bIsSpellItem = (spellType == kType_Spell);
	if (IsAutoCalc() && IsClonedForm(refID)) {
		UInt32 magickaCost = GetMagickaCost();
		if (magickaCost < GetSpellLevelMin(kLevel_Apprentice)) {
			return kLevel_Novice;
		} else if (magickaCost < GetSpellLevelMin(kLevel_Journeyman)) {
			return kLevel_Apprentice;
		} else if (magickaCost < GetSpellLevelMin(kLevel_Expert)) {
			return kLevel_Journeyman;
		} else if (magickaCost < GetSpellLevelMin(kLevel_Master)) {
			return kLevel_Expert;
		} else {
			return kLevel_Master;
		}
	} else {
		return masteryLevel;
	}
}

TESSkill* TESSkill::SkillForActorVal(UInt32 valSkill)
{
	return &(*g_dataHandler)->skills[valSkill-kSkill_Armorer];
}

const char* TESSkill::GetLevelQuoteText(UInt32 level)
{
	UInt32 codes[4] = { 'MANA', 'MANJ', 'MANE', 'MANM' };
	if (level < 4) {
		return description.GetText(this, codes[level]);
	}

	return NULL;
}

void TESForm::MarkAsTemporary(void)
{
	// only tested for Script objects in 'con_bat'

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	ThisStdCall(0x004658C0, this);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	ThisStdCall(0x0046B490, this);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x0046B590, this);
#else
#error unsupported oblivion version
#endif
}

TESFullName* TESForm::GetFullName()
{
	TESForm* form = this;
	TESFullName* fullName = NULL;

	TESObjectREFR* refr = OBLIVION_CAST(form, TESForm, TESObjectREFR);
	if (refr)
	{
		// deleted references will have a NULL base form, so check for that
		if (refr->baseForm)
		{
			// is it a mapmarker?
			if (refr->baseForm->typeID == kFormType_Stat)
			{
				ExtraMapMarker* mapMarker = (ExtraMapMarker*)refr->baseExtraList.GetByType(kExtraData_MapMarker);
				if (mapMarker && mapMarker->data)
					fullName = &mapMarker->data->fullName;
			}
			else		// use base form
				form = refr->baseForm;
		}
	}
	else if (typeID == kFormType_Cell)	// some exterior cells inherit name of parent worldspace
	{
		TESObjectCELL* cell = OBLIVION_CAST(this, TESForm, TESObjectCELL);
		if (cell && cell->worldSpace)
			if (!cell->fullName.name.m_data || !cell->fullName.name.m_dataLen)
				form = cell->worldSpace;
	}

	if (!fullName)
		fullName = OBLIVION_CAST(form, TESForm, TESFullName);

	return fullName;
}

const char* TESForm::GetEditorID()
{
	TESQuest* quest = OBLIVION_CAST(this, TESForm, TESQuest);
	if (quest)
		return quest->editorName.m_data;

	TESObjectCELL* cell = OBLIVION_CAST(this, TESForm, TESObjectCELL);
	if (cell)
	{
		ExtraEditorID* xData = (ExtraEditorID*)cell->extraData.GetByType(kExtraData_EditorID);
		if (xData)
			return xData->editorID.m_data;
	}

	TESWorldSpace* worldspace = OBLIVION_CAST(this, TESForm, TESWorldSpace);
	if (worldspace)
		return worldspace->editorID.m_data;

	// nothing else handled
	return NULL;
}

bool TESForm::IsInventoryObject() const
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	typedef bool (* _IsInventoryObjectType)(UInt32 formType);
	static _IsInventoryObjectType IsInventoryObjectType = (_IsInventoryObjectType)0x00469520;
	return IsInventoryObjectType(typeID);
#else
#error unsupported Oblivion version
#endif
}

TESForm* TESSpellList::GetNthSpell(UInt32 whichSpell) const
{
	SpellListVisitor visitor(&spellList);
	return visitor.GetNthInfo(whichSpell);
}

UInt32 TESSpellList::RemoveAllSpells()
{
	UInt32 nRemoved = 0;

	Entry* curEntry = NULL;
	Entry* nextEntry = spellList.next;
	spellList.next = NULL;
	while (nextEntry) {
		// cache the current entry
		curEntry = nextEntry;
		// figure out the next entry
		nextEntry = nextEntry->next;
		// get rid of the current entry
		FormHeap_Free(curEntry);
		nRemoved++;
	}
	if (spellList.type) {
		spellList.type = NULL;
		nRemoved++;
	}
	return nRemoved;
}

void TESSpellList::Entry::Delete()
{
	FormHeap_Free(this);
}

void TESSpellList::Entry::DeleteHead(TESSpellList::Entry* replaceWith)
{
	if (replaceWith)
	{
		type = replaceWith->type;
		next = replaceWith->next;
		FormHeap_Free(replaceWith);
	}
	else
		memset(this, 0, sizeof(Entry));
}

bool EffectSetting::ForSpellmaking() const
{
	return (effectFlags & kEffect_ForSpellmaking) != 0;
}

bool EffectSetting::ForEnchanting() const
{
	return (effectFlags & kEffect_ForEnchanting) != 0;
}

bool EffectSetting::IsHostile() const
{
	return (effectFlags & kEffect_IsHostile) != 0;
}

bool EffectSetting::IsDetrimental() const
{
	return (effectFlags & kEffect_IsDetrimental) != 0;
}

bool EffectSetting::CanRecover() const
{
	return (effectFlags & kEffect_CanRecover) != 0;
}

bool EffectSetting::MagnitudeIsPercent() const
{
	return (effectFlags & kEffect_MagnitudePercent) != 0;
}

bool EffectSetting::FXPersists() const
{
	return (effectFlags & kEffect_FXPersists) != 0;
}

bool EffectSetting::OnSelfAllowed() const
{
	return (effectFlags & kEffect_OnSelfAllowed) != 0;
}

bool EffectSetting::OnTouchAllowed() const
{
	return (effectFlags & kEffect_OnTouchAllowed) != 0;
}

bool EffectSetting::OnTargetAllowed() const
{
	return (effectFlags & kEffect_OnTargetAllowed) != 0;
}

bool EffectSetting::NoDuration() const
{
	return (effectFlags & kEffect_NoDuration) != 0;
}

bool EffectSetting::NoMagnitude() const
{
	return (effectFlags & kEffect_NoMagnitude) != 0;
}

bool EffectSetting::NoArea() const
{
	return (effectFlags & kEffect_NoArea) != 0;
}

bool EffectSetting::NoIngredient() const
{
	return (effectFlags & kEffect_NoIngredient) != 0;
}

bool EffectSetting::UseWeapon() const
{
	return (effectFlags & kEffect_UseWeapon) != 0;
}

bool EffectSetting::UseArmor() const
{
	return (effectFlags & kEffect_UseArmor) != 0;
}

bool EffectSetting::UseCreature() const
{
	return (effectFlags & kEffect_UseCreature) != 0;
}

bool EffectSetting::UseSkill() const
{
	return (effectFlags & kEffect_UseSkill) != 0;
}

bool EffectSetting::UseAttribute() const
{
	return (effectFlags & kEffect_UseAttribute) != 0;
}

bool EffectSetting::UseOtherActorValue() const
{
	return (effectFlags & kEffect_UseActorValue) != 0;
}

bool EffectSetting::NoRecast() const
{
	return false;
}

bool EffectSetting::NoHitEffect() const
{
	return (effectFlags & kEffect_NoHitEffect) != 0;
}

bool EffectSetting::MatchesType(const std::string& typeStr) const
{
	if (typeStr.length() > 4)
		return false;

	char* myType = (char*)&effectCode;
	for (UInt32 i = 0; i < typeStr.length(); i++) {
		if (typeStr[i] != myType[i])
			return false;
	}

	return true;
}

bool EffectSetting::GetEffectChars(char* out) const
{
	*((UInt32*)out) = effectCode;
	out[4] = 0;
	return true;
}

bool EffectSetting::IsSummonEffect() const
{
	return MatchesType("Z");
}

bool EffectSetting::IsBoundItemEffect() const
{
	return MatchesType("BA") || MatchesType("BW");
}

// static
UInt32 EffectSetting::RefIdForC(UInt32 effectCode)
{
	EffectSetting* magicEffect = EffectSettingForC(effectCode);
	return (magicEffect) ? magicEffect->refID : 0;
}

// static
EffectSetting* EffectSetting::EffectSettingForC(UInt32 effectCode)
{
	EffectSetting* magicEffect = g_EffectSettingCollection->Lookup(effectCode);
	return magicEffect;
}

EffectItem::EffectItem() {}
EffectItem::~EffectItem() {}

bool EffectItem::HasActorValue() const
{
	return setting->UseAttribute() || setting->UseSkill();
}

bool EffectItem::IsValidActorValue(UInt32 actorValue) const
{
	if (setting->UseAttribute()) {
		return actorValue <= kActorVal_Luck;
	} else if (setting->UseSkill()) {
		return actorValue >= kActorVal_Armorer && actorValue <= kActorVal_Speechcraft;
	} else return false;
}

UInt32 EffectItem::GetActorValue() const
{
	return (HasActorValue()) ? actorValueOrOther : kActorVal_NoActorValue;
}

void EffectItem::SetActorValue(UInt32 actorValue)
{
	if (HasActorValue() && IsValidActorValue(actorValue)) {
		actorValueOrOther = actorValue;
	}
}

void EffectItem::CopyFrom(const EffectItem* copyFrom)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00413FC0, this, (void*)copyFrom);
#else
#error unsupported Oblivion version
#endif
}

EffectItem* EffectItem::Clone() const
{
	EffectItem* clone = EffectItem::Create(effectCode);
	if (clone) {
		clone->CopyFrom(this);
	}

	return clone;
}

// static
EffectItem* EffectItem::Create(UInt32 mgefCode)
{
	EffectItem* effItem = (EffectItem*)FormHeap_Allocate(sizeof(EffectItem));
	EffectSetting* effSetting = EffectSetting::EffectSettingForC(mgefCode);
	if (effSetting) {
		// call game constructor
		ThisStdCall(0x00414790, effItem, effSetting);
	}
	else {
		// pre-0019 behavior
		memset(effItem, 0, sizeof(EffectItem));
	}

	return effItem;
}

EffectItem::ScriptEffectInfo* EffectItem::ScriptEffectInfo::Clone() const
{
	ScriptEffectInfo* clone = Create();
	if (clone) {
		clone->CopyFrom(this);
	}
	return clone;
}

EffectItem::ScriptEffectInfo* EffectItem::ScriptEffectInfo::Create()
{
	UInt32 size = sizeof(ScriptEffectInfo);
	void* memory = FormHeap_Allocate(size);
	memset(memory, 0, size);
	// assume no vtable
	ScriptEffectInfo* scriptEffectInfo = (ScriptEffectInfo*)memory;
	scriptEffectInfo->SetName("Unknown");
	return scriptEffectInfo;
}

void EffectItem::ScriptEffectInfo::CopyFrom(const EffectItem::ScriptEffectInfo* copyFrom)
{
	if (copyFrom) {
		scriptRefID = copyFrom->scriptRefID;
		school = copyFrom->school;
		effectName.Set(copyFrom->effectName.m_data);
		isHostile = copyFrom->isHostile;
	}
}

void EffectItem::ScriptEffectInfo::SetName(const char*name)
{
	effectName.Set(name);
}

void EffectItem::ScriptEffectInfo::SetSchool(UInt32 nuSchool)
{
	// both the actor value enums and the effect setting enums should work
	// if we're given something in or above the actor value numbers, adjust them
	// into the effect setting range
	if (nuSchool >= kActorVal_Alteration) {
		nuSchool -= kActorVal_Alteration;
	}
	// only set if the value is one of the schools
	if (nuSchool <= EffectSetting::kEffect_Restoration) {
		school = nuSchool;
	}
}

void EffectItem::ScriptEffectInfo::SetVisualEffectCode(UInt32 code)
{
	visualEffectCode = code;
}

void EffectItem::ScriptEffectInfo::SetScriptRefID(UInt32 refID)
{
	scriptRefID = refID;
}

void EffectItem::ScriptEffectInfo::SetIsHostile(bool bIsHostile)
{
	isHostile = (bIsHostile) ? 1 : 0;
}

bool EffectItem::ScriptEffectInfo::IsHostile() const
{
	return (isHostile == 1) ? true : false;
}

bool EffectItem::operator<(EffectItem* rhs) const
{
	return effectCode < rhs->effectCode;
}

void EffectItem::GetQualifiedName(char* outBuf)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00413A90, this, outBuf);
#else
#error unsupported Oblivion version
#endif
}

typedef std::map<UInt32, EffectItem> EffectProxyMap;
typedef std::pair<UInt32, EffectItem> EffectProxyPair;
static EffectProxyMap gProxyMap;

void InitProxy(UInt32 effectCode, UInt32 magnitude, UInt32 area, UInt32 duration, UInt32 range, UInt32 actorValueOrOther, float cost)
{
	EffectSetting* magicEffect = EffectSetting::EffectSettingForC(effectCode);
	if (magicEffect) {
		EffectItem effectItem;
		effectItem.effectCode = effectCode;
		effectItem.magnitude = magnitude;
		effectItem.area = area;
		effectItem.duration = duration;
		effectItem.range = range;
		effectItem.actorValueOrOther = actorValueOrOther;
		effectItem.scriptEffectInfo = NULL;
		effectItem.setting = magicEffect;
		effectItem.cost = cost;

		if (effectCode == Swap32('SEFF')) {
			effectItem.scriptEffectInfo = EffectItem::ScriptEffectInfo::Create();
		}
		gProxyMap.insert(EffectProxyPair(effectCode, effectItem));
	}
}

void InitProxyMap()
{
//	InitProxy(Swap32('Code'),	Mag,Area,Dur,Range,	AVorOther,	Unk1);	// RestoreHealth

	InitProxy(Swap32('ABAT'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Luck, -1);	//	Absorb Attribute
	InitProxy(Swap32('ABFA'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Fatigue, -1);	//  Absorb Fatigue
	InitProxy(Swap32('ABHE'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Health, -1);	//  Absorb Health
	InitProxy(Swap32('ABSK'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Acrobatics, -1);	//  Absorb Skill
	InitProxy(Swap32('ABSP'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Magicka, -1);	//  Absorb Spell Points
	InitProxy(Swap32('BA01'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA02'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA03'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA04'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA05'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA06'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA07'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA08'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA09'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BA10'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BABO'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Boots
	InitProxy(Swap32('BACU'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Cuirass
	InitProxy(Swap32('BAGA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Gauntlets
	InitProxy(Swap32('BAGR'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Greaves
	InitProxy(Swap32('BAHE'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Helmet
	InitProxy(Swap32('BASH'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Shield
	InitProxy(Swap32('BRDN'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Encumbrance, -1);	//  Burden
	InitProxy(Swap32('BW01'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW02'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW03'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW04'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW05'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW06'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW07'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BW08'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('BWAX'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Axe
	InitProxy(Swap32('BWBO'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Bow
	InitProxy(Swap32('BWDA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Dagger
	InitProxy(Swap32('BWMA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Mace
	InitProxy(Swap32('BWSW'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Bound Sword
	InitProxy(Swap32('CALM'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Aggression, -1);	//  Calm
	InitProxy(Swap32('CHML'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Chameleon, -1);	//  Chameleon
	InitProxy(Swap32('CHRM'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Personality, -1);	//  Charm
	InitProxy(Swap32('COCR'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Command Creature
	InitProxy(Swap32('COHU'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Command Humanoid
	InitProxy(Swap32('CUDI'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Cure Disease
	InitProxy(Swap32('CUPA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Cure Paralysis
	InitProxy(Swap32('CUPO'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Cure Poison
	InitProxy(Swap32('DARK'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Darkness, -1);	//  Darkness - DO NOT USE
	InitProxy(Swap32('DEMO'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Confidence, -1);	//  Demoralize
	InitProxy(Swap32('DGAT'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Luck, -1);	//  Damage Attribute
	InitProxy(Swap32('DGFA'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Fatigue, -1);	//  Damage Fatigue
	InitProxy(Swap32('DGHE'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Health, -1);	//  Damage Health
	InitProxy(Swap32('DGSP'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Magicka, -1);	//  Damage Spell Points
	InitProxy(Swap32('DIAR'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Disintegrate Armor
	InitProxy(Swap32('DIWE'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Disintegrate Weapons
	InitProxy(Swap32('DRAT'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Luck, -1);	//  Drain Attribute
	InitProxy(Swap32('DRFA'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Fatigue, -1);	//  Drain Fatigue
	InitProxy(Swap32('DRHE'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Health, -1);	//  Drain Health
	InitProxy(Swap32('DRSK'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Acrobatics, -1);	//  Drain Skill
	InitProxy(Swap32('DRSP'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Magicka, -1);	//  Drain Spell Points
	InitProxy(Swap32('DSPL'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Dispel
	InitProxy(Swap32('DTCT'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_DetectLifeRange, -1);	//  Detect Life
	InitProxy(Swap32('DUMY'), 0, 0, 0, EffectItem::kRange_Self, 0x8, -1);	//  Mehrunes Dagon Custom Effect
	InitProxy(Swap32('FIDG'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Fire Damage
	InitProxy(Swap32('FISH'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistFire, -1);	//  Fire Shield
	InitProxy(Swap32('FOAT'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Luck, -1);	//  Fortify Attribute
	InitProxy(Swap32('FOFA'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Fatigue, -1);	//  Fortify Fatigue
	InitProxy(Swap32('FOHE'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Health, -1);	//  Fortify Health
	InitProxy(Swap32('FOMM'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_MagickaMultiplier, -1);	//  Fortify Magicka Multiplier
	InitProxy(Swap32('FOSK'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Acrobatics, -1);	//  Fortify Skill
	InitProxy(Swap32('FOSP'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Magicka, -1);	//  Fortify Spell Points
	InitProxy(Swap32('FRDG'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Frost Damage
	InitProxy(Swap32('FRNZ'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Aggression, -1);	//  Frenzy
	InitProxy(Swap32('FRSH'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistFrost, -1);	//  Frost Shield
	InitProxy(Swap32('FTHR'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Encumbrance, -1);	//  Feather
	InitProxy(Swap32('INVI'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Invisibility, -1);	//  Invisibility
	InitProxy(Swap32('LGHT'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Light
	InitProxy(Swap32('LISH'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistShock, -1);	//  Lightning Shield
	InitProxy(Swap32('LOCK'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Lock - DO NOT USE
	InitProxy(Swap32('MYHL'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Mythic Dawn Helm
	InitProxy(Swap32('MYTH'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Mythic Dawn Armor
	InitProxy(Swap32('NEYE'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_NightEyeBonus, -1);	//  Night-Eye
	InitProxy(Swap32('OPEN'),	0, 0, 0,	EffectItem::kRange_Target, 0x8, -1);	//  Open
	InitProxy(Swap32('PARA'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Paralysis, -1);	//  Paralyze
	InitProxy(Swap32('POSN'), 0, 0, 0, EffectItem::kRange_Touch, 0x8, -1);	//  Poison Info
	InitProxy(Swap32('RALY'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Confidence, -1);	//  Rally
	InitProxy(Swap32('REAN'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Reanimate
	InitProxy(Swap32('REAT'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Luck, -1);	//  Restore Attribute
	InitProxy(Swap32('REDG'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ReflectDamage, -1);	//  Reflect Damage
	InitProxy(Swap32('REFA'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Fatigue, -1);	//  Restore Fatigue
	InitProxy(Swap32('REHE'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	// RestoreHealth
	InitProxy(Swap32('RESP'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_Magicka, -1);	//  Restore Spell Points
	InitProxy(Swap32('RFLC'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_SpellReflectChance, -1);	//  Reflect
	InitProxy(Swap32('RSDI'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistDisease, -1);	//  Resist Disease
	InitProxy(Swap32('RSFI'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistFire, -1);	//  Resist Fire
	InitProxy(Swap32('RSFR'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistFrost, -1);	//  Resist Frost
	InitProxy(Swap32('RSMA'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistMagic, -1);	//  Resist Magic
	InitProxy(Swap32('RSNW'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistNormalWeapons, -1);	//  Resist Normal Weapons
	InitProxy(Swap32('RSPA'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistParalysis, -1);	//  Resist Paralysis
	InitProxy(Swap32('RSPO'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistPoison, -1);	//  Resist Poison
	InitProxy(Swap32('RSSH'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistShock, -1);	//  Resist Shock
	InitProxy(Swap32('RSWD'), 0, 0, 0, EffectItem::kRange_Self, 0x8, -1);	//  Resist Water Damage
	InitProxy(Swap32('SABS'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_SpellAbsorbChance, -1);	//  Spell Absorption
	InitProxy(Swap32('SEFF'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Script Effect
	InitProxy(Swap32('SHDG'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Shock Damage
	InitProxy(Swap32('SHLD'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_DefendBonus, -1);	//  Shield
	InitProxy(Swap32('SLNC'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_Silence, -1);	//  Silence
	InitProxy(Swap32('STMA'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_StuntedMagicka, -1);	//  Stunted Magicka
	InitProxy(Swap32('STRP'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Soul Trap
	InitProxy(Swap32('SUDG'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Sun Damage
	InitProxy(Swap32('TELE'),	0, 0, 0,	EffectItem::kRange_Target, 0x8, -1);	//  Telekinesis
	InitProxy(Swap32('TURN'),	0, 0, 0,	EffectItem::kRange_Touch, 0x8, -1);	//  Turn Undead
	InitProxy(Swap32('VAMP'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Vampirism
	InitProxy(Swap32('WABR'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_WaterBreathing, -1);	//  Water Breathing
	InitProxy(Swap32('WAWA'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_WaterWalking, -1);	//  Water Walking
	InitProxy(Swap32('WKDI'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_ResistDisease, -1);	//  Weakness to Disease
	InitProxy(Swap32('WKFI'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_ResistFire, -1);	//  Weakness to Fire
	InitProxy(Swap32('WKFR'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_ResistFrost, -1);	//  Weakness to Frost
	InitProxy(Swap32('WKMA'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_ResistMagic, -1);	//  Weakness to Magic
	InitProxy(Swap32('WKNW'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_ResistNormalWeapons, -1);	//  Weakness to Normal Weapons
	InitProxy(Swap32('WKPO'),	0, 0, 0,	EffectItem::kRange_Touch, kActorVal_ResistPoison, -1);	//  Weakness to Poison
	InitProxy(Swap32('WKSH'),	0, 0, 0,	EffectItem::kRange_Self, kActorVal_ResistShock, -1);	//  Weakness to Shock
	InitProxy(Swap32('Z001'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Rufio's Ghost (Extra Summon 01)
	InitProxy(Swap32('Z002'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Ancestor Guardian (Extra Summon 02)
	InitProxy(Swap32('Z003'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Extra Summon 03
	InitProxy(Swap32('Z004'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Extra Summon 04
	InitProxy(Swap32('Z005'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Bear (Extra Summon 05)
	InitProxy(Swap32('Z006'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z007'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z008'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z009'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z010'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z011'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z012'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z013'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z014'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z015'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z016'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z017'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z018'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z019'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('Z020'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//
	InitProxy(Swap32('ZCLA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Clannfear
	InitProxy(Swap32('ZDAE'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Daedroth
	InitProxy(Swap32('ZDRE'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Dremora
	InitProxy(Swap32('ZDRL'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Dremora Lord
	InitProxy(Swap32('ZFIA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Fire Atronach
	InitProxy(Swap32('ZFRA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Frost Atronach
	InitProxy(Swap32('ZGHO'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Ghost
	InitProxy(Swap32('ZHDZ'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Headless Zombie
	InitProxy(Swap32('ZLIC'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Lich
	InitProxy(Swap32('ZSCA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Scamp
	InitProxy(Swap32('ZSKA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Skeleton
	InitProxy(Swap32('ZSKC'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Skeleton Archer
	InitProxy(Swap32('ZSKE'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Skeleton Champion
	InitProxy(Swap32('ZSKH'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Skeleton Hero
	InitProxy(Swap32('ZSPD'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Spider Daedra
	InitProxy(Swap32('ZSTA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Storm Atronach
	InitProxy(Swap32('ZWRA'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Wraith
	InitProxy(Swap32('ZWRL'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Wraith Lord
	InitProxy(Swap32('ZXIV'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Xivilai
	InitProxy(Swap32('ZZOM'),	0, 0, 0,	EffectItem::kRange_Self, 0x8, -1);	//  Summon Zombie

	//Currently unavailable magic effects
	//InitProxy(Swap32('MODB'), 0, 0, 0, EffectItem::kRange_Self, 0x8, -1);	//
	//InitProxy(Swap32('MODL'), 0, 0, 0, EffectItem::kRange_Self, 0x8, -1);	//
	//InitProxy(Swap32('MODT'), 0, 0, 0, EffectItem::kRange_Self, 0x8, -1);	//
}

EffectItem* EffectItem::ProxyEffectItemFor(UInt32 effectCode) {
	if (gProxyMap.empty()) {
		InitProxyMap();
	}
	EffectProxyMap::iterator it = gProxyMap.find(effectCode);
	if (it != gProxyMap.end()) {
		return &it->second;
	} else {
		return NULL;
	}
}

bool EffectItem::IsScriptedEffect() const
{
	return effectCode == kSEFF;
}

UInt32 EffectItem::ScriptEffectRefId() const
{
	return (scriptEffectInfo) ? scriptEffectInfo->scriptRefID : 0;
}

UInt32 EffectItem::ScriptEffectSchool() const
{
	return (scriptEffectInfo) ? scriptEffectInfo->school : 0;
}

UInt32 EffectItem::ScriptEffectVisualEffectCode() const
{
	return (scriptEffectInfo) ? scriptEffectInfo->visualEffectCode : 0;
}

bool EffectItem::IsScriptEffectHostile() const
{
	return (scriptEffectInfo) ? scriptEffectInfo->IsHostile() : false;
}

bool EffectItem::IsHostile() const
{
	if (IsScriptedEffect()) {
		return IsScriptEffectHostile();
	} else if (setting) {
		return setting->IsHostile();
	} else return false;
}

static float AreaFactor(UInt32 area)
{
	float areaFactor = 1.0;
	SettingInfo* setting = NULL;
	if (GetGameSetting("fMagicAreaBaseCostMult", &setting)) {
		float areaMult = setting->f;
		areaFactor = areaMult * area;
		if (areaFactor < 1.0) {
			areaFactor = 1.0;
		}
	}
	return areaFactor;
}

static float DurationModifier(UInt32 duration)
{
	if (duration == 0) {
		duration = 1.0;
	}
	float durationModifier = 1.0;
	SettingInfo* setting = NULL;
	if (GetGameSetting("fMagicDurMagBaseCostMult", &setting)) {
		durationModifier = setting->f * duration;
	}
	return durationModifier;
}

static float EffectBaseCost(UInt32 effectCode)
{
	float baseCost = 1.0;
	EffectSetting* effect = EffectSetting::EffectSettingForC(effectCode);
	if (effect) {
		baseCost = effect->baseCost;
	}
	return baseCost;
}

static float DurationFactor(UInt32 duration, UInt32 magnitude, float effectBaseCost)
{
	float durationFactor = 1.0;
	float durationModifier = DurationModifier(duration);
	float magnitudeModifier = 1.0;
	SettingInfo* setting = NULL;
	if (GetGameSetting("fMagicCostScale", &setting)) {
		if (magnitude == 0) {
			magnitude = 1;
		}
		magnitudeModifier = pow(magnitude,setting->f);
	}
	durationFactor = effectBaseCost * durationModifier * magnitudeModifier;
	return durationFactor;
}

static float RangeFactor(UInt32 range) {
	float rangeFactor = 1.0;
	if (range == EffectItem::kRange_Target) {
		SettingInfo* setting = NULL;
		if (GetGameSetting("fMagicRangeTargetCostMult", &setting)) {
			rangeFactor = setting->f;
		}
	}
	return rangeFactor;
}

static float SkillFactor(TESForm* actorCasting, UInt32 school)
{
	if (!actorCasting) return 1.0;

	float skillFactor = 1.0;
	UInt8 skillLevel = 0;
	UInt8 luck = 0;

	switch(actorCasting->typeID) {
		case kFormType_Creature:
			{
				TESCreature* creature = (TESCreature*)(EffectSetting*)Oblivion_DynamicCast(actorCasting, 0, RTTI_TESForm, RTTI_TESCreature, 0);
				if (creature) {
					skillLevel = creature->magicSkill;
					luck = creature->attributes.attr[kActorVal_Luck];
				}
				break;
			}
		case kFormType_NPC:
			{
				TESNPC* npc = (TESNPC*)(EffectSetting*)Oblivion_DynamicCast(actorCasting, 0, RTTI_TESForm, RTTI_TESNPC, 0);
				if (npc) {
					static UInt8 offset = kActorVal_Alteration - kActorVal_Armorer;
					skillLevel = npc->skillLevels[school + offset];
					luck = npc->attributes.attr[kActorVal_Luck];
				}
				break;
			}
		default:
			return 1.0;
	}

	float fMagicCasterSkillCostBase = 1.0;
	float fMagicCasterSkillCostMult = 1.0;
	float fActorLuckSkillMult = .4;	// default value in the CS

	SettingInfo* setting = NULL;
	if (GetGameSetting("fMagicCasterSkillCostBase", &setting)) {
		fMagicCasterSkillCostBase = setting->f;
	}

	if (GetGameSetting("fMagicCasterSkillCostMult", &setting)) {
		fMagicCasterSkillCostMult = setting->f;
	}

	if (GetGameSetting("fActorLuckSkillMult", &setting)) {
		fActorLuckSkillMult = setting->f;
	}

	// adjust skill level for luck
	skillLevel += (SInt8)((luck - 50) * fActorLuckSkillMult);
	// UPDATED 0019: don't cap at 100, to accomodate mods like Elys Uncapper which allow skills > 100
	//if (skillLevel > 100) skillLevel = 100;

	skillFactor = fMagicCasterSkillCostBase + ((1.0 - skillLevel*.01) * fMagicCasterSkillCostMult);
	return skillFactor;
}

float EffectItem::MagickaCost(TESForm *actorCasting) const
{
	float areaFactor = AreaFactor(area);
	float durationFactor = DurationFactor(duration, magnitude, setting->baseCost);
	float rangeFactor = RangeFactor(range);
	float skillFactor = SkillFactor(actorCasting, setting->school);
	float magickaCost = areaFactor * durationFactor * rangeFactor * skillFactor;
	if (actorCasting && magickaCost < 1) magickaCost = 1;
	return magickaCost;
}

void EffectItem::SetMagnitude(UInt32 mag)
{
	magnitude = mag;
}

void EffectItem::ModMagnitude(float modBy)
{
	magnitude = SafeModUInt32(magnitude, modBy);
}

void EffectItem::SetArea(UInt32 _area)
{
	area = _area;
}

void EffectItem::ModArea(float modBy)
{
	area = SafeModUInt32(area, modBy);
}

void EffectItem::SetDuration(UInt32 dur)
{
	duration = dur;
}

void EffectItem::ModDuration(float modBy)
{
	duration = SafeModUInt32(duration, modBy);
}

void EffectItem::SetRange(UInt32 _range)
{
	if (_range <= kRange_Target) {
		range = _range;
	}
}

bool EffectItemList::RemoveItem(UInt32 whichItem)
{
	EffectItem* toRemove = this->ItemAt(whichItem);
	if (!toRemove) {
		return false;
	}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00414BC0, this, toRemove);
#else
#error unsupported Oblivion version
#endif

	return true;
}

void EffectItemList::RemoveAllItems()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00414C70, this);
#else
#error unsupported Oblivion version
#endif
}

float EffectItemList::CalcGoldValue()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	float result = 0;
	ThisStdCall(0x004151C0, this, NULL);
	__asm { fstp [result] }
	return result;
#else
#error unsupported Oblivion version
#endif
}

UInt32 EffectItemList::GetSchoolSkillAV() {
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x00415360, this);
#else
#error unsupported oblivion version
#endif
}

UInt32 EffectItemList::GetSchoolCode() {
	UInt32 skillAV = GetSchoolSkillAV() - kActorVal_Alteration;
	return (skillAV < 6) ? skillAV : -1;
}

EffectItemList* GetEffectList(TESForm* form)
{
	return OBLIVION_CAST(form, TESForm, EffectItemList);
}

static const bool kFindHostile = true;
static const bool kFindNonHostile = false;
class HostileItemCounter {
	UInt32 m_count;
	bool m_bFindHostile;
	bool m_bStopAtFirst;
public:
	HostileItemCounter(bool bFindHostile, bool bStopAtFirst) :
		m_count(0), m_bFindHostile(bFindHostile), m_bStopAtFirst(bStopAtFirst) { }

	bool Accept(EffectItem* pEffectItem) {
		if (pEffectItem && pEffectItem->IsHostile() == m_bFindHostile) {
			m_count++;
			if (m_bStopAtFirst) return false;
		}
		return true;
	}

	UInt32 Count() const { return m_count; }
};

bool EffectItemList::HasNonHostileItem() const
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	// i.e. return !HasOnlyHostileEffects()
	return ThisStdCall(0x00414EB0, (void*)this) ? false : true;
#else
#error unsupported Oblivion version
#endif
}

bool EffectItemList::HasHostileItem() const
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x004149F0, (void*)this) ? true : false;
#else
#error unsupported Oblivion version
#endif
}

UInt32 EffectItemList::CountItems() const
{
	EffectItemVisitor visitor(&effectList);
	return visitor.Count();
}

UInt32 EffectItemList::CountHostileItems() const
{
	EffectItemVisitor visitor(&effectList);
	HostileItemCounter counter(kFindHostile, false);
	visitor.Visit(counter);
	return counter.Count();
}

EffectItem* EffectItemList::ItemAt(UInt32 whichItem)
{
	EffectItemVisitor visitor(&effectList);
	return visitor.GetNthInfo(whichItem);
}

class HighMagickaCostFinder {
	EffectItem* m_pEffectItem;
	UInt32 m_highCost;
public:
	HighMagickaCostFinder() : m_pEffectItem(NULL), m_highCost(0) {}
	bool Accept(EffectItem* pEffectItem) {
		if (pEffectItem) {
			UInt32 cost = pEffectItem->MagickaCost();
			if (cost > m_highCost) {
				m_highCost = cost;
				m_pEffectItem = pEffectItem;
			}
		}
		return true;
	}
	EffectItem* HighestItem() const { return m_pEffectItem; }
};

EffectItem* EffectItemList::ItemWithHighestMagickaCost() const
{
	const Entry* entry = &effectList;

	// quick exit for the common case with only one effect item
	if (entry && entry->effectItem && !entry->next) {
		return entry->effectItem;
	}

	EffectItemVisitor visitor(entry);
	HighMagickaCostFinder finder;
	visitor.Visit(finder);
	return finder.HighestItem();
}

class MagickaCostCounter
{
	TESForm* m_pForm;
	UInt32 m_cost;
public:
	MagickaCostCounter(TESForm* pForm) : m_pForm(pForm), m_cost(0) {}
	bool Accept(EffectItem* pEffectItem) {
		if (pEffectItem) {
			m_cost += pEffectItem->MagickaCost(m_pForm);
		}
		return true;
	}

	UInt32 Cost() const { return m_cost; }
};

UInt32 EffectItemList::GetMagickaCost(TESForm* form) const
{
	EffectItemVisitor visitor(&effectList);
	MagickaCostCounter counter(form);
	visitor.Visit(counter);
	return counter.Cost();
}

UInt32 EffectItemList::AddItem(EffectItem* effectItem)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00414B90, this, effectItem);
#else
#error unsupported Oblivion version
#endif

	return CountItems() - 1;	// the index of the new node
}

UInt32 EffectItemList::AddItemCopy(EffectItem* effectItem)
{
	// make a copy of the incoming item
	EffectItem* nuEffectItem = effectItem->Clone();
	return AddItem(nuEffectItem);
}

UInt32 EffectItemList::CopyItemFrom(EffectItemList& fromList, UInt32 whichEffect)
{
	UInt32 nuIndex = -1;
	EffectItem* effectItem = fromList.ItemAt(whichEffect);
	if (effectItem) {
		nuIndex = AddItemCopy(effectItem);
	}
	return nuIndex;
}

const char* EffectItemList::GetNthEIName(UInt32 whichEffect) const
{
	EffectItemVisitor visitor(&effectList);
	EffectItem* effItem = visitor.GetNthInfo(whichEffect);
	if (effItem->scriptEffectInfo)
		return effItem->scriptEffectInfo->effectName.m_data;
	else if (effItem->setting)
		return GetFullName(effItem->setting);
	else
		return "<no name>";
}

bool AlchemyItem::IsPoison() const
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return (ThisStdCall(0x00414EB0, (void*)(&this->magicItem.list)) & 0x000000FF) ? true : false;
#else
#error unsupported Oblivion version
#endif
}

float AlchemyItem::GetGoldValue()
{
	if (!IsAutoCalc()) {
		return goldValue;
	}
	else {
		return magicItem.list.CalcGoldValue();
	}
}

void AlchemyItem::SetGoldValue(UInt32 newValue)
{
	SetAutoCalc(false);
	goldValue = newValue;
}

bool AlchemyItem::_IsFlagSet(UInt32 flag) const
{
	return (moreFlags & flag) != 0;
}

void AlchemyItem::_SetFlag(UInt32 flag, bool bSet)
{
	if (bSet) {
		moreFlags |= flag;
	} else {
		moreFlags &= ~flag;
	}
}

bool TESObjectBOOK::CantBeTaken() const
{
	return (bookFlags & kBook_CantBeTaken) != 0;
}

void TESObjectBOOK::SetCantBeTaken(bool bCantBeTaken)
{
	if (bCantBeTaken) {
		bookFlags |= kBook_CantBeTaken;
	} else {
		bookFlags &= ~kBook_CantBeTaken;
	}
}

bool TESObjectBOOK::IsScroll() const
{
	return (bookFlags & kBook_IsScroll) != 0;
}

void TESObjectBOOK::SetIsScroll(bool bIsScroll)
{
	if (bIsScroll) {
		bookFlags |= kBook_IsScroll;
	} else {
		bookFlags &= ~kBook_IsScroll;
	}
}

float TESObjectBOOK::Teaches() const
{
	if (teachesSkill == 255) return -1;
	else return (teachesSkill + kActorVal_Armorer);
}

void TESObjectBOOK::SetTeaches(UInt32 skill)
{
	if (skill >= kActorVal_Armorer && skill <= kActorVal_Speechcraft) {
		teachesSkill = (skill - kActorVal_Armorer);
	} else {
		teachesSkill = 255;
	}
}

/*
TESObjectWEAP* CloneWeapon(TESObjectWEAP* weaponToCopy)
{
	if(weaponToCopy)
	{
		TESObjectWEAP	* weap = (TESObjectWEAP *)CreateFormInstance(kFormType_Weapon);
		if(weap)
		{
			Console_Print("created weapon %08X", weap->refID);
			weap->CopyFrom(weaponToCopy);
			AddFormToDataHandler(*g_dataHandler, weap);
			AddFormToCreatedBaseObjectsList(*g_createdBaseObjList, weap);
			return weap;
		}
	}
	return NULL;
}
*/

TESForm* CloneForm(TESForm* formToClone)
{
	TESForm* clone = NULL;
	if (formToClone) {
		clone = (TESForm*)CreateFormInstance(formToClone->typeID);
		if (clone) {
			clone->CopyFrom(formToClone);
			AddFormToDataHandler(*g_dataHandler, clone);
			AddFormToCreatedBaseObjectsList(*g_createdBaseObjList, clone);
		}
	}
	return clone;
}

UInt32 TESRace::GetBaseAttribute(UInt32 attribute, bool bForFemale) const
{
	if (attribute > kActorVal_Luck) return 0;
	return (bForFemale) ? femaleAttr.attr[attribute] : maleAttr.attr[attribute];
}

UInt32 TESRace::GetSkillBonus(UInt32 skill) const
{
	for (UInt32 ix = 0; ix < 7; ++ix) {
		const BonusSkillInfo& skillInfo = bonusSkills[ix];
		if (skillInfo.skill == skill) {
			return skillInfo.bonus;
		}
	}
	return 0;
}

bool TESRace::IsBonusSkill(UInt32 skill) const
{
	for (UInt32 ix = 0; ix < 7; ++ix) {
		const BonusSkillInfo& skillInfo = bonusSkills[ix];
		if (skillInfo.skill == skill) return true;
	}
	return false;
}

UInt32 TESRace::GetNthBonusSkill(UInt32 n) const
{
	if (n > 6) return 0;
	const BonusSkillInfo& skillInfo = bonusSkills[n];
	return skillInfo.skill;
}

bool TESClimate::HasMasser() const
{
	return (moonInfo & kClimate_Masser) == kClimate_Masser;
};

bool TESClimate::HasSecunda() const
{
	return (moonInfo & kClimate_Secunda) == kClimate_Secunda;
}

UInt8 TESClimate::GetPhaseLength() const
{
	return moonInfo & kClimate_PhaseLengthMask;
}

void TESClimate::SetPhaseLength(UInt8 nuVal)
{
	moonInfo = (moonInfo & ~kClimate_PhaseLengthMask) | (nuVal & kClimate_PhaseLengthMask);
}

void TESClimate::SetHasMasser(bool bHasMasser)
{
	if (bHasMasser) {
		moonInfo |= kClimate_Masser;
	} else {
		moonInfo &= ~kClimate_Masser;
	}
}

void TESClimate::SetHasSecunda(bool bHasSecunda)
{
	if (bHasSecunda) {
		moonInfo |= kClimate_Secunda;
	} else {
		moonInfo &= ~kClimate_Secunda;
	}
}

void TESClimate::SetSunriseBegin(UInt8 nuVal)
{
	if (nuVal < sunriseEnd) {
		sunriseBegin = nuVal;
	}
}

void TESClimate::SetSunriseEnd(UInt8 nuVal)
{
	if (nuVal > sunriseBegin && nuVal < sunsetBegin) {
		sunriseEnd = nuVal;
	}
}

void TESClimate::SetSunsetBegin(UInt8 nuVal)
{
	if (nuVal > sunriseEnd && nuVal < sunsetEnd) {
		sunsetBegin = nuVal;
	}
}

void TESClimate::SetSunsetEnd(UInt8 nuVal)
{
	if (nuVal > sunsetBegin) {
		sunsetEnd = nuVal;
	}
}

bool TESObjectCELL::HasWater() const
{
	return IsInterior() ? ((flags0 & kFlags0_HasWater) != 0) : true;
}

bool TESObjectCELL::IsInterior() const
{
	return worldSpace == NULL;
}

float TESObjectCELL::GetWaterHeight() const
{
	float waterHeight = 0;
	if (HasWater()) {
		BSExtraData* xData = extraData.GetByType(kExtraData_WaterHeight);
		if (xData) {
			ExtraWaterHeight* xHeight = (ExtraWaterHeight*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraWaterHeight, 0);
			if (xHeight) {
				waterHeight = xHeight->waterHeight;
			}
		}
	}
	return waterHeight;
}

TESWaterForm* TESObjectCELL::GetWaterType() const
{
	TESWaterForm* water = NULL;
	if (HasWater()) {
		ExtraCellWaterType* xWater = (ExtraCellWaterType*)extraData.GetByType(kExtraData_CellWaterType);
		if (xWater) {
			water = xWater->waterType;
		}
	}

	return water;
}

bool TESObjectCELL::SetWaterHeight(float newHeight)
{
	if (IsInterior()) {	// only usable on interior cells
		if (!HasWater()) {
			// flag it as having water
			SetHasWater(true);
		}

		// does it already have an ExtraCellWaterHeight?
		ExtraWaterHeight* xHeight = (ExtraWaterHeight*)extraData.GetByType(kExtraData_WaterHeight);
		if (!xHeight) {
			if (!FloatEqual(newHeight, 0.0)) {
				// create new ExtraCellWaterHeight and add to base extra list, only if height != 0
				xHeight = ExtraWaterHeight::Create(newHeight);
				extraData.Add(xHeight);
			}

			return true;
		}
		else {
			if (FloatEqual(newHeight, 0.0)) {
				// remove, as 0 is the default height
				extraData.Remove(xHeight);
				FormHeap_Free(xHeight);
				xHeight = NULL;
			}
			else {
				xHeight->waterHeight = newHeight;
			}

			return true;
		}
	}
	else {
		// exterior cell, can't change height
		return false;
	}
}

bool TESObjectCELL::SetWaterType(TESWaterForm* type)
{
	if (IsInterior()) {
		if (!HasWater()) {
			SetHasWater(true);
		}

		ExtraCellWaterType* xWater = (ExtraCellWaterType*)extraData.GetByType(kExtraData_CellWaterType);
		if (!xWater) {
			if (type) {
				xWater = ExtraCellWaterType::Create(type);
				extraData.Add(xWater);
			}

			// if type is NULL, do nothing
			return true;
		}

		// ExtraCellWaterType already exists
		if (type) {
			xWater->waterType = type;
		}
		else {
			extraData.Remove(xWater);
		}

		return true;
	}
	else {
		return false;
	}
}

void TESObjectCELL::SetInteriorClimate(TESClimate* climate)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00424380, &extraData, climate);
#else
#error unsupported Oblivion version
#endif
}

void TESObjectCELL::SetIsPublic(bool bSet)
{
	if (IsInterior()) {
		if (bSet) {
			flags0 |= kFlags0_Public;
		}
		else {
			flags0 &= ~kFlags0_Public;
		}
	}
}

void TESObjectCELL::SetBehavesLikeExterior(bool bSet)
{
	if (bSet) {
		flags0 |= kFlags0_BehaveLikeExterior;
	}
	else {
		flags0 &= ~kFlags0_BehaveLikeExterior;
	}
}

void TESObjectCELL::SetHasWater(bool bHas)
{
	if (bHas) {
		flags0 |= kFlags0_HasWater;
	}
	else if (HasWater()) {
		flags0 &= ~kFlags0_HasWater;
		extraData.RemoveByType(kExtraData_CellWaterType);
		extraData.RemoveByType(kExtraData_WaterHeight);
	}
}

TESLeveledList::ListData*	TESLeveledList::CreateData(TESForm* form, UInt16 level, UInt16 count)
{
	ListData*	newData = (ListData*)FormHeap_Allocate(sizeof(ListData));
	newData->form = form;
	newData->level = level;
	newData->count = count;

	return newData;
}
TESLeveledList::ListEntry*	TESLeveledList::CreateEntry(TESLeveledList::ListData* data)
{
	ListEntry*	newEntry = (ListEntry*)FormHeap_Allocate(sizeof(ListEntry));
	newEntry->data = data;
	newEntry->next = NULL;

	return newEntry;
}

void TESLeveledList::AddItem(TESForm* form, UInt16 level, UInt16 count)
{
	ListEntry*	oldEntry = &list;
	ListData*	newData = CreateData(form, level, count);

	if (!list.data)		//empty list
	{
		list.data = newData;
		return;
	}

	ListEntry*	newEntry = CreateEntry(newData);

	while (oldEntry->data->level < level)
	{
		if (!oldEntry->next)		//add at end of list
		{
			oldEntry->next = newEntry;
			return;
		}
		oldEntry = oldEntry->next;
	}

	ListData*	swapData = oldEntry->data;
	ListEntry*	swapNext = oldEntry->next;
	oldEntry->data = newData;
	oldEntry->next = newEntry;
	newEntry->data = swapData;
	newEntry->next = swapNext;

	return;
}

UInt32 TESLeveledList::RemoveItem(TESForm* form)
{
	UInt32 numRemoved = 0;
	TESLeveledList::ListData*	curData;
	TESLeveledList::ListEntry*	curEntry;

	if (!(list.data))		//empty list
		return 0;

	while (list.data && list.data->form == form)	//removal from head requires shifting next element
	{
		numRemoved++;
		curData = list.data;
		curEntry = list.next;
		if (!curEntry)
		{
			FormHeap_Free(list.data);
			list.data = NULL;
			return numRemoved;
		}
		list.data = curEntry->data;
		list.next = curEntry->next;
		FormHeap_Free(curData);
		FormHeap_Free(curEntry);
	}

	curEntry = list.next;
	TESLeveledList::ListEntry*	prevEntry = &list;
	while (curEntry)
	{
		if (curEntry->data && curEntry->data->form == form)
		{
			numRemoved++;
			prevEntry->next = curEntry->next;
			TESLeveledList::ListEntry* temp = curEntry->next;
			FormHeap_Free(curEntry->data);
			FormHeap_Free(curEntry);
			curEntry = prevEntry->next;
		}
		else
		{
			prevEntry = curEntry;
			curEntry = curEntry->next;
		}
	}

	return numRemoved;
}

class LeveledListEntryDumper
{
public:
	bool Accept(TESLeveledList::ListData* data)
	{
		if (data)
			Console_Print("%32s (%0x) Level: %d Count: %d",	GetFullName(data->form),
															data->form->refID,
															data->level,
															data->count);
			_MESSAGE("%32s (%0x) Level: %d Count: %d",	GetFullName(data->form),
															data->form->refID,
															data->level,
															data->count);

			return true;
	}
};

void TESLeveledList::Dump()
{
	LeveledListVisitor visitor(&list);
	LeveledListEntryDumper dumper;
	visitor.Visit(dumper);
}

TESForm* TESLeveledList::CalcElement(UInt32 cLevel, bool useChanceNone, UInt32 levelDiff, bool noRecurse)
{
	ListEntry* curEntry = &list;
	UInt32 minLevel = 0;
	UInt32 maxLevel = 0;
	TESForm* item = NULL;

	if (useChanceNone && MersenneTwister::genrand_int32() % 100 < chanceNone)
		return NULL;

	//find max/min for level range
	if (!(flags & kFlags_CalcAllLevels) && !noRecurse)
	{
		while (curEntry && curEntry->data && curEntry->data->level <= cLevel)
		{
			maxLevel = curEntry->data->level;
			curEntry = curEntry->next;
		}
		if ( maxLevel > levelDiff)
			minLevel = maxLevel - levelDiff;
	}
	else
		maxLevel = cLevel;

	//skip entries below minLevel
	curEntry = &list;
	while (curEntry && curEntry->data && curEntry->data->level < minLevel)
		curEntry = curEntry->next;

	//pick an item with 1/numMatches probability
	UInt32 numMatches = 0;
	UInt32 curIndex = 0;
	while (curEntry && curEntry->data && curEntry->data->level <= maxLevel)
	{
		if (MersenneTwister::genrand_int32() % ++numMatches == 0)
		{
			item = curEntry->data->form;
			curIndex = numMatches - 1;
		}
		curEntry = curEntry->next;
	}

	//Recurse if a nested leveled item was chosen, unless otherwise specified
	if (!noRecurse)
	{
		TESLeveledList* nestedList = (TESLeveledList*)Oblivion_DynamicCast(item, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
		if (nestedList)
			item = nestedList->CalcElement(cLevel, useChanceNone, levelDiff);
	}

	return item;
}

class LevListFinderByLevel
{
	UInt32 m_whichLevel;

public:
	LevListFinderByLevel(UInt32 whichLevel) : m_whichLevel(whichLevel)
		{	}
	bool Accept(TESLeveledList::ListData* data)
	{
		if (data->level == m_whichLevel)
			return true;
		else
			return false;
	}
};

class LevListFinderByForm
{
	TESForm* m_formToMatch;

public:
	LevListFinderByForm(TESForm* form) : m_formToMatch(form)
		{	}
	bool Accept(const TESLeveledList::ListData* data)
	{
		if (data->form == m_formToMatch)
			return true;
		else
			return false;
	}
};

UInt32 TESLeveledList::RemoveByLevel(UInt32 whichLevel)
{
	UInt32 numRemoved = 0;
	if (list.data)
		numRemoved = LeveledListVisitor(&list).RemoveIf(LevListFinderByLevel(whichLevel));

	return numRemoved;
}

bool TESLeveledList::RemoveNthItem(UInt32 itemIndex)
{
	if (itemIndex == 0)
	{
		list.DeleteHead(list.next);
		return true;
	}
	else
	{
		TESLeveledList::ListEntry* curEntry = &list;
		TESLeveledList::ListEntry* prevEntry = NULL;
		UInt32 idx;
		for (idx = 0;
			 idx < itemIndex && curEntry;
			 idx++, prevEntry = curEntry, curEntry = curEntry->next)
		{	}	// just incrementing the counter

		if (curEntry && (idx == itemIndex))
		{
			prevEntry->SetNext(curEntry->next);
			curEntry->Delete();
			return true;
		}
	}

	return false;
}

void TESLeveledList::ListEntry::Delete() {
	FormHeap_Free(data);
	FormHeap_Free(this);
}

void TESLeveledList::ListEntry::DeleteHead(TESLeveledList::ListEntry* replaceWith) {
	FormHeap_Free(data);
	if (replaceWith)
	{
		data = replaceWith->data;
		next = replaceWith->next;
		FormHeap_Free(replaceWith);
	}
	else
		memset(this, 0, sizeof(ListEntry));
}

TESForm* TESLeveledList::GetElementByLevel(UInt32 whichLevel)
{
	TESForm* foundForm = NULL;
	LeveledListVisitor visitor(&list);
	const TESLeveledList::ListEntry* foundEntry = visitor.Find(LevListFinderByLevel(whichLevel));
	if (foundEntry && foundEntry->data)
		foundForm = foundEntry->data->form;

	return foundForm;
}

UInt32 TESLeveledList::GetItemIndexByLevel(UInt32 level)
{
	LeveledListVisitor visitor(&list);
	return visitor.GetIndexOf(LevListFinderByLevel(level));
}

UInt32 TESLeveledList::GetItemIndexByForm(TESForm* form)
{
	LeveledListVisitor visitor(&list);
	return visitor.GetIndexOf(LevListFinderByForm(form));
}

TESWeather::RGBA& TESWeather::GetColor(UInt32 whichColor, UInt8 time)
{
	static RGBA bogus;
	if (whichColor > eColor_Lightning || time > eTime_Night) return bogus;
	if (whichColor == eColor_Lightning)	{
		return lightningColor;
	} else {
		return colors[whichColor].colors[time];
	}
}

SInt8 TESActorBaseData::GetFactionRank(TESFaction* faction)
{
	FactionListEntry* entry = &factionList;
	while (entry && entry->data)
	{
		if (entry->data->faction == faction)
			return entry->data->rank;
		entry = entry->next;
	}
	return -1;
}

class ContainerFormFinder
{
	TESForm* m_formToFind;
public:
	ContainerFormFinder(TESForm* form) : m_formToFind(form) {}

	bool Accept(TESContainer::Data* pData) const
	{
		return (pData && pData->type == m_formToFind);
	}
};

TESContainer::Data* TESContainer::DataByType(TESForm *type) const
{
	ContainerVisitor visitor(&list);
	const Entry* entry = visitor.Find(ContainerFormFinder(type));
	return (entry) ? entry->data : NULL;
}

const char* TESFaction::GetNthRankName(UInt32 whichRank, bool bFemale)
{
	TESFaction::RankData* rankData = FactionRankVisitor(&ranks).GetNthInfo(whichRank);
	if (!rankData)
		return NULL;
	else
		return bFemale ? rankData->femaleRank.m_data : rankData->maleRank.m_data;
}

void TESFaction::SetNthRankName(const char* newName, UInt32 whichRank, bool bFemale)
{
	TESFaction::RankData* rankData = FactionRankVisitor(&ranks).GetNthInfo(whichRank);
	if (rankData)
	{
		if (bFemale)
			rankData->femaleRank.Set(newName);
		else
			rankData->maleRank.Set(newName);
	}
}

bool TESCreature::HasSounds()
{
	return actorBaseData.IsFlagSet(TESActorBaseData::kFlag_CreatureHasSounds);
}

bool TESCreature::SetSoundBase(TESCreature* base)
{
	// can only assign sound base if doesn't define its own sounds
	// (else it could be a base for other creatures)
	if (!HasSounds()) {
		// base can be NULL, or otherwise must actually be a sound base (defines its own sounds)
		if (!base || base->HasSounds()) {
			soundData.soundBase = base;
			return true;
		}
	}

	return false;
}

TESCreature* TESCreature::GetSoundBase()
{
	return HasSounds() ? this : soundData.soundBase;
}

TESSound* TESCreature::GetSound(UInt32 whichSound)
{
	TESSound* sound = NULL;
	if (whichSound < eCreatureSound_MAX && soundData.soundBase)
	{
		CreatureSoundEntry** sndTbl = GetSoundBase()->soundData.sounds;
		if (sndTbl[whichSound] && sndTbl[whichSound]->data)
			sound = sndTbl[whichSound]->data->sound;
	}
	return sound;
}

UInt32 TESCreature::GetSoundChance(UInt32 whichSound)
{
	UInt32 chance = -1;
	if (whichSound < eCreatureSound_MAX && soundData.soundBase)
	{
		CreatureSoundEntry** sndTbl = GetSoundBase()->soundData.sounds;
		if (sndTbl[whichSound] && sndTbl[whichSound]->data)
			chance = sndTbl[whichSound]->data->chance;
	}
	return chance;
}

const TESModelList::Entry* TESModelList::FindNifPath(char* path)
{
	return ModelListVisitor(&modelList).FindString(path);
}

bool TESModelList::RemoveEntry(char* nifToRemove)
{
	Entry* toRemove = const_cast<Entry*>(ModelListVisitor(&modelList).FindString(nifToRemove));
	if (!toRemove)
		return false;

	if (&modelList == toRemove)	//remove from head, shift elements down
	{
		FormHeap_Free(modelList.nifPath);
		if (modelList.next)
		{
			toRemove = modelList.next;
			modelList.nifPath = modelList.next->nifPath;
			modelList.next = modelList.next->next;
			FormHeap_Free(toRemove);
		}
		else
			modelList.nifPath = NULL;

		return true;
	}
	//otherwise, find within list and remove
	Entry* curEntry = &modelList;
	while (curEntry)
	{
		if (curEntry->next == toRemove)
		{
			toRemove = curEntry->next;
			curEntry->next = toRemove->next;
			FormHeap_Free(toRemove->nifPath);
			FormHeap_Free(toRemove);
			return true;
		}
		curEntry = curEntry->next;
	}
	return false;
}

bool TESModelList::AddEntry(char* nifToAdd)
{
	if (ModelListVisitor(&modelList).FindString(nifToAdd))	//already present
		return false;

	UInt32 newLen = strlen(nifToAdd) + 1;
	char* newStr = (char*)FormHeap_Allocate(newLen);
	strcpy_s(newStr, newLen, nifToAdd);
	if (!modelList.nifPath)
		modelList.nifPath = newStr;
	else
	{
		Entry* lastEntry = const_cast<Entry*>(ModelListVisitor(&modelList).GetLastNode());
		Entry* newEntry = (Entry*)FormHeap_Allocate(sizeof(Entry));
		lastEntry->next = newEntry;
		newEntry->nifPath = newStr;
		newEntry->next = NULL;
	}
	return true;
}

bool SpellItem::IsHostile()
{
	return magicItem.list.HasHostileItem();
}

void SpellItem::SetHostile(bool bHostile)
{
	magicItem.list.hostileEffectCount = bHostile ? 1 : 0;
}

void TESObjectBOOK::Constructor(void)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	ThisStdCall(0x004ACF90, this);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	ThisStdCall(0x004B5770, this);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x004B59F0, this);
#else
#error unsupported Oblivion version
#endif
}

const char* TESPackage::TargetData::StringForTargetCode(UInt8 targetCode)
{
	switch (targetCode) {
		case TESPackage::kTargetType_Refr:
			return "Reference";
		case TESPackage::kTargetType_BaseObject:
			return "Object";
		case TESPackage::kTargetType_TypeCode:
			return "ObjectType";
		default:
			return NULL;
	}
}

UInt8 TESPackage::TargetData::TargetCodeForString(const char* targetStr)
{
	if (!_stricmp(targetStr, "REFERENCE"))
		return TESPackage::kTargetType_Refr;
	else if (!_stricmp(targetStr, "OBJECT"))
		return TESPackage::kTargetType_BaseObject;
	else if (!_stricmp(targetStr, "OBJECTTYPE"))
		return TESPackage::kTargetType_TypeCode;
	else
		return 0xFF;
}

TESPackage::TargetData* TESPackage::TargetData::Create()
{
	TargetData* data = (TargetData*)FormHeap_Allocate(sizeof(TargetData));

	// fill out with same defaults as editor uses
	data->count = 0;
	data->target.objectCode = TESPackage::kObjectType_Activator;
	data->targetType = TESPackage::kTargetType_TypeCode;

	return data;
}

TESPackage::TargetData* TESPackage::GetTargetData()
{
	if (!target)
		target = TargetData::Create();

	return target;
}

void TESPackage::SetTarget(TESObjectREFR* refr)
{
	TargetData* tdata = GetTargetData();
	tdata->targetType = kTargetType_Refr;
	tdata->target.refr = refr;
	tdata->count = 1;
}

void TESPackage::SetTarget(TESForm* baseForm, UInt32 count)
{
	TargetData* tdata = GetTargetData();
	tdata->targetType = kTargetType_BaseObject;
	tdata->count = count;
	tdata->target.form = baseForm;
}

void TESPackage::SetTarget(UInt8 typeCode, UInt32 count)
{
	if (typeCode > 0 && typeCode < kObjectType_Max)
	{
		TargetData* tdata = GetTargetData();
		tdata->targetType = kTargetType_TypeCode;
		tdata->target.objectCode = typeCode;
		tdata->count= count;
	}
}

TESPackage::LocationData* TESPackage::LocationData::Create()
{
	LocationData* data = (LocationData*)FormHeap_Allocate(sizeof(LocationData));

	data->locationType = kPackLocation_CurrentLocation;
	data->object.form = NULL;
	data->radius = 0;

	return data;
}

TESPackage::LocationData* TESPackage::GetLocationData()
{
	if (!location)
		location = LocationData::Create();

	return location;
}

bool TESPackage::IsFlagSet(UInt32 flag)
{
	return (packageFlags & flag) == flag;
}

void TESPackage::SetFlag(UInt32 flag, bool bSet)
{
	if (bSet)
		packageFlags |= flag;
	else
		packageFlags &= ~flag;

	// handle either-or flags
	switch (flag)
	{
	case kPackageFlag_LockDoorsAtStart:
		if (IsFlagSet(kPackageFlag_UnlockDoorsAtStart) == bSet)
			SetFlag(kPackageFlag_UnlockDoorsAtStart, !bSet);
		break;
	case kPackageFlag_UnlockDoorsAtStart:
		if (IsFlagSet(kPackageFlag_LockDoorsAtStart) == bSet)
			SetFlag(kPackageFlag_LockDoorsAtStart, !bSet);
		break;
	case kPackageFlag_LockDoorsAtEnd:
		if (IsFlagSet(kPackageFlag_UnlockDoorsAtEnd) == bSet)
			SetFlag(kPackageFlag_UnlockDoorsAtEnd, !bSet);
		break;
	case kPackageFlag_UnlockDoorsAtEnd:
		if (IsFlagSet(kPackageFlag_LockDoorsAtEnd) == bSet)
			SetFlag(kPackageFlag_LockDoorsAtEnd, !bSet);
		break;
	case kPackageFlag_LockDoorsAtLocation:
		if (IsFlagSet(kPackageFlag_UnlockDoorsAtLocation) == bSet)
			SetFlag(kPackageFlag_UnlockDoorsAtLocation, !bSet);
		break;
	case kPackageFlag_UnlockDoorsAtLocation:
		if (IsFlagSet(kPackageFlag_LockDoorsAtLocation) == bSet)
			SetFlag(kPackageFlag_LockDoorsAtLocation, !bSet);
		break;
	}
}

static const char* TESPackage_ObjectTypeStrings[TESPackage::kObjectType_Max] =
{
	"NONE", "Activators", "Apparatus", "Armor", "Books", "Clothing", "Containers", "Doors", "Ingredients", "Lights", "Miscellaneous", "Flora", "Furniture",
	"Weapons: Any", "Ammo", "NPCs", "Creatures", "Soulgems", "Keys", "Alchemy", "Food", "All: Combat Wearable", "All: Wearable", "Weapons: NONE", "Weapons: Melee",
	"Weapons: Ranged", "Spells: Any", "Spells: Range Target", "Spells: Range Touch", "Spells: Range Self", "Spells: School Alteration",
	"Spells: School Conjuration", "Spells: School Destruction", "Spells: School Illusion", "Spells: School Mysticism", "Spells: School Restoration"
};

// add 1 to code before indexing
static const char* TESPackage_DayStrings[] = {
	"Any", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Weekdays", "Weekends", "MWF", "TT"
};

// add 1
static const char* TESPackage_MonthString[] = {
	"Any", "January", "February", "March", "April", "May", "June", "July", "August", "September",
	"October", "November", "December", "Spring", "Summer", "Autumn", "Winter"
};

static const char* TESPackage_LocationStrings[] = {
	"Reference", "Cell", "Current", "Editor", "Object", "ObjectType"
};

static const char* TESPackage_TypeStrings[] = {
	"Find", "Follow", "Escort", "Eat", "Sleep", "Wander", "Travel", "Accompany", "UseItemAt", "Ambush",
	"FleeNotCombat", "Cast", "Combat", "Unk0D"
};

const char* TESPackage::StringForPackageType(UInt32 pkgType)
{
	if (pkgType < kPackType_MAX) {
		return TESPackage_TypeStrings[pkgType];
	}
	else {
		return "";
	}
}

const char* TESPackage::StringForObjectCode(UInt8 objCode)
{
	if (objCode < kObjectType_Max)
		return TESPackage_ObjectTypeStrings[objCode];

	return "";
}

UInt8 TESPackage::ObjectCodeForString(const char* objString)
{
	for (UInt32 i = 0; i < kObjectType_Max; i++) {
		if (!_stricmp(objString, TESPackage_ObjectTypeStrings[i]))
			return i;
	}

	return kObjectType_Max;
}

const char* TESPackage::StringForProcedureCode(eProcedure proc, bool bRemovePrefix)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const char** s_procNames = (const char**)0x00B15020;
#else
#error unsupported Oblivion version
#endif
	static size_t prefixLen = strlen("PROCEDURE_");

	const char* name = NULL;
	// special-case "AQUIRE" (sic) to fix typo in game executable
	if (proc == TESPackage::kProcedure_AQUIRE) {
		name = "PROCEDURE_ACQUIRE";
	}
	else {
		name = (proc <= TESPackage::kProcedure_MAX) ? s_procNames[proc] : NULL;
	}

	if (name && bRemovePrefix) {
		name += prefixLen;
	}

	return name;
}

const char* TESPackage::Time::DayForCode(UInt8 dayCode)
{
	dayCode += 1;
	if (dayCode >= sizeof(TESPackage_DayStrings))
		return "";
	return TESPackage_DayStrings[dayCode];
}

const char* TESPackage::Time::MonthForCode(UInt8 monthCode)
{
	monthCode += 1;
	if (monthCode >= sizeof(TESPackage_MonthString))
		return "";
	return TESPackage_MonthString[monthCode];
}

UInt8 TESPackage::Time::CodeForDay(const char* dayStr)
{
	for (UInt8 i = 0; i < sizeof(TESPackage_DayStrings); i++) {
		if (!_stricmp(dayStr, TESPackage_DayStrings[i])) {
			return i-1;
		}
	}

	return kWeekday_Any;
}

UInt8 TESPackage::Time::CodeForMonth(const char* monthStr)
{
	for (UInt8 i = 0; i < sizeof(TESPackage_MonthString); i++) {
		if (!_stricmp(monthStr, TESPackage_MonthString[i])) {
			return i-1;
		}
	}

	return kMonth_Any;
}

const char* TESPackage::LocationData::StringForLocationCode(UInt8 locCode)
{
	if (locCode < kPackLocation_Max)
		return TESPackage_LocationStrings[locCode];
	return "";
}

UInt8 TESPackage::LocationData::LocationCodeForString(const char* locStr)
{
	for (UInt32 i = 0; i < kPackLocation_Max; i++)
		if (!_stricmp(locStr, TESPackage_LocationStrings[i]))
			return i;
	return kPackLocation_Max;
}

void TESAnimation::AnimationNode::Delete()
{
	FormHeap_Free(animationName);
	FormHeap_Free(this);
}

void TESAnimation::AnimationNode::DeleteHead(TESAnimation::AnimationNode* replaceWith)
{
	FormHeap_Free(animationName);
	if (replaceWith)
	{
		animationName = replaceWith->animationName;
		next = replaceWith->next;
		FormHeap_Free(replaceWith);
	}
	else
		memset(this, 0, sizeof(AnimationNode));
}

class FindReactionInfo
{
	TESForm* m_pTarget;
public:
	FindReactionInfo(TESForm* pTarget) : m_pTarget(pTarget) {}
	bool Accept(const TESReactionForm::ReactionInfo* pToMatch) const {
		if (pToMatch && pToMatch->target == m_pTarget) {
			return true;
		} else {
			return false;
		}
	}
};

SInt32 TESReactionForm::ReactionFor(TESForm* pTarget) const
{
	ReactionInfo* pInfo = targetList.Find(FindReactionInfo(pTarget));
	if (pInfo) {
		return pInfo->reaction;
	}
	return 0;
}

bool TESClass::IsMajorSkill(UInt32 skill) const
{
	for (int ix = 0; ix < 7; ++ix) {
		if (majorSkills[ix] == skill) {
			return true;
		}
	}
	return false;
}

TESObjectCELL * TESWorldSpace::LookupCell(SInt32 x, SInt32 y) const
{
	UInt32 key = x << 16;
	key |= y & 0x0000FFFF;

	return cellMap->Lookup(key);
}

bool TESPathGridPoint::SetEdgeEnabled(TESPathGridPoint* target, bool bEnable)
{
	if (target) {
		if (bEnable) {
			if (edges.IndexOf(target) == -1) {
				return edges.AddAt(target, 0) != eListInvalid;
			}
		}
		else {
			return edges.Remove(target);
		}
	}

	return false;
}

void TESPathGrid::SetLinkedPointsEnabled(TESObjectREFR* linkedRefr, bool bEnablePoints)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x004E5170, this, linkedRefr, bEnablePoints);
#else
#error unsupported Oblivion version
#endif
}

bool TESPathGrid::SetPointDisabled(UInt16 index, bool bDisabled)
{
	if (index >= nodeCount)
		return false;

	TESPathGridPoint* pt = nodes->data[index];
	pt->SetDisabled(bDisabled);
	// ###TODO: game may or may not want disabled nodes removed from the quick lookup table
	return true;
}

UInt32 TESPathGrid::GetPartitionKey(float x, float y)
{
	SInt32 dx = x;
	SInt32 dy = y;
	UInt32 key = ((dx >> 9) << 16) | ((dy >> 9) & 0x0000FFFF);
	return key;
}

TESPathGrid::PointList* TESPathGrid::GetListForPosition(float x, float y, bool bCreateIfNeeded)
{
	UInt32 key = GetPartitionKey(x, y);
	PointList* list = pointsByPartition.Lookup(key);
	if (!list && bCreateIfNeeded) {
		list = (PointList*)FormHeap_Allocate(sizeof(PointList));
		memset(list, 0, sizeof(PointList));
		// add it to the map
		NiTPointerMap<PointList>::Entry* entry = (NiTPointerMap<PointList>::Entry*)FormHeap_Allocate(sizeof(NiTPointerMap<PointList>::Entry));
		entry->data = list;
		entry->next = NULL;
		entry->key = key;
		if (!pointsByPartition.Insert(entry)) {
			// wtf?
			FormHeap_Free(entry);
			FormHeap_Free(list);
			list = NULL;
		}
	}

	return list;
}

bool TESPathGrid::SetPointPreferred(UInt16 index, bool bPreferred)
{
	// the game doesn't appear to keep the 'preferred' status around
	// it simply inserts preferred nodes into the list of nodes for its partition ahead of non-preferred nodes,
	// so they will be found and returned first on lookup
	// so this cmd just moves nodes to the front or the rear of the list

	if (index >= nodeCount)
		return false;

	TESPathGridPoint* pt = nodes->data[index];
	PointList* list = GetListForPosition(pt->x, pt->y);
	if (list) {
		// ###TODO
	}

	return false;
}

UInt16 TESPathGrid::IndexOf(TESPathGridPoint* pt)
{
	if (pt && nodes) {
		for (UInt16 i = 0; i < nodes->numObjs; i++) {
			if (nodes->data[i] == pt) {
				return i;
			}
		}
	}

	return -1;
}

UInt16 TESPathGrid::AddNode(float x, float y, float z, bool bPreferred)
{
	// ###TODO: if nodes array NULL, create?
	if (nodes && nodes->growSize > nodeCount) {
		TESPathGridPoint* pt = TESPathGridPoint::Create(x, y, z);
		// add to partition
		PointList* list = GetListForPosition(x, y, true);
		if (bPreferred) {
			list->AddAt(pt, 0);
		}
		else {
			// ###TODO: port Visitor::Append() to tList so this doesn't have to walk the list twice to count and append
			list->AddAt(pt, list->Count());
		}

		UInt16 index = nodeCount;
		if (index >= nodes->capacity) {
			nodes->SetCapacity(index + 0x10);	// somewhat arbitrary
		}

		nodes->AddAtIndex(index, &pt);
		return index;
	}

	return -1;
}

TESPathGridPoint* TESPathGrid::GetByIndex(UInt16 index)
{
	if (nodes && index < nodeCount) {
		return nodes->data[index];
	}
	else {
		return NULL;
	}
}

TESPathGrid::ExternalEdge* TESPathGrid::ExternalEdge::Create(UInt16 id, TESPathGridPoint* target)
{
	ExternalEdge* edge = NULL;
	if (target) {
		edge = (ExternalEdge*)FormHeap_Allocate(sizeof(ExternalEdge));
		edge->localNodeID = id;
		edge->x = target->x;
		edge->y = target->y;
		edge->z = target->z;
	}

	return edge;
}

class ExternalEdgeComparator {
	UInt16				m_localNodeID;
	TESPathGridPoint*	m_targetNode;
public:
	ExternalEdgeComparator(UInt16 localID, TESPathGridPoint* target) : m_localNodeID(localID), m_targetNode(target) { }
	bool Accept(const TESPathGrid::ExternalEdge* edge) {
		if (edge && edge->localNodeID == m_localNodeID) {
			if (FloatEqual(edge->x, m_targetNode->x) && FloatEqual(edge->y, m_targetNode->y) && FloatEqual(edge->z, m_targetNode->z)) {
				return true;
			}
		}

		return false;
	}
};

bool TESPathGrid::SetExternalEdge(UInt16 localID, TESPathGridPoint* target, bool bEnableEdge)
{
	TESPathGridPoint* localPt = GetByIndex(localID);
	if (localPt && target) {
		if (bEnableEdge) {
			if (externalEdges.IsNull()) {
				externalEdges.SetList(tListBase<ExternalEdge, true>::Create(FormHeap_Allocate));
			}
			else if (externalEdges->Find(ExternalEdgeComparator(localID, target))) {
				// edge already exists
				return false;
			}

			return externalEdges->AddAt(ExternalEdge::Create(localID, target), 0) != eListInvalid;
		}
		else {
			if (!externalEdges.IsNull()) {
				if (externalEdges->RemoveIf(ExternalEdgeComparator(localID, target)) != 0) {
					if (externalEdges->IsEmpty()) {
						externalEdges.Free();
					}

					return true;
				}
			}

			return false;
		}
	}

	return false;
}

bool TESPathGrid::SetEdge(UInt16 localID, UInt16 targetID, bool bEnableEdge, TESPathGrid* targetGrid)
{
	if (targetGrid != NULL && targetGrid != this) {
		TESPathGridPoint* a = GetByIndex(localID);
		TESPathGridPoint* b = targetGrid->GetByIndex(targetID);
		if (SetExternalEdge(localID, b, bEnableEdge)) {
			if (targetGrid->SetExternalEdge(targetID, a, bEnableEdge)) {
				// external edges also included in each node's own list of connected nodes
				return a->SetEdgeEnabled(b, bEnableEdge) && b->SetEdgeEnabled(a, bEnableEdge);
			}
		}

		return false;
	}

	// both nodes are local to this grid
	if (localID != targetID) {	// don't link a node to itself
		TESPathGridPoint* a = GetByIndex(localID);
		TESPathGridPoint* b = GetByIndex(targetID);
		if (a && b) {
			return a->SetEdgeEnabled(b, bEnableEdge) && b->SetEdgeEnabled(a, bEnableEdge);
		}
	}

	return false;
}

TESObjectREFR* TESPathGrid::GetLinkedRef(TESPathGridPoint* pt)
{
	NiTPointerMap<PointList>::Iterator iter(&linkedPathPoints);
	for (PointList* list = iter.Get(); !iter.Done(); iter.Next()) {
		for (PointList::Iterator listIter = list->Begin(); !listIter.End(); ++listIter) {
			if (listIter.Get() == pt) {
				return (TESObjectREFR*)iter.GetKey();
			}
		}
	}

	return NULL;
}

TESPathGridPoint* TESPathGridPoint::Create(float _x, float _y, float _z)
{
	TESPathGridPoint* pt = (TESPathGridPoint*)FormHeap_Allocate(sizeof(TESPathGridPoint));
	memset(pt, 0, sizeof(TESPathGridPoint));
	pt->x = _x;
	pt->y = _y;
	pt->z = _z;
	return pt;
}

TESQuest::StageEntry* TESQuest::GetStageEntry(UInt32 index)
{
	class StageEntryFinder {
	public:
		StageEntryFinder(UInt32 index) : m_index(index) { }
		bool Accept(const StageEntry* entry) {
			return entry->index == m_index;
		}
	private:
		UInt32 m_index;
	};

	return stageList.Find(StageEntryFinder(index));
}