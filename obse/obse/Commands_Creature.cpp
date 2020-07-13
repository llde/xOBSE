#include "Commands_Creature.h"
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
#include "NiObjects.h"

class PrintAnimation
{
public:
	PrintAnimation() : index(0) {}
	UInt32 index;

	bool Accept(char* animName)
	{
		Console_Print("%d> %s", index, animName);
		_MESSAGE("%d> %s", index, animName);
		++index;
		return true;
	}
};

static bool Cmd_IsCreature_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESActorBase* actorBase = NULL;
	ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &actorBase);

	if (!actorBase) {
		if (!thisObj) return true;
		actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
		if (!actorBase) return true;
	}

	TESCreature* creature = (TESCreature*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESCreature, 0);
	if (creature) {
//		AnimationVisitor visitor(&creature->animation.data);
//		UInt32 animationCount = visitor.Count();
//		Console_Print("%s has %d animations", actorBase->GetEditorName(), animationCount);
//		PrintAnimation printer;
//		visitor.Visit(printer);
		*result = 1;
	}
	return true;
}

enum {
	kCreature_Type = 0,
	kCreature_CombatSkill,
	kCreature_MagicSkill,
	kCreature_StealthSkill,
	kCreature_Reach,
	kCreature_BaseScale,
	kCreature_SoulLevel,
	kCreature_Walks,
	kCreature_Swims,
	kCreature_Flies,
	kCreature_Biped,
	kCreature_WeaponAndShield,
	kCreature_NoHead,
	kCreature_NoLArm,
	kCreature_NoRArm,
	kCreature_NoCombatInWater,
	kCreature_NoMovement,
};

static bool GetCreatureValue(COMMAND_ARGS, UInt32 whichVal)
{
	*result = 0;
	TESActorBase* actorBase = NULL;
	ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &actorBase);

	if (!actorBase) {
		if (!thisObj) return true;
		actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
		if (!actorBase) return true;
	}

	TESCreature* creature = (TESCreature*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESCreature, 0);
	switch(whichVal) {
		case kCreature_Type: 
			{
				*result = (creature) ? creature->type : -1;
				break;
			}

		case kCreature_CombatSkill:
			{
				*result = (creature) ? creature->combatSkill : 0;
				break;
			}

		case kCreature_MagicSkill:
			{
				*result = (creature) ? creature->magicSkill : 0;
				break;
			}

		case kCreature_StealthSkill:
			{
				*result = (creature) ? creature->stealthSkill : 0;
				break;
			}
		case kCreature_Reach:
			{
				*result = (creature) ? creature->attackReach : 0;
				break;
			}

		case kCreature_BaseScale:
			{
				*result = (creature) ? creature->baseScale : 1.0;
				break;
			}
		case kCreature_SoulLevel:
			{
				if (creature) {
					*result = creature->soulLevel;
				} else {
					TESNPC* npc = (TESNPC*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESNPC, 0);
					if (npc) *result = 5;
				}
				break;
			}

		case kCreature_Walks:
			{
				*result = (creature && creature->actorBaseData.CreatureWalks()) ? 1 : 0;
				break;
			}

		case kCreature_Swims:
			{
				*result = (creature && creature->actorBaseData.CreatureSwims()) ? 1 : 0;
				break;
			}
		case kCreature_Flies:
			{
				*result = (creature && creature->actorBaseData.CreatureFlies()) ? 1 : 0;
				break;
			}
		case kCreature_Biped:
			{
				*result = (creature && creature->actorBaseData.IsCreatureBiped()) ? 1 : 0;
				break;
			}
		case kCreature_WeaponAndShield:
			{
				*result = (creature && creature->actorBaseData.CreatureHasWeaponAndShield()) ? 1 : 0;
				break;
			}
		case kCreature_NoHead:
			{
				*result = (creature && creature->actorBaseData.CreatureHasNoHead()) ? 1 : 0;
				break;
			}
		case kCreature_NoLArm:
			{
				*result = (creature && creature->actorBaseData.CreatureHasNoLeftArm()) ? 1 : 0;
				break;
			}
		case kCreature_NoRArm:
			{
				*result = (creature && creature->actorBaseData.CreatureHasNoRightArm()) ? 1 : 0;
				break;
			}
		case kCreature_NoCombatInWater:
			{
				*result = (creature && creature->actorBaseData.CreatureNoCombatInWater()) ? 1 : 0;
				break;
			}
		case kCreature_NoMovement:
			{
				*result = (creature && creature->actorBaseData.CreatureHasNoMovement()) ? 1 : 0;
				break;
			}

		default:
			*result = 0;
	}
	return true;
}

static bool Cmd_GetCreatureType_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_Type);
}

