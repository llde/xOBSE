#include "CommandTable.h"
#include "ParamInfos.h"
#include "Commands_General.h"
#include "ScriptUtils.h"
#include "Hooks_Script.h"

#if OBLIVION

#include "GameAPI.h"
#include "GameExtraData.h"
#include "GameObjects.h"
#include "InventoryReference.h"
#include "FunctionScripts.h"
#include "Loops.h"
#include <obse/Settings.h>

#define GET_EXECUTION_STATE(STATE)			\
{											\
	UInt32	_esi;							\
	__asm { mov _esi, esi }					\
	STATE = (ScriptExecutionState*)_esi;		\
}

// *********** commands

static bool Cmd_Let_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator evaluator(PASS_COMMAND_ARGS);
	evaluator.ExtractArgs();

	return true;
}

// used to evaluate OBSE expressions within 'if' statements
// i.e. if eval (array[idx] == someThing)
static bool Cmd_eval_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);

	if (eval.ExtractArgs() && eval.Arg(0))
		*result = eval.Arg(0)->GetBool() ? 1 : 0;

	return true;
}

// attempts to evaluate an expression. Returns false if error occurs, true otherwise. Suppresses error messages
//The suppression is now controlled with the bTestExprComplainsOnError in the obse.ini. This allow modders that want to check explicitly for error to do so
static bool Cmd_testexpr_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (NoisyTestExpr == 0) {
		eval.ToggleErrorSuppression(true);
	}
	if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->IsGood() && !eval.HasErrors())
	{
		if (eval.Arg(0)->Type() == kTokenType_ArrayElement)		// is it an array elem with valid index?
		{
			const ArrayKey* key = eval.Arg(0)->GetArrayKey();
			*result = (g_ArrayMap.HasKey(eval.Arg(0)->GetOwningArrayID(), *key)) ? 1 : 0;
		}
		else
			*result = 1;
	}

	eval.ToggleErrorSuppression(false);
	return true;
}

static bool Cmd_While_Execute(COMMAND_ARGS)
{
	ScriptExecutionState* state;
	GET_EXECUTION_STATE(state);

	// read offset to end of loop
	UInt8* data = (UInt8*)arg1 + *opcodeOffsetPtr;
	UInt32 offsetToEnd = *(UInt32*)data;

	// calc offset of first instruction following this While expression
	data += 5;		// UInt32 offset + UInt8 numArgs
	UInt16 exprLen = *((UInt16*)data);
	UInt32 startOffset = *opcodeOffsetPtr + 5 + exprLen;

	// create the loop and add it to loop manager
	WhileLoop* loop = new WhileLoop(*opcodeOffsetPtr + 4);
	LoopManager* mgr = LoopManager::GetSingleton();
	mgr->Add(loop, state, startOffset, offsetToEnd, PASS_COMMAND_ARGS);

	// test condition, break immediately if false
	*opcodeOffsetPtr = startOffset;		// need to update to point to _next_ instruction before calling loop->Update()
	if (!loop->Update(PASS_COMMAND_ARGS))
		mgr->Break(state, PASS_COMMAND_ARGS);

	return true;
}

static bool Cmd_ForEach_Execute(COMMAND_ARGS)
{
	ScriptExecutionState* state;
	GET_EXECUTION_STATE(state);

	// get offset to end of loop
	UInt8* data = (UInt8*)arg1 + *opcodeOffsetPtr;
	UInt32 offsetToEnd = *(UInt32*)data;

	// calc offset to first instruction within loop
	data += 5;
	UInt16 exprLen = *((UInt16*)data);
	UInt32 startOffset = *opcodeOffsetPtr + 5 + exprLen;

	ForEachLoop* loop = NULL;

	// evaluate the expression to get the context
	*opcodeOffsetPtr += 4;				// set to start of expression
	{
		// ExpressionEvaluator enclosed in this scope so that it's lock is released once we've extracted the args.
		// This eliminates potential for deadlock when adding loop to LoopManager
		ExpressionEvaluator eval(PASS_COMMAND_ARGS);
		bool bExtracted = eval.ExtractArgs();
		*opcodeOffsetPtr -= 4;				// restore

		if (!bExtracted || !eval.Arg(0))
		{
			ShowRuntimeError(scriptObj, "ForEach expression failed to return a value");
			return false;
		}

		const ForEachContext* context = eval.Arg(0)->GetForEachContext();
		if (!context)
			ShowRuntimeError(scriptObj, "Cmd_ForEach: Expression does not evaluate to a ForEach context");
		else		// construct the loop
		{
			if (context->variableType == Script::eVarType_Array)
			{
				ArrayIterLoop* arrayLoop = new ArrayIterLoop(context, scriptObj->GetModIndex());
				loop = arrayLoop;
			}
			else if (context->variableType == Script::eVarType_String)
			{
				StringIterLoop* stringLoop = new StringIterLoop(context);
				loop = stringLoop;
			}
			else if (context->variableType == Script::eVarType_Ref)
			{
				loop = new ContainerIterLoop(context);
			}
		}
	}

	if (loop)
	{
		LoopManager* mgr = LoopManager::GetSingleton();
		mgr->Add(loop, state, startOffset, offsetToEnd, PASS_COMMAND_ARGS);
		if (loop->IsEmpty())
		{
			*opcodeOffsetPtr = startOffset;
			mgr->Break(state, PASS_COMMAND_ARGS);
		}
	}

	return true;
}

