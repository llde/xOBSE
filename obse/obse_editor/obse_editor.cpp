#include "obse/obse.h"
#include "obse/CommandTable.h"
#include "EditorHookWindow.h"
#include "common/IThread.h"
#include "obse_common/SafeWrite.h"
#include "richedit.h"
#include "obse/Hooks_Script.h"
#include "obse/Settings.h"
#include "obse/Utilities.h"

IDebugLog	gLog("obse_editor.log");

IThread	hookWindowThread;

static void HookWindowThread(void * param)
{
	_MESSAGE("create hook window");
	EditorHookWindow	* window = new EditorHookWindow;

	window->PumpEvents();

	_MESSAGE("hook window closed");

	delete window;
}

static void CreateHookWindow(void)
{
	hookWindowThread.Start(HookWindowThread);
}

static HANDLE	fontHandle;
static LOGFONT	fontInfo;
static bool		userSetFont = false;

static void DoModScriptWindow(HWND wnd)
{
	SendMessage(wnd, EM_EXLIMITTEXT, 0, 0x00FFFFFF);

	bool	setFont = false;

	if(GetAsyncKeyState(VK_F11))
	{
		LOGFONT		newFontInfo = fontInfo;
		CHOOSEFONT	chooseInfo = { sizeof(chooseInfo) };

		chooseInfo.lpLogFont = &newFontInfo;
		chooseInfo.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_SCREENFONTS;

		if(ChooseFont(&chooseInfo))
		{
			HANDLE	newFont = CreateFontIndirect(&newFontInfo);
			if(newFont)
			{
				DeleteObject(fontHandle);

				fontInfo = newFontInfo;
				fontHandle = newFont;
			}
			else
				_WARNING("couldn't create font");
		}

		setFont = true;
	}

	if(GetAsyncKeyState(VK_F12) || setFont || userSetFont)
	{
		userSetFont = true;
		SendMessage(wnd, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
		SendMessage(wnd, WM_SETFONT, (WPARAM)fontHandle, 1);

		UInt32	tabStopSize = 16;
		SendMessage(wnd, EM_SETTABSTOPS, 1, (LPARAM)&tabStopSize);	// one tab stop every 16 dialog units
	}
}

static __declspec(naked) void ModScriptWindow() {
	__asm {
		pushad
		push eax
		call DoModScriptWindow
		add esp, 4
		popad
		pop ecx
		push 0x030000
		push 0x00
		push ecx
		ret
	}
}

static UInt32 pModScriptWindow = (UInt32)&ModScriptWindow;

// copied from a call to ChooseFont
static const LOGFONT kLucidaConsole9 =
{
	-12,	// height
	0,		// width
	0,		// escapement
	0,		// orientation
	400,	// weight
	0,		// italic
	0,		// underline
	0,		// strikeout
	0,		// charset
	3,		// out precision
	2,		// clip precision
	1,		// quality
	49,		// pitch and family
	"Lucida Console"
};

static void FixEditorFont(void)
{
	// try something nice, otherwise fall back on SYSTEM_FIXED_FONT
	fontHandle = CreateFontIndirect(&kLucidaConsole9);
	if(fontHandle)
	{
		fontInfo = kLucidaConsole9;
	}
	else
	{
		fontHandle = GetStockObject(SYSTEM_FIXED_FONT);
		GetObject(fontHandle, sizeof(fontInfo), &fontInfo);
	}

	UInt32	basePatchAddr = 0x004FEAB9;

	SafeWrite8(basePatchAddr + 0,	0xFF);
	SafeWrite8(basePatchAddr + 1,	0x15);
	SafeWrite32(basePatchAddr + 2,	(UInt32)&pModScriptWindow);
	SafeWrite8(basePatchAddr + 6,	0x90);
}

static void FixErrorReportBug(void)
{
	// bethesda passes strings containing user input to printf-like functions
	// this causes crashes when the user input contains tokens printf is interested in
	// so we fix it

	// move the entire block of code before the call to printf down, add a new argument pointing to "%s"

	const UInt32	kBlockMoveDelta =	5;

	const UInt32	kBlockMoveSrc =		0x00500001;
	const UInt32	kBlockMoveDst =		kBlockMoveSrc + kBlockMoveDelta;
	const UInt32	kBlockMoveSize =	0x00500035 - kBlockMoveSrc;
	const UInt32	kFormatStrPos =		0x0092BBE4;	// "%s"
	const UInt32	kStackFixupPos =	0x0B + 2;
	const UInt32	kPCRelFixups[] =	{ 0x00 + 1, 0x06 + 1, 0x28 + 1 };


	UInt8	tempBuf[kBlockMoveSize];

	memcpy(tempBuf, (void *)kBlockMoveSrc, kBlockMoveSize);

	// jumps/calls are pc-relative, we're moving code so fix them up
	*((UInt32 *)&tempBuf[kPCRelFixups[0]]) -= kBlockMoveDelta;
	*((UInt32 *)&tempBuf[kPCRelFixups[1]]) -= kBlockMoveDelta;
	*((UInt32 *)&tempBuf[kPCRelFixups[2]]) -= kBlockMoveDelta;

	// added a new arg, so we need to clean it off the stack
	tempBuf[kStackFixupPos] += 4;

	SafeWriteBuf(kBlockMoveDst, tempBuf, kBlockMoveSize);

	SafeWrite8(kBlockMoveSrc + 0, 0x68);	// push "%s"
	SafeWrite32(kBlockMoveSrc + 1, kFormatStrPos);
}

static void CreateTokenTypedefs(void)
{
	char** tokenAlias_Float = (char**)0x009F10AC;	//reserved for array variable
	char** tokenAlias_Long	= (char**)0x009F1084;	//string variable

	*tokenAlias_Long = "string_var";
	*tokenAlias_Float = "array_var";
}

static void PatchConditionalCommands(void)
{
	// editor will not accept commands outside of the vanilla opcode range
	// for use as conditionals in dialog/packages/quests/etc
	// nop out a conditional branch to fix
	SafeWrite16(0x00457DD0, 0x9090);

}

static UInt32 ScriptIsAlpha_PatchAddr = 0x00500ACA;
static UInt32 IsAlphaCallAddr = 0x00890E4C;


static void __declspec(naked) ScriptIsAlphaHook(void)
{
	__asm
	{
		mov al, byte ptr [esp+4]	// arg to isalpha()

		cmp al, '['					// brackets used for array indexing
		jz AcceptChar
		cmp al, ']'
		jz AcceptChar
		cmp al, '$'					// $ used as prefix for string variables
		jz AcceptChar

		jmp IsAlphaCallAddr			// else use default behavior

	AcceptChar:
		mov eax, 1
		retn
	}
}

void PatchIsAlpha()
{
	WriteRelCall(ScriptIsAlpha_PatchAddr, (UInt32)ScriptIsAlphaHook);
}

/*
static const UInt32 kErrorReturn = 0x004FFFEC;


static  UInt32 WINAPI MessageBoxHook(HWND hWnd, const char* text, const char* caption, UInt32 flags) {
	bool isWarning = flags & MSG_WARNING;
	bool isSuppressed = flags & MSG_SUPPRESSED;
	if (isSuppressed) return 0;
	if (isWarning) returnCode = MessageBoxA(hWnd, text, "Script Warning", MB_ICONWARNING | MB_OKCANCEL);
	else returnCode = MessageBoxA(hWnd, text, caption, MB_ICONERROR | MB_OK);
	return returnCode;
}


static const UInt32 kFormHeapFree = 0x00401EA0;
static const UInt32 kRet = 0x0050000C;
static void __declspec(naked) HeapFreeHook() {
//arguments are already pushed here
	__asm {
		call kFormHeapFree
		mov eax, [returnCode]
		jmp kRet
	}
}
UInt32 flags = 0;
static UInt32 fixupsReturn = 0x004FFFE4 + 5;
void __declspec(naked) FixupFlags() {

	_asm {
		pushad
	}
	flags = 0; //Start clean
	if (isWarning) flags |= MSG_WARNING;
	if (block && isWarning) flags |= MSG_SUPPRESSED;  //TODO make suppressions and warnings a lookup
	__asm {
		popad
		//ebx are flags
		mov  ebx, [flags]
		jmp [fixupsReturn]
	}
}

void PatchScriptErrorReporting() {
	WriteRelCall(0x004FFFFA, (UInt32)MessageBoxHook);
	SafeWrite8(0x004FFFFA +5 , 0x90);
	WriteRelCall(0x00500001, (UInt32)StubMsgBox);  //Stub to avoid nopping a lot of stuffs
	WriteRelJump(0x00500007, (UInt32)HeapFreeHook);  //Hook to ensure we got the proper EAX
	SafeWrite8(0x004FFFEA, 0x90);
	SafeWrite8(0x004FFFEA + 1, 0x90);
	SafeWrite8(0x004FFFE2, 0x90);
	SafeWrite8(0x004FFFE2 + 1, 0x90);
	SafeWrite8(0x004FFFDA, 0x90);
	SafeWrite8(0x004FFFDA + 1, 0x90); 
	WriteRelJump(0x004FFFE4, (UInt32)FixupFlags);
	SafeWrite8(0x004FFFE4 + 5, 0x90);
}
*/

static UInt32 returnCode = 0;

int __cdecl StubMsgBox(const char* format, ...) {
	returnCode = 0;
	HWND parentHwnd = *(HWND*)0x00A0AF20;
	char* buf = (char*)calloc(1024, sizeof(char));
	va_list va;
	va_start(va, format);
	_vsprintf_p(buf, 1024, format, va);
	UInt32 flags = 0;
	const char* caption = nullptr;
	char* hold = nullptr;
	for (size_t i = 0; i < 1024; i++) {
		if (buf[i] == '\n') {
			hold = buf + i + 1;
			break;
		}
	}
	UInt32 len = strlen(hold);
	if (len > 9 && 0 == strncmp(hold, "[SUPPRESSED]", 12)) return returnCode;
	else if (len > 9 && 0 == strncmp(hold, "[WARNING]", 9)) {
		flags = MB_OKCANCEL | MB_ICONWARNING;
		caption = "Script Warning";
		memmove(hold, hold + 9, len - 9);
		hold[len - 9] = '\0';
	}
	else {
		flags |= MB_ICONERROR | MB_OK;
		caption = "Script Error";
	}
	returnCode = MessageBoxA(parentHwnd, buf, caption, flags);
	free(buf);
	return returnCode;
}

static const UInt32 kFormHeapFree = 0x00401EA0;
static const UInt32 kRet = 0x0050000C + 5;
static void __declspec(naked) HeapFreeHook() {
	//arguments are already pushed here
	__asm {
		call kFormHeapFree
		mov eax, [returnCode]
		jmp kRet
	}
}

void CSHookFixups() {
//TODO assume CSE is installed for now
	WriteRelCall(0x00500006, (UInt32)StubMsgBox);  //Save ecx and do stuff
	WriteRelJump(0x0050000C, (UInt32)HeapFreeHook);  //Hook to ensure we got the proper EAX
}

extern "C" {
void OBSE_Initialize(void)
{
	__try {
		_MESSAGE("OBSE editor: initialize (version = %d.%d %08X)", OBSE_VERSION_INTEGER, OBSE_VERSION_INTEGER_MINOR, CS_VERSION);
		InitializeSettings();
#if 0
		while(!IsDebuggerPresent())
		{
			Sleep(10);
		}

		Sleep(1000 * 2);
#endif

		FixEditorFont();
		FixErrorReportBug();
//		PatchScriptErrorReporting();
	//TODO get if CSE is installed, if not handle ourself
		CSHookFixups();

		CommandTable::Init();  


		// Add "string_var" as alias for "long"
		CreateTokenTypedefs();

		// Disable check on vanilla opcode range for use of commands as conditionals
		PatchConditionalCommands();

		// Allow use of special characters '$', '[', and ']' in string params to script commands
		PatchIsAlpha();

		// hook the start of script compilation
		Hook_Compiler_Init();

		// this is very very helpful for debugging and analyzing data formats
		// but completely useless for end users, so it's #if 0'd out
#if 0
		CreateHookWindow();
#endif
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		_ERROR("exception");
	}
}

void OBSE_DeInitialize(void)
{
	DeleteObject(fontHandle);

	_MESSAGE("OBSE: deinitialize");
}
};

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
	)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		OBSE_Initialize();
		break;

	case DLL_PROCESS_DETACH:
		OBSE_DeInitialize();
		break;
	};

	return TRUE;
}