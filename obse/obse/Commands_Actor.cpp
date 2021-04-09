#include "Commands_Actor.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "ArrayVar.h"
#include "ScriptUtils.h"
#include "GameMagicEffects.h"
#include "Utilities.h"
#include "Hooks_Gameplay.h"
#include "EventManager.h"

static bool Cmd_HasSpell_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj) return true;

	TESActorBase* npc = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
	if (!npc) return true;

	TESForm	* form = NULL;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &form)) return true;

	if(form)
	{
		SpellItem* spell = (SpellItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_SpellItem, 0);

		TESSpellList& spellList = npc->spellList;
		TESSpellList::Entry* curEntry = &spellList.spellList;
		while (curEntry && curEntry->type != NULL) {
			TESForm* spellForm = curEntry->type;
			if (form == spellForm) {
				*result = 1;
				return true;
			}
			curEntry = curEntry->next;
		}
	}

	return true;
}

static bool Cmd_GetSpellCount_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 spellCount = 0;

	TESForm* baseForm = (thisObj) ? thisObj->baseForm : (*g_thePlayer)->baseForm;
	TESSpellList* spellList = (TESSpellList *)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESSpellList, 0);
	if (!spellList) return true;

	TESSpellList::Entry* curEntry = &spellList->spellList;
	while (curEntry && curEntry->type != NULL) {
		++spellCount;
		curEntry = curEntry->next;
	}
	*result = spellCount;
	return true;
}

static bool Cmd_GetNthSpell_Execute(COMMAND_ARGS)
{
	UInt32	* refResult = (UInt32 *)result;
	*refResult = 0;

	TESForm* baseForm = (thisObj) ? thisObj->baseForm : (*g_thePlayer)->baseForm;
	TESSpellList* spellList = (TESSpellList *)Oblivion_DynamicCast(baseForm, 0, RTTI_TESForm, RTTI_TESSpellList, 0);
	if (!spellList) return true;

	UInt32 whichSpell = 0;
	if(!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &whichSpell)) return true;

	TESForm* spellForm = spellList->GetNthSpell(whichSpell);
	if (spellForm) {
		*refResult = spellForm->refID;
	}
	return true;
}

static bool Cmd_RemoveAllSpells_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (thisObj) {
		TESSpellList* spellList = (TESSpellList *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESSpellList, 0);
		if (spellList) {
			spellList->RemoveAllSpells();
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_GetActorValueC_Execute(COMMAND_ARGS)
{
	UInt32	type = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &type)) return true;

	if(thisObj && thisObj->IsActor())
	{
		Actor	* actor = (Actor *)thisObj;
		*result = actor->GetActorValue(type);

		if (IsConsoleMode()) {
			Console_Print("GetActorValueC >> %.2f", *result);
		}
	}

	return true;
}

static bool Cmd_GetBaseActorValueC_Execute(COMMAND_ARGS)
{
	UInt32	type = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &type)) return true;

	if(thisObj && thisObj->IsActor())
	{
		Actor	* actor = (Actor *)thisObj;
		*result = actor->GetBaseActorValue(type);
	}

	return true;
}

static bool Cmd_SetActorValueC_Execute(COMMAND_ARGS)
{
	UInt32	type = 0;
	int		amount = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &type, &amount)) return true;

	if(thisObj && thisObj->IsActor())
	{
		Actor	* actor = (Actor *)thisObj;
		actor->SetActorValue(type, amount);
	}

	return true;
}

static bool Cmd_ModActorValue2_Execute(COMMAND_ARGS)
{
	UInt32	type = 0;
	int		amount = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &type, &amount)) return true;

	if(thisObj && thisObj->IsActor())
	{
		Actor	* actor = (Actor *)thisObj;
		actor->DamageAV(type, amount, 0);
	}

	return true;
}

static bool Cmd_IsRefEssential_Execute(COMMAND_ARGS)
{
	if (!thisObj) return true;

	*result = 0;

	TESActorBaseData* actorBaseData = (TESActorBaseData*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBaseData, 0);
	if (actorBaseData) {
		*result = actorBaseData->IsEssential();
	}

	return true;
}

static bool Cmd_SetRefEssential_Execute(COMMAND_ARGS)
{
	if (!thisObj) return true;

	*result = 0;

	UInt32 setEssential;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &setEssential)) return true;

	TESActorBaseData* actorBaseData = (TESActorBaseData*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBaseData, 0);
	if (actorBaseData) {
		bool bEssential = (setEssential != 0) ? true : false;
		actorBaseData->SetEssential(bEssential);
	}

	return true;
}

static bool Cmd_GetActorLightAmount_Execute(COMMAND_ARGS)
{
	*result = 100.0f;

	if(!thisObj) return true;
	if(!thisObj->IsActor()) return true;

	Actor	* actor = (Actor *)thisObj;

	if(!actor->process) return true;

	*result = actor->process->GetLightAmount(actor, 0);

	//Console_Print("light amount = %f", (float)*result);

	return true;
}

static bool Cmd_GetMerchantContainer_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32*)result;
	if (!thisObj) return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_MerchantContainer);
	if (xData) {
		ExtraMerchantContainer* xContainer = (ExtraMerchantContainer*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraMerchantContainer, 0);
		if (xContainer) {
			*refResult = xContainer->containerRef->refID;
		}
	}
	return true;
}

static bool Cmd_SetMerchantContainer_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!thisObj) return true;
	TESObjectREFR* objectRef = NULL;
	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &objectRef))
		return true;
	if (!thisObj) return true;

	TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_TESNPC, 0);
	if (!npc) return true;

	if (objectRef->baseForm->typeID != kFormType_Container) return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_MerchantContainer);
	if (xData) {
		ExtraMerchantContainer* xContainer = (ExtraMerchantContainer*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraMerchantContainer, 0);
		if (xContainer) {
			*refResult = xContainer->containerRef->refID;
			xContainer->containerRef = objectRef;
		}
	}
	return true;
}

static bool Cmd_IsUnderWater_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (!thisObj) return true;
	TESObjectCELL* cell = thisObj->parentCell;
	if (!cell) return true;

	if (!cell->HasWater()) return true;

	float waterHeight = cell->GetWaterHeight();
	float bottom = thisObj->posZ;
	bool bIsSwimming = false;
	bool bIsSneaking = false;
	MobileObject* mobile = (MobileObject*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MobileObject, 0);
	if (mobile && mobile->process) {
		UInt32 moveFlags = mobile->process->GetMovementFlags();
		bIsSwimming = (moveFlags & BaseProcess::kMovementFlag_Swimming) != 0;
	}

	float scale = thisObj->GetScale();
	float standingHeight = scale * 128.0;	// base height = 128
	float factor = (bIsSwimming) ? .9 : .73;
	float height = standingHeight * factor;
	float top = bottom + height;
	*result = (top < waterHeight) ? 1 : 0;

	return true;
}

