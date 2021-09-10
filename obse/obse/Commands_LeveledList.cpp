#include "ParamInfos.h"
#include "obse\Commands_LeveledList.h"
#include "Script.h"


#if OBLIVION

#include "GameForms.h"
#include "GameObjects.h"
#include "ArrayVar.h"

static bool Cmd_AddToLeveledList_Execute(COMMAND_ARGS)
{
	TESForm*	list;
	TESForm*	form;
	UInt32		level = 1;	
	UInt32		count = 1;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &list, &form, &level, &count)) {
		TESLeveledList*	levList = (TESLeveledList*)Oblivion_DynamicCast(list, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
		if (!levList || !form)
		{
			return true;
		}

		levList->AddItem(form, level, count);
	}

	return true;
}

static bool Cmd_RemoveFromLeveledList_Execute(COMMAND_ARGS)
{	
	TESForm*	list;
	TESForm*	form;
	*result = 0;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &list, &form);
	TESLeveledList*	levList = (TESLeveledList*)Oblivion_DynamicCast(list, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	if (!levList || !form)
	{
		return true;
	}

	*result = (double)(levList->RemoveItem(form));
	return true;
}

static bool CalcLevItem_Execute(COMMAND_ARGS, bool noRecurse)
{
	TESForm* list;
	SInt32 level = -1;
	UInt32 useChanceNone = 1;
	SInt32 levelDiff = -1;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &list, &level, &useChanceNone, &levelDiff);
	if (level == -1)	//param omitted
		return true;

	TESLeveledList* levList = (TESLeveledList*)Oblivion_DynamicCast(list, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	if (!list || level == -1)
		return true;

	if (levelDiff == -1)	//param omitted so use default
	{
		SettingInfo * info = NULL;
		if (GetGameSetting("iLevItemLevelDifferenceMax", &info))
			levelDiff = info->i;
	}

	TESForm* item = levList->CalcElement(level, (useChanceNone ? true : false), levelDiff, noRecurse);
	if (item)
		*refResult = item->refID;

	return true;
}

static bool Cmd_CalcLeveledItem_Execute(COMMAND_ARGS)
{
	CalcLevItem_Execute(PASS_COMMAND_ARGS, false);
	return true;
}

static bool Cmd_CalcLeveledItemNR_Execute(COMMAND_ARGS)
{
	CalcLevItem_Execute(PASS_COMMAND_ARGS, true);
	return true;
}

static bool Cmd_GetLevItemByLevel_Execute(COMMAND_ARGS)
{
	UInt32 lev = 0;
	TESForm* form = NULL;
	TESLeveledList* levList = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &lev, &form))
		levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);

	if (levList)
	{
		form = levList->GetElementByLevel(lev);
		if (form)
			*refResult = form->refID;
	}

	return true;
}

static bool Cmd_SetChanceNone_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 newChance = 0;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &newChance, &form))
	{
		TESLeveledList* levList = OBLIVION_CAST(form, TESForm, TESLeveledList);
		if (levList && newChance <= 100)
			levList->chanceNone = newChance;
	}

	return true;
}

static bool Cmd_RemoveLevItemByLevel_Execute(COMMAND_ARGS)
{
	UInt32 lev = 0;
	TESForm* form = NULL;
	TESLeveledList* levList = NULL;
	*result = 0;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &lev, &form))
		levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);

	if (levList)
		*result = levList->RemoveByLevel(lev);

	return true;
}

static TESLeveledList* ExtractLeveledList(COMMAND_ARGS)
{
	TESForm* form = NULL;
	TESLeveledList* levList = NULL;
	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form))
		levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);

	return levList;
}

static bool Cmd_GetChanceNone_Execute(COMMAND_ARGS)
{
	*result = -1;

	TESLeveledList* levList = ExtractLeveledList(PASS_COMMAND_ARGS);
	if (levList)
		*result = levList->chanceNone;

	return true;
}

static bool Cmd_GetCalcAllLevels_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESLeveledList* levList = ExtractLeveledList(PASS_COMMAND_ARGS);
	if (levList && (levList->flags & levList->kFlags_CalcAllLevels))
		*result = 1;

	return true;
}

static bool Cmd_GetCalcEachInCount_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESLeveledList* levList = ExtractLeveledList(PASS_COMMAND_ARGS);
	if (levList && (levList->flags & levList->kFlags_CalcEachInCount))
		*result = 1;

	return true;
}

static bool Cmd_GetNumLevItems_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESLeveledList* levList = ExtractLeveledList(PASS_COMMAND_ARGS);
	if (levList)
		*result = LeveledListVisitor(&levList->list).Count();

	if (IsConsoleMode())
		Console_Print("GetNumLevitems >> %.0f", *result);

	return true;
}

static bool Cmd_GetNthLevItem_Execute(COMMAND_ARGS)
{
	UInt32 index = 0;
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &index, &form))
		return true;

	TESLeveledList* levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	if (levList)
	{
		TESLeveledList::ListData* data = LeveledListVisitor(&levList->list).GetNthInfo(index);
		if (data && data->form)
		{
			*refResult = data->form->refID;
			if (IsConsoleMode())
				Console_Print("GetNthLevItem >> %s (%08x)", GetFullName(data->form), *refResult);
		}
	}

	return true;
}

static bool Cmd_GetNthLevItemCount_Execute(COMMAND_ARGS)
{
	UInt32 index = 0;
	TESForm* form = NULL;
	*result = 0;

	if (!ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &index, &form))
		return true;

	TESLeveledList* levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	if (levList)
	{
		TESLeveledList::ListData* data = LeveledListVisitor(&levList->list).GetNthInfo(index);
		if (data)
			*result = data->count;
	}
	return true;
}

static bool Cmd_GetNthLevItemLevel_Execute(COMMAND_ARGS)
{
	UInt32 index = 0;
	TESForm* form = NULL;
	*result = 0;

	if (!ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &index, &form))
		return true;

	TESLeveledList* levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	if (levList)
	{
		TESLeveledList::ListData* data = LeveledListVisitor(&levList->list).GetNthInfo(index);
		if (data)
			*result = data->level;
	}
	return true;
}

static bool Cmd_ClearLeveledList_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	if (!ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form))
		return true;

	TESLeveledList* levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	if (levList && levList->list.data)
		LeveledListVisitor(&levList->list).RemoveAll();

	return true;
}

static bool Cmd_RemoveNthLevItem_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 index = 0;
	TESLeveledList* levList = NULL;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &index, &form))
		levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);

	if (levList)
		levList->RemoveNthItem(index);

	return true;
}

static bool Cmd_GetLevItemIndexByLevel_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 level = 0;
	TESLeveledList* levList = NULL;
	*result = -1;
	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &level, &form))
		levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);

	if (levList)
	{
		UInt32 idx = levList->GetItemIndexByLevel(level);
		if (idx != -1)
			*result = idx;
	}

	return true;
}

static bool Cmd_GetLevItemIndexByForm_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	TESForm* formToMatch = NULL;
	TESLeveledList* levList = NULL;
	*result = -1;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form, &formToMatch))
		levList = (TESLeveledList*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESLeveledList, 0);
	
	if (levList && formToMatch)
	{
		UInt32 idx = levList->GetItemIndexByForm(formToMatch);
		if (idx != -1)
			*result = idx;
	}

	return true;
}

static bool Cmd_DumpLevList_Execute(COMMAND_ARGS)
{
	TESLeveledList* levList = ExtractLeveledList(PASS_COMMAND_ARGS);
	if (levList)
		levList->Dump();

	return true;
}

static bool Cmd_CalcLevItems_Execute(COMMAND_ARGS)
{
	TESForm* levItemForm = NULL;
	UInt32 level = 0;
	UInt32 count = 1;

	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &levItemForm, &level, &count) && levItemForm)
	{
		TESLevItem* levItem = OBLIVION_CAST(levItemForm, TESForm, TESLevItem);
		if (levItem)
		{
			TESContainer* cont = (TESContainer*)FormHeap_Allocate(sizeof(TESContainer));
			CALL_MEMBER_FN(cont, Constructor)();
			CALL_MEMBER_FN(&levItem->leveledList, CalcItemForContainer)(level, count, cont);

			// copy items to the array
			UInt32 idx = 0;
			for (TESContainer::Entry* cur = &cont->list; cur && cur->data; cur = cur->next)
			{
				if (cur->data->count > 0)
				{
					ArrayID smap = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
					g_ArrayMap.SetElementFormID(smap, "item", cur->data->type->refID);
					g_ArrayMap.SetElementNumber(smap, "count", cur->data->count);
					g_ArrayMap.SetElementArray(arrID, idx, smap);
					idx++;
				}
			}

			CALL_MEMBER_FN(cont, Destructor)();
		}
	}

	return true;
}

static bool Cmd_GetLevCreatureTemplate_Execute(COMMAND_ARGS)
{
	TESForm* levCreature = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &levCreature)) {
		if (levCreature)	{
			TESLevCreature* base = OBLIVION_CAST(levCreature, TESForm, TESLevCreature);

			if (base && base->templateForm) {
				*refResult = base->templateForm->refID;
			}
		}
	}

	return true;
}

static bool Cmd_SetLevCreatureTemplate_Execute(COMMAND_ARGS)
{
	TESForm* levCreature = NULL;
	TESForm* templateActor = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &levCreature, &templateActor)) {
		if (levCreature)	{
			TESLevCreature* base = OBLIVION_CAST(levCreature, TESForm, TESLevCreature);
			TESActorBase* temp = OBLIVION_CAST(templateActor, TESForm, TESActorBase);

			if (base) {
				base->templateForm = temp;
			}
		}
	}

	return true;
}

static bool Cmd_SetCalcAllLevels_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESForm* list = NULL;
	UInt32 state = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &list, &state)) {
		if (list)	{
			TESLeveledList* levList = OBLIVION_CAST(list, TESForm, TESLeveledList);

			if (levList) {
				if (state)
					levList->flags |= levList->kFlags_CalcAllLevels;
				else
					levList->flags &= ~levList->kFlags_CalcAllLevels;
			}
		}
	}
	
	return true;
}

#endif

static ParamInfo kParams_TwoInventoryObjects_TwoOptionalInts[4] =
{
	{	"leveled list", kParamType_InventoryObject, 0 },
	{	"item to add",	kParamType_InventoryObject, 0 },
	{	"level",		kParamType_Integer,			1 },
	{	"count",		kParamType_Integer,			1},
};

CommandInfo kCommandInfo_AddToLeveledList =
{
	"AddToLeveledList",
	"AddLevList",
	0,
	"adds an object to a leveled list",
	0,
	4,
	kParams_TwoInventoryObjects_TwoOptionalInts,
	HANDLER(Cmd_AddToLeveledList_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_TwoInventoryObjects[2] =
{
	{	"leveled list",		kParamType_InventoryObject, 0 },
	{	"item to remove",	kParamType_InventoryObject, 0 },
};

CommandInfo kCommandInfo_RemoveFromLeveledList =
{
	"RemoveFromLeveledList",
	"RemLevList",
	0,
	"removes all occurrences of an object from a leveled list",
	0,
	2,
	kParams_TwoInventoryObjects,
	HANDLER(Cmd_RemoveFromLeveledList_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_CalcLeveledItem[4] = 
{
	{	"leveled list",			kParamType_InventoryObject, 0	},
	{	"level",				kParamType_Integer,			0	},
	{	"chance none flag",		kParamType_Integer,			1	},
	{	"min level difference",	kParamType_Integer,			1	},
};

CommandInfo kCommandInfo_CalcLeveledItem = 
{
	"CalcLeveledItem",
	"CalcLevItem",
	0,
	"chooses a random item for a given level from the list",
	0,
	4,
	kParams_CalcLeveledItem,
	HANDLER(Cmd_CalcLeveledItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_CalcLeveledItemNR = 
{
	"CalcLeveledItemNR",
	"CalcLevItemNR",
	0,
	"returns the index of an randomly chosen element from a leveled list",
	0,
	4,
	kParams_CalcLeveledItem,
	HANDLER(Cmd_CalcLeveledItemNR_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetChanceNone =
{
	"GetChanceNone",
	"",
	0,
	"returns the chance a leveled list will return nothing",
	0,
	1,
	kParams_OneInventoryObject,
	HANDLER(Cmd_GetChanceNone_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCalcAllLevels =
{
	"GetCalcAllLevels",
	"",
	0,
	"returns 1 if the Calc All Levels flag is set on the leveled list",
	0,
	1,
	kParams_OneInventoryObject,
	HANDLER(Cmd_GetCalcAllLevels_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCalcEachInCount =
{
	"GetCalcEachInCount",
	"",
	0,
	"returns 1 if the Calc Each In Count flag is set for the leveled list",
	0,
	1,
	kParams_OneInventoryObject,
	HANDLER(Cmd_GetCalcEachInCount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumLevItems =
{
	"GetNumLevItems",
	"",
	0,
	"returns the number of elements in a leveled list",
	0,
	1,
	kParams_OneInventoryObject,
	HANDLER(Cmd_GetNumLevItems_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetNthLevItem[2] =
{
	{	"index",		kParamType_Integer,			0	},
	{	"leveled list",	kParamType_InventoryObject,	0	},
};

CommandInfo kCommandInfo_GetNthLevItem =
{
	"GetNthLevItem",
	"",
	0,
	"returns the nth element of a leveled list",
	0,
	2,
	kParams_GetNthLevItem,
	HANDLER(Cmd_GetNthLevItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(ClearLeveledList,
			   clears all data from a leveled list,
			   0,
			   1,
			   kParams_OneInventoryObject);

DEFINE_COMMAND(GetNthLevItemCount,
			   returns the count of the nth element of a leveled list,
			   0,
			   2,
			   kParams_GetNthLevItem);
DEFINE_COMMAND(GetNthLevItemLevel,
			   returns the level of the nth element of a leveled list,
			   0,
			   2,
			   kParams_GetNthLevItem);

DEFINE_COMMAND(GetLevItemByLevel,
			   returns the first element of the specified level,
			   0,
			   2,
			   kParams_GetNthLevItem);

DEFINE_COMMAND(RemoveLevItemByLevel,
			   removes all elements of the specified level,
			   0,
			   2,
			   kParams_GetNthLevItem);

DEFINE_COMMAND(RemoveNthLevItem,
			   removes the nth element of a leveled list,
			   0,
			   2,
			   kParams_GetNthLevItem);

DEFINE_COMMAND(GetLevItemIndexByLevel,
			   returns the index of the first item in the list with the specified level,
			   0,
			   2,
			   kParams_GetNthLevItem);

DEFINE_COMMAND(GetLevItemIndexByForm,
			   returns the index of the first occurrence of the specified form in a leveled list,
			   0,
			   2,
			   kParams_TwoInventoryObjects);

DEFINE_COMMAND(DumpLevList,
			   dumps the contents of a list to the console for debugging,
			   0,
			   1,
			   kParams_OneInventoryObject);

DEFINE_COMMAND(SetChanceNone,
			   sets the chance that a leveled list returns no object,
			   0,
			   2,
			   kParams_GetNthLevItem);

static ParamInfo kParams_CalcLevItems[3] =
{
	{	"leveledItemList",	kParamType_InventoryObject,	0	},
	{	"level",			kParamType_Integer,			0	},
	{	"count",			kParamType_Integer,			1	},
};

DEFINE_COMMAND(CalcLevItems, returns an Array containing items calculated from a leveled item list,
			   0, 3, kParams_CalcLevItems);

static ParamInfo kParams_GetLevCreatureTemplate[1] =
{
	{	"leveledCreature",		kParamType_TESObject,	0	},
};

static ParamInfo kParams_SetLevCreatureTemplate[2] =
{
	{	"leveledCreature",		kParamType_TESObject,	0	},
	{	"template",				kParamType_ActorBase,	1	}
};

static ParamInfo kParams_SetCalcAllLevels[2] =
{
	{	"leveledlist",			kParamType_TESObject,	0	},
	{	"state",				kParamType_Integer,	0	}
};

DEFINE_COMMAND(GetLevCreatureTemplate, gets the template of the leveled creature, 0, 1, kParams_GetLevCreatureTemplate);
DEFINE_COMMAND(SetLevCreatureTemplate, sets the template of the leveled creature, 0, 2, kParams_SetLevCreatureTemplate);
DEFINE_COMMAND(SetCalcAllLevels, sets the Calc On All Levels flag on the leveled list, 0, 2, kParams_SetCalcAllLevels);
