#include "Commands_ActorValue.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "ArrayVar.h"
#include "ScriptUtils.h"

static bool Cmd_GetSkillUseIncrement_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 valSkill = 0;
	UInt32 whichUse = 0;
	ExtractArgs(PASS_EXTRACT_ARGS, &valSkill, &whichUse);
	if (!IsSkill(valSkill)) return true;

	TESSkill *skill = TESSkill::SkillForActorVal(valSkill);
	if (skill) {
		*result = (whichUse == 1) ? skill->useValue1 : skill->useValue0;
	}
	return true;
}

static bool Cmd_SetSkillUseIncrement_Execute(COMMAND_ARGS)
{
	*result = 0;
	float nuVal = 0.0;
	UInt32 valSkill = 0;
	UInt32 whichUse = 0;
	ExtractArgs(PASS_EXTRACT_ARGS, &nuVal, &valSkill, &whichUse);
	if (!IsSkill(valSkill)) return true;

	TESSkill *skill = TESSkill::SkillForActorVal(valSkill);
	if (skill) {
		if (whichUse == 1) {
			skill->useValue1 = nuVal;
		} else skill->useValue0 = nuVal;
	}
	return true;
}


static bool Cmd_GetSkillGoverningAttribute_Execute(COMMAND_ARGS)
{
	*result = -1;
	UInt32 skillCode = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &skillCode) && skillCode >= kActorVal_Armorer && skillCode <= kActorVal_Speechcraft)
	{
		TESSkill* skill = TESSkill::SkillForActorVal(skillCode);
		if (skill)
		{
			*result = skill->attribute;
		}
	}

	return true;
}

static bool Cmd_ActorValueToCode_Execute(COMMAND_ARGS)
{
	UInt32 actorVal = -1;
	ExtractArgs(PASS_EXTRACT_ARGS, &actorVal);
	*result = actorVal;
	return true;
}

static bool Cmd_SetSkillGoverningAttribute_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 skillCode = 0;
	UInt32 attrCode = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &skillCode, &attrCode))
	{
		if (skillCode >= kActorVal_Armorer && skillCode <= kActorVal_Speechcraft && attrCode < kActorVal_Luck)
		{
			TESSkill* skill = TESSkill::SkillForActorVal(skillCode);
			if (skill)
			{
				skill->attribute = attrCode;
				*result = 1;
			}
		}
	}

	return true;
}

static bool Cmd_StringToActorValue_Execute(COMMAND_ARGS)
{
	*result = 0;
	char avString[256] = { 0 };

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &avString))
		return true;
	
	UInt32 av = GetActorValueForString(avString);
	*result = av;
#if _DEBUG
	Console_Print("%s: %i", avString, av);
#endif
	return true;
}

static eAVModifier GetAVModifierForString(const char* str)
{
	if (!_stricmp(str, "script"))
		return kAVModifier_Offset;
	else if (!_stricmp(str, "damage"))
		return kAVModifier_Damage;
	else if (!_stricmp(str, "max"))
		return kAVModifier_Max;
	else
		return kAVModifier_Invalid;
}

static bool Cmd_GetAVMod_Execute(COMMAND_ARGS)
{
	// float actor.GetAVMod actorValue {"script"|"damage"|"max"}
	UInt32 avCode = kActorVal_OblivionMax;
	char buf[0x100] = { 0 };
	*result = 0;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (!actor) {
		return true;
	}

	if (ExtractArgs(PASS_EXTRACT_ARGS, &avCode, buf)) {
		if (avCode < kActorVal_OblivionMax) {
			eAVModifier mod = GetAVModifierForString(buf);
			*result = actor->GetAVModifier(mod, avCode);
		}
	}

	return true;
}

static bool ModAVMod_Execute(COMMAND_ARGS, bool bSet)
{
	// float actor.ModAVMod actorval {script|damage|max} amt
	UInt32 avCode = kActorVal_OblivionMax;
	char buf[0x100] = { 0 };
	float amt = 0;
	*result = 0;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && ExtractArgs(PASS_EXTRACT_ARGS, &avCode, buf, &amt)) {
		if (avCode < kActorVal_OblivionMax) {
			eAVModifier mod = GetAVModifierForString(buf);
			if (mod != kAVModifier_Invalid) {
				if (bSet) {
					amt -= actor->GetAVModifier(mod, avCode);
				}

				switch (mod) {
					case kAVModifier_Max:
						actor->ModMaxAV_F(avCode, amt, NULL);
						break;
					case kAVModifier_Offset:
						actor->ApplyScriptAVMod_F(avCode, amt, NULL);
						break;
					case kAVModifier_Damage:
						actor->DamageAV_F(avCode, amt, NULL);
						break;
				}
			
				// return the new value of the modifier
				*result = actor->GetAVModifier(mod, avCode);
			}
		}
	}

	return true;
}

static bool Cmd_ModAVMod_Execute(COMMAND_ARGS)
{
	return ModAVMod_Execute(PASS_COMMAND_ARGS, false);
}

static bool Cmd_SetAVMod_Execute(COMMAND_ARGS)
{
	return ModAVMod_Execute(PASS_COMMAND_ARGS, true);
}

static bool Cmd_GetMaxAV_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	UInt32 avCode = kActorVal_OblivionMax;

	if (actor && ExtractArgs(PASS_EXTRACT_ARGS, &avCode)) {
		if (avCode < kActorVal_OblivionMax) {
			*result = actor->GetCalculatedBaseAV(avCode) + actor->GetAVModifier(kAVModifier_Max, avCode);
		}
	}

	return true;
}

static bool Cmd_GetAVForBaseActor_Execute(COMMAND_ARGS)
{
	UInt32 avCode = kActorVal_OblivionMax;
	TESActorBase* base = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &avCode, &base)) {
		if (!base && thisObj) {
			base = OBLIVION_CAST(thisObj->baseForm, TESForm, TESActorBase);
		}

		if (base && avCode < kActorVal_OblivionMax) {
			*result = base->GetActorValue(avCode);
		}
	}

	return true;
}	

static bool Cmd_GetAVModC_Execute(COMMAND_ARGS)
{
	return Cmd_GetAVMod_Execute(PASS_COMMAND_ARGS);
}

static bool Cmd_ModAVModC_Execute(COMMAND_ARGS)
{
	return Cmd_ModAVMod_Execute(PASS_COMMAND_ARGS);
}

static bool Cmd_SetAVModC_Execute(COMMAND_ARGS)
{
	return Cmd_SetAVMod_Execute(PASS_COMMAND_ARGS);
}

static bool Cmd_GetMaxAVC_Execute(COMMAND_ARGS)
{
	return Cmd_GetMaxAV_Execute(PASS_COMMAND_ARGS);
}

static bool Cmd_GetAVForBaseActorC_Execute(COMMAND_ARGS)
{
	return Cmd_GetAVForBaseActor_Execute(PASS_COMMAND_ARGS);
}

static bool Cmd_GetLuckModifiedSkill_Execute(COMMAND_ARGS)
{
	UInt32 skill, luck, capped = 1;
	*result = -1.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &skill, &luck, &capped)) {
		*result = GetLuckModifiedSkill(skill, luck, capped);
	}

	if (IsConsoleMode()) {
		Console_Print("GetLuckModifiedSkill >> %.2f", *result);
	}

	return true;
}

static bool Cmd_GetAVSkillMasteryLevel_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 skillCode = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &skillCode) && thisObj)
	{
		if (skillCode >= kActorVal_Armorer && skillCode <= kActorVal_Speechcraft)
		{
			Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
			if (actor)
			{
				*result = GetSkillMasteryLevel(actor->GetBaseActorValue(skillCode));

				if (IsConsoleMode()) {
					Console_Print("GetAVSkillMasteryLevel >> %.2f", *result);
				}
			}
		}
	}

	return true;
}

#endif


static ParamInfo kParams_SkillUseIncrement[2] =
{
	{	"skill", kParamType_ActorValue, 0 },
	{	"index", kParamType_Integer, 1 },
};

CommandInfo kCommandInfo_GetSkillUseIncrement =
{
	"GetSkillUseIncrement",
	"",
	0,
	"returns the skill experience use increment for the specified skill",
	0,
	2,
	kParams_SkillUseIncrement,
	HANDLER(Cmd_GetSkillUseIncrement_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SkillUseIncrementC[2] =
{
	{	"skill", kParamType_Integer, 0 },
	{	"index", kParamType_Integer, 1 },
};

CommandInfo kCommandInfo_GetSkillUseIncrementC =
{
	"GetSkillUseIncrementC",
	"",
	0,
	"returns the skill experience use increment for the specified skill",
	0,
	2,
	kParams_SkillUseIncrementC,
	HANDLER(Cmd_GetSkillUseIncrement_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetSkillUseIncrement[3] =
{
	{	"nuVal", kParamType_Float, 0 },
	{	"skill", kParamType_ActorValue, 0 },
	{	"index", kParamType_Integer, 1 },
};

CommandInfo kCommandInfo_SetSkillUseIncrement =
{
	"SetSkillUseIncrement",
	"",
	0,
	"sets the skill experience use increment for the specified skill",
	0,
	3,
	kParams_SetSkillUseIncrement,
	HANDLER(Cmd_SetSkillUseIncrement_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetSkillUseIncrementC[3] =
{
	{	"nuVal", kParamType_Float, 0 },
	{	"skill", kParamType_Integer, 0 },
	{	"index", kParamType_Integer, 1 },
};

CommandInfo kCommandInfo_SetSkillUseIncrementC =
{
	"SetSkillUseIncrementC",
	"",
	0,
	"sets the skill experience use increment for the specified skill",
	0,
	3,
	kParams_SetSkillUseIncrementC,
	HANDLER(Cmd_SetSkillUseIncrement_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetSkillGoverningAttribute, returns the governing attribute of a skill given an actor value, 0, 1, kParams_OneActorValue);
DEFINE_COMMAND(SetSkillGoverningAttribute, sets the governing attribute of a skill, 0, 2, kParams_TwoActorValues);

CommandInfo kCommandInfo_GetSkillGoverningAttributeC =
{
	"GetSkillGoverningAttributeC",
	"",
	0,
	"returns the governing attribute of a skill passed as an actor value code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetSkillGoverningAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetSkillGoverningAttributeC =
{
	"SetSkillGoverningAttributeC",
	"",
	0,
	"sets the governing attribute of a skill",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetSkillGoverningAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ActorValueToCode =
{
	"ActorValueToCode",
	"AVtoC",
	0,
	"returns the integer code for an actor value",
	0,
	1,
	kParams_OneActorValue,
	HANDLER(Cmd_ActorValueToCode_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_StringToActorValue =
{
	"StringToActorValue",
	"StringAV",
	0,
	"returns actor value code of the string",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_StringToActorValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetAVMod[] =
{
	{	"actor value",	kParamType_ActorValue,	0	},
	{	"modifier",		kParamType_String,		0	},
};

static ParamInfo kParams_GetAVModC[] =
{
	{	"actor value",	kParamType_Integer,		0	},
	{	"modifier",		kParamType_String,		0	},
};

static ParamInfo kParams_ModAVMod[] =
{
	{	"actor value",	kParamType_ActorValue,	0	},
	{	"modifier",		kParamType_String,		0	},
	{	"amount",		kParamType_Float,		0	},
};	

static ParamInfo kParams_ModAVModC[] =
{
	{	"actor value",	kParamType_Integer,		0	},
	{	"modifier",		kParamType_String,		0	},
	{	"amount",		kParamType_Float,		0	},
};	

static ParamInfo kParams_GetAVForBaseActor[] =
{
	{	"actor value",	kParamType_ActorValue,	0	},
	{	"base actor",	kParamType_ActorBase,	1	},
};

static ParamInfo kParams_GetAVForBaseActorC[] =
{
	{	"actor value",	kParamType_Integer,		0	},
	{	"base actor",	kParamType_ActorBase,	1	},
};

DEFINE_COMMAND(GetAVMod, 
			   returns the specified modifier affecting the specified actor value for the calling actor,
			   1, 2, kParams_GetAVMod);

DEFINE_COMMAND(ModAVMod,
			   "modifies the calling actor's modifier for the specified actor value",
			   1, 3, kParams_ModAVMod);

DEFINE_COMMAND(SetAVMod,
			   "sets the calling actor's modifier for the specified actor value",
			   1, 3, kParams_ModAVMod);

DEFINE_COMMAND(GetMaxAV, returns the maximum value of the actor value for the calling actor, 1, 1, kParams_OneActorValue);
DEFINE_COMMAND(GetAVForBaseActor, returns the actor value defined for the base actor, 0, 2, kParams_GetAVForBaseActor);

DEFINE_COMMAND(GetAVModC, 
			   "returns the damage, max, or script modifier affecting the specified actor value for the calling actor",
			   1, 2, kParams_GetAVModC);

DEFINE_COMMAND(ModAVModC,
			   "modifies the calling actor's modifier for the specified actor value",
			   1, 3, kParams_ModAVModC);

DEFINE_COMMAND(SetAVModC,
			   "sets the calling actor's modifier for the specified actor value",
			   1, 3, kParams_ModAVModC);

DEFINE_COMMAND(GetMaxAVC, returns the maximum value of the actor value for the calling actor, 1, 1, kParams_OneInt);
DEFINE_COMMAND(GetAVForBaseActorC, returns the actor value defined for the base actor, 0, 2, kParams_GetAVForBaseActorC);

static ParamInfo kParams_GetLuckModifiedSkill[] =
{
	{	"skill level",	kParamType_Integer,		0	},
	{	"luck",			kParamType_Integer,		0	},
	{	"capped",		kParamType_Integer,		1	},
};

DEFINE_COMMAND(GetLuckModifiedSkill, returns the skill modified for luck, 0, 3, kParams_GetLuckModifiedSkill);

DEFINE_COMMAND(GetAVSkillMasteryLevel, returns the mastery level of the skill, 1, 1, kParams_OneActorValue);
CommandInfo kCommandInfo_GetAVSkillMasteryLevelC =
{
	"GetAVSkillMasteryLevelC",
	"",
	0,
	"returns the mastery level of the skills",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetAVSkillMasteryLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};