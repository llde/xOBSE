#include "Script.h"
#include "GameForms.h"
#include "obse/GameAPI.h"
#include "obse/GameObjects.h"
#include "obse/CommandTable.h"
#include "GameMenus.h"
#include "GameData.h"
#include "GameTiles.h"
#include "StringVar.h"
#include <cstdarg>

#if OBSE_CORE
#include "Hooks_Script.h"
#include "ScriptUtils.h"
#endif

#include <float.h>
#include <cctype>
#include <set>

/***
 *	opcodes
 *
 *	tokens
 *	0010	begin
 *	0011	end
 *	0012	short		int
 *	0013	long
 *	0014	float
 *	0015	set
 *	0016	if
 *	0017	else
 *	0018	elseif
 *	0019	endif
 *	001A	while_DEPRECATED
 *	001B	endwhile_DEPRECATED
 *	001C	.
 *	001D	scriptname	scn
 *	001E	return
 *	001F	reference	ref
 *
 ***/

static OBSEStringVarInterface* s_StringVarInterface = NULL;

void ScriptEventList::Dump(void)
{
	for(EventEntry * entry = m_eventList; entry; entry = entry->next)
	{
		if(entry->event)
		{
			Event	* event = entry->event;

			Console_Print("%08X (%s) %08X", event->object, GetObjectClassName(event->object), event->eventMask);
		}
	}
}

UInt32 ScriptEventList::ResetAllVariables()
{
	UInt32 numVars = 0;
	for (VarEntry * entry = m_vars; entry; entry = entry->next)
		if (entry->var)
		{
			entry->var->data = 0.0;
			numVars++;
		}

	return numVars;
}

ScriptEventList::Var * ScriptEventList::GetVariable(UInt32 id)
{
	for(VarEntry * entry = m_vars; entry; entry = entry->next)
		if(entry->var && entry->var->id == id)
			return entry->var;

	return NULL;
}

ScriptEventList* EventListFromForm(TESForm* form)
{
	ScriptEventList* eventList = NULL;
	TESObjectREFR* refr = OBLIVION_CAST(form, TESForm, TESObjectREFR);
	if (refr)
		eventList = refr->GetEventList();
	else
	{
		TESQuest* quest = OBLIVION_CAST(form, TESForm, TESQuest);
		if (quest)
			eventList = quest->scriptEventList;
	}

	return eventList;
}

void ScriptEventList::Destructor()
{
	ThisStdCall(0x004FB4E0, this);

}

// arg1 = 1, ignored if canCreateNew is false, passed to 'init' function if a new object is created
typedef void * (* _GetSingleton)(bool canCreateNew, bool arg1);
typedef bool (* _IsMenuMode)(void);

const _Console_Print Console_Print = (_Console_Print)0x00579B9B;
const _ExtractArgs ExtractArgs = (_ExtractArgs)0x004FAE80;
const _CreateFormInstance CreateFormInstance = (_CreateFormInstance)0x00447DF0;
const _LookupFormByID LookupFormByID = (_LookupFormByID)0x0046B250;
const _FormHeap_Allocate FormHeap_Allocate = (_FormHeap_Allocate)0x00401F00;
const _FormHeap_Free FormHeap_Free = (_FormHeap_Free)0x00401F20;
const _GetGlobalScriptStateObj GetGlobalScriptStateObj = (_GetGlobalScriptStateObj)0x00585C10;
const _GetSingleton InterfaceManager_GetSingleton = (_GetSingleton)0x00582160;
const _ShowMessageBox ShowMessageBox = (_ShowMessageBox)0x00579C10;
const _ShowMessageBox_Callback ShowMessageBox_Callback = (_ShowMessageBox_Callback)0x005003E0;
const _ShowMessageBox_pScriptRefID ShowMessageBox_pScriptRefID = (_ShowMessageBox_pScriptRefID)0x00B361C8;
const _QueueUIMessage QueueUIMessage = (_QueueUIMessage)0x0057ACC0;
const _QueueUIMessage_2 QueueUIMessage_2 = (_QueueUIMessage_2)0x0057ADD0;
const _ShowMessageBox_button ShowMessageBox_button = (_ShowMessageBox_button)0x00B09E24;
MemoryHeap * g_formHeap = (MemoryHeap *)0x00B02020;
const bool * g_bConsoleMode = (bool*)0x00B361AC;
const _IsGodMode IsGodMode = (_IsGodMode)0x0065D820;
const _ScancodeToChar ScancodeToChar = (_ScancodeToChar)0x00403CF0;
const bool * g_bIsConsoleOpen = (bool*)0x00B33415;
const _IsMenuMode IsMenuMode = (_IsMenuMode)0x00578F60;
Script** g_ExecutingScript = (Script**)0x00B361B0;
ScriptEventList** g_ExecutingEventList = (ScriptEventList**)0x00B361B4;
const _MarkBaseExtraListScriptEvent MarkBaseExtraListScriptEvent = (_MarkBaseExtraListScriptEvent)0x004FBF90;
static const UInt32 g_IniSettingCollection = 0x00B07BF0;

typedef FontManager* (* _FontManager_GetSingleton)(void);
const _FontManager_GetSingleton FontManager_GetSingleton = (_FontManager_GetSingleton)0x00576A30;

const NiTMapBase<const char*, NiFormArray*>**  g_IdleAnimationMap = (const NiTMapBase<const char*, NiFormArray*>**)0x00B362C0;
const _GetFormModelPath GetFormModelPath = (_GetFormModelPath)0x0046D3F0;


// make sure this doesn't get optimized
#pragma optimize("", off)

// ### TODO: these functions are really cheesy and not thread-safe, fix it