static bool Cmd_Break_Execute(COMMAND_ARGS)
{
	ScriptExecutionState* state;
	GET_EXECUTION_STATE(state);

	LoopManager* mgr = LoopManager::GetSingleton();
	if (mgr->Break(state, PASS_COMMAND_ARGS))
		return true;

	ShowRuntimeError(scriptObj, "Break called outside of a valid loop context.");
	return false;
}

static bool Cmd_Continue_Execute(COMMAND_ARGS)
{
	ScriptExecutionState* state;
	GET_EXECUTION_STATE(state);

	LoopManager* mgr = LoopManager::GetSingleton();
	if (mgr->Continue(state, PASS_COMMAND_ARGS))
	{
		*opcodeOffsetPtr += 4;
		return true;
	}

	ShowRuntimeError(scriptObj, "Continue called outside of a valid loop context.");
	return false;
}

static bool Cmd_Loop_Execute(COMMAND_ARGS)
{
	ScriptExecutionState* state;
	GET_EXECUTION_STATE(state);

	LoopManager* mgr = LoopManager::GetSingleton();
	if (mgr->Continue(state, PASS_COMMAND_ARGS))
		return true;

	ShowRuntimeError(scriptObj, "Loop called outside of a valid loop context.");
	return false;
}

static bool Cmd_ToString_Execute(COMMAND_ARGS)
{
	*result = 0;
	std::string tokenAsString = "NULL";

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0))
	{
		if (eval.Arg(0)->CanConvertTo(kTokenType_String))
			tokenAsString = eval.Arg(0)->GetString();
		else if (eval.Arg(0)->CanConvertTo(kTokenType_Number))
		{
			char buf[0x20];
			sprintf_s(buf, sizeof(buf), "%g", eval.Arg(0)->GetNumber());
			tokenAsString = buf;
		}
		else if (eval.Arg(0)->CanConvertTo(kTokenType_Form))
			tokenAsString = GetFullName(eval.Arg(0)->GetTESForm());
	}

	AssignToStringVar(PASS_COMMAND_ARGS, tokenAsString.c_str());
	return true;
}

static bool Cmd_Print_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->CanConvertTo(kTokenType_String))
	{
		const char* str = eval.Arg(0)->GetString();
		if (strlen(str) < 512)
			Console_Print(str);
		else
			Console_Print_Long(str);
#if _DEBUG
		// useful for testing script output
		_MESSAGE(str);
#endif
	}

	return true;
}

static bool Cmd_TypeOf_Execute(COMMAND_ARGS)
{
	std::string typeStr = "NULL";

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0))
	{
		if (eval.Arg(0)->CanConvertTo(kTokenType_Number))
			typeStr = "Number";
		else if (eval.Arg(0)->CanConvertTo(kTokenType_String))
			typeStr = "String";
		else if (eval.Arg(0)->CanConvertTo(kTokenType_Form))
			typeStr = "Form";
		else if (eval.Arg(0)->CanConvertTo(kTokenType_Array))
			typeStr = g_ArrayMap.GetTypeString(eval.Arg(0)->GetArray());
	}

	AssignToStringVar(PASS_COMMAND_ARGS, typeStr.c_str());
	return true;
}