static bool Cmd_CopyEyes_Execute(COMMAND_ARGS)
{
	TESNPC* copyFrom = NULL;
	TESNPC* copyTo = NULL;
	*result = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &copyFrom, &copyTo))
		return true;

	if (!copyFrom)
		return true;

	if (!copyTo)
	{
		if (!thisObj)
			return true;
		copyTo = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!copyTo)
			return true;
	}

	copyTo->eyes = copyFrom->eyes;
	*result = 1;

	return true;
}

static bool Cmd_SetEyes_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	TESForm*  npcF = NULL;
	*result = 0;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &npcF);
	if (!form)
		return true;

	TESEyes* eyes = (TESEyes*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESEyes, 0);

	if (!eyes)
		return true;
	else if (!npcF)
	{
		if (!thisObj)
			return true;
		npcF = thisObj->baseForm;
	}

	TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(npcF, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (!npc)
		return true;

	npc->eyes = eyes;

	*result = 1;
	return true;
}

static bool Cmd_SetHair_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	TESForm*  npcF = NULL;
	*result = 0;

	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &npcF);
	if (!form)
		return true;

	TESHair* hair = (TESHair*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESHair, 0);
	if (!hair)
		return true;
	else if (!npcF)
	{
		if (!thisObj)
			return true;
		npcF = thisObj->baseForm;
	}

	TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(npcF, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (!npc)
		return true;
	npc->hair = hair;

	*result = 1;
	return true;
}

static bool Cmd_CopyHair_Execute(COMMAND_ARGS)
{
	TESNPC* copyFrom = NULL;
	TESNPC* copyTo = NULL;
	*result = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &copyFrom, &copyTo))
		return true;

	if (!copyFrom)
		return true;

	if (!copyTo)
	{
		if (!thisObj)
			return true;
		copyTo = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!copyTo)
			return true;
	}

	copyTo->hair = copyFrom->hair;
	copyTo->hairLength = copyFrom->hairLength;
	for (UInt32 i = 0; i < 4; i++)
		copyTo->hairColorRGB[i] = copyFrom->hairColorRGB[i];

	*result = 1;

	return true;
}

static TESActorBase* ExtractActorBase(COMMAND_ARGS)
{
	TESActorBase* actorBase = NULL;
	TESForm* actorForm = NULL;

	ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &actorForm);
	if (!actorForm)
		if (thisObj)
			actorForm = thisObj->baseForm;

	if (actorForm)
	{
		actorBase = (TESActorBase*)Oblivion_DynamicCast(actorForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
	}

	return actorBase;
}

static bool Cmd_IsActorRespawning_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.IsRespawning())
		*result = 1;

	return true;
}

static bool Cmd_IsPCLevelOffset_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.IsPCLevelOffset())
	{
		*result = 1;
	}

	return true;
}

static bool Cmd_GetActorBaseLevel_Execute(COMMAND_ARGS)
{
	// is level offset or absolute level, depending on PCLevelOffset flag
	*result = 0.0;
	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase) {
		*result = actorBase->actorBaseData.level;
	}
	return true;
}

static bool Cmd_GetActorMinLevel_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.IsPCLevelOffset())
	{
		*result = actorBase->actorBaseData.minLevel;
	}

	return true;
}

static bool Cmd_GetActorMaxLevel_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.IsPCLevelOffset())
	{
		*result = actorBase->actorBaseData.maxLevel;
	}

	return true;
}

static bool Cmd_HasLowLevelProcessing_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.HasLowLevelProcessing())
		*result = 1;

	return true;
}

static bool Cmd_IsSummonable_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.IsSummonable())
		*result = 1;

	return true;
}

static bool Cmd_HasNoPersuasion_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.HasNoPersuasion())
		*result = 1;

	return true;
}

static bool Cmd_CanCorpseCheck_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.CanCorpseCheck())
		*result = 1;

	return true;
}

static bool Cmd_IsFemale_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase && actorBase->actorBaseData.IsFemale())
		*result = 1;

	return true;
}

static TESActorBase* ExtractSetActorBase(COMMAND_ARGS, UInt32* bMod)
{
	TESActorBase* actorBase = NULL;
	TESForm* actorForm = NULL;
	*bMod = 0;

	ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, bMod, &actorForm);
	if (!actorForm)
		if (thisObj)
			actorForm = thisObj->baseForm;

	if (actorForm)
	{
		actorBase = (TESActorBase*)Oblivion_DynamicCast(actorForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
	}
	return actorBase;
}

static bool Cmd_SetFemale_Execute(COMMAND_ARGS)
{
	UInt32 bMod = 0;
	*result = 0;
	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &bMod);
	if (actorBase)
	{
		actorBase->actorBaseData.SetFemale(bMod ? true : false);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
		*result = 1;
	}

	return true;
}

static bool Cmd_SetActorRespawns_Execute(COMMAND_ARGS)
{
	UInt32 bMod = 0;
	*result = 0;
	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &bMod);
	if (actorBase)
	{
		actorBase->actorBaseData.SetRespawning(bMod ? true : false);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
		*result = 1;
	}

	return true;
}

static bool Cmd_SetLowLevelProcessing_Execute(COMMAND_ARGS)
{
	UInt32 bMod = 0;
	*result = 0;
	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &bMod);
	if (actorBase)
	{
		actorBase->actorBaseData.SetLowLevelProcessing(bMod ? true : false);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
		*result = 1;
	}

	return true;
}

static bool Cmd_SetNoPersuasion_Execute(COMMAND_ARGS)
{
	UInt32 bMod = 0;
	*result = 0;
	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &bMod);
	if (actorBase)
	{
		actorBase->actorBaseData.SetNoPersuasion(bMod ? true : false);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
		*result = 1;
	}

	return true;
}

static bool Cmd_SetSummonable_Execute(COMMAND_ARGS)
{
	UInt32 bMod = 0;
	*result = 0;
	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &bMod);
	if (actorBase)
	{
		actorBase->actorBaseData.SetSummonable(bMod ? true : false);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
		*result = 1;
	}

	return true;
}

