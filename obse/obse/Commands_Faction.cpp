#include "Commands_Faction.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameProcess.h"

static bool Cmd_IsFactionEvil_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	*result = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact))
	{
		if (fact && fact->IsEvil())
			*result = 1;
	}

	return true;
}

static bool Cmd_IsFactionHidden_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	*result = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact))
	{
		if (fact && fact->IsHidden())
			*result = 1;
	}

	return true;
}

static bool Cmd_FactionHasSpecialCombat_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	*result = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact))
	{
		if (fact && fact->HasSpecialCombat())
			*result = 1;
	}

	return true;
}

static bool Cmd_SetFactionEvil_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	UInt32 bMod = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact, &bMod))
	{
		if (fact)
			fact->SetEvil(bMod ? true : false);
	}

	return true;
}

static bool Cmd_SetFactionHidden_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	UInt32 bMod = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact, &bMod))
	{
		if (fact)
			fact->SetHidden(bMod ? true : false);
	}

	return true;
}

static bool Cmd_SetFactionSpecialCombat_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	UInt32 bMod = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact, &bMod))
	{
		if (fact)
			fact->SetSpecialCombat(bMod ? true : false);
	}

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

static bool Cmd_GetNumFactions_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESActorBase* actorBase = ExtractActorBase(PASS_COMMAND_ARGS);
	if (actorBase)
	{
		*result = FactionListVisitor(&(actorBase->actorBaseData.factionList)).Count();
	}

	return true;
}

static bool Cmd_GetNthFaction_Execute(COMMAND_ARGS)
{
	UInt32 factionIdx = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	TESActorBase* actorBase = ExtractSetActorBase(PASS_COMMAND_ARGS, &factionIdx);
	if (actorBase)
	{
		TESActorBaseData::FactionListData* data = FactionListVisitor(&(actorBase->actorBaseData.factionList)).GetNthInfo(factionIdx);
		if (data)
			*refResult = data->faction->refID;
	}

	return true;
}

static bool Cmd_GetNumRanks_Execute(COMMAND_ARGS)
{
	TESFaction* fact = NULL;
	*result = 0;

	if (ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &fact))
		*result = FactionRankVisitor(&(fact->ranks)).Count();

	return true;
}

#endif

static ParamInfo kParams_OneIntOneOptionalActorBase[2] =
{
	{	"bool",			kParamType_Integer,		0	},
	{	"base actor",	kParamType_ActorBase,	1	},
};

CommandInfo kCommandInfo_GetNumFactions =
{
	"GetNumFactions", "",
	0,
	"returns the number of factions to which an actor belongs",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetNumFactions_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthFaction =
{
	"GetNthFaction", "",
	0,
	"returns the nth faction to which an actor belongs",
	0,
	2,
	kParams_OneIntOneOptionalActorBase,
	HANDLER(Cmd_GetNthFaction_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneFaction[1] =
{
	{ "faction",	kParamType_Faction,	0	},
};

CommandInfo kCommandInfo_IsFactionEvil =
{
	"IsFactionEvil", "",
	0,
	"returns true if the faction is marked as evil",
	0,
	1,
	kParams_OneFaction,
	HANDLER(Cmd_IsFactionEvil_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsFactionHidden =
{
	"IsFactionHidden", "",
	0,
	"returns true if the faction is marked as hidden",
	0,
	1,
	kParams_OneFaction,
	HANDLER(Cmd_IsFactionHidden_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_FactionHasSpecialCombat =
{
	"FactionHasSpecialCombat", "",
	0,
	"returns true if the faction has special combat",
	0,
	1,
	kParams_OneFaction,
	HANDLER(Cmd_FactionHasSpecialCombat_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetFactionFlag[2] =
{
	{	"faction",	kParamType_Faction,	0	},
	{	"bool",		kParamType_Integer,	0	},
};

CommandInfo kCommandInfo_SetFactionEvil =
{
	"SetFactionEvil", "",
	0,
	"changes the evil flag on the faction",
	0,
	2,
	kParams_SetFactionFlag,
	HANDLER(Cmd_SetFactionEvil_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetFactionHidden =
{
	"SetFactionHidden", "",
	0,
	"changes the hidden flag on the faction",
	0,
	2,
	kParams_SetFactionFlag,
	HANDLER(Cmd_SetFactionHidden_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetFactionSpecialCombat =
{
	"SetFactionSpecialCombat", "",
	0,
	"changes the special combat flag on the faction",
	0,
	2,
	kParams_SetFactionFlag,
	HANDLER(Cmd_SetFactionSpecialCombat_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumRanks =
{
	"GetNumRanks", 
	"GetNumFactionRanks",
	0,
	"returns the number of ranks in the faction",
	0,
	1,
	kParams_OneFaction,
	HANDLER(Cmd_GetNumRanks_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};