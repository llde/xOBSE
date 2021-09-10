#include "ExpressionEvaluator.h"
#include "ThreadLocal.h"
#include <obse/ScriptUtils.h>
#include <obse/GameData.h>
#include <obse/ParamInfos.h>

#ifdef OBLIVION

bool ExpressionEvaluator::Active()
{
	return ThreadLocalData::Get().expressionEvaluator != NULL;
}

void ExpressionEvaluator::ToggleErrorSuppression(bool bSuppress) {
	if (bSuppress) {
		m_flags.Clear(kFlag_ErrorOccurred);
		m_flags.Set(kFlag_SuppressErrorMessages);
	}
	else
		m_flags.Clear(kFlag_SuppressErrorMessages);
}

void ExpressionEvaluator::Error(const char* fmt, ...)
{
	m_flags.Set(kFlag_ErrorOccurred);

	if (m_flags.IsSet(kFlag_SuppressErrorMessages))
		return;

	va_list args;
	va_start(args, fmt);

	char	errorMsg[0x400];
	vsprintf_s(errorMsg, 0x400, fmt, args);
	// include script data offset and command name/opcode
//	UInt16* opcodePtr = (UInt16*)((UInt8*)m_scriptData + m_baseOffset);
	//	UInt16* opcodePtr = (UInt16*)((UInt8*)script->data + m_baseOffset);
	UInt16* opcodePtr = (UInt16*)((UInt8*)script->data + (*m_opcodeOffsetPtr - 4));
	CommandInfo* cmd = g_scriptCommands.GetByOpcode(*opcodePtr);

	// include mod filename, save having to ask users to figure it out themselves
	const char* modName = "Savegame";
	if (script->GetModIndex() != 0xFF)
	{
		modName = (*g_dataHandler)->GetNthModName(script->GetModIndex());
		if (!modName || !modName[0])
			modName = "Unknown";
	}

	ShowRuntimeError(script, "%s\n    File: %s Offset: 0x%04X Command: %s", errorMsg, modName, m_baseOffset, cmd ? cmd->longName : "<unknown>");
	//	if (m_flags.IsSet(kFlag_StackTraceOnError))
	PrintStackTrace();
}


void ExpressionEvaluator::Error(const char* fmt, ScriptToken* tok, ...)
{
	m_flags.Set(kFlag_ErrorOccurred);

	if (m_flags.IsSet(kFlag_SuppressErrorMessages))
		return;

	va_list args;
	va_start(args, tok);

	char	errorMsg[0x400];
	vsprintf_s(errorMsg, 0x400, fmt, args);
	CommandInfo* cmd = nullptr;
	if (tok != nullptr && tok->Type() == Token_Type::kTokenType_Command) cmd = tok->GetCommandInfo();
	else {
		// include script data offset and command name/opcode
	//	UInt16* opcodePtr = (UInt16*)((UInt8*)m_scriptData + m_baseOffset);
		//	UInt16* opcodePtr = (UInt16*)((UInt8*)script->data + m_baseOffset);
		UInt16* opcodePtr = (UInt16*)((UInt8*)script->data + (*m_opcodeOffsetPtr - 4));
		cmd = g_scriptCommands.GetByOpcode(*opcodePtr);
	}
	// include mod filename, save having to ask users to figure it out themselves
	const char* modName = "Savegame";
	if (script->GetModIndex() != 0xFF)
	{
		modName = (*g_dataHandler)->GetNthModName(script->GetModIndex());
		if (!modName || !modName[0])
			modName = "Unknown";
	}

	ShowRuntimeError(script, "%s\n    File: %s Offset: 0x%04X Command: %s", errorMsg, modName, m_baseOffset, cmd ? cmd->longName : "<unknown>");
	//	if (m_flags.IsSet(kFlag_StackTraceOnError))
	PrintStackTrace();
}
void ExpressionEvaluator::PrintStackTrace() {
	std::stack<const ExpressionEvaluator*> stackCopy;
	char output[0x100];

	ExpressionEvaluator* eval = this;
	while (eval) {
		//		CommandInfo* cmd = g_scriptCommands.GetByOpcode(*((UInt16*)((UInt8*)eval->script->data + eval->m_baseOffset)));
	//	CommandInfo* cmd = g_scriptCommands.GetByOpcode(*((UInt16*)((UInt8*)eval->m_scriptData + eval->m_baseOffset)));
		UInt16* opcodePtr = (UInt16*)((UInt8*)eval->script->data + (*eval->m_opcodeOffsetPtr - 4));
		CommandInfo* cmd = g_scriptCommands.GetByOpcode(*opcodePtr);
		sprintf_s(output, sizeof(output), "  %s @%04X script %08X", cmd ? cmd->longName : "<unknown>", eval->m_baseOffset, eval->script->refID);
		_MESSAGE(output);
		Console_Print(output);

		eval = eval->m_parent;
	}
}