static bool Cmd_SetCanCorpseCheck_Execute(COMMAND_ARGS)
{
	UInt32 bMod = 0;
	*result = 0;
	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &bMod);
	if (actorBase)
	{
		actorBase->actorBaseData.SetCanCorpseCheck(bMod ? true : false);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
		*result = 1;
	}

	return true;
}

static bool Cmd_SetPCLevelOffset_Execute(COMMAND_ARGS)
{
	UInt32 minLevel = -1;
	UInt32 maxLevel = -1;
	UInt32 bMod = 0;
	TESActorBase* actorBase = NULL;
	TESForm* actorForm = NULL;

	ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &bMod, &minLevel, &maxLevel, &actorForm);
	if (!actorForm)
		if (thisObj)
			actorForm = thisObj->baseForm;

	if (actorForm)
	{
		actorBase = (TESActorBase*)Oblivion_DynamicCast(actorForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
	}

	if (actorBase)
	{
		actorBase->actorBaseData.SetPCLevelOffset((bMod ? true : false), minLevel, maxLevel);
		actorBase->MarkAsModified(TESActorBaseData::kModified_ActorBaseFlags);
	}

	return true;
}

static bool Cmd_GetHorse_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		Character* actor = (Character*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_Character, 0);
		if (actor && actor->horseOrRider)
			*refResult = actor->horseOrRider->refID;
	}

	return true;
}

static bool Cmd_GetHair_Execute(COMMAND_ARGS)
{
	TESNPC* npc = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &npc))
		return true;

	if (!npc)
	{
		if (thisObj && (thisObj->baseForm->typeID == kFormType_NPC))
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		else
			return true;
	}

	if (npc && npc->hair)
		*refResult = npc->hair->refID;

	return true;
}

static bool Cmd_GetEyes_Execute(COMMAND_ARGS)
{
	TESNPC* npc = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &npc))
		return true;

	if (!npc)
	{
		if (thisObj && (thisObj->baseForm->typeID == kFormType_NPC))
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
	}

	if (npc && npc->eyes)
		*refResult = npc->eyes->refID;

	return true;
}

static bool Cmd_GetHairColor_Execute(COMMAND_ARGS)
{
	TESNPC* npc = 0;
	UInt32 whichColor = 0;
	*result = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &whichColor, &npc))
		return true;

	if (!npc)
	{
		if (thisObj && (thisObj->baseForm->typeID == kFormType_NPC))
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
	}

	if (npc && whichColor < 3)
		*result = npc->hairColorRGB[whichColor];

	return true;
}

static bool Cmd_GetRace_Execute(COMMAND_ARGS)
{
	TESNPC* npc = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &npc))
		return true;

	if (!npc)
	{
		if (thisObj && (thisObj->baseForm->typeID == kFormType_NPC))
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		else
			return true;
	}

	*refResult = npc->race.race->refID;
	return true;
}

// this command can have unpredictable results if used on an actor reference whose base form is not unique
// It copies the race from another NPC rather than accepting a race directly, because it needs to change the skeleton path
static bool Cmd_CopyRace_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	TESForm* fromArg = NULL;

	if (thisObj)
		npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (!npc)
		return true;

	if(!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &fromArg))
		return true;

	TESNPC* fromNPC = (TESNPC*)Oblivion_DynamicCast(fromArg, 0, RTTI_TESForm, RTTI_TESNPC, 0);
	if (!fromNPC)
		return true;

	npc->race.race = fromNPC->race.race;
	npc->model.SetModelPath (fromNPC->model.GetModelPath ());
	thisObj->Update3D ();

	return true;
}

static bool Cmd_GetEquippedItems_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetEquippedItems must be called within the context of an OBSE expression");
		return false;
	}

	// create temp array to hold the items
	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	// get the items
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	EquippedItemsList eqItems = actor->GetEquippedItems();

	// store items in array
	for (UInt32 i = 0; i < eqItems.size(); i++)
		g_ArrayMap.SetElementFormID(arrID, i, eqItems[i] ? eqItems[i]->refID : 0);

	return true;
}

static bool Cmd_GetActorAlpha_Execute(COMMAND_ARGS)
{
	*result = 1;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && actor->process)
	{
		MiddleHighProcess* midProc = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
		if (midProc)
			*result = midProc->actorAlpha;
	}

	return true;
}

class ActiveEffectModifierRemover
{
	UInt32		m_baseActorValue;
	UInt32		m_fixedActorValue;
public:
	ActiveEffectModifierRemover(UInt32 baseVal) : m_baseActorValue(baseVal), m_fixedActorValue(baseVal)
		{	}

	bool Accept(const ActiveEffect* info)
	{
		if (info->spellType == SpellItem::kType_Ability)
		{
			if (info->effectItem->effectCode == MACRO_SWAP32('DRHE'))
				m_fixedActorValue += info->magnitude;
			else if (info->effectItem->effectCode == MACRO_SWAP32('FOHE'))
				m_fixedActorValue -= info->magnitude;
		}

		return true;
	}

	UInt32 Val()  { return m_fixedActorValue; }
};
//DEPRECATED
static bool Cmd_GetBaseAV2_Execute(COMMAND_ARGS)
{
	// only differs from vanilla cmd for player, and only for attributes and health

	UInt32 actorVal = -1;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorVal))
	{
		*result = actor->GetBaseActorValue(actorVal);
		if (actor == *g_thePlayer)
		{
			if (actorVal <= kActorVal_Luck)
			{
				TESAttributes* attr = OBLIVION_CAST((*g_thePlayer)->baseForm, TESForm, TESAttributes);
				*result = attr->attr[actorVal];
			}
			else if (actorVal == kActorVal_Health)
			{
				ActiveEffectVisitor visitor(actor->GetMagicTarget()->GetEffectList());
				ActiveEffectModifierRemover fixer(*result);
				visitor.Visit(fixer);
				*result = fixer.Val();
			}
		}
	}

	return true;
}