static bool Cmd_SetCreatureType_Execute(COMMAND_ARGS)
{
	// cmd is a little flaky and probably doesn't do what 90% of users would want it to (make non-horse creatures ridable), but works for the purpose it was requested for
	// problems can arise with mountable creatures: if we set type to something other than horse while creature is ridden, rider can't dismount
	// if we set type to horse for a creature without an ActorParent node, weirdness occurs if actor tries to mount
	// both mostly addressed below
	UInt32 newType;
	Creature* creatureRef = OBLIVION_CAST(thisObj, TESObjectREFR, Creature);
	if (creatureRef && ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &newType) && newType < TESCreature::eCreatureType_MAX)
	{
		TESCreature* creatureBase = (TESCreature*)Oblivion_DynamicCast(creatureRef->baseForm, 0, RTTI_TESForm, RTTI_TESCreature, 0);
		if (!creatureBase)
			return true;

		// don't change creature type while creature is being ridden
		if (creatureRef->horseOrRider)
			return true;

		// don't change to horse-type unless it is ridable
		if (newType == TESCreature::eCreatureType_Horse && NULL == creatureRef->niNode->GetObject ("ActorParent"))
			return true;

		// what we *can't* feasibly check is if another reference to this base object exists in the world and is currently being ridden by an actor.
		// So ideally this cmd should only be used on a mountable creature if the creature is unique
		creatureBase->type = newType;
		*result = 1.0;
	}

	return true;
}

static bool Cmd_GetCreatureCombatSkill_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_CombatSkill);
}

static bool Cmd_GetCreatureMagicSkill_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_MagicSkill);
}

static bool Cmd_GetCreatureStealthSkill_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_StealthSkill);
}

static bool Cmd_GetCreatureReach_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_Reach);
}

static bool Cmd_GetCreatureBaseScale_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_BaseScale);
}

static bool Cmd_GetCreatureSoulLevel_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_SoulLevel);
}

static bool Cmd_GetCreatureWalks_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_Walks);
}

static bool Cmd_GetCreatureSwims_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_Swims);
}

static bool Cmd_GetCreatureFlies_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_Flies);
}

static bool Cmd_IsCreatureBiped_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_Biped);
}

static bool Cmd_CreatureUsesWeaponAndShield_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_WeaponAndShield);
}

static bool Cmd_CreatureHasNoHead_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_NoHead);
}

static bool Cmd_CreatureHasNoLeftArm_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_NoLArm);
}

static bool Cmd_CreatureHasNoRightArm_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_NoRArm);
}

static bool Cmd_CreatureNoCombatInWater_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_NoCombatInWater);
}

static bool Cmd_CreatureHasNoMovement_Execute(COMMAND_ARGS)
{
	return GetCreatureValue(PASS_COMMAND_ARGS, kCreature_NoMovement);
}


static bool Cmd_GetRider_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj)
	{
		Creature* horse = (Creature*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_Creature, 0);
		if (horse && horse->horseOrRider)
			*refResult = horse->horseOrRider->refID;
	}

	return true;
}


static bool Cmd_GetCreatureSoundBase_Execute(COMMAND_ARGS)
{
	TESActorBase* actorBase = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &actorBase))
		return true;

	if (!actorBase)
		if (thisObj)
			actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);

	TESCreature* crea = (TESCreature*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESCreature, 0);
	if (crea)
	{
		TESCreature* base = crea->GetSoundBase();
		if (base)
			*refResult = base->refID;
	}

	return true;
}

static bool Cmd_HasModel_Execute(COMMAND_ARGS)
{
	char nifPath[512];
	TESActorBase* actorBase = 0;
	*result = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &nifPath, &actorBase))
		return true;

	if (!actorBase)
		if (thisObj)
			actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);

	TESCreature* crea = (TESCreature*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESCreature, 0);
	if (crea && crea->modelList.FindNifPath(nifPath))
			*result = 1;


	return true;
}

class ModelListDumper
{
public:
	bool Accept(char* nifPath)
	{
		Console_Print("%s", nifPath);
		_MESSAGE("%s", nifPath);
		return true;
	}
};

static bool Cmd_ToggleCreatureModel_Execute(COMMAND_ARGS)
{
	TESActorBase* actorBase = NULL;
	UInt32 bEnable = 0;
	char nifPath[512];
	*result = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &nifPath, &bEnable, &actorBase))
		return false;

	if (!actorBase)
		if (thisObj)
			actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);	

	TESCreature* crea = (TESCreature*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESCreature, 0);
	if (!crea)
		return true;
	if (bEnable)
	{
		if (crea->modelList.AddEntry(nifPath))
			*result = 1;
	}
	else
	{
		//ModelListVisitor(&(crea->modelList.modelList)).Visit(ModelListDumper());
		if (crea->modelList.RemoveEntry(nifPath))
			*result = 1;
		//ModelListVisitor(&crea->modelList.modelList).Visit(ModelListDumper());
	}
	return true;
}