static UInt32 g_ThisCall_returnAddr = 0;	// this is kludgy but I want to test things with this

__declspec(naked) UInt32 __cdecl ThisCall(UInt32 function, void * _this, ...)
{
	// eax, ecx, edx volatile
	// ebx, ebp, esi, edi must be preserved

	__asm
	{
		//// stack on entry
		// ...
		// args
		// _this
		// function
		// return address <- esp

		pop eax		// eax = return address
		pop edx		// edx = function pointer
		pop ecx		// ecx = _this

		mov g_ThisCall_returnAddr, eax

		//// new stack
		// ...
		// args <- esp

		call edx

		push 0
		push 0
		push g_ThisCall_returnAddr

		retn
	}
}

#pragma optimize("", on)

UInt32 AddFormToDataHandler(DataHandler * dataHandler, TESForm * form)
{
	return ThisStdCall(0x0044D950, dataHandler, form);
}

DataHandler ** g_dataHandler = (DataHandler **)0x00B33A98;

void AddFormToCreatedBaseObjectsList(TESSaveLoadGame * objList, TESForm * form)
{
	ThisStdCall(0x00459800, objList, form);
}

TESSaveLoadGame ** g_createdBaseObjList = (TESSaveLoadGame **)0x00B33B00;

UInt32 NiTPointerMap_Lookup(void * map, const void * key, void ** data)
{
	return ThisStdCall(0x0055E000, map, key, data);
}

void * g_gameSettingsTable = (void *)0x00B35574;
char*** g_baseActorValueNames = (char***)0x00B12758;
char** g_extraActorValueNames = (char**)0x00B12908;
char** g_scriptActorValueNames = (char**)0x00B0A1A8;

#include "obse/GameRTTI_1_2_416.inl"

const _Oblivion_DynamicCast Oblivion_DynamicCast = (_Oblivion_DynamicCast)0x009832E6;

CRITICAL_SECTION * g_extraListMutex = (CRITICAL_SECTION *)0x00B33800;
CRITICAL_SECTION * g_pathingMutex = (CRITICAL_SECTION *)0x00B3BE80;

MemoryPool **	g_memoryHeap_poolsBySize = (MemoryPool **)0x00B33080;		// size = 0x81
MemoryPool **	g_memoryHeap_poolsByAddress = (MemoryPool **)0x00B32C80;	// size = 0x100



const char * GetFullName(TESForm * baseForm)
{
	if(baseForm)
	{
		TESFullName* fullName = baseForm->GetFullName();
		if(fullName && fullName->name.m_data)
		{
			if (fullName->name.m_dataLen)
				return fullName->name.m_data;
		}
	}

	return "<no name>";
}

ScriptEventList* ResolveExternalVar(ScriptEventList* in_EventList, Script* in_Script, UInt8* &scriptData)
{
	ScriptEventList* refEventList = NULL;
	UInt16 varIdx = *((UInt16*)++scriptData);
	scriptData += 2;

	Script::RefVariable* refVar = in_Script->GetVariable(varIdx);
	if (refVar)
	{
		refVar->Resolve(in_EventList);
		TESForm* refObj = refVar->form;
		if (refObj)
		{
			if (refObj->typeID == kFormType_REFR)
			{
				TESObjectREFR* refr = (TESObjectREFR*)Oblivion_DynamicCast(refObj, 0, RTTI_TESForm, RTTI_TESObjectREFR, 0);
				if (refr)
					refEventList = refr->GetEventList();
			}
			else if (refObj->typeID == kFormType_Quest)
			{
				TESQuest* quest = (TESQuest*)Oblivion_DynamicCast(refObj, 0, RTTI_TESForm, RTTI_TESQuest, 0);
				if (quest)
					refEventList = quest->scriptEventList;
			}
		}
	}

	return refEventList;
}

TESGlobal* ResolveGlobalVar(ScriptEventList* in_EventList, Script* in_Script, UInt8* &scriptData)
{
	TESGlobal* global = NULL;
	UInt16 varIdx = *((UInt16*)++scriptData);
	scriptData += 2;

	Script::RefVariable* globalRef = in_Script->GetVariable(varIdx);
	if (globalRef)
		global = (TESGlobal*)Oblivion_DynamicCast(globalRef->form, 0, RTTI_TESForm, RTTI_TESGlobal, 0);

	return global;
}

static bool ExtractFloat(double& out, UInt8* &scriptData, Script* scriptObj, ScriptEventList* eventList)
{
	//extracts one float arg

	bool ret = false;
	if (*scriptData == 'r')		//reference to var in another script
	{
		eventList = ResolveExternalVar(eventList, scriptObj, scriptData);
		if (!eventList)			//couldn't resolve script ref
			return false;
	}

	switch (*scriptData)
	{
	case 'G':		//global var
	{
		TESGlobal* global = ResolveGlobalVar(eventList, scriptObj, scriptData);
		if (global)
		{
			out = global->data;
			ret = true;
		}
		break;
	}
	case 'z':		//literal double
	{
		out = *((double*)++scriptData);
		scriptData += sizeof(double);
		ret = true;
		break;
	}
	case 'f':
	case 's':		//local var
	{
		UInt16 varIdx = *((UInt16*)++scriptData);
		scriptData += 2;
		ScriptEventList::Var* var = eventList->GetVariable(varIdx);
		if (var)
		{
			out = var->data;
			ret = true;
		}
		break;
	}
	}
	return ret;
}

