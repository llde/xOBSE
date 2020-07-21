#include "ParamInfos.h"
#include "obse/Commands_String.h"
#include "CommandTable.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION
#include "GameAPI.h"
#include "StringVar.h"
#include "GameData.h"
#include "GameObjects.h"
#include "Utilities.h"

//////////////////////////
// Utility commands
//////////////////////////

// Commands that assign to a string_var should ALWAYS assign a value and return a string ID
// unless the lefthand variable cannot be extracted

static bool Cmd_sv_Construct_Execute(COMMAND_ARGS)
{
	char buffer[kMaxMessageLength] = {0};

	//not checking the return value here 'cuz we need to assign to the string regardless
	ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_sv_Construct.numParams);

	AssignToStringVar(PASS_COMMAND_ARGS, buffer);

	return true;
}

static bool Cmd_sv_Set_Execute(COMMAND_ARGS)
{
	char buffer[kMaxMessageLength] = { 0 };
	UInt32 stringID = 0;
	if (ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_sv_Set.numParams, &stringID))
	{
		StringVar* var = g_StringMap.Get(stringID);
		if (var)
		{
			var->Set(buffer);
		}
	}

	return true;
}

static bool Cmd_sv_Destruct_Execute(COMMAND_ARGS)
{
    // as of v0017 beta 2, sv_Destruct has 2 different usages:
    //        'set var to sv_Destruct' -> destroys string contained in 'var'
    //        'sv_Destruct var1 [var2 ... var10]' -> destroys var1, var2, ... var10
    UInt16 numArgs = 0;
    UInt8* dataStart = (UInt8*)arg1;
    if (*dataStart == 0x58 || *dataStart == 0x72) // !!! Only if inside a set statement !!!
    {
        *result = 0;            //store zero in destructed string_var
        double strID = 0;
        UInt8 modIndex = 0;

        if (ExtractSetStatementVar(scriptObj, eventList, arg1, &strID, &modIndex))
            g_StringMap.Delete(strID);

        return true;
    }
	// alternate syntax: 'sv_Destruct var1 var2 .. var10
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (!eval.ExtractArgs())
		return true;

	for (UInt32 i = 0; i < eval.NumArgs(); i++)
	{
		if (eval.Arg(i)->CanConvertTo(kTokenType_StringVar))
		{
			ScriptEventList::Var* var = eval.Arg(i)->GetVar();
			if (var)
			{
				g_StringMap.Delete(var->data);
				var->data = 0;
			}
		}
	}

	return true;
}

static bool Cmd_sv_SubString_Execute(COMMAND_ARGS)
{
	UInt32 rhStrID = 0;
	UInt32 startPos = 0;
	UInt32 howMany = -1;
	std::string subStr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &rhStrID, &startPos, &howMany))
	{
		StringVar* rhVar = g_StringMap.Get(rhStrID);
		if (!rhVar)
			return true;

		if (howMany == -1)
			howMany = rhVar->GetLength() - startPos;

		subStr = rhVar->SubString(startPos, howMany);
	}

	AssignToStringVar(PASS_COMMAND_ARGS, subStr.c_str());
	return true;
}

static bool Cmd_sv_Compare_Execute(COMMAND_ARGS)
{
	*result = -2;			//sentinel value if comparison fails
	UInt32 stringID = 0;
	char buffer[kMaxMessageLength] = { 0 };
	UInt32 bCaseSensitive = 0;

	if (!ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_sv_Compare.numParams, &stringID, &bCaseSensitive))
		return true;

	StringVar* lhs = g_StringMap.Get(stringID);
	if (!lhs)
		return true;

	*result = lhs->Compare(buffer, bCaseSensitive ? true : false);

	return true;
}

static bool Cmd_sv_Length_Execute(COMMAND_ARGS)
{
	*result = -1;			// sentinel value if extraction fails
	UInt32 strID = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &strID))
	{
		StringVar* str = g_StringMap.Get(strID);
		if (str)
			*result = str->GetLength();
	}

	return true;
}

static bool Cmd_sv_Erase_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 strID = 0;
	UInt32 startPos = 0;
	UInt32 howMany = -1;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &strID, &startPos, &howMany))
		return true;

	StringVar* strVar = g_StringMap.Get(strID);
	if (strVar)
	{
		if (howMany == -1)
			howMany = strVar->GetLength() - startPos;

		strVar->Erase(startPos, howMany);
	}

	return true;
}

enum {
	eMode_svFind,
	eMode_svCount,
	eMode_svReplace,
};