static bool Cmd_GetCreatureModelPaths_Execute(COMMAND_ARGS)
{
	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	TESActorBase* actorBase = NULL;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorBase))
	{
		if (!actorBase && thisObj)
			actorBase = OBLIVION_CAST(thisObj->baseForm, TESForm, TESActorBase);

		if (actorBase)
		{
			TESCreature* crea = OBLIVION_CAST(actorBase, TESActorBase, TESCreature);
			if (crea)
			{
				UInt32 idx = 0;
				for (TESModelList::Entry* cur = &crea->modelList.modelList; cur && cur->nifPath; cur = cur->next)
				{
					g_ArrayMap.SetElementString(arrID, ArrayKey(idx), cur->nifPath);
					idx++;
				}
			}
		}
	}

	return true;
}

static bool Cmd_GetCreatureSound_Execute(COMMAND_ARGS)
{
	TESActorBase* actorBase = 0;
	UInt32 whichSound = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &whichSound, &actorBase))
		return true;

	if (!actorBase)
		if (thisObj)
			actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);

	TESCreature* crea = (TESCreature*)Oblivion_DynamicCast(actorBase, 0, RTTI_TESActorBase, RTTI_TESCreature, 0);
	if (crea)
	{
		TESSound* sound = crea->GetSound(whichSound);
		if (sound)
			*refResult = sound->refID;
	}
	return true;
}

static bool Cmd_SetCreatureSoundBase_Execute(COMMAND_ARGS)
{
	TESActorBase* abCrea = NULL;
	TESActorBase* abBase = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &abBase, &abCrea)) {
		if (!abCrea && thisObj)
			abCrea = OBLIVION_CAST(thisObj->baseForm, TESForm, TESActorBase);

		TESCreature* crea = OBLIVION_CAST(abCrea, TESActorBase, TESCreature);
		TESCreature* base = OBLIVION_CAST(abBase, TESActorBase, TESCreature);

		if (crea && base)
			*result = crea->SetSoundBase(base) ? 1.0 : 0.0;
	}

	return true;
}

static bool Cmd_SetCreatureSkill_Execute(COMMAND_ARGS)
{
	char skillname[0x200] = { 0 };
	UInt32 skill = 0;
	TESActorBase* actorBase = NULL;

	if (ExtractArgs(PASS_EXTRACT_ARGS, skillname, &skill, &actorBase)) {
		if (!actorBase && thisObj) {
			actorBase = OBLIVION_CAST(thisObj->baseForm, TESForm, TESActorBase);
		}

		TESCreature* crea = OBLIVION_CAST(actorBase, TESActorBase, TESCreature);
		if (crea && skill < 0x100) {
			if (!_stricmp(skillname, "combat")) {
				crea->combatSkill = skill;
			}
			else if (!_stricmp(skillname, "stealth")) {
				crea->stealthSkill = skill;
			}
			else if (!_stricmp(skillname, "magic")) {
				crea->magicSkill = skill;
			}
		}
	}

	return true;
}

#endif