TESForm* ExtractFormFromFloat(UInt8* &scriptData, Script* scriptObj, ScriptEventList* eventList)
{
	TESForm* outForm = NULL;
	if (*scriptData == 'r')		//doesn't work as intended yet so refs must be local vars
	{
		eventList = ResolveExternalVar(eventList, scriptObj, scriptData);
		if (!eventList)
			return NULL;
	}

	UInt16 varIdx = *(UInt16*)++scriptData;
	scriptData += 2;

	ScriptEventList::Var* var = eventList->GetVariable(varIdx);
	if (var)
		outForm = LookupFormByID(*((UInt64 *)&var->data));

	return outForm;
}

TESForm* ResolveForm(UInt8* &scriptData, Script* scriptObj, ScriptEventList* eventList)
{
	TESForm* outForm = NULL;
	char argType = *scriptData;
	UInt16	varIdx = *((UInt16 *)(scriptData+1));
//	scriptData += 2;

	switch (argType)
	{
	case 'r':
		{
			Script::RefVariable	* var = scriptObj->GetVariable(varIdx);
			if(var)
			{
				var->Resolve(eventList);
				outForm = var->form;
				scriptData += 3;
			}
		}
		break;
	case 'f':
			outForm = ExtractFormFromFloat(scriptData, scriptObj, eventList);
			break;
	}
	return outForm;
}

static const char* StringFromStringVar(UInt32 strID)
{
#if OBSE_CORE
	StringVar* strVar = g_StringMap.Get(strID);
	return strVar ? std::get<0>(strVar->GetCString()) : "";
#else
	if (s_StringVarInterface)
		return s_StringVarInterface->GetString(strID);
	else
		return "";
#endif
}

static const char* ResolveStringArgument(ScriptEventList* eventList, const char* stringArg)
{
	const char* result = stringArg;

	if (stringArg && stringArg[0] == '$')
	{
		Script::VariableInfo* varInfo = eventList->m_script->GetVariableByName(stringArg + 1);
		if (varInfo)
		{
			ScriptEventList::Var* var = eventList->GetVariable(varInfo->idx);
			if (var)
				result = StringFromStringVar(var->data);
		}
	}

	return result;
}

static bool v_ExtractArgsEx(SInt16 numArgs, ParamInfo * paramInfo, UInt8* &scriptData, Script * scriptObj, ScriptEventList * eventList, va_list args)
{
#if OBSE_CORE
	if (numArgs < 0) {
		UInt32 opcodeOffsetPtr = 0;
		if (ExtractArgsOverride::ExtractArgs(paramInfo, scriptData, scriptObj, eventList, &opcodeOffsetPtr, args, false, numArgs)) {
			scriptData += opcodeOffsetPtr;
			return true;
		}
		else {
			DEBUG_PRINT("v_ExtractArgsEx returns false");
			return false;
		}
	}
#endif

	for(UInt32 i = 0; i < numArgs; i++)
	{
		ParamInfo	* info = &paramInfo[i];

		//DEBUG_PRINT("ParamType: %d Type: %d Param: %s scriptData: %08x", info->typeID, *scriptData, info->typeStr, scriptData);

		switch(info->typeID)
		{
			case kParamType_String:
			{
				char	* out = va_arg(args, char *);

				UInt16	len = *((UInt16 *)scriptData);
				scriptData += 2;

				memcpy(out, scriptData, len);
				scriptData += len;

				out[len] = 0;

				const char* resolved = ResolveStringArgument(eventList, out);
				if (resolved != out)
				{
					UInt32 resolvedLen = strlen(resolved);
					memcpy(out, resolved, resolvedLen);
					out[resolvedLen] = 0;
				}
			}
			break;

			case kParamType_Integer:
			{
				UInt32	* out = va_arg(args, UInt32 *);
				UInt8   type = *scriptData;
				switch(type)
				{
					case 0x6E: // "n"
						*out = *((UInt32 *)++scriptData);
						scriptData += sizeof(UInt32);
						break;
					case 0x7A: // "z"
						*out = *((double *)++scriptData);
						scriptData += sizeof(double);
						break;
					case 0x66: // "f"
					case 0x72: // "r"
					case 0x73: // "s"
					case 0x47: // "G"
					{
						double data = 0;
						if (ExtractFloat(data, scriptData, scriptObj, eventList))
							*out = data;
						else
							return false;

						break;
					}

					default:
						return false;
				}
			}
			break;

			case kParamType_Float:
			{
				float	* out = va_arg(args, float *);
				UInt8	type = *scriptData;
				switch(type)
				{
					case 0x7A: // "z"
						*out = *((double *)++scriptData);
						scriptData += sizeof(double);
						break;

					case 0x72: // "r"
					case 0x66: // "f"
					case 0x73: // "s"
					case 0x47: // "G"
					{
						double data = 0;
						if (ExtractFloat(data, scriptData, scriptObj, eventList))
							*out = data;
						else
							return false;

						break;
					}

					default:
						return false;
				}
			}
			break;

			case kParamType_InventoryObject:
			case kParamType_ObjectRef:
			case kParamType_Actor:
			case kParamType_SpellItem:
			case kParamType_Cell:
			case kParamType_MagicItem:
			case kParamType_Sound:
			case kParamType_Topic:
			case kParamType_Quest:
			case kParamType_Race:
			case kParamType_Class:
			case kParamType_Faction:
			case kParamType_Global:
			case kParamType_Furniture:
			case kParamType_TESObject:
			case kParamType_MapMarker:
			case kParamType_ActorBase:
			case kParamType_Container:
			case kParamType_WorldSpace:
			case kParamType_AIPackage:
			case kParamType_CombatStyle:
			case kParamType_MagicEffect:
			case kParamType_Birthsign:
			case kParamType_WeatherID:
			case kParamType_NPC:
			case kParamType_Owner:
			case kParamType_EffectShader:
			case kParamType_Climate:
			{
				TESForm	** out = va_arg(args, TESForm **);
				TESForm* form = ResolveForm(scriptData, scriptObj, eventList);
				if (!form)
					return false;

				*out = form;
			}
			break;

			case kParamType_ActorValue:
			case kParamType_AnimationGroup:
			case kParamType_Sex:
			case kParamType_CrimeType:
			{
				UInt32	* out = va_arg(args, UInt32 *);

				*out = *((UInt16 *)scriptData);
				scriptData += 2;
			}
			break;

			default:
				_MESSAGE("Unhandled type encountered. Arg #%d numArgs = %d paramType = %d paramStr = %s",
					i, numArgs, info->typeID, info->typeStr);
				HALT("unhandled type");
				break;
		}
	}

	return true;
}