static double GetBaseAV3(UInt32 actorValue, Actor* actor){
	double val = actor->GetBaseActorValue(actorValue);  //In a perfect world we should try to fix this function directly instead of workaroundin this.
	MagicTarget::EffectNode* list = actor->GetMagicTarget()->GetEffectList();
	/*
		If the EffectList is empty or not initialized MagicTarget->GetEffectList() still return  a MagicTarget::EffectNode* with a null data and next field.
	*/
	while(list){
		if (list->data == NULL) {
			DEBUG_PRINT("Null list data");
			list = list->next;
			continue;
		}
		if(list->data->spellType == SpellItem::kType_Ability){
			if(actorValue <= kActorVal_Luck){
				if(list->data->effectItem->actorValueOrOther == actorValue){
					if(list->data->effectItem->effectCode == MACRO_SWAP32('DRAT')){
						val += list->data->effectItem->magnitude;
						//Note account for Truncated Drain effects
					}
					else if(list->data->effectItem->effectCode == MACRO_SWAP32('FOAT')){
						val -= list->data->effectItem->magnitude;
					}
				}
			}
			else if(actorValue == kActorVal_Health){
				if(list->data->effectItem->effectCode == MACRO_SWAP32('DRHE')){
					val += list->data->effectItem->magnitude;
					//Note account for Truncated Drain effects
				}
				else if(list->data->effectItem->effectCode == MACRO_SWAP32('FOHE')){
					val -= list->data->effectItem->magnitude;
				}
			}
			else if(actorValue == kActorVal_Magicka){	
				if(list->data->effectItem->effectCode == MACRO_SWAP32('DRSP')){
					val += list->data->effectItem->magnitude;
					//Note account for Truncated Drain effects
				}
				else if(list->data->effectItem->effectCode == MACRO_SWAP32('FOSP')){
					val -= list->data->effectItem->magnitude;
				}
			}
			else if(actorValue == kActorVal_Energy){
				if(list->data->effectItem->effectCode == MACRO_SWAP32('DRFA')){
					val += list->data->effectItem->magnitude;
					//Note account for Truncated Drain effects  TODO?
				}
				else if(list->data->effectItem->effectCode == MACRO_SWAP32('FOFA')){
					val -= list->data->effectItem->magnitude;
				}
			}
		}
		list = list->Next();
	}
	return val;
}   

static bool Cmd_GetBaseAV3_Eval(COMMAND_ARGS_EVAL){
	*result = 0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	UInt32 actorVal = *((UInt32*)arg1);
	if(actor)  *result = GetBaseAV3(actorVal, actor);
	return true;
}

static bool Cmd_GetBaseAV3_Execute(COMMAND_ARGS){
	*result = 0;
	UInt32 actorVal = -1;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)	return true;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorVal))	*result = GetBaseAV3(actorVal, actor);
	//Truncate negative optionally
	if(*result < 0) *result = 0;
	if(IsConsoleMode()) Console_Print("GetBaseAV3 :  %f ",*result);
	return true;
}

//Return true if actor has no LowerBody armour or clothing equipped and optionally for UpperBody. 
static UInt32 IsNaked(Actor* npc, bool requiredUpperBody){
	EquippedItemsList list = npc->GetEquippedItems();
	bool foundLower = false;
	bool foundUpper = requiredUpperBody ?  false : true;
//	if(IsConsoleMode()) Console_Print("IsNaked SizeList:  %08X", list.size());
	for(int i = 0; i < list.size(); i++){
		TESForm* eq = list.at(i);
//		if(IsConsoleMode()) Console_Print("IsNaked Element:  %08X", eq->refID);
		//Not interested in Weapons
		if(!eq) continue;
		//TODO TESBIpedForm
		TESObjectARMO* armor = OBLIVION_CAST(eq, TESForm, TESObjectARMO);
		if(!armor){
			TESObjectCLOT* cloth = OBLIVION_CAST(eq, TESForm, TESObjectCLOT);
			if(!cloth) continue;
			if(foundLower == false)	foundLower = (cloth->bipedModel.partMask & 8) == 8;
			if(foundUpper == false)	foundUpper = (cloth->bipedModel.partMask & 4) == 4;
		}
		else{
			if(foundLower == false) foundLower = (armor->bipedModel.partMask & 8) == 8;
			if(foundUpper == false) foundUpper = (armor->bipedModel.partMask & 4) == 4;
		}
		if(foundLower == true  && foundUpper == true){
//			if(IsConsoleMode()) Console_Print("IsNaked Exit with true:  %08X", 0);
			return 0;
		}
	}
//	if(IsConsoleMode()) Console_Print("IsNaked Exit with true:  %08X", 1);
	return 1;
}

static bool Cmd_IsNaked_Eval(COMMAND_ARGS_EVAL){
	*result = 0;
	Actor* act = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if(!act) return true;
	bool requireUpperBody = false;
	if(arg1) requireUpperBody = (*((UInt32*)arg1)) != 0 ? true : false;
	*result = IsNaked(act, requireUpperBody);
	return true;
}

static bool Cmd_IsNaked_Execute(COMMAND_ARGS){
	*result = 0;
	Actor* act = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	UInt32 requireUpper = 0;
	if(!act) return true;
	ExtractArgs(PASS_EXTRACT_ARGS, &requireUpper);
	bool requireUpperBody = requireUpper != 0 ? true : false;
	*result = IsNaked(act, requireUpperBody); 
	if(IsConsoleMode()) Console_Print("IsNaked:  %f" , *result);
	return true;
}

static bool Cmd_ToggleSpecialAnim_Execute(COMMAND_ARGS)
{
	// [actor].ToggleSpecialAnim animPath bOnOff [actorBase]
	char animPath[512];
	TESActorBase* actorBase = NULL;
	UInt32 bToggleOn = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, animPath, &bToggleOn, &actorBase))
	{
		TESAnimation* anim = OBLIVION_CAST(actorBase, TESActorBase, TESAnimation);
		if (!anim && thisObj)
			anim = OBLIVION_CAST(thisObj->baseForm, TESForm, TESAnimation);

		if (anim)
		{
			AnimationVisitor visitor(&anim->data);
			if (bToggleOn)
			{
				if (!visitor.FindString(animPath))
				{
					UInt32 sLen = strlen(animPath);
					TESAnimation::AnimationNode* newNode = (TESAnimation::AnimationNode*)FormHeap_Allocate(sizeof(TESAnimation::AnimationNode));
					newNode->animationName = (char*)FormHeap_Allocate(sLen + 1);
					strcpy_s(newNode->animationName, sLen + 1, animPath);
					newNode->next = NULL;

					visitor.Append(newNode);
					*result = 1;
				}
			}
			else
				* result = visitor.RemoveString(animPath) ? 1 : 0;
		}
	}

	if (IsConsoleMode())
		Console_Print("ToggleSpecialAnim %d >> %.0f", bToggleOn, *result);

	return true;
}