static bool StringVar_Find_Execute(COMMAND_ARGS, UInt32 mode, CommandInfo* commandInfo)
{
	*result = -1;
	UInt32 strID = 0;
	UInt32 startPos = 0;
	UInt32 numChars = -1;
	UInt32 bCaseSensitive = 0;
	UInt32 numToReplace = -1;			//replace all by default
	char toFind[kMaxMessageLength] = { 0 };

	UInt32 intResult = -1;

	if (!ExtractFormatStringArgs(0, toFind, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, commandInfo->numParams, &strID, &startPos, &numChars, &bCaseSensitive, &numToReplace))
		return true;

	StringVar* strVar = g_StringMap.Get(strID);
	if (strVar)
	{
		if (numChars == -1)
			numChars = strVar->GetLength() - startPos;

		switch (mode)
		{
		case eMode_svFind:
			intResult = strVar->Find(toFind, startPos, numChars, bCaseSensitive ? true : false);
			break;
		case eMode_svCount:
			intResult = strVar->Count(toFind, startPos, numChars, bCaseSensitive ? true : false);
			break;
		case eMode_svReplace:
			{
				std::string str(toFind);
				UInt32 splitPoint = str.find(GetSeparatorChar(scriptObj));
				if (splitPoint != -1 && splitPoint < str.length())
				{
					toFind[splitPoint] = '\0';
					char* replaceWith = (splitPoint == str.length() - 1) ? "" : toFind + splitPoint + 1;
					intResult = strVar->Replace(toFind, replaceWith, startPos, numChars, bCaseSensitive ? true : false, numToReplace);
				}
				break;
			}
		}
	}

	if (intResult != -1)
		*result = intResult;

	return true;
}

static bool Cmd_sv_Find_Execute(COMMAND_ARGS)
{
	StringVar_Find_Execute(PASS_COMMAND_ARGS, eMode_svFind, &kCommandInfo_sv_Find);
	return true;
}

static bool Cmd_sv_Count_Execute(COMMAND_ARGS)
{
	StringVar_Find_Execute(PASS_COMMAND_ARGS, eMode_svCount, &kCommandInfo_sv_Count);
	return true;
}

static bool Cmd_sv_Replace_Execute(COMMAND_ARGS)
{
	StringVar_Find_Execute(PASS_COMMAND_ARGS, eMode_svReplace, &kCommandInfo_sv_Replace);
	return true;
}

static bool Cmd_sv_ToNumeric_Execute(COMMAND_ARGS)
{
	UInt32 strID = 0;
	UInt32 startPos = 0;
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &strID, &startPos))
		return true;

	StringVar* strVar = g_StringMap.Get(strID);
	if (strVar)
	{
		const char* cStr = strVar->GetCString();
		*result = strtod(cStr + startPos, NULL);
	}

	return true;
}

static bool Cmd_sv_Insert_Execute(COMMAND_ARGS)
{
	UInt32 strID = 0;
	UInt32 insertionPos = 0;
	char subString[kMaxMessageLength] = { 0 };
	*result = 0;

	if (!ExtractFormatStringArgs(0, subString, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_sv_Insert.numParams, &strID, &insertionPos))
		return true;

	StringVar* lhs = g_StringMap.Get(strID);
	if (lhs)
		lhs->Insert(subString, insertionPos);

	return true;
}

static bool Cmd_sv_GetChar_Execute(COMMAND_ARGS)
{
	UInt32 strID = 0;
	UInt32 charPos = 0;
	*result = -1;			// error return value

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &strID, &charPos))
		return true;

	StringVar* strVar = g_StringMap.Get(strID);
	if (strVar)
		*result = strVar->At(charPos);

	return true;
}

static bool MatchCharType_Execute(COMMAND_ARGS, UInt32 mask)
{
	UInt32 charCode = 0;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &charCode))
		if ((StringVar::GetCharType(charCode) & mask) == mask)
			*result = 1;

	return true;
}

static bool Cmd_IsLetter_Execute(COMMAND_ARGS)
{
	return MatchCharType_Execute(PASS_COMMAND_ARGS, kCharType_Alphabetic);
}

static bool Cmd_IsDigit_Execute(COMMAND_ARGS)
{
	return MatchCharType_Execute(PASS_COMMAND_ARGS, kCharType_Digit);
}

static bool Cmd_IsPunctuation_Execute(COMMAND_ARGS)
{
	return MatchCharType_Execute(PASS_COMMAND_ARGS, kCharType_Punctuation);
}

static bool Cmd_IsPrintable_Execute(COMMAND_ARGS)
{
	return MatchCharType_Execute(PASS_COMMAND_ARGS, kCharType_Printable);
}

static bool Cmd_IsUpperCase_Execute(COMMAND_ARGS)
{
	return MatchCharType_Execute(PASS_COMMAND_ARGS, kCharType_Uppercase);
}