// g_baseActorValueNames is only filled in after oblivion's global initializers run
const char* GetActorValueString(UInt32 actorValue)
{
	const char* name = 0;
	if (actorValue <= kActorVal_Infamy)
		name = *g_baseActorValueNames[actorValue];
	else if (actorValue < kActorVal_OblivionMax)
		name = g_extraActorValueNames[actorValue - kActorVal_MagickaMultiplier];

	if (!name)
		name = "unknown";

	return name;
}

UInt32 GetActorValueForScript(const char* avStr)
{
	for (UInt32 i = 0; i < kActorVal_OblivionMax; i++) {
		if (!_stricmp(avStr, g_scriptActorValueNames[i])) {
			return i;
		}
	}

	return kActorVal_NoActorValue;
}

UInt32 GetActorValueForString(const char* strActorVal, bool bForScript)
{
	if (bForScript)
		return GetActorValueForScript(strActorVal);

	for (UInt32 n = 0; n < kActorVal_MagickaMultiplier; n++) {
		char* name = *g_baseActorValueNames[n];
		if (_stricmp(strActorVal, name) == 0)
			return n;
	}

	UInt32 nExtraActorVals = kActorVal_OblivionMax - kActorVal_MagickaMultiplier;
	for (UInt32 n = 0; n < nExtraActorVals; n++) {
		char *name = g_extraActorValueNames[n];
		if (_stricmp(strActorVal, name) == 0)
			return n + kActorVal_MagickaMultiplier;
	}
	return kActorVal_NoActorValue;
}

bool IsValidActorValue(UInt32 actorValue)
{
	return actorValue < kActorVal_OblivionMax;
}

UInt32 SafeModUInt32(UInt32 originalVal, float modBy) {
	double val = originalVal;
	val += modBy;
	if (val > ULONG_MAX)	{return ULONG_MAX;}
	else if (val < 0)		{return 0;}
	else					{return (UInt32) val;}
}

float SafeChangeFloat(float originalVal, float changeVal, bool bMod, bool bNegativeAllowed) {
	double val = (bMod) ? originalVal : changeVal;
	if (bMod) { val+= changeVal; }
	if (val > FLT_MAX)			{return FLT_MAX;}
	else if (val >= 0)			{return (float) val;}
	else if (!bNegativeAllowed) { return 0;}
	else if (val > FLT_MIN)		{return (float) val;}
	else						{ return FLT_MIN; }
}

SettingInfo::EType SettingInfo::Type() const
{
	if (!name) return kSetting_Other;
	switch(name[0]) {
		case 'b': return kSetting_Bool;
		case 'c': return kSetting_c;
		case 'i': return kSetting_Integer;
		case 'u': return kSetting_Unsigned;
		case 'f': return kSetting_Float;
		case 's':
		case 'S':
			return kSetting_String;
		case 'r': return kSetting_r;
		case 'a': return kSetting_a;
		default:
			return kSetting_Other;
	}
}

bool GetGameSetting(const char* settingName, SettingInfo** setting)
{
	return (NiTPointerMap_Lookup(g_gameSettingsTable, settingName, (void **)setting) != 0);
}

IniSettingCollection* IniSettingCollection::GetSingleton ()
{
	return (IniSettingCollection*)g_IniSettingCollection;
}

InterfaceManager * InterfaceManager::GetSingleton(void)
{
	return (InterfaceManager *)InterfaceManager_GetSingleton(false, true);
}

static void ConvertLiteralPercents(std::string* str)
{
	UInt32 idx = 0;
	while ((idx = str->find('%', idx)) != -1)
	{
		str->insert(idx, "%");
		idx += 2;
	}
}

static void SkipArgs(UInt8* &scriptData)
{
	switch (*scriptData)
	{
	case 'z':
		scriptData += sizeof(double) + 1;
		break;
	case 'r':
		scriptData += 6;
		break;
	default:
		scriptData += 3;
		break;
	}
}

static void OmitFormatStringArgs(std::string str, FormatStringArgs& args)
{
	//skip any args omitted by the %{ specifier
	UInt32 strIdx = 0;
	while ((strIdx = str.find('%', strIdx)) != -1 && args.HasMoreArgs())
	{
		switch(str[++strIdx])
		{
		case '%':
		case 'q':
		case 'Q':
		case 'r':
		case 'R':
			break;
		case 'c':
		case 'C':
			args.SkipArgs(2);
			break;
		default:
			args.SkipArgs(1);
		}
		strIdx++;
	}
}

void RegisterStringVarInterface(OBSEStringVarInterface* intfc)
{
	s_StringVarInterface = intfc;
}

