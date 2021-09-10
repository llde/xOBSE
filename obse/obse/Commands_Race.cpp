#include "Commands_Race.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "ArrayVar.h"
#include "ScriptUtils.h"
#include "Hooks_Gameplay.h"

enum {
	kRace_Attribute = 0,
	kRace_BonusSkill,
	kRace_IsBonusSkill,
	kRace_NthBonusSkill,
	kRace_NthSpell,
};

static bool GetRaceValue_Execute(COMMAND_ARGS, UInt32 whichVal)
{
	*result = 0;
	UInt32 intVal = 0;
	TESRace* race = 0;
	UInt32 isFemaleArg = 0; // male
	ExtractArgs(PASS_EXTRACT_ARGS, &intVal, &race, &isFemaleArg);

	bool bIsFemale = (isFemaleArg == 0) ? false : true;
	if (!race) {
		if (!thisObj) return true;
		TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc) return true;
		race = npc->race.race;
		if (!race) return true;
		bIsFemale = npc->actorBaseData.IsFemale();
	}

	switch (whichVal) {
		case kRace_Attribute: 
			{
				if (intVal > kActorVal_Luck) return true;
				*result = race->GetBaseAttribute(intVal, bIsFemale);
				break;
			}
		case kRace_BonusSkill: 
			{
				if (!IsSkill(intVal)) return true;
				*result = race->GetSkillBonus(intVal);
				break;
			}
		case kRace_IsBonusSkill:
			{
				if (!IsSkill(intVal)) return true;
				*result = race->IsBonusSkill(intVal);
				break;
			}
		case kRace_NthBonusSkill:
			{
				if (intVal > 6) return true;
				*result = race->GetNthBonusSkill(intVal);
				break;
			}
		case kRace_NthSpell:
			{
				UInt32* refResult = (UInt32*)result;
				SpellListVisitor visitor(&race->spells.spellList);
				TESForm* form = visitor.GetNthInfo(intVal);
				SpellItem* spell = (SpellItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_SpellItem, 0);
				if (spell) {
					*refResult = spell->refID;
				}
				break;
			}

	}
	return true;
}

static bool Cmd_GetRaceAttribute_Execute(COMMAND_ARGS)
{
	return GetRaceValue_Execute(PASS_COMMAND_ARGS, kRace_Attribute);
}

static bool Cmd_GetRaceSkillBonus_Execute(COMMAND_ARGS)
{
	return GetRaceValue_Execute(PASS_COMMAND_ARGS, kRace_BonusSkill);
}

static bool Cmd_IsRaceBonusSkill_Execute(COMMAND_ARGS)
{
	return GetRaceValue_Execute(PASS_COMMAND_ARGS, kRace_IsBonusSkill);
}

static bool Cmd_GetNthRaceBonusSkill_Execute(COMMAND_ARGS)
{
	return GetRaceValue_Execute(PASS_COMMAND_ARGS, kRace_NthBonusSkill);
}

static bool Cmd_GetRaceSpellCount_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESRace* race = 0;
	ExtractArgs(PASS_EXTRACT_ARGS, &race);
	if (!race) {
		if (!thisObj) return true;
		TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc) return true;
		race = npc->race.race;
		if (!race) return true;
	}
	SpellListVisitor visitor(&race->spells.spellList);
	*result = visitor.Count();
	return true;
}

static bool Cmd_GetNthRaceSpell_Execute(COMMAND_ARGS)
{
	return GetRaceValue_Execute(PASS_COMMAND_ARGS, kRace_NthSpell);
}

static bool Cmd_SetRaceAlias_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	TESRace* alias = NULL;
	UInt32 bEnableAlias = 1;		// default 'true' if not specified in script

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &race, &alias, &bEnableAlias))
		return true;
	else if (!race || !alias)
		return true;

	SetRaceAlias(race, alias, bEnableAlias ? true : false);
	
	return true;
}