static bool Cmd_GetSpecialAnims_Execute(COMMAND_ARGS)
{
	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	TESActorBase* actorBase = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorBase))
	{
		TESAnimation* anim = OBLIVION_CAST(actorBase, TESActorBase, TESAnimation);
		if (!anim && thisObj)
			anim = OBLIVION_CAST(thisObj->baseForm, TESForm, TESAnimation);

		if (anim)
		{
			double idx = 0;
			for (TESAnimation::AnimationNode* cur = &anim->data; cur && cur->animationName; cur = cur->next)
			{
				g_ArrayMap.SetElementString(arrID, idx, cur->animationName);
				idx += 1.0;
			}
		}
	}

	return true;
}

static bool Cmd_CanCastPower_Execute(COMMAND_ARGS)
{
	*result = 0;
	SpellItem* power = NULL;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (ExtractArgs(EXTRACT_ARGS, &power) && actor)
	{
		if (actor->CanCastGreaterPower(power))
			*result = 1;
	}

	if (IsConsoleMode())
		Console_Print("CanCastPower >> %.0f", *result);

	return true;
}

static bool Cmd_SetCanCastPower_Execute(COMMAND_ARGS)
{
	*result = 0;
	SpellItem* power = NULL;
	UInt32 bAllowUse = 1;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && ExtractArgs(EXTRACT_ARGS, &power, &bAllowUse))
		actor->SetCanUseGreaterPower(power, bAllowUse ? true : false);

	return true;
}

static bool Cmd_GetUsedPowers_Execute(COMMAND_ARGS)
{
	// returns an array of arrays:
	// Powers[n]["power"] := spellitem
	// Powers[n]["timer"] := seconds until power is usable again

	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetUsedPowers must be called within an OBSE expression.");
		return true;
	}

	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	// populate the array
	UInt32 idx = 0;
	for (Actor::PowerListEntry* entry = &actor->greaterPowerList; entry && entry->data; entry = entry->next)
	{
		ArrayID inner = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
		g_ArrayMap.SetElementFormID(inner, "power", entry->data->power ? entry->data->power->refID : 0);
		g_ArrayMap.SetElementNumber(inner, "timer", entry->data->timer);
		g_ArrayMap.SetElementArray(arr, idx, inner);
		idx++;
	}

	return true;
}

static bool Cmd_SetPowerTimer_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	SpellItem* power = NULL;
	float timer = 0;
	if (!ExtractArgs(EXTRACT_ARGS, &power, &timer) || !power)
		return true;

	actor->SetCanUseGreaterPower(power, false, timer);
	*result = 1;

	return true;
}

CombatController* GetCombatController(TESObjectREFR* thisObj)
{
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor)
		return actor->GetCombatController();

	return NULL;
}

enum {
	kCombatController_Allies,
	kCombatController_Targets,
	kCombatController_SelectedSpells,
	kCombatController_AvailableSpells,
};

static bool GetCombatControllerData_Execute(COMMAND_ARGS, UInt32 type)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	CombatController* controller = GetCombatController(thisObj);
	if (controller) {
		double idx = 0;

		switch (type) {
			case kCombatController_Allies:
				for (CombatController::AlliesList* cur = &controller->allies; cur && cur->ally; cur = cur->next) {
					g_ArrayMap.SetElementFormID(arr, idx, cur->ally->refID);
					idx += 1;
				}
				break;
			case kCombatController_Targets:
				for (CombatController::TargetList* cur = controller->targets; cur && cur->info && cur->info->target; cur = cur->next) {
					g_ArrayMap.SetElementFormID(arr, idx, cur->info->target->refID);
					idx += 1;
				}
				break;
			case kCombatController_SelectedSpells:
				{
					CombatController::SelectedSpellInfo* spells[] =	{
						controller->selectedBoundArmorSpell,
						controller->selectedBoundWeaponSpell,
						controller->selectedBuffSpell,
						controller->selectedMeleeSpell,
						controller->selectedRangedSpell,
						controller->selectedRestoreSpell,
						controller->selectedSummonSpell
					};

					for (UInt32 i = 0; i < sizeof(spells)  / sizeof(CombatController::SelectedSpellInfo*); i++) {
						if (spells[i] && spells[i]->item) {
							g_ArrayMap.SetElementFormID(arr, idx, (OBLIVION_CAST(spells[i]->item, MagicItem, TESForm))->refID);
							idx += 1;
						}
					}
				}
				break;
			case kCombatController_AvailableSpells:
				{
					CombatController::AvailableSpellList** cur = &controller->rangedSpells;
					for (CombatController::AvailableSpellList* list = *cur; list && list->info && list->info->item; list = list->next) {
						TESForm* magicForm = OBLIVION_CAST(list->info->item, MagicItem, TESForm);
						g_ArrayMap.SetElementFormID(arr, idx, magicForm->refID);
						idx += 1;
					}					
				}
				break;
		}
	}

	return true;
}

static bool Cmd_GetAllies_Execute(COMMAND_ARGS)
{
	return GetCombatControllerData_Execute(PASS_COMMAND_ARGS, kCombatController_Allies);
}

static bool Cmd_GetTargets_Execute(COMMAND_ARGS)
{
	return GetCombatControllerData_Execute(PASS_COMMAND_ARGS, kCombatController_Targets);
}

static bool Cmd_GetSelectedSpells_Execute(COMMAND_ARGS)
{
	return GetCombatControllerData_Execute(PASS_COMMAND_ARGS, kCombatController_SelectedSpells);
}

static bool Cmd_GetCombatSpells_Execute(COMMAND_ARGS)
{
	return GetCombatControllerData_Execute(PASS_COMMAND_ARGS, kCombatController_AvailableSpells);
}

static bool Cmd_PlayIdle_Execute(COMMAND_ARGS)
{
	UInt32 bForceIdle = 0;
	Actor* callingActor = NULL;
	TESForm* idleForm = NULL;
	*result = 0;

	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &idleForm, &bForceIdle))
		return true;

	switch (thisObj->typeID)
	{
	case kFormType_ACHR:
	case kFormType_ACRE:
		callingActor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
		break;
	default:
		return true;
	}

	if (callingActor->process)
	{
		TESIdleForm* idle = OBLIVION_CAST(idleForm, TESForm, TESIdleForm);

		if (idle && idle->animModel.nifPath.m_data)
		{
			std::string str(idle->animModel.nifPath.m_data);
			if (str.find(".kf") != std::string::npos)
			{
				ActorAnimData* animData = (ActorAnimData*)ThisVirtualStdCall(0x00A6E074, 0x164, callingActor);
				if (animData)
				{						// ### TODO expose the gunk that follows
					if (!animData->unkC8[2] || (UInt32)animData->niNodes24[0] != (UInt32)idle)
					{
						UInt32 unk01 = animData->unkC8[1], unk02 = unk01 + 0x10, unk03 = ThisStdCall(0x00472EA0, animData);

						if (bForceIdle || !unk03 || (unk01 && (*((UInt32*)unk01 + 4) != 3 || (*((UInt32*)unk02) && !((UInt32*)unk02 + 0x24)))))
						{
							ThisStdCall(0x00477DB0, animData, idle, callingActor, (idle->animFlags & 0x7F), 3); // ActorAnimData::QueueIdle (probably)
							if (IsConsoleMode()) {
								Console_Print("PlayIdle >> %s on %08X", idle->animModel.nifPath.m_data, idle->refID);
							}
							*result = 1;
						}
					}
				}
			}
		}
	}

	return true;
}