static bool Cmd_Function_Execute(COMMAND_ARGS)
{
	*result = UserFunctionManager::Enter(scriptObj) ? 1 : 0;
	return true;
}

static bool Cmd_Call_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	ScriptToken* funcResult = UserFunctionManager::Call(&eval);
	if (funcResult)
	{
		if (funcResult->CanConvertTo(kTokenType_Number))
			*result = funcResult->GetNumber();
		else if (funcResult->CanConvertTo(kTokenType_String))
		{
			AssignToStringVar(PASS_COMMAND_ARGS, funcResult->GetString());
			eval.ExpectReturnType(kRetnType_String);
		}
		else if (funcResult->CanConvertTo(kTokenType_Form))
		{
			UInt32* refResult = (UInt32*)result;
			*refResult = funcResult->GetFormID();
			eval.ExpectReturnType(kRetnType_Form);
		}
		else if (funcResult->CanConvertTo(kTokenType_Array))
		{
			*result = funcResult->GetArray();
			eval.ExpectReturnType(kRetnType_Array);
		}
		else
			ShowRuntimeError(scriptObj, "Function call returned unexpected token type %d", funcResult->Type());
	}

	delete funcResult;
	return true;
}

static bool Cmd_SetFunctionValue_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (!UserFunctionManager::Return(&eval))
		ShowRuntimeError(scriptObj, "SetFunctionValue statement failed.");

	return true;
}

static bool Cmd_GetUserTime_Execute(COMMAND_ARGS)
{
	ArrayID arrID = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	*result = arrID;

	SYSTEMTIME localTime;
	GetLocalTime(&localTime);

	g_ArrayMap.SetElementNumber(arrID, "Year", localTime.wYear);
	g_ArrayMap.SetElementNumber(arrID, "Month", localTime.wMonth);
	g_ArrayMap.SetElementNumber(arrID, "DayOfWeek", localTime.wDayOfWeek + 1);
	g_ArrayMap.SetElementNumber(arrID, "Day", localTime.wDay);
	g_ArrayMap.SetElementNumber(arrID, "Hour", localTime.wHour);
	g_ArrayMap.SetElementNumber(arrID, "Minute", localTime.wMinute);
	g_ArrayMap.SetElementNumber(arrID, "Second", localTime.wSecond);
	g_ArrayMap.SetElementNumber(arrID, "Millisecond", localTime.wMilliseconds);

	return true;
}

class ModLocalDataManager
{
	// the idea here is to allow mods to store data in memory independent of any savegame
	// this lets them keep track of their mod state as savegames are loaded, resetting things if necessary or performing other housework
	// data is stored as key:value pairs, key is string, value is a formID, number, or string

public:
	ArrayElement* Get(UInt8 modIndex, const char* key);
	bool Set(UInt8 modIndex, const char* key, const ArrayElement& data);
	bool Set(UInt8 modIndex, const char* key, ScriptToken* data, ExpressionEvaluator& eval);
	bool Remove(UInt8 modIndex, const char* key);
	ArrayID GetAllAsOBSEArray(UInt8 modIndex);

	~ModLocalDataManager();
private:
	typedef std::map<const char*, ArrayElement*, bool (*)(const char*, const char*)> ModLocalData;
	typedef std::map<UInt8, ModLocalData*> ModLocalDataMap;

	ModLocalDataMap m_data;
};

ModLocalDataManager s_modDataManager;

ArrayElement* ModLocalDataManager::Get(UInt8 modIndex, const char* key)
{
	ModLocalDataMap::iterator iter = m_data.find(modIndex);
	if (iter != m_data.end() && iter->second) {
		ModLocalData::iterator dataIter = iter->second->find(key);
		if (dataIter != iter->second->end()) {
			return dataIter->second;
		}
	}

	return NULL;
}

ArrayID ModLocalDataManager::GetAllAsOBSEArray(UInt8 modIndex)
{
	ArrayID id = g_ArrayMap.Create(kDataType_String, false, modIndex);
	ModLocalDataMap::iterator iter = m_data.find(modIndex);
	if (iter != m_data.end() && iter->second) {
		for (ModLocalData::iterator dataIter = iter->second->begin(); dataIter != iter->second->end(); ++dataIter) {
			ArrayElement* elem = dataIter->second;
			const char* key = dataIter->first;
			g_ArrayMap.SetElement(id, key, *elem);
		}
	}

	return id;
}