static bool Cmd_SetRaceVoice_Execute(COMMAND_ARGS)
{
	UInt32 gender = 2;	//0=male, 1=female, 2=both
	TESRace* thisRace = NULL;
	TESRace* voiceRace = NULL;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &thisRace, &voiceRace, &gender))
		return true;
	else if (!thisRace || !voiceRace)
		return true;

	if (gender != 1)	//male or both
		thisRace->voiceRaces[0] = voiceRace;

	if (gender != 0)	//female or both
		thisRace->voiceRaces[1] = voiceRace;

	return true;
}

static bool Cmd_GetRaceVoice_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 gender = -1;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race, &gender) && race && gender >= 0 && gender <= 1) {
		TESRace* voiceRace = race->voiceRaces[gender];
		// if no voice race defined, return this race
		*refResult = voiceRace ? voiceRace->refID : race->refID;
	}

	return true;
}

static bool Cmd_SetRacePlayable_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 bPlayable = 1;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &race, &bPlayable))
		return true;
	else if (!race)
		return true;

	race->isPlayable = (bPlayable) ? 1 : 0;

	return true;
}


static bool Cmd_IsRacePlayable_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &race) && race)
		*result = race->isPlayable ? 1 : 0;
	return true;
}

static bool Cmd_GetRaceReaction_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESRace* pFromRace = NULL;
	TESRace* pToRace = NULL;
	bool bExtracted = ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &pToRace, &pFromRace);
	if (!pToRace) return true;
	if (!pFromRace) {
		if (!thisObj) return true;
		TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc) return true;
		pFromRace = npc->race.race;
		if (!pFromRace) return true;
	}

	SInt32 reaction = pFromRace->reaction.ReactionFor(pToRace);
	*result = reaction;
	return true;
}

static bool Cmd_GetRaceScale_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 bFemale = 0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race, &bFemale) && race)
		*result = bFemale ? race->femaleScale : race->maleScale;

	return true;
}

static bool Cmd_GetRaceWeight_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 bFemale = 0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race, &bFemale) && race)
		*result = bFemale ? race->femaleWeight : race->maleWeight;

	return true;
}

static bool Cmd_SetRaceScale_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 bFemale = 0;
	float scale = 0.0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race, &bFemale, &scale) && race) {
		if (bFemale)
			race->femaleScale = scale;
		else
			race->maleScale = scale;
		*result = 1.0;
	}

	return true;
}

static bool Cmd_SetRaceWeight_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 bFemale = 0;
	float weight = 0.0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race, &bFemale, &weight) && race) {
		if (bFemale)
			race->femaleWeight = weight;
		else
			race->maleWeight = weight;
		*result = 1.0;
	}

	return true;
}

static bool Cmd_GetRaceDefaultHair_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	UInt32 bFemale = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race, &bFemale) && race) {
		bFemale = bFemale ? 1 : 0;
		*refResult = race->defaultHair[bFemale]->refID;
		if (IsConsoleMode()) {
			Console_Print("GetRaceDefaultHair >> %s (%08X)", GetFullName(race->defaultHair[bFemale]), *refResult);
		}
	}

	return true;
}

static bool Cmd_GetRaceHairs_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race) && race) {
		double idx = 0.0;
		for (tList<TESHair>::Iterator iter = race->hairs.Begin(); !iter.End(); ++iter) {
			if (iter.Get()) {
				g_ArrayMap.SetElementFormID(arr, idx, iter.Get()->refID);
				idx += 1.0;
			}
		}
	}

	return true;
}

static bool Cmd_GetRaceEyes_Execute(COMMAND_ARGS)
{
	TESRace* race = NULL;
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &race) && race) {
		double idx = 0.0;
		for (tList<TESEyes>::Iterator iter = race->eyes.Begin(); !iter.End(); ++iter) {
			if (iter.Get()) {
				g_ArrayMap.SetElementFormID(arr, idx, iter.Get()->refID);
				idx += 1.0;
			}
		}
	}

	return true;
}