static bool Cmd_GetActors(tList<Actor>* list, Script* scriptObj, double* result)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;
	double idx = 0.0;
	for (tList<Actor>::Iterator iter = list->Begin(); !iter.End(); ++iter) {
		Actor* actor = iter.Get();
		if (actor) {
			g_ArrayMap.SetElementFormID(arr, idx, actor->refID);
			idx += 1.0;
		}
	}

	return true;
}

static bool Cmd_GetHighActors_Execute(COMMAND_ARGS)
{
	return Cmd_GetActors(&g_actorProcessManager->highActors, scriptObj, result);
}

static bool Cmd_GetMiddleHighActors_Execute(COMMAND_ARGS)
{
	return Cmd_GetActors(&g_actorProcessManager->middleHighActors.head, scriptObj, result);
}

static bool Cmd_GetLowActors_Execute(COMMAND_ARGS)
{
	return Cmd_GetActors(&g_actorProcessManager->lowActors18.head, scriptObj, result);
}


static bool Cmd_ToggleSkillPerk_Execute(COMMAND_ARGS)
{
	UInt32 actorVal = 0;
	UInt32 mastery = 0;
	UInt32 bEnable = 0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorVal, &mastery, &bEnable)) {
		if (ToggleSkillPerk(actorVal, mastery, bEnable ? true : false)) {
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_GetFactions_Execute(COMMAND_ARGS)
{
	class Counter {
	public:
		Counter(ArrayID arrID) : m_arrID(arrID), m_curIndex(0) { }

		bool Accept(const TESActorBaseData::FactionListData* entry) {
			if (entry == NULL)
				return false;

			g_ArrayMap.SetElementFormID(m_arrID, m_curIndex++, entry->faction->refID);
			return true;
		}
	private:
		ArrayID m_arrID;
		UInt32	m_curIndex;
	};

	TESActorBase* actor = NULL;
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &actor)) {
		if (actor == NULL && thisObj && (thisObj->baseForm->typeID == kFormType_Creature || thisObj->baseForm->typeID == kFormType_NPC))
			actor = OBLIVION_CAST(thisObj->baseForm, TESForm, TESActorBase);
		
		if (actor)	{
			FactionListVisitor(&actor->actorBaseData.factionList).Visit(Counter(arr));
		}
	}

	return true;
}

static bool Cmd_GetActorSwimBreath_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	HighProcess* highProcess = OBLIVION_CAST(actor->process, BaseProcess, HighProcess);
	if ( highProcess )
		*result = highProcess->swimBreath;

	return true;
}
static bool Cmd_SetActorSwimBreath_Execute(COMMAND_ARGS)
{
	float nuBreath = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &nuBreath))
		return true;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	HighProcess* highProcess = OBLIVION_CAST(actor->process, BaseProcess, HighProcess);
	if ( highProcess )
		highProcess->swimBreath = nuBreath;

	return true;
}
static bool Cmd_GetActorMaxSwimBreath_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	HighProcess* highProcess = OBLIVION_CAST(actor->process, BaseProcess, HighProcess);
	if ( highProcess )
		*result = EventManager::GetActorMaxSwimBreath(actor);

	return true;
}
static bool Cmd_SetActorMaxSwimBreath_Execute(COMMAND_ARGS)
{
	float nuMax = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &nuMax))
		return true;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	HighProcess* highProcess = OBLIVION_CAST(actor->process, BaseProcess, HighProcess);
	if ( highProcess )
		EventManager::SetActorMaxSwimBreath(actor, nuMax);

	return true;
}
static bool Cmd_OverrideActorSwimBreath_Execute(COMMAND_ARGS)
{
	UInt32 state = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &state))
		return true;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor)
		return true;

	HighProcess* highProcess = OBLIVION_CAST(actor->process, BaseProcess, HighProcess);
	if ( highProcess )
		EventManager::SetActorSwimBreathOverride(actor, state);

	return true;
}

#endif

static ParamInfo kParams_OneNPC[1] =
{
	{	"NPC",	kParamType_NPC,	1	},
};

