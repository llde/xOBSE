#include "Commands_Script.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameData.h"

#include "EventManager.h"
#include "FunctionScripts.h"
#include "ModTable.h"

enum EScriptMode {
	eScript_HasScript,
	eScript_Get,
	eScript_Remove,
};

static bool GetScript_Execute(COMMAND_ARGS, EScriptMode eMode)
{
	*result = 0;
	TESForm* form = 0;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
	}

	TESScriptableForm* scriptForm = (TESScriptableForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESScriptableForm, 0);
	Script* script = (scriptForm) ? scriptForm->script : NULL;

	if (eMode == eScript_HasScript) {
		*result = (script != NULL) ? 1 : 0;
	} else {
		if (script) {
			UInt32* refResult = (UInt32*)result;
			*refResult = script->refID;
		}
		if (eMode == eScript_Remove && scriptForm) {
			scriptForm->script = NULL;
		}
	}

	return true;
}

static bool Cmd_IsScripted_Execute(COMMAND_ARGS)
{
	return GetScript_Execute(PASS_COMMAND_ARGS, eScript_HasScript);
}

static bool Cmd_GetScript_Execute(COMMAND_ARGS)
{
	return GetScript_Execute(PASS_COMMAND_ARGS, eScript_Get);
}

static bool Cmd_RemoveScript_Execute(COMMAND_ARGS)
{
	return GetScript_Execute(PASS_COMMAND_ARGS, eScript_Remove);
}

static bool Cmd_SetScript_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32*)result;

	TESForm* form = NULL;
	TESForm* scriptArg = NULL;
	bool bModifyingThisObj = false;

	ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &scriptArg, &form);
	form = form->TryGetREFRParent();
	if (!form) {
		if (!thisObj) return true;
		form = thisObj->baseForm;
		bModifyingThisObj = true;
	}

	TESScriptableForm* scriptForm = (TESScriptableForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESScriptableForm, 0);
	if (!scriptForm) return true;

	Script* script = (Script*)Oblivion_DynamicCast(scriptArg, 0, RTTI_TESForm, RTTI_Script, 0);
	if (!script) return true;

	// we can't get a magic script here
	if (script->IsMagicScript()) return true;

	if (script->IsQuestScript() && form->typeID == kFormType_Quest) {
		*refResult = scriptForm->script->refID;

		// if the quest already has an eventlist, destroy it and create a new one
		TESQuest* quest = OBLIVION_CAST(form, TESForm, TESQuest);
		if (quest->scriptEventList) {
			quest->scriptEventList->Destructor();
			FormHeap_Free(quest->scriptEventList);
			quest->scriptEventList = script->CreateEventList();
		}

		scriptForm->script = script;
	} else if (script->IsObjectScript()) {
		if (scriptForm->script) {
			*refResult = scriptForm->script->refID;
		}
		if (bModifyingThisObj) {
			// create or replace event list
			ExtraScript* xScript = (ExtraScript*)thisObj->baseExtraList.GetByType(kExtraData_Script);
			if (xScript) {
				if (xScript->eventList) {
					xScript->eventList->Destructor();
					FormHeap_Free(xScript->eventList);
					xScript->script = script;
					xScript->eventList = script->CreateEventList();
				}
			}
			else {
				xScript = ExtraScript::Create(script);
				thisObj->baseExtraList.Add(xScript);
			}
		}

		scriptForm->script = script;
	}
	return true;
}

static bool Cmd_IsFormValid_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = 0;
	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form) && form)
		*result = 1;
	return true;
}

static bool Cmd_IsReference_Execute(COMMAND_ARGS)
{
	TESForm* refr = NULL;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &refr))
		if (refr && refr->IsReference()) *result = 1;

	return true;
}

enum {
	eScriptVar_Get = 1,
	eScriptVar_GetRef,
	eScriptVar_Has,
};