static bool Cmd_ToUpper_Execute(COMMAND_ARGS)
{
	UInt32 character = 0;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &character))
		*result = toupper(character);

	return true;
}

static bool Cmd_ToLower_Execute(COMMAND_ARGS)
{
	UInt32 character = 0;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &character))
		*result = tolower(character);

	return true;
}

static bool Cmd_CharToAscii_Execute(COMMAND_ARGS)
{
	//converts a single char to ASCII
	*result = -1;
	char buffer[512];		//user shouldn't pass string of more than one char but someone will anyway

	if (ExtractArgs(PASS_EXTRACT_ARGS, &buffer))
		if (strlen(buffer) == 1)
			*result = *buffer;

	return true;
}

/////////////////////////////////////
// Getters/Setters
/////////////////////////////////////

static bool Cmd_GetNthModName_Execute(COMMAND_ARGS)
{
	UInt32 modIdx = 0xFF;
	const char* modName = "";

	if (ExtractArgs(PASS_EXTRACT_ARGS, &modIdx))
		modName = (*g_dataHandler)->GetNthModName(modIdx);

	AssignToStringVar(PASS_COMMAND_ARGS, modName);

	return true;
}

// This works for any TESFullName-derived form as well as references
static bool Cmd_GetName_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	const char* name = "";

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		if (!form)
			if (thisObj)
				form = thisObj;

		if (form)
			name = GetFullName(form);
	}

	AssignToStringVar(PASS_COMMAND_ARGS, name);
	return true;
}

static bool Cmd_GetStringGameSetting_Execute(COMMAND_ARGS)
{
	char settingName[0x100] = { 0 };
	const char* settingString = "";

	if (ExtractArgs(PASS_EXTRACT_ARGS, &settingName))
	{
		SettingInfo* settingInfo = NULL;
		if (GetGameSetting(settingName, &settingInfo) && settingInfo && settingInfo->Type() == SettingInfo::kSetting_String)
			if (settingInfo->Type() == SettingInfo::kSetting_String)
				settingString = settingInfo->s;
	}

	AssignToStringVar(PASS_COMMAND_ARGS, settingString);

	return true;
}

// setting name included in format string i.e. "sSomeSetting|newSettingValue"
static bool Cmd_SetStringGameSettingEX_Execute(COMMAND_ARGS)
{
	char fmtString[kMaxMessageLength] = { 0 };
	*result = 0;

	if (ExtractFormatStringArgs(0, fmtString, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetStringGameSettingEX.numParams))
	{
		UInt32 pipePos = std::string(fmtString).find(GetSeparatorChar(scriptObj));
		if (pipePos != -1)
		{
			fmtString[pipePos] = 0;
			char* newValue = fmtString + pipePos + 1;

			SettingInfo* settingInfo = NULL;
			if (GetGameSetting(fmtString, &settingInfo) && settingInfo && settingInfo->Type() == SettingInfo::kSetting_String)
			{
				settingInfo->Set(newValue);;
				*result = 1;
			}
		}
	}

	return true;
}

static bool Cmd_GetModelPath_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	const char* pathStr = "";
	*result = 0;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;

		TESModel* model = (TESModel*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESModel, 0);
		if (model)
			pathStr = model->nifPath.m_data;
	}

	AssignToStringVar(PASS_COMMAND_ARGS, pathStr);
	return true;
}

static bool Cmd_SetModelPathEX_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	char newPath[kMaxMessageLength] = { 0 };

	if (ExtractFormatStringArgs(0, newPath, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetModelPathEX.numParams, &form))
	{
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;

		TESModel* model = (TESModel*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESModel, 0);
		if (model)
			model->SetPath(newPath);
	}

	return true;
}

enum {
	kPath_Icon,
	kPath_Texture
};

static bool GetPath_Execute(COMMAND_ARGS, UInt32 whichPath)
{
	TESForm* form = NULL;
	const char* pathStr = "";

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
	{
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;
		switch (whichPath) {
			case kPath_Icon:
				{
					TESIcon* icon = (TESIcon*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESIcon, 0);
					if (icon)
						pathStr = icon->ddsPath.m_data;
				}
				break;
			case kPath_Texture:
				{
					TESTexture* tex = (TESTexture*)OBLIVION_CAST(form, TESForm, TESTexture);
					if (tex)
						pathStr = tex->ddsPath.m_data;
				}
				break;
		}
	}

	AssignToStringVar(PASS_COMMAND_ARGS, pathStr);
	return true;
}

static bool Cmd_GetIconPath_Execute(COMMAND_ARGS)
{
	// not all objects with icons can be cast to TESIcon (e.g. skills, classes, TESObjectMISC)
	// so GetTexturePath is preferred over GetIconPath
	return GetPath_Execute(PASS_COMMAND_ARGS, kPath_Icon);
}