//static bool ExtractFormattedString(UInt32 &numArgs, char* buffer, UInt8* &scriptData, Script* scriptObj, ScriptEventList* eventList)
bool ExtractFormattedString(FormatStringArgs& args, char* buffer)
{
	//extracts args based on format string, prints formatted string to buffer
	static const int maxArgs = 20;
	double f[maxArgs] = {0.0};
	UInt32 argIdx = 0;

	std::string fmtString = args.GetFormatString();
	UInt32 strIdx = 0;

	//extract args
	while ((strIdx = fmtString.find('%', strIdx)) != -1)
	{
		char argType = fmtString.at(strIdx+1);
		switch (argType)
		{
		case '%':										//literal %
			strIdx += 2;
			break;
		case 'z':
		case 'Z':										//string variable
			{
				fmtString.erase(strIdx, 2);
				double strID = 0;
				if (!args.Arg(args.kArgType_Float, &strID))
					return false;

				const char* toInsert = StringFromStringVar(strID);

				if (toInsert && toInsert[0])
					fmtString.insert(strIdx, toInsert);
			}
			break;
		case 'r':										//newline
		case 'R':
			fmtString.erase(strIdx, 2);
			fmtString.insert(strIdx, "\n");
			break;
		case 'e':
		case 'E':										//workaround for CS not accepting empty strings
			fmtString.erase(strIdx, 2);
			break;
		case 'B':										// toggle blue text on, console only
			fmtString.erase(strIdx, 2);
			fmtString.insert(strIdx, "\2");
			break;
		case 'b':										// toggle blue text off, console only
			fmtString.erase(strIdx, 2);
			fmtString.insert(strIdx, "\3");
			break;
		case 'a':
		case 'A':										//character specified by ASCII code
			{
				fmtString.erase(strIdx, 2);
				double fCharCode = 0;
				if (args.Arg(args.kArgType_Float, &fCharCode))
					fmtString.insert(strIdx, 1, (char)fCharCode);
				else
					return false;
			}
			break;
		case 'n':										// name of obj/ref
		case 'N':
			{
				fmtString.erase(strIdx, 2);
				TESForm* form = NULL;
				if (!args.Arg(args.kArgType_Form, &form))
					return false;

				std::string strName(GetFullName(form));
				ConvertLiteralPercents(&strName);
				fmtString.insert(strIdx, strName);
				strIdx += strName.length();
			}
			break;
		case 'i':											//formID
		case 'I':
			{
				fmtString.erase(strIdx, 2);
				TESForm* form = NULL;
				if (!(args.Arg(args.kArgType_Form, &form)))
					return false;
				else if (!form)
					fmtString.insert(strIdx, "00000000");
				else
				{
					char formID[9];
					sprintf_s(formID, 9, "%08X", form->refID);
					fmtString.insert(strIdx, formID);
				}
			}
			break;
		case 'c':											//named component of another object
		case 'C':											//2 args - object and index
			{
				TESForm* form = NULL;
				if (!args.Arg(args.kArgType_Form, &form))
					return false;

				fmtString.erase(strIdx, 2);
				if (!form)
					fmtString.insert(strIdx, "NULL");
				else
				{
					double objIdx = 0;
					if (!args.Arg(args.kArgType_Float, &objIdx))
						return false;
					else
					{
						std::string strName("unknown");
						switch (form->typeID)
						{
						case kFormType_Spell:
						case kFormType_Enchantment:
						case kFormType_Ingredient:
						case kFormType_AlchemyItem:
							{
								MagicItem* magItm = (MagicItem*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_MagicItem, 0);
								if (!magItm)
									strName = "NULL";
								else
								{
									EffectItem* effItem = magItm->list.ItemAt(objIdx);
									if (effItem)
									{
										char effName[0x200] = { 0 };
										effItem->GetQualifiedName(effName);
										strName = effName;
									}
								}
								break;
							}
						case kFormType_SigilStone:
							{
								TESSigilStone* stone = (TESSigilStone*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESSigilStone, 0);
								if (!stone)
									strName = "NULL";
								else
								{
									strName = stone->effectList.GetNthEIName(objIdx);
									EffectItem* effItem = stone->effectList.ItemAt(objIdx);
									if (effItem)
									{
										char effName[0x200] = { 0 };
										effItem->GetQualifiedName(effName);
										strName = effName;
									}
								}
								break;
							}
						case kFormType_Faction:
							{
								TESFaction* fact = (TESFaction*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFaction, 0);
								if (!fact)
									strName = "NULL";
								else
								{
									strName = fact->GetNthRankName(objIdx);
								}
								break;
							}
						}
						ConvertLiteralPercents(&strName);
						fmtString.insert(strIdx, strName);
						strIdx += strName.length();
					}
				}
				}
				break;
		case 'k':
		case 'K':											//DX code
			{
				double keycode = 0;
				fmtString.erase(strIdx, 2);
				if (!args.Arg(args.kArgType_Float, &keycode))
					return false;

				const char* desc = GetDXDescription(keycode);
				fmtString.insert(strIdx, desc);
			}
			break;
		case 'v':
		case 'V':											//actor value
			{
				double actorVal = kActorVal_OblivionMax;
				fmtString.erase(strIdx, 2);
				if (!args.Arg(args.kArgType_Float, &actorVal))
					return false;

				std::string valStr(GetActorValueString(actorVal));
				if (valStr.length())
				{
					for (UInt32 idx = 1; idx < valStr.length(); idx++)
						if (isupper(valStr[idx]))
						{								//insert spaces to make names more presentable
							valStr.insert(idx, " ");
							idx += 2;
						}
				}
				fmtString.insert(strIdx, valStr);
			}
			break;
		case 'p':
		case 'P':											//pronouns
			{
				fmtString.erase(strIdx, 2);
				char pronounType = fmtString[strIdx];
				fmtString.erase(strIdx, 1);
				TESForm* form = NULL;
				if (!args.Arg(args.kArgType_Form, &form))
					return false;

				if (!form)
					fmtString.insert(strIdx, "NULL");
				else
				{
					TESObjectREFR* refr = (TESObjectREFR*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectREFR, 0);
					if (refr)
						form = refr->baseForm;

					short objType = 0;
					if (form->typeID == kFormType_NPC)
					{
						TESActorBaseData* actorBase = (TESActorBaseData*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESActorBaseData, 0);
						objType = (actorBase->IsFemale()) ? 2 : 1;
					}

					switch (pronounType)
					{
					case 'o':
					case 'O':
						if (objType == 1)
							fmtString.insert(strIdx, "him");
						else if (objType == 2)
							fmtString.insert(strIdx, "her");
						else
							fmtString.insert(strIdx, "it");
						break;
					case 's':
					case 'S':
						if (objType == 1)
							fmtString.insert(strIdx, "he");
						else if (objType == 2)
							fmtString.insert(strIdx, "she");
						else
							fmtString.insert(strIdx, "it");
						break;
					case 'p':
					case 'P':
						if (objType == 1)
							fmtString.insert(strIdx, "his");
						else if (objType == 2)
							fmtString.insert(strIdx, "her");
						else
							fmtString.insert(strIdx, "its");
						break;
					default:
						fmtString.insert(strIdx, "NULL");
					}
				}
			}
			break;
		case 'q':
		case 'Q':											//double quote
			fmtString.erase(strIdx, 2);
			fmtString.insert(strIdx, "\"");
			break;
		case '{':											//omit portion of string based on flag param
			{
				fmtString.erase(strIdx, 2);
				double flag = 0;
				if (!args.Arg(args.kArgType_Float, &flag))
					return false;

				UInt32 omitEnd = fmtString.find("%}", strIdx);
				if (omitEnd == -1)
					omitEnd = fmtString.length();

				if (!flag)
				{
					OmitFormatStringArgs(fmtString.substr(strIdx, omitEnd - strIdx), args);
					fmtString.erase(strIdx, omitEnd - strIdx + 2);
				}
				else
					fmtString.erase(omitEnd, 2);
			}
			break;
		case '}':											//in case someone left a stray closing bracket
			fmtString.erase(strIdx, 2);
			break;
		case 'x':											//hex
		case 'X':
			{
				double data = 0;
				if (!args.Arg(args.kArgType_Float, &data))
					return false;

				UInt64* hexArg = (UInt64*)(&f[argIdx++]);
				*hexArg = data;
				fmtString.erase(strIdx, 2);
				char width = 0;
				if (strIdx < fmtString.length())
				{
					if (isdigit(fmtString[strIdx]))	//single-digit width specifier optionally follows %x
					{
						width = fmtString[strIdx];
						fmtString.erase(strIdx, 1);
					}
				}
				fmtString.insert(strIdx, "%0llX");
				if (width)
					fmtString.insert(strIdx + 2, 1, width);
				strIdx++;
			}
			break;
		default:											//float
			{
				double data = 0;
				if (!args.Arg(args.kArgType_Float, &data))
					return false;

				f[argIdx++] = data;
				strIdx++;
			}
		}
	}
#pragma warning(push)	// disable warning about sprintf b/c we don't always know buf size, and debug builds will attempt to
#pragma warning(disable: 4996)	// fill unused portion of buffer with 0xFD
	if (fmtString.length() >= kMaxMessageLength - 2 || fmtString.length() == 0) {
		buffer[0] = '\0';
		return true;
	}
	else if (sprintf(buffer, fmtString.c_str(), f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8], f[9], f[10], f[11], f[12], f[13], f[14], f[15], f[16], f[17], f[18], f[19]) > 0)
	{
		return true;
	}
	else
		return false;