static bool GetVariable_Execute(COMMAND_ARGS, UInt32 whichAction)
{
	char varName[256] = { 0 };
	TESQuest* quest = NULL;
	Script* targetScript = NULL;
	ScriptEventList* targetEventList = NULL;
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &varName, &quest))
		return true;
	if (quest)
	{
		TESScriptableForm* scriptable = (TESScriptableForm*)Oblivion_DynamicCast(quest, 0, RTTI_TESQuest, RTTI_TESScriptableForm, 0);
		targetScript = scriptable->script;
		targetEventList = quest->scriptEventList;
	}
	else if (thisObj)
	{
		TESScriptableForm* scriptable = (TESScriptableForm*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESScriptableForm, 0);
		if (scriptable)
		{
			targetScript = scriptable->script;
			targetEventList = thisObj->GetEventList();
		}
	}

	if (targetScript && targetEventList)
	{
		Script::VariableInfo* varInfo = targetScript->GetVariableByName(varName);
		if (whichAction == eScriptVar_Has)
			return varInfo ? true : false;
		else if (varInfo)
		{
			ScriptEventList::Var* var = targetEventList->GetVariable(varInfo->idx);
			if (var)
			{
				if (whichAction == eScriptVar_Get)
					*result = var->data;
				else if (whichAction == eScriptVar_GetRef)
				{
					UInt32* refResult = (UInt32*)result;
					*refResult = (*(UInt64*)&var->data);
				}
				return true;
			}
		}
	}

	return false;
}

static bool Cmd_HasVariable_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (GetVariable_Execute(PASS_COMMAND_ARGS, eScriptVar_Has))
		*result = 1;

	return true;
}

static bool Cmd_GetVariable_Execute(COMMAND_ARGS)
{
	GetVariable_Execute(PASS_COMMAND_ARGS, eScriptVar_Get);
	return true;
}

static bool Cmd_GetRefVariable_Execute(COMMAND_ARGS)
{
	GetVariable_Execute(PASS_COMMAND_ARGS, eScriptVar_GetRef);
	return true;
}

static bool Cmd_GetArrayVariable_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active()) {
		ShowRuntimeError(scriptObj, "GetArrayVariable must be called within the context of an OBSE expression");
		return false;
	}

	GetVariable_Execute(PASS_COMMAND_ARGS, eScriptVar_Get);
	return true;
}

static bool Cmd_CompareScripts_Execute(COMMAND_ARGS)
{
	Script* script1 = NULL;
	Script* script2 = NULL;
	*result = 0;

	if (!ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &script1, &script2))
		return true;
	script1 = (Script*)Oblivion_DynamicCast(script1, 0, RTTI_TESForm, RTTI_Script, 0);
	script2 = (Script*)Oblivion_DynamicCast(script2, 0, RTTI_TESForm, RTTI_Script, 0);

	if (script1 && script2 && script1->info.dataLength == script2->info.dataLength)
	{
		if (script1 == script2)
			*result = 1;
		else if (!memcmp(script1->data, script2->data, script1->info.dataLength))
			*result = 1;
	}

	return true;
}

static bool Cmd_ResetAllVariables_Execute(COMMAND_ARGS)
{
	//sets all vars to 0
	*result = 0;

	ScriptEventList* list = eventList;			//reset calling script by default
	if (thisObj)								//call on a reference to reset that ref's script vars
		list = thisObj->GetEventList();

	if (list)
		*result = list->ResetAllVariables();

	return true;
}

static bool Cmd_GetFormFromMod_Execute(COMMAND_ARGS)
{
	char refIDString[0x20] = { 0 };
	char modName[0x100] = { 0 };
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &modName, &refIDString))
	{
		UInt8 modIndex = 0xFF;
		if (_stricmp(modName, "NONE"))	// pass "NONE" to look up dynamic form (in savegame)
		{
			modIndex = ModTable::Get().GetModIndex(modName);
			if (modIndex == 0xFF)	// mod not loaded
				return true;
		}

		UInt32 refID;
		if(sscanf_s(refIDString, "%x", &refID) == 1)
		{
			refID &= 0x00FFFFFF;
			refID |= (modIndex << 24);
			DEBUG_PRINT("Fixed refID = %08x", refID);

			TESForm* form = LookupFormByID(refID);
			if (form)
				*refResult = refID;
		}
	}

	return true;
}

class ExplicitRefFinder
{
public:
	bool Accept(const Script::RefVariable* var)
	{
		if (var && var->varIdx == 0)
			return true;

		return false;
	}
};