bool ModLocalDataManager::Remove(UInt8 modIndex, const char* key)
{
	ModLocalDataMap::iterator iter = m_data.find(modIndex);
	if (iter != m_data.end() && iter->second) {
		ModLocalData::iterator dataIter = iter->second->find(key);
		if (dataIter != iter->second->end()) {
			iter->second->erase(dataIter);
			return true;
		}
	}

	return false;
}

bool ModLocalDataManager::Set(UInt8 modIndex, const char* key, const ArrayElement& data)
{
	ArrayID dummy = 0;
	if (key && !data.GetAsArray(&dummy)) {
		UInt32 len = strlen(key) + 1;
		char* newKey = new char[len+1];
		strcpy_s(newKey, len, key);
		MakeUpper(newKey);
		key = newKey;

		ModLocalDataMap::iterator indexIter = m_data.find(modIndex);
		if (indexIter == m_data.end()) {
			indexIter = m_data.insert(ModLocalDataMap::value_type(modIndex, new ModLocalData(ci_less))).first;
		}

		ModLocalData::iterator dataIter = indexIter->second->find(key);
		if (dataIter == indexIter->second->end()) {
			dataIter = indexIter->second->insert(ModLocalData::value_type(key, new ArrayElement())).first;
		}

		return dataIter->second->Set(data);
	}

	return false;
}

bool ModLocalDataManager::Set(UInt8 modIndex, const char* key, ScriptToken* data, ExpressionEvaluator& eval)
{
	if (data) {
		ArrayElement elem;
		if (BasicTokenToElem(data, elem, &eval)) {
			return Set(modIndex, key, elem);
		}
	}

	return false;
}

ModLocalDataManager::~ModLocalDataManager()
{
	for (ModLocalDataMap::iterator index = m_data.begin(); index != m_data.end(); ++index) {
		for (ModLocalData::iterator data = index->second->begin(); data != index->second->end(); ++data) {
			delete data->second;
			delete data->first;
		}
		delete index->second;
	}
}

static bool Cmd_SetModLocalData_Execute(COMMAND_ARGS)
{
	*result = 0.0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 2 && eval.Arg(0)->CanConvertTo(kTokenType_String)) {
		*result = s_modDataManager.Set(scriptObj->GetModIndex(), eval.Arg(0)->GetString(), eval.Arg(1), eval) ? 1.0 : 0.0;
	}

	return true;
}

static bool Cmd_RemoveModLocalData_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 1 && eval.Arg(0)->CanConvertTo(kTokenType_String)) {
		*result = s_modDataManager.Remove(scriptObj->GetModIndex(), eval.Arg(0)->GetString()) ? 1.0 : 0.0;
	}

	return true;
}

static bool Cmd_GetModLocalData_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	eval.ExpectReturnType(kRetnType_Default);
	*result = 0;

	if (eval.ExtractArgs() && eval.NumArgs() == 1 && eval.Arg(0)->CanConvertTo(kTokenType_String)) {
		ArrayElement* data = s_modDataManager.Get(scriptObj->GetModIndex(), eval.Arg(0)->GetString());
		if (data) {
			double num = 0;
			UInt32 formID = 0;
			std::string str;

			if (data->GetAsNumber(&num)) {
				*result = num;
				return true;
			}
			else if (data->GetAsFormID(&formID)) {
				UInt32* refResult = (UInt32*)result;
				*refResult = formID;
				eval.ExpectReturnType(kRetnType_Form);
				return true;
			}
			else if (data->GetAsString(str)) {
				AssignToStringVar(PASS_COMMAND_ARGS, str.c_str());
				eval.ExpectReturnType(kRetnType_String);
				return true;
			}
		}
	}

	return true;
}

static bool Cmd_ModLocalDataExists_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 1 && eval.Arg(0)->CanConvertTo(kTokenType_String)) {
		*result = s_modDataManager.Get(scriptObj->GetModIndex(), eval.Arg(0)->GetString()) ? 1.0 : 0.0;
	}

	return true;
}

static bool Cmd_GetAllModLocalData_Execute(COMMAND_ARGS)
{
	*result = s_modDataManager.GetAllAsOBSEArray(scriptObj->GetModIndex());
	return true;
}

static bool Cmd_Internal_PushExecutionContext_Execute(COMMAND_ARGS)
{
	ExtractArgsOverride::PushContext(thisObj, contObj, (UInt8*)arg1, opcodeOffsetPtr);
	return true;
}

