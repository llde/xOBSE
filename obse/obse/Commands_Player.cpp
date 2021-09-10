#include "Commands_Player.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "InternalSerialization.h"
#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameProcess.h"
#include "Hooks_Gameplay.h"
#include "ArrayVar.h"
#include "ScriptUtils.h"
#include "GameData.h"
#include "GameMagicEffects.h"

static const _Cmd_Execute Cmd_AddSpell_Execute = (_Cmd_Execute)0x00514950;
static const _Cmd_Execute Cmd_RemoveSpell_Execute = (_Cmd_Execute)0x00510B90;
static const _Cmd_Execute Cmd_GetBaseAV_Execute = (_Cmd_Execute)0x00501A00;

static bool Cmd_GetActiveSpell_Execute(COMMAND_ARGS)
{
	UInt32			* refResult = (UInt32 *)result;
	MagicItem		* activeMagicItem = (*g_thePlayer)->GetActiveMagicItem();

	*refResult = 0;

	if(activeMagicItem)
	{
		TESForm	* activeMagicItemForm = (TESForm *)Oblivion_DynamicCast(activeMagicItem, 0, RTTI_MagicItem, RTTI_TESForm, 0);
		if(activeMagicItemForm)
			*refResult = activeMagicItemForm->refID;
	}

	if (IsConsoleMode())
		Console_Print("GetActiveSpell >> %08X", *refResult);

	return true;
}

static bool Cmd_SetActiveSpell_Execute(COMMAND_ARGS)
{
	TESForm	* spell = NULL;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &spell)) return true;

	SpellItem	* spellItem = (SpellItem *)Oblivion_DynamicCast(spell, 0, RTTI_TESForm, RTTI_SpellItem, 0);
	if(spellItem)
	{
		(*g_thePlayer)->SetActiveSpell(&spellItem->magicItem);
	}

	return true;
}

static bool Cmd_IsThirdPerson_Execute(COMMAND_ARGS)
{
	// g_thePlayer guaranteed to be non-null
	*result = (*g_thePlayer && (*g_thePlayer)->isThirdPerson) ? 1 : 0;

//	Console_Print("IsThirdPerson = %f", *result);

	return true;
}

// (expValue:float) IncrementPlayerSkillUse skill:actor value whichTrigger
static bool Cmd_IncrementPlayerSkillUse_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 valSkill = 0;
	UInt32 whichUse = 0;
	float howManyTimes = 1.0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &valSkill, &whichUse, &howManyTimes))
		return true;
	if (!IsSkill(valSkill)) return true;

	(*g_thePlayer)->ModExperience(valSkill, whichUse, howManyTimes);

	*result = (*g_thePlayer)->skillExp[valSkill-kActorVal_Armorer];
	return true;
}

// (expValue:float) IncrementPlayerSkillUse skill:actor value whichTrigger
static bool Cmd_TriggerPlayerSkillUse_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 valSkill = 0;
	UInt32 whichUse = 0;
	float howManyTimes = 1.0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &valSkill, &whichUse, &howManyTimes))
		return true;
	if (!IsSkill(valSkill)) return true;

	(*g_thePlayer)->ChangeExperience(valSkill, whichUse, howManyTimes);
	*result = (*g_thePlayer)->skillExp[valSkill-kActorVal_Armorer];

	return true;
}

// (expValue:float) IncrementPlayerSkillUse skill:actor value whichTrigger
static bool Cmd_ModPlayerSkillExp_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 valSkill = 0;
	float expChange = 0.0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &valSkill, &expChange))
		return true;
	if (!IsSkill(valSkill)) return true;

	(*g_thePlayer)->ChangeExperience(valSkill, expChange);
	*result = (*g_thePlayer)->skillExp[valSkill-kActorVal_Armorer];

	return true;
}

// (expValue:float) GetPlayerSkillUse skill:actor value
static bool Cmd_GetPlayerSkillUse_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 valSkill = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &valSkill))
		return true;
	if (!IsSkill(valSkill)) return true;

	*result = (*g_thePlayer)->skillExp[valSkill - kActorVal_Armorer];
	return true;
}