static Script* GetScriptArg(TESObjectREFR* thisObj, TESForm* form)
{
	Script* targetScript = NULL;
	if (form)
		targetScript = (Script*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_Script, 0);
	else if (thisObj)
	{
		TESScriptableForm* scriptable = (TESScriptableForm*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESScriptableForm, 0);
		if (scriptable)
			targetScript = scriptable->script;
	}

	return targetScript;
}

static bool Cmd_GetNumExplicitRefs_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	Script* targetScript = NULL;
	*result = 0;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &form))
	{
		targetScript = GetScriptArg(thisObj, form);
		if (targetScript)
			*result = (Script::RefListVisitor(&targetScript->refList).CountIf(ExplicitRefFinder()));
	}

	if (IsConsoleMode())
		Console_Print("GetNumExplicitRefs >> %.0f", *result);

	return true;
}

static bool Cmd_GetNthExplicitRef_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 refIdx = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &refIdx, &form))
	{
		Script* targetScript = GetScriptArg(thisObj, form);
		if (targetScript)
		{
			Script::RefListVisitor visitor(&targetScript->refList);
			UInt32 count = 0;
			const Script::RefListEntry* entry = NULL;
			while (count <= refIdx)
			{
				entry = visitor.Find(ExplicitRefFinder(), entry);
				if (!entry)
					break;

				count++;
			}

			if (count == refIdx + 1 && entry && entry->var && entry->var->form)
			{
				*refResult = entry->var->form->refID;
				if (IsConsoleMode())
					Console_Print("GetNthExplicitRef >> %s (%08x)", GetFullName(entry->var->form), *refResult);
			}
		}
	}

	return true;
}

static bool Cmd_RunScripts_Execute(COMMAND_ARGS)
{
	if (thisObj)
		*result = thisObj->RunScripts() ? 1 : 0;

	return true;
}

static bool Cmd_RunScript_Execute(COMMAND_ARGS)
{
	TESForm* scriptForm = NULL;

	if (ExtractArgsEx(PASS_EXTRACT_ARGS_EX, &scriptForm))
	{
		Script* script = OBLIVION_CAST(scriptForm, TESForm, Script);
		if (script)
		{
			bool runResult = CALL_MEMBER_FN(script, Execute)(thisObj, 0, 0, 0);
			Console_Print("ran script, returned %s",runResult ? "true" : "false");
		}
	}

	return true;
}

static bool ExtractEventCallback(ExpressionEvaluator& eval, EventManager::EventCallback* outCallback, char* outName)
{
	if (eval.ExtractArgs() && eval.NumArgs() >= 2) {
		const char* eventName = eval.Arg(0)->GetString();
		Script* script = OBLIVION_CAST(eval.Arg(1)->GetTESForm(), TESForm, Script);
		if (eventName && script) {
			strcpy_s(outName, 0x20, eventName);
			TESObjectREFR* sourceFilter = NULL;
			TESForm* targetFilter = NULL;
			TESObjectREFR* thisObjFilter = NULL;

			// any filters?
			//TODO respect the parameters
			for (UInt32 i = 2; i < eval.NumArgs(); i++) {
				const TokenPair* pair = eval.Arg(i)->GetPair();
				if (pair && pair->left && pair->right) {
					const char* key = pair->left->GetString();
					if (key) {
						if (!_stricmp(key, "ref") || !_stricmp(key, "first")) {
							if (_stricmp(eventName, "OnKeyEvent") == 0 || _stricmp(eventName, "OnControlEvent") == 0) 
								sourceFilter = (TESObjectREFR*)(UInt32)pair->right->GetNumber();
							else sourceFilter = OBLIVION_CAST(pair->right->GetTESForm(), TESForm, TESObjectREFR);
						}
						else if (!_stricmp(key, "object") || !_stricmp(key, "second")) {
							if (_stricmp(eventName, "OnKeyEvent") == 0 || _stricmp(eventName, "OnControlEvent") == 0)
								targetFilter = (TESObjectREFR*)(UInt32)pair->right->GetNumber();

							// special-case MGEF
							else if (!_stricmp(eventName, "onmagiceffecthit")) {
								const char* effStr = pair->right->GetString();
								if (effStr && strlen(effStr) == 4) {
									targetFilter = EffectSetting::EffectSettingForC(*((UInt32*)effStr));
								}
							}
							else if (!_stricmp(eventName, "onhealthdamage")) {
								// special-case OnHealthDamage - don't filter by damage, do filter by damaged actor
								thisObjFilter = OBLIVION_CAST(pair->right->GetTESForm(), TESForm, TESObjectREFR);
							}
							else {
								targetFilter = pair->right->GetTESForm();
							}
						}
					}
				}
			}

			*outCallback = EventManager::EventCallback(script, sourceFilter, targetFilter, thisObjFilter);
			return true;
		}
	}

	return false;
}