static bool Cmd_GetTexturePath_Execute(COMMAND_ARGS)
{
	return GetPath_Execute(PASS_COMMAND_ARGS, kPath_Texture);
}

static bool Cmd_ActorValueToString_Execute(COMMAND_ARGS)
{
	UInt32 av = kActorVal_OblivionMax;
	std::string avStr = "";

	if (ExtractArgs(PASS_EXTRACT_ARGS, &av) && av < kActorVal_OblivionMax)
		avStr = GetActorValueString(av);

	AssignToStringVar(PASS_COMMAND_ARGS, avStr.c_str());
	return true;
}

static bool Cmd_SetIconPathEX_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	char newPath[kMaxMessageLength] = { 0 };

	if (ExtractFormatStringArgs(0, newPath, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetIconPathEX.numParams, &form))
	{
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;

		TESIcon* icon = (TESIcon*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESIcon, 0);
		if (icon)
			icon->SetPath(newPath);
	}

	return true;
}

static bool Cmd_SetTexturePath_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0) {
		TESForm* form = NULL;
		if (eval.NumArgs() == 2) {
			form = eval.Arg(1)->GetTESForm();
		}
		else if (thisObj) {
			form = thisObj->baseForm;
		}

		TESTexture* tex = OBLIVION_CAST(form, TESForm, TESTexture);
		if (tex) {
			const char* nuPath = eval.Arg(0)->GetString();
			if (nuPath) {
				tex->ddsPath.Set(nuPath);
				*result = 1;
			}
		}
	}

	return true;
}

enum {
	eMode_Get,
	eMode_Set
};

static bool BipedPathFunc_Execute(COMMAND_ARGS, UInt32 mode, bool bIcon)
{
	UInt32 whichPath = 0;
	TESForm* form = NULL;
	const char* pathStr = "";
	char newPath[kMaxMessageLength] = { 0 };

	bool bExtracted = false;
	if (mode == eMode_Set)
		bExtracted = ExtractFormatStringArgs(0, newPath, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetBipedModelPathEX.numParams, &whichPath, &form);
	else
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &whichPath, &form);

	if (bExtracted)
	{
		if (!form)
			if (thisObj)
				form = thisObj->baseForm;

		TESBipedModelForm* bipedModel = (TESBipedModelForm*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
		if (bipedModel)
		{
			bool bFemale = (whichPath % 2) ? true : false;
			whichPath = bIcon ? bipedModel->kPath_Icon : whichPath / 2;

			if (mode == eMode_Set)
				bipedModel->SetPath(newPath, whichPath, bFemale);
			else
				pathStr = bipedModel->GetPath(whichPath, bFemale);
		}
	}

	if (mode == eMode_Get)
		AssignToStringVar(PASS_COMMAND_ARGS, pathStr);

	return true;
}

static bool Cmd_GetBipedModelPath_Execute(COMMAND_ARGS)
{
	return BipedPathFunc_Execute(PASS_COMMAND_ARGS, eMode_Get, false);
}

static bool Cmd_SetBipedModelPathEX_Execute(COMMAND_ARGS)
{
return BipedPathFunc_Execute(PASS_COMMAND_ARGS, eMode_Set, false);
}

static bool Cmd_GetBipedIconPath_Execute(COMMAND_ARGS)
{
	return BipedPathFunc_Execute(PASS_COMMAND_ARGS, eMode_Get, true);
}

static bool Cmd_SetBipedIconPathEX_Execute(COMMAND_ARGS)
{
	return BipedPathFunc_Execute(PASS_COMMAND_ARGS, eMode_Set, true);
}

static bool Cmd_GetNthFactionRankName_Execute(COMMAND_ARGS)
{
	TESFaction* faction = NULL;
	UInt32 rank = 0;
	UInt32 gender = 0;
	const char* rankName = "";

	if (ExtractArgs(PASS_EXTRACT_ARGS, &faction, &rank, &gender))
		rankName = faction->GetNthRankName(rank, (gender ? true : false));

	AssignToStringVar(PASS_COMMAND_ARGS, rankName);
	return true;
}

static bool Cmd_SetNthFactionRankNameEX_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 rank = 0;
	UInt32 gender = 0;
	char newName[kMaxMessageLength] = { 0 };

	if (ExtractFormatStringArgs(0, newName, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetNthFactionRankNameEX.numParams, &form, &rank, &gender))
	{
		TESFaction* faction = (TESFaction*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESFaction, 0);
		if (faction)
			faction->SetNthRankName(newName, rank, gender ? true : false);
	}

	return true;
}