static bool Cmd_GetPlayerSkillAdvances_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 valSkill = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &valSkill))
		return true;
	if (!IsSkill(valSkill)) return true;

	*result = (*g_thePlayer)->GetSkillAdvanceCount(valSkill);
	return true;
}

static bool Cmd_SetPlayerSkillAdvances_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 valSkill = 0;
	SInt32 advCount = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &valSkill, &advCount))
		return true;
	if (!IsSkill(valSkill)  || advCount < 0) return true;

	(*g_thePlayer)->SetSkillAdvanceCount(valSkill, advCount);

	return true;
}

static bool Cmd_SetPCAMurderer_Execute(COMMAND_ARGS)
{
	*result = (*g_thePlayer)->isAMurderer != 0;

	int	value = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &value)) return true;

	(*g_thePlayer)->isAMurderer = value ? 1 : 0;

	return true;
}

static bool Cmd_GetPlayersLastRiddenHorse_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if ((*g_thePlayer)->lastRiddenHorse)
		*refResult = (*g_thePlayer)->lastRiddenHorse->refID;

	return true;
}

static bool Cmd_SetPlayersLastRiddenHorse_Execute(COMMAND_ARGS)
{
	TESObjectREFR* horseRef = NULL;
	*result = 0.0;

	// must be a persistent reference since game keeps a pointer to it
	if (ExtractArgs(PASS_EXTRACT_ARGS, &horseRef) && horseRef && horseRef->IsPersistent()) {
		Creature* horse = OBLIVION_CAST(horseRef, TESObjectREFR, Creature);
		// must be a creature
		if (horse && horse->baseForm) {
			TESCreature* horseBase = OBLIVION_CAST(horse->baseForm, TESForm, TESCreature);
			// base must be a horse-type creature
			if (horseBase && horseBase->type == TESCreature::eCreatureType_Horse) {
				(*g_thePlayer)->lastRiddenHorse = horse;
				*result = 1.0;
			}
		}
	}

	return true;
}

static bool Cmd_ClearPlayersLastRiddenHorse_Execute(COMMAND_ARGS)
{
	// this is fine to use while player is mounted - lastRiddenHorse is not updated when dismounting, only when mounting.
	(*g_thePlayer)->lastRiddenHorse = NULL;
	return true;
}

static bool Cmd_GetPlayersLastActivatedLoadDoor_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if ((*g_thePlayer)->lastActivatedLoadDoor)
		*refResult = (*g_thePlayer)->lastActivatedLoadDoor->refID;

	return true;
}

static bool Cmd_AddSpellNS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_AddSpell_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);
	return true;
}

static bool Cmd_RemoveSpellNS_Execute(COMMAND_ARGS)
{
	ToggleUIMessages(false);
	Cmd_RemoveSpell_Execute(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);
	return true;
}

static bool Cmd_GetPCMajorSkillUps_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (*g_thePlayer)
		*result = (*g_thePlayer)->majorSkillAdvances;

	return true;
}

static bool Cmd_SetPCMajorSkillUps_Execute(COMMAND_ARGS)
{
	UInt32 nuValue = 0;
	*result = 0;
	PlayerCharacter* pc = *g_thePlayer;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &nuValue) && pc)
	{
		pc->majorSkillAdvances = nuValue;

		// check if advancements allow player to level up
		SettingInfo* setting = NULL;
		if (GetGameSetting("iLevelUpSkillCount", &setting))
		{
			float advPts = nuValue / setting->i;
			pc->bCanLevelUp = (advPts >= 1.0) ? 1 : 0;

			// HUD LevelUp icon updates automatically
		}

		*result = (pc->bCanLevelUp) ? 1 : 0;
	}

	return true;
}

static bool Cmd_GetPCAttributeBonus_Execute(COMMAND_ARGS)
{
	UInt32 whichAttribute = 0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &whichAttribute))
	{
		*result = (*g_thePlayer)->GetAttributeBonus(whichAttribute);
		if (IsConsoleMode())
			Console_Print("GetPCAttributeBonus >> %.0f", *result);
	}
	return true;
}