CommandInfo kCommandInfo_HasSpell =
{
	"HasSpell",
	"hspl",
	0,
	"returns 1 if the actor has the spell",
	1,
	1,
	kParams_OneSpellItem,
	HANDLER(Cmd_HasSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMerchantContainer =
{
	"GetMerchantContainer",
	"",
	0,
	"returns the merchant container of the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetMerchantContainer_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_ModActorValue2[2] =
{
	{	"actor value", kParamType_ActorValue, 0 },
	{	"amount", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_ModActorValue2 =
{
	"ModActorValue2", "ModAV2",
	0,
	"Modify an actor's value in a non-permanent fashion. [player.modabv luck, -10]",
	1, 2, kParams_ModActorValue2,
	HANDLER(Cmd_ModActorValue2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetActorValueC =
{
	"GetActorValueC", "GetAVC",
	0,
	"Return an actor's value by code",
	1, 1, kParams_OneInt,
	HANDLER(Cmd_GetActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetBaseActorValueC =
{
	"GetBaseActorValueC", "GetBAVC",
	0,
	"Return an actor's base value by code",
	1, 1, kParams_OneInt,
	HANDLER(Cmd_GetBaseActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetBaseAVC =
{
	"GetBaseAVC", "",
	0,
	"Return an actor's base value by code",
	1, 1, kParams_OneInt,
	HANDLER(Cmd_GetBaseActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetModActorValueC[2] =
{
	{	"actor value", kParamType_Integer, 0 },
	{	"amount", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_SetActorValueC =
{
	"SetActorValueC", "SetAVC",
	0,
	"Set an actor's value by code. ",
	1, 2, kParams_SetModActorValueC,
	HANDLER(Cmd_SetActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModActorValueC =
{
	"ModActorValueC", "ModAVC",
	0,
	"Modify an actor's value in a non-permanent fashion.",
	1, 2, kParams_SetModActorValueC,
	HANDLER(Cmd_ModActorValue2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsRefEssential =
{
	"IsRefEssential",
	"",
	0,
	"returns 1 if the calling reference is essential",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsRefEssential_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetRefEssential =
{
	"SetRefEssential",
	"",
	0,
	"changes the calling reference to esential",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetRefEssential_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetActorLightAmount =
{
	"GetActorLightAmount",
	"",
	0,
	"returns a float describing the amount of light falling on an actor, or 100 if the actor is not in high/medium-high process",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetActorLightAmount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMerchantContainer =
{
	"SetMerchantContainer",
	"",
	0,
	"sets the merchant container of the reference",
	1,
	1,
	kParams_OneObjectRef,
	HANDLER(Cmd_SetMerchantContainer_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsUnderWater =
{
	"IsUnderWater",
	"",
	0,
	"returns 1 if the object is under the cell's water level",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsUnderWater_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_CopyNPCBodyData[2] =
{
	{	"copy from",	kParamType_NPC,		0	},
	{	"copy to",		kParamType_NPC,		1	},
};

CommandInfo kCommandInfo_CopyHair =
{
	"CopyHair",
	"",
	0,
	"copies hair from source to target",
	0,
	2,
	kParams_CopyNPCBodyData,
	HANDLER(Cmd_CopyHair_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CopyEyes =
{
	"CopyEyes",
	"",
	0,
	"copies eyes from source to target",
	0,
	2,
	kParams_CopyNPCBodyData,
	HANDLER(Cmd_CopyEyes_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetNPCBodyData[2] =
{
	{	"body data",	kParamType_InventoryObject,	0	},
	{	"base NPC",		kParamType_NPC,				1	},
};

CommandInfo kCommandInfo_SetHair =
{
	"SetHair",
	"",
	0,
	"sets the hair of the NPC",
	0,
	2,
	kParams_SetNPCBodyData,
	HANDLER(Cmd_SetHair_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetEyes =
{
	"SetEyes",
	"",
	0,
	"sets the hair of the NPC",
	0,
	2,
	kParams_SetNPCBodyData,
	HANDLER(Cmd_SetEyes_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetActorBaseFlag[1] =
{
	{	"base actor",	kParamType_ActorBase,	1	},
};

static ParamInfo kParams_SetActorBaseFlag[2] =
{
	{	"bool",			kParamType_Integer,		0	},
	{	"base actor",	kParamType_ActorBase,	1	},
};

static ParamInfo kParams_SetPCLevelOffset[4] =
{
	{	"bool",			kParamType_Integer,		0	},
	{	"min level",	kParamType_Integer,		1	},
	{	"max level",	kParamType_Integer,		1	},
	{	"base actor",	kParamType_ActorBase,	1	},
};

CommandInfo kCommandInfo_IsActorRespawning =
{
	"IsActorRespawning",
	"",
	0,
	"returns true if the actor respawns",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_IsActorRespawning_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsPCLevelOffset =
{
	"IsPCLevelOffset",
	"",
	0,
	"returns true if the actor's level is relative to the player's level",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_IsPCLevelOffset_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_HasLowLevelProcessing =
{
	"HasLowLevelProcessing",
	"HasLowLevProc",
	0,
	"returns true if the actor has low level processing",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_HasLowLevelProcessing_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsSummonable =
{
	"IsSummonable",
	"",
	0,
	"returns true if the actor is summonable",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_IsSummonable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_HasNoPersuasion =
{
	"HasNoPersuasion",
	"",
	0,
	"returns true if the actor has no persuasion",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_HasNoPersuasion_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CanCorpseCheck =
{
	"CanCorpseCheck",
	"",
	0,
	"returns true if the actor can corpse check",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_CanCorpseCheck_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsFemale =
{
	"IsFemale", "",
	0,
	"returns true if the actor is female",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_IsFemale_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetActorBaseLevel, returns the level or level offset of the specified actor, 0, 1, kParams_GetActorBaseFlag);

CommandInfo kCommandInfo_GetActorMinLevel =
{
	"GetActorMinLevel",
	"GetMinLevel",
	0,
	"returns the minimum level of the actor",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_GetActorMinLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetActorMaxLevel =
{
	"GetActorMaxLevel",
	"GetMaxLevel",
	0,
	"returns the maximum level of the actor",
	0,
	1,
	kParams_GetActorBaseFlag,
	HANDLER(Cmd_GetActorMaxLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetFemale =
{
	"SetFemale",
	"",
	0,
	"toggles female flag on the actor",
	0,
	2,
	kParams_SetActorBaseFlag,
	HANDLER(Cmd_SetFemale_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetActorRespawns =
{
	"SetActorRespawns",
	"",
	0,
	"toggles respawn flag on the actor",
	0,
	2,
	kParams_SetActorBaseFlag,
	HANDLER(Cmd_SetActorRespawns_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetLowLevelProcessing =
{
	"SetLowLevelProcessing",
	"SetLowLevProc",
	0,
	"toggles low level processing flag on the actor",
	0,
	2,
	kParams_SetActorBaseFlag,
	HANDLER(Cmd_SetLowLevelProcessing_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetSummonable =
{
	"SetSummonable",
	"",
	0,
	"toggles summonable flag on the actor",
	0,
	2,
	kParams_SetActorBaseFlag,
	HANDLER(Cmd_SetSummonable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetNoPersuasion =
{
	"SetNoPersuasion",
	"",
	0,
	"toggles no persuasion flag on the actor",
	0,
	2,
	kParams_SetActorBaseFlag,
	HANDLER(Cmd_SetNoPersuasion_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCanCorpseCheck =
{
	"SetCanCorpseCheck",
	"SetCorpseCheck",
	0,
	"toggles can corpse check flag on the actor",
	0,
	2,
	kParams_SetActorBaseFlag,
	HANDLER(Cmd_SetCanCorpseCheck_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetPCLevelOffset =
{
	"SetPCLevelOffset",
	"",
	0,
	"toggles PC level offset flag and optionally sets min/max levels",
	0,
	4,
	kParams_SetPCLevelOffset,
	HANDLER(Cmd_SetPCLevelOffset_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetHorse =
{
	"GetHorse", "",
	0,
	"returns a reference to the horse currently ridden by the calling actor",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetHorse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetRace =
{
	"GetRace",
	"",
	0,
	"returns the race of the specified NPC",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_GetRace_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetHair =
{
	"GetHair",
	"",
	0,
	"returns the refID of the NPC's hair",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_GetHair_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneIntOneOptionalNPC[2] =
{
	{	"RGB value",	kParamType_Integer,	0	},
	{	"NPC",			kParamType_NPC,		1	},
};

CommandInfo kCommandInfo_GetHairColor =
{
	"GetHairColor",
	"",
	0,
	"returns the R G or B value of the NPC's hair color",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_GetHairColor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetEyes =
{
	"GetEyes",
	"",
	0,
	"returns the refID of the NPC's eyes",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_GetEyes_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetEquippedItems,
			   "returns an array containing the calling actor's equipped items",
			   1, 0, NULL);

DEFINE_COMMAND(GetBaseAV2, 	"returns the base actor value without magical modifiers (DEPRECATED)", 1, 1, kParams_OneActorValue);
CommandInfo kCommandInfo_GetBaseAV2C =
{
	"GetBaseAV2C",
	"",
	0,
	"returns the base actor value without magical modifiers",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetBaseAV2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


DEFINE_COMMAND_CONDITIONAL(GetBaseAV3, "returns the base actor value without magical modifiers", 1, 1, kParams_OneActorValue);
CommandInfo kCommandInfo_GetBaseAV3C =
{
	"GetBaseAV3C",
	"",
	0,
	"returns the base actor value without magical modifiers",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetBaseAV3_Execute),
	Cmd_Default_Parse,
	HANDLER_EVAL(Cmd_GetBaseAV3_Eval),
	0
};

static ParamInfo kParams_ToggleSpecialAnim[3] =
{
	{	"animPath",		kParamType_String,		0	},
	{	"bToggleOn",	kParamType_Integer,		0	},
	{	"actor base",	kParamType_ActorBase,	1	},
};

DEFINE_COMMAND(ToggleSpecialAnim, adds or removes a special anim from an actor, 0, 3, kParams_ToggleSpecialAnim);
DEFINE_COMMAND(GetSpecialAnims, returns an array of special anims for an actor, 0, 1, kParams_OneOptionalActorBase);

DEFINE_COMMAND(GetActorAlpha, returns the actors alpha, 1, 0, NULL);

static ParamInfo kParams_SetCanCastPower[2] =
{
	{	"greater power",	kParamType_SpellItem,	0	},
	{	"bCanCast",			kParamType_Integer,		1	},
};

DEFINE_COMMAND(CanCastPower, returns 1 if the actor is allowed to cast the greater power, 1, 1, kParams_OneSpellItem);
DEFINE_COMMAND(SetCanCastPower, allows or disallows the actor from casting the greater power, 1, 2, kParams_SetCanCastPower);

DEFINE_COMMAND(GetUsedPowers, "returns an array containing info about the greater powers an actor has used", 1, 0, NULL);

static ParamInfo kParams_SetPowerTimer[2] =
{
	{	"greater power",	kParamType_SpellItem,	0	},
	{	"timer",			kParamType_Float,		0	},
};

DEFINE_COMMAND(SetPowerTimer, "sets the time until a power becomes castable again", 1, 2, kParams_SetPowerTimer);

CommandInfo kCommandInfo_GetPlayerSpellCount =
{
	"GetPlayerSpellCount",
	"GetSpellCount",
	0,
	"returns the number of spells in the player's spell list",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetSpellCount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthPlayerSpell =
{
	"GetNthPlayerSpell",
	"GetNthSpell",
	0,
	"returns the spell at the specified slot in the list",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveAllSpells =
{
	"RemoveAllSpells",
	"RemSpells",
	0,
	"removes all of the spells from the reference",
	1,
	0,
	0,
	HANDLER(Cmd_RemoveAllSpells_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetAllies, returns an actors allies in combat, 1, 0, NULL);
DEFINE_COMMAND(GetTargets, returns an actors targets in combat, 1, 0, NULL);
DEFINE_COMMAND(GetSelectedSpells, returns an actors selected spells in combat, 1, 0, NULL);
DEFINE_COMMAND(GetCombatSpells, returns an actors spell list in combat, 1, 0, NULL);

static ParamInfo kParams_PlayIdle[2] =
{
	{	"idle form",  kParamType_InventoryObject, 0},
	{	"force idle", kParamType_Integer, 1},
};

DEFINE_COMMAND(PlayIdle, plays a specific idle on the calling actor, 1, 2, kParams_PlayIdle);

DEFINE_COMMAND(GetHighActors, returns all high process actors, 0, 0, NULL);
DEFINE_COMMAND(GetMiddleHighActors, returns all middle-high process actors, 0, 0, NULL);

static ParamInfo kParams_ToggleSkillPerk[3] =
{
	{	"skill",	kParamType_ActorValue,	0	},
	{	"mastery",	kParamType_Integer,		0	},
	{	"bEnable",	kParamType_Integer,		0	},
};

DEFINE_COMMAND(ToggleSkillPerk, toggles a skill perk on or off, 0, 3, kParams_ToggleSkillPerk);

DEFINE_COMMAND(CopyRace, sets the race of the calling npc, 1, 1, kParams_OneNPC);

static ParamInfo kParams_GetFactions[1] =
{
	{	"actor", kParamType_ActorBase, 1}
};

DEFINE_COMMAND(GetFactions, gets the factions of the calling actor, 0, 1, kParams_GetFactions);
DEFINE_COMMAND(GetLowActors, returns all low process actors, 0, 0, NULL);

DEFINE_COMMAND(GetActorSwimBreath, returns an actors breath left in seconds, 1, 0, NULL);
DEFINE_COMMAND(SetActorSwimBreath, sets an actors breath left in seconds, 1, 1, kParams_OneFloat);

DEFINE_COMMAND(GetActorMaxSwimBreath, returns an actors maximum breath in seconds, 1, 0, NULL);
DEFINE_COMMAND(SetActorMaxSwimBreath, sets an actors maximum breath in seconds, 1, 1, kParams_OneFloat);

DEFINE_COMMAND(OverrideActorSwimBreath, overrides breath behaviour with a few possible options, 1, 1, kParams_OneInt);

DEFINE_COMMAND_CONDITIONAL(IsNaked, "Return 1 if the actor doesn't have a lowerbody model equipped, or an upperbody with argument 1, 0 otherwise" , 1, 1, kParams_OneOptionalInt);