static bool Cmd_GetNthEffectItemScriptName_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 whichEffect = 0;
	const char* effectName = "";

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &whichEffect))
	{
		EffectItemList* effectList = GetEffectList(form);
		if (effectList)
		{
			EffectItem* effectItem = effectList->ItemAt(whichEffect);
			if (effectItem && effectItem->scriptEffectInfo)
				effectName = effectItem->scriptEffectInfo->effectName.m_data;
		}
	}

	AssignToStringVar(PASS_COMMAND_ARGS, effectName);
	return true;
}

static bool Cmd_SetNthEffectItemScriptNameEX_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	UInt32 whichEffect = 0;
	char newName[kMaxMessageLength] = { 0 };

	if (ExtractFormatStringArgs(0, newName, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetNthEffectItemScriptNameEX.numParams, &form, &whichEffect))
	{
		EffectItemList* effectList = GetEffectList(form);
		if (effectList)
		{
			EffectItem* effectItem = effectList->ItemAt(whichEffect);
			if (effectItem && effectItem->scriptEffectInfo)
				effectItem->scriptEffectInfo->effectName.Set(newName);
		}
	}

	return true;
}

static bool Cmd_GetKeyName_Execute(COMMAND_ARGS)
{
	const char* keyname = "";
	UInt32 keycode = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &keycode))
		keyname = GetDXDescription(keycode);

	AssignToStringVar(PASS_COMMAND_ARGS, keyname);
	return true;
}

static bool Cmd_AsciiToChar_Execute(COMMAND_ARGS)
{
	char charStr[2] = "\0";
	UInt32 asciiCode = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &asciiCode) && asciiCode < 256)
		charStr[0] = asciiCode;

	AssignToStringVar(PASS_COMMAND_ARGS, charStr);
	return true;
}

static bool Cmd_GetFormIDString_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form);
	if (!form)
		form = thisObj;

	UInt32 formID = form ? form->refID : 0;
	char str[0x20] = { 0 };
	sprintf_s(str, sizeof(str), "%08X", formID);
	AssignToStringVar(PASS_COMMAND_ARGS, str);
	return true;
}

static bool Cmd_GetRawFormIDString_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	char str[0x20] = { 0 };
	UInt32 formID = 0;

	if (eval.ExtractArgs() && eval.NumArgs() == 1) {
		ScriptToken* arg = eval.Arg(0);
		if (arg->CanConvertTo(kTokenType_ArrayElement)) {
			formID = arg->GetFormID();
		}
		else if (arg->Type() == kTokenType_RefVar) {
			ScriptEventList::Var* var = arg->GetVar();
			if (var) {
				formID = *((UInt32*)(&var->data));
			}
		}
	}

	sprintf_s(str, sizeof(str), "%08X", formID);
	AssignToStringVar(PASS_COMMAND_ARGS, str);
	DEBUG_PRINT(str);
	return true;
}

static bool Cmd_NumToHex_Execute(COMMAND_ARGS)
{
	UInt32 num = 0;
	UInt32 width = 8;
	ExtractArgs(PASS_EXTRACT_ARGS, &num, &width);

	char fmtStr[0x20];
	width = width <= 8 ? width : 8;
	sprintf_s(fmtStr, sizeof(fmtStr), "%%0%dX", width);

	char hexStr[0x20];
	sprintf_s(hexStr, sizeof(hexStr), fmtStr, num);

	AssignToStringVar(PASS_COMMAND_ARGS, hexStr);
	return true;
}

static bool Cmd_ToNumber_Execute(COMMAND_ARGS)
{
	// usage: ToNumber string bool:fromHex
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0)
	{
		bool bHex = false;
		if (eval.Arg(1) && eval.Arg(1)->CanConvertTo(kTokenType_Number))
			bHex = eval.Arg(1)->GetNumber() ? true : false;

		*result = eval.Arg(0)->GetNumericRepresentation(bHex);
	}

	return true;
}

static bool Cmd_sv_Split_Execute(COMMAND_ARGS)
{
	// args: string delims
	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 2 && eval.Arg(0)->CanConvertTo(kTokenType_String) && eval.Arg(1)->CanConvertTo(kTokenType_String))
	{
		Tokenizer tokens(eval.Arg(0)->GetString(), eval.Arg(1)->GetString());
		std::string token;

		double idx = 0.0;
		while (tokens.NextToken(token) != -1)
		{
			g_ArrayMap.SetElementString(arrID, idx, token);
			idx += 1.0;
		}
	}

	return true;
}

static bool Cmd_GetOblivionDirectory_Execute(COMMAND_ARGS)
{
	std::string path = GetOblivionDirectory();
	const char* pathStr = path.c_str();
	AssignToStringVar(PASS_COMMAND_ARGS, pathStr);
	return true;
}