UInt8 ExpressionEvaluator::ReadByte()
{
	UInt8 byte = *m_data;
	m_data++;
	return byte;
}

SInt8 ExpressionEvaluator::ReadSignedByte()
{
	SInt8 byte = *((SInt8*)m_data);
	m_data++;
	return byte;
}

UInt16 ExpressionEvaluator::Read16()
{
	UInt16 data = *((UInt16*)m_data);
	m_data += 2;
	return data;
}

SInt16 ExpressionEvaluator::ReadSigned16()
{
	SInt16 data = *((SInt16*)m_data);
	m_data += 2;
	return data;
}

UInt32 ExpressionEvaluator::Read32()
{
	UInt32 data = *((UInt32*)m_data);
	m_data += 4;
	return data;
}

SInt32 ExpressionEvaluator::ReadSigned32()
{
	SInt32 data = *((SInt32*)m_data);
	m_data += 4;
	return data;
}

double ExpressionEvaluator::ReadFloat()
{
	double data = *((double*)m_data);
	m_data += sizeof(double);
	return data;
}

std::string ExpressionEvaluator::ReadString()
{
	UInt16 len = Read16();
	char* buf = new char[len + 1];
	memcpy(buf, m_data, len);
	buf[len] = 0;
	std::string str = buf;
	m_data += len;
	delete buf;
	return str;
}

void ExpressionEvaluator::PushOnStack()
{
	ThreadLocalData& localData = ThreadLocalData::Get();
	ExpressionEvaluator* top = localData.expressionEvaluator;
	m_parent = top;
	localData.expressionEvaluator = this;
	// inherit properties of parent
	if (top) {
		/*
		_MESSAGE("%08X   %08X", m_baseOffset, top->m_data - (UInt8*)script->data - 4);
		// figure out base offset into script data
		if (top->script == script) {
			m_baseOffset = top->m_data - (UInt8*)script->data - 4;
		}
		else {	// non-recursive user-defined function call
			m_baseOffset = m_data - (UInt8*)script->data - 4;
		}
		*/
		// inherit flags
		m_flags.RawSet(top->m_flags.Get());
		m_flags.Clear(kFlag_ErrorOccurred);
	}
}

void ExpressionEvaluator::PopFromStack()
{
	if (m_parent) {
		// propogate info to parent
		m_parent->m_expectedReturnType = m_expectedReturnType;
		if (m_flags.IsSet(kFlag_ErrorOccurred)) {
			m_parent->m_flags.Set(kFlag_ErrorOccurred);
		}
	}

	ThreadLocalData& localData = ThreadLocalData::Get();
	localData.expressionEvaluator = m_parent;
}

ExpressionEvaluator::ExpressionEvaluator(COMMAND_ARGS) : m_opcodeOffsetPtr(opcodeOffsetPtr), m_result(result),
m_thisObj(thisObj), script(scriptObj), eventList(eventList), m_params(paramInfo), m_numArgsExtracted(0), m_baseOffset(0),
m_expectedReturnType(kRetnType_Default)
{
	m_scriptData = (UInt8*)arg1;
	m_data = m_scriptData + *m_opcodeOffsetPtr;

	memset(m_args, 0, sizeof(m_args));

	m_containingObj = contObj;
	m_baseOffset = *opcodeOffsetPtr - 4;

	m_flags.Clear();

	PushOnStack();
}

ExpressionEvaluator::~ExpressionEvaluator()
{
	PopFromStack();

	for (UInt32 i = 0; i < kMaxArgs; i++)
		delete m_args[i];
}

bool ExpressionEvaluator::ExtractArgs()
{
	UInt32 numArgs = ReadByte();
	UInt32 curArg = 0;

	while (curArg < numArgs)
	{
		ScriptToken* arg = Evaluate();
		if (!arg)
			break;

		m_args[curArg++] = arg;
	}

	if (numArgs == curArg)			// all args extracted
	{
		m_numArgsExtracted = curArg;
		return true;
	}
	else
		return false;
}

bool ExpressionEvaluator::ExtractDefaultArgs(va_list varArgs, bool bConvertTESForms)
{
	if (ExtractArgs()) {
		for (UInt32 i = 0; i < NumArgs(); i++) {
			ScriptToken* arg = Arg(i);
			ParamInfo* info = &m_params[i];
			if (!ConvertDefaultArg(arg, info, bConvertTESForms, varArgs)) {
				return false;
			}
		}

		return true;
	}
	else {
		DEBUG_PRINT("Couldn't extract default args");
		return false;
	}
}


bool ExpressionEvaluator::ExtractFormatStringArgs(va_list varArgs, UInt32 fmtStringPos, char* fmtStringOut, UInt32 maxParams)
{
	// first simply evaluate all arguments, whether intended for fmt string or cmd args
	if (ExtractArgs()) {
		if (NumArgs() < fmtStringPos) {
			// uh-oh.
			return false;
		}

		// convert and store any cmd args preceding fmt string
		for (UInt32 i = 0; i < fmtStringPos; i++) {
			if (!ConvertDefaultArg(Arg(i), &m_params[i], false, varArgs)) {
				return false;
			}
		}

		// grab the format string
		const char* fmt = Arg(fmtStringPos)->GetString();
		if (!fmt) {
			return false;
		}

		// interpret the format string
		OverriddenScriptFormatStringArgs fmtArgs(this, fmtStringPos);
		if (ExtractFormattedString(fmtArgs, fmtStringOut)) {
			// convert and store any remaining cmd args
			UInt32 trailingArgsOffset = fmtArgs.GetCurArgIndex();
			if (trailingArgsOffset < NumArgs()) {
				for (UInt32 i = trailingArgsOffset; i < NumArgs(); i++) {
					// adjust index into params to account for 20 'variable' args to format string.
					if (!ConvertDefaultArg(Arg(i), &m_params[fmtStringPos + SIZEOF_FMT_STRING_PARAMS + (i - trailingArgsOffset)], false, varArgs)) {
						return false;
					}
				}
			}

			// hooray
			return true;
		}
	}
	// something went wrong.
	return false;
}