static TESRace* RaceFromForm(TESForm* baseForm, TESObjectREFR* thisObj)
{
	// this accepts a race, base NPC, or calling reference
	TESRace* race = NULL;

	if (!baseForm && thisObj)
		baseForm = thisObj->baseForm;

	if (baseForm) {
		race = OBLIVION_CAST(baseForm, TESForm, TESRace);
		if (!race) {
			TESNPC* npc = OBLIVION_CAST(baseForm, TESForm, TESNPC);
			if (npc) {
				race = npc->race.race;
			}
		}
	}

	return race;
}

static bool Cmd_HasTail_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = 0.0;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form)) {
		TESRace* race = RaceFromForm(form, thisObj);

		if (race && race->tails[0].nifPath.m_data) {
			*result = 1.0;
		}
	}

	if (IsConsoleMode()) {
		Console_Print("HasTail >> %.0f", *result);
	}

	return true;
}

static bool Cmd_GetTailModelPath_Execute(COMMAND_ARGS)
{
	UInt32 gender = 0;	// default male
	TESForm* form = NULL;	
	const char* tailPath = NULL;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &gender, &form) && gender <= 1) {
		TESRace* race = RaceFromForm(form, thisObj);
		if (race) {
			tailPath = race->tails[gender].nifPath.m_data;
		}
	}

	if (!tailPath)
		tailPath = "";

	AssignToStringVar(PASS_COMMAND_ARGS, tailPath);

	if (IsConsoleMode()) {
		Console_Print("GetTailModelPath >> %s", tailPath);
	}

	return true;
}

#endif

static ParamInfo kParams_GetRaceAttribute[3] =
{
	{	"attribute", kParamType_ActorValue, 0 },
	{	"race", kParamType_Race, 1 },
	{	"sex", kParamType_Sex, 1},
};

static ParamInfo kParams_GetRaceAttributeC[3] =
{
	{	"which", kParamType_Integer, 0 },
	{	"race", kParamType_Race, 1 },
	{	"sex", kParamType_Sex, 1},
};