static bool Cmd_sv_Percentify_Execute(COMMAND_ARGS)
{
	std::string converted = "";
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 1) {
		const char* src = eval.Arg(0)->GetString();
		if (src) {
			converted = src;
			UInt32 pos = 0;
			while (pos < converted.length() && (pos = converted.find('%', pos)) != -1) {
				converted.insert(pos, 1, '%');
				pos += 2;
			}
		}
	}

	AssignToStringVar(PASS_COMMAND_ARGS, converted.c_str());
	return true;
}

static bool ChangeCase_Execute (COMMAND_ARGS, bool bUpper)
{
	std::string converted = "";
	ExpressionEvaluator eval (PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 1)
	{
		const char* src = eval.Arg (0)->GetString();
		if (src)
		{
			converted = src;
			if (bUpper)
				MakeUpper (converted);
			else
				MakeLower (converted);
		}
	}

	AssignToStringVar (PASS_COMMAND_ARGS, converted.c_str ());
	return true;
}

static bool Cmd_sv_ToUpper_Execute (COMMAND_ARGS)
{
	return ChangeCase_Execute (PASS_COMMAND_ARGS, true);
}

static bool Cmd_sv_ToLower_Execute (COMMAND_ARGS)
{
	return ChangeCase_Execute (PASS_COMMAND_ARGS, false);
}

#endif

static ParamInfo kParams_sv_Destruct[10] =
{
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
	{	"string_var",	kOBSEParamType_StringVar,	1	},
};