bool ExpressionEvaluator::ConvertDefaultArg(ScriptToken* arg, ParamInfo* info, bool bConvertTESForms, va_list& varArgs)
{
	// hooray humongous switch statements
	switch (info->typeID)
	{
	case kParamType_Array:
	{
		UInt32* out = va_arg(varArgs, UInt32*);
		*out = arg->GetArray();
	}

	break;
	case kParamType_Integer:
		// handle string_var passed as integer to sv_* cmds
		if (arg->CanConvertTo(kTokenType_StringVar) && arg->GetVar()) {
			UInt32* out = va_arg(varArgs, UInt32*);
			*out = arg->GetVar()->data;
			break;
		}
		// fall-through intentional
	case kParamType_QuestStage:
	case kParamType_CrimeType:
		if (arg->CanConvertTo(kTokenType_Number)) {
			UInt32* out = va_arg(varArgs, UInt32*);
			*out = arg->GetNumber();
		}
		else {
			return false;
		}

		break;
	case kParamType_Float:
		if (arg->CanConvertTo(kTokenType_Number)) {
			float* out = va_arg(varArgs, float*);
			*out = arg->GetNumber();
		}
		else {
			return false;
		}
		break;
	case kParamType_String:
	{
		const char* str = arg->GetString();
		if (str) {
			char* out = va_arg(varArgs, char*);
#pragma warning(push)
#pragma warning(disable: 4996)
			strcpy(out, str);
#pragma warning(pop)
		}
		else {
			return false;
		}
	}
	break;
	case kParamType_Axis:
	{
		char axis = arg->GetAxis();
		if (axis != -1) {
			char* out = va_arg(varArgs, char*);
			*out = axis;
		}
		else {
			return false;
		}
	}
	break;
	case kParamType_ActorValue:
	{
		UInt32 actorVal = arg->GetActorValue();
		if (actorVal != kActorVal_NoActorValue) {
			UInt32* out = va_arg(varArgs, UInt32*);
			*out = actorVal;
		}
		else {
			return false;
		}
	}
	break;
	case kParamType_AnimationGroup:
	{
		UInt32 animGroup = arg->GetAnimGroup();
		if (animGroup != TESAnimGroup::kAnimGroup_Max) {
			UInt32* out = va_arg(varArgs, UInt32*);
			*out = animGroup;
		}
		else {
			return false;
		}
	}
	break;
	case kParamType_Sex:
	{
		UInt32 sex = arg->GetSex();
		if (sex != -1) {
			UInt32* out = va_arg(varArgs, UInt32*);
			*out = sex;
		}
		else {
			return false;
		}
	}
	break;
	case kParamType_MagicEffect:
	{
		EffectSetting* eff = arg->GetEffectSetting();
		if (eff) {
			EffectSetting** out = va_arg(varArgs, EffectSetting**);
			*out = eff;
		}
		else {
			return false;
		}
	}
	break;
	default:	// all the rest are TESForm
	{
		if (arg->CanConvertTo(kTokenType_Form)) {
			TESForm* form = arg->GetTESForm();

			if (!bConvertTESForms) {
				// we're done
				TESForm** out = va_arg(varArgs, TESForm**);
				*out = form;
			}
			else if (form) {	// expect form != null
				// gotta make sure form matches expected type, do pointer cast
				switch (info->typeID) {
				case kParamType_InventoryObject:
					if (form->IsInventoryObject()) {
						TESForm** out = va_arg(varArgs, TESForm**);
						*out = form;
					}
					else {
						return false;
					}
					break;
				case kParamType_ObjectRef:
				case kParamType_MapMarker:
				{
					TESObjectREFR* refr = OBLIVION_CAST(form, TESForm, TESObjectREFR);
					if (refr) {
						// kParamType_MapMarker must be a mapmarker refr
						if (info->typeID == kParamType_MapMarker && !refr->IsMapMarker()) {
							return false;
						}

						TESObjectREFR** out = va_arg(varArgs, TESObjectREFR**);
						*out = refr;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_Actor:
				{
					Actor* actor = OBLIVION_CAST(form, TESForm, Actor);
					if (actor) {
						Actor** out = va_arg(varArgs, Actor**);
						*out = actor;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_SpellItem:
				{
					SpellItem* spell = OBLIVION_CAST(form, TESForm, SpellItem);
					if (spell || form->typeID == kFormType_Book) {
						TESForm** out = va_arg(varArgs, TESForm**);
						*out = form;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_Cell:
				{
					TESObjectCELL* cell = OBLIVION_CAST(form, TESForm, TESObjectCELL);
					if (cell) {
						TESObjectCELL** out = va_arg(varArgs, TESObjectCELL**);
						*out = cell;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_MagicItem:
				{
					MagicItem* magic = OBLIVION_CAST(form, TESForm, MagicItem);
					if (magic) {
						MagicItem** out = va_arg(varArgs, MagicItem**);
						*out = magic;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_TESObject:
				{
					TESObject* object = OBLIVION_CAST(form, TESForm, TESObject);
					if (object) {
						TESObject** out = va_arg(varArgs, TESObject**);
						*out = object;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_ActorBase:
				{
					TESActorBase* base = OBLIVION_CAST(form, TESForm, TESActorBase);
					if (base) {
						TESActorBase** out = va_arg(varArgs, TESActorBase**);
						*out = base;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_Container:
				{
					TESObjectREFR* refr = OBLIVION_CAST(form, TESForm, TESObjectREFR);
					if (refr && refr->GetContainer()) {
						TESObjectREFR** out = va_arg(varArgs, TESObjectREFR**);
						*out = refr;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_WorldSpace:
				{
					TESWorldSpace* space = OBLIVION_CAST(form, TESForm, TESWorldSpace);
					if (space) {
						TESWorldSpace** out = va_arg(varArgs, TESWorldSpace**);
						*out = space;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_AIPackage:
				{
					TESPackage* pack = OBLIVION_CAST(form, TESForm, TESPackage);
					if (pack) {
						TESPackage** out = va_arg(varArgs, TESPackage**);
						*out = pack;
					}
					else {
						return false;
					}
				}
				break;
				case kParamType_CombatStyle:
				{
					TESCombatStyle* style = OBLIVION_CAST(form, TESForm, TESCombatStyle);
					if (style) {
						TESCombatStyle** out = va_arg(varArgs, TESCombatStyle**);
						*out = style;
					}
					else {
						return false;
					}
				}
				break;
				default:
					// these all check against a particular formtype, return TESForm*
				{
					UInt32 typeToMatch = -1;
					switch (info->typeID) {
					case kParamType_Sound:
						typeToMatch = kFormType_Sound; break;
					case kParamType_Topic:
						typeToMatch = kFormType_Dialog; break;
					case kParamType_Quest:
						typeToMatch = kFormType_Quest; break;
					case kParamType_Race:
						typeToMatch = kFormType_Race; break;
					case kParamType_Faction:
						typeToMatch = kFormType_Faction; break;
					case kParamType_Class:
						typeToMatch = kFormType_Class; break;
					case kParamType_Global:
						typeToMatch = kFormType_Global; break;
					case kParamType_Furniture:
						typeToMatch = kFormType_Furniture; break;
					case kParamType_Birthsign:
						typeToMatch = kFormType_BirthSign; break;
					case kParamType_WeatherID:
						typeToMatch = kFormType_Weather; break;
					case kParamType_NPC:
						typeToMatch = kFormType_NPC; break;
					case kParamType_EffectShader:
						typeToMatch = kFormType_EffectShader; break;
					}

					if (form->typeID == typeToMatch) {
						TESForm** out = va_arg(varArgs, TESForm**);
						*out = form;
					}
					else {
						return false;
					}
				}
				}
			}
			else {
				// null form
				return false;
			}
		}
	}
	}

	return true;
}


ScriptToken* ExpressionEvaluator::Evaluate()
{
	std::stack<ScriptToken*> operands;

	UInt16 argLen = Read16();
	UInt8* endData = m_data + argLen - sizeof(UInt16);
	while (m_data < endData)
	{
		ScriptToken* curToken = ScriptToken::Read(this);
		if (!curToken)
			break;

		if (curToken->Type() == kTokenType_Command)
		{
			// execute the command
			CommandInfo* cmdInfo = curToken->GetCommandInfo();
			if (!cmdInfo)
			{
				Error("Command is NULL");
				delete curToken;
				curToken = NULL;
				break;
			}

			TESObjectREFR* callingObj = m_thisObj;
			Script::RefVariable* callingRef = script->GetVariable(curToken->GetRefIndex());
			if (callingRef)
			{
				callingRef->Resolve(eventList);
				if (callingRef->form && callingRef->form->IsReference())
					callingObj = OBLIVION_CAST(callingRef->form, TESForm, TESObjectREFR);
				else
				{
					Error("Attempting to call a function on a NULL reference or base object: %s", curToken, cmdInfo->longName);
					delete curToken;
					curToken = NULL;
					break;
				}
			}
			TESObjectREFR* contObj = callingRef ? NULL : m_containingObj;
			if (cmdInfo->needsParent && !callingObj) {
				Error("Attempting to call function %s without a reference", curToken, cmdInfo->longName);
				delete curToken;
				curToken = nullptr;
				break;
			}
			double cmdResult = 0;
			UInt16 argsLen = Read16();
			UInt32 numBytesRead = 0;
			ExpectReturnType(kRetnType_Default);	// expect default return type unless called command specifies otherwise
			bool bExecuted = cmdInfo->execute(cmdInfo->params, m_data, callingObj, contObj, script, eventList, &cmdResult, &numBytesRead);

			if (!bExecuted)
			{
				delete curToken;
				curToken = NULL;
				Error("Command %s failed to execute", curToken, cmdInfo->longName);
				break;
			}

			m_data += argsLen - 2;

			// create a new ScriptToken* based on result type, delete command token when done
			ScriptToken* cmdToken = curToken;
			curToken = ScriptToken::Create(cmdResult);
			// adjust token type if we know command return type
			CommandReturnType retnType = g_scriptCommands.GetReturnType(cmdInfo);
			if (retnType == kRetnType_Ambiguous || retnType == kRetnType_ArrayIndex)	// return type ambiguous, cmd will inform us of type to expect
				retnType = GetExpectedReturnType();

			switch (retnType)
			{
			case kRetnType_String:
			{
				StringVar* strVar = g_StringMap.Get(cmdResult);
				delete curToken;
				curToken = ScriptToken::Create(strVar ? strVar->GetCString() : "");
				break;
			}
			case kRetnType_Array:
			{
				// ###TODO: cmds can return arrayID '0', not necessarily an error, does this support that?
				if (g_ArrayMap.Exists(cmdResult) || !cmdResult)
				{
					delete curToken;
					curToken = ScriptToken::CreateArray(cmdResult);
					break;
				}
				else
				{
					Error("A command returned an invalid array", curToken);
					break;
				}
			}
			case kRetnType_Form:
			{
				delete curToken;
				curToken = ScriptToken::CreateForm(*((UInt64*)&cmdResult));
				break;
			}
			case kRetnType_Default:
				delete curToken;
				curToken = ScriptToken::Create(cmdResult);
				break;
			default:
				Error("Unknown command return type %d while executing command in ExpressionEvaluator::Evaluate()", curToken, retnType);
			}

			delete cmdToken;
			cmdToken = NULL;
		}

		if (!(curToken->Type() == kTokenType_Operator))
			operands.push(curToken);
		else
		{
			Operator* op = curToken->GetOperator();
			ScriptToken* lhOperand = NULL;
			ScriptToken* rhOperand = NULL;

			if (op->numOperands > operands.size())
			{
				delete curToken;
				curToken = NULL;
				Error("Too few operands for operator %s", op->symbol);
				break;
			}

			switch (op->numOperands)
			{
			case 2:
				rhOperand = operands.top();
				operands.pop();
			case 1:
				lhOperand = operands.top();
				operands.pop();
			}

			ScriptToken* opResult = op->Evaluate(lhOperand, rhOperand, this);
			delete lhOperand;
			delete rhOperand;
			delete curToken;
			curToken = NULL;

			if (!opResult)
			{
				Error("Operator %s failed to evaluate to a valid result", op->symbol);
				break;
			}

			operands.push(opResult);
		}
	}

	// adjust opcode offset ptr (important for recursive calls to Evaluate()
	*m_opcodeOffsetPtr = m_data - m_scriptData;

	if (operands.size() != 1)		// should have one operand remaining - result of expression
	{
		Error("An expression failed to evaluate to a valid result");
		while (operands.size())
		{
			delete operands.top();
			operands.pop();
		}
		return NULL;
	}
	//TODO port xNVSE code for printing errors
	return operands.top();
}

#endif