CommandInfo kCommandInfo_IsCreature =
{
	"IsCreature",
	"",
	0,
	"returns 1 if the passed actor base is a creature",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_IsCreature_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureType =
{
	"GetCreatureType",
	"",
	0,
	"returns the type of the calling creature or passed refID",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureCombatSkill =
{
	"GetCreatureCombatSkill",
	"GetCreatureCombat",
	0,
	"returns the combat skill of the calling creature or passed refID",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureCombatSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureMagicSkill =
{
	"GetCreatureMagicSkill",
	"GetCreatureMagic",
	0,
	"returns the magic skill of the calling creature or passed refID",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureMagicSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureStealthSkill =
{
	"GetCreatureStealthSkill",
	"GetCreatureStealth",
	0,
	"returns the stealth skill of the calling creature or passed refID",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureStealthSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureBaseScale =
{
	"GetCreatureBaseScale",
	"GetCreatureScale",
	0,
	"returns the base scale of the calling creature or passed refID",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureBaseScale_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureReach =
{
	"GetCreatureReach",
	"",
	0,
	"returns the reach of the calling creature or passed refID",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureReach_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureSoulLevel =
{
	"GetCreatureSoulLevel", "GetActorSoulLevel",
	0,
	"returns the soul level of the calling actor or passed refID",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureSoulLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureWalks =
{
	"GetCreatureWalks", "CreatureWalks",
	0,
	"returns 1 if the calling creature or creature refID has the Walk flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureWalks_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureFlies =
{
	"GetCreatureFlies", "CreatureFlies",
	0,
	"returns 1 if the calling creature or creature refID has the Flies flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureFlies_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureSwims =
{
	"GetCreatureSwims", "CreatureSwims",
	0,
	"returns 1 if the calling creature or creature refID has the Swims flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureSwims_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsCreatureBiped =
{
	"IsCreatureBiped", "IsBiped",
	0,
	"returns 1 if the calling creature or creature refID is marked as a biped",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_IsCreatureBiped_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CreatureHasNoMovement =
{
	"CreatureHasNoMovement", "",
	0,
	"returns 1 if the calling creature or creature refID has the None movement flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_CreatureHasNoMovement_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CreatureHasNoHead =
{
	"CreatureHasNoHead", "",
	0,
	"returns 1 if the calling creature or creature refID has the NoHead flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_CreatureHasNoHead_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CreatureHasNoLeftArm =
{
	"CreatureHasNoLeftArm", "",
	0,
	"returns 1 if the calling creature or creature refID has the NoLeftArm flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_CreatureHasNoLeftArm_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CreatureHasNoRightArm =
{
	"CreatureHasNoRightArm", "",
	0,
	"returns 1 if the calling creature or creature refID has the NoRightArm flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_CreatureHasNoRightArm_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CreatureNoCombatInWater =
{
	"CreatureNoCombatInWater", "",
	0,
	"returns 1 if the calling creature or creature refID has the NoCombatInWater flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_CreatureNoCombatInWater_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CreatureUsesWeaponAndShield =
{
	"CreatureUsesWeaponAndShield", "",
	0,
	"returns 1 if the calling creature or creature refID has the WeaponAndShield flag",
	0, 1, kParams_OneOptionalActorBase,
	HANDLER(Cmd_CreatureUsesWeaponAndShield_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetRider =
{
	"GetRider", "",
	0,
	"returns a reference to the actor currently riding the calling horse",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetRider_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureSoundBase =
{
	"GetCreatureSoundBase", "",
	0,
	"returns the creature from which the specified creature's sounds are derived",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureSoundBase_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneStringOneOptionalActorBase[2] =
{
	{	"model path",	kParamType_String,		0	},
	{	"creature",		kParamType_ActorBase,	1	},
};

CommandInfo kCommandInfo_HasModel =
{
	"HasModel",
	"",
	0,
	"returns 1 if the creature has the specified model path",
	0,
	2,
	kParams_OneStringOneOptionalActorBase,
	HANDLER(Cmd_HasModel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_ToggleCreatureModel[3] =
{
	{	"model path",	kParamType_String,		0	},
	{	"bool",			kParamType_Integer,		0	},
	{	"creature",		kParamType_ActorBase,	1	},
};

CommandInfo kCommandInfo_ToggleCreatureModel =
{
	"ToggleCreatureModel",
	"ToggleModel",
	0,
	"toggles a nifpath on or off in a creature's model list",
	0,
	3,
	kParams_ToggleCreatureModel,
	HANDLER(Cmd_ToggleCreatureModel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneIntOneOptionalActorBase[2] =
{
	{	"int",	kParamType_Integer,		0 },
	{	"actor",kParamType_ActorBase,	1 },
};

CommandInfo kCommandInfo_GetCreatureSound =
{
	"GetCreatureSound",
	"",
	0,
	"returns the sound associated with a creature action",
	0,
	2,
	kParams_OneIntOneOptionalActorBase,
	HANDLER(Cmd_GetCreatureSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCreatureModelPaths =
{
	"GetCreatureModelPaths",
	"",
	0,
	"returns an array of model paths",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetCreatureModelPaths_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneActorBase_OneOptionalActorBase[] =
{
	{	"actor base",	kParamType_ActorBase,	0	},
	{	"actor base",	kParamType_ActorBase,	1	},
};

DEFINE_COMMAND(SetCreatureSoundBase, "sets the creature from which the creature inherits its sounds. The inheriting creature must not itself be a sound base (mustn't define its own sounds), and the creature specified as the sound base must be a sound base (defining its own sounds) or null.",
			   0, 2, kParams_OneActorBase_OneOptionalActorBase);

static ParamInfo kParams_SetCreatureSkill[3] =
{
	{ "skillName",	kParamType_String,		0	},
	{ "skillLevel", kParamType_Integer,		0	},
	{ "creature",	kParamType_ActorBase,	1	},
};

DEFINE_COMMAND(SetCreatureSkill, sets the skill level for a creatures skill, 0, 3, kParams_SetCreatureSkill);

DEFINE_COMMAND(SetCreatureType, sets the type of the creature, 1, 1, kParams_OneInt);