CommandInfo kCommandInfo_sv_Destruct =
{
	"sv_Destruct",
	"",
	0,
	"destroys one or more string variables",
	0,
	10,
	kParams_sv_Destruct,
	HANDLER(Cmd_sv_Destruct_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_sv_Construct[21] =
{
	FORMAT_STRING_PARAMS,
};

DEFINE_COMMAND(sv_Construct,
			   returns a formatted string,
			   0,
			   21,
			   kParams_sv_Construct);

static ParamInfo kParams_sv_Set[22] =
{
	FORMAT_STRING_PARAMS,
	{	"stringVar",	kParamType_StringVar,	1	},
};

DEFINE_COMMAND(sv_Set, sets the contents of a string variable, 0, 22, kParams_sv_Set);

static ParamInfo kParams_sv_Compare[23] =
{
	FORMAT_STRING_PARAMS,
	{	"stringVar",		kParamType_StringVar,	0	},
	{	"bCaseSensitive",	kParamType_Integer,		1	},
};

DEFINE_COMMAND(sv_Compare,
			   compares two strings,
			   0,
			   23,
			   kParams_sv_Compare);

DEFINE_COMMAND(sv_Length,
			   returns the number of characters in a string,
			   0,
			   1,
			   kParams_OneInt);

static ParamInfo kParams_SubString[3] =
{
	{	"stringVar",	kParamType_Integer,	0	},
	{	"startPos",		kParamType_Integer,	1	},
	{	"howMany",		kParamType_Integer,	1	},
};

DEFINE_COMMAND(sv_Erase,
			   erases a portion of a string variable,
			   0,
			   3,
			   kParams_SubString);

DEFINE_COMMAND(sv_SubString,
			   returns a substring of a string variable,
				0,
				3,
				kParams_SubString);

static ParamInfo kParams_sv_ToNumeric[2] =
{
	{	"stringVar",	kParamType_Integer,	0	},
	{	"startPos",		kParamType_Integer,	1	},
};

DEFINE_COMMAND(sv_ToNumeric,
			   converts a string variable to a numeric type,
			   0,
			   2,
			   kParams_sv_ToNumeric);

DEFINE_COMMAND(sv_Insert,
			   inserts a substring in a string variable,
			   0,
			   23,
			   kParams_sv_Compare);

static ParamInfo kParams_sv_Find[25] =
{
	FORMAT_STRING_PARAMS,
	{	"stringVar",	kParamType_Integer,		0	},
	{	"startPos",		kParamType_Integer,		1	},
	{	"endPos",		kParamType_Integer,		1	},
	{	"bCaseSensitive", kParamType_Integer,	1	},
};

DEFINE_COMMAND(sv_Count,
			   returns the number of occurences of a substring within a string variable,
			   0,
			   25,
			   kParams_sv_Find);

DEFINE_COMMAND(sv_Find,
			   returns the position of a substring within a string variable or -1 if not found,
			   0,
			   25,
			   kParams_sv_Find);

static ParamInfo kParams_sv_Replace[26] =
{
	FORMAT_STRING_PARAMS,
	{	"stringVar",	kParamType_Integer,		0	},
	{	"startPos",		kParamType_Integer,		1	},
	{	"endPos",		kParamType_Integer,		1	},
	{	"bCaseSensitive", kParamType_Integer,		1	},
	{	"howMany",		kParamType_Integer,		1	},
};

DEFINE_COMMAND(sv_Replace,
			   replaces 1 or more occurences of a substring within a string variable,
			   0,
			   26,
			   kParams_sv_Replace);

DEFINE_COMMAND(IsLetter,
			   returns 1 if the character is an alphabetic character,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(IsDigit,
			   returns 1 if the character is a numeric character,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(IsPrintable,
			   returns 1 if the character is a printable character,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(IsPunctuation,
			   returns 1 if the character is a punctuation character,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(IsUpperCase,
			   returns 1 if the character is an uppercase letter,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(sv_GetChar,
			   returns the ASCII code of the character at the specified position in a string variable,
			   0,
			   2,
			   kParams_TwoInts);

DEFINE_COMMAND(CharToAscii,
			   converts a single character to its equivalent ASCII code,
			   0,
			   1,
			   kParams_OneString);

DEFINE_COMMAND(ToUpper,
			   converts a character to uppercase,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(ToLower,
			   converts a character to lowercase,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetNthModName,
			   returns the name of the nth active mod,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetName,
			   returns the name of an object,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetModelPath,
			   returns the model path of an object,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetIconPath,
			   returns the icon path of an object,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

static ParamInfo kParams_GetNthEffectItemScriptName[2] =
{
	{	"magic item", kParamType_MagicItem, 0 },
	{	"which effect", kParamType_Integer, 0 },
};

DEFINE_COMMAND(GetNthEffectItemScriptName,
			   returns the name of the nth scripted effect item,
			   0,
			   2,
			   kParams_GetNthEffectItemScriptName);

DEFINE_COMMAND(GetStringGameSetting,
			   returns the value of a string game setting,
			   0,
			   1,
			   kParams_OneString);

DEFINE_COMMAND(GetBipedModelPath,
			   returns a model path,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInventoryObject);

DEFINE_COMMAND(GetBipedIconPath,
			   returns an icon path,
			   0,
			   2,
			   kParams_OneInt_OneOptionalInventoryObject);

static ParamInfo kParams_GetNthRankName[3] =
{
	{	"faction",		kParamType_Faction,	0	},
	{	"rank",			kParamType_Integer,	0	},
	{	"bFemale",		kParamType_Integer,	1	},
};

DEFINE_COMMAND(GetNthFactionRankName,
			   returns the name of the faction rank,
			   0,
			   3,
			   kParams_GetNthRankName);

static ParamInfo kParams_SetPathEX[SIZEOF_FMT_STRING_PARAMS + 1] =
{
	FORMAT_STRING_PARAMS,
	{	"object",	kParamType_InventoryObject,	1	},
};

DEFINE_COMMAND(SetModelPathEX,
			   sets a simple model path,
			   0,
			   NUM_PARAMS(kParams_SetPathEX),
			   kParams_SetPathEX);

DEFINE_COMMAND(SetIconPathEX,
			   sets a simple icon path,
			   0,
			   NUM_PARAMS(kParams_SetPathEX),
			   kParams_SetPathEX);

static ParamInfo kParams_SetNthEffectItemScriptNameEX[SIZEOF_FMT_STRING_PARAMS + 2] =
{
	FORMAT_STRING_PARAMS,
	{	"magic item", kParamType_MagicItem, 0 },
	{	"which effect", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_SetNthEffectItemScriptNameEX =
{
	"SetNthEffectItemScriptNameEX",
	"SetNthEIScriptNameEX",
	0,
	"sets the name of the nth effect item script effect",
	0,
	NUM_PARAMS(kParams_SetNthEffectItemScriptNameEX),
	kParams_SetNthEffectItemScriptNameEX,
	HANDLER(Cmd_SetNthEffectItemScriptNameEX_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetBipedPathEX[SIZEOF_FMT_STRING_PARAMS + 2] =
{
	FORMAT_STRING_PARAMS,
	{	"whichPath",	kParamType_Integer,			0	},
	{	"item",			kParamType_InventoryObject,	1	},
};

DEFINE_COMMAND(SetBipedIconPathEX,
				sets a biped icon path,
				0,
				NUM_PARAMS(kParams_SetBipedPathEX),
				kParams_SetBipedPathEX);

DEFINE_COMMAND(SetBipedModelPathEX,
			   sets a biped model path,
			   0,
			   NUM_PARAMS(kParams_SetBipedPathEX),
			   kParams_SetBipedPathEX);

DEFINE_COMMAND(SetStringGameSettingEX,
			   sets a string game setting,
			   0,
			   NUM_PARAMS(kParams_FormatString),
			   kParams_FormatString);

static ParamInfo kParams_SetNthRankNameEX[SIZEOF_FMT_STRING_PARAMS + 3] =
{
	FORMAT_STRING_PARAMS,
	{	"faction",		kParamType_Faction,		0	},
	{	"rank",			kParamType_Integer,		0	},
	{	"bFemale",		kParamType_Integer,		1	},
};

DEFINE_COMMAND(SetNthFactionRankNameEX,
			   sets the name of the nth faction rank,
			   0,
			   NUM_PARAMS(kParams_SetNthRankNameEX),
			   kParams_SetNthRankNameEX);

CommandInfo kCommandInfo_ActorValueToString =
{
	"ActorValueToString",
	"AVString",
	0,
	"returns the localized string corresponding to an actor value",
	0,
	1,
	kParams_OneActorValue,
	HANDLER(Cmd_ActorValueToString_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ActorValueToStringC =
{
	"ActorValueToStringC",
	"AVStringC",
	0,
	"returns the localized string corresponding to an actor value code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_ActorValueToString_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetKeyName, returns the name of a key given a scan code, 0, 1, kParams_OneInt);
DEFINE_COMMAND(AsciiToChar, returns a single character string given an ASCII code, 0, 1, kParams_OneInt);
DEFINE_COMMAND(GetFormIDString, returns a formID of a form as a hex string, 0, 1, kParams_OneOptionalInventoryObject);
DEFINE_COMMAND(NumToHex, returns a number as a hex string of the specified width, 0, 2, kParams_OneInt_OneOptionalInt);

static ParamInfo kOBSEParams_OneStringOneOptionalInt[2] =
{
	{	"string",	kOBSEParamType_String,		0	},
	{	"bHex",		kOBSEParamType_Number,		1	},
};

CommandInfo kCommandInfo_ToNumber =
{
	"ToNumber",
	"",
	0,
	"translates a string to a number",
	0,
	2,
	kOBSEParams_OneStringOneOptionalInt,
	HANDLER(Cmd_ToNumber_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_TwoStrings[2] =
{
	{	"string",		kOBSEParamType_String,	0	},
	{	"string",		kOBSEParamType_String,	0	},
};

CommandInfo kCommandInfo_sv_Split =
{
	"sv_Split",
	"",
	0,
	"split a string into substrings returning an array",
	0,
	2,
	kOBSEParams_TwoStrings,
	HANDLER(Cmd_sv_Split_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetOblivionDirectory =
{
	"GetOblivionDirectory",
	"GetOblivionDir",
	0,
	"returns the path to the Oblivion directory",
	0,
	0,
	0,
	HANDLER(Cmd_GetOblivionDirectory_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneOBSEString[] =
{
	{	"string",	kOBSEParamType_String,	0	},
};

CommandInfo kCommandInfo_sv_Percentify =
{
	"sv_Percentify", "",
	0,
	"converts all percent signs in a string to double percent signs",
	0, 1, kParams_OneOBSEString,
	HANDLER(Cmd_sv_Percentify_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};

DEFINE_COMMAND(GetTexturePath, "returns the texture path of an object. This command is identical to GetIconPath, but also works for other object types such as skills, classes, and miscellaneous objects.",
			   0, 1, kParams_OneOptionalInventoryObject);

static ParamInfo kOBSEParams_OneString_OneOptionalForm[] =
{
	{	"string",	kOBSEParamType_String,	0	},
	{	"object",	kOBSEParamType_Form,	1	},
};

CommandInfo kCommandInfo_SetTexturePath =
{
	"SetTexturePath", "",
	0,
	"sets the texture path of an object. This command works for a broader set of objects than SetIconPathEX.",
	0, 2, kOBSEParams_OneString_OneOptionalForm,
	HANDLER(Cmd_SetTexturePath_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};

static ParamInfo kOBSEParams_OneForm[1] =
{
	{	"object",	kOBSEParamType_Form,	0	},
};

CommandInfo kCommandInfo_GetRawFormIDString =
{
	"GetRawFormIDString", "GetFormIDString2", 0,
	"returns the form ID stored in an array element or ref variable as a string, regardless of whether or not the formID is valid",
	0, 1, kOBSEParams_OneForm,
	HANDLER(Cmd_GetRawFormIDString_Execute),
	Cmd_Expression_Parse, NULL, 0
};

CommandInfo kCommandInfo_sv_ToLower =
{
	"sv_ToLower", "",
	0,
	"converts all characters in the string to lowercase",
	0, 1, kParams_OneOBSEString,
	HANDLER(Cmd_sv_ToLower_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_sv_ToUpper =
{
	"sv_ToUpper", "",
	0,
	"converts all characters in the string to uppercase",
	0, 1, kParams_OneOBSEString,
	HANDLER(Cmd_sv_ToUpper_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};