static bool Cmd_SetPCAttributeBonus_Execute(COMMAND_ARGS)
{
	UInt32 whichAttribute = 0;
	UInt32 newValue = 0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &whichAttribute, &newValue) && *g_thePlayer)
		if (newValue < 0x100)
			(*g_thePlayer)->SetAttributeBonus(whichAttribute, newValue);

	return true;
}

static bool Cmd_GetTotalPCAttributeBonus_Execute(COMMAND_ARGS)
{
	*result = 0;
	for (UInt32 attr = 0; attr < kActorVal_Luck; attr++)
		*result += (*g_thePlayer)->GetAttributeBonus(attr);

	return true;
}

static bool Cmd_GetSpellEffectiveness_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (thisObj)
	{
		MagicCaster* caster = (MagicCaster*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MagicCaster, 0);
		if (caster)
			*result = caster->GetSpellEffectiveness(0, 0);
	}

	return true;
}

static bool Cmd_ModPCSpellEffectiveness_Execute(COMMAND_ARGS)
{
	float modBy = 0;
	UInt32 bPersist = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &modBy, &bPersist))
		ModPlayerSpellEffectiveness(modBy, bPersist ? true : false);

	return true;
}

static bool Cmd_GetPCSpellEffectivenessModifier_Execute(COMMAND_ARGS)
{
	*result = GetPlayerSpellEffectivenessModifier();
	return true;
}

static bool Cmd_ModPCMovementSpeed_Execute(COMMAND_ARGS)
{
	float modBy = 0;
	UInt32 bPersist = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &modBy, &bPersist))
		ModPlayerMovementSpeed(modBy, bPersist ? true : false);

	return true;
}

static bool Cmd_GetPCMovementSpeedModifier_Execute(COMMAND_ARGS)
{
	*result = GetPlayerMovementSpeedModifier();
	return true;
}

static bool Cmd_ToggleFirstPerson_Execute(COMMAND_ARGS)
{
	UInt32 bFirstPerson = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &bFirstPerson) && *g_thePlayer)
		(*g_thePlayer)->TogglePOV(bFirstPerson ? true : false);

	return true;
}

static bool Cmd_GetPCTrainingSessionsUsed_Execute(COMMAND_ARGS)
{
	*result = (*g_thePlayer)->trainingSessionsUsed;
	return true;
}

static bool Cmd_SetPCTrainingSessionsUsed_Execute(COMMAND_ARGS)
{
	UInt32 numSessions = 0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &numSessions))
		(*g_thePlayer)->trainingSessionsUsed = numSessions;

	return true;
}

static bool Cmd_GetPCTrainingSessionsUsed_Eval(COMMAND_ARGS_EVAL)
{
	*result = (*g_thePlayer)->trainingSessionsUsed;
	return true;
}

static bool Cmd_GetCurrentRegion_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;
	TESRegion* region = (*g_thePlayer)->region;
	if (region)
		*refResult = region->refID;

	if (IsConsoleMode())
		Console_Print("GetCurrentRegion >> %08X", (region ? region->refID : 0));

	return true;
}

static bool Cmd_GetCurrentRegions_Execute(COMMAND_ARGS)
{
	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	TESObjectCELL* cell = (*g_thePlayer)->parentCell;
	if (cell)
	{
		ExtraRegionList* xRegionList = (ExtraRegionList*)cell->extraData.GetByType(kExtraData_RegionList);
		if (xRegionList && xRegionList->regionList)
		{
			double idx = 0.0;
			for (TESRegionList::Entry* cur = &xRegionList->regionList->regionList; cur && cur->region; cur = cur->next)
			{
				g_ArrayMap.SetElementFormID(arrID, idx, cur->region->refID);
				idx += 1.0;
#if _DEBUG
				Console_Print("Region %08x addr %08x", cur->region->refID, cur->region);
#endif
			}
		}
	}

	return true;
}

static bool Cmd_SetPlayerBirthSign_Execute(COMMAND_ARGS)
{
	BirthSign* birthsign;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &birthsign))
		(*g_thePlayer)->SetBirthSign(birthsign);

	return true;
}