#pragma warning(pop)
}

#if OBSE_CORE
bool ExtractArgsEx(ParamInfo * paramInfo, void * scriptDataIn, UInt32 * scriptDataOffset, Script * scriptObj, ScriptEventList * eventList, ...)
{
	va_list	args;
	va_start(args, eventList);

	UInt8	* scriptData = ((UInt8 *)scriptDataIn) + *scriptDataOffset;
	UInt32	numArgs = *((UInt16 *)scriptData);
	scriptData += 2;

	bool bExtracted = v_ExtractArgsEx(numArgs, paramInfo, scriptData, scriptObj, eventList, args);
	va_end(args);
	return bExtracted;
}

//fmtStringPos is index of fmtString param in paramInfo, with first param = 0
bool ExtractFormatStringArgs(UInt32 fmtStringPos, char* buffer, ParamInfo * paramInfo, void * scriptDataIn, UInt32 * scriptDataOffset, Script * scriptObj, ScriptEventList * eventList, UInt32 maxParams, ...)
{
	va_list args;
	va_start(args, maxParams);

	UInt8	* scriptData = ((UInt8 *)scriptDataIn) + *scriptDataOffset;
	SInt16	numArgs = *((SInt16 *)scriptData);
	scriptData += 2;

	if (numArgs < 0) {
		UInt32 offsetPtr = 0;
		bool bResult = ExtractArgsOverride::ExtractFormattedString(paramInfo, scriptData, scriptObj, eventList, &offsetPtr, args,
			fmtStringPos, buffer, maxParams);
		if (!bResult) {
			DEBUG_PRINT("ExtractFormatStringArgs returns false");
		}
		return bResult;
	}

	numArgs -= fmtStringPos + 1;

	bool bExtracted = false;
	if (fmtStringPos > 0)
	{
		bExtracted = v_ExtractArgsEx(fmtStringPos, paramInfo, scriptData, scriptObj, eventList, args);
		if (!bExtracted)
			return false;
	}

	ScriptFormatStringArgs scriptArgs(numArgs, scriptData, scriptObj, eventList);
	bExtracted = ExtractFormattedString(scriptArgs, buffer);

	numArgs = scriptArgs.GetNumArgs();
	scriptData = scriptArgs.GetScriptData();
	//NOTE: if v_ExtractArgsEx was called above, passing args again in second call below = undefined behavior. Needs fixing.
	if (bExtracted && numArgs > 0)			//some optional normal params following format string params
	{
		if ((numArgs + fmtStringPos + 21) > maxParams)		//scripter included too many optional params - adjust to prevent crash
			numArgs = (maxParams - fmtStringPos - 21);

		bExtracted = v_ExtractArgsEx(numArgs, &(paramInfo[fmtStringPos + 21]), scriptData, scriptObj, eventList, args);
	}

	va_end(args);
	return bExtracted;
}
#endif

static bool s_bConsoleOutputEnabled = true;

bool IsConsoleMode()
{
	return *g_bConsoleMode;
}

bool IsConsoleOpen()
{
	return *g_bIsConsoleOpen;
}

bool ExtractSetStatementVar(Script* script, ScriptEventList* eventList, void* scriptDataIn, double * outVarData, UInt8* outModIndex)
{
	//when script command called as righthand side of a set statement, the script data containing the variable
	//to assign to remains on the stack as arg to a previous function. We can get to it through arg1 in COMMAND_ARGS

	UInt8* dataStart = (UInt8*)scriptDataIn;	// should be 0x58 (or 0x72 if called with dot syntax)

	if (!((*dataStart == 0x58 || *dataStart == 0x72))) {
		return false;
	}

	UInt32 scriptDataPtrAddr = (UInt32)(scriptDataIn) - 0x3C;
	UInt32* scriptDataAddr = (UInt32*)scriptDataPtrAddr;
	UInt8* scriptData = (UInt8*)(*scriptDataAddr);

	SInt32 scriptDataOffset = (UInt32)scriptData - (UInt32)(script->data);
	if (scriptDataOffset < 5)
		return false;

	bool bExtracted = false;
	scriptData -= 5;

	switch (*scriptData)			//work backwards from opcode to find lefthand var
	{
	case 'G':						//global
		{
			UInt16 refIdx = *(UInt16*)(scriptData + 1);
			Script::RefVariable* refVar = script->GetVariable(refIdx);
			if (!refVar)
				break;

			TESGlobal* globalVar = (TESGlobal*)Oblivion_DynamicCast(refVar->form, 0, RTTI_TESForm, RTTI_TESGlobal, 0);
			if (globalVar)
			{
				*outVarData = globalVar->data;
				if (outModIndex)
					*outModIndex = (globalVar->refID >> 24);
				bExtracted = true;
			}
		}
		break;
	case 'l':
	case 'f':
	case 's':
		{
			if (scriptDataOffset >= 8 && *(scriptData - 3) == 'r')	//external var
			{
				UInt16 refIdx = *(UInt16*)(scriptData - 2);
				Script::RefVariable* refVar = script->GetVariable(refIdx);
				if (!refVar)
					break;

				refVar->Resolve(eventList);
				TESForm* refForm = refVar->form;
				if (!refForm)
					break;

				if (refForm->typeID == kFormType_REFR)
				{
					TESObjectREFR* refr = (TESObjectREFR*)Oblivion_DynamicCast(refForm, 0, RTTI_TESForm, RTTI_TESObjectREFR, 0);
					TESScriptableForm* scriptable = (TESScriptableForm*)Oblivion_DynamicCast(refr->baseForm, 0, RTTI_TESForm, RTTI_TESScriptableForm, 0);
					if (scriptable)
					{
						script = scriptable->script;
						eventList = refr->GetEventList();
					}
					else
						break;
				}
				else if (refForm->typeID == kFormType_Quest)
				{
					TESScriptableForm* scriptable = (TESScriptableForm*)Oblivion_DynamicCast(refForm, 0, RTTI_TESForm, RTTI_TESScriptableForm, 0);
					if (scriptable)
					{
						script = scriptable->script;
						TESQuest* quest = (TESQuest*)Oblivion_DynamicCast(scriptable, 0, RTTI_TESScriptableForm, RTTI_TESQuest, 0);
						eventList = quest->scriptEventList;
					}
					else
						break;
				}
				else
					break;
			}

			UInt16 varIdx = *(UInt16*)(scriptData + 1);
			ScriptEventList::Var* var = eventList->GetVariable(varIdx);
			if (var)
			{
				*outVarData = var->data;
				if (outModIndex)
					*outModIndex = (script->refID >> 24);
				bExtracted = true;
			}
		}
		break;
	default:
#ifdef OBSE_CORE
		ShowRuntimeError(script, "Function must be used within a Set statement");
#endif
		break;
	}

	return bExtracted;
}

//Generate a game messagebox
void MessageBoxAlert(const char * msg)
{
	*ShowMessageBox_button = 0xFF;
	*ShowMessageBox_pScriptRefID = 0;
	ShowMessageBox(msg, ShowMessageBox_Callback, 0, "Ok", 0);
}

//Log error if expression evaluates to false
bool SCRIPT_ASSERT(bool expr, Script* script, const char * errorMsg, ...)
{
	static bool bAlerted = false;			//only alert user on first error
	static std::set<UInt32> naughtyScripts;	//one error per script to avoid thrashing

	if (!expr && naughtyScripts.find(script->refID) == naughtyScripts.end())
	{
		const ModEntry ** activeMods = (*g_dataHandler)->GetActiveModList();
		UInt8 modIndex = script->GetModIndex();
		const ModEntry * modEntry = activeMods[modIndex];

		const char * modName;
		if (modIndex != 0xFF && modEntry && modEntry->data && modEntry->data->name)
			modName = modEntry->data->name;
		else
			modName = "Unknown";

//		sprintf_s(errorHeader, sizeof(errorHeader) - 1, "** Error: Script %08X in file \"%s\" **", script->refID, modName);
//		_MESSAGE("%s", errorHeader);
		_MESSAGE("** Script Error: Script %08x in file \"%s\" **", script->refID, modName);

		va_list args;
		va_start(args, errorMsg);

		char errorBuf[512];
		vsprintf_s(errorBuf, sizeof(errorBuf) - 1, errorMsg, args);
		va_end(args);

		gLog.Indent();
		_MESSAGE("%s", errorBuf);
		gLog.Outdent();

		if (!bAlerted)
		{
			MessageBoxAlert("OBSE has detected a script error. \n\nPlease check obse.log for details.");
			bAlerted = true;
		}

		naughtyScripts.insert(script->refID);
	}

	return expr;
}

ScriptFormatStringArgs::ScriptFormatStringArgs(UInt32 _numArgs, UInt8* _scriptData, Script* _scriptObj, ScriptEventList* _eventList)
	: numArgs(_numArgs), scriptData(_scriptData), scriptObj(_scriptObj), eventList(_eventList)
{
	//extract format string
	UInt16 len = *((UInt16*)scriptData);
	char* szFmt = new char[len+1];
	scriptData += 2;
	memcpy(szFmt, scriptData, len);
	szFmt[len] = '\0';

	scriptData += len;
	fmtString = std::string(ResolveStringArgument(eventList, szFmt));
	delete szFmt;
}

std::string ScriptFormatStringArgs::GetFormatString()
{
	return fmtString;
}

bool ScriptFormatStringArgs::HasMoreArgs()
{
	return (numArgs > 0);
}

UInt32 ScriptFormatStringArgs::GetNumArgs()
{
	return numArgs;
}

UInt8* ScriptFormatStringArgs::GetScriptData()
{
	return scriptData;
}

bool ScriptFormatStringArgs::SkipArgs(UInt32 numToSkip)
{
	while (numToSkip--)
	{
		switch (*scriptData)
		{
		case 'z':
			scriptData += sizeof(double) + 1;
			break;
		case 'r':
			scriptData += 6;
			break;
		default:
			scriptData += 3;
			break;
		}

		numArgs--;
	}

	return true;
}

bool ScriptFormatStringArgs::Arg(FormatStringArgs::argType asType, void * outResult)
{
	if (numArgs == 0)
	{
#ifdef OBSE_CORE
		ShowRuntimeError(scriptObj, "Too few args for format specifier");
#endif
		return false;
	}

	numArgs--;

	switch (asType)
	{
	case kArgType_Float:
		{
			double data = 0;
			if (ExtractFloat(data, scriptData, scriptObj, eventList))
			{
				*((double*)outResult) = data;
				return true;
			}
		}
		break;
	case kArgType_Form:
		{
			TESForm* form = ExtractFormFromFloat(scriptData, scriptObj, eventList);
			*((TESForm**)outResult) = form;
			return true;
		}
	}

	return false;
}

bool InterfaceManager::CreateTextEditMenu(const char* promptText, const char* defaultText)
{
	return ThisStdCall(0x005DCF20, this, promptText, defaultText) ? true : false;

}

float InterfaceManager::GetDepth()
{
	float depth = 0;

	ThisStdCall(0x00588890, this);

	__asm {
		fstp	[depth]
	}

	return depth;
}

bool InterfaceManager::MenuModeHasFocus(UInt32 menuType)
{
	UInt8 bMenuMode = (UInt8)ThisStdCall(0x0057C140, this, menuType);

	return bMenuMode ? true : false;
}

UInt32 InterfaceManager::GetTopVisibleMenuID()
{
	return ThisStdCall(0x0057CF60, this);
}

bool InterfaceManager::IsGameMode()
{
	return !IsMenuMode();
}

void InterfaceManager::ToggleDebugText()
{
	typedef void (* _fn)();
	((_fn)0x0057C200)();
}

Script* GetCurrentExecutingScript(void)
{
	return *g_ExecutingScript;
}

ScriptEventList* GetCurrentExecutingScriptEventList(void)
{
	return *g_ExecutingEventList;
}

void ShowCompilerError(ScriptLineBuffer* lineBuf, const char * fmt, ...)
{
	char errorHeader[0x400];
	UInt32 offset = sprintf_s(errorHeader, 0x400, "Error on line %d\n\n", lineBuf->lineNumber);

	va_list	args;
	va_start(args, fmt);

	char	errorMsg[0x200];
	vsprintf_s(errorMsg, 0x200, fmt, args);

	strcat_s(errorHeader, 0x400, errorMsg);
	Console_Print(errorHeader);
	_MESSAGE(errorHeader);

	va_end(args);
}

void SettingInfo::Set(const char* str)
{
	if (Type() == kSetting_String)
	{
		ThisStdCall(0x004A7980, this, str);

	}
}

FontManager* FontManager::GetSingleton()
{
	return FontManager_GetSingleton();
}

FontManager::FontInfo* FontManager::FontInfo::Load(const char* path, UInt32 ID)
{
	FontInfo* info = (FontInfo*)FormHeap_Allocate(sizeof(FontInfo));
	return (FontManager::FontInfo*)ThisStdCall(0x005757A0, info, ID, path, 1);
}

bool FontManager::FontInfo::GetName(char* out)
{
	UInt32 len = strlen(path);
	len -= 4;					// '.fnt'
	UInt32 start = len;
	while (path[start-1] != '\\') {
		start--;
	}

	len -= start;

	memcpy(out, path+start, len);
	out[len] = 0;

	return true;
}

