#include "CommandTable.h"
#include "Commands_Inventory.h"
#include "Commands_Input.h"
#include "Commands_FileIO.h"
#include "Commands_Console.h"
#include "Commands_Math.h"
#include "Commands_Player.h"
#include "Commands_Game.h"
#include "Commands_Magic.h"
#include "Commands_MagicEffect.h"
#include "Commands_Weather.h"
#include "Commands_ActiveEffect.h"
#include "Commands_MiscReference.h"
#include "Commands_Faction.h"
#include "Commands_LeveledList.h"
#include "Commands_Creature.h"
#include "Commands_Script.h"
#include "Commands_AI.h"
#include "Commands_Menu.h"
#include "Commands_String.h"
#include "Commands_TextInput.h"
#include "Commands_MiscForms.h"
#include "Commands_Array.h"
#include "Commands_General.h"
#include "Commands_CombatStyle.h"
#include "Commands_Class.h"
#include "Commands_Race.h"
#include "Commands_Cell.h"
#include "Commands_ActorValue.h"
#include "Commands_Actor.h"
#include "Commands_InventoryRef.h"
#include "Commands_PathGrid.h"
#include "Commands_Physics.h"
#include "Commands_Sound.h"
#include "Commands_Quest.h"

#include "ParamInfos.h"
#include "PluginManager.h"
#include "Hooks_Memory.h"
#include "obse_common/SafeWrite.h"
#include "obse_common/obse_version.h"
#include "Utilities.h"
#include "Script.h"

/*

// arg0 - ParamInfo *
// arg1 - void * scriptData - contains argument data
// arg2 - TESObjectREFR * to the 'this' object (confirmed)
// arg3 - script instance state (contains local vars)
// arg4 - Script * to the script form
// arg5 - object event linked list
// arg6 - floating point result output (double!)
// arg7 - UInt16 * scriptData offset - offset in to scriptData
// opcodeOffsetPtr - UInt16 *, offset in to scriptData
bool Command_GetPos(unk arg0, unk arg1, unk arg2, unk arg3, unk arg4, unk arg5, unk arg6, unk opcodeOffsetPtr)
{
	bool	result = false;

	char	axis;

	if(ExtractArgs(arg0, arg1, opcodeOffsetPtr, arg2, arg3, arg4, arg5, &axis))
	{
		result = HandleGetPos(arg2, axis, 0, arg6);
	}

	return result;
}

*/

#ifdef OBLIVION

#include "GameAPI.h"
#include "GameObjects.h"
#include "GameForms.h"
#include "GameMagicEffects.h"
#include "GameTiles.h"
#include "GameData.h"
#include "GameMenus.h"
#include "GameExtraData.h"

#include "common/IFileStream.h"

bool Cmd_TestExtractArgs_Execute(COMMAND_ARGS)
{
	*result = 0;

	_MESSAGE("TestExtractArgs command executing, script %08x", scriptObj->refID);
	if (thisObj)
		_MESSAGE("thisObj %08x", thisObj->refID);
	return true;

	UInt8	* scriptData = (UInt8 *)arg1;
	UInt8	* scriptDataBase = scriptData;

	scriptData += *opcodeOffsetPtr;

	scriptData -= 2;

	UInt32	opcodeDataLen = *((UInt16 *)scriptData);
	scriptData += 2;

	UInt32	numArgs = *((UInt16 *)scriptData);
	scriptData += 2;

	_MESSAGE("len = %04X numArgs = %04X", opcodeDataLen, numArgs);

	{
		static int	exportID = 0;
		char		name[64];

		sprintf_s(name, sizeof(name), "arg_%d", exportID);
		exportID++;

		IFileStream	out;
		if(out.Create(name))
			out.WriteBuf(scriptData - 2, opcodeDataLen);
	}

	for(UInt32 i = 0; i < numArgs; i++)
	{
		ParamInfo	* info = &paramInfo[i];

		switch(info->typeID)
		{
			case kParamType_InventoryObject:
			{
				UInt8	unk0 = *scriptData++;
				UInt16	varIdx = *((UInt16 *)scriptData);
				Script::RefVariable	* var = scriptObj->GetVariable(varIdx);
				ASSERT(var);

				var->Resolve(eventList);

				_MESSAGE("inventory object %02X %04X %08X (%08X)", unk0, varIdx, var->form,
					var->form ? var->form->refID : 0);
			}
			break;

			default:
				_MESSAGE("unknown type %02X", info->typeID);
				break;
		}
	}

	*opcodeOffsetPtr += opcodeDataLen;

	return true;
}

bool Cmd_GetOBSEVersion_Execute(COMMAND_ARGS)
{
	*result = OBSE_VERSION_INTEGER;
	if (IsConsoleMode()) Console_Print("OBSE Version %d.%d", OBSE_VERSION_INTEGER, OBSE_VERSION_INTEGER_MINOR);
	return true;
}

bool Cmd_GetOBSERevision_Execute(COMMAND_ARGS)
{
	*result = OBSE_VERSION_INTEGER_MINOR;
	return true;
}

static bool Cmd_DumpExtraData_Execute(COMMAND_ARGS)
{
	if (thisObj)
	{
		_MESSAGE("Dumping extra data for ref %08x", thisObj);
		for (BSExtraData* xData = thisObj->baseExtraList.m_data; xData; xData = xData->next)
		{
			_MESSAGE("ExtraData Type: %02x", xData->type);
			DumpClass(xData, 8);
		}
		_MESSAGE(" ");
	}

	return true;
}

DEFINE_COMMAND(DumpExtraData,
			   testing,
			   1,
			   0,
			   NULL);

static void DumpExtraDataList(ExtraDataList * list)
{
	for(BSExtraData * traverse = list->m_data; traverse; traverse = traverse->next)
	{
		Console_Print("%s", GetObjectClassName(traverse));

		if(traverse->type == kExtraData_Worn)
		{
			Console_Print("worn = %02X %02X %02X", traverse->pad[0], traverse->pad[1], traverse->pad[2]);
		}
	}
}

static const UInt32 kMaxSavedIPs = 0x100;
static SavedIPInfo s_savedIPTable[kMaxSavedIPs] = { { 0 } };

bool Cmd_SaveIP_Execute(COMMAND_ARGS)
{
	UInt32	_esi;

	// ### assume nothing modifies esi before we get here
	// ### MAKE SURE THIS IS THE FIRST CODE TO RUN
	// ### the alternative is a __declspec(naked) and __asm wrapper
	__asm { mov _esi, esi }

	// make sure this is only called from the main execution loop
	ASSERT_STR(arg1 == scriptObj->data, "SaveIP may not be called inside a set or if statement");

	UInt32	idx = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &idx)) return true;

	// this must happen after extractargs updates opcodeOffsetPtr so it points to the next instruction
	if(idx < kMaxSavedIPs)
	{
		SavedIPInfo				* info = &s_savedIPTable[idx];
		ScriptExecutionState	* state = (ScriptExecutionState *)_esi;

		info->ip = *opcodeOffsetPtr;
		info->stackDepth = state->stackDepth;
		ASSERT((info->stackDepth + 1) < kMaxSavedIPStack);
		memcpy(info->stack, state->stack, (info->stackDepth + 1) * sizeof(UInt32));
	}

	return true;
}

bool Cmd_RestoreIP_Execute(COMMAND_ARGS)
{

	static const UInt32 kDataDeltaStackOffset = 482;

	UInt32	_esi;

	// ### assume nothing modifies esi before we get here
	__asm { mov _esi, esi }

	// make sure this is only called from the main execution loop
	ASSERT_STR(arg1 == scriptObj->data, "RestoreIP may not be called inside a set or if statement");

	UInt32	idx = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &idx)) return true;

	if(idx < kMaxSavedIPs)
	{
		SavedIPInfo				* info = &s_savedIPTable[idx];
		ScriptExecutionState	* state = (ScriptExecutionState *)_esi;

		// ### this is major stack abuse
		// ### the variable storing the number of data bytes happens to be here
		// ### it will be added to the current instruction pointer after we return
		// ### we change it to fake branching
		opcodeOffsetPtr[kDataDeltaStackOffset] += info->ip - opcodeOffsetPtr[0];

		// restore the if/else/endif stack
		state->stackDepth = info->stackDepth;
		memcpy(state->stack, info->stack, (info->stackDepth + 1) * sizeof(UInt32));
	}

	return true;
}

bool Cmd_Test_Execute(COMMAND_ARGS)
{
	_MESSAGE("Cmd_Test_Execute: %08X %08X %08X (%s) %08X %08X (%s) %08X %08X %08X",
		paramInfo, arg1, thisObj, GetObjectClassName((void *)thisObj), arg3, scriptObj, GetObjectClassName((void *)scriptObj), eventList, result, opcodeOffsetPtr);

#if 1
	InterfaceManager	* interfaceManager = InterfaceManager::GetSingleton();
	if(interfaceManager && interfaceManager->menuRoot)
	{
#if 0
		Tile	* textEditTile = interfaceManager->menuRoot->ReadXML("data\\menus\\options\\load_menu.xml");
		if(textEditTile)
		{
			Tile	* textEditRoot = textEditTile->GetRoot();
			if(textEditRoot)
			{
				TileMenu	* textEditMenuTile = tile_cast <TileMenu>(textEditRoot);
				DumpClass(textEditMenuTile, 20);
				if(textEditMenuTile)
				{
					Menu	* textEditMenu = textEditMenuTile->menu;
					/*LoadMenu* textEdit = (TextEditMenu*)Oblivion_DynamicCast(textEditMenu, 0, RTTI_Menu, RTTI_TextEditMenu, 0);
					if (textEdit) {
						UInt32 x = 0;
					};
					*/
					if(textEditMenu)
					{
						textEditMenu->RegisterTile(textEditMenuTile);
						textEditMenu->EnableMenu(false);
					}
				}
			}
		}
#endif

		interfaceManager->menuRoot->DebugDump();
	}
#endif

#if 0
	for(DataHandler::ModEntry * traverse = &(*g_dataHandler)->modList; traverse; traverse = traverse->next)
	{
		if(traverse->data)
		{
			_MESSAGE("%08X %s", traverse->data->flags, traverse->data->name);
			if(traverse->data->flags & DataHandler::ModEntry::Data::kFlag_Loaded)
			{
				gLog.Indent();

				DumpClass(traverse->data, 0x600 >> 2);

				gLog.Outdent();
			}
		}
	}
#endif

#if 0
	if(thisObj)
		_MESSAGE("thisObj->flags = %08X", thisObj->flags);
	else
		_MESSAGE("thisObj = NULL");

	_MESSAGE("ShowMessageBox_pScriptRefID = %08X", *ShowMessageBox_pScriptRefID);
#endif

	return true;
}

bool Cmd_TestArgs_Execute(COMMAND_ARGS)
{
	_MESSAGE("Cmd_TestArgs_Execute: %08X %08X %08X (%s) %08X %08X (%s) %08X %08X %08X",
		paramInfo, arg1, thisObj, GetObjectClassName((void *)thisObj), arg3, scriptObj, GetObjectClassName((void *)scriptObj), eventList, result, opcodeOffsetPtr);

	UInt32	arg;

	if(ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
	{
		Console_Print("testargcommand: %d", arg);
	}
	else
	{
		Console_Print("testargcommand: couldn't extract args");
	}

	return true;
}

bool Cmd_DumpVarInfo_Execute(COMMAND_ARGS)
{
#if 0
	if(scriptObj)
	{
		Console_Print("script vars %08X", scriptObj);

		UInt32	idx = 0;

		for(Script::RefListEntry * traverse = &scriptObj->refList; traverse; traverse = traverse->next, idx++)
		{
			if(traverse->var)
			{
				Console_Print("%d: %08X %08X %08X (%08X%s%s) %08X",
					idx,
					traverse->var->unk0,
					traverse->var->unk1,
					traverse->var->form,
					traverse->var->form ? traverse->var->form->refID : 0,
					traverse->var->form ? " " : "",
					traverse->var->form ? GetFullName(traverse->var->form) : "",
					traverse->var->varIdx);

				_MESSAGE("%d: %08X %08X %08X (%08X%s%s) %08X",
					idx,
					traverse->var->unk0,
					traverse->var->unk1,
					traverse->var->form,
					traverse->var->form ? traverse->var->form->refID : 0,
					traverse->var->form ? " " : "",
					traverse->var->form ? GetFullName(traverse->var->form) : "",
					traverse->var->varIdx);
			}
		}
	}

	if(eventList)
	{
		Console_Print("event list %08X", eventList);

		UInt32	idx = 0;

		for(ScriptEventList::VarEntry * traverse = eventList->m_vars; traverse; traverse = traverse->next, idx++)
		{
			if(traverse->var)
			{
				Console_Print("%d: %08X %08X %f %016I64X",
					idx,
					traverse->var->id,
					&traverse->var->nextEntry,
					traverse->var->data,
					*((UInt64 *)&traverse->var->data));

				_MESSAGE("%d: %08X %08X %f %016I64X",
					idx,
					traverse->var->id,
					&traverse->var->nextEntry,
					traverse->var->data,
					*((UInt64 *)&traverse->var->data));
			}
		}
	}
#endif

	return true;
}

bool Cmd_DumpDocs_Execute(COMMAND_ARGS)
{
	UInt32 opCodeStart = g_scriptCommands.GetByName("GetMagicEffectCharsC")->opcode;
	if (ExtractArgs(EXTRACT_ARGS, &opCodeStart)) {
		g_scriptCommands.DumpCommandDocumentation(opCodeStart);
	}
	return true;
}

bool Cmd_DumpXmlDocs_Execute(COMMAND_ARGS)
{
	UInt32 opCodeStart = kObseOpCodeStart;
	if (ExtractArgs(EXTRACT_ARGS, &opCodeStart))
		g_scriptCommands.DumpCommandXML(opCodeStart);
	return true;
}

#endif

// nop command handler for script editor
bool Cmd_Default_Execute(COMMAND_ARGS)
{
	return true;
}
// nop command handler for script editor
bool Cmd_Default_Eval(COMMAND_ARGS_EVAL)
{
	return true;
}

// called from 004F90A5
bool Cmd_Default_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
#ifdef _DEBUG
#if 0
	_MESSAGE("Cmd_Default_Parse: %08X %08X %08X %08X",
		arg0, arg1, arg2, arg3);
#endif
#endif

	#ifdef OBLIVION

	static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x004FDE30;
	
	#else

	#if CS_VERSION == CS_VERSION_1_0
	static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x004F69C0;
	#elif CS_VERSION == CS_VERSION_1_2
	static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x00500FF0;
	#else
	#error unsupported cs version
	#endif

	#endif

	// arg0 = idx?
	// arg1 = ParamInfo *
	// arg2 = ptr to line to parse, skip UInt32 header first
	// arg3 = ptr to script info? first UInt32 is ptr to script data

	return g_defaultParseCommand(numParams, paramInfo, lineBuf, scriptBuf);
}

CommandTable::CommandTable()
{
	//
}

CommandTable::~CommandTable()
{
	//
}

static CommandInfo kCommandInfo_DumpVarInfo =
{
	"DumpVarInfo",
	"",
	0,
	"",
	0,
	0,
	NULL,

	HANDLER(Cmd_DumpVarInfo_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static ParamInfo kTestExtractArgs_Params[] =
{
	{	"TESForm",	kParamType_InventoryObject,	0 },
};

static CommandInfo kCommandInfo_TestExtractArgs =
{
	"TestExtractArgs",
	"",
	0,
	"",
	0,
	1,
	kTestExtractArgs_Params,

	HANDLER(Cmd_TestExtractArgs_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static CommandInfo kCommandInfo_GetOBSEVersion =
{
	"GetOBSEVersion",
	"",
	0,
	"returns the installed version of OBSE",
	0,
	0,
	NULL,

	HANDLER(Cmd_GetOBSEVersion_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static CommandInfo kCommandInfo_GetOBSERevision =
{
	"GetOBSERevision",
	"",
	0,
	"returns the numbered revision of the installed version of OBSE",
	0,
	0,
	NULL,

	HANDLER(Cmd_GetOBSERevision_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};
static CommandInfo kTestCommand =
{
	"testcommand",
	"tcmd",
	0,
	"test command for obse",
	0,		// doesn't require parent obj
	0,		// doesn't have params
	NULL,	// no param table

	HANDLER(Cmd_Test_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static ParamInfo kTestArgCommand_Params[] =
{
	{	"int", kParamType_Integer, 0 }
};

static CommandInfo kTestArgCommand =
{
	"testargcommand",
	"tacmd",
	0,
	"test argument command for obse",
	0,
	1,
	kTestArgCommand_Params,

	HANDLER(Cmd_TestArgs_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static CommandInfo kCommandInfo_SaveIP =
{
	"SaveIP",
	"Label",
	0,
	"",
	0,
	1,
	kParams_OneOptionalInt,

	HANDLER(Cmd_SaveIP_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static CommandInfo kCommandInfo_RestoreIP =
{
	"RestoreIP",
	"Goto",
	0,
	"",
	0,
	1,
	kParams_OneOptionalInt,

	HANDLER(Cmd_RestoreIP_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

static CommandInfo kCommandInfo_DumpDocs =
{
	"DumpDocs",
	"",
	0,
	"dump command documentation",
	0,
	1,
	kParams_OneOptionalInt,

	HANDLER(Cmd_DumpDocs_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

DEFINE_COMMAND(DumpXmlDocs, dump docs in xml format, 0, 1, kParams_OneOptionalInt);

// this was defined in oblivion in a patch, but the CS wasn't updated to include it
static CommandInfo kCommandInfo_PurgeCellBuffers =
{
	"PurgeCellBuffers", "pcb",
	0x016B,
	"Forcibly unloads all unattached cells in cell buffers.",
	0,
	0,
	NULL,

	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

//0000116C 0000 SetPlayerInSEWorld SPInSE
//	00000001 00000000 Integer
static CommandInfo kCommandInfo_SetPlayerInSEWorld =
{
	"SetPlayerInSEWorld", "SPInSE",
	0x0000116C,
	"",
	0,
	1,
	kParams_OneInt,
	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

//0000116D 0000 GetPlayerInSEWorld gpInSE
static CommandInfo kCommandInfo_GetPlayerInSEWorld =
{
	"GetPlayerInSEWorld", "gpInSE",
	0x0000116D,
	"",
	0,
	0,
	NULL,

	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

//0000116E 0001 PushActorAway
//	00000004 00000000 ObjectReferenceID
//	00000001 00000000 Integer
static ParamInfo kParams_PushActorAway[2] =
{
	{	"objectRef", kParamType_ObjectRef, 0},
	{	"int", kParamType_Integer, 0}
};

static CommandInfo kCommandInfo_PushActorAway =
{
	"PushActorAway", "",
	0x0000116E,
	"",
	1,
	2,
	kParams_PushActorAway,

	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

//0000116F 0001 SetActorsAI
//	00000001 00000000 Integer
static CommandInfo kCommandInfo_SetActorsAI =
{
	"SetActorsAI", "",
	0x0000116F,
	"",
	1,
	1,
	kParams_OneInt,

	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

static CommandInfo kPaddingCommand =
{
	"", "",
	0,
	"command used for padding",
	0,
	0,
	NULL,

	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

struct PatchLocation
{
	UInt32	ptr;
	UInt32	offset;
	UInt32	type;
};

#ifdef OBLIVION

static const PatchLocation kPatch_ScriptCommands_Start[] =
{
	// 004FCA30
	{	0x004FCA68 + 3,	0x00 },

	// 004FCA70
	{	0x004FCAB9 + 1, 0x04 },
	{	0x004FCB52 + 3,	0x08 },

	// 004FDAF0
	{	0x004FDC79 + 3,	0x00 },

	// 004FF710
	{	0x004FFF6E + 3,	0x00 },

	// 00501540 - print console/script command help
	{	0x005015D4 + 2,	0x0C },
	{	0x005015F0 + 2,	0x04 },
	{	0x0050160F + 2,	0x00 },
	{	0x00501625 + 2,	0x00 },

	// 0056AC50
	{	0x0056ACA5 + 3,	0x20 },
	{	0x0056ACB6 + 2,	0x10 },

	// 0056AE20
	{	0x0056AE82 + 3,	0x20 },
	{	0x0056AF96 + 3,	0x00 },

	// 0056B170
	{	0x0056B181 + 4,	0x12 },

	// 0056B190
	{	0x0056B1AC + 3,	0x12 },
	{	0x0056B1B8 + 2,	0x14 },

	// 0056B1D0
	{	0x0056B1EC + 3,	0x12 },
	{	0x0056B1F8 + 2,	0x14 },

	// 0056B220
	{	0x0056B241 + 4,	0x12 },
	{	0x0056B24F + 2,	0x14 },

	// 0056B2E0
	{	0x0056B332 + 4,	0x12 },
	{	0x0056B385 + 4,	0x12 },
	{	0x0056B397 + 2,	0x14 },
	{	0x0056B44D + 3,	0x14 },
	{	0x0056B4F1 + 4,	0x12 },
	{	0x0056B503 + 2,	0x14 },

	{	0 },
};

static const PatchLocation kPatch_ScriptCommands_End[] =
{
	// 004FCA70
	{	0x004FCAE7 + 2,	0x04 },

	{	0 },
};

// check 00854AC2, unlikely

static const PatchLocation kPatch_ScriptCommands_MaxIdx[] =
{
	// 004F3320
	{	0x004F33A4 + 3,	1,	1 },

	// 004FCA30
	{	0x004FCA59 + 2,	(UInt32)(-0x1000) },

	// 004FCCE0 - assert
//	{	0x004FCCE0 + 6,	1 },

	// 004FDAF0
	{	0x004FDC6B + 1,	(UInt32)(-0x1000) },

	// 004FF710
	{	0x004FFF63 + 2,	(UInt32)(-0x1000) },

	// 0056B170
	{	0x0056B176 + 2, (UInt32)(-0x1000) + 1 },

	// 0056B190
	{	0x0056B196 + 2,	(UInt32)(-0x1000) + 1 },

	// 0056B1D0
	{	0x0056B1D6 + 2,	(UInt32)(-0x1000) + 1 },

	// 0056B220
	{	0x0056B22B + 1, (UInt32)(-0x1000) + 1 },

	// 0056B2E0
	{	0x0056B324 + 1, (UInt32)(-0x1000) + 1 },
	{	0x0056B373 + 1, (UInt32)(-0x1000) + 1 },
	{	0x0056B4DF + 1, (UInt32)(-0x1000) + 1 },

	{	0 },
};


#else // OBLIVION (CS stuff goes here)

#if CS_VERSION == CS_VERSION_1_0

// could patch the stuff around 004F5620 but those are just asserts

// weird things going on at sub_456A20

//	00	longName
//	04	shortName
//	08	opcode
//	0C	helpText
//	10	unk0
//	12	numParams
//	14	params
//	18	unk1
//	1C	unk2
//	20	unk3
//	24	flags

// original = C0 D8 9D 00
static const PatchLocation kPatch_ScriptCommands_Start[] =
{
	{	0x00454E99 + 1,	0 },

	{	0x00457023 + 3,	0 },
	{	0x004575EC + 3,	0 },
	{	0x004577F2 + 3,	0 },
	{	0x004579A1 + 4,	0x12 },
	{	0x004579C9 + 3,	0x12 },
	{	0x004579D5 + 2,	0x14 },
	{	0x00457A09 + 3, 0x12 },
	{	0x00457A15 + 2, 0x14 },
	{	0x00457A50 + 3,	0x12 },
	{	0x00457A5B + 2,	0x14 },
	{	0x00457B06 + 4, 0x12 },
	{	0x00457B31 + 3,	0x12 },
	{	0x00457B3C + 2,	0x14 },
	{	0x00457B8E + 4,	0x12 },
	{	0x00457BB4 + 3,	0x12 },
	{	0x00457BBF + 2,	0x14 },

	{	0x004583AD + 4,	0x12 },
	{	0x004583E3 + 3,	0x12 },
	{	0x004583EE + 2,	0x14 },
	{	0x00458496 + 4,	0x12 },

	{	0x004584D4 + 3,	0x12 },
	{	0x004584DF + 2,	0x14 },
	{	0x004585D6 + 4, 0x12 },
	{	0x00458628 + 3,	0x12 },
	{	0x0045863B + 2,	0x14 },

	{	0x0045880C + 3,	0x12 },
	{	0x0045881B + 2,	0x14 },
	{	0x00458A74 + 4,	0x12 },
	{	0x00458BDE + 3,	0x14 },
	{	0x00458C20 + 3,	0x14 },

	{	0x00458E07 + 4,	0x12 },
	{	0x00458F13 + 4,	0x12 },
	{	0x00458F5F + 3,	0x14 },
	{	0x00459006 + 3,	0x14 },
	{	0x004590EF + 4,	0x12 },
	{	0x004592BC + 3,	0x14 },

	{	0x004F569C + 3,	0 },
	{	0x004F56F3 + 1,	4 },
	{	0x004F578A + 3,	8 },
	{	0x004F8FF0 + 3,	0 },
	{	0 },
};

// original = 78 11 9E 00
static const PatchLocation kPatch_ScriptCommands_End[] =
{
	{	0x00454EBA + 2,	0 },
	{	0x004F571D + 2,	4 },
	{	0 },
};

// original = 6B 11 00 00
static const PatchLocation kPatch_ScriptCommands_MaxIdx[] =
{
	{	0x00456A2D + 1, 1 },
	{	0x00456A44 + 1, (UInt32)(-0x1000) + 1 },
	{	0x00456A58 + 2, (UInt32)(-0x1000) + 1 },

	{	0x00457996 + 2,	(UInt32)(-0x1000) + 1 },
	{	0x004579B6 + 2, (UInt32)(-0x1000) + 1 },
	{	0x004579F6 + 2, (UInt32)(-0x1000) + 1 },
	{	0x00457A3B + 1, (UInt32)(-0x1000) + 1 },
	{	0x00457AFC + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00457B24 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00457B84 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00457BA7 + 1,	(UInt32)(-0x1000) + 1 },

	{	0x0045839F + 1,	(UInt32)(-0x1000) + 1 },
	{	0x004583D6 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00458488 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x004584C3 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x004585C8 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00458617 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x004587FB + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00458A64 + 2,	(UInt32)(-0x1000) + 1 },
	{	0x00458DFD + 1,	(UInt32)(-0x1000) + 1 },
	{	0x00458F05 + 1,	(UInt32)(-0x1000) + 1 },
	{	0x004590E5 + 1,	(UInt32)(-0x1000) + 1 },

	{	0x004EF049 + 3,	1, 1 },
	{	0x004F568D + 2,	1 },
	{	0x004F8FE2 + 1,	1 },
	{	0 }
};

#elif CS_VERSION == CS_VERSION_1_2

// 10 36 9F 00
static const PatchLocation kPatch_ScriptCommands_Start[] =
{
	// 00455FF0
	{	0x00456009 + 1,	0x00 },

	// 004578B0
	{	0x00457905 + 3,	0x20 },
	{	0x00457916 + 2,	0x10 },

	// 004581B0
	{	0x00458212 + 3,	0x20 },
	{	0x00458326 + 3,	0x00 },

	// 004588A0
	{	0x0045892B + 3,	0x00 },

	// 00458AE0
	{	0x00458B4C + 3,	0x00 },

	// 00458CF0
	{	0x00458D01 + 4,	0x12 },

	// 00458D10
	{	0x00458D33 + 3,	0x14 },

	// 00458D50
	{	0x00458D73 + 3,	0x14 },

	// 00458DA0
	{	0x00458DC3 + 3,	0x14 },

	// 004592B0
	{	0x004592D0 + 3,	0x14 },
	{	0x00459604 + 3,	0x14 },

	// 00459CB0
	{	0x00459E35 + 3,	0x14 },
	{	0x00459E77 + 3,	0x14 },

	// 0045A130
	{	0x0045A1C0 + 3,	0x14 },
	{	0x0045A275 + 3,	0x14 },

	// 0045A380
	{	0x0045A529 + 3,	0x14 },

	// 004F59B0
	{	0x004F5A37 + 1,	0x24 },

	// 004FFBE0
	{	0x004FFC18 + 3,	0x00 },

	// 0x004FFC20
	{	0x004FFC69 + 1,	0x04 },
	{	0x004FFD02 + 3,	0x08 },

	{	0 },
};

// B8 6F 9F 00
static const PatchLocation kPatch_ScriptCommands_End[] =
{
	// 00455FF0
	{	0x0045602B + 2,	0x00 },

	// 004F59B0
	{	0x004F5A64 + 2,	0x24 },

	// 004FFBA0 - assert

	// 004FFC20
	{	0x004FFC97 + 2,	0x04 },

	{	0 },
};

// 1170 / 170 / 171
static const PatchLocation kPatch_ScriptCommands_MaxIdx[] =
{
	// 00457DA0 - suspicious, but seems to be unrelated?

	// 00458CF0
	{	0x00458CF6 + 2,	(UInt32)(-0x1000) + 1 },

	// 00458D10
	{	0x00458D17 + 2,	(UInt32)(-0x1000) + 1 },

	// 00458D50
	{	0x00458D57 + 2,	(UInt32)(-0x1000) + 1 },

	// 00458DA0
	{	0x00458DA7 + 2,	(UInt32)(-0x1000) + 1 },

	// 004F92A0
	{	0x004F9324 + 3,	1, 1 },

	// 004FFBE0
	{	0x004FFC09 + 2,	(UInt32)(-0x1000) },

	{	0 },
};

#else

#error unsupported cs version

#endif

#endif

static void ApplyPatch(const PatchLocation * patch, UInt32 newData)
{
	for(; patch->ptr; ++patch)
	{
		switch(patch->type)
		{
			case 0:
				SafeWrite32(patch->ptr, newData + patch->offset);
				break;

			case 1:
				SafeWrite16(patch->ptr, newData + patch->offset);
				break;
		}
	}
}

void ImportConsoleCommand(const char * name)
{
	CommandInfo	* info = g_consoleCommands.GetByName(name);
	if(info)
	{
		CommandInfo	infoCopy = *info;

		std::string	newName;

		newName = std::string("con_") + name;

		infoCopy.shortName = "";
		infoCopy.longName = _strdup(newName.c_str());	// this leaks but meh

		g_scriptCommands.Add(&infoCopy);

//		_MESSAGE("imported console command %s", name);
	}
	else
	{
		_WARNING("couldn't find console command (%s)", name);

		// pad it
		g_scriptCommands.Add(&kPaddingCommand);
	}
}

#define ADD_CMD(x) g_scriptCommands.Add(&kCommandInfo_##x)
#define ADD_CMD_RET(x, y) g_scriptCommands.Add(&kCommandInfo_##x, y)

void CommandTable::Init(void)
{
	g_consoleCommands.SetBaseID(0x0100);
	g_scriptCommands.SetBaseID(0x1000);

	static CommandInfo* kCmdInfo_OnYield;
#ifdef OBLIVION

	g_consoleCommands.Read((CommandInfo *)0x00B0B420, (CommandInfo *)0x00B0C898);
	g_scriptCommands.Read((CommandInfo *)0x00B0C8C0, (CommandInfo *)0x00B10268);

	kCmdInfo_OnYield = (CommandInfo*)0x00B0B060;

#else

#if CS_VERSION == CS_VERSION_1_0

	g_consoleCommands.Read((CommandInfo *)0x009DC420, (CommandInfo *)0x009DD898);
	g_scriptCommands.Read((CommandInfo *)0x009DD8C0, (CommandInfo *)0x009E1178);
//	g_tokenCommands.Read((CommandInfo *)0x009DB2D4, (CommandInfo *)0x009DB554);	// keeping around just for addresses, format is different

	// extra commands in 1.1
	g_scriptCommands.Add(&kCommandInfo_PurgeCellBuffers);

	// extra commands in 1.2
	g_scriptCommands.Add(&kCommandInfo_SetPlayerInSEWorld);
	g_scriptCommands.Add(&kCommandInfo_GetPlayerInSEWorld);
	g_scriptCommands.Add(&kCommandInfo_PushActorAway);
	g_scriptCommands.Add(&kCommandInfo_SetActorsAI);

	kCmdInfo_OnYield = (CommandInfo*)0x009DC060;
#elif CS_VERSION == CS_VERSION_1_2

	g_consoleCommands.Read((CommandInfo *)0x009F2170, (CommandInfo *)0x009F35E8);
	g_scriptCommands.Read((CommandInfo *)0x009F3610, (CommandInfo *)0x009F6FB8);

	kCmdInfo_OnYield = (CommandInfo*)0x009F1DB0;
#else
#error unsupported cs version
#endif

#endif

	// blocktype "OnYield_Unused" becomes "Function"
	UInt16 onYieldOpcode = kCmdInfo_OnYield->opcode;
	*kCmdInfo_OnYield = kCommandInfo_Function;
	kCmdInfo_OnYield->opcode = onYieldOpcode;

	// record return type of vanilla commands which return forms
	g_scriptCommands.SetReturnType(0x1025, kRetnType_Form);		// PlaceAtMe
	g_scriptCommands.SetReturnType(0x10CD, kRetnType_Form);		// GetActionRef
	g_scriptCommands.SetReturnType(0x10CE, kRetnType_Form);		// GetSelf
	g_scriptCommands.SetReturnType(0x10CF, kRetnType_Form);		// GetContainer
	g_scriptCommands.SetReturnType(0x10E8, kRetnType_Form);		// GetCombatTarget
	g_scriptCommands.SetReturnType(0x10E9, kRetnType_Form);		// GetPackageTarget
	g_scriptCommands.SetReturnType(0x1113, kRetnType_Form);		// GetParentRef
	g_scriptCommands.SetReturnType(0x1167, kRetnType_Form);		// CreateFullActorCopy

	// pad to opcode 0x1400 to give Bethesda lots of room
	g_scriptCommands.PadTo(kObseOpCodeStart);

	// add the new commands
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kTestCommand);
	g_scriptCommands.Add(&kTestArgCommand);
	g_scriptCommands.Add(&kCommandInfo_GetNumItems);
	g_scriptCommands.Add(&kCommandInfo_GetInventoryItemType);
	g_scriptCommands.Add(&kCommandInfo_IsKeyPressed);
	g_scriptCommands.Add(&kCommandInfo_GetEquipmentSlotType);
	g_scriptCommands.Add(&kCommandInfo_PrintToConsole);
	g_scriptCommands.Add(&kCommandInfo_GetActiveSpell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetActiveSpell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SquareRoot);
	g_scriptCommands.Add(&kCommandInfo_Sin);
	g_scriptCommands.Add(&kCommandInfo_Cos);
	g_scriptCommands.Add(&kCommandInfo_Tan);
	g_scriptCommands.Add(&kCommandInfo_ASin);
	g_scriptCommands.Add(&kCommandInfo_ACos);
	g_scriptCommands.Add(&kCommandInfo_ATan);
	g_scriptCommands.Add(&kCommandInfo_Log);
	g_scriptCommands.Add(&kCommandInfo_Exp);
	g_scriptCommands.Add(&kCommandInfo_GetParentCell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_Log10);
	g_scriptCommands.Add(&kCommandInfo_Floor);
	g_scriptCommands.Add(&kCommandInfo_Ceil);
	g_scriptCommands.Add(&kCommandInfo_Abs);
	g_scriptCommands.Add(&kCommandInfo_Rand);
	g_scriptCommands.Add(&kCommandInfo_Pow);
	g_scriptCommands.Add(&kCommandInfo_ATan2);
	g_scriptCommands.Add(&kCommandInfo_Sinh);
	g_scriptCommands.Add(&kCommandInfo_Cosh);
	g_scriptCommands.Add(&kCommandInfo_Tanh);
	g_scriptCommands.Add(&kCommandInfo_dSin);
	g_scriptCommands.Add(&kCommandInfo_dCos);
	g_scriptCommands.Add(&kCommandInfo_dTan);
	g_scriptCommands.Add(&kCommandInfo_dASin);
	g_scriptCommands.Add(&kCommandInfo_dACos);
	g_scriptCommands.Add(&kCommandInfo_dATan);
	g_scriptCommands.Add(&kCommandInfo_dATan2);
	g_scriptCommands.Add(&kCommandInfo_dSinh);
	g_scriptCommands.Add(&kCommandInfo_dCosh);
	g_scriptCommands.Add(&kCommandInfo_dTanh);
	g_scriptCommands.Add(&kCommandInfo_GetInventoryObject, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedObject, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsKeyPressed2);
	g_scriptCommands.Add(&kCommandInfo_TapKey);
	g_scriptCommands.Add(&kCommandInfo_HoldKey);
	g_scriptCommands.Add(&kCommandInfo_ReleaseKey);
	g_scriptCommands.Add(&kCommandInfo_HammerKey);
	g_scriptCommands.Add(&kCommandInfo_AHammerKey);
	g_scriptCommands.Add(&kCommandInfo_UnHammerKey);
	g_scriptCommands.Add(&kCommandInfo_DisableKey);
	g_scriptCommands.Add(&kCommandInfo_EnableKey);
	g_scriptCommands.Add(&kCommandInfo_MoveMouseX);
	g_scriptCommands.Add(&kCommandInfo_MoveMouseY);
	g_scriptCommands.Add(&kCommandInfo_SetMouseSpeedX);
	g_scriptCommands.Add(&kCommandInfo_SetMouseSpeedY);
	g_scriptCommands.Add(&kCommandInfo_DisableMouse);
	g_scriptCommands.Add(&kCommandInfo_EnableMouse);
	g_scriptCommands.Add(&kCommandInfo_GetOBSEVersion);
	ImportConsoleCommand("SetGameSetting");
	ImportConsoleCommand("SetINISetting");
	ImportConsoleCommand("GetINISetting");
	ImportConsoleCommand("SetFog");
	ImportConsoleCommand("SetClipDist");
	ImportConsoleCommand("SetImageSpaceGlow");
	ImportConsoleCommand("ToggleDetection");
	ImportConsoleCommand("SetCameraFOV");
	ImportConsoleCommand("SexChange");
	ImportConsoleCommand("RefreshINI");
	ImportConsoleCommand("HairTint");
	ImportConsoleCommand("SetTargetRefraction");
	ImportConsoleCommand("SetTargetRefractionFire");
	ImportConsoleCommand("SetSkyParam");
	ImportConsoleCommand("RunMemoryPass");
	ImportConsoleCommand("ModWaterShader");
	ImportConsoleCommand("WaterShallowColor");
	ImportConsoleCommand("WaterDeepColor");
	ImportConsoleCommand("WaterReflectionColor");
	ImportConsoleCommand("SetGamma");
	ImportConsoleCommand("SetHDRParam");
	g_scriptCommands.Add(&kCommandInfo_GetCurrentValue);
	g_scriptCommands.Add(&kCommandInfo_GetObjectValue);
	g_scriptCommands.Add(&kCommandInfo_GetBaseObject, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetType);
	g_scriptCommands.Add(&kCommandInfo_IsWeapon);
	g_scriptCommands.Add(&kCommandInfo_IsAmmo);
	g_scriptCommands.Add(&kCommandInfo_IsArmor);
	g_scriptCommands.Add(&kCommandInfo_IsClothing);
	g_scriptCommands.Add(&kCommandInfo_IsBook);
	g_scriptCommands.Add(&kCommandInfo_IsIngredient);
	g_scriptCommands.Add(&kCommandInfo_IsContainer);
	g_scriptCommands.Add(&kCommandInfo_IsKey);
	g_scriptCommands.Add(&kCommandInfo_IsAlchemyItem);
	g_scriptCommands.Add(&kCommandInfo_IsApparatus);
	g_scriptCommands.Add(&kCommandInfo_IsSoulGem);
	g_scriptCommands.Add(&kCommandInfo_IsSigilStone);
	g_scriptCommands.Add(&kCommandInfo_IsDoor);
	g_scriptCommands.Add(&kCommandInfo_IsActivator);
	g_scriptCommands.Add(&kCommandInfo_IsLight);
	g_scriptCommands.Add(&kCommandInfo_IsFurniture);
	g_scriptCommands.Add(&kCommandInfo_HasSpell);
	g_scriptCommands.Add(&kCommandInfo_GetClass, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsClassSkill);
	g_scriptCommands.Add(&kCommandInfo_GetClassAttribute);
	g_scriptCommands.Add(&kCommandInfo_GetClassSkill);
	g_scriptCommands.Add(&kCommandInfo_GetClassSpecialization);
	g_scriptCommands.Add(&kCommandInfo_ModActorValue2);
	g_scriptCommands.Add(&kCommandInfo_SetNumericGameSetting);
	g_scriptCommands.Add(&kCommandInfo_GetControl);
	g_scriptCommands.Add(&kCommandInfo_GetAltControl);
	g_scriptCommands.Add(&kCommandInfo_GetNumKeysPressed);
	g_scriptCommands.Add(&kCommandInfo_GetKeyPress);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedCurrentValue);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedObjectValue);
	g_scriptCommands.Add(&kCommandInfo_GetMagicItemValue);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectValue);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectCodeValue);
	g_scriptCommands.Add(&kCommandInfo_MagicItemHasEffect);
	g_scriptCommands.Add(&kCommandInfo_MagicItemHasEffectCode);
	g_scriptCommands.Add(&kCommandInfo_GetNumMouseButtonsPressed);
	g_scriptCommands.Add(&kCommandInfo_GetMouseButtonPress);

	// v0009
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_IsRefEssential);
	g_scriptCommands.Add(&kCommandInfo_SetRefEssential);
	g_scriptCommands.Add(&kCommandInfo_GetWeight);
	g_scriptCommands.Add(&kCommandInfo_SetWeight);
	g_scriptCommands.Add(&kCommandInfo_ModWeight);
	g_scriptCommands.Add(&kCommandInfo_GetGoldValue);
	g_scriptCommands.Add(&kCommandInfo_SetGoldValue);
	g_scriptCommands.Add(&kCommandInfo_ModGoldValue);
	g_scriptCommands.Add(&kCommandInfo_GetObjectHealth);
	g_scriptCommands.Add(&kCommandInfo_SetObjectHealth);
	g_scriptCommands.Add(&kCommandInfo_ModObjectHealth);
	g_scriptCommands.Add(&kCommandInfo_GetEquipmentSlot);
	g_scriptCommands.Add(&kCommandInfo_SetEquipmentSlot);
	g_scriptCommands.Add(&kCommandInfo_GetObjectCharge);
	g_scriptCommands.Add(&kCommandInfo_SetObjectCharge);
	g_scriptCommands.Add(&kCommandInfo_ModObjectCharge);
	g_scriptCommands.Add(&kCommandInfo_IsQuestItem);
	g_scriptCommands.Add(&kCommandInfo_SetQuestItem);
	g_scriptCommands.Add(&kCommandInfo_GetEnchantment, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetEnchantment, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_RemoveEnchantment, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetAttackDamage);
	g_scriptCommands.Add(&kCommandInfo_SetAttackDamage);
	g_scriptCommands.Add(&kCommandInfo_ModAttackDamage);
	g_scriptCommands.Add(&kCommandInfo_GetWeaponReach);
	g_scriptCommands.Add(&kCommandInfo_SetWeaponReach);
	g_scriptCommands.Add(&kCommandInfo_ModWeaponReach);
	g_scriptCommands.Add(&kCommandInfo_GetWeaponSpeed);
	g_scriptCommands.Add(&kCommandInfo_SetWeaponSpeed);
	g_scriptCommands.Add(&kCommandInfo_ModWeaponSpeed);
	g_scriptCommands.Add(&kCommandInfo_GetWeaponType);
	g_scriptCommands.Add(&kCommandInfo_SetWeaponType);
	g_scriptCommands.Add(&kCommandInfo_GetIgnoresResistance);
	g_scriptCommands.Add(&kCommandInfo_SetIgnoresResistance);
	g_scriptCommands.Add(&kCommandInfo_GetArmorAR);
	g_scriptCommands.Add(&kCommandInfo_SetArmorAR);
	g_scriptCommands.Add(&kCommandInfo_ModArmorAR);
	g_scriptCommands.Add(&kCommandInfo_GetArmorType);
	g_scriptCommands.Add(&kCommandInfo_SetArmorType);
	g_scriptCommands.Add(&kCommandInfo_SoulLevel);
	g_scriptCommands.Add(&kCommandInfo_GetSoulGemCapacity);
	g_scriptCommands.Add(&kCommandInfo_IsFood);
	g_scriptCommands.Add(&kCommandInfo_SetIsFood);
	g_scriptCommands.Add(&kCommandInfo_IsPoison);
	g_scriptCommands.Add(&kCommandInfo_SetName);
	g_scriptCommands.Add(&kCommandInfo_SetModelPath);
	g_scriptCommands.Add(&kCommandInfo_SetIconPath);
	g_scriptCommands.Add(&kCommandInfo_SetMaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_SetFemaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_SetMaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_SetFemaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_SetMaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_SetFemaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedCurrentHealth);
	g_scriptCommands.Add(&kCommandInfo_SetEquippedCurrentHealth);
	g_scriptCommands.Add(&kCommandInfo_ModEquippedCurrentHealth);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedCurrentCharge);
	g_scriptCommands.Add(&kCommandInfo_SetEquippedCurrentCharge);
	g_scriptCommands.Add(&kCommandInfo_ModEquippedCurrentCharge);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedWeaponPoison, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetEquippedWeaponPoison);
	g_scriptCommands.Add(&kCommandInfo_RemoveEquippedWeaponPoison, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_CloneForm, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsClonedForm);
	g_scriptCommands.Add(&kCommandInfo_SetNumericINISetting);
	g_scriptCommands.Add(&kCommandInfo_GetMagicItemType);
	g_scriptCommands.Add(&kCommandInfo_GetMagicItemEffectCount);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemCode);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemArea);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemDuration);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemRange);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemActorValue);
	g_scriptCommands.Add(&kCommandInfo_GetSpellType);
	g_scriptCommands.Add(&kCommandInfo_GetSpellMagickaCost);
	g_scriptCommands.Add(&kCommandInfo_GetSpellMasteryLevel);
	g_scriptCommands.Add(&kCommandInfo_GetEnchantmentType);
	g_scriptCommands.Add(&kCommandInfo_GetEnchantmentCharge);
	g_scriptCommands.Add(&kCommandInfo_GetEnchantmentCost);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectCode);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectBaseCost);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectSchool);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectProjectileSpeed);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectEnchantFactor);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectBarterFactor);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectBaseCostC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectSchoolC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectProjectileSpeedC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectEnchantFactorC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectBarterFactorC);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemMagnitude);
	g_scriptCommands.Add(&kCommandInfo_ModNthEffectItemMagnitude);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemArea);
	g_scriptCommands.Add(&kCommandInfo_ModNthEffectItemArea);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemDuration);
	g_scriptCommands.Add(&kCommandInfo_ModNthEffectItemDuration);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemRange);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemActorValue);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemActorValueC);
	g_scriptCommands.Add(&kCommandInfo_RemoveNthEffectItem);
	g_scriptCommands.Add(&kCommandInfo_SetSpellType);
	g_scriptCommands.Add(&kCommandInfo_SetSpellMagickaCost);
	g_scriptCommands.Add(&kCommandInfo_ModSpellMagickaCost);
	g_scriptCommands.Add(&kCommandInfo_SetSpellMasteryLevel);
	g_scriptCommands.Add(&kCommandInfo_SetEnchantmentCharge);
	g_scriptCommands.Add(&kCommandInfo_ModEnchantmentCharge);
	g_scriptCommands.Add(&kCommandInfo_SetEnchantmentCost);
	g_scriptCommands.Add(&kCommandInfo_ModEnchantmentCost);
	g_scriptCommands.Add(&kCommandInfo_SetEnchantmentType);
	g_scriptCommands.Add(&kCommandInfo_SetNumericINISetting);
	g_scriptCommands.Add(&kCommandInfo_GetNumericINISetting);
	g_scriptCommands.Add(&kCommandInfo_GetActorLightAmount);
	g_scriptCommands.Add(&kCommandInfo_GetGameLoaded);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectHostile);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectHostileC);
	g_scriptCommands.Add(&kCommandInfo_SaveIP);
	g_scriptCommands.Add(&kCommandInfo_RestoreIP);

	// v0010
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_CopyNthEffectItem);
	g_scriptCommands.Add(&kCommandInfo_CopyAllEffectItems);
	g_scriptCommands.Add(&kCommandInfo_AddEffectItem);
	g_scriptCommands.Add(&kCommandInfo_AddEffectItemC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicItemAutoCalc);
	g_scriptCommands.Add(&kCommandInfo_SetMagicItemAutoCalc);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectForSpellmaking);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectForSpellmakingC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectForEnchanting);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectForEnchantingC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectDetrimental);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectDetrimentalC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectCanRecover);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectCanRecoverC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectMagnitudePercent);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectMagnitudePercentC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectFXPersists);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectFXPersistsC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectOnSelfAllowed);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectOnSelfAllowedC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectOnTouchAllowed);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectOnTouchAllowedC);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectOnTargetAllowed);
	g_scriptCommands.Add(&kCommandInfo_IsMagicEffectOnTargetAllowedC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoDuration);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoDurationC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoMagnitude);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoArea);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoAreaC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoIngredient);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoIngredientC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoHitEffect);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectHasNoHitEffectC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesWeapon);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesWeaponC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesArmor);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesArmorC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesCreature);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesCreatureC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesSkill);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesSkillC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesAttribute);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesAttributeC);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesOtherActorValue);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectUsesOtherActorValueC);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentHealth);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentCharge);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentSoulLevel);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectOtherActorValue);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectOtherActorValueC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectUsedObject, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectUsedObjectC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsCreature);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureType);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureCombatSkill);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureMagicSkill);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureStealthSkill);
	g_scriptCommands.Add(&kCommandInfo_GetBookCantBeTaken);
	g_scriptCommands.Add(&kCommandInfo_GetBookIsScroll);
	g_scriptCommands.Add(&kCommandInfo_GetBookSkillTaught);
	g_scriptCommands.Add(&kCommandInfo_SetBookCantBeTaken);
	g_scriptCommands.Add(&kCommandInfo_SetBookIsScroll);
	g_scriptCommands.Add(&kCommandInfo_SetBookSkillTaught);
	g_scriptCommands.Add(&kCommandInfo_GetApparatusType);
	g_scriptCommands.Add(&kCommandInfo_SetApparatusType);
	g_scriptCommands.Add(&kCommandInfo_GetQuality);
	g_scriptCommands.Add(&kCommandInfo_SetQuality);
	g_scriptCommands.Add(&kCommandInfo_ModQuality);
	g_scriptCommands.Add(&kCommandInfo_SetSoulLevel);
	g_scriptCommands.Add(&kCommandInfo_SetSoulGemCapacity);
	g_scriptCommands.Add(&kCommandInfo_ModModelPath);
	g_scriptCommands.Add(&kCommandInfo_ModIconPath);
	g_scriptCommands.Add(&kCommandInfo_ModMaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_ModFemaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_ModMaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_ModFemaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_ModMaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_ModFemaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_CompareModelPath);
	g_scriptCommands.Add(&kCommandInfo_CompareIconPath);
	g_scriptCommands.Add(&kCommandInfo_CompareMaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_CompareFemaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_CompareMaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_CompareFemaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_CompareMaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_CompareFemaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_GetPlayerSpellCount);
	g_scriptCommands.Add(&kCommandInfo_GetNthPlayerSpell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_CopyModelPath);
	g_scriptCommands.Add(&kCommandInfo_CopyIconPath);
	g_scriptCommands.Add(&kCommandInfo_CopyMaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_CopyFemaleBipedPath);
	g_scriptCommands.Add(&kCommandInfo_CopyMaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_CopyFemaleGroundPath);
	g_scriptCommands.Add(&kCommandInfo_CopyMaleIconPath);
	g_scriptCommands.Add(&kCommandInfo_CopyFemaleIconPath);
	ImportConsoleCommand("TCL");
	ImportConsoleCommand("Save");
	ImportConsoleCommand("ToggleAI");
	ImportConsoleCommand("ToggleCombatAI");
	ImportConsoleCommand("ToggleMenus");
	g_scriptCommands.Add(&kCommandInfo_IsScripted);
	g_scriptCommands.Add(&kCommandInfo_CompareName);
	g_scriptCommands.Add(&kCommandInfo_MenuTapKey);
	g_scriptCommands.Add(&kCommandInfo_MenuHoldKey);
	g_scriptCommands.Add(&kCommandInfo_MenuReleaseKey);
	g_scriptCommands.Add(&kCommandInfo_GetEquipmentSlotMask);
	g_scriptCommands.Add(&kCommandInfo_LeftShift);
	g_scriptCommands.Add(&kCommandInfo_RightShift);
	g_scriptCommands.Add(&kCommandInfo_LogicalAnd);
	g_scriptCommands.Add(&kCommandInfo_LogicalOr);
	g_scriptCommands.Add(&kCommandInfo_LogicalXor);
	g_scriptCommands.Add(&kCommandInfo_LogicalNot);
	g_scriptCommands.Add(&kCommandInfo_GetSpellSchool);
	g_scriptCommands.Add(&kCommandInfo_IsNthEffectItemScripted);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemScript, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemScriptVisualEffect);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemScriptSchool);
	g_scriptCommands.Add(&kCommandInfo_IsNthEffectItemScriptHostile);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScriptVisualEffect);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScriptVisualEffectC);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScriptSchool);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScriptHostile);
	ImportConsoleCommand("CAL");
	g_scriptCommands.Add(&kCommandInfo_Fmod);
	g_scriptCommands.Add(&kCommandInfo_CopyName);
	g_scriptCommands.Add(&kCommandInfo_GetFPS);
	g_scriptCommands.Add(&kCommandInfo_IsThirdPerson);

	// v0011
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_IsGlobalCollisionDisabled);
	g_scriptCommands.Add(&kCommandInfo_SetDisableGlobalCollision);
	g_scriptCommands.Add(&kCommandInfo_AddFullEffectItem);
	g_scriptCommands.Add(&kCommandInfo_AddFullEffectItemC);
	g_scriptCommands.Add(&kCommandInfo_GetPlayerSkillUse);
	g_scriptCommands.Add(&kCommandInfo_RunBatchScript);
	g_scriptCommands.Add(&kCommandInfo_GetSkillUseIncrement);
	g_scriptCommands.Add(&kCommandInfo_SetSkillUseIncrement);
	g_scriptCommands.Add(&kCommandInfo_IncrementPlayerSkillUse);
	g_scriptCommands.Add(&kCommandInfo_IsClassAttribute);
	ImportConsoleCommand("SaveINI");
	ImportConsoleCommand("QuitGame");
	g_scriptCommands.Add(&kCommandInfo_ModName);
	ImportConsoleCommand("TGM");
	g_scriptCommands.Add(&kCommandInfo_AppendToName);

	// v0012
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_GetRaceAttribute);
	g_scriptCommands.Add(&kCommandInfo_GetRaceAttributeC);
	g_scriptCommands.Add(&kCommandInfo_GetRaceSkillBonus);
	g_scriptCommands.Add(&kCommandInfo_IsRaceBonusSkillC);
	g_scriptCommands.Add(&kCommandInfo_IsRaceBonusSkill);
	g_scriptCommands.Add(&kCommandInfo_GetRaceSkillBonusC);
	g_scriptCommands.Add(&kCommandInfo_GetNthRaceBonusSkill);
	g_scriptCommands.Add(&kCommandInfo_GetMerchantContainer, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentWeatherID, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentClimateID, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetClimateSunriseBegin);
	g_scriptCommands.Add(&kCommandInfo_GetClimateSunriseEnd);
	g_scriptCommands.Add(&kCommandInfo_GetClimateSunsetBegin);
	g_scriptCommands.Add(&kCommandInfo_GetClimateSunsetEnd);
	ImportConsoleCommand("TFC");
	g_scriptCommands.Add(&kCommandInfo_GetSpellExplodesWithNoTarget);
	g_scriptCommands.Add(&kCommandInfo_SetSpellExplodesWithNoTarget);
	g_scriptCommands.Add(&kCommandInfo_RemoveAllEffectItems);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScript, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScriptName);
	g_scriptCommands.Add(&kCommandInfo_SetMerchantContainer);
	g_scriptCommands.Add(&kCommandInfo_MagicItemHasEffectCount);
	g_scriptCommands.Add(&kCommandInfo_MagicItemHasEffectCountCode);
	g_scriptCommands.Add(&kCommandInfo_GetScript, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_RemoveScript, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetScript, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsPlayable);
	g_scriptCommands.Add(&kCommandInfo_SetPlayable);

	// v0013
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_SetPCAMurderer);
	g_scriptCommands.Add(&kCommandInfo_GetClimateMoonPhaseLength);
	g_scriptCommands.Add(&kCommandInfo_GetClimateHasMasser);
	g_scriptCommands.Add(&kCommandInfo_GetClimateHasSecunda);
	g_scriptCommands.Add(&kCommandInfo_SetClimateSunriseBegin);
	g_scriptCommands.Add(&kCommandInfo_SetClimateSunriseEnd);
	g_scriptCommands.Add(&kCommandInfo_SetClimateSunsetBegin);
	g_scriptCommands.Add(&kCommandInfo_SetClimateSunsetEnd);
	g_scriptCommands.Add(&kCommandInfo_SetClimateMoonPhaseLength);
	g_scriptCommands.Add(&kCommandInfo_SetClimateHasMasser);
	g_scriptCommands.Add(&kCommandInfo_SetClimateHasSecunda);
	g_scriptCommands.Add(&kCommandInfo_GetClimateVolatility);
	g_scriptCommands.Add(&kCommandInfo_SetClimateVolatility);
	g_scriptCommands.Add(&kCommandInfo_IsKeyPressed3);
	g_scriptCommands.Add(&kCommandInfo_IsControlPressed);
	g_scriptCommands.Add(&kCommandInfo_DisableControl);
	g_scriptCommands.Add(&kCommandInfo_EnableControl);
	g_scriptCommands.Add(&kCommandInfo_OnKeyDown);
	g_scriptCommands.Add(&kCommandInfo_OnControlDown);
	g_scriptCommands.Add(&kCommandInfo_GetActiveEffectCount);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectCode);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectDuration);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectTimeElapsed);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectMagicItem, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectCaster, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectMagicItemIndex);
	g_scriptCommands.Add(&kCommandInfo_GetTotalActiveEffectMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAENonAbilityMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEAbilityMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAESpellMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEDiseaseMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAELesserPowerMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEPowerMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEAllSpellsMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEEnchantmentMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEAlchemyMagnitude);
	ImportConsoleCommand("LoadGame");
	g_scriptCommands.Add(&kCommandInfo_IsPluginInstalled);
	g_scriptCommands.Add(&kCommandInfo_GetPluginVersion);
	g_scriptCommands.Add(&kCommandInfo_GetNthActiveEffectData, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetTotalActiveEffectMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAENonAbilityMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEAbilityMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAESpellMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEDiseaseMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAELesserPowerMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEPowerMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEAllSpellsMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEEnchantmentMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_GetTotalAEAlchemyMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_ParentCellHasWater);
	g_scriptCommands.Add(&kCommandInfo_GetParentCellWaterHeight);
	g_scriptCommands.Add(&kCommandInfo_IsUnderWater);
	g_scriptCommands.Add(&kCommandInfo_GetDebugSelection, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_AddToLeveledList);
	g_scriptCommands.Add(&kCommandInfo_RemoveFromLeveledList);
	g_scriptCommands.Add(&kCommandInfo_GetTravelHorse, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetTravelHorse);
	g_scriptCommands.Add(&kCommandInfo_CompareNames);
	g_scriptCommands.Add(&kCommandInfo_GetGameRestarted);
	g_scriptCommands.Add(&kCommandInfo_TapControl);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherWindSpeed);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherCloudSpeedLower);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherCloudSpeedUpper);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherTransDelta);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherSunGlare);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherSunDamage);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherFogDayNear);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherFogDayFar);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherFogNightNear);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherFogNightFar);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherHDRValue);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherHDRValue);
	g_scriptCommands.Add(&kPaddingCommand);	// used to be SetCurrentClimate
	g_scriptCommands.Add(&kCommandInfo_GetWeatherColor);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherColor);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherLightningFrequency);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherWindSpeed);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherCloudSpeedLower);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherCloudSpeedUpper);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherTransDelta);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherSunGlare);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherSunDamage);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherFogDayNear);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherFogDayFar);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherFogNightNear);
	g_scriptCommands.Add(&kCommandInfo_SetWeatherFogNightFar);
	g_scriptCommands.Add(&kCommandInfo_RefreshCurrentClimate);
	g_scriptCommands.Add(&kCommandInfo_CalcLeveledItem, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetOpenKey, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetOpenKey);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherLightningFrequency);
	g_scriptCommands.Add(&kCommandInfo_SetNthActiveEffectMagnitude);
	g_scriptCommands.Add(&kCommandInfo_ModNthActiveEffectMagnitude);
	g_scriptCommands.Add(&kCommandInfo_GetScriptActiveEffectIndex);
	g_scriptCommands.Add(&kCommandInfo_GetParentCellOwner, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetOwner, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetOwningFactionRequiredRank);
	g_scriptCommands.Add(&kCommandInfo_GetParentCellOwningFactionRequiredRank);
	g_scriptCommands.Add(&kCommandInfo_SetHair);
	g_scriptCommands.Add(&kCommandInfo_CopyHair);
	g_scriptCommands.Add(&kCommandInfo_SetEyes);
	g_scriptCommands.Add(&kCommandInfo_CopyEyes);
	g_scriptCommands.Add(&kCommandInfo_GetContainerRespawns);
	g_scriptCommands.Add(&kCommandInfo_SetContainerRespawns);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureReach);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureBaseScale);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureSoulLevel);
	g_scriptCommands.Add(&kCommandInfo_IsLoadDoor);
	g_scriptCommands.Add(&kCommandInfo_GetLinkedDoor, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetTeleportCell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetFirstRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNextRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNumRefs);
	g_scriptCommands.Add(&kCommandInfo_RefreshControlMap);

	// v0014
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_IsPersistent);
	g_scriptCommands.Add(&kCommandInfo_IsOffLimits);
	g_scriptCommands.Add(&kCommandInfo_MessageEX);
	g_scriptCommands.Add(&kCommandInfo_MessageBoxEX);
	ImportConsoleCommand("PlayerSpellBook");
	ImportConsoleCommand("ToggleMapMarkers");

	g_scriptCommands.Add(&kCommandInfo_GetNumChildRefs);
	g_scriptCommands.Add(&kCommandInfo_GetNthChildRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetScaleEX);
	g_scriptCommands.Add(&kCommandInfo_GetNumFollowers);
	g_scriptCommands.Add(&kCommandInfo_GetNthFollower, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetCellMusicType);

	g_scriptCommands.Add(&kCommandInfo_IsActorRespawning);
	g_scriptCommands.Add(&kCommandInfo_IsPCLevelOffset);
	g_scriptCommands.Add(&kCommandInfo_HasLowLevelProcessing);
	g_scriptCommands.Add(&kCommandInfo_IsSummonable);
	g_scriptCommands.Add(&kCommandInfo_HasNoPersuasion);
	g_scriptCommands.Add(&kCommandInfo_CanCorpseCheck);
	g_scriptCommands.Add(&kCommandInfo_GetActorMinLevel);
	g_scriptCommands.Add(&kCommandInfo_GetActorMaxLevel);
	g_scriptCommands.Add(&kCommandInfo_SetFemale);
	g_scriptCommands.Add(&kCommandInfo_SetActorRespawns);
	g_scriptCommands.Add(&kCommandInfo_SetPCLevelOffset);
	g_scriptCommands.Add(&kCommandInfo_SetLowLevelProcessing);
	g_scriptCommands.Add(&kCommandInfo_SetSummonable);
	g_scriptCommands.Add(&kCommandInfo_SetNoPersuasion);
	g_scriptCommands.Add(&kCommandInfo_SetCanCorpseCheck);

	g_scriptCommands.Add(&kCommandInfo_GetCrosshairRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNumFactions);
	g_scriptCommands.Add(&kCommandInfo_GetNthFaction, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_IsFactionEvil);
	g_scriptCommands.Add(&kCommandInfo_IsFactionHidden);
	g_scriptCommands.Add(&kCommandInfo_FactionHasSpecialCombat);
	g_scriptCommands.Add(&kCommandInfo_SetFactionEvil);
	g_scriptCommands.Add(&kCommandInfo_SetFactionHidden);
	g_scriptCommands.Add(&kCommandInfo_SetFactionSpecialCombat);

	g_scriptCommands.Add(&kCommandInfo_IsLightCarriable);
	g_scriptCommands.Add(&kCommandInfo_GetLightRadius);
	g_scriptCommands.Add(&kCommandInfo_SetLightRadius);

	g_scriptCommands.Add(&kCommandInfo_GetRaceSpellCount);
	g_scriptCommands.Add(&kCommandInfo_GetNthRaceSpell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_MagicItemHasEffectItemScript);
	g_scriptCommands.Add(&kCommandInfo_SetCurrentSoulLevel);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureWalks);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureSwims);
	g_scriptCommands.Add(&kCommandInfo_GetCreatureFlies);
	g_scriptCommands.Add(&kCommandInfo_IsCreatureBiped);
	g_scriptCommands.Add(&kCommandInfo_CreatureHasNoMovement);
	g_scriptCommands.Add(&kCommandInfo_CreatureHasNoHead);
	g_scriptCommands.Add(&kCommandInfo_CreatureHasNoLeftArm);
	g_scriptCommands.Add(&kCommandInfo_CreatureHasNoRightArm);
	g_scriptCommands.Add(&kCommandInfo_CreatureNoCombatInWater);
	g_scriptCommands.Add(&kCommandInfo_CreatureUsesWeaponAndShield);

	g_scriptCommands.Add(&kCommandInfo_GetPlayersLastRiddenHorse, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetPlayersLastActivatedLoadDoor, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetHorse, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetRider, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsFemale);
	g_scriptCommands.Add(&kCommandInfo_IsActivatable);
	g_scriptCommands.Add(&kCommandInfo_IsHarvested);
	g_scriptCommands.Add(&kCommandInfo_SetHarvested);
	g_scriptCommands.Add(&kCommandInfo_GetActorValueC);
	g_scriptCommands.Add(&kCommandInfo_SetActorValueC);
	g_scriptCommands.Add(&kCommandInfo_ModActorValueC);
	g_scriptCommands.Add(&kCommandInfo_ModNthEffectItemScriptName);

	g_scriptCommands.Add(&kCommandInfo_GetBaseActorValueC);

	g_scriptCommands.Add(&kCommandInfo_GetCreatureSoundBase, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNumRanks);
	g_scriptCommands.Add(&kCommandInfo_HasModel);
	g_scriptCommands.Add(&kCommandInfo_IsModLoaded);

	g_scriptCommands.Add(&kCommandInfo_GetRace, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_HasName);
	g_scriptCommands.Add(&kCommandInfo_HasBeenPickedUp);

	// v0015
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_GetRefCount);
	g_scriptCommands.Add(&kCommandInfo_SetRefCount);
	g_scriptCommands.Add(&kCommandInfo_GetProjectileType);
	g_scriptCommands.Add(&kCommandInfo_GetMagicProjectileSpell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetArrowProjectileEnchantment, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetArrowProjectileBowEnchantment, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetArrowProjectilePoison, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetMagicProjectileSpell);
	g_scriptCommands.Add(&kCommandInfo_GetProjectileSource, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_GetModIndex);
	g_scriptCommands.Add(&kCommandInfo_GetSourceModIndex);
	g_scriptCommands.Add(&kCommandInfo_GetCalcAllLevels);
	g_scriptCommands.Add(&kCommandInfo_GetCalcEachInCount);
	g_scriptCommands.Add(&kCommandInfo_GetChanceNone);
	g_scriptCommands.Add(&kCommandInfo_CalcLeveledItemNR, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNumLevItems);
	g_scriptCommands.Add(&kCommandInfo_GetNthLevItem, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_AddItemNS);
	g_scriptCommands.Add(&kCommandInfo_RemoveItemNS);
	g_scriptCommands.Add(&kCommandInfo_EquipItemNS);
	g_scriptCommands.Add(&kCommandInfo_UnequipItemNS);
	g_scriptCommands.Add(&kCommandInfo_AddSpellNS);
	g_scriptCommands.Add(&kCommandInfo_RemoveSpellNS);
	g_scriptCommands.Add(&kCommandInfo_GetHair, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetEyes, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetHairColor);
	g_scriptCommands.Add(&kCommandInfo_GetOpenSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetCloseSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetLoopSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetOpenSound);
	g_scriptCommands.Add(&kCommandInfo_SetCloseSound);
	g_scriptCommands.Add(&kCommandInfo_SetLoopSound);
	g_scriptCommands.Add(&kCommandInfo_GetNumLoadedMods);

	g_scriptCommands.Add(&kCommandInfo_GetCreatureSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsPlayable2);
	g_scriptCommands.Add(&kCommandInfo_IsFormValid);
	g_scriptCommands.Add(&kCommandInfo_IsReference);
	g_scriptCommands.Add(&kCommandInfo_ToggleCreatureModel);

	g_scriptCommands.Add(&kCommandInfo_SetMessageSound);
	g_scriptCommands.Add(&kCommandInfo_SetMessageIcon);

	g_scriptCommands.Add(&kCommandInfo_GetVariable);
	g_scriptCommands.Add(&kCommandInfo_GetRefVariable);
	g_scriptCommands.Add(&kCommandInfo_HasVariable);

	g_scriptCommands.Add(&kCommandInfo_GetFullGoldValue);

	g_scriptCommands.Add(&kCommandInfo_GetNumDetectedActors);
	g_scriptCommands.Add(&kCommandInfo_GetNthDetectedActor, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_SetDetectionState);

	g_scriptCommands.Add(&kCommandInfo_GetHotKeyItem);
	g_scriptCommands.Add(&kCommandInfo_ClearHotKey);
	g_scriptCommands.Add(&kCommandInfo_SetHotKeyItem);

	g_scriptCommands.Add(&kCommandInfo_OffersWeapons);
	g_scriptCommands.Add(&kCommandInfo_OffersArmor);
	g_scriptCommands.Add(&kCommandInfo_OffersClothing);
	g_scriptCommands.Add(&kCommandInfo_OffersBooks);
	g_scriptCommands.Add(&kCommandInfo_OffersLights);
	g_scriptCommands.Add(&kCommandInfo_OffersIngredients);
	g_scriptCommands.Add(&kCommandInfo_OffersApparatus);
	g_scriptCommands.Add(&kCommandInfo_OffersMiscItems);
	g_scriptCommands.Add(&kCommandInfo_OffersMagicItems);
	g_scriptCommands.Add(&kCommandInfo_OffersSpells);
	g_scriptCommands.Add(&kCommandInfo_OffersPotions);
	g_scriptCommands.Add(&kCommandInfo_OffersTraining);
	g_scriptCommands.Add(&kCommandInfo_OffersRecharging);
	g_scriptCommands.Add(&kCommandInfo_OffersRepair);
	g_scriptCommands.Add(&kCommandInfo_OffersServicesC);
	g_scriptCommands.Add(&kCommandInfo_GetTrainerSkill);
	g_scriptCommands.Add(&kCommandInfo_GetTrainerLevel);

	g_scriptCommands.Add(&kCommandInfo_SetOffersWeapons);
	g_scriptCommands.Add(&kCommandInfo_SetOffersArmor);
	g_scriptCommands.Add(&kCommandInfo_SetOffersClothing);
	g_scriptCommands.Add(&kCommandInfo_SetOffersBooks);
	g_scriptCommands.Add(&kCommandInfo_SetOffersIngredients);
	g_scriptCommands.Add(&kCommandInfo_SetOffersSpells);
	g_scriptCommands.Add(&kCommandInfo_SetOffersLights);
	g_scriptCommands.Add(&kCommandInfo_SetOffersMiscItems);
	g_scriptCommands.Add(&kCommandInfo_SetOffersMagicItems);
	g_scriptCommands.Add(&kCommandInfo_SetOffersApparatus);
	g_scriptCommands.Add(&kCommandInfo_SetOffersPotions);
	g_scriptCommands.Add(&kCommandInfo_SetOffersTraining);
	g_scriptCommands.Add(&kCommandInfo_SetOffersRecharging);
	g_scriptCommands.Add(&kCommandInfo_SetOffersRepair);
	g_scriptCommands.Add(&kCommandInfo_SetOffersServicesC);
	g_scriptCommands.Add(&kCommandInfo_GetServicesMask);
	g_scriptCommands.Add(&kCommandInfo_SetTrainerSkill);
	g_scriptCommands.Add(&kCommandInfo_SetTrainerLevel);
	g_scriptCommands.Add(&kCommandInfo_GetNumPackages);
	g_scriptCommands.Add(&kCommandInfo_GetNthPackage, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_IsBlocking);
	g_scriptCommands.Add(&kCommandInfo_IsAttacking);
	g_scriptCommands.Add(&kCommandInfo_IsRecoiling);
	g_scriptCommands.Add(&kCommandInfo_IsDodging);
	g_scriptCommands.Add(&kCommandInfo_IsStaggered);

	g_scriptCommands.Add(&kCommandInfo_IsMovingForward);
	g_scriptCommands.Add(&kCommandInfo_IsMovingBackward);
	g_scriptCommands.Add(&kCommandInfo_IsMovingLeft);
	g_scriptCommands.Add(&kCommandInfo_IsMovingRight);
	g_scriptCommands.Add(&kCommandInfo_IsTurningLeft);
	g_scriptCommands.Add(&kCommandInfo_IsTurningRight);
	g_scriptCommands.Add(&kCommandInfo_GetOBSERevision);

	g_scriptCommands.Add(&kCommandInfo_IsInAir);
	g_scriptCommands.Add(&kCommandInfo_IsJumping);
	g_scriptCommands.Add(&kCommandInfo_IsOnGround);
	g_scriptCommands.Add(&kCommandInfo_IsFlying);
	g_scriptCommands.Add(&kCommandInfo_GetFallTimer);
	g_scriptCommands.Add(&kCommandInfo_GetGodMode);

	g_scriptCommands.Add(&kCommandInfo_CompareScripts);
	g_scriptCommands.Add(&kCommandInfo_IsPowerAttacking);
	g_scriptCommands.Add(&kCommandInfo_IsCasting);
	g_scriptCommands.Add(&kCommandInfo_IsAnimGroupPlaying);
	g_scriptCommands.Add(&kCommandInfo_AnimPathIncludes);
	g_scriptCommands.Add(&kCommandInfo_ClearLeveledList);
	g_scriptCommands.Add(&kCommandInfo_GetNthLevItemCount);
	g_scriptCommands.Add(&kCommandInfo_GetNthLevItemLevel);

	g_scriptCommands.Add(&kCommandInfo_IsSpellHostile);
	g_scriptCommands.Add(&kCommandInfo_SetSpellHostile);
	g_scriptCommands.Add(&kCommandInfo_RemoveLevItemByLevel);

	g_scriptCommands.Add(&kCommandInfo_IsModelPathValid);
	g_scriptCommands.Add(&kCommandInfo_IsIconPathValid);
	g_scriptCommands.Add(&kCommandInfo_IsBipedModelPathValid);
	g_scriptCommands.Add(&kCommandInfo_IsBipedIconPathValid);
	g_scriptCommands.Add(&kCommandInfo_FileExists);

	g_scriptCommands.Add(&kCommandInfo_GetPCMajorSkillUps);
	g_scriptCommands.Add(&kCommandInfo_GetPCAttributeBonus);
	g_scriptCommands.Add(&kCommandInfo_GetTotalPCAttributeBonus);

	g_scriptCommands.Add(&kCommandInfo_SetNameEx);
	g_scriptCommands.Add(&kCommandInfo_SetPlayerProjectile);
	g_scriptCommands.Add(&kCommandInfo_SetHasBeenPickedUp);
	g_scriptCommands.Add(&kCommandInfo_GetProcessLevel);

	g_scriptCommands.Add(&kCommandInfo_GetActiveMenuMode);
	g_scriptCommands.Add(&kCommandInfo_GetEnchMenuEnchItem, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetEnchMenuSoulgem, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetActiveMenuFilter);
	g_scriptCommands.Add(&kCommandInfo_GetActiveMenuSelection, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetActiveMenuObject, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetActiveMenuRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IsBarterMenuActive);

	g_scriptCommands.Add(&kCommandInfo_GetSoundPlaying);

	g_scriptCommands.Add(&kCommandInfo_GetAlchMenuIngredient, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetAlchMenuIngredientCount);
	g_scriptCommands.Add(&kCommandInfo_GetAlchMenuApparatus, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetContainerMenuView);
	g_scriptCommands.Add(&kCommandInfo_GetAltControl2);
	g_scriptCommands.Add(&kCommandInfo_GetLevItemByLevel, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_CloseAllMenus);
	g_scriptCommands.Add(&kCommandInfo_SetControl);
	g_scriptCommands.Add(&kCommandInfo_SetAltControl);

	//0016
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_sv_Construct, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_sv_Compare);
	g_scriptCommands.Add(&kCommandInfo_sv_Erase);
	g_scriptCommands.Add(&kCommandInfo_sv_Length);
	g_scriptCommands.Add(&kCommandInfo_sv_SubString, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_sv_Count);
	g_scriptCommands.Add(&kCommandInfo_sv_Find);
	g_scriptCommands.Add(&kCommandInfo_sv_Replace);
	g_scriptCommands.Add(&kCommandInfo_sv_ToNumeric);
	g_scriptCommands.Add(&kCommandInfo_sv_Insert);
	g_scriptCommands.Add(&kCommandInfo_IsDigit);
	g_scriptCommands.Add(&kCommandInfo_IsLetter);
	g_scriptCommands.Add(&kCommandInfo_IsPrintable);
	g_scriptCommands.Add(&kCommandInfo_IsPunctuation);
	g_scriptCommands.Add(&kCommandInfo_IsUpperCase);
	g_scriptCommands.Add(&kCommandInfo_sv_GetChar);

	g_scriptCommands.Add(&kCommandInfo_SetDebugMode);
	g_scriptCommands.Add(&kCommandInfo_DebugPrint);
	g_scriptCommands.Add(&kCommandInfo_RunScriptLine);
	g_scriptCommands.Add(&kCommandInfo_ResetAllVariables);
	g_scriptCommands.Add(&kCommandInfo_CharToAscii);

	g_scriptCommands.Add(&kCommandInfo_GetFirstRefInCell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNumRefsInCell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_sv_Destruct);

	g_scriptCommands.Add(&kCommandInfo_OpenTextInput);
	g_scriptCommands.Add(&kCommandInfo_UpdateTextInput);
	g_scriptCommands.Add(&kCommandInfo_GetInputText, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_CloseTextInput);
	g_scriptCommands.Add(&kCommandInfo_IsTextInputInUse);
	g_scriptCommands.Add(&kCommandInfo_InsertInInputText);
	g_scriptCommands.Add(&kCommandInfo_GetTextInputControlPressed);

	g_scriptCommands.Add(&kCommandInfo_SetRaceAlias);
	g_scriptCommands.Add(&kCommandInfo_SetRaceVoice);
	g_scriptCommands.Add(&kCommandInfo_SetRacePlayable);

	g_scriptCommands.Add(&kCommandInfo_GetSpellEffectiveness);
	g_scriptCommands.Add(&kCommandInfo_GetIngredient, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetIngredientChance);
	g_scriptCommands.Add(&kCommandInfo_SetIngredient);
	g_scriptCommands.Add(&kCommandInfo_SetIngredientChance);

	g_scriptCommands.Add(&kCommandInfo_SetProjectileSource);
	g_scriptCommands.Add(&kCommandInfo_SetArrowProjectileEnchantment);
	g_scriptCommands.Add(&kCommandInfo_SetArrowProjectileBowEnchantment);
	g_scriptCommands.Add(&kCommandInfo_SetArrowProjectilePoison);
	g_scriptCommands.Add(&kCommandInfo_GetProjectileSpeed);
	g_scriptCommands.Add(&kCommandInfo_GetProjectileDistanceTraveled);
	g_scriptCommands.Add(&kCommandInfo_GetProjectileLifetime);
	g_scriptCommands.Add(&kCommandInfo_SetProjectileSpeed);

	g_scriptCommands.Add(&kCommandInfo_DeleteReference);
	g_scriptCommands.Add(&kCommandInfo_ModPCSpellEffectiveness);
	g_scriptCommands.Add(&kCommandInfo_GetPCSpellEffectivenessModifier);

	g_scriptCommands.Add(&kCommandInfo_GetCurrentFrameIndex);		// for haama (undocumented)

	g_scriptCommands.Add(&kCommandInfo_GetNthModName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetStringGameSetting, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetModelPath, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetIconPath, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetBipedModelPath, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetBipedIconPath, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetNthFactionRankName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemScriptName, kRetnType_String);

	g_scriptCommands.Add(&kCommandInfo_SetModelPathEX);
	g_scriptCommands.Add(&kCommandInfo_SetIconPathEX);
	g_scriptCommands.Add(&kCommandInfo_SetNthFactionRankNameEX);
	g_scriptCommands.Add(&kCommandInfo_SetStringGameSettingEX);
	g_scriptCommands.Add(&kCommandInfo_SetBipedModelPathEX);
	g_scriptCommands.Add(&kCommandInfo_SetBipedIconPathEX);
	g_scriptCommands.Add(&kCommandInfo_SetNthEffectItemScriptNameEX);

	g_scriptCommands.Add(&kCommandInfo_ToggleFirstPerson);
	g_scriptCommands.Add(&kCommandInfo_GetFormFromMod, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_GetHidesRings);
	g_scriptCommands.Add(&kCommandInfo_SetHidesRings);
	g_scriptCommands.Add(&kCommandInfo_GetHidesAmulet);
	g_scriptCommands.Add(&kCommandInfo_SetHidesAmulet);

	g_scriptCommands.Add(&kCommandInfo_IsMapMarkerVisible);
	g_scriptCommands.Add(&kCommandInfo_SetMapMarkerVisible);
	g_scriptCommands.Add(&kCommandInfo_CanTravelToMapMarker);
	g_scriptCommands.Add(&kCommandInfo_SetCanTravelToMapMarker);
	g_scriptCommands.Add(&kCommandInfo_GetMapMarkerType);
	g_scriptCommands.Add(&kCommandInfo_SetMapMarkerType);

	g_scriptCommands.Add(&kCommandInfo_GetCursorPos);
	g_scriptCommands.Add(&kCommandInfo_IsFlora);
	g_scriptCommands.Add(&kCommandInfo_SetPCMajorSkillUps);
	g_scriptCommands.Add(&kCommandInfo_SetPCAttributeBonus);

	ImportConsoleCommand("Show1stPerson");

	g_scriptCommands.Add(&kCommandInfo_GetMenuFloatValue);
	g_scriptCommands.Add(&kCommandInfo_GetMenuStringValue, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_SetMenuFloatValue);
	g_scriptCommands.Add(&kCommandInfo_SetMenuStringValue);

	g_scriptCommands.Add(&kCommandInfo_ToUpper);
	g_scriptCommands.Add(&kCommandInfo_ToLower);
	g_scriptCommands.Add(&kCommandInfo_SetButtonPressed);
	g_scriptCommands.Add(&kCommandInfo_SetBaseForm);

	g_scriptCommands.Add(&kCommandInfo_GetBipedSlotMask);
	g_scriptCommands.Add(&kCommandInfo_SetBipedSlotMask);

	g_scriptCommands.Add(&kCommandInfo_GetLastEnchantedItem, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetLastCreatedSpell, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetNumExplicitRefs);
	g_scriptCommands.Add(&kCommandInfo_GetNthExplicitRef, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_GetActiveUIComponentName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetActiveUIComponentFullName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetActiveUIComponentID);

	g_scriptCommands.Add(&kCommandInfo_ClickMenuButton);
	g_scriptCommands.Add(&kCommandInfo_GetProjectile, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_RemoveNthLevItem);
	g_scriptCommands.Add(&kCommandInfo_GetLevItemIndexByLevel);
	g_scriptCommands.Add(&kCommandInfo_GetLevItemIndexByForm);

	g_scriptCommands.Add(&kCommandInfo_GetLastCreatedPotion, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetLastUniqueCreatedPotion, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_IsConsoleOpen);
	g_scriptCommands.Add(&kCommandInfo_GetBookText, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetMenuHasTrait);
	g_scriptCommands.Add(&kCommandInfo_SetChanceNone);
	g_scriptCommands.Add(&kCommandInfo_sv_Set);

	g_scriptCommands.Add(&kCommandInfo_IsControl);
	g_scriptCommands.Add(&kCommandInfo_SetIsControl);

	g_scriptCommands.Add(&kCommandInfo_LoadGameEx);
	g_scriptCommands.Add(&kCommandInfo_DeleteFromInputText);

	g_scriptCommands.Add(&kCommandInfo_GetPCTrainingSessionsUsed);
	g_scriptCommands.Add(&kCommandInfo_SetPCTrainingSessionsUsed);

	g_scriptCommands.Add(&kCommandInfo_MoveTextInputCursor);
	g_scriptCommands.Add(&kCommandInfo_GetStringIniSetting, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_SetStringIniSetting);
	g_scriptCommands.Add(&kCommandInfo_GetTextInputCursorPos);

	// v0017
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_RunScripts);
	g_scriptCommands.Add(&kCommandInfo_IsKeyDisabled);
	g_scriptCommands.Add(&kCommandInfo_IsControlDisabled);

	g_scriptCommands.Add(&kCommandInfo_Let);
	g_scriptCommands.Add(&kCommandInfo_ar_Construct, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ar_Size);
	g_scriptCommands.Add(&kCommandInfo_ar_Dump);
	g_scriptCommands.Add(&kCommandInfo_eval);
	g_scriptCommands.Add(&kCommandInfo_ar_DumpID);
	g_scriptCommands.Add(&kCommandInfo_While);
	g_scriptCommands.Add(&kCommandInfo_Loop);
	g_scriptCommands.Add(&kCommandInfo_ForEach);
	g_scriptCommands.Add(&kCommandInfo_Break);
	g_scriptCommands.Add(&kCommandInfo_Continue);
	g_scriptCommands.Add(&kCommandInfo_ToString, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_DBG_echo);		// for internal debug usage
	g_scriptCommands.Add(&kCommandInfo_Print);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentRegion, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetEquippedItems, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_Activate2);
	g_scriptCommands.Add(&kCommandInfo_ar_Erase);

	g_scriptCommands.Add(&kCommandInfo_InstallModelMapHook);

	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectNameC, kRetnType_String);

	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectHitShader, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectHitShaderC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectEnchantShader, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectEnchantShaderC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectLight, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectLightC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectCastingSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectCastingSoundC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectBoltSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectBoltSoundC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectHitSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectHitSoundC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectAreaSound, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectAreaSoundC, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectNumCounters);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectNumCountersC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectResistValue);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectResistValueC);

	g_scriptCommands.Add(&kCommandInfo_GetNthMagicEffectCounter);
	g_scriptCommands.Add(&kCommandInfo_GetNthMagicEffectCounterC);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectCounters, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectCountersC, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectIcon, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectIconC, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectModel, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetMagicEffectModelC, kRetnType_String);

	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectIsHostile);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectIsHostileC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectCanRecover);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectCanRecoverC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectIsDetrimental);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectIsDetrimentalC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectMagnitudePercent);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectMagnitudePercentC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOnSelfAllowed);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOnSelfAllowedC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOnTouchAllowed);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOnTouchAllowedC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOnTargetAllowed);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOnTargetAllowedC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoDuration);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoDurationC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoMagnitude);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoMagnitudeC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoArea);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoAreaC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectFXPersists);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectFXPersistsC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectForSpellmaking);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectForSpellmakingC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectForEnchanting);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectForEnchantingC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoIngredient);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoIngredientC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesWeapon);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesWeaponC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesArmor);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesArmorC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesCreature);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesCreatureC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesSkill);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesSkillC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesAttribute);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesAttributeC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesActorValue);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsesActorValueC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoHitEffect);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNoHitEffectC);

	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectName);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectNameC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectSchool);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectSchoolC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectBaseCost);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectBaseCostC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectResistValue);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectResistValueC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectIcon);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectIconC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectCastingSound);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectCastingSoundC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectBoltSound);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectBoltSoundC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectHitSound);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectHitSoundC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectAreaSound);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectAreaSoundC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectEnchantFactor);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectEnchantFactorC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectBarterFactor);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectBarterFactorC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectModel);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectModelC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectHitShader);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectHitShaderC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectEnchantShader);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectEnchantShaderC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectProjectileSpeed);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectProjectileSpeedC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectLight);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectLightC);

	g_scriptCommands.Add(&kCommandInfo_AddMagicEffectCounter);
	g_scriptCommands.Add(&kCommandInfo_AddMagicEffectCounterC);
	g_scriptCommands.Add(&kCommandInfo_RemoveNthMagicEffectCounter);
	g_scriptCommands.Add(&kCommandInfo_RemoveNthMagicEffectCounterC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectCounters);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectCountersC);

	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOtherActorValue);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectOtherActorValueC);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsedObject);
	g_scriptCommands.Add(&kCommandInfo_SetMagicEffectUsedObjectC);

	g_scriptCommands.Add(&kCommandInfo_DumpMagicEffectUnknowns);

	g_scriptCommands.Add(&kCommandInfo_GetCombatStyle, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeLRChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeLRTimerMin);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeLRTimerMax);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeFWTimerMin);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeFWTimerMax);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeBackTimerMin);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeBackTimerMax);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleIdleTimerMin);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleIdleTimerMax);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleBlockChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleStaggerBonusToAttack);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleKOBonusToAttack);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleH2HBonusToAttack);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStylePowerAttackChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleStaggerBonusToPowerAttack);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleKOBonusToPowerAttack);
//	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackChoiceChances);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleHoldTimerMin);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleHoldTimerMax);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAcrobaticsDodgeChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleRangeOptimalMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleRangeMaxMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleSwitchDistMelee);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleSwitchDistRanged);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleBuffStandoffDist);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleRangedStandoffDist);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleGroupStandoffDist);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleRushAttackChance);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleRushAttackDistMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeFatigueModMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeFatigueModBase);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleEncumberedSpeedModBase);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleEncumberedSpeedModMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeBackUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeBackNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeFWAttackingMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleDodgeFWNotAttackingMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleBlockSkillModMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleBlockSkillModBase);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleBlockUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleBlockNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackSkillModMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackSkillModBase);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleAttackDuringBlockMult);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStylePowerAttackFatigueModBase);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStylePowerAttackFatigueModMult);

	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeLRChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeLRTimerMin);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeLRTimerMax);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeFWTimerMin);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeFWTimerMax);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeBackTimerMin);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeBackTimerMax);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleIdleTimerMin);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleIdleTimerMax);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleBlockChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleStaggerBonusToAttack);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleKOBonusToAttack);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleH2HBonusToAttack);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStylePowerAttackChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleStaggerBonusToPowerAttack);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleKOBonusToPowerAttack);
//	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackChoiceChances);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleHoldTimerMin);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleHoldTimerMax);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAcrobaticsDodgeChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleRangeOptimalMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleRangeMaxMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleSwitchDistMelee);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleSwitchDistRanged);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleBuffStandoffDist);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleRangedStandoffDist);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleGroupStandoffDist);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleRushAttackChance);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleRushAttackDistMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeFatigueModMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeFatigueModBase);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleEncumberedSpeedModBase);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleEncumberedSpeedModMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeBackUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeBackNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeFWAttackingMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleDodgeFWNotAttackingMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleBlockSkillModMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleBlockSkillModBase);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleBlockUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleBlockNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackSkillModMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackSkillModBase);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackNotUnderAttackMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleAttackDuringBlockMult);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStylePowerAttackFatigueModBase);

	g_scriptCommands.Add(&kCommandInfo_GetNthEffectItemName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_ActorValueToString, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetParentWorldspace, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetEditorID, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_MatchPotion, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectFromCode, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_MagicEffectFromChars, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_IncrementPlayerSkillUseC);
	g_scriptCommands.Add(&kCommandInfo_GetSkillUseIncrementC);
	g_scriptCommands.Add(&kCommandInfo_SetSkillUseIncrementC);
	g_scriptCommands.Add(&kCommandInfo_testexpr);
	g_scriptCommands.Add(&kCommandInfo_SetCellWaterHeight);
	g_scriptCommands.Add(&kCommandInfo_GetFollowers, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ar_Sort, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_DispelNthActiveEffect);
	g_scriptCommands.Add(&kCommandInfo_IsRefDeleted);
	g_scriptCommands.Add(&kCommandInfo_GetItems, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetBaseItems, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetSpells, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ModPCMovementSpeed);
	g_scriptCommands.Add(&kCommandInfo_GetPCMovementSpeedModifier);
	g_scriptCommands.Add(&kCommandInfo_IsRacePlayable);
	g_scriptCommands.Add(&kCommandInfo_ar_Find, kRetnType_ArrayIndex);
	g_scriptCommands.Add(&kCommandInfo_IsGameMessageBox);
	g_scriptCommands.Add(&kCommandInfo_GetMessageBoxType);
	g_scriptCommands.Add(&kCommandInfo_ar_SortAlpha, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_TypeOf, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetPCLastDroppedItem, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetPCLastDroppedItemRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetTileTraits, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetTileChildren, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_PrintTileInfo);

	g_scriptCommands.Add(&kCommandInfo_ar_First, kRetnType_ArrayIndex);
	g_scriptCommands.Add(&kCommandInfo_ar_Last, kRetnType_ArrayIndex);
	g_scriptCommands.Add(&kCommandInfo_ar_Next, kRetnType_ArrayIndex);
	g_scriptCommands.Add(&kCommandInfo_ar_Prev, kRetnType_ArrayIndex);

	g_scriptCommands.Add(&kCommandInfo_ar_Keys, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ar_HasKey);
	g_scriptCommands.Add(&kCommandInfo_ar_BadStringIndex, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_ar_BadNumericIndex);
	g_scriptCommands.Add(&kCommandInfo_ar_Copy, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ar_Null, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ar_DeepCopy, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_ActorValueToStringC, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetKeyName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_AsciiToChar, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetFormIDString, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_NumToHex, kRetnType_String);

	g_scriptCommands.Add(&kCommandInfo_CanCastPower);
	g_scriptCommands.Add(&kCommandInfo_SetCanCastPower);
	g_scriptCommands.Add(&kCommandInfo_GetUsedPowers, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_SetPowerTimer);

	// 0018
	g_scriptCommands.RecordReleaseVersion();

	g_scriptCommands.Add(&kCommandInfo_Call, kRetnType_Ambiguous);
	g_scriptCommands.Add(&kCommandInfo_SetFunctionValue);

	g_scriptCommands.Add(&kCommandInfo_GetClassSkills, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_SetClassSkills);
	g_scriptCommands.Add(&kCommandInfo_SetClassSpecialization);
	g_scriptCommands.Add(&kCommandInfo_SetClassAttribute);
	g_scriptCommands.Add(&kCommandInfo_SetClassAttributeC);

	g_scriptCommands.Add(&kCommandInfo_GetCreatureModelPaths, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetUserTime, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_GetSkillGoverningAttribute);
	g_scriptCommands.Add(&kCommandInfo_SetSkillGoverningAttribute);
	g_scriptCommands.Add(&kCommandInfo_GetSkillGoverningAttributeC);
	g_scriptCommands.Add(&kCommandInfo_SetSkillGoverningAttributeC);
	g_scriptCommands.Add(&kCommandInfo_ActorValueToCode);

	g_scriptCommands.Add(&kCommandInfo_GetPlayerBirthsign, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetBirthsignSpells, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_GetActorAlpha);
	g_scriptCommands.Add(&kCommandInfo_RemoveBaseSpell);

	g_scriptCommands.Add(&kCommandInfo_GetMapMenuMarkerName, kRetnType_String);
	g_scriptCommands.Add(&kCommandInfo_GetMapMenuMarkerRef, kRetnType_Form);
	g_scriptCommands.Add(&kCommandInfo_GetWeatherClassification);

	g_scriptCommands.Add(&kCommandInfo_ToNumber);
	g_scriptCommands.Add(&kCommandInfo_GetActiveEffectCodes, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetActiveEffectCasters, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_GetCurrentRegions, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_SetPackageTarget);

	g_scriptCommands.Add(&kCommandInfo_GetBaseAV2);

	g_scriptCommands.Add(&kCommandInfo_GetSpecialAnims, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ToggleSpecialAnim);

	g_scriptCommands.Add(&kCommandInfo_GetLeveledSpells, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_GetDescription, kRetnType_String);

	g_scriptCommands.Add(&kCommandInfo_ar_Resize);
	g_scriptCommands.Add(&kCommandInfo_ar_Insert);
	g_scriptCommands.Add(&kCommandInfo_ar_InsertRange);

	g_scriptCommands.Add(&kCommandInfo_GetPackageOffersServices);
	g_scriptCommands.Add(&kCommandInfo_GetPackageMustReachLocation);
	g_scriptCommands.Add(&kCommandInfo_GetPackageMustComplete);
	g_scriptCommands.Add(&kCommandInfo_GetPackageLockDoorsAtStart);
	g_scriptCommands.Add(&kCommandInfo_GetPackageLockDoorsAtEnd);
	g_scriptCommands.Add(&kCommandInfo_GetPackageLockDoorsAtLocation);
	g_scriptCommands.Add(&kCommandInfo_GetPackageUnlockDoorsAtStart);
	g_scriptCommands.Add(&kCommandInfo_GetPackageUnlockDoorsAtEnd);
	g_scriptCommands.Add(&kCommandInfo_GetPackageUnlockDoorsAtLocation);
	g_scriptCommands.Add(&kCommandInfo_GetPackageContinueIfPCNear);
	g_scriptCommands.Add(&kCommandInfo_GetPackageOncePerDay);
	g_scriptCommands.Add(&kCommandInfo_GetPackageSkipFalloutBehavior);
	g_scriptCommands.Add(&kCommandInfo_GetPackageAlwaysRun);
	g_scriptCommands.Add(&kCommandInfo_GetPackageAlwaysSneak);
	g_scriptCommands.Add(&kCommandInfo_GetPackageAllowSwimming);
	g_scriptCommands.Add(&kCommandInfo_GetPackageAllowFalls);
	g_scriptCommands.Add(&kCommandInfo_GetPackageArmorUnequipped);
	g_scriptCommands.Add(&kCommandInfo_GetPackageWeaponsUnequipped);
	g_scriptCommands.Add(&kCommandInfo_GetPackageDefensiveCombat);
	g_scriptCommands.Add(&kCommandInfo_GetPackageUseHorse);
	g_scriptCommands.Add(&kCommandInfo_GetPackageNoIdleAnims);

	g_scriptCommands.Add(&kCommandInfo_SetPackageOffersServices);
	g_scriptCommands.Add(&kCommandInfo_SetPackageMustReachLocation);
	g_scriptCommands.Add(&kCommandInfo_SetPackageMustComplete);
	g_scriptCommands.Add(&kCommandInfo_SetPackageLockDoorsAtStart);
	g_scriptCommands.Add(&kCommandInfo_SetPackageLockDoorsAtEnd);
	g_scriptCommands.Add(&kCommandInfo_SetPackageLockDoorsAtLocation);
	g_scriptCommands.Add(&kCommandInfo_SetPackageUnlockDoorsAtStart);
	g_scriptCommands.Add(&kCommandInfo_SetPackageUnlockDoorsAtEnd);
	g_scriptCommands.Add(&kCommandInfo_SetPackageUnlockDoorsAtLocation);
	g_scriptCommands.Add(&kCommandInfo_SetPackageContinueIfPCNear);
	g_scriptCommands.Add(&kCommandInfo_SetPackageOncePerDay);
	g_scriptCommands.Add(&kCommandInfo_SetPackageSkipFalloutBehavior);
	g_scriptCommands.Add(&kCommandInfo_SetPackageAlwaysRun);
	g_scriptCommands.Add(&kCommandInfo_SetPackageAlwaysSneak);
	g_scriptCommands.Add(&kCommandInfo_SetPackageAllowSwimming);
	g_scriptCommands.Add(&kCommandInfo_SetPackageAllowFalls);
	g_scriptCommands.Add(&kCommandInfo_SetPackageArmorUnequipped);
	g_scriptCommands.Add(&kCommandInfo_SetPackageWeaponsUnequipped);
	g_scriptCommands.Add(&kCommandInfo_SetPackageDefensiveCombat);
	g_scriptCommands.Add(&kCommandInfo_SetPackageUseHorse);
	g_scriptCommands.Add(&kCommandInfo_SetPackageNoIdleAnims);

	g_scriptCommands.Add(&kCommandInfo_GetActorPackages, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleIgnoreAlliesInArea);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleWillYield);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleRejectsYields);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleFleeingDisabled);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStylePrefersRanged);
	g_scriptCommands.Add(&kCommandInfo_GetCombatStyleMeleeAlertOK);

	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleIgnoreAlliesInArea);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleWillYield);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleRejectsYields);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleFleeingDisabled);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStylePrefersRanged);
	g_scriptCommands.Add(&kCommandInfo_SetCombatStyleMeleeAlertOK);

	g_scriptCommands.Add(&kCommandInfo_GetGameDifficulty);
	g_scriptCommands.Add(&kCommandInfo_SetGameDifficulty);

	g_scriptCommands.Add(&kCommandInfo_SetCurrentCharge);
	g_scriptCommands.Add(&kCommandInfo_ModCurrentCharge);

	g_scriptCommands.Add(&kCommandInfo_SetDoorTeleport);
	g_scriptCommands.Add(&kCommandInfo_GetDoorTeleportX);
	g_scriptCommands.Add(&kCommandInfo_GetDoorTeleportY);
	g_scriptCommands.Add(&kCommandInfo_GetDoorTeleportZ);
	g_scriptCommands.Add(&kCommandInfo_GetDoorTeleportRot);

	g_scriptCommands.Add(&kCommandInfo_PrintD);

	g_scriptCommands.Add(&kCommandInfo_GetBaseAV2C);
	g_scriptCommands.Add(&kCommandInfo_GetTelekinesisRef, kRetnType_Form);

	g_scriptCommands.Add(&kCommandInfo_sv_Split, kRetnType_Array);
	g_scriptCommands.Add(&kCommandInfo_ar_List, kRetnType_Array);

	g_scriptCommands.Add(&kCommandInfo_GetCellDetachTime);
	g_scriptCommands.Add(&kCommandInfo_GetCellResetHours);
	g_scriptCommands.Add(&kCommandInfo_SetCellResetHours);

	g_scriptCommands.Add(&kCommandInfo_IsClassSkillC);
	g_scriptCommands.Add(&kCommandInfo_IsClassAttributeC);

	g_scriptCommands.Add(&kCommandInfo_SetPlayerBirthSign);
	g_scriptCommands.Add(&kCommandInfo_GetPlayerSkillUseC);

	g_scriptCommands.Add(&kCommandInfo_GetOblivionDirectory, kRetnType_String);

	g_scriptCommands.Add(&kCommandInfo_GetWaterShader);
	g_scriptCommands.Add(&kCommandInfo_SetOLMPGrids);
	g_scriptCommands.Add(&kCommandInfo_OLMPOR);
	ImportConsoleCommand("OutputLocalMapPictures");
	g_scriptCommands.Add(&kCommandInfo_GetGridsToLoad);
	g_scriptCommands.Add(&kCommandInfo_GetRaceReaction);
	ADD_CMD(GetBaseAVC);
	ADD_CMD(TriggerPlayerSkillUse);
	ADD_CMD(TriggerPlayerSkillUseC);
	ADD_CMD(SetCurrentHealth);
	ADD_CMD(ModPlayerSkillExp);
	ADD_CMD(ModPlayerSkillExpC);
	ADD_CMD(GetPlayerSkillAdvances);
	ADD_CMD(GetPlayerSkillAdvancesC);
	ADD_CMD(SetPlayerSkillAdvances);
	ADD_CMD(SetPlayerSkillAdvancesC);

	ADD_CMD(GetRaceScale);
	ADD_CMD_RET(GetCurrentPackage, kRetnType_Form);
	ADD_CMD_RET(GetWorldSpaceParentWorldSpace, kRetnType_Form);
	ADD_CMD(QuestExists);
	ADD_CMD(GlobalVariableExists);
	ADD_CMD_RET(GetCurrentEditorPackage, kRetnType_Form);
	ADD_CMD(StringToActorValue);

	ADD_CMD_RET(CalcLevItems, kRetnType_Array);
	ADD_CMD_RET(ar_Range, kRetnType_Array);

	ADD_CMD(GetRaceWeight);
	ADD_CMD(GetCellBehavesAsExterior);
	ADD_CMD(GetMouseButtonsSwapped);

	ADD_CMD_RET(GetAllies, kRetnType_Array);
	ADD_CMD_RET(GetTargets, kRetnType_Array);
	ADD_CMD_RET(GetSelectedSpells, kRetnType_Array);
	ADD_CMD_RET(GetCombatSpells, kRetnType_Array);

	ADD_CMD(IsOblivionGate);
	ADD_CMD(EquipItem2);
	ADD_CMD(EquipItem2NS);
	ADD_CMD_RET(GetTeleportCellName, kRetnType_String);
	ADD_CMD(EquipMe);
	ADD_CMD_RET(GetNthActiveEffectEnchantObject, kRetnType_Form);
	g_scriptCommands.Add(&kPaddingCommand);		// was UnequipMe
	ADD_CMD_RET(sv_Percentify, kRetnType_String);
	ADD_CMD_RET(GetNthActiveEffectBoundItem, kRetnType_Form);
	ADD_CMD_RET(GetNthActiveEffectSummonRef, kRetnType_Form);

	ADD_CMD(SetPlayerSkeletonPath);

	// 0019
	g_scriptCommands.RecordReleaseVersion();

	ADD_CMD_RET(GetMagicEffectCharsC, kRetnType_String);

	ADD_CMD(GetSpellPCStart);
	ADD_CMD(GetSpellImmuneToSilence);
	ADD_CMD(GetSpellAreaEffectIgnoresLOS);
	ADD_CMD(GetSpellScriptEffectAlwaysApplies);
	ADD_CMD(GetSpellDisallowAbsorbReflect);

	ADD_CMD(SetSpellPCStart);
	ADD_CMD(SetSpellImmuneToSilence);
	ADD_CMD(SetSpellAreaEffectIgnoresLOS);
	ADD_CMD(SetSpellScriptEffectAlwaysApplies);
	ADD_CMD(SetSpellDisallowAbsorbReflect);

	ADD_CMD(SetActiveQuest);
	ADD_CMD_RET(GetActiveQuest, kRetnType_Form);

	ADD_CMD_RET(GetPackageData, kRetnType_Array);
	ADD_CMD_RET(GetPackageScheduleData, kRetnType_Array);
	ADD_CMD_RET(GetPackageLocationData, kRetnType_Array);
	ADD_CMD_RET(GetPackageTargetData, kRetnType_Array);

	ADD_CMD(SetPackageData);
	ADD_CMD(SetPackageTargetData);
	ADD_CMD(SetPackageLocationData);
	ADD_CMD(SetPackageScheduleData);

	ADD_CMD_RET(ar_Map, kRetnType_Array);
	ADD_CMD_RET(GetCurrentQuests, kRetnType_Array);
	ADD_CMD_RET(GetCompletedQuests, kRetnType_Array);

	ADD_CMD_RET(GetTexturePath, kRetnType_String);
	ADD_CMD(SetTexturePath);

	ADD_CMD(GetSigilStoneUses);
	ADD_CMD(SetSigilStoneUses);
	ADD_CMD(ModSigilStoneUses);

	ADD_CMD(SetDescription);
	ADD_CMD(SetCreatureSoundBase);

	ADD_CMD(Update3D);
	ADD_CMD_RET(GetBoundingBox, kRetnType_Array);

	ADD_CMD_RET(GetBarterItem, kRetnType_Form);
	ADD_CMD(GetBarterItemQuantity);
	ADD_CMD_RET(GetLastTransactionItem, kRetnType_Form);
	ADD_CMD(GetLastTransactionQuantity);

	ADD_CMD(GetQMCurrent);
	ADD_CMD(GetQMMaximum);
	ADD_CMD_RET(GetQMItem, kRetnType_Form);

	ADD_CMD(IsEquipped);
	ADD_CMD(GetPCAttributeBonusC);
	ADD_CMD(SetPCAttributeBonusC);

	ADD_CMD(RemoveMeIR);

	ADD_CMD(ar_Append);
	ADD_CMD(CopyIR);
	ADD_CMD_RET(CreateTempRef, kRetnType_Form);

	ADD_CMD_RET(GetMagicEffectChars, kRetnType_String);

	ADD_CMD_RET(GetLastUsedSigilStone, kRetnType_Form);
	ADD_CMD_RET(GetLastSigilStoneEnchantedItem, kRetnType_Form);
	ADD_CMD_RET(GetLastSigilStoneCreatedItem, kRetnType_Form);

	ADD_CMD(SetOwner);
	ADD_CMD(SetClassSkills2);
	ADD_CMD(GetCellChanged);

	ADD_CMD_RET(GetArrayVariable, kRetnType_Array);

	ADD_CMD(SetModLocalData);
	ADD_CMD_RET(GetModLocalData, kRetnType_Ambiguous);
	ADD_CMD_RET(GetRawFormIDString, kRetnType_String);
	ADD_CMD(ModLocalDataExists);

	ADD_CMD(UnequipMe);

	// matrix mathematics
	ADD_CMD_RET(GenerateZeroMatrix, kRetnType_Array);
	ADD_CMD_RET(GenerateIdentityMatrix, kRetnType_Array);
	ADD_CMD_RET(GenerateRotationMatrix, kRetnType_Array);
		ADD_CMD(VectorMagnitude);
	ADD_CMD_RET(VectorNormalize, kRetnType_Array);
		ADD_CMD(VectorDot);
	ADD_CMD_RET(VectorCross, kRetnType_Array);
	ADD_CMD_RET(ForceRowVector, kRetnType_Array);
	ADD_CMD_RET(ForceColumnVector, kRetnType_Array);
		ADD_CMD(MatrixTrace);
		ADD_CMD(MatrixDeterminant);
	ADD_CMD_RET(MatrixRREF, kRetnType_Array);
	ADD_CMD_RET(MatrixInvert, kRetnType_Array);
	ADD_CMD_RET(MatrixTranspose, kRetnType_Array);
	ADD_CMD_RET(MatrixScale, kRetnType_Array);
	ADD_CMD_RET(MatrixAdd, kRetnType_Array);
	ADD_CMD_RET(MatrixSubtract, kRetnType_Array);
	ADD_CMD_RET(MatrixMultiply, kRetnType_Array);

	ADD_CMD(GetAVMod);
	ADD_CMD(GetAVModC);
	ADD_CMD(ModAVMod);
	ADD_CMD(ModAVModC);
	ADD_CMD(SetAVMod);
	ADD_CMD(SetAVModC);
	ADD_CMD(GetMaxAV);
	ADD_CMD(GetMaxAVC);
	ADD_CMD(GetAVForBaseActor);
	ADD_CMD(GetAVForBaseActorC);

	ADD_CMD(SetEventHandler);
	ADD_CMD(RemoveEventHandler);
	ADD_CMD(GetBookLength);

	ADD_CMD_RET(GetClassMenuSelectedClass, kRetnType_Form);
	ADD_CMD_RET(GetClassMenuHighlightedClass, kRetnType_Form);
	ADD_CMD_RET(GetEnchMenuBaseItem, kRetnType_Form);
	ADD_CMD(IsQuestCompleted);
	ADD_CMD(UncompleteQuest);

	ADD_CMD_RET(GetRaceDefaultHair, kRetnType_Form);
	ADD_CMD_RET(GetRaceHairs, kRetnType_Array);
	ADD_CMD_RET(GetRaceEyes, kRetnType_Array);

	ADD_CMD_RET(GetInvRefsForItem, kRetnType_Array);
	ADD_CMD_RET(GetPackageType, kRetnType_String);
	ADD_CMD_RET(GetWeatherOverride, kRetnType_Form);

	ADD_CMD(ClearActiveQuest);
	ADD_CMD(RemoveModLocalData);

	ADD_CMD_RET(GetCurrentEventName, kRetnType_String);

	ADD_CMD_RET(GetLightRGB, kRetnType_Array);
	ADD_CMD(SetLightRGB);
	ADD_CMD_RET(GetTransactionInfo, kRetnType_Array);

	ADD_CMD(PrintActiveTileInfo);
	ADD_CMD(MagicEffectCodeFromChars);

	ADD_CMD_RET(GetAllModLocalData, kRetnType_Array);

	// 0020
	g_scriptCommands.RecordReleaseVersion();

	ADD_CMD_RET(GetCellLighting, kRetnType_Array);
	ADD_CMD(SetCellLighting);
	ADD_CMD(IsNthActiveEffectApplied);
	ADD_CMD_RET(GetMapMarkers, kRetnType_Array);
	g_scriptCommands.Add(&kPaddingCommand);		// used to be GetMouseState
	ADD_CMD(PlayIdle);
	ADD_CMD_RET(ar_CustomSort, kRetnType_Array);

	ADD_CMD_RET(GetAllPathNodes, kRetnType_Array);	// for testing, not recommended for general use (slow)
	ADD_CMD(IsPathNodeDisabled);
	ADD_CMD(SetPathNodeDisabled);
	ADD_CMD(GetPathNodePos);
	ADD_CMD(SetPathNodePreferred);

	ADD_CMD_RET(GetAllPathEdges, kRetnType_Array);	// for testing, not recommended for general use (slow)
	ADD_CMD(PathEdgeExists);
	ADD_CMD(SetPathEdgeExists);

	ADD_CMD_RET(GetCellClimate, kRetnType_Form);
	ADD_CMD(SetCellClimate);
	ADD_CMD(SetCellBehavesAsExterior);
	ADD_CMD(SetCellHasWater);

	ADD_CMD(GetBoundingRadius);
	ADD_CMD(GetEditorSize);

	ADD_CMD_RET(GetNthEffectItem, kRetnType_Array);

	ADD_CMD_RET(GetPathNodeLinkedRef, kRetnType_Form);

	ADD_CMD(SetCellIsPublic);
	ADD_CMD(IsCellPublic);

	ADD_CMD(Internal_PushExecutionContext);
	ADD_CMD(Internal_PopExecutionContext);

	ADD_CMD(GetTerrainHeight);
	ADD_CMD(ResolveModIndex);

	ADD_CMD(SetPos_T);
	ADD_CMD(SetOwnership_T);
	ADD_CMD(ClearOwnership_T);

	ADD_CMD(GetRequiredSkillExp);
	ADD_CMD(HasEffectShader);
	ADD_CMD_RET(GetRaceVoice, kRetnType_Form);

	ADD_CMD(IsOblivionInterior);
	ADD_CMD(IsOblivionWorld);
	ADD_CMD(CanFastTravelFromWorld);
	ADD_CMD(SetCanFastTravelFromWorld);
	ADD_CMD(IsInOblivion);

	ADD_CMD(GetLightDuration);
	ADD_CMD(SetLightDuration);
	ADD_CMD(GetTimeLeft);
	ADD_CMD(SetTimeLeft);

	ADD_CMD(SetCreatureSkill);

	ADD_CMD_RET(GetPathNodesInRadius, kRetnType_Array);

	ADD_CMD(SetTextInputDefaultControlsDisabled);
	ADD_CMD(SetTextInputControlHandler);
	ADD_CMD(SetInputText);

	ADD_CMD_RET(GetPathNodesInRect, kRetnType_Array);

	ADD_CMD_RET(GetCurrentScript, kRetnType_Form);
	ADD_CMD_RET(GetCallingScript, kRetnType_Form);
	ADD_CMD(GetNthActiveEffectActorValue);
	ADD_CMD(SetPlayersLastRiddenHorse);
	ADD_CMD(ClearPlayersLastRiddenHorse);

	ADD_CMD(EquipItemSilent);
	ADD_CMD(UnequipItemSilent);

	ADD_CMD_RET(GetHighActors, kRetnType_Array);
	ADD_CMD_RET(GetMiddleHighActors, kRetnType_Array);

	ADD_CMD(SetVerticalVelocity);

	ADD_CMD(GetSkillSpecialization);
	ADD_CMD(SetSkillSpecialization);

	ADD_CMD_RET(GetCurrentPackageProcedure, kRetnType_String);

	ADD_CMD(GetEquippedTorchTimeLeft);

	ADD_CMD_RET(GetCellWaterType, kRetnType_Form);
	ADD_CMD(SetCellWaterType);

	ADD_CMD(IsHiddenDoor);
	ADD_CMD(IsAutomaticDoor);
	ADD_CMD(IsMinimalUseDoor);

	ADD_CMD(SetIsOblivionGate);
	ADD_CMD(SetIsHiddenDoor);
	ADD_CMD(SetIsAutomaticDoor);
	ADD_CMD(SetIsMinimalUseDoor);

	ADD_CMD(GetVerticalVelocity);

	ADD_CMD(GetLocalGravity);
	ADD_CMD(SetLocalGravity);

	ADD_CMD(SetVelocity);
	ADD_CMD(SetLocalGravityVector);

	ADD_CMD(GetLocalGravity);
	ADD_CMD(GetVelocity);

	ADD_CMD(GetCellNorthRotation);
	ADD_CMD(GetActorBaseLevel);
	ADD_CMD(SetGoldValue_T);
	ADD_CMD(SetRaceScale);
	ADD_CMD(SetRaceWeight);

	ADD_CMD(ToggleSkillPerk);

	// 0021
	g_scriptCommands.RecordReleaseVersion();

	ADD_CMD(SetCombatStylePowerAttackFatigueModMult);
	ADD_CMD(HasTail);
	ADD_CMD(GetLuckModifiedSkill);
	ADD_CMD(SetCellMusicType);
	ADD_CMD(GetSoundAttenuation);
	ADD_CMD(SetSoundAttenuation);

	ADD_CMD_RET(GetStageIDs, kRetnType_Array);
	ADD_CMD_RET(GetStageEntries, kRetnType_Array);
	ADD_CMD(SetStageText);
	ADD_CMD(UnsetStageText);
	ADD_CMD(SetStageDate);

	ADD_CMD_RET(GetTailModelPath, kRetnType_String);
	ADD_CMD(UpdateContainerMenu);
	ADD_CMD(UpdateSpellPurchaseMenu);

	ADD_CMD(ToggleDebugText2);

	ADD_CMD_RET (GetModAlias, kRetnType_String);
	ADD_CMD (SetModAlias);

	ADD_CMD (LinkToDoor);

	ADD_CMD_RET (sv_ToUpper, kRetnType_String);
	ADD_CMD_RET (sv_ToLower, kRetnType_String);
	ADD_CMD (CopyRace);
	ADD_CMD (SetCreatureType);
	ADD_CMD (DispatchEvent);
	ADD_CMD (GetGroundSurfaceMaterial);
	ADD_CMD (GetSkillSpecializationC);
	ADD_CMD (SetSkillSpecializationC);
	ADD_CMD (GetRequiredSkillExpC);
	ADD_CMD (GetAVSkillMasteryLevel);
	ADD_CMD (GetAVSkillMasteryLevelC);
	ADD_CMD_RET(GetFactions, kRetnType_Array);
	ADD_CMD_RET(GetLowActors, kRetnType_Array);
	ADD_CMD_RET (GetLevCreatureTemplate, kRetnType_Form);
	ADD_CMD (SetLevCreatureTemplate);
	ADD_CMD (SetCalcAllLevels);

	ADD_CMD (GetActorSwimBreath);
	ADD_CMD (SetActorSwimBreath);
	ADD_CMD (GetActorMaxSwimBreath);
	ADD_CMD (SetActorMaxSwimBreath);
	ADD_CMD (OverrideActorSwimBreath);
	ADD_CMD (SetFlyCameraSpeedMult);
	//OBSE v21.5-21.8
	ADD_CMD(EventHandlerExist);
	ADD_CMD(GetBaseAV3);
	ADD_CMD(GetBaseAV3C);
    ADD_CMD(IsNaked);
	//OBSE 22.4
	ADD_CMD(SetAltControl2);
	ADD_CMD(IsMiscItem);
	/* to add later if problems can be solved
	g_scriptCommands.Add(&kCommandInfo_SetCurrentClimate); // too many problems
	g_scriptCommands.Add(&kCommandInfo_SetWorldspaceClimate);
	g_scriptCommands.Add(&kCommandInfo_GetParentWorldspace);
	g_scriptCommands.Add(&kCommandInfo_FloatFromFile);
	g_scriptCommands.Add(&kCommandInfo_FloatToFile);
	g_scriptCommands.Add(&kCommandInfo_PrintToFile);
	g_scriptCommands.Add(&kCommandInfo_SetProjectileSource);
	g_scriptCommands.Add(&kCommandInfo_ClearProjectileSource);
	ADD_CMD(UnequipMe);	// not yet fully functional
	*/

	/* to be implemented later
	g_scriptCommands.Add(&kCommandInfo_RemoveHostileEffectItems);
	g_scriptCommands.Add(&kCommandInfo_RemoveNonHostileEffectItems);
	g_scriptCommands.Add(&kCommandInfo_SetBookSkillTaughtC);
	g_scriptCommands.Add(&kCommandInfo_RemoveAllSpells); // had some crashing problems
	g_scriptCommands.Add(&kCommandInfo_SetScriptedEffectItem);
	*/

#ifdef _DEBUG
	// only for testing, don't use in scripts
	g_scriptCommands.Add(&kCommandInfo_TestExtractArgs);
	g_scriptCommands.Add(&kCommandInfo_DumpVarInfo);
	g_scriptCommands.Add(&kCommandInfo_PrintToFile);		//useful for debugging
	g_scriptCommands.Add(&kCommandInfo_DumpLevList);
	g_scriptCommands.Add(&kCommandInfo_DumpAE);
	g_scriptCommands.Add(&kCommandInfo_RunScript);
	g_scriptCommands.Add(&kCommandInfo_scrwtf);				// scruggsy test cmd
	g_scriptCommands.Add(&kCommandInfo_DumpDocs);
	g_scriptCommands.Add(&kCommandInfo_DumpXmlDocs);

#ifdef OBLIVION
	g_scriptCommands.Add(&kCommandInfo_DebugMemDump);
	g_scriptCommands.Add(&kCommandInfo_DumpExtraData);
#endif

#endif

	if(!g_pluginManager.Init())
		_WARNING("couldn't load plugins");

	g_scriptCommands.RemoveDisabledPlugins();

#ifdef _DEBUG
#if 0
	_MESSAGE("console commands");
	g_consoleCommands.Dump();
	_MESSAGE("script commands");
	g_scriptCommands.Dump();
#endif
#endif

	// patch the code
	ApplyPatch(kPatch_ScriptCommands_Start, (UInt32)g_scriptCommands.GetStart());
	ApplyPatch(kPatch_ScriptCommands_End, (UInt32)g_scriptCommands.GetEnd());
	ApplyPatch(kPatch_ScriptCommands_MaxIdx, g_scriptCommands.GetMaxID());

	_MESSAGE("patched");
}

void CommandTable::Read(CommandInfo * start, CommandInfo * end)
{
	UInt32	numCommands = end - start;
	m_commands.reserve(m_commands.size() + numCommands);

	for(; start != end; ++start)
		Add(start);
}

void CommandTable::Add(CommandInfo * info, CommandReturnType retnType, UInt32 parentPluginOpcodeBase)
{
	UInt32	backCommandID = m_baseID + m_commands.size();	// opcode of the next command to add

	info->opcode = m_curID;

	if(m_curID == backCommandID)
	{
		// adding at the end?
		m_commands.push_back(*info);
	}
	else if(m_curID < backCommandID)
	{
		// adding to existing data?
		ASSERT(m_curID >= m_baseID);

		m_commands[m_curID - m_baseID] = *info;
	}
	else
	{
		HALT("CommandTable::Add: adding past the end");
	}

	m_curID++;

	// record return value if other than default
	if (retnType != kRetnType_Default)
		m_returnTypes[info->opcode] = retnType;
	if(parentPluginOpcodeBase)
		m_opcodesByPlugin[info->opcode] = parentPluginOpcodeBase;
}

bool CommandTable::Replace(UInt32 opcodeToReplace, CommandInfo* replaceWith)
{
	for (CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		if (iter->opcode == opcodeToReplace)
		{
			*iter = *replaceWith;
			iter->opcode = opcodeToReplace;
			return true;
		}
	}

	return false;
}

void CommandTable::PadTo(UInt32 id, CommandInfo * info)
{
	if(!info) info = &kPaddingCommand;

	while(m_baseID + m_commands.size() < id)
	{
		info->opcode = m_baseID + m_commands.size();
		m_commands.push_back(*info);
	}

	m_curID = id;
}

void CommandTable::Dump(void)
{
	for(CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		_MESSAGE("%08X %04X %s %s", iter->opcode, iter->needsParent, iter->longName, iter->shortName);
//		_MESSAGE("%08X %08X %08X %08X %08X %s", iter->params, iter->unk1, iter->unk2, iter->unk3, iter->flags, iter->longName);
		gLog.Indent();

#if 0
		for(UInt32 i = 0; i < iter->numParams; i++)
		{
			ParamInfo	* param = &iter->params[i];
			_MESSAGE("%08X %08X %s", param->typeID, param->isOptional, param->typeStr);
		}
#endif

		gLog.Outdent();
	}
}

void CommandTable::DumpAlternateCommandNames(void)
{
	for (CommandList::iterator iter= m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		if (iter->shortName)
			_MESSAGE("%s", iter->shortName);
	}
}

CommandInfo * CommandTable::GetStart(void)
{
	return &m_commands[0];
}

CommandInfo * CommandTable::GetEnd(void)
{
	return &m_commands[0] + m_commands.size();
}

CommandInfo * CommandTable::GetByName(const char * name)
{
	for(CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
		if(!_stricmp(name, iter->longName) || (iter->shortName && !_stricmp(name, iter->shortName)))
			return &(*iter);

	return NULL;
}

CommandInfo* CommandTable::GetByOpcode(UInt32 opcode)
{
	try {
		const auto baseOpcode = m_commands.begin()->opcode;
		const auto arrayIndex = opcode - baseOpcode;
		auto* const command = &m_commands.at(arrayIndex);
		if (command->opcode != opcode) {
			_MESSAGE("ERROR: mismatched command opcodes when executing CommandTable::GetByOpcode (opcode:%X base: %X index %X: %d index opcode %x)",
				opcode, baseOpcode, arrayIndex, command->opcode
			);
			return nullptr;
		}
		return command;
	}
	catch(std::out_of_range&) {
		_MESSAGE("ERROR: opcode %X out of range (end is %X) when executing CommandTable:GetByOpcode", opcode, m_commands.back().opcode);
		return nullptr;
	}
}

CommandReturnType CommandTable::GetReturnType(const CommandInfo* cmd)
{
	if (m_returnTypes.find(cmd->opcode) != m_returnTypes.end())
		return m_returnTypes[cmd->opcode];

	return kRetnType_Default;
}

void CommandTable::SetReturnType(UInt32 opcode, CommandReturnType retnType)
{
	CommandInfo* cmdInfo = GetByOpcode(opcode);
	if (!cmdInfo)
		_MESSAGE("CommandTable::SetReturnType() - cannot locate command with opcode %04X", opcode);
	else
		m_returnTypes[opcode] = retnType;
}

void CommandTable::RecordReleaseVersion(void)
{
	m_opcodesByRelease.push_back(GetCurID());
}

UInt32 CommandTable::GetRequiredOBSEVersion(const CommandInfo* cmd)
{
	UInt32  ver = 0;
	if (cmd) {
		if (cmd->opcode < m_opcodesByRelease[0])	// vanilla cmd
			ver = 0;
		else if (cmd->opcode >= 0x2000)	// plugin cmd, we have no way of knowing
			ver = -1;
		else {
			for (UInt32 i = 0; i < m_opcodesByRelease.size(); i++) {
				if (cmd->opcode >= m_opcodesByRelease[i]) {
					ver = i+8;
				}
				else {
					break;
				}
			}
		}
	}

	return ver;
}

void CommandTable::RemoveDisabledPlugins(void)
{
	for(OpcodeToPluginMap::iterator iter = m_opcodesByPlugin.begin(); iter != m_opcodesByPlugin.end();)
	{
		// we are removing items from the map, so update iter before moving on
		OpcodeToPluginMap::iterator	cur = iter;
		++iter;

		// plugin failed to load but still registered some commands?
		// realistically the game is going to go down hard if this happens anyway
		if(g_pluginManager.LookupHandleFromBaseOpcode(cur->second) == kPluginHandle_Invalid)
		{
			_MESSAGE("removing orphaned command %04X (parent %04X)", cur->first, cur->second);
			m_opcodesByPlugin.erase(cur);
		}
	}
}

PluginInfo * CommandTable::GetParentPlugin(const CommandInfo * cmd)
{
	OpcodeToPluginMap::iterator iter = m_opcodesByPlugin.find(cmd->opcode);
	if(iter != m_opcodesByPlugin.end())
	{
		PluginInfo	* info = g_pluginManager.GetInfoFromBase(iter->second);
		if(info)
			return info;
	}

	return NULL;
}

const char* SimpleStringForParamType(UInt32 paramType)
{
	switch(paramType) {
		case kParamType_String: return "string";
		case kParamType_Integer: return "integer";
		case kParamType_Float: return "float";
		case kParamType_InventoryObject: return "ref";
		case kParamType_ObjectRef: return "ref";
		case kParamType_ActorValue: return "actorValue";
		case kParamType_Actor: return "ref";
		case kParamType_SpellItem: return "ref";
		case kParamType_Axis: return "axis";
		case kParamType_Cell: return "ref";
		case kParamType_AnimationGroup: return "animGroup";
		case kParamType_MagicItem: return "ref";
		case kParamType_Sound: return "ref";
		case kParamType_Topic: return "ref";
		case kParamType_Quest: return "ref";
		case kParamType_Race: return "ref";
		case kParamType_Class: return "ref";
		case kParamType_Faction: return "ref";
		case kParamType_Sex: return "sex";
		case kParamType_Global: return "global";
		case kParamType_Furniture: return "ref";
		case kParamType_TESObject: return "ref";
		case kParamType_VariableName: return "string";
		case kParamType_QuestStage: return "short";
		case kParamType_MapMarker: return "ref";
		case kParamType_ActorBase: return "ref";
		case kParamType_Container: return "ref";
		case kParamType_WorldSpace: return "ref";
		case kParamType_CrimeType: return "crimeType";
		case kParamType_AIPackage: return "ref";
		case kParamType_CombatStyle: return "ref";
		case kParamType_MagicEffect: return "ref";
		case kParamType_FormType: return "formType";
		case kParamType_WeatherID: return "ref";
		case kParamType_NPC: return "ref";
		case kParamType_Owner: return "ref";
		case kParamType_EffectShader: return "ref";
		case kParamType_Birthsign: return "ref";
		default: return "<unknown>";
	}
}

const char* StringForParamType(UInt32 paramType)
{
	switch(paramType) {
		case kParamType_String: return "String";
		case kParamType_Integer: return "Integer";
		case kParamType_Float: return "Float";
		case kParamType_InventoryObject: return "InventoryObject";
		case kParamType_ObjectRef: return "ObjectRef";
		case kParamType_ActorValue: return "ActorValue";
		case kParamType_Actor: return "Actor";
		case kParamType_SpellItem: return "SpellItem";
		case kParamType_Axis: return "Axis";
		case kParamType_Cell: return "Cell";
		case kParamType_AnimationGroup: return "AnimationGroup";
		case kParamType_MagicItem: return "MagicItem";
		case kParamType_Sound: return "Sound";
		case kParamType_Topic: return "Topic";
		case kParamType_Quest: return "Quest";
		case kParamType_Race: return "Race";
		case kParamType_Class: return "Class";
		case kParamType_Faction: return "Faction";
		case kParamType_Sex: return "Sex";
		case kParamType_Global: return "Global";
		case kParamType_Furniture: return "Furniture";
		case kParamType_TESObject: return "Object";
		case kParamType_VariableName: return "VariableName";
		case kParamType_QuestStage: return "QuestStage";
		case kParamType_MapMarker: return "MapMarker";
		case kParamType_ActorBase: return "ActorBase";
		case kParamType_Container: return "Container";
		case kParamType_WorldSpace: return "WorldSpace";
		case kParamType_CrimeType: return "CrimeType";
		case kParamType_AIPackage: return "AIPackage";
		case kParamType_CombatStyle: return "CombatStyle";
		case kParamType_MagicEffect: return "MagicEffect";
		case kParamType_FormType: return "FormType";
		case kParamType_WeatherID: return "WeatherID";
		case kParamType_NPC: return "NPC";
		case kParamType_Owner: return "Owner";
		case kParamType_EffectShader: return "EffectShader";
		case kParamType_Birthsign: return "Birthsign";
		default: return "<unknown>";
	}
}

bool IsParamOptionalCallingRef(UInt32 paramType)
{
	switch(paramType)
	{
		case kParamType_InventoryObject:
		case kParamType_ObjectRef:
		case kParamType_Actor:
		case kParamType_Furniture:
		case kParamType_TESObject:
		case kParamType_MapMarker:
		case kParamType_ActorBase:
		case kParamType_Container:
		case kParamType_NPC:
		case kParamType_Owner:
			return true;
	}
	return false;
}

void CommandTable::DumpCommandDocumentation(UInt32 startWithID)
{
	_MESSAGE("FOSE Commands from: %#x", startWithID);

	CommandList::iterator itEnd = m_commands.end();
	_MESSAGE("<br><b>Function List</b>\n<ul>\n");
	for (CommandList::iterator iter = m_commands.begin();iter != itEnd; ++iter) {
		if (iter->opcode >= startWithID) {
			_MESSAGE("<li><a href=\"#%s\">%s</a></li>", iter->longName, iter->longName);
		}
	}
	_MESSAGE("</ul>\n\n");

	_MESSAGE("<br><b>Function Quick Reference</b>");
	for (CommandList::iterator iter = m_commands.begin();iter != itEnd; ++iter) {
		if (iter->opcode >= startWithID) {
			iter->DumpFunctionDef();
		}
	}

	_MESSAGE("<hr><br><b>Functions In Detail</b>");
	for (CommandList::iterator iter = m_commands.begin();iter != itEnd; ++iter) {
		if (iter->opcode >= startWithID) {
			iter->DumpDocs();
		}
	}
}

void CommandTable::DumpCommandXML(UInt32 startWithID)
{
	std::ofstream out("obse_command_docs_autogen.xml");
	out << "<doc name=\"OBSE Command Documentation\">";

	CommandList::iterator itEnd = m_commands.end();
	for (CommandList::iterator iter = m_commands.begin(); iter != itEnd; ++iter)
	{
		if (iter->opcode >= startWithID)
		{
			out << "\n";
			iter->DumpXML(out);
		}
	}

	out << "</doc>";
}

CommandTable	g_consoleCommands;
CommandTable	g_scriptCommands;
//CommandTable	g_tokenCommands;

void CommandInfo::DumpDocs() const
{
	const char* name = longName;
	const char* retnTypes[] = {"","ref","string","array","arrayIndex","multi"};

	_MESSAGE("<p><a id=\"%s\" class=\"f\" href=\"http://cs.elderscrolls.com/constwiki/index.php/%s", name, name);

	_MESSAGE("\">%s", name);
	if (shortName && shortName[0]) {
		_MESSAGE(" (%s)", shortName);
	}
	_MESSAGE("</a> - %s<br />", helpText);

	_MESSAGE("<code class=\"s\">(%s) ", retnTypes[g_scriptCommands.GetReturnType(this)]);

	if (needsParent) {
		_MESSAGE("reference.");
	}

	_MESSAGE(name);

	bool usesOBSEParams = parse != Cmd_Default_Parse;
	for (UInt32 i=0; i<numParams; i++) {
		ParamInfo* param = &params[i];
		if (param->isOptional) {
			_MESSAGE("<span class=\"op\">");
		}

		_MESSAGE(" %s:%s", param->typeStr, SimpleStringForParamType(param->typeID));

		if (param->isOptional) {
			_MESSAGE("</span>");
		}
	}

	_MESSAGE("</code></p>\n\n");

#if 0
	_MESSAGE("<p><a name=\"%s\"></a><b>%s</b> ", longName, longName);
	_MESSAGE("<br><b>Alias:</b> %s<br><b>Parameters:</b>%d", (strlen(shortName) != 0) ? shortName : "none", numParams);
	if (numParams > 0) {
		for(UInt32 i = 0; i < numParams; i++)
		{
			ParamInfo	* param = &params[i];
			const char* paramTypeName = StringForParamType(param->typeID);
			if (param->isOptional != 0) {
				_MESSAGE("<br>&nbsp;&nbsp;&nbsp;<i>%s:%s</i> ", param->typeStr, paramTypeName);
			} else {
				_MESSAGE("<br>&nbsp;&nbsp;&nbsp;%s:%s ", param->typeStr, paramTypeName);
			}
		}
	}
	_MESSAGE("<br><b>Return Type:</b> FixMe<br><b>Opcode:</b> %#4x (%d)<br><b>Condition Function:</b> %s<br><b>Description:</b> %s</p>", opcode, opcode, eval ? "Yes" : "No",helpText);
#endif
}

void CommandInfo::DumpXML(std::ofstream& out)
{
	// command
	out << "\t<command name = \"" << longName << "\"";
	if (shortName && strlen(shortName))
		out << " alias=\"" << shortName << "\"";

	CommandReturnType retnType = g_scriptCommands.GetReturnType(this);
	out << " returntype=\"";
	switch (retnType)
	{
	case kRetnType_Default:
		out << "nothing?";
		break;
	case kRetnType_Form:
		out << "form";
		break;
	case kRetnType_String:
		out << "string";
		break;
	case kRetnType_Array:
		out << "array?";
		break;
	case kRetnType_ArrayIndex:
		out << "arrayKey";
		break;
	case kRetnType_Ambiguous:
		out << "multi";
		break;
	default:
		out << "UNKNOWN_ENUM_VAL?";
	};

	out << "\"";

	out << " ref=\"";
	if (needsParent)
		out << "required\"";
	else
	{
		if (numParams && params[numParams-1].isOptional && IsParamOptionalCallingRef(params[numParams-1].typeID))
			out << "optional?\"";
		else
			out << "none\"";
	}

	out << " category=\"cat?\">";

	// description
	out << "\n\t\t<description>";
	if (helpText)
		out << helpText;
	else
		out << "nodescription?";

	out << "</description>\n";

	// args
	bool bIsOBSEParamInfo = (parse != Cmd_Default_Parse);

	for (UInt32 i = 0; i < numParams; i++)
	{
		ParamInfo* param = &params[i];

		out << "\t\t<arg type=\"";
		if (!bIsOBSEParamInfo)
			out << StringForParamType(param->typeID);
		else
			out << "obseparam?";

		out << "\" optional=\"";
		if (param->isOptional)
			out << "true";
		else
			out << "false";

		out << "\">" << param->typeStr << "</arg>\n";
	}

	out << "\t</command>\n";
}

void CommandInfo::DumpFunctionDef() const
{
	_MESSAGE("<br>(FixMe) %s<a href=\"#%s\">%s</a> ", needsParent > 0 ? "reference." : "", longName, longName);
	if (numParams > 0) {
		for(UInt32 i = 0; i < numParams; i++)
		{
			ParamInfo	* param = &params[i];
			const char* paramTypeName = StringForParamType(param->typeID);
			if (param->isOptional != 0) {
				_MESSAGE("<i>%s:%s</i> ", param->typeStr, paramTypeName);
			} else {
				_MESSAGE("%s:%s ", param->typeStr, paramTypeName);
			}
		}
	}
}

namespace PluginAPI {
	const CommandInfo* GetCmdTblStart() { return g_scriptCommands.GetStart(); }
	const CommandInfo* GetCmdTblEnd() { return g_scriptCommands.GetEnd(); }
	const CommandInfo* GetCmdByOpcode(UInt32 opcode) { return g_scriptCommands.GetByOpcode(opcode); }
	const CommandInfo* GetCmdByName(const char* name) { return g_scriptCommands.GetByName(name); }
	UInt32 GetCmdRetnType(const CommandInfo* cmd) { return g_scriptCommands.GetReturnType(cmd); }
	UInt32 GetReqVersion(const CommandInfo* cmd) { return g_scriptCommands.GetRequiredOBSEVersion(cmd); }
	const PluginInfo* GetCmdParentPlugin(const CommandInfo* cmd) { return g_scriptCommands.GetParentPlugin(cmd); }
}