/*
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2
const UInt32 kPCDeathPatchAddress = 0x006009BC;
const UInt32 kPCDeathReturnAddress = 0x006009C1;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
const UInt32 kPCDeathPatchAddress = 0x00600B7C;
const UInt32 kPCDeathReturnAddress = 0x00600B81;
#endif
static __declspec(naked)void PCDeathHook(void)
{
	__asm
	{
		pushad								// save the registers before doing anything else
	}

	Console_Print("Bang, you're dead.");	// or do something more interesting

	__asm
	{
		popad								// restore the registers
		push	0x00A6ED58					// code overwritten by hook
		jmp		[kPCDeathReturnAddress]		// resume after hook
	}
}
//install a hook for when the player dies
	WriteRelJump(kPCDeathPatchAddress, (UInt32)PCDeathHook);
	Console_Print("hook: %08x", PCDeathHook);
	return true;
*/

/*
#include "GameMenus.h"
static bool Cmd_Debug_Execute(COMMAND_ARGS)
{
	if (!InterfaceManager::GetSingleton())
		_MESSAGE("interface manager singleton is NULL");

	return true;

	EnchantmentMenu* menu = (EnchantmentMenu*)GetMenuByType(kMenuType_Enchantment);
	Console_Print("%08x", menu);

	return true;

	typedef void (* _DoLoadGameMenu)(void);
	_DoLoadGameMenu DoLoadGameMenu = (_DoLoadGameMenu)0x005AEA60;

	DoLoadGameMenu();
	return true;

	EquippedItemsList itemList = (*g_thePlayer)->GetEquippedItems();
	for (UInt32 i = 0; i < itemList.size(); i++)
		PrintItemType(itemList[i]);

	return true;
}

*/

static bool Cmd_GetPCLastDroppedItem_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	TESForm* form = GetPCLastDroppedItem();
	if (form)
		*refResult = form->refID;

	if (IsConsoleMode())
		Console_Print("GetPCLastDroppedItem >> %08X (%s)", *refResult, GetFullName(form));

	return true;
}

static bool Cmd_GetPCLastDroppedItemRef_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = GetPCLastDroppedItemRef();
	if (IsConsoleMode())
		Console_Print("GetPCLastDroppedItemRef >> %08X", *refResult);

	return true;
}

static bool Cmd_GetPlayerBirthsign_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	BirthSign* sign = (*g_thePlayer)->birthSign;
	*refResult = sign ? sign->refID : 0;
	return true;
}

static bool Cmd_SetPlayerSkeletonPath_Execute(COMMAND_ARGS)
{
	*result = 0.0;

	char skelliePath[MAX_PATH];
	PlayerCharacter* pc = *g_thePlayer;
	if (pc)
	{
		if (ExtractArgs(PASS_EXTRACT_ARGS, skelliePath)) {
			*result = pc->SetSkeletonPath(skelliePath) ? 1.0 : 0.0;
		}
	}

	return true;
}

static bool Cmd_GetTransactionInfo_Execute(COMMAND_ARGS)
{
	char type[0x100] = { 0 };
	ArrayID arrID = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, type) && type[0]) {
		const TransactionInfo* info = NULL;
		if (!_stricmp(type, "buy")) {
			info = GetLastTransactionInfo(kPC_Buy, scriptObj->refID);
		}
		else if (!_stricmp(type, "sell")) {
			info = GetLastTransactionInfo(kPC_Sell, scriptObj->refID);
		}

		if (info) {
			arrID = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
			g_ArrayMap.SetElementFormID(arrID, "buyer", info->buyer->refID);
			g_ArrayMap.SetElementFormID(arrID, "seller", info->seller->refID);
			g_ArrayMap.SetElementFormID(arrID, "item", info->item->refID);
			g_ArrayMap.SetElementNumber(arrID, "price", info->price);
			g_ArrayMap.SetElementNumber(arrID, "quantity", info->quantity);
		}
	}

	*result = arrID;
	return true;
}

static bool Cmd_GetRequiredSkillExp_Execute(COMMAND_ARGS)
{
	SInt32 av = -1;
	*result = -1.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &av) && IsSkill(av) && *g_thePlayer) {
		av -= kActorVal_Armorer;
		*result = (*g_thePlayer)->requiredSkillExp[av];
	}

	return true;
}

PlayerCharacter* pc = *g_thePlayer;

static bool Cmd_SetFlyCameraSpeedMult_Execute(COMMAND_ARGS)
{
	static const float kOriginalValue = 10.0f;

	float multiplier = 10.0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &multiplier)) {
		g_PlayerFlyCamSpeed = kOriginalValue * multiplier;
	}

	return true;
}

#endif

CommandInfo kCommandInfo_GetActiveSpell =
{
	"GetPlayerSpell",
	"GetActiveSpell",
	0,
	"returns the base spell object for the player's active spell",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetActiveSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetActiveSpell =
{
	"SetActiveSpell",
	"sspl",
	0,
	"sets the active spell to the argument",
	0,
	1,
	kParams_OneSpellItem,
	HANDLER(Cmd_SetActiveSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsThirdPerson =
{
	"IsThirdPerson",
	"",
	0,
	"returns if the player is using a third-person camera",
	0,
	0,
	NULL,
	HANDLER(Cmd_IsThirdPerson_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetPlayerSkillUse =
{
	"GetPlayerSkillUse",
	"",
	0,
	"returns the player's skills",
	0,
	1,
	kParams_OneActorValue,
	HANDLER(Cmd_GetPlayerSkillUse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetPlayerSkillUseC =
{
	"GetPlayerSkillUseC",
	"",
	0,
	"returns the player's skills",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetPlayerSkillUse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetPlayerSkillAdvances =
{
	"GetPlayerSkillAdvances",
	"",
	0,
	"returns the number of advances for the given skill",
	0,
	1,
	kParams_OneActorValue,
	HANDLER(Cmd_GetPlayerSkillAdvances_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetPlayerSkillAdvancesC =
{
	"GetPlayerSkillAdvancesC",
	"",
	0,
	"returns the number of advances for the given skill",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetPlayerSkillAdvances_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetPlayerSkillAdvances[2] =
{
	{	"skill", kParamType_ActorValue, 0 },
	{	"advances", kParamType_Integer, 0},
};

CommandInfo kCommandInfo_SetPlayerSkillAdvances =
{
	"SetPlayerSkillAdvances",
	"",
	0,
	"sets the number of advances for the given skill",
	0,
	2,
	kParams_SetPlayerSkillAdvances,
	HANDLER(Cmd_SetPlayerSkillAdvances_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetPlayerSkillAdvancesC[2] =
{
	{	"skill", kParamType_Integer, 0 },
	{	"advances", kParamType_Integer, 0},
};

CommandInfo kCommandInfo_SetPlayerSkillAdvancesC =
{
	"SetPlayerSkillAdvancesC",
	"",
	0,
	"setss the number of advances for the given skill",
	0,
	2,
	kParams_SetPlayerSkillAdvancesC,
	HANDLER(Cmd_SetPlayerSkillAdvances_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_IncrementPlayerSkillUse[3] =
{
	{	"skill", kParamType_ActorValue, 0 },
	{	"index", kParamType_Integer, 1 },
	{	"howManyTimes", kParamType_Float, 1},
};

CommandInfo kCommandInfo_IncrementPlayerSkillUse =
{
	"IncrementPlayerSkillUse",
	"",
	0,
	"increments the player's skill as if the appropriate action occured",
	0,
	3,
	kParams_IncrementPlayerSkillUse,
	HANDLER(Cmd_IncrementPlayerSkillUse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_TriggerPlayerSkillUse =
{
	"TriggerPlayerSkillUse",
	"",
	0,
	"increments the player's skill as if the appropriate action occured",
	0,
	3,
	kParams_IncrementPlayerSkillUse,
	HANDLER(Cmd_TriggerPlayerSkillUse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_IncrementPlayerSkillUseC[3] =
{
	{	"skill", kParamType_Integer, 0 },
	{	"index", kParamType_Integer, 1 },
	{	"howManyTimes", kParamType_Float, 1},
};

CommandInfo kCommandInfo_IncrementPlayerSkillUseC =
{
	"IncrementPlayerSkillUseC",
	"",
	0,
	"increments the player's skill as if the appropriate action occured",
	0,
	3,
	kParams_IncrementPlayerSkillUseC,
	HANDLER(Cmd_IncrementPlayerSkillUse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_TriggerPlayerSkillUseC =
{
	"TriggerPlayerSkillUseC",
	"",
	0,
	"increments the player's skill as if the appropriate action occured",
	0,
	3,
	kParams_IncrementPlayerSkillUseC,
	HANDLER(Cmd_TriggerPlayerSkillUse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_ModPlayerSkillExp[2] =
{
	{	"skill", kParamType_ActorValue, 0 },
	{	"amount", kParamType_Float, 0},
};

CommandInfo kCommandInfo_ModPlayerSkillExp =
{
	"ModPlayerSkillExp",
	"",
	0,
	"adjusts the given skill experience by the specified amount",
	0,
	2,
	kParams_ModPlayerSkillExp,
	HANDLER(Cmd_ModPlayerSkillExp_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_ModPlayerSkillExpC[2] =
{
	{	"skill", kParamType_Integer, 0 },
	{	"amount", kParamType_Float, 0},
};

CommandInfo kCommandInfo_ModPlayerSkillExpC =
{
	"ModPlayerSkillExpC",
	"",
	0,
	"adjusts the given skill experience by the specified amount",
	0,
	2,
	kParams_ModPlayerSkillExpC,
	HANDLER(Cmd_ModPlayerSkillExp_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetPCAMurderer =
{
	"SetPCAMurderer",
	"",
	0,
	"sets whether or not the PC has ever killed an NPC",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetPCAMurderer_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetPlayersLastRiddenHorse =
{
	"GetPlayersLastRiddenHorse", "GetPCLastHorse",
	0,
	"returns the last horse ridden by the player",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetPlayersLastRiddenHorse_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetPlayersLastActivatedLoadDoor =
{
	"GetPlayersLastActivatedLoadDoor", "GetPCLastLoadDoor",
	0,
	"returns the last load door activated by the player",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetPlayersLastActivatedLoadDoor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneNPC[1] =
{
	{	"NPC",	kParamType_NPC,	1	},
};

CommandInfo kCommandInfo_AddSpellNS =
{
	"AddSpellNS",
	"",
	0,
	"version of AddSpell which doesn't generate UI messages",
	1,
	1,
	kParams_OneSpellItem,
	HANDLER(Cmd_AddSpellNS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveSpellNS =
{
	"RemoveSpellNS",
	"",
	0,
	"version of RemoveSpell which doesn't generate UI messages",
	1,
	1,
	kParams_OneSpellItem,
	HANDLER(Cmd_RemoveSpellNS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetPCMajorSkillUps,
			   returns the total major skill advances for this level,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetPCAttributeBonus,
			   returns the level-up bonus for the specified attribute,
			   0,
			   1,
			   kParams_OneActorValue);

DEFINE_COMMAND(SetPCMajorSkillUps,
			   sets the total number of major skill advances for the current level,
			   0,
			   1,
			   kParams_OneInt);

static ParamInfo kParams_ModActorValue2[2] =
{
	{	"actor value", kParamType_ActorValue, 0 },
	{	"amount", kParamType_Integer, 0 },
};

DEFINE_COMMAND(SetPCAttributeBonus,
			   sets the level up bonus for the specified attribute,
			   0,
			   2,
			   kParams_ModActorValue2);

CommandInfo kCommandInfo_GetPCAttributeBonusC =
{
	"GetPCAttributeBonusC", "", 0,
   "returns the level-up bonus for the specified attribute",
   0, 1, kParams_OneInt,
   HANDLER(Cmd_GetPCAttributeBonus_Execute), Cmd_Default_Parse,
   NULL, 0
};

CommandInfo kCommandInfo_SetPCAttributeBonusC =
{
	"SetPCAttributeBonusC", "", 0,
   "sets the level-up bonus for the specified attribute",
   0, 2, kParams_TwoInts,
   HANDLER(Cmd_SetPCAttributeBonus_Execute), Cmd_Default_Parse,
   NULL, 0
};

DEFINE_COMMAND(GetTotalPCAttributeBonus,
			   returns the total number of attribute bonuses for the current level,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetSpellEffectiveness,
			   returns the player spell effectiveness,
			   1,
			   0,
			   NULL);

static ParamInfo kParams_OneFloat_OneOptionalInt[2] =
{
	{ "float",	kParamType_Float,	0	},
	{ "int",	kParamType_Integer,	1	}
};

DEFINE_COMMAND(ModPCSpellEffectiveness,
			   modifies the player spell effectiveness,
			   0,
			   2,
			   kParams_OneFloat_OneOptionalInt);

DEFINE_COMMAND(GetPCSpellEffectivenessModifier,
			   returns the modifier on player spell effectiveness,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(ModPCMovementSpeed, modifies the players movement speed, 0, 2, kParams_OneFloat_OneOptionalInt);
DEFINE_COMMAND(GetPCMovementSpeedModifier, returns the modifier on the players movement speed, 0, 0, NULL);

DEFINE_COMMAND(ToggleFirstPerson,
			   toggles the POV,
			   0,
			   1,
			   kParams_OneInt);

/*DEFINE_COMMAND(GetPCTrainingSessionsUsed,
			   returns the number of training sessions used during the players current level,
			   0,
			   0,
			   NULL);
*/

static bool Cmd_Parse_Test(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3)
{
	_MESSAGE("Cmd_Parse_Test: %08x %08x %08x %08x", arg0, arg1, arg2, arg3);
		return true;
}

CommandInfo kCommandInfo_GetPCTrainingSessionsUsed =
{
	"GetPCTrainingSessionsUsed",
	"",
	0,
	"returns the number of training sessions used during the players current level",
	0,
	0,
	//(ParamInfo*)0x0092BB8A,
	NULL,
	HANDLER(Cmd_GetPCTrainingSessionsUsed_Execute),
	Cmd_Default_Parse,
	HANDLER_EVAL(Cmd_GetPCTrainingSessionsUsed_Eval),
	1
};

DEFINE_COMMAND(SetPCTrainingSessionsUsed,
			   sets the number of training sessions used during the players current level,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetCurrentRegion,
			   returns the region the player is currently in,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetPCLastDroppedItem,
			   returns the base object of the last item dropped by the player,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetPCLastDroppedItemRef,
			   returns a reference to the item most recently dropped by the player,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetPlayerBirthsign, returns the players birthsign, 0, 0, NULL);
DEFINE_COMMAND(GetCurrentRegions, returns an array of regions the player is currently in, 0, 0, NULL);

static ParamInfo kParams_OneBirthSign[1] =
{
	{	"birthsign",	kParamType_Birthsign,	0	},
};

DEFINE_COMMAND(SetPlayerBirthSign, changes the players birthsign, 0, 1, kParams_OneBirthSign);
DEFINE_COMMAND(SetPlayerSkeletonPath, changes the skeleton used by the player, 0, 1, kParams_OneString);

DEFINE_COMMAND(GetTransactionInfo, returns info about the last barter transaction,
			   0, 1, kParams_OneString);

DEFINE_COMMAND(GetRequiredSkillExp, returns the amount of experience needed to increase the skill, 0, 1, kParams_OneActorValue);
CommandInfo kCommandInfo_GetRequiredSkillExpC =
{
	"GetRequiredSkillExpC",
	"",
	0,
	"returns the amount of experience needed to increase the skill",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetRequiredSkillExp_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(SetPlayersLastRiddenHorse, sets the horse last ridden by the player, 0, 1, kParams_OneObjectRef);
DEFINE_COMMAND(ClearPlayersLastRiddenHorse, marks the player as having no last ridden horse,
			   0, 0, NULL);

CommandInfo kCommandInfo_SetFlyCameraSpeedMult =
{
	"SetFlyCameraSpeedMult",
	"SFCSM",
	0,
	"changes the speed multiplier of the fly camera",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_SetFlyCameraSpeedMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