static bool Cmd_SetEventHandler_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	EventManager::EventCallback callback;
	char eventName[0x20];
	if (ExtractEventCallback(eval, &callback, eventName)) {
		if (EventManager::SetHandler(eventName, callback))
			*result = 1.0;
	}

	return true;
}

static bool Cmd_RemoveEventHandler_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	EventManager::EventCallback callback;
	char eventName[0x20];
	if (ExtractEventCallback(eval, &callback, eventName)) {
		if (EventManager::RemoveHandler(eventName, callback))
			*result = 1.0;
	}

	return true;
}

static bool Cmd_EventHandlerExist_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	EventManager::EventCallback callback;
	char eventName[0x20];
	*result = 0.0;
	if (ExtractEventCallback(eval, &callback, eventName)) {
		if (EventManager::EventHandlerExist(eventName, callback))
			*result = 1.0;
	}
	return true;
}

static bool Cmd_GetCurrentEventName_Execute(COMMAND_ARGS)
{
	std::string&& eventName = EventManager::GetCurrentEventName();
	AssignToStringVar(PASS_COMMAND_ARGS, eventName.c_str());
	return true;
}

static bool Cmd_GetCurrentScript_Execute(COMMAND_ARGS)
{
	// apparently this is useful
	UInt32* refResult = (UInt32*)result;
	*refResult = scriptObj->refID;
	return true;
}

static bool Cmd_GetCallingScript_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;
	Script* caller = UserFunctionManager::GetInvokingScript(scriptObj);
	if (caller) {
		*refResult = caller->refID;
	}

	return true;
}

static bool Cmd_DispatchEvent_Execute (COMMAND_ARGS)
{
	ExpressionEvaluator eval (PASS_COMMAND_ARGS);
	if (!eval.ExtractArgs () || eval.NumArgs() == 0)
		return true;

	const char* eventName = eval.Arg(0)->GetString ();
	if (!eventName)
		return true;

	ArrayID argsArrayId = 0;
	const char* senderName = NULL;
	if (eval.NumArgs() > 1)
	{
		if (!eval.Arg(1)->CanConvertTo (kTokenType_Array))
			return true;
		argsArrayId = eval.Arg(1)->GetArray ();

		if (eval.NumArgs() > 2)
			senderName = eval.Arg(2)->GetString ();
	}

	*result = EventManager::DispatchUserDefinedEvent (eventName, scriptObj, argsArrayId, senderName) ? 1.0 : 0.0;
	return true;
}

#endif

CommandInfo kCommandInfo_IsScripted =
{
	"IsScripted",
	"",
	0,
	"returns 1 if the object or reference has a script attached to it",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_IsScripted_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetScript =
{
	"GetScript",
	"",
	0,
	"returns the script of the referenced or passed object",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetScript_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveScript =
{
	"RemoveScript",
	"",
	0,
	"removes the script of the referenced or passed object",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_RemoveScript_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

ParamInfo kParamInfo_SetScript[2] =
{
	{	"script", kParamType_MagicItem, 0 },
	{	"object", kParamType_InventoryObject, 1},
};

CommandInfo kCommandInfo_SetScript =
{
	"SetScript",
	"",
	0,
	"returns the script of the referenced or passed object",
	0,
	2,
	kParamInfo_SetScript,
	HANDLER(Cmd_SetScript_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsFormValid =
{
	"IsFormValid",
	"",
	0,
	"returns true if the ref variable contains a valid form",
	0,
	1,
	kParams_OneInventoryObject,
	HANDLER(Cmd_IsFormValid_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsReference =
{
	"IsReference",
	"",
	0,
	"returns true if the ref variable contains a REFR",
	0,
	1,
	kParams_OneObjectRef,
	HANDLER(Cmd_IsReference_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetVariable[2] =
{
	{	"variable name",	kParamType_String,	0	},
	{	"quest",			kParamType_Quest,	1	},
};

CommandInfo kCommandInfo_GetVariable =
{
	"GetVariable",
	"GetVar",
	0,
	"looks up the value of a variable by name",
	0,
	2,
	kParams_GetVariable,
	HANDLER(Cmd_GetVariable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_HasVariable =
{
	"HasVariable",
	"HasVar",
	0,
	"returns true if the script has a variable with the specified name",
	0,
	2,
	kParams_GetVariable,
	HANDLER(Cmd_HasVariable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetRefVariable =
{
	"GetRefVariable",
	"GetRefVar",
	0,
	"looks up the value of a ref variable by name",
	0,
	2,
	kParams_GetVariable,
	HANDLER(Cmd_GetRefVariable_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_CMD_ALT(GetArrayVariable, GetArrayVar, looks up an array variable by name on the calling object or specified quest, 0, kParams_GetVariable);

static ParamInfo kParams_CompareScripts[2] =
{
	{	"script",	kParamType_InventoryObject,	0	},
	{	"script",	kParamType_InventoryObject,	0	},
};

CommandInfo kCommandInfo_CompareScripts =
{
	"CompareScripts",
	"",
	0,
	"returns true if the compiled scripts are identical",
	0,
	2,
	kParams_CompareScripts,
	HANDLER(Cmd_CompareScripts_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(ResetAllVariables,
			   sets all variables in a script to zero,
			   0,
			   0,
			   NULL);

static ParamInfo kParams_GetFormFromMod[2] =
{
	{	"modName",	kParamType_String,	0	},
	{	"formID",	kParamType_String,	0	},
};

DEFINE_COMMAND(GetFormFromMod,
			   looks up a form from another mod,
			   0,
			   2,
			   kParams_GetFormFromMod);

DEFINE_COMMAND(GetNumExplicitRefs,
			   returns the number of literal references in a script,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetNthExplicitRef,
			   returns the nth literal reference in a script,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInventoryObject);

DEFINE_COMMAND(RunScripts,
			   runs all scripts attached to the calling object,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(RunScript,
			   debug,
			   0,
			   1,
			   kParams_OneInventoryObject);

static ParamInfo kOBSEParams_SetEventHandler[4] =
{
	{ "event name",			kOBSEParamType_String,	0 },
	{ "function script",	kOBSEParamType_Form,	0 },
	{ "filter",				kOBSEParamType_Pair,	1 },
	{ "filter",				kOBSEParamType_Pair,	1 },
};

CommandInfo kCommandInfo_SetEventHandler =
{
	"SetEventHandler", "", 0,
	"defines a function script to serve as a callback for game events",
	0, 4, kOBSEParams_SetEventHandler,
	HANDLER(Cmd_SetEventHandler_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveEventHandler =
{
	"RemoveEventHandler", "", 0,
	"removes event handlers matching the event, script, and optional filters specified",
	0, 4, kOBSEParams_SetEventHandler,
	HANDLER(Cmd_RemoveEventHandler_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_EventHandlerExist =
{
	"EventHandlerExist", "", 0,
	"check if an event handler matching the event, script, and optional filters specified exists",
	0, 4, kOBSEParams_SetEventHandler,
	HANDLER(Cmd_EventHandlerExist_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetCurrentEventName, returns the name of the event currently being processed by an event handler,
			   0, 0, NULL);
DEFINE_COMMAND(GetCurrentScript, returns the calling script, 0, 0, NULL);
DEFINE_COMMAND(GetCallingScript, returns the script that called the executing function script, 0, 0, NULL);

static ParamInfo kOBSEParams_DispatchEvent[3] =
{
	{	"eventName",			kOBSEParamType_String,	0	},
	{	"args",					kOBSEParamType_Array,	1	},
	{	"sender",				kOBSEParamType_String,	1	}
};

CommandInfo kCommandInfo_DispatchEvent =
{
	"DispatchEvent", "", 0,
	"dispatches a user-defined event to any registered listeners",
	0, 3, kOBSEParams_DispatchEvent,
	HANDLER(Cmd_DispatchEvent_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};