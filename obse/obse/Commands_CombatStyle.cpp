#include "obse/Commands_CombatStyle.h"
#include "ParamInfos.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "Utilities.h"

#if OBLIVION

#include "GameAPI.h"

bool Cmd_GetCombatStyle_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32 *)result;
	TESCombatStyle* combatStyle = NULL;

	if (!thisObj)
		return true;

	else if (thisObj->baseForm->typeID == kFormType_Creature) {
		TESCreature* creature = (TESCreature *)OBLIVION_CAST(thisObj->baseForm, TESForm, TESCreature);
		if ( creature )
			combatStyle = creature->combatStyle;
	}
	else if (thisObj->baseForm->typeID == kFormType_NPC) {
		TESNPC* NPC = (TESNPC *)OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
		if ( NPC )
			combatStyle = NPC->combatStyle;
	}
	else
		return true;
	
	if ( combatStyle )
		*refResult = combatStyle->refID;

	return true;
}

enum {
	kCombatStyle_dodgeChance = 0,
	kCombatStyle_LRChance,
	kCombatStyle_pad1A,
	kCombatStyle_dodgeLRTimerMin,
	kCombatStyle_dodgeLRTimerMax,
	kCombatStyle_dodgeFWTimerMin,
	kCombatStyle_dodgeFWTimerMax,
	kCombatStyle_dodgeBackTimerMin,
	kCombatStyle_dodgeBackTimerMax,
	kCombatStyle_idleTimerMin,
	kCombatStyle_idleTimerMax,
	kCombatStyle_blockChance,
	kCombatStyle_attackChance,
	kCombatStyle_pad3E,
	kCombatStyle_staggerBonusToAttack,
	kCombatStyle_KOBonusToAttack,
	kCombatStyle_H2HBonusToAttack,
	kCombatStyle_powerAttackChance,
	kCombatStyle_pad4D,
	kCombatStyle_staggerBonusToPower,
	kCombatStyle_KOBonusToPower,
	kCombatStyle_attackChoiceChances,
	kCombatStyle_pad5D,
	kCombatStyle_holdTimerMin,
	kCombatStyle_holdTimerMax,
	kCombatStyle_unk68,
	kCombatStyle_acroDodgeChance,
	kCombatStyle_pad6A,
	kCombatStyle_rangeMultOptimal,
	kCombatStyle_rangeMultMax,
	kCombatStyle_switchDistMelee,
	kCombatStyle_switchDistRanged,
	kCombatStyle_buffStandoffDist,
	kCombatStyle_rangedStandoffDist,
	kCombatStyle_groupStandoffDist,
	kCombatStyle_rushAttackChance,
	kCombatStyle_pad89,
	kCombatStyle_rushAttackDistMult,
	kCombatStyle_unk90,

	// Extra Settings (dynamically allocated)
	kCombatStyle_DynamicMin,

	kCombatStyle_dodgeFatigueModMult = kCombatStyle_DynamicMin,
	kCombatStyle_dodgeFatigueModBase,
	kCombatStyle_encumSpeedModBase,
	kCombatStyle_encumSpeedModMult,
	kCombatStyle_dodgeUnderAttackMult,
	kCombatStyle_dodgeNotUnderAttackMult,
	kCombatStyle_dodgeBackUnderAttackMult,
	kCombatStyle_dodgeBackNotUnderAttackMult,
	kCombatStyle_dodgeFWAttackingMult,
	kCombatStyle_dodgeFWNotAttackingMult,
	kCombatStyle_blockSkillModMult,
	kCombatStyle_blockSkillModBase,
	kCombatStyle_blockUnderAttackMult,
	kCombatStyle_blockNotUnderAttackMult,
	kCombatStyle_attackSkillModMult,
	kCombatStyle_attackSkillModBase,
	kCombatStyle_attackUnderAttackMult,
	kCombatStyle_attackNotUnderAttackMult,
	kCombatStyle_attackDuringBlockMult,
	kCombatStyle_powerAttackFatigueModBase,
	kCombatStyle_powerAttackFatigueModMult,
};

SettingInfo* CombatSettingList[60];
bool CombatSettingListInit = 0;
SettingInfo* ReturnGameSetting(char* settingName)
{
	SettingInfo* setting = NULL;
	if ( GetGameSetting(settingName, &setting) )
		return setting;
	else
		return NULL;
}
bool InitCombatSettingList() {
	CombatSettingList[kCombatStyle_dodgeChance]					= ReturnGameSetting("iAIDefaultDodgeChance");
	CombatSettingList[kCombatStyle_LRChance]					= ReturnGameSetting("iAIDefaultLeftRightChance");
	CombatSettingList[kCombatStyle_dodgeLRTimerMin]				= ReturnGameSetting("fAIDefaultDodgeLeftRightMinTime");
	CombatSettingList[kCombatStyle_dodgeLRTimerMax]				= ReturnGameSetting("fAIDefaultDodgeLeftRightMaxTime");
	CombatSettingList[kCombatStyle_dodgeFWTimerMin]				= ReturnGameSetting("fAIDefaultDodgeForwardMinTime");
	CombatSettingList[kCombatStyle_dodgeFWTimerMax]				= ReturnGameSetting("fAIDefaultDodgeForwardMaxTime");
	CombatSettingList[kCombatStyle_dodgeBackTimerMin]			= ReturnGameSetting("fAIDefaultDodgeBackwardMinTime");
	CombatSettingList[kCombatStyle_dodgeBackTimerMax]			= ReturnGameSetting("fAIDefaultDodgeBackwardMaxTime");
	CombatSettingList[kCombatStyle_idleTimerMin]				= ReturnGameSetting("fAIDefaultIdleMinTime");
	CombatSettingList[kCombatStyle_idleTimerMax]				= ReturnGameSetting("fAIDefaultIdleMinTime");
	CombatSettingList[kCombatStyle_blockChance]					= ReturnGameSetting("iAIDefaultBlockChance");
	CombatSettingList[kCombatStyle_attackChance]				= ReturnGameSetting("iAIDefaultAttackChance");
	CombatSettingList[kCombatStyle_staggerBonusToAttack]		= ReturnGameSetting("fAIDefaultAttackDuringRecoilStaggerBonus");
	CombatSettingList[kCombatStyle_KOBonusToAttack]				= ReturnGameSetting("fAIDefaultAttackDuringUnconsciousBonus");
	CombatSettingList[kCombatStyle_H2HBonusToAttack]			= ReturnGameSetting("fAIDefaultAttackHandBonus");
	CombatSettingList[kCombatStyle_powerAttackChance]			= ReturnGameSetting("iADDefaultPowerAttackChance");
	CombatSettingList[kCombatStyle_staggerBonusToPower]			= ReturnGameSetting("fAIDefaultPowerAttackRecoilStaggerBonus");
	CombatSettingList[kCombatStyle_KOBonusToPower]				= ReturnGameSetting("fAIDefaultPowerAttackUnconsciousBonus");
	//CombatSettingList[kCombatStyle_attackChoiceChances]		= ReturnGameSetting(""); // what is this?
	CombatSettingList[kCombatStyle_holdTimerMin]				= ReturnGameSetting("fAIDefaultHoldMinTime");
	CombatSettingList[kCombatStyle_holdTimerMax]				= ReturnGameSetting("fAIDefaultHoldMaxTime");
	CombatSettingList[kCombatStyle_acroDodgeChance]				= ReturnGameSetting("iAIDefaultAcrobaticsDodgeChance");
	CombatSettingList[kCombatStyle_rangeMultOptimal]			= ReturnGameSetting("fAIDefaultOptimalRangeMult");
	CombatSettingList[kCombatStyle_rangeMultMax]				= ReturnGameSetting("fAIDefaultMaximumRangeMult");
	CombatSettingList[kCombatStyle_switchDistMelee]				= ReturnGameSetting("fAIDefaultSwitchToMeleeDistance");
	CombatSettingList[kCombatStyle_switchDistRanged]			= ReturnGameSetting("fAIDefaultSwitchToRangedDistance");
	CombatSettingList[kCombatStyle_buffStandoffDist]			= ReturnGameSetting("fAIDefaultBuffStandoffDistance");
	CombatSettingList[kCombatStyle_rangedStandoffDist]			= ReturnGameSetting("fAIDefaultRangedStandoffDistance");
	CombatSettingList[kCombatStyle_groupStandoffDist]			= ReturnGameSetting("fAIDefaultGroupStandoffDistance");
	CombatSettingList[kCombatStyle_rushAttackChance]			= ReturnGameSetting("iAIDefaultRushingAttackPercentChance");
	CombatSettingList[kCombatStyle_rushAttackDistMult]			= ReturnGameSetting("fAIDefaultRushingAttackDistanceMult");
	CombatSettingList[kCombatStyle_dodgeFatigueModMult]			= ReturnGameSetting("fAIDefaultDodgeFatigueMult");
	CombatSettingList[kCombatStyle_dodgeFatigueModBase]			= ReturnGameSetting("fAIDefaultDodgeFatigueBase");
	CombatSettingList[kCombatStyle_encumSpeedModBase]			= ReturnGameSetting("fAIDefaultDodgeSpeedBase");
	CombatSettingList[kCombatStyle_encumSpeedModMult]			= ReturnGameSetting("fAIDefaultDodgeSpeedMult");
	CombatSettingList[kCombatStyle_dodgeUnderAttackMult]		= ReturnGameSetting("fAIDefaultDodgeDuringAttackMult");
	CombatSettingList[kCombatStyle_dodgeNotUnderAttackMult]		= ReturnGameSetting("fAIDefaultDodgeNoAttackMult");
	CombatSettingList[kCombatStyle_dodgeBackUnderAttackMult]	= ReturnGameSetting("fAIDefaultDodgeBackDuringAttackMult");
	CombatSettingList[kCombatStyle_dodgeBackNotUnderAttackMult]	= ReturnGameSetting("fAIDefaultDodgeBackNoAttackMult");
	CombatSettingList[kCombatStyle_dodgeFWAttackingMult]		= ReturnGameSetting("fAIDefaultDodgeForwardWhileAttackMult");
	CombatSettingList[kCombatStyle_dodgeFWNotAttackingMult]		= ReturnGameSetting("fAIDefaultDodgeForwardNotAttackingMult");
	CombatSettingList[kCombatStyle_blockSkillModMult]			= ReturnGameSetting("fAIDefaultBlockSkillMult");
	CombatSettingList[kCombatStyle_blockSkillModBase]			= ReturnGameSetting("fAIDefaultBlockSkillBase");
	CombatSettingList[kCombatStyle_blockUnderAttackMult]		= ReturnGameSetting("fAIDefaultBlockDuringAttackMult");
	CombatSettingList[kCombatStyle_blockNotUnderAttackMult]		= ReturnGameSetting("fAIDefaultBlockNoAttackMult");
	CombatSettingList[kCombatStyle_attackSkillModMult]			= ReturnGameSetting("fAIDefaultAttackSkillBase");
	CombatSettingList[kCombatStyle_attackSkillModBase]			= ReturnGameSetting("fAIDefaultAttackSkillMult");
	CombatSettingList[kCombatStyle_attackUnderAttackMult]		= ReturnGameSetting("fAIDefaultAttackDuringAttackMult");
	CombatSettingList[kCombatStyle_attackNotUnderAttackMult]	= ReturnGameSetting("fAIDefaultAttackNoAttackMult");
	CombatSettingList[kCombatStyle_attackDuringBlockMult]		= ReturnGameSetting("fAIDefaultAttackDuringBlockMult");
	CombatSettingList[kCombatStyle_powerAttackFatigueModBase]	= ReturnGameSetting("fAIDefaultPowerAttackFatigueBase");
	CombatSettingList[kCombatStyle_powerAttackFatigueModMult]	= ReturnGameSetting("fAIDefaultPowerAttackFatigueMult");
	return true;
}

// tedium
#define SET_CS_EXTRA(field) style->extraSettings-> ## field ## = CombatSettingList[kCombatStyle_ ## field ## ]->f

static void InitExtraSettings(TESCombatStyle* style)
{
	if (style && !style->extraSettings) {
		style->extraSettings = (TESCombatStyle::ExtraSettings*)FormHeap_Allocate(sizeof(TESCombatStyle::ExtraSettings));

		SET_CS_EXTRA(dodgeFatigueModMult);
		SET_CS_EXTRA(dodgeFatigueModBase);
		SET_CS_EXTRA(encumSpeedModBase);
		SET_CS_EXTRA(encumSpeedModMult);
		SET_CS_EXTRA(dodgeUnderAttackMult);
		SET_CS_EXTRA(dodgeNotUnderAttackMult);
		SET_CS_EXTRA(dodgeBackUnderAttackMult);
		SET_CS_EXTRA(dodgeBackNotUnderAttackMult);
		SET_CS_EXTRA(dodgeFWAttackingMult);
		SET_CS_EXTRA(dodgeFWNotAttackingMult);
		SET_CS_EXTRA(blockSkillModMult);
		SET_CS_EXTRA(blockSkillModBase);
		SET_CS_EXTRA(blockUnderAttackMult);
		SET_CS_EXTRA(blockNotUnderAttackMult);
		SET_CS_EXTRA(attackSkillModMult);
		SET_CS_EXTRA(attackSkillModBase);
		SET_CS_EXTRA(attackUnderAttackMult);
		SET_CS_EXTRA(attackNotUnderAttackMult);
		SET_CS_EXTRA(attackDuringBlockMult);
		SET_CS_EXTRA(powerAttackFatigueModBase);
		SET_CS_EXTRA(powerAttackFatigueModMult);
	}
}

#undef SET_CS_EXTRA

static bool GetCombatStyleValue(TESCombatStyle* style, UInt32 whichValue, double* result)
{
	if ( !result || !style)
		return true;
	else if (whichValue >= kCombatStyle_DynamicMin && !style->extraSettings) {
		// extraSettings not defined, use default (they are all float)
		*result = CombatSettingList[whichValue]->f;
		return true;
	}

	*result = 0;

	if (style) {
		switch(whichValue) {
			case kCombatStyle_dodgeChance:
				*result = style->dodgeChance;
				break;

			case kCombatStyle_LRChance:
				*result = style->LRChance;
				break;

			case kCombatStyle_dodgeLRTimerMin:
				*result = style->dodgeLRTimerMin;
				break;

			case kCombatStyle_dodgeLRTimerMax:
				*result = style->dodgeLRTimerMax;
				break;

			case kCombatStyle_dodgeFWTimerMin:
				*result = style->dodgeFWTimerMin;
				break;

			case kCombatStyle_dodgeFWTimerMax:
				*result = style->dodgeFWTimerMax;
				break;

			case kCombatStyle_dodgeBackTimerMin:
				*result = style->dodgeBackTimerMin;
				break;

			case kCombatStyle_dodgeBackTimerMax:
				*result = style->dodgeBackTimerMax;
				break;

			case kCombatStyle_idleTimerMin:
				*result = style->idleTimerMin;
				break;

			case kCombatStyle_idleTimerMax:
				*result = style->idleTimerMax;
				break;

			case kCombatStyle_blockChance:
				*result = style->blockChance;
				break;

			case kCombatStyle_attackChance:
				*result = style->attackChance;
				break;

			case kCombatStyle_staggerBonusToAttack:
				*result = style->staggerBonusToAttack;
				break;

			case kCombatStyle_KOBonusToAttack:
				*result = style->KOBonusToAttack;
				break;

			case kCombatStyle_H2HBonusToAttack:
				*result = style->H2HBonusToAttack;
				break;

			case kCombatStyle_powerAttackChance:
				*result = style->powerAttackChance;
				break;

			case kCombatStyle_staggerBonusToPower:
				*result = style->staggerBonusToPower;
				break;

			case kCombatStyle_KOBonusToPower:
				*result = style->KOBonusToPower;
				break;

/*			case kCombatStyle_attackChoiceChances:
				*result = style->attackChoiceChances; what is this?
				break;*/

			case kCombatStyle_holdTimerMin:
				*result = style->holdTimerMin;
				break;

			case kCombatStyle_holdTimerMax:
				*result = style->holdTimerMax;
				break;

			case kCombatStyle_acroDodgeChance:
				*result = style->acroDodgeChance;
				break;

			case kCombatStyle_rangeMultOptimal:
				*result = style->rangeMultOptimal;
				break;

			case kCombatStyle_rangeMultMax:
				*result = style->rangeMultMax;
				break;

			case kCombatStyle_switchDistMelee:
				*result = style->switchDistMelee;
				break;

			case kCombatStyle_switchDistRanged:
				*result = style->switchDistRanged;
				break;

			case kCombatStyle_buffStandoffDist:
				*result = style->buffStandoffDist;
				break;

			case kCombatStyle_rangedStandoffDist:
				*result = style->rangedStandoffDist;
				break;

			case kCombatStyle_groupStandoffDist:
				*result = style->groupStandoffDist;
				break;

			case kCombatStyle_rushAttackChance:
				*result = style->rushAttackChance;
				break;

			case kCombatStyle_rushAttackDistMult:
				*result = style->rushAttackDistMult;
				break;

			case kCombatStyle_dodgeFatigueModMult:
				*result = style->extraSettings->dodgeFatigueModMult;
				break;

			case kCombatStyle_dodgeFatigueModBase:
				*result = style->extraSettings->dodgeFatigueModBase;
				break;

			case kCombatStyle_encumSpeedModBase:
				*result = style->extraSettings->encumSpeedModBase;
				break;

			case kCombatStyle_encumSpeedModMult:
				*result = style->extraSettings->encumSpeedModMult;
				break;

			case kCombatStyle_dodgeUnderAttackMult:
				*result = style->extraSettings->dodgeUnderAttackMult;
				break;

			case kCombatStyle_dodgeNotUnderAttackMult:
				*result = style->extraSettings->dodgeNotUnderAttackMult;
				break;

			case kCombatStyle_dodgeBackUnderAttackMult:
				*result = style->extraSettings->dodgeBackUnderAttackMult;
				break;

			case kCombatStyle_dodgeBackNotUnderAttackMult:
				*result = style->extraSettings->dodgeBackNotUnderAttackMult;
				break;

			case kCombatStyle_dodgeFWAttackingMult:
				*result = style->extraSettings->dodgeFWAttackingMult;
				break;

			case kCombatStyle_dodgeFWNotAttackingMult:
				*result = style->extraSettings->dodgeFWNotAttackingMult;
				break;

			case kCombatStyle_blockSkillModMult:
				*result = style->extraSettings->blockSkillModMult;
				break;

			case kCombatStyle_blockSkillModBase:
				*result = style->extraSettings->blockSkillModBase;
				break;

			case kCombatStyle_blockUnderAttackMult:
				*result = style->extraSettings->blockUnderAttackMult;
				break;

			case kCombatStyle_blockNotUnderAttackMult:
				*result = style->extraSettings->blockNotUnderAttackMult;
				break;

			case kCombatStyle_attackSkillModMult:
				*result = style->extraSettings->attackSkillModMult;
				break;

			case kCombatStyle_attackSkillModBase:
				*result = style->extraSettings->attackSkillModBase;
				break;

			case kCombatStyle_attackUnderAttackMult:
				*result = style->extraSettings->attackUnderAttackMult;
				break;

			case kCombatStyle_attackNotUnderAttackMult:
				*result = style->extraSettings->attackNotUnderAttackMult;
				break;

			case kCombatStyle_attackDuringBlockMult:
				*result = style->extraSettings->attackDuringBlockMult;
				break;

			case kCombatStyle_powerAttackFatigueModBase:
				*result = style->extraSettings->powerAttackFatigueModBase;
				break;

			case kCombatStyle_powerAttackFatigueModMult:
				*result = style->extraSettings->powerAttackFatigueModMult;
				break;
		}
	}
	else { // default style
		if (!CombatSettingListInit)
			CombatSettingListInit = InitCombatSettingList();
		SettingInfo* defSet = CombatSettingList[whichValue];
		if (defSet)
		{
			switch (defSet->Type()) {
				case SettingInfo::kSetting_Bool:
				case SettingInfo::kSetting_Integer:
					*result = defSet->i;
					break;

				case SettingInfo::kSetting_Float:
					*result = defSet->f;
					break;

				case SettingInfo::kSetting_Unsigned:
					*result = defSet->u;
					break;

				// String settings not used for Combat Settings; good thing, too
			}
		}
		else
			*result = 0.0;
	}
	return true;
}

static bool GetCombatStyleValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	TESCombatStyle* style = NULL;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &style)) return true;
	return GetCombatStyleValue(style, whichValue, result);
}

static bool Cmd_GetCombatStyleDodgeChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeChance);
}

static bool Cmd_GetCombatStyleDodgeLRChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_LRChance);
}

static bool Cmd_GetCombatStyleDodgeLRTimerMin_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeLRTimerMin);
}

static bool Cmd_GetCombatStyleDodgeLRTimerMax_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeLRTimerMax);
}

static bool Cmd_GetCombatStyleDodgeFWTimerMin_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWTimerMin);
}

static bool Cmd_GetCombatStyleDodgeFWTimerMax_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWTimerMax);
}

static bool Cmd_GetCombatStyleDodgeBackTimerMin_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackTimerMin);
}

static bool Cmd_GetCombatStyleDodgeBackTimerMax_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackTimerMax);
}

static bool Cmd_GetCombatStyleIdleTimerMin_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_idleTimerMin);
}

static bool Cmd_GetCombatStyleIdleTimerMax_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_idleTimerMax);
}

static bool Cmd_GetCombatStyleBlockChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockChance);
}

static bool Cmd_GetCombatStyleAttackChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackChance);
}

static bool Cmd_GetCombatStyleStaggerBonusToAttack_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_staggerBonusToAttack);
}

static bool Cmd_GetCombatStyleKOBonusToAttack_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_KOBonusToAttack);
}

static bool Cmd_GetCombatStyleH2HBonusToAttack_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_H2HBonusToAttack);
}

static bool Cmd_GetCombatStylePowerAttackChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_powerAttackChance);
}

static bool Cmd_GetCombatStyleStaggerBonusToPowerAttack_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_staggerBonusToPower);
}

static bool Cmd_GetCombatStyleKOBonusToPowerAttack_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_KOBonusToPower);
}
/*
static bool Cmd_GetCombatStyleAttackChoiceChances_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackChoiceChances);
}
*/
static bool Cmd_GetCombatStyleHoldTimerMin_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_holdTimerMin);
}

static bool Cmd_GetCombatStyleHoldTimerMax_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_holdTimerMax);
}

static bool Cmd_GetCombatStyleAcrobaticsDodgeChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_acroDodgeChance);
}

static bool Cmd_GetCombatStyleRangeOptimalMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rangeMultOptimal);
}

static bool Cmd_GetCombatStyleRangeMaxMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rangeMultMax);
}

static bool Cmd_GetCombatStyleSwitchDistMelee_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_switchDistMelee);
}

static bool Cmd_GetCombatStyleSwitchDistRanged_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_switchDistRanged);
}

static bool Cmd_GetCombatStyleBuffStandoffDist_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_buffStandoffDist);
}

static bool Cmd_GetCombatStyleRangedStandoffDist_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rangedStandoffDist);
}

static bool Cmd_GetCombatStyleGroupStandoffDist_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_groupStandoffDist);
}

static bool Cmd_GetCombatStyleRushAttackChance_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rushAttackChance);
}

static bool Cmd_GetCombatStyleRushAttackDistMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rushAttackDistMult);
}

static bool Cmd_GetCombatStyleDodgeFatigueModMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFatigueModMult);
}

static bool Cmd_GetCombatStyleDodgeFatigueModBase_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFatigueModBase);
}

static bool Cmd_GetCombatStyleEncumberedSpeedModBase_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_encumSpeedModBase);
}

static bool Cmd_GetCombatStyleEncumberedSpeedModMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_encumSpeedModMult);
}

static bool Cmd_GetCombatStyleDodgeUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeUnderAttackMult);
}

static bool Cmd_GetCombatStyleDodgeNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeNotUnderAttackMult);
}

static bool Cmd_GetCombatStyleDodgeBackUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackUnderAttackMult);
}

static bool Cmd_GetCombatStyleDodgeBackNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackNotUnderAttackMult);
}

static bool Cmd_GetCombatStyleDodgeFWAttackingMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWAttackingMult);
}

static bool Cmd_GetCombatStyleDodgeFWNotAttackingMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWNotAttackingMult);
}

static bool Cmd_GetCombatStyleBlockSkillModMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockSkillModMult);
}

static bool Cmd_GetCombatStyleBlockSkillModBase_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockSkillModBase);
}

static bool Cmd_GetCombatStyleBlockUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockUnderAttackMult);
}

static bool Cmd_GetCombatStyleBlockNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockNotUnderAttackMult);
}

static bool Cmd_GetCombatStyleAttackSkillModMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackSkillModMult);
}

static bool Cmd_GetCombatStyleAttackSkillModBase_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackSkillModBase);
}

static bool Cmd_GetCombatStyleAttackUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackUnderAttackMult);
}

static bool Cmd_GetCombatStyleAttackNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackNotUnderAttackMult);
}

static bool Cmd_GetCombatStyleAttackDuringBlockMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackDuringBlockMult);
}

static bool Cmd_GetCombatStylePowerAttackFatigueModBase_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_powerAttackFatigueModBase);
}

static bool Cmd_GetCombatStylePowerAttackFatigueModMult_Execute(COMMAND_ARGS)
{
	return GetCombatStyleValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_powerAttackFatigueModMult);
}

static bool SetCombatStyleIntValue(TESCombatStyle* style, UInt32 whichValue, int newValue, double* result)
{
	*result = 0;

	if (style) {
		// we may need to allocate extraSettings if none already defined for this style
		if (whichValue >= kCombatStyle_DynamicMin && !style->extraSettings) {
			InitExtraSettings(style);
		}

		switch(whichValue) {
			case kCombatStyle_dodgeChance:
				style->dodgeChance = newValue;
				*result = 1;
				break;

			case kCombatStyle_LRChance:
				style->LRChance = newValue;
				*result = 1;
				break;

			case kCombatStyle_blockChance:
				style->blockChance = newValue;
				*result = 1;
				break;

			case kCombatStyle_attackChance:
				style->attackChance = newValue;
				*result = 1;
				break;

			case kCombatStyle_powerAttackChance:
				style->powerAttackChance = newValue;
				*result = 1;
				break;

	/*		case kCombatStyle_attackChoiceChances
				style->attackChoiceChances = newValue;
				*result = 1;
				break;*/

			case kCombatStyle_acroDodgeChance:
				style->acroDodgeChance = newValue;
				*result = 1;
				break;

			case kCombatStyle_rushAttackChance:
				style->rushAttackChance = newValue;
				*result = 1;
				break;
		}
	}
	return true;
}

static bool SetCombatStyleIntValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	TESCombatStyle* style = NULL;
	int newValue = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &newValue, &style)) return true;
	return SetCombatStyleIntValue(style, whichValue, newValue, result);
}

static bool SetCombatStyleFloatValue(TESCombatStyle* style, UInt32 whichValue, float newValue, double* result)
{
	*result = 0;

	if (style) {
		// we may need to allocate extraSettings if none already defined for this style
		if (whichValue >= kCombatStyle_DynamicMin && !style->extraSettings) {
			InitExtraSettings(style);
		}

		switch(whichValue) {
			case kCombatStyle_dodgeLRTimerMin:
				style->dodgeLRTimerMin = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeLRTimerMax:
				style->dodgeLRTimerMax = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeFWTimerMin:
				style->dodgeFWTimerMin = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeFWTimerMax:
				style->dodgeFWTimerMax = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeBackTimerMin:
				style->dodgeBackTimerMin = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeBackTimerMax:
				style->dodgeBackTimerMax = newValue;
				*result = 1;
				break;

			case kCombatStyle_idleTimerMin:
				style->idleTimerMin = newValue;
				*result = 1;
				break;

			case kCombatStyle_idleTimerMax:
				style->idleTimerMax = newValue;
				*result = 1;
				break;

			case kCombatStyle_staggerBonusToAttack:
				style->staggerBonusToAttack = newValue;
				*result = 1;
				break;

			case kCombatStyle_KOBonusToAttack:
				style->KOBonusToAttack = newValue;
				*result = 1;
				break;

			case kCombatStyle_staggerBonusToPower:
				style->staggerBonusToPower = newValue;
				*result = 1;
				break;

			case kCombatStyle_KOBonusToPower:
				style->KOBonusToPower = newValue;
				*result = 1;
				break;

			case kCombatStyle_holdTimerMin:
				style->holdTimerMin = newValue;
				*result = 1;
				break;

			case kCombatStyle_rangeMultOptimal:
				style->rangeMultOptimal = newValue;
				*result = 1;
				break;

			case kCombatStyle_rangeMultMax:
				style->rangeMultMax = newValue;
				*result = 1;
				break;

			case kCombatStyle_switchDistMelee:
				style->switchDistMelee = newValue;
				*result = 1;
				break;

			case kCombatStyle_switchDistRanged:
				style->switchDistRanged = newValue;
				*result = 1;
				break;

			case kCombatStyle_buffStandoffDist:
				style->buffStandoffDist = newValue;
				*result = 1;
				break;

			case kCombatStyle_rangedStandoffDist:
				style->rangedStandoffDist = newValue;
				*result = 1;
				break;

			case kCombatStyle_rushAttackDistMult:
				style->rushAttackDistMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeFatigueModMult:
				style->extraSettings->dodgeFatigueModMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeFatigueModBase:
				style->extraSettings->dodgeFatigueModBase = newValue;
				*result = 1;
				break;

			case kCombatStyle_encumSpeedModBase:
				style->extraSettings->encumSpeedModBase = newValue;
				*result = 1;
				break;

			case kCombatStyle_encumSpeedModMult:
				style->extraSettings->encumSpeedModMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeUnderAttackMult:
				style->extraSettings->dodgeUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeNotUnderAttackMult:
				style->extraSettings->dodgeNotUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeBackUnderAttackMult:
				style->extraSettings->dodgeBackUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeBackNotUnderAttackMult:
				style->extraSettings->dodgeBackNotUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeFWAttackingMult:
				style->extraSettings->dodgeFWAttackingMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_dodgeFWNotAttackingMult:
				style->extraSettings->dodgeFWNotAttackingMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_blockSkillModMult:
				style->extraSettings->blockSkillModMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_blockSkillModBase:
				style->extraSettings->blockSkillModBase = newValue;
				*result = 1;
				break;

			case kCombatStyle_blockUnderAttackMult:
				style->extraSettings->blockUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_blockNotUnderAttackMult:
				style->extraSettings->blockNotUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_attackSkillModMult:
				style->extraSettings->attackSkillModMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_attackSkillModBase:
				style->extraSettings->attackSkillModBase = newValue;
				*result = 1;
				break;

			case kCombatStyle_attackUnderAttackMult:
				style->extraSettings->attackUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_attackNotUnderAttackMult:
				style->extraSettings->attackNotUnderAttackMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_attackDuringBlockMult:
				style->extraSettings->attackDuringBlockMult = newValue;
				*result = 1;
				break;

			case kCombatStyle_powerAttackFatigueModBase:
				style->extraSettings->powerAttackFatigueModBase = newValue;
				*result = 1;
				break;

			case kCombatStyle_powerAttackFatigueModMult:
				style->extraSettings->powerAttackFatigueModMult = newValue;
				*result = 1;
				break;
		}
	}
	return true;
}

static bool SetCombatStyleFloatValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	TESCombatStyle* style = NULL;
	float newValue = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &newValue, &style)) return true;
	return SetCombatStyleFloatValue(style, whichValue, newValue, result);
}

static bool Cmd_SetCombatStyleDodgeChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeChance);
}

static bool Cmd_SetCombatStyleDodgeLRChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_LRChance);
}

static bool Cmd_SetCombatStyleDodgeLRTimerMin_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeLRTimerMin);
}

static bool Cmd_SetCombatStyleDodgeLRTimerMax_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeLRTimerMax);
}

static bool Cmd_SetCombatStyleDodgeFWTimerMin_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWTimerMin);
}

static bool Cmd_SetCombatStyleDodgeFWTimerMax_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWTimerMax);
}

static bool Cmd_SetCombatStyleDodgeBackTimerMin_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackTimerMin);
}

static bool Cmd_SetCombatStyleDodgeBackTimerMax_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackTimerMax);
}

static bool Cmd_SetCombatStyleIdleTimerMin_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_idleTimerMin);
}

static bool Cmd_SetCombatStyleIdleTimerMax_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_idleTimerMax);
}

static bool Cmd_SetCombatStyleBlockChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockChance);
}

static bool Cmd_SetCombatStyleAttackChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackChance);
}

static bool Cmd_SetCombatStyleStaggerBonusToAttack_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_staggerBonusToAttack);
}

static bool Cmd_SetCombatStyleKOBonusToAttack_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_KOBonusToAttack);
}

static bool Cmd_SetCombatStyleH2HBonusToAttack_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_H2HBonusToAttack);
}

static bool Cmd_SetCombatStylePowerAttackChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_powerAttackChance);
}

static bool Cmd_SetCombatStyleStaggerBonusToPowerAttack_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_staggerBonusToPower);
}

static bool Cmd_SetCombatStyleKOBonusToPowerAttack_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_KOBonusToPower);
}
/*
static bool Cmd_SetCombatStyleAttackChoiceChances_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackChoiceChances);
}
*/
static bool Cmd_SetCombatStyleHoldTimerMin_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_holdTimerMin);
}

static bool Cmd_SetCombatStyleHoldTimerMax_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_holdTimerMax);
}

static bool Cmd_SetCombatStyleAcrobaticsDodgeChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_acroDodgeChance);
}

static bool Cmd_SetCombatStyleRangeOptimalMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rangeMultOptimal);
}

static bool Cmd_SetCombatStyleRangeMaxMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rangeMultMax);
}

static bool Cmd_SetCombatStyleSwitchDistMelee_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_switchDistMelee);
}

static bool Cmd_SetCombatStyleSwitchDistRanged_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_switchDistRanged);
}

static bool Cmd_SetCombatStyleBuffStandoffDist_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_buffStandoffDist);
}

static bool Cmd_SetCombatStyleRangedStandoffDist_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rangedStandoffDist);
}

static bool Cmd_SetCombatStyleGroupStandoffDist_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_groupStandoffDist);
}

static bool Cmd_SetCombatStyleRushAttackChance_Execute(COMMAND_ARGS)
{
	return SetCombatStyleIntValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rushAttackChance);
}

static bool Cmd_SetCombatStyleRushAttackDistMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_rushAttackDistMult);
}

static bool Cmd_SetCombatStyleDodgeFatigueModMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFatigueModMult);
}

static bool Cmd_SetCombatStyleDodgeFatigueModBase_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFatigueModBase);
}

static bool Cmd_SetCombatStyleEncumberedSpeedModBase_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_encumSpeedModBase);
}

static bool Cmd_SetCombatStyleEncumberedSpeedModMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_encumSpeedModMult);
}

static bool Cmd_SetCombatStyleDodgeUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeUnderAttackMult);
}

static bool Cmd_SetCombatStyleDodgeNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeNotUnderAttackMult);
}

static bool Cmd_SetCombatStyleDodgeBackUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackUnderAttackMult);
}

static bool Cmd_SetCombatStyleDodgeBackNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeBackNotUnderAttackMult);
}

static bool Cmd_SetCombatStyleDodgeFWAttackingMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWAttackingMult);
}

static bool Cmd_SetCombatStyleDodgeFWNotAttackingMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_dodgeFWNotAttackingMult);
}

static bool Cmd_SetCombatStyleBlockSkillModMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockSkillModMult);
}

static bool Cmd_SetCombatStyleBlockSkillModBase_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockSkillModBase);
}

static bool Cmd_SetCombatStyleBlockUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockUnderAttackMult);
}

static bool Cmd_SetCombatStyleBlockNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_blockNotUnderAttackMult);
}

static bool Cmd_SetCombatStyleAttackSkillModMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackSkillModMult);
}

static bool Cmd_SetCombatStyleAttackSkillModBase_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackSkillModBase);
}

static bool Cmd_SetCombatStyleAttackUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackUnderAttackMult);
}

static bool Cmd_SetCombatStyleAttackNotUnderAttackMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackNotUnderAttackMult);
}

static bool Cmd_SetCombatStyleAttackDuringBlockMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_attackDuringBlockMult);
}

static bool Cmd_SetCombatStylePowerAttackFatigueModBase_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_powerAttackFatigueModBase);
}

static bool Cmd_SetCombatStylePowerAttackFatigueModMult_Execute(COMMAND_ARGS)
{
	return SetCombatStyleFloatValue_Execute(PASS_COMMAND_ARGS, kCombatStyle_powerAttackFatigueModMult);
}

static bool GetSetCombatStyleFlag(COMMAND_ARGS, UInt8 flag, bool bSet)
{
	TESCombatStyle* style = NULL;
	UInt32 newVal = 1;
	*result = 0;

	bool bExtracted = false;
	if (bSet)
		bExtracted = ExtractArgs(PASS_EXTRACT_ARGS, &newVal, &style);
	else
		bExtracted = ExtractArgs(PASS_EXTRACT_ARGS, &style);

	if (bExtracted && style)
	{
		if (bSet)
		{
			if (newVal)
				style->styleFlags |= flag;
			else
				style->styleFlags &= ~flag;
		}
		else
			*result = (style->styleFlags & flag) ? 1 : 0;
	}

	return true;
}

#define DEFINE_CS_FLAG_FUNC(x) \
	static bool Cmd_GetCombatStyle ## x ## _Execute(COMMAND_ARGS) \
{ \
	return GetSetCombatStyleFlag(PASS_COMMAND_ARGS, TESCombatStyle::kFlag_ ## x, false); \
} \
	static bool Cmd_SetCombatStyle ## x ## _Execute(COMMAND_ARGS) \
{ \
	return GetSetCombatStyleFlag(PASS_COMMAND_ARGS, TESCombatStyle::kFlag_ ## x, true); \
}

DEFINE_CS_FLAG_FUNC(IgnoreAlliesInArea)
DEFINE_CS_FLAG_FUNC(WillYield)
DEFINE_CS_FLAG_FUNC(RejectsYields)
DEFINE_CS_FLAG_FUNC(FleeingDisabled)
DEFINE_CS_FLAG_FUNC(PrefersRanged)
DEFINE_CS_FLAG_FUNC(MeleeAlertOK)


#ifdef DEBUG 
bool Cmd_DumpCombatStyle_Execute(COMMAND_ARGS)
{
	*result = 1;

	TESForm* style1 = NULL;
	style1 = LookupFormByID(0x00067196);
	TESForm* style2 = NULL;
	style2 = LookupFormByID(0x00041056);

	if ( style1 )
	    DumpClass(style1, 0x98 / 4);                    // sizeof(TESCombatStyle) is 0x98 bytes
	if ( style2 )
	    DumpClass(style2, 0x98 / 4);
    return true;
}
#endif

#endif

CommandInfo kCommandInfo_GetCombatStyle = 
{
	"GetCombatStyle",
	"",
	0,
	"returns the Combat Style of the actor",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetCombatStyle_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeChance = 
{
	"GetCombatStyleDodgeChance",
	"",
	0,
	"returns the combat style's Dodge Chance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeLRChance = 
{
	"GetCombatStyleDodgeLRChance",
	"",
	0,
	"returns the combat style's Left/Right Dodge Chance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeLRChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeLRTimerMin = 
{
	"GetCombatStyleDodgeLRTimerMin",
	"",
	0,
	"returns the combat style's Left/Right Dodge Timer Minimum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeLRTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeLRTimerMax = 
{
	"GetCombatStyleDodgeLRTimerMax",
	"",
	0,
	"returns the combat style's Left/Right Dodge Timer Maximum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeLRTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeFWTimerMin = 
{
	"GetCombatStyleDodgeFWTimerMin",
	"",
	0,
	"returns the combat style's Forward Dodge Timer Minimum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeFWTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeFWTimerMax = 
{
	"GetCombatStyleDodgeFWTimerMax",
	"",
	0,
	"returns the combat style's Forward Dodge Timer Maximum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeFWTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeBackTimerMin = 
{
	"GetCombatStyleDodgeBackTimerMin",
	"",
	0,
	"returns the combat style's Backward Dodge Timer Minimum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeBackTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeBackTimerMax = 
{
	"GetCombatStyleDodgeBackTimerMax",
	"",
	0,
	"returns the combat style's Backward Dodge Timer Maximum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeBackTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleIdleTimerMin = 
{
	"GetCombatStyleIdleTimerMin",
	"",
	0,
	"returns the combat style's Minimum Idle Timer",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleIdleTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleIdleTimerMax = 
{
	"GetCombatStyleIdleTimerMax",
	"",
	0,
	"returns the combat style's Maximum Idle Timer",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleIdleTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleBlockChance = 
{
	"GetCombatStyleBlockChance",
	"",
	0,
	"returns the combat style's Block Chance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleBlockChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAttackChance = 
{
	"GetCombatStyleAttackChance",
	"",
	0,
	"returns the combat style's Attack Chance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleStaggerBonusToAttack = 
{
	"GetCombatStyleStaggerBonusToAttack",
	"",
	0,
	"returns the combat style's Stagger/Recoil Bonus to Attack",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleStaggerBonusToAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleKOBonusToAttack = 
{
	"GetCombatStyleKOBonusToAttack",
	"",
	0,
	"returns the combat style's Unconscious Bonus to Attack",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleKOBonusToAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleH2HBonusToAttack = 
{
	"GetCombatStyleH2HBonusToAttack",
	"",
	0,
	"returns the combat style's Hand-to-Hand Bonus to Attack",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleH2HBonusToAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStylePowerAttackChance = 
{
	"GetCombatStylePowerAttackChance",
	"",
	0,
	"returns the combat style's Power Attack Chance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStylePowerAttackChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleStaggerBonusToPowerAttack = 
{
	"GetCombatStyleStaggerBonusToPowerAttack",
	"",
	0,
	"returns the combat style's Stagger/Recoil Bonus to Power Attack",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleStaggerBonusToPowerAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleKOBonusToPowerAttack = 
{
	"GetCombatStyleKOBonusToPowerAttack",
	"",
	0,
	"returns the combat style's Unconscious Bonus to Power Attack",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleKOBonusToPowerAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
/*
CommandInfo kCommandInfo_GetCombatStyleAttackChoiceChances = 
{
	"GetCombatStyleAttackChoiceChances",
	"",
	0,
	"returns the combat style's ",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackChoiceChances_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
*/
CommandInfo kCommandInfo_GetCombatStyleHoldTimerMin = 
{
	"GetCombatStyleHoldTimerMin",
	"",
	0,
	"returns the combat style's Hold Timer Minimum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleHoldTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleHoldTimerMax = 
{
	"GetCombatStyleHoldTimerMax",
	"",
	0,
	"returns the combat style's Hold Timer Maximum",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleHoldTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAcrobaticsDodgeChance = 
{
	"GetCombatStyleAcrobaticsDodgeChance",
	"",
	0,
	"returns the combat style's Acrobatic Dodge Chance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAcrobaticsDodgeChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleRangeOptimalMult = 
{
	"GetCombatStyleRangeOptimalMult",
	"",
	0,
	"returns the combat style's Optimal Range Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleRangeOptimalMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleRangeMaxMult = 
{
	"GetCombatStyleRangeMaxMult",
	"",
	0,
	"returns the combat style's Maximum Range Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleRangeMaxMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleSwitchDistMelee = 
{
	"GetCombatStyleSwitchDistMelee",
	"",
	0,
	"returns the combat style's Melee Switch Distance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleSwitchDistMelee_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleSwitchDistRanged = 
{
	"GetCombatStyleSwitchDistRanged",
	"",
	0,
	"returns the combat style's Ranged Switch Distance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleSwitchDistRanged_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleBuffStandoffDist = 
{
	"GetCombatStyleBuffStandoffDist",
	"",
	0,
	"returns the combat style's Buff Standoff Distance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleBuffStandoffDist_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleRangedStandoffDist = 
{
	"GetCombatStyleRangedStandoffDist",
	"",
	0,
	"returns the combat style's Ranged Standoff Distance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleRangedStandoffDist_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleGroupStandoffDist = 
{
	"GetCombatStyleGroupStandoffDist",
	"",
	0,
	"returns the combat style's Group Standoff Distance",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleGroupStandoffDist_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleRushAttackChance = 
{
	"GetCombatStyleRushAttackChance",
	"",
	0,
	"returns the combat style's Chance to Rush Attack",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleRushAttackChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleRushAttackDistMult = 
{
	"GetCombatStyleRushAttackDistMult",
	"",
	0,
	"returns the combat style's Rush Attack Distance Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleRushAttackDistMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeFatigueModMult = 
{
	"GetCombatStyleDodgeFatigueModMult",
	"",
	0,
	"returns the combat style's Dodge Fatigue Mod Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeFatigueModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeFatigueModBase = 
{
	"GetCombatStyleDodgeFatigueModBase",
	"",
	0,
	"returns the combat style's Dodge Fatigue Mod Base",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeFatigueModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleEncumberedSpeedModBase = 
{
	"GetCombatStyleEncumberedSpeedModBase",
	"",
	0,
	"returns the combat style's Encumbered Speed Mod Base",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleEncumberedSpeedModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleEncumberedSpeedModMult = 
{
	"GetCombatStyleEncumberedSpeedModMult",
	"",
	0,
	"returns the combat style's Encumbered Speed Mod Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleEncumberedSpeedModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeUnderAttackMult = 
{
	"GetCombatStyleDodgeUnderAttackMult",
	"",
	0,
	"returns the combat style's Dodge while Under Attack Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeNotUnderAttackMult = 
{
	"GetCombatStyleDodgeNotUnderAttackMult",
	"",
	0,
	"returns the combat style's Dodge while Not Attacked Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeBackUnderAttackMult = 
{
	"GetCombatStyleDodgeBackUnderAttackMult",
	"",
	0,
	"returns the combat style's Dodge Backward while Under Attack Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeBackUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeBackNotUnderAttackMult = 
{
	"GetCombatStyleDodgeBackNotUnderAttackMult",
	"",
	0,
	"returns the combat style's Dodge Backward while Not Attacked Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeBackNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeFWAttackingMult = 
{
	"GetCombatStyleDodgeFWAttackingMult",
	"",
	0,
	"returns the combat style's Dodge Forward while Attacking Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeFWAttackingMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleDodgeFWNotAttackingMult = 
{
	"GetCombatStyleDodgeFWNotAttackingMult",
	"",
	0,
	"returns the combat style's Dodge Forward while Not Attacking Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleDodgeFWNotAttackingMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleBlockSkillModMult = 
{
	"GetCombatStyleBlockSkillModMult",
	"",
	0,
	"returns the combat style's Block Skill Mod Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleBlockSkillModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleBlockSkillModBase = 
{
	"GetCombatStyleBlockSkillModBase",
	"",
	0,
	"returns the combat style's Block Skill Mod Base",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleBlockSkillModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleBlockUnderAttackMult = 
{
	"GetCombatStyleBlockUnderAttackMult",
	"",
	0,
	"returns the combat style's Block while Under Attack Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleBlockUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleBlockNotUnderAttackMult = 
{
	"GetCombatStyleBlockNotUnderAttackMult",
	"",
	0,
	"returns the combat style's Block while Not Attacked Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleBlockNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAttackSkillModMult = 
{
	"GetCombatStyleAttackSkillModMult",
	"",
	0,
	"returns the combat style's Attack Skill Mod Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackSkillModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAttackSkillModBase = 
{
	"GetCombatStyleAttackSkillModBase",
	"",
	0,
	"returns the combat style's Attack Skill Mod Base",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackSkillModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAttackUnderAttackMult = 
{
	"GetCombatStyleAttackUnderAttackMult",
	"",
	0,
	"returns the combat style's Attack while Under Attack Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAttackNotUnderAttackMult = 
{
	"GetCombatStyleAttackNotUnderAttackMult",
	"",
	0,
	"returns the combat style's Attack while Not Attacked Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStyleAttackDuringBlockMult = 
{
	"GetCombatStyleAttackDuringBlockMult",
	"",
	0,
	"returns the combat style's Attack During Block Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStyleAttackDuringBlockMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStylePowerAttackFatigueModBase = 
{
	"GetCombatStylePowerAttackFatigueModBase",
	"",
	0,
	"returns the combat style's Power Attack Fatigue Mod Base",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStylePowerAttackFatigueModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCombatStylePowerAttackFatigueModMult = 
{
	"GetCombatStylePowerAttackFatigueModMult",
	"",
	0,
	"returns the combat style's Power Attack Fatigue Mod Mult",
	0,
	1,
	kParams_OneCombatStyle,
	HANDLER(Cmd_GetCombatStylePowerAttackFatigueModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneInt_OneCombatStyle[2] = 
{
	{	"int",			kParamType_Integer,		0	},
	{	"combat style",	kParamType_CombatStyle,	0	},
};

static ParamInfo kParams_OneFloat_OneCombatStyle[2] = 
{
	{	"float",		kParamType_Float,		0	},
	{	"combat style",	kParamType_CombatStyle,	0	},
};

CommandInfo kCommandInfo_SetCombatStyleDodgeChance = 
{
	"SetCombatStyleDodgeChance",
	"",
	0,
	"sets the combat style's Dodge Chance",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeLRChance = 
{
	"SetCombatStyleDodgeLRChance",
	"",
	0,
	"sets the combat style's Left/Right Dodge Chance",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeLRChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeLRTimerMin = 
{
	"SetCombatStyleDodgeLRTimerMin",
	"",
	0,
	"sets the combat style's Left/Right Dodge Timer Minimum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeLRTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeLRTimerMax = 
{
	"SetCombatStyleDodgeLRTimerMax",
	"",
	0,
	"sets the combat style's Left/Right Dodge Timer Maximum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeLRTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeFWTimerMin = 
{
	"SetCombatStyleDodgeFWTimerMin",
	"",
	0,
	"sets the combat style's Forward Dodge Timer Minimum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeFWTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeFWTimerMax = 
{
	"SetCombatStyleDodgeFWTimerMax",
	"",
	0,
	"sets the combat style's Forward Dodge Timer Maximum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeFWTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeBackTimerMin = 
{
	"SetCombatStyleDodgeBackTimerMin",
	"",
	0,
	"sets the combat style's Backward Dodge Timer Minimum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeBackTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeBackTimerMax = 
{
	"SetCombatStyleDodgeBackTimerMax",
	"",
	0,
	"sets the combat style's Backward Dodge Timer Maximum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeBackTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleIdleTimerMin = 
{
	"SetCombatStyleIdleTimerMin",
	"",
	0,
	"sets the combat style's Minimum Idle Timer",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleIdleTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleIdleTimerMax = 
{
	"SetCombatStyleIdleTimerMax",
	"",
	0,
	"sets the combat style's Maximum Idle Timer",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleIdleTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleBlockChance = 
{
	"SetCombatStyleBlockChance",
	"",
	0,
	"sets the combat style's Block Chance",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleBlockChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAttackChance = 
{
	"SetCombatStyleAttackChance",
	"",
	0,
	"sets the combat style's Attack Chance",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleStaggerBonusToAttack = 
{
	"SetCombatStyleStaggerBonusToAttack",
	"",
	0,
	"sets the combat style's Stagger/Recoil Bonus to Attack",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleStaggerBonusToAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleKOBonusToAttack = 
{
	"SetCombatStyleKOBonusToAttack",
	"",
	0,
	"sets the combat style's Unconscious Bonus to Attack",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleKOBonusToAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleH2HBonusToAttack = 
{
	"SetCombatStyleH2HBonusToAttack",
	"",
	0,
	"sets the combat style's Hand-to-Hand Bonus to Attack",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleH2HBonusToAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStylePowerAttackChance = 
{
	"SetCombatStylePowerAttackChance",
	"",
	0,
	"sets the combat style's Power Attack Chance",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStylePowerAttackChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleStaggerBonusToPowerAttack = 
{
	"SetCombatStyleStaggerBonusToPowerAttack",
	"",
	0,
	"sets the combat style's Stagger/Recoil Bonus to Power Attack",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleStaggerBonusToPowerAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleKOBonusToPowerAttack = 
{
	"SetCombatStyleKOBonusToPowerAttack",
	"",
	0,
	"sets the combat style's Unconscious Bonus to Power Attack",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleKOBonusToPowerAttack_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
/*
CommandInfo kCommandInfo_SetCombatStyleAttackChoiceChances = 
{
	"SetCombatStyleAttackChoiceChances",
	"",
	0,
	"sets the combat style's ",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackChoiceChances_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
*/
CommandInfo kCommandInfo_SetCombatStyleHoldTimerMin = 
{
	"SetCombatStyleHoldTimerMin",
	"",
	0,
	"sets the combat style's Hold Timer Minimum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleHoldTimerMin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleHoldTimerMax = 
{
	"SetCombatStyleHoldTimerMax",
	"",
	0,
	"sets the combat style's Hold Timer Maximum",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleHoldTimerMax_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAcrobaticsDodgeChance = 
{
	"SetCombatStyleAcrobaticsDodgeChance",
	"",
	0,
	"sets the combat style's Acrobatic Dodge Chance",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAcrobaticsDodgeChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleRangeOptimalMult = 
{
	"SetCombatStyleRangeOptimalMult",
	"",
	0,
	"sets the combat style's Optimal Range Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleRangeOptimalMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleRangeMaxMult = 
{
	"SetCombatStyleRangeMaxMult",
	"",
	0,
	"sets the combat style's Maximum Range Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleRangeMaxMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleSwitchDistMelee = 
{
	"SetCombatStyleSwitchDistMelee",
	"",
	0,
	"sets the combat style's Melee Switch Distance",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleSwitchDistMelee_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleSwitchDistRanged = 
{
	"SetCombatStyleSwitchDistRanged",
	"",
	0,
	"sets the combat style's Ranged Switch Distance",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleSwitchDistRanged_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleBuffStandoffDist = 
{
	"SetCombatStyleBuffStandoffDist",
	"",
	0,
	"sets the combat style's Buff Standoff Distance",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleBuffStandoffDist_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleRangedStandoffDist = 
{
	"SetCombatStyleRangedStandoffDist",
	"",
	0,
	"sets the combat style's Ranged Standoff Distance",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleRangedStandoffDist_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleGroupStandoffDist = 
{
	"SetCombatStyleGroupStandoffDist",
	"",
	0,
	"sets the combat style's Group Standoff Distance",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleGroupStandoffDist_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleRushAttackChance = 
{
	"SetCombatStyleRushAttackChance",
	"",
	0,
	"sets the combat style's Chance to Rush Attack",
	0,
	2,
	kParams_OneInt_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleRushAttackChance_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleRushAttackDistMult = 
{
	"SetCombatStyleRushAttackDistMult",
	"",
	0,
	"sets the combat style's Rush Attack Distance Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleRushAttackDistMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeFatigueModMult = 
{
	"SetCombatStyleDodgeFatigueModMult",
	"",
	0,
	"sets the combat style's Dodge Fatigue Mod Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeFatigueModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeFatigueModBase = 
{
	"SetCombatStyleDodgeFatigueModBase",
	"",
	0,
	"sets the combat style's Dodge Fatigue Mod Base",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeFatigueModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleEncumberedSpeedModBase = 
{
	"SetCombatStyleEncumberedSpeedModBase",
	"",
	0,
	"sets the combat style's Encumbered Speed Mod Base",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleEncumberedSpeedModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleEncumberedSpeedModMult = 
{
	"SetCombatStyleEncumberedSpeedModMult",
	"",
	0,
	"sets the combat style's Encumbered Speed Mod Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleEncumberedSpeedModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeUnderAttackMult = 
{
	"SetCombatStyleDodgeUnderAttackMult",
	"",
	0,
	"sets the combat style's Dodge while Under Attack Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeNotUnderAttackMult = 
{
	"SetCombatStyleDodgeNotUnderAttackMult",
	"",
	0,
	"sets the combat style's Dodge while Not Attacked Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeBackUnderAttackMult = 
{
	"SetCombatStyleDodgeBackUnderAttackMult",
	"",
	0,
	"sets the combat style's Dodge Backward while Under Attack Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeBackUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeBackNotUnderAttackMult = 
{
	"SetCombatStyleDodgeBackNotUnderAttackMult",
	"",
	0,
	"sets the combat style's Dodge Backward while Not Attacked Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeBackNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeFWAttackingMult = 
{
	"SetCombatStyleDodgeFWAttackingMult",
	"",
	0,
	"sets the combat style's Dodge Forward while Attacking Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeFWAttackingMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleDodgeFWNotAttackingMult = 
{
	"SetCombatStyleDodgeFWNotAttackingMult",
	"",
	0,
	"sets the combat style's Dodge Forward while Not Attacking Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleDodgeFWNotAttackingMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleBlockSkillModMult = 
{
	"SetCombatStyleBlockSkillModMult",
	"",
	0,
	"sets the combat style's Block Skill Mod Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleBlockSkillModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleBlockSkillModBase = 
{
	"SetCombatStyleBlockSkillModBase",
	"",
	0,
	"sets the combat style's Block Skill Mod Base",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleBlockSkillModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleBlockUnderAttackMult = 
{
	"SetCombatStyleBlockUnderAttackMult",
	"",
	0,
	"sets the combat style's Block while Under Attack Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleBlockUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleBlockNotUnderAttackMult = 
{
	"SetCombatStyleBlockNotUnderAttackMult",
	"",
	0,
	"sets the combat style's Block while Not Attacked Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleBlockNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAttackSkillModMult = 
{
	"SetCombatStyleAttackSkillModMult",
	"",
	0,
	"sets the combat style's Attack Skill Mod Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackSkillModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAttackSkillModBase = 
{
	"SetCombatStyleAttackSkillModBase",
	"",
	0,
	"sets the combat style's Attack Skill Mod Base",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackSkillModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAttackUnderAttackMult = 
{
	"SetCombatStyleAttackUnderAttackMult",
	"",
	0,
	"sets the combat style's Attack while Under Attack Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAttackNotUnderAttackMult = 
{
	"SetCombatStyleAttackNotUnderAttackMult",
	"",
	0,
	"sets the combat style's Attack while Not Attacked Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackNotUnderAttackMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStyleAttackDuringBlockMult = 
{
	"SetCombatStyleAttackDuringBlockMult",
	"",
	0,
	"sets the combat style's Attack During Block Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStyleAttackDuringBlockMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStylePowerAttackFatigueModBase = 
{
	"SetCombatStylePowerAttackFatigueModBase",
	"",
	0,
	"sets the combat style's Power Attack Fatigue Mod Base",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStylePowerAttackFatigueModBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetCombatStylePowerAttackFatigueModMult = 
{
	"SetCombatStylePowerAttackFatigueModMult",
	"",
	0,
	"sets the combat style's Power Attack Fatigue Mod Mult",
	0,
	2,
	kParams_OneFloat_OneCombatStyle,
	HANDLER(Cmd_SetCombatStylePowerAttackFatigueModMult_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneCombatStyle_OneInt[2] =
{
	{	"combat style",	kParamType_CombatStyle,	0	},
	{	"int",			kParamType_Integer,		0	},
};


DEFINE_COMMAND(GetCombatStyleIgnoreAlliesInArea, gets a combat style flag, 0, 1, kParams_OneCombatStyle);
DEFINE_COMMAND(GetCombatStyleWillYield, gets a combat style flag, 0, 1, kParams_OneCombatStyle);
DEFINE_COMMAND(GetCombatStyleRejectsYields, gets a combat style flag, 0, 1, kParams_OneCombatStyle);
DEFINE_COMMAND(GetCombatStyleFleeingDisabled, gets a combat style flag, 0, 1, kParams_OneCombatStyle);
DEFINE_COMMAND(GetCombatStylePrefersRanged, gets a combat style flag, 0, 1, kParams_OneCombatStyle);
DEFINE_COMMAND(GetCombatStyleMeleeAlertOK, gets a combat style flag, 0, 1, kParams_OneCombatStyle);

DEFINE_COMMAND(SetCombatStyleIgnoreAlliesInArea, sets a combat style flag, 0, 2, kParams_OneInt_OneCombatStyle);
DEFINE_COMMAND(SetCombatStyleWillYield, sets a combat style flag, 0, 2, kParams_OneInt_OneCombatStyle);
DEFINE_COMMAND(SetCombatStyleRejectsYields, sets a combat style flag, 0, 2, kParams_OneInt_OneCombatStyle);
DEFINE_COMMAND(SetCombatStyleFleeingDisabled, sets a combat style flag, 0, 2, kParams_OneInt_OneCombatStyle);
DEFINE_COMMAND(SetCombatStylePrefersRanged, sets a combat style flag, 0, 2, kParams_OneInt_OneCombatStyle);
DEFINE_COMMAND(SetCombatStyleMeleeAlertOK, sets a combat style flag, 0, 2, kParams_OneInt_OneCombatStyle);