static bool Cmd_Internal_PopExecutionContext_Execute(COMMAND_ARGS)
{
	return ExtractArgsOverride::PopContext();
}

#endif

static bool Cmd_Let_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
#if OBLIVION
	Console_Print("Let cannot be called from the console.");
	return false;
#endif

	ExpressionParser parser(scriptBuf, lineBuf);
	if (!parser.ParseArgs(paramInfo, numParams))
		return false;

	// verify that assignment operator is last data recorded
	UInt8 lastData = lineBuf->dataBuf[lineBuf->dataOffset - 1];
	switch (lastData)
	{
	case kOpType_Assignment:
	case kOpType_PlusEquals:
	case kOpType_TimesEquals:
	case kOpType_DividedEquals:
	case kOpType_ExponentEquals:
	case kOpType_MinusEquals:
		return true;
	default:
		#ifndef OBLIVION
			ShowCompilerError(scriptBuf, "Expected assignment in Let statement.\n\nCompiled script not saved.");
		#endif
		return false;
	}
}

static bool Cmd_BeginLoop_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	// reserve space for a UInt32 storing offset past end of loop
	RegisterLoopStart(scriptBuf->scriptData + scriptBuf->dataOffset + lineBuf->dataOffset + 4);
	lineBuf->dataOffset += sizeof(UInt32);

	// parse the loop condition
	ExpressionParser parser(scriptBuf, lineBuf);
	return parser.ParseArgs(paramInfo, numParams);
}

static bool Cmd_Loop_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	UInt8* endPtr = scriptBuf->scriptData + scriptBuf->dataOffset + lineBuf->dataOffset + 4;
	UInt32 offset = endPtr - scriptBuf->scriptData;		// num bytes between beginning of script and instruction following Loop

	if (!HandleLoopEnd(offset))
	{
#ifndef OBLIVION
		ShowCompilerError(scriptBuf, "'Loop' encountered without matching 'While' or 'ForEach'.\n\nCompiled script not saved.");
#endif
		return false;
	}

	return true;
}

static bool Cmd_Null_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	// takes no args, writes no data
	return true;
}

static bool Cmd_Function_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	ExpressionParser parser(scriptBuf, lineBuf);
	return parser.ParseUserFunctionDefinition();
}

static bool Cmd_Call_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	ExpressionParser parser(scriptBuf, lineBuf);
	return parser.ParseUserFunctionCall();
}

static ParamInfo kParams_OneBasicType[] =
{
	{	"expression",	kOBSEParamType_BasicType,	0	},
};

CommandInfo kCommandInfo_Let =
{
	"Let",
	"",
	0,
	"assigns the value of an expression to a variable",
	0,
	1,
	kParams_OneBasicType,
	HANDLER(Cmd_Let_Execute),
	Cmd_Let_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneBoolean[] =
{
	{	"boolean expression",	kOBSEParamType_Boolean,	0	},
};

CommandInfo kCommandInfo_eval =
{
	"eval",
	"",
	0,
	"evaluates an expression and returns a boolean result.",
	0,
	1,
	kParams_OneBoolean,
	HANDLER(Cmd_eval_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_NoTypeChecking[] =
{
	{	"expression",	kOBSEParamType_NoTypeCheck,	0	},
};

CommandInfo kCommandInfo_testexpr =
{
	"testexpr",
	"",
	0,
	"returns false if errors occur while evaluating expression, true otherwise",
	0,
	1,
	kParams_NoTypeChecking,
	HANDLER(Cmd_testexpr_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_While =
{
	"while",
	"",
	0,
	"loops until the given condition evaluates to false",
	0,
	1,
	kParams_OneBoolean,
	HANDLER(Cmd_While_Execute),
	Cmd_BeginLoop_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Loop =
{
	"loop",
	"",
	0,
	"marks the end of a While or ForEach loop",
	0,
	1,
	kParams_OneOptionalInt,				// unused, but need at least one param for Parse() to be called
	HANDLER(Cmd_Loop_Execute),
	Cmd_Loop_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Continue =
{
	"continue",
	"",
	0,
	"returns control to the top of a loop",
	0,
	1,
	kParams_OneOptionalInt,				// unused, but need at least one param for Parse() to be called
	HANDLER(Cmd_Continue_Execute),
	Cmd_Null_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Break =
{
	"break",
	"",
	0,
	"exits a loop",
	0,
	1,
	kParams_OneOptionalInt,				// unused, but need at least one param for Parse() to be called
	HANDLER(Cmd_Break_Execute),
	Cmd_Null_Parse,
	NULL,
	0
};

static ParamInfo kParams_ForEach[] =
{
	{	"ForEach expression",	kOBSEParamType_ForEachContext,	0	},
};

CommandInfo kCommandInfo_ForEach =
{
	"ForEach",
	"",
	0,
	"iterates over the elements of an array",
	0,
	1,
	kParams_ForEach,
	HANDLER(Cmd_ForEach_Execute),
	Cmd_BeginLoop_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneOBSEString[] =
{
	{	"string",	kOBSEParamType_String,	0	},
};

CommandInfo kCommandInfo_ToString =
{
	"ToString",
	"",
	0,
	"attempts to convert an expression to a string",
	0,
	1,
	kParams_OneBasicType,
	HANDLER(Cmd_ToString_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_TypeOf =
{
	"TypeOf",
	"",
	0,
	"returns a string representing the type of the expression",
	0,
	1,
	kParams_OneBasicType,
	HANDLER(Cmd_TypeOf_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Print =
{
	"Print",
	"",
	0,
	"prints a string expression to the console",
	0,
	1,
	kParams_OneOBSEString,
	HANDLER(Cmd_Print_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Function =
{
	"Function",
	"",
	0,
	"defines a function",
	0,
	1,
	kParams_OneOptionalInt,
	HANDLER(Cmd_Function_Execute),
	Cmd_Function_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Call =
{
	"Call",
	"",
	0,
	"calls a user-defined function",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_Call_Execute),
	Cmd_Call_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetFunctionValue =
{
	"SetFunctionValue",
	"",
	0,
	"returns a value from a user-defined function",
	0,
	1,
	kParams_OneBasicType,
	HANDLER(Cmd_SetFunctionValue_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetUserTime, returns the users local time and date as a stringmap, 0, 0, NULL);

static ParamInfo kOBSEParams_SetModLocalData[2] =
{
	{	"key",	kOBSEParamType_String,	0	},
	{	"data",	kOBSEParamType_String | kOBSEParamType_Number | kOBSEParamType_Form,	0	},
};

static ParamInfo kOBSEParams_OneString[1] =
{
	{	"string",	kOBSEParamType_String,	0	},
};

CommandInfo kCommandInfo_SetModLocalData =
{
	"SetModLocalData",
	"",
	0,
	"sets a key-value pair for the mod",
	0,
	2,
	kOBSEParams_SetModLocalData,
	HANDLER(Cmd_SetModLocalData_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetModLocalData =
{
	"GetModLocalData",
	"",
	0,
	"gets a key-value pair for the mod",
	0,
	1,
	kOBSEParams_OneString,
	HANDLER(Cmd_GetModLocalData_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModLocalDataExists =
{
	"ModLocalDataExists",
	"",
	0,
	"returns true if mod local data exists for the specified key",
	0,
	1,
	kOBSEParams_OneString,
	HANDLER(Cmd_ModLocalDataExists_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveModLocalData =
{
	"RemoveModLocalData", "", 0,
	"removes the specified entry from the mod's local data",
	0, 1, kOBSEParams_OneString,
	HANDLER(Cmd_RemoveModLocalData_Execute),
	Cmd_Expression_Parse, NULL, 0
};

DEFINE_COMMAND(GetAllModLocalData, returns a StringMap containing all local data for the calling mod, 0, 0, NULL);

CommandInfo kCommandInfo_Internal_PushExecutionContext =
{
	"@PushExecutionContext", "", 0,
	"internal command - pushes execution context for current script for use with ExtractArgsOverride",
	0, 0, NULL,
	HANDLER(Cmd_Internal_PushExecutionContext_Execute), Cmd_Null_Parse, NULL, 0
};

CommandInfo kCommandInfo_Internal_PopExecutionContext =
{
	"@PopExecutionContext", "", 0,
	"internal command - pops execution context for current script for use with ExtractArgsOverride",
	0, 0, NULL,
	HANDLER(Cmd_Internal_PopExecutionContext_Execute), Cmd_Null_Parse, NULL, 0
};