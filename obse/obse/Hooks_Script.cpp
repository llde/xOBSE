#include "obse_common/SafeWrite.h"
#include "Hooks_Script.h"
#include "GameForms.h"
#include "Script.h"
#include "ScriptUtils.h"
#include "CommandTable.h"
#include "GameData.h"
#include "PluginManager.h"

// a size of ~1KB should be enough for a single line of code
char s_ExpressionParserAltBuffer[0x500] = {0};

#if OBLIVION

#include "StringVar.h"

	const UInt32 ExtractStringPatchAddr = 0x004FB1EB;
	const UInt32 ExtractStringRetnAddr = 0x004FB1F4;

	static const UInt32 kResolveRefVarPatchAddr		= 0x004FA9F1;
	static const UInt32 kResolveNumericVarPatchAddr = 0x004FA11E;
	static const UInt32 kEndOfLineCheckPatchAddr	= 0;	// not yet supported at run-time

	// incremented on each recursive call to Activate, limit of 5 hard-coded
	static UInt32* kActivationRecurseDepth = (UInt32*)0x00B35F00;

	static const UInt32 kExpressionParserBufferOverflowHookAddr_1 = 0x004F40F9;
	static const UInt32 kExpressionParserBufferOverflowRetnAddr_1 = 0x004F40FE;

	static const UInt32 kExpressionParserBufferOverflowHookAddr_2 = 0x004F42A4;
	static const UInt32 kExpressionParserBufferOverflowRetnAddr_2 = 0x004F42A9;

	static const UInt32 kExtractArgsNumArgsHookAddr = 0x004FB4B2;
	static const UInt32 kExtractArgsEndProcAddr = 0x004FB34B;		// returns true from ExtractArgs()
	static const UInt32 kExtractArgsReadNumArgsPatchAddr = 0x004FAE9A;	// movzx edx, word ptr (num args)
	static const UInt32 kExtractArgsNoArgsPatchAddr = 0x004FAEBA;			// jle kExtractArgsEndProcAddr (if num args == 0)

	static const UInt32 kScriptRunner_RunHookAddr = 0x0051737F;
	static const UInt32 kScriptRunner_RunRetnAddr = kScriptRunner_RunHookAddr + 5;
	static const UInt32 kScriptRunner_RunCallAddr = 0x005792E0;			// overwritten call
	static const UInt32 kScriptRunner_RunEndProcAddr = 0x00517637;		// retn 0x20


static void __stdcall DoExtractString(char* scriptData, UInt32 dataLen, char* dest, ScriptEventList* eventList)
{
	// copy the string
	memcpy(dest, scriptData, dataLen);
	dest[dataLen] = 0;

	if (dataLen && dest[0] == '$' && eventList && eventList->m_script)	// variable name
	{
		// local var or quest var?
		std::string varname(dest+1);
		std::string::size_type dotPos = varname.find('.');
		if (dotPos != std::string::npos) {
			// quest var, look up the quest and its event list
			TESQuest* quest = (*g_dataHandler)->GetQuestByEditorName(varname.substr(0, dotPos).c_str(), dotPos);
			if (quest && quest->scriptEventList) {
				eventList = quest->scriptEventList;
				varname.erase(0, dotPos+1);
			}
		}

		Script::VariableInfo* varInfo = NULL;
		varInfo = eventList->m_script->GetVariableByName(varname.c_str());
		if (varInfo)
		{
			ScriptEventList::Var* var;
			var = eventList->GetVariable(varInfo->idx);
			if (var)
			{
				StringVar* strVar;
				strVar = g_StringMap.Get(var->data);
				if (strVar)
					if (strVar->GetLength() < 0x100)		// replace string with contents of string var
						strcpy_s(dest, strVar->GetLength() + 1, strVar->GetCString());
			}
		}
	}			// "%e" becomes an empty string
	else if (dataLen == 2 && dest[0] == '%' && toupper(dest[1]) == 'E')
		dest[0] = 0;
}

static __declspec(naked) void ExtractStringHook(void)
{
	// This hooks immediately before a call to memcpy().
	// Original code copies a string literal from script data to buffer
	// If string is of format $localVariableName, replace literal with contents of string var

	static char* scriptData;	// pointer to beginning of string in script data

	__asm {
		// Grab the args to memcpy()
		mov scriptData, eax
		mov eax, [esp+0x50]		// ScriptEventList
		pushad

		push eax
		push edi				// destination buffer
		push ebp				// data len
		mov eax, scriptData
		push eax
		call DoExtractString

		popad
		mov eax, scriptData
		jmp [ExtractStringRetnAddr]
	}
}

static __declspec(naked) void ExpressionParserBufferOverflowHook_1(void)
{
	__asm {
		lea	eax, s_ExpressionParserAltBuffer		// swap buffers
		push eax
		jmp	[kExpressionParserBufferOverflowRetnAddr_1]
	}
}

static __declspec(naked) void ExpressionParserBufferOverflowHook_2(void)
{
	__asm {
		push ebp
		lea edx, s_ExpressionParserAltBuffer
		jmp	[kExpressionParserBufferOverflowRetnAddr_2]
	}
}

void Hook_Script_Init()
{
	WriteRelJump(ExtractStringPatchAddr, (UInt32)&ExtractStringHook);

	// patch the "apple bug"
	// game caches information about the most recently retrieved RefVariable for the current executing script
	// if same refIdx requested twice in a row returns previously returned ref without
	// bothering to check if form stored in ref var has changed
	// this fixes it by overwriting a conditional jump with an unconditional one
	SafeWrite8(kResolveRefVarPatchAddr, 0xEB);

	// game also caches information about the most recently retrieved local numeric variable for
	// currently executing script. Causes issues with function scripts. As above, overwrite conditional jump with unconditional
	SafeWrite8(kResolveNumericVarPatchAddr, 0xEB);

	// hook code in the vanilla expression parser's subroutine to fix the buffer overflow
	WriteRelJump(kExpressionParserBufferOverflowHookAddr_1, (UInt32)&ExpressionParserBufferOverflowHook_1);
	WriteRelJump(kExpressionParserBufferOverflowHookAddr_2, (UInt32)&ExpressionParserBufferOverflowHook_2);

	// hook ExtractArgs() to handle commands normally compiled with Cmd_Default_Parse which were instead compiled with Cmd_Expression_Parse
	ExtractArgsOverride::Init_Hooks();
}

void ResetActivationRecurseDepth()
{
	// reset to circumvent the hard-coded limit
	*kActivationRecurseDepth = 0;
}

#else	// CS-stuff

static const UInt32 kEndOfLineCheckPatchAddr = 0x00501E22;
static const UInt32 kCopyStringArgHookAddr		 = 0x00501A8A;

static const UInt32 kBeginScriptCompilePatchAddr = 0x005034A9;	// calls CompileScript()
static const UInt32 kBeginScriptCompileCallAddr  = 0x00503330;	// bool __fastcall CompileScript(unk, unk, Script*, ScriptBuffer*)
static const UInt32 kBeginScriptCompileRetnAddr	 = 0x005034AE;

static const UInt32 kExpressionParserBufferOverflowHookAddr_1 = 0x004F9712;
static const UInt32 kExpressionParserBufferOverflowRetnAddr_1 = 0x004F9717;

static const UInt32 kExpressionParserBufferOverflowHookAddr_2 = 0x004F9863;
static const UInt32 kExpressionParserBufferOverflowRetnAddr_2 = 0x004F986A;

static const UInt32 kWarnForDeprecatedCommandsHook = 0x00503119;
static const UInt32 kWarnForDeprecatedCommandsReturn = 0x0050311E;


static __declspec(naked) void ExpressionParserBufferOverflowHook_1(void)
{
	__asm {
		lea	edx, s_ExpressionParserAltBuffer
		push edx
		jmp	[kExpressionParserBufferOverflowRetnAddr_1]
	}
}

static __declspec(naked) void ExpressionParserBufferOverflowHook_2(void)
{
	__asm {
		lea	edx, s_ExpressionParserAltBuffer
		mov	byte ptr [esi], 0x20
		jmp	[kExpressionParserBufferOverflowRetnAddr_2]
	}
}

#endif	// OBLIVION

// Patch compiler check on end of line when calling commands from within other commands
// TODO: implement for run-time compiler
void PatchEndOfLineCheck(bool bDisableCheck)
{
	if (bDisableCheck)
		SafeWrite8(kEndOfLineCheckPatchAddr, 0xEB);		// unconditional (short) jump
	else
		SafeWrite8(kEndOfLineCheckPatchAddr, 0x73);		// conditional jnb (short)
}

static bool s_bParsingExpression = false;

bool ParsingExpression()
{
	return s_bParsingExpression;
}

bool ParseNestedFunction(CommandInfo* cmd, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	// disable check for end of line
	PatchEndOfLineCheck(true);

	s_bParsingExpression = true;
	bool bParsed = cmd->parse(cmd->numParams, cmd->params, lineBuf, scriptBuf);
	s_bParsingExpression = false;

	// re-enable EOL check
	PatchEndOfLineCheck(false);

	return bParsed;
}

static std::stack<UInt8*> s_loopStartOffsets;

void RegisterLoopStart(UInt8* offsetPtr)
{
	s_loopStartOffsets.push(offsetPtr);
}

bool HandleLoopEnd(UInt32 offsetToEnd)
{
	if (!s_loopStartOffsets.size())
		return false;

	UInt8* startPtr = s_loopStartOffsets.top();
	s_loopStartOffsets.pop();

	*((UInt32*)startPtr) = offsetToEnd;
	return true;
}

#if !OBLIVION

namespace CompilerOverride {
	static Mode s_currentMode = kOverride_BlockType;
	static const UInt16 kOpcode_Begin = 0x0010;
	static const UInt16 kOpcode_Dot = 0x001C;

	static bool __stdcall DoCmdDefaultParseHook(UInt32 numParams, ParamInfo* params, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
	{
		// record the mode so ExtractArgs hook knows what to expect
		lineBuf->Write16(s_currentMode);

		// parse args
		ExpressionParser parser(scriptBuf, lineBuf);
		bool bResult = parser.ParseArgs(params, numParams, false);
		return bResult;
	}

	static __declspec(naked) void Hook_Cmd_Default_Parse(void)
	{
		static UInt32 numParams;
		static ParamInfo* params;
		static ScriptLineBuffer* lineBuf;
		static ScriptBuffer* scriptBuf;
		static UInt8 result;

		__asm {
			// grab args
			mov	eax, [esp+4]
			mov	[numParams], eax
			mov	eax, [esp+8]
			mov	[params], eax
			mov	eax, [esp+0xC]
			mov [lineBuf], eax
			mov eax, [esp+0x10]
			mov [scriptBuf], eax

			// call override parse routine
			pushad
			push scriptBuf
			push lineBuf
			push params
			push numParams
			call DoCmdDefaultParseHook

			// store result
			mov	 [result], al

			// clean up
			popad

			// return result
			xor eax, eax
			mov al, [result]
			retn
		}
	}

	void __stdcall ToggleOverride(bool bOverride)
	{
		static const UInt32 patchLoc = 0x00500FF0;	// editor default parse routine

		// ToggleOverride() only gets invoked when we parse a begin or end statement, so set mode accordingly
		s_currentMode = kOverride_BlockType;

		// overwritten instructions
		static const UInt8 s_patchedInstructions[5] = { 0x81, 0xEC, 0x30, 0x02, 0x00 };
		if (bOverride) {
			WriteRelJump(patchLoc, (UInt32)&Hook_Cmd_Default_Parse);
		}
		else {
			for (UInt32 i = 0; i < sizeof(s_patchedInstructions); i++) {
				SafeWrite8(patchLoc+i, s_patchedInstructions[i]);
			}
		}
	}

	static const UInt32 stricmpAddr = 0x0088CFAE;			// doesn't clean stack
	static const UInt32 stricmpPatchAddr_1 = 0x005029DC;	// stricmp(cmdInfo->longName, blockName)
	static const UInt32 stricmpPatchAddr_2 = 0x005029F4;	// stricmp(cmdInfo->altName, blockName)
	// stores offset into ScriptBuffer's data buffer at which to store the jump offset for the current block
	static const UInt32* kBeginBlockDataOffsetPtr = (const UInt32*)0x00A11038;

	// target of an overwritten jump after parsing of block args
	// copies block args to ScriptBuffer after doing some bounds-checking
	static const UInt32 kCopyBlockArgsAddr = 0x005031A9;

	// address of an overwritten call to a void function(void) which does nothing.
	// after ScriptLineBuffer data has been copied to ScriptBuffer and CompileLine() is about to return true
	static const UInt32 nullCallAddr = 0x0050327C;

	// address at which the block len is calculated when compiling 'end' statement
	static const UInt32 storeBlockLenHookAddr = 0x00502C2B;
	static const UInt32 storeBlockLenRetnAddr = 0x00502C31;



	static bool s_overridden;	// is true if compiler override in effect

	bool IsActive()
	{
		return s_overridden;
	}

	static __declspec(naked) void CompareBlockName(void)
	{
		// edx: token
		// ecx: CommandInfo longName or altName

		__asm {
			push edx	// we may be incrementing edx here, so preserve its original value
			push eax

			// first, toggle override off if it's already activated
			test [s_overridden], 1
			jz TestForUnderscore
			pushad
			push 0
			call ToggleOverride
			popad

	TestForUnderscore:
			// if block name preceded by '_', enable compiler override
			mov al, byte ptr [edx]
			cmp al, '_'
			setz al
			mov [s_overridden], al
			test al, al
			jz DoStricmp
			// skip the '@' character
			add edx, 1

	DoStricmp:
			// do the comparison
			pop eax
			push edx
			push ecx
			call [stricmpAddr]
			add esp, 8
			pop edx

			// a match?
			test eax, eax
			jnz Done

			// a match. override requested?
			test [s_overridden], 1
			jz Done

			// toggle the override
			pushad
			push 1
			call ToggleOverride
			popad

		Done:
			// return the result of stricmp(). caller will clean stack
			retn
		}
	}

	static void __stdcall ProcessBeginOrEnd(ScriptBuffer* buf, UInt32 opcode)
	{
		ASSERT(opcode == 0x10 || opcode == 0x11);

		UInt32 maxScriptLength = IsCseLoaded() ? 0x8000 : 0x4000;	// default length of 0x4000 is doubled when running the CSE plugin

		// make sure we've got enough room in the buffer
		if (buf->dataOffset + 4 >= maxScriptLength)
			g_ErrOut.Show("Error: Max script length exceeded. Please edit and recompile.", buf);
		else {
			const char* cmdName = "@PushExecutionContext";
			SInt32 offset = 0;

			if (opcode == 0x11) {
				// 'end'. Need to rearrange so we push context before the 'end' statement
				cmdName = "@PopExecutionContext";
				offset = -4;

				// move 'end' state forward by 4 bytes (i.e. at current offset)
				*((UInt32*)(buf->scriptData+buf->dataOffset)) = 0x00000011;
			}

			CommandInfo* cmdInfo = g_scriptCommands.GetByName(cmdName);
			ASSERT(cmdInfo != NULL);

			// write a call to our cmd
			*((UInt16*)(buf->scriptData + buf->dataOffset + offset)) = cmdInfo->opcode;
			buf->dataOffset += sizeof(UInt16);

			// write length of params
			*((UInt16*)(buf->scriptData + buf->dataOffset + offset)) = 0;
			buf->dataOffset += sizeof(UInt16);
		}
	}

	static __declspec(naked) void OnCompileSuccessHook(void)
	{
		// esi: ScriptLineBuffer
		// edi: ScriptBuffer
		// eax: volatile
		__asm {
			// is override in effect?
			test [s_overridden], 1
			jz Done

			// have we just parsed a begin or end statement?
			movzx eax, word ptr [esi+0x410]		// cmd opcode for this line
			cmp eax, 0x10						// 0x10 == 'begin'
			jz Begin
			cmp eax, 0x11						// 0x11 == 'end'
			jz End
			jmp Done							// not a block statement

		Begin:
			// commands following start of block should have access to script context
			mov	[s_currentMode], kOverride_Command
			jmp Process

		End:
			// expect a new block or end of script, no execution context available
			mov [s_currentMode], kOverride_BlockType

		Process:
			// got a begin or end statement, handle it
			pushad
			push eax		// opcode
			push edi		// ScriptBuffer
			call ProcessBeginOrEnd
			popad

		Done:
			retn
		}
	}

	static __declspec(naked) void StoreBlockLenHook(void)
	{
		__asm {
			// overwritten code
			mov ecx, [eax]
			sub edx, ecx

			// override in effect?
			test [s_overridden], 1
			jz Done

			// add 4 bytes to block len to account for 'push context' cmd
			add edx, 4

	Done:
			// (overwritten) store block len
			mov [eax], edx
			jmp [storeBlockLenRetnAddr]
		}
	}

	void InitHooks()
	{
		// overwrite calls to compare block name when parsing 'begin'
		WriteRelCall(stricmpPatchAddr_1, (UInt32)&CompareBlockName);
		WriteRelCall(stricmpPatchAddr_2, (UInt32)&CompareBlockName);

		// overwrite a call to a null sub just before returning true from CompileScriptLine()
		WriteRelCall(nullCallAddr, (UInt32)&OnCompileSuccessHook);

		// hook code that records the jump offset for the current block
		WriteRelJump(storeBlockLenHookAddr, (UInt32)&StoreBlockLenHook);
	}

	bool Cmd_Plugin_Default_Parse(UInt32 numParams, ParamInfo* params, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
	{
		bool bOverride = IsActive();
		if (bOverride) {
			ToggleOverride(false);
		}

		bool result = Cmd_Default_Parse(numParams, params, lineBuf, scriptBuf);

		if (bOverride) {
			ToggleOverride(true);
		}

		return result;
	}
}

bool __stdcall HandleBeginCompile(ScriptBuffer* buf)
{
	// empty out the loop stack
	while (s_loopStartOffsets.size())
		s_loopStartOffsets.pop();

	// Preprocess the script:
	//  - check for usage of array variables in Set statements (disallowed)
	//  - check loop structure integrity
	//  - check for use of ResetAllVariables on scripts containing string/array vars

	bool bResult = PrecompileScript(buf);
	if (bResult) {
		PluginManager::Dispatch_Message(0, OBSEMessagingInterface::kMessage_Precompile, buf, sizeof(buf), NULL);
	}

	return bResult;
}

static __declspec(naked) void CompileScriptHook(void)
{
	static bool precompileResult;

	__asm
	{
		mov		eax,	[esp+4]					// grab the second arg (ScriptBuffer*)
		pushad
		push	eax
		call	HandleBeginCompile				// Precompile
		mov		[precompileResult],	al			// save result
		popad
		call	[kBeginScriptCompileCallAddr]	// let the compiler take over
		test	al, al
		jz		EndHook							// return false if CompileScript() returned false
		mov		al,	[precompileResult]			// else return result of Precompile
	EndHook:
		// there's a small possibility that the compiler override is still in effect here (e.g. scripter forgot an 'end')
		// so make sure there's no chance it remains in effect, otherwise potentially could affect result script compilation
		pushad
		push 0
		call CompilerOverride::ToggleOverride
		popad
		jmp		[kBeginScriptCompileRetnAddr]	// bye
	}
}

// replace special characters ("%q" -> '"', "%r" -> '\n')
UInt32 __stdcall CopyStringArg(char* dest, const char* src, UInt32 len, ScriptLineBuffer* lineBuf)
{
	if (!src || !len || !dest)
		return len;

	std::string str(src);
	UInt32 pos = 0;

	while ((pos = str.find('%', pos)) != -1 && pos < str.length() - 1)
	{
		char toInsert = 0;
		switch (str[pos + 1])
		{
		case '%':
			pos += 2;
			continue;
		case 'r':
		case 'R':
			toInsert = '\n';
			break;
		case 'q':
		case 'Q':
			toInsert = '"';
			break;
		default:
			pos += 1;
			continue;
		}

		str.insert(pos, 1, toInsert);	// insert char at current pos
		str.erase(pos + 1, 2);			// erase format specifier
		pos += 1;
	}

	// copy the string to script data
	memcpy(dest, str.c_str(), str.length());

	// write length of string
	lineBuf->dataOffset -= 2;
	lineBuf->Write16(str.length());

	return str.length();
}

// major code differences between CS versions here so hooks differ significantly
static __declspec(naked) void __cdecl CopyStringArgHook(void)
{
	// overwrite call to memcpy()

	// On entry:
	//	eax: dest buffer
	//	edx: src string
	//	edi: string len
	//  esi: ScriptLineBuffer* (must be preserved)
	// edi must be updated to reflect length of modified string (added to dataOffset on return)

	__asm
	{
		push	esi

		push	esi
		push	edi
		push	edx
		push	eax
		call	CopyStringArg

		mov		edi, eax
		pop		esi

		retn
	}

}
static void WarnDeprecatedCommand(CommandInfo* info, ScriptBuffer* buffer){
	if (info->flags & CommandInfo_Deprecated)
		CompilerMessages::Show(CompilerMessages::kWarning_DeprecatedCommand, buffer, info->longName);
}

static __declspec(naked) void __cdecl WarnForDeprecatedCommands(void){
	//ebp is the CommandInfo*
	__asm {
		push edi
		push ebp
		call WarnDeprecatedCommand
		add esp, 8

		mov al, [ebp+0x10]
		test al, al
		jmp [kWarnForDeprecatedCommandsReturn]
	}
}


void Hook_Compiler_Init()
{
	// hook beginning of compilation process
	WriteRelJump(kBeginScriptCompilePatchAddr, (UInt32)&CompileScriptHook);

	// hook copying of string argument to compiled data
	// lets us modify the string before its copied
	WriteRelCall(kCopyStringArgHookAddr, (UInt32)&CopyStringArgHook);
	WriteRelJump(kWarnForDeprecatedCommandsHook, (UInt32)&WarnForDeprecatedCommands);

	// hook code in the vanilla expression parser's subroutine to fix the buffer overflow
	WriteRelJump(kExpressionParserBufferOverflowHookAddr_1, (UInt32)&ExpressionParserBufferOverflowHook_1);
	WriteRelJump(kExpressionParserBufferOverflowHookAddr_2, (UInt32)&ExpressionParserBufferOverflowHook_2);

	CompilerOverride::InitHooks();
}

#else			// run-time

#include "common/ICriticalSection.h"

namespace ExtractArgsOverride {
	static std::vector<ExecutingScriptContext*>	s_stack;
	static ICriticalSection s_critSection;

	ExecutingScriptContext::ExecutingScriptContext(TESObjectREFR* thisObj, TESObjectREFR* container, UInt16 opcode)
	{
		callingRef = thisObj;
		containerRef = container;
		cmdOpcode = opcode;
		threadID = GetCurrentThreadId();
	}

	ExecutingScriptContext* PushContext(TESObjectREFR* thisObj, TESObjectREFR* containerObj, UInt8* scriptData, UInt32* offsetPtr)
	{
		UInt16* opcodePtr = (UInt16*)(scriptData + *offsetPtr - 4);
		ExecutingScriptContext* context = new ExecutingScriptContext(thisObj, containerObj, *opcodePtr);

		s_critSection.Enter();
		s_stack.push_back(context);
		s_critSection.Leave();

		return context;
	}

	bool PopContext()
	{
		DWORD curThreadID = GetCurrentThreadId();
		bool popped = false;

		s_critSection.Enter();
		for (std::vector<ExecutingScriptContext*>::reverse_iterator iter = s_stack.rbegin(); iter != s_stack.rend(); ++iter) {
			ExecutingScriptContext* context = *iter;
			if (context->threadID == curThreadID) {
				delete context;
				s_stack.erase((++iter).base());
				popped = true;
				break;
			}
		}
		s_critSection.Leave();

		return popped;
	}

	ExecutingScriptContext* GetCurrentContext()
	{
		ExecutingScriptContext* context = NULL;
		DWORD curThreadID = GetCurrentThreadId();

		s_critSection.Enter();
		for (SInt32 i = s_stack.size() - 1; i >= 0; i--) {
			if (s_stack[i]->threadID == curThreadID) {
				context = s_stack[i];
				break;
			}
		}
		s_critSection.Leave();

		return context;
	}

	static bool __stdcall DoExtractExtendedArgs(CompilerOverride::Mode mode, UInt32 _esp, va_list varArgs)
	{
		static const UInt8 stackOffset = 0x2C;

		// grab the args to ExtractArgs()
		_esp += stackOffset;
		ParamInfo* paramInfo = *(ParamInfo**)(_esp+0x00);
		UInt8* scriptData = *(UInt8**)(_esp+0x04);
		UInt32* opcodeOffsetPtr = *(UInt32**)(_esp+0x08);
		Script* scriptObj = *(Script**)(_esp+0x14);
		ScriptEventList* eventList = *(ScriptEventList**)(_esp+0x18);

		// extract
		return ExtractArgs(paramInfo, scriptData, scriptObj, eventList, opcodeOffsetPtr, varArgs, true, mode);
	}

	bool ExtractArgs(ParamInfo* paramInfo, UInt8* scriptData, Script* scriptObj, ScriptEventList* eventList, UInt32* opcodeOffsetPtr,
		va_list varArgs, bool bConvertTESForms, UInt16 numArgs)
	{
		// get thisObj and containerObj from currently executing script context, if available
		TESObjectREFR* thisObj = NULL;
		TESObjectREFR* containingObj = NULL;
		if (numArgs == CompilerOverride::kOverride_Command) {
			// context should be available
			ExecutingScriptContext* context = GetCurrentContext();
			if (!context) {
				_MESSAGE("ERROR: Could not get execution context in DoExtractExtendedArgs()");
				return false;
			}
			else {
				thisObj = context->callingRef;
				containingObj = context->containerRef;
			}
		}
	//	_MESSAGE("OVERRIDE  %08X   %08X", scriptObj->refID, thisObj ? thisObj->refID : 0);
		// extract
		ExpressionEvaluator eval(paramInfo, scriptData, thisObj, (UInt32)containingObj, scriptObj, eventList, NULL, opcodeOffsetPtr);
		if (eval.ExtractDefaultArgs(varArgs, bConvertTESForms)) {
			return true;
		}
		else {
			DEBUG_PRINT("ExtractArgsOverride::ExtractArgs returns false");
			return false;
		}
	}

	bool ExtractFormattedString(ParamInfo* paramInfo, UInt8* scriptData, Script* scriptObj, ScriptEventList* eventList,
		UInt32* opcodeOffsetPtr, va_list varArgs, UInt32 fmtStringPos, char* fmtStringOut, UInt32 maxParams)
	{
		ExecutingScriptContext* context = GetCurrentContext();
		if (!context) {
			g_ErrOut.Show("ERROR: Could not get execution context in ExtractFormattedString()");
			return false;
		}

		ExpressionEvaluator eval(paramInfo, scriptData, context->callingRef, (UInt32)context->containerRef, scriptObj, eventList,
			NULL, opcodeOffsetPtr);
		return eval.ExtractFormatStringArgs(varArgs, fmtStringPos, fmtStringOut, maxParams);
	}

	static __declspec(naked) void ExtractExtendedArgsHook(void)
	{
		static UInt32	_esp;
		static UInt32	bResult;

		__asm {
			mov		[_esp],	esp
			pushad

			push	esi						// va_list
			mov		eax,	[_esp]
			push	eax
			movsx	eax,	dx				// num args
			push	eax
			call	DoExtractExtendedArgs
			mov		[bResult], eax

			popad
			mov		eax, [bResult]
			jmp		[kExtractArgsEndProcAddr]
		}
	}
	// these hooks don't get invoked unless they are required (no run-time penalty if compiler override is not used)
	static void Init_Hooks()
	{
		// change movzx to movsx to read # of args as a signed value
		// < 0 indicates compiled with Cmd_Expression_Parse rather than Cmd_Default_Parse
		SafeWrite16(kExtractArgsReadNumArgsPatchAddr, 0xBF0F);

		// if numArgs == 0, return true; else execute our hook
		WriteRelJnz(kExtractArgsNumArgsHookAddr, (UInt32)&ExtractExtendedArgsHook);	// numArgs < 0, execute hook
		WriteRelJump(kExtractArgsNumArgsHookAddr + 6, kExtractArgsEndProcAddr);		// numArgs == 0, return true

		// if numArgs <= 0, jump to our hook test instead of end proc
		WriteRelJle(kExtractArgsNoArgsPatchAddr, kExtractArgsNumArgsHookAddr);
	}
}	// namespace

#endif			// run-time