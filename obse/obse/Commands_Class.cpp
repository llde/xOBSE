#include "Commands_Class.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameProcess.h"
#include "ArrayVar.h"

static bool Cmd_IsMajor_Eval(COMMAND_ARGS_EVAL){
    *result = 0;
    UInt32 skill = *((UInt32*)arg1);
    TESClass* theClass = (TESClass*)arg2;
    if (!IsSkill(skill)) return true;
    if (!theClass) {
        if (!thisObj) return true;
        TESNPC* npc = (TESNPC *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
        if (!npc || !npc->npcClass) return true;
        theClass = npc->npcClass;
    }

    if (theClass->IsMajorSkill(skill)) {
        *result = 1;
    }
    return true;
}

static bool Cmd_IsMajor_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 skill = 0;
	TESClass* theClass = NULL;

	ExtractArgs(PASS_EXTRACT_ARGS, &skill, &theClass);

	if (!IsSkill(skill)) return true;
	if (!theClass) {
		if (!thisObj) return true;
		TESNPC* npc = (TESNPC *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc || !npc->npcClass) return true;
		theClass = npc->npcClass;
	}

	if (theClass->IsMajorSkill(skill)) {
		*result = 1;
	}
	return true;
}

static bool Cmd_IsMajorRef_Execute(COMMAND_ARGS) {
	*result = 0;
	UInt32 skill = 0;
	if (!thisObj->IsActor()) return true;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &skill)) return true;
	if (!IsSkill(skill)) return true;
	TESNPC* npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
	if (!npc || !npc->npcClass) return true;
	*result = npc->npcClass->IsMajorSkill(skill);
	return true;
}

static bool Cmd_IsMajorRef_Eval(COMMAND_ARGS_EVAL) {
	*result = 0;
	TESNPC* npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
	UInt32 skill = *(UInt32*)arg1;
	if (!npc && !npc->npcClass && !IsSkill(skill)) return true;
	*result = npc->npcClass->IsMajorSkill(skill);
	return true;
}

static bool Cmd_IsClassAttribute_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 attribute = 0;
	TESClass* theClass = NULL;

	ExtractArgs(PASS_EXTRACT_ARGS, &attribute, &theClass);

	if (attribute > kActorVal_Luck) return true;
	if (!theClass) {
		if (!thisObj) return true;
		TESNPC* npc = (TESNPC *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc || !npc->npcClass) return true;
		theClass = npc->npcClass;
	}

	for (int ix = 0; ix < 2; ++ix) {
		if (theClass->attributes[ix] == attribute) {
			*result = 1;
			return true;
		}
	}

	return true;
}

static bool Cmd_GetClass_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	TESNPC* npc = NULL;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &npc))
		return true;

	if (!npc && thisObj)
		npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (!npc || !npc->npcClass) return true;

	*refResult = npc->npcClass->refID;
	if (IsConsoleMode())
		Console_Print("GetClass >> %s (%08X)", GetFullName(npc->npcClass), *refResult);

	return true;
}

static bool Cmd_GetClassAttribute_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 which = 0;
	TESClass* theClass = NULL;

	ExtractArgs(PASS_EXTRACT_ARGS, &which, &theClass);

	if (which > 1) return true;
	if (!theClass) {
		if(!thisObj) return true;
		TESNPC* npc = (TESNPC *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc || !npc->npcClass) return true;
		theClass = npc->npcClass;
	}

	*result = theClass->attributes[which];
	return true;
}

static bool Cmd_SetClassAttribute_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 which = 0;
	UInt32 nuAttr = -1;
	TESClass* theClass = NULL;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &which, &nuAttr, &theClass) && which < 2 && nuAttr <= kActorVal_Luck)
	{
		if (!theClass && thisObj)
		{
			TESNPC* npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
			if (npc && npc->npcClass)
				theClass = npc->npcClass;
		}

		if (theClass)
		{
			// make sure attribute is not the same as other governing attribute
			if (theClass->attributes[1 - which] != nuAttr)
			{
				theClass->attributes[which] = nuAttr;
				*result = 1;
			}
		}
	}

	return true;
}

static bool Cmd_GetClassSkill_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 which = 0;
	TESClass* theClass = NULL;

	ExtractArgs(PASS_EXTRACT_ARGS, &which, &theClass);

	if (which > 6) return true;
	if (!theClass) {
		if(!thisObj) return true;
		TESNPC* npc = (TESNPC *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc || !npc->npcClass) return true;
		theClass = npc->npcClass;
	}

	*result = theClass->majorSkills[which];
	return true;
}

static bool Cmd_GetClassSkills_Execute(COMMAND_ARGS)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	TESClass* theClass = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &theClass))
	{
		if (!theClass && thisObj)
		{
			TESNPC* npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
			theClass = npc ? npc->npcClass : NULL;
		}

		if (theClass)
		{
			for (UInt32 idx = 0; idx < 7; idx++)
			{
				g_ArrayMap.SetElementNumber(arr, idx, theClass->majorSkills[idx]);
			}
		}
	}

	return true;
}

static bool SetClassSkills_Execute(COMMAND_ARGS, bool bAllowDuplicates)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		TESClass* theClass = NULL;
		if (eval.Arg(1))
			theClass = OBLIVION_CAST(eval.Arg(1)->GetTESForm(), TESForm, TESClass);
		else if (thisObj)
		{
			TESNPC* npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
			theClass = npc ? npc->npcClass : NULL;
		}

		if (theClass)
		{
			std::vector<const ArrayElement*> vec;
			if (g_ArrayMap.AsVector(arrID, vec) && vec.size() == 7)
			{
				// make sure all skills are unique and valid skills
				std::set<UInt32> skillSet;
				std::vector<UInt32> skills;

				for (UInt32 i = 0; i < 7; i++)
				{
					double skill;
					if (vec[i]->GetAsNumber(&skill) && skill >= kActorVal_Armorer && skill <= kActorVal_Speechcraft) {
						skillSet.insert(skill);
						skills.push_back(skill);
					}
				}

				if (bAllowDuplicates || skillSet.size() == 7)
				{
					for (UInt32 idx = 0; idx < skills.size(); idx++)
					{
						theClass->majorSkills[idx] = skills[idx];
					}

					*result = 1;
				}
			}
		}
	}

	return true;
}

static bool Cmd_SetClassSkills_Execute(COMMAND_ARGS)
{
	return SetClassSkills_Execute(PASS_COMMAND_ARGS, false);
}

static bool Cmd_SetClassSkills2_Execute(COMMAND_ARGS)
{
	return SetClassSkills_Execute(PASS_COMMAND_ARGS, true);
}

static bool Cmd_GetClassSpecialization_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESClass* theClass = NULL;

	ExtractArgs(PASS_EXTRACT_ARGS, &theClass);

	if (!theClass) {
		if(!thisObj) return true;
		TESNPC* npc = (TESNPC *)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (!npc || !npc->npcClass) return true;
		theClass = npc->npcClass;
	}

	*result = theClass->specialization;
	return true;
}

static bool Cmd_SetClassSpecialization_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 nuSpec = -1;
	TESClass* theClass = NULL;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &nuSpec, &theClass) && nuSpec <= TESClass::eSpec_Stealth)
	{
		if (!theClass && thisObj)
		{
			TESNPC* npc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);
			theClass = npc ? npc->npcClass : NULL;
		}

		if (theClass)
		{
			theClass->specialization = nuSpec;
			*result = 1;
		}
	}

	return true;
}

static bool Cmd_GetSkillSpecialization_Execute(COMMAND_ARGS)
{
	UInt32 skillAV = -1;
	*result = -1.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &skillAV)) {
		TESSkill* skill = TESSkill::SkillForActorVal(skillAV);
		if (skill) {
			*result = skill->specialization;

			if (IsConsoleMode()) {
				Console_Print("GetSkillSpecialization >> %d", skill->specialization);
			}
		}
	}

	return true;
}

static bool Cmd_SetSkillSpecialization_Execute(COMMAND_ARGS)
{
	UInt32 skillAV = -1;
	UInt32 newSpec = -1;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &skillAV, &newSpec) && newSpec < TESClass::eSpec_MAX) {
		TESSkill* skill = TESSkill::SkillForActorVal(skillAV);
		if (skill) {
			skill->specialization = newSpec;
			*result = 1.0;

			if (IsConsoleMode()) {
				Console_Print("Skill specialization set to %d", skill->specialization);
			}
		}
	}

	return true;
}

#endif

CommandInfo kCommandInfo_GetClass =
{
	"GetClass",
	"gclass",
	0,
	"returns the ref to the class of the calling actor",
	0,
	1,
	kParams_OneOptionalNPC,
	HANDLER(Cmd_GetClass_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_IsMajorRef[1] = {
	{"skill" , kParamType_ActorValue, 0}
};

static ParamInfo kParams_IsMajor[2] =
{
	{	"skill", kParamType_ActorValue, 0 },
	{	"class", kParamType_Class, 1 },
};

static ParamInfo kParams_IsMajorC[2] =
{
	{	"skill", kParamType_Integer, 0 },
	{	"class", kParamType_Class, 1 },
};

CommandInfo kCommandInfo_IsClassSkill =
{
	"IsClassSkill",
	"IsMajor",
	0,
	"returns 1 if the skill is a major skill of the class",
	0,
	2,
	kParams_IsMajor,
	HANDLER(Cmd_IsMajor_Execute),
	Cmd_Default_Parse,
	HANDLER_EVAL(Cmd_IsMajor_Eval),
	0
};

CommandInfo kCommandInfo_IsMajorRef =
{
	"IsMajorRef",
	"",
	0,
	"returns 1 if the skill is a major skill of the class of the reference npc",
	1,
	1,
	kParams_IsMajorRef,
	HANDLER(Cmd_IsMajorRef_Execute),
	Cmd_Default_Parse,
	HANDLER_EVAL(Cmd_IsMajorRef_Eval),
	0
};

CommandInfo kCommandInfo_IsClassAttribute =
{
	"IsClassAttribute",
	"",
	0,
	"returns 1 if the attribute is part of the class",
	0,
	2,
	kParams_IsMajor,
	HANDLER(Cmd_IsClassAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsClassSkillC =
{
	"IsClassSkillC",
	"IsMajorC",
	0,
	"returns 1 if the skill is a major skill of the class",
	0,
	2,
	kParams_IsMajorC,
	HANDLER(Cmd_IsMajor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsClassAttributeC =
{
	"IsClassAttributeC",
	"",
	0,
	"returns 1 if the attribute is part of the class",
	0,
	2,
	kParams_IsMajorC,
	HANDLER(Cmd_IsClassAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_ClassInfo[2] =
{
	{	"index", kParamType_Integer, 0 },
	{	"class", kParamType_Class, 1 },
};

CommandInfo kCommandInfo_GetClassAttribute =
{
	"GetClassAttribute",
	"GetAttrib",
	0,
	"returns a code for the specified index for a class attribute",
	0,
	2,
	kParams_ClassInfo,
	HANDLER(Cmd_GetClassAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClassSkill =
{
	"GetClassSkill",
	"GetSkill",
	0,
	"returns a code for the specified index of the skills for the given class",
	0,
	2,
	kParams_ClassInfo,
	HANDLER(Cmd_GetClassSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetClassSpecialization =
{
	"GetClassSpecialization",
	"GetSpec",
	0,
	"returns a code for the given classes specialization",
	0,
	1,
	kParams_OneOptionalClass,
	HANDLER(Cmd_GetClassSpecialization_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetClassAttribute[3] =
{
	{	"index",	kParamType_Integer,		0	},
	{	"attribute",kParamType_ActorValue,	0	},
	{	"class",	kParamType_Class,		1	}
};

static ParamInfo kParams_SetClassAttributeC[3] =
{
	{	"index",	kParamType_Integer,		0	},
	{	"attribute",kParamType_Integer,		0	},
	{	"class",	kParamType_Class,		1	},
};

DEFINE_COMMAND(SetClassAttribute, sets a class governing attribute, 0, 3, kParams_SetClassAttribute);

CommandInfo kCommandInfo_SetClassAttributeC =
{
	"SetClassAttributeC",
	"",
	0,
	"sets a class governing attribute",
	0,
	3,
	kParams_SetClassAttributeC,
	HANDLER(Cmd_SetClassAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(SetClassSpecialization, sets a class specialization, 0, 2, kParams_ClassInfo);
DEFINE_COMMAND(GetClassSkills, returns an array of class skills, 0, 1, kParams_OneOptionalClass);

static ParamInfo kOBSEParams_SetClassSkills[2] =
{
	{	"skills",	kOBSEParamType_Array,	0	},
	{	"class",	kOBSEParamType_Form,	1	},
};

CommandInfo kCommandInfo_SetClassSkills =
{
	"SetClassSkills",
	"",
	0,
	"sets the major skills associated with a class",
	0,
	2,
	kOBSEParams_SetClassSkills,
	HANDLER(Cmd_SetClassSkills_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetClassSkills2 =
{
	"SetClassSkills2",
	"",
	0,
	"identical to SetClassSkills, but allows duplicate skills to be present.",
	0,
	2,
	kOBSEParams_SetClassSkills,
	HANDLER(Cmd_SetClassSkills2_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetSkillSpecialization, returns the skills specialization, 0, 1, kParams_OneActorValue);
CommandInfo kCommandInfo_GetSkillSpecializationC =
{
	"GetSkillSpecializationC",
	"",
	0,
	"returns the skills specialization",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetSkillSpecialization_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetSkillSpecialization[2] =
{
	{ "skill",			kParamType_ActorValue,	0	},
	{ "specialization",	kParamType_Integer,		0	},
};

static ParamInfo kParams_SetSkillSpecializationC[2] =
{
	{ "skill",			kParamType_Integer,		0	},
	{ "specialization",	kParamType_Integer,		0	},
};

DEFINE_COMMAND(SetSkillSpecialization, sets the skills specialization, 0, 2, kParams_SetSkillSpecialization);
CommandInfo kCommandInfo_SetSkillSpecializationC =
{
	"SetSkillSpecializationC",
	"",
	0,
	"sets the skills specialization",
	0,
	2,
	kParams_SetSkillSpecializationC,
	HANDLER(Cmd_SetSkillSpecialization_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