CommandInfo kCommandInfo_GetRaceAttribute =
{
	"GetRaceAttribute",
	"",
	0,
	"returns the specified attibute for the race",
	0,
	2,
	kParams_GetRaceAttribute,
	HANDLER(Cmd_GetRaceAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetRaceAttributeC =
{
	"GetRaceAttributeC",
	"",
	0,
	"returns the specified attibute for the race",
	0,
	2,
	kParams_GetRaceAttributeC,
	HANDLER(Cmd_GetRaceAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetRaceValue[2] =
{
	{	"attribute", kParamType_ActorValue, 0 },
	{	"race", kParamType_Race, 1 },
};

static ParamInfo kParams_GetRaceValueC[2] =
{
	{	"attribute", kParamType_Integer, 0 },
	{	"race", kParamType_Race, 1 },
};

CommandInfo kCommandInfo_GetRaceSkillBonus =
{
	"GetRaceSkillBonus",
	"",
	0,
	"returns the specified skill bonus for the race",
	0,
	2,
	kParams_GetRaceValue,
	HANDLER(Cmd_GetRaceSkillBonus_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetRaceSkillBonusC =
{
	"GetRaceSkillBonusC",
	"",
	0,
	"returns the specified skill bonus for the race",
	0,
	2,
	kParams_GetRaceValueC,
	HANDLER(Cmd_GetRaceSkillBonus_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsRaceBonusSkill =
{
	"IsRaceBonusSkill",
	"",
	0,
	"returns whether the specified skill has a bonus for the race",
	0,
	2,
	kParams_GetRaceValue,
	HANDLER(Cmd_IsRaceBonusSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsRaceBonusSkillC =
{
	"IsRaceBonusSkillC",
	"",
	0,
	"returns whether the specified skill has a bonus for the race",
	0,
	2,
	kParams_GetRaceValueC,
	HANDLER(Cmd_IsRaceBonusSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthRaceBonusSkill =
{
	"GetNthRaceBonusSkill",
	"",
	0,
	"returns the nth bonus skill of the race",
	0,
	2,
	kParams_GetRaceValue,
	HANDLER(Cmd_GetNthRaceBonusSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneOptionalRace[1] =
{
	{	"race", kParamType_Race, 1 },
};

CommandInfo kCommandInfo_GetRaceSpellCount =
{
	"GetRaceSpellCount",
	"",
	0,
	"returns the number of spells provided by the race",
	0,
	1,
	kParams_OneOptionalRace,
	HANDLER(Cmd_GetRaceSpellCount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthRaceSpell =
{
	"GetNthRaceSpell",
	"",
	0,
	"returns the nth spell of the race",
	0,
	2,
	kParams_GetRaceValueC,
	HANDLER(Cmd_GetNthRaceSpell_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


static ParamInfo kParams_SetRaceAlias[3] =
{
	{	"race",			kParamType_Race,	0	},
	{	"alias",		kParamType_Race,	0	},
	{	"bEnableAlias",	kParamType_Integer,	1	},
};

DEFINE_COMMAND(SetRaceAlias,
			   defines an alias for a race,
			   0,
			   3,
			   kParams_SetRaceAlias);

DEFINE_COMMAND(SetRaceVoice,
			   sets the voice used by a race,
			   0,
			   3,
			   kParams_SetRaceAlias);

static ParamInfo kParams_SetRacePlayable[2] =
{
	{	"race",	kParamType_Race,	0	},
	{	"bool",	kParamType_Integer,	1	},
};

DEFINE_COMMAND(SetRacePlayable,
			   toggles a race as playable or unplayable,
			   0,
			   2,
			   kParams_SetRacePlayable);

DEFINE_COMMAND(IsRacePlayable, returns true if the race is flagged as playable, 0, 1, kParams_OneRace);

static ParamInfo kParams_GetRaceReaction[2] =
{
	{	"toRace",	kParamType_NPC,	0	},
	{	"fromRace",	kParamType_NPC,	1	},
};

DEFINE_COMMAND(GetRaceReaction, returns the reaction value from one race to another, 0, 1, kParams_GetRaceReaction);

DEFINE_COMMAND(GetRaceScale, returns the base scale of a race, 0, 2, kParams_SetRacePlayable);
DEFINE_COMMAND(GetRaceWeight, returns the weight of a race, 0, 2, kParams_SetRacePlayable);

DEFINE_COMMAND(GetRaceDefaultHair, returns the default hair for the race, 0, 2, kParams_SetRacePlayable);
DEFINE_COMMAND(GetRaceHairs, returns all hairs for the specified race, 0, 1, kParams_OneRace);
DEFINE_COMMAND(GetRaceEyes, returns all eyes for the specified race, 0, 1, kParams_OneRace);

static ParamInfo kParams_OneRace_OneInt[2] =
{
	{ "race",	kParamType_Race,	0	},
	{ "int",	kParamType_Integer,	0	},
};

DEFINE_COMMAND(GetRaceVoice, returns the voice race used for this race, 0, 2, kParams_OneRace_OneInt);

static ParamInfo kParams_OneRace_OneInt_OneFloat[3] =
{
	{ "race",	kParamType_Race,	0	},
	{ "bFemale",kParamType_Integer, 0	},
	{ "value",	kParamType_Float,	0	}
};

DEFINE_COMMAND(SetRaceScale, sets the scale of the race, 0, 3, kParams_OneRace_OneInt_OneFloat);
DEFINE_COMMAND(SetRaceWeight, sets the weight of the race, 0, 3, kParams_OneRace_OneInt_OneFloat);
DEFINE_COMMAND(HasTail, returns 1 if the race has a tail, 0, 1, kParams_OneOptionalRace);

static ParamInfo kParams_GetTailModelPath [2] =
{
	{ "gender",	kParamType_Sex,		1	},
	{ "race",	kParamType_Race,	1	}
};

DEFINE_COMMAND(GetTailModelPath, returns the tail nif path for the specified race and gender, 0, 2, kParams_GetTailModelPath);