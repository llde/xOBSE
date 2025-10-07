#include <set>
#include "Hooks_SaveLoad.h"
#include "obse_common/SafeWrite.h"
#include "GameAPI.h"
#include "NiNodes.h"
#include "Serialization.h"
#include "GameTasks.h"
#include "PluginManager.h"
#include "EventManager.h"
#include "Tasks.h"
#include "GameObjects.h"
#include "Script.h"
#include <obse/GameData.h>

UInt32 g_gameLoaded = 0;

static const UInt32	kLoadGamePatchAddr =	0x00466995;
static const UInt32	kLoadGameRetnAddr =		0x0046699A;

#define	kLoadGameEBPOffset					0x40

static const UInt32	kSaveGamePatchAddr =	0x004657D6;
static const UInt32	kSaveGameRetnAddr =		0x004657DB;

static const UInt32	kDeleteGamePatchAddr =	0x004534A6;

static const UInt32	kRenameGamePatchAddr =	0x0045F5C0;

static const UInt32 kPreLoadPatchAddr =		0x00465BDD;
static const UInt32 kPreLoadRetnAddr =		0x00465BE3;

static const UInt32 kPostLoadPatchAddr =	0x00466AAA;
static const UInt32 kPostLoadRetnAddr =		0x00466AAF;
static const UInt32 kPostLoadGameFinishedAddr = 0x00466AC3;	// exit point for TESSaveLoadGame::LoadGame()

static const UInt32	kScript_RunAddr =		0x004FBE00;	// entry to Script::Run()

static const UInt8 Script_RunPatchInstructions[] =
{
	0xB0, 0x01,				// mov al, 1
	0xC2, 0x10, 0x00		// retn 0x10
};

static const UInt8 Script_RunOverwrittenInstructions[] =
{
	0x56, 0x8B, 0xF1, 0x8B, 0x4C
};


/*
	When loading a saved game, scripts attached to references are executed as soon as the refr
	is loaded. This can cause CTD in mods which use token scripts as ref vars may point to unloaded references.
	It can also cause errors with array/string variable access as the variable data has not yet been loaded
	From obse 0018 through 0019 beta 2, script execution was disabled during Load to prevent these problems,
	but some mods rely on execution during load in order to function properly.
	0019 beta 3 deferred execution of refr scripts until after Load had completed, but this leaves a chance that array
	or string variables are not initialized when the script runs, or that scripts don't run in the order in which they
	were invoked.

	0019 beta 4 changes Loadgame hooks as follows:
	-Preload: Load array and string vars from co-save, keep a backup of the previous vars. Script execution not disabled.
		^ vanilla CTD bug with token scripts returns, worth addressing separately
	-FinishLoad: If LoadGame() returns false (failed to load game), restore the backup of the previous array/string var maps.
		Otherwise erase them. Dispatch message to plugins to indicate whether or not LoadGame() succeeded
*/

void __stdcall DoPreLoadGame(BSFile* file)
{
	Serialization::HandlePreloadGame(file->m_path);
}

static BSFile* s_curLoadingFile = NULL;
static __declspec(naked) void PreLoadGameHook(void) noexcept
{
	static IOManager* IOManager = *g_ioManager;

	__asm {
		pushad
		mov [s_curLoadingFile], esi
		push	esi
		call	DoPreLoadGame
		popad

		mov		edx, IOManager
		jmp		[kPreLoadRetnAddr]
	}
}

static void __stdcall DispatchLoadGameEventToScripts(BSFile* file)
{
	if (file)
		EventManager::HandleOBSEMessage(OBSEMessagingInterface::kMessage_LoadGame, (void*)file->m_path);
}

static __declspec(naked) void PostLoadGameHook(void)
{
	__asm {
		pushad

		mov		eax, [s_curLoadingFile]
		push	eax
		call	DispatchLoadGameEventToScripts

		popad
		
		pop ecx
		pop edi
		pop esi
		pop ebp
		pop ebx
		jmp		[kPostLoadRetnAddr]
	}
}

static void __stdcall DoFinishLoadGame(bool bLoadedSuccessfully)
{
	DEBUG_PRINT("LoadGame() %s", bLoadedSuccessfully ? "succeeded" : "failed");
	Serialization::HandlePostLoadGame(bLoadedSuccessfully);
	EventManager::HandleOBSEMessage(OBSEMessagingInterface::kMessage_PostLoadGame, (void*)bLoadedSuccessfully);
}

static __declspec(naked) void FinishLoadGameHook(void)
{
	__asm {
		pushad

		movzx eax, al
		push eax		// bool retn value from LoadGame()
		call DoFinishLoadGame

		popad
		retn 0xC
	}
}

// stdcall to make it easier to work with in asm
static void __stdcall DoLoadGameHook(BSFile * file)
{
	g_gameLoaded = 1;

	_MESSAGE("DoLoadGameHook: %s", file->m_path);

	Serialization::HandleLoadGame(file->m_path);
}

static __declspec(naked) void LoadGameHook(void)
{
	__asm
	{
		pushad
		push		esi		// esi = BSFile *
		call		DoLoadGameHook
		popad

		// overwritten code
		mov			ecx, [ebp + kLoadGameEBPOffset]
		test		ecx, ecx
		jmp			[kLoadGameRetnAddr]
	}
}

static void __stdcall DoSaveGameHook(BSFile * file)
{
	_MESSAGE("DoSaveGameHook: %s", file->m_path);

	Serialization::HandleSaveGame(file->m_path);
}

static __declspec(naked) void SaveGameHook(void)
{
	__asm
	{
		pushad
		push		esi		// esi = BSFile *
		call		DoSaveGameHook
		popad

		// overwritten code
		mov			ecx, [ebp + kLoadGameEBPOffset]
		test		ecx, ecx	// not all versions do exactly this, but they are all equivalent
		jmp			[kSaveGameRetnAddr]
	}
}

// overwriting a call to DeleteFileA
static void __stdcall DeleteGameHook(const char * path)
{
	_MESSAGE("DeleteGameHook: %s", path);

	Serialization::HandleDeleteGame(path);

	// overwritten code
	DeleteFile(path);
}

static void RenameGameHook(const char * oldPath, const char * newPath)
{
	_MESSAGE("RenameGameHook: %s -> %s", oldPath, newPath);

	Serialization::HandleRenameGame(oldPath, newPath);

	rename(oldPath, newPath);
}

void Hook_SaveLoad_Init(void)
{
	WriteRelJump(kLoadGamePatchAddr, (UInt32)&LoadGameHook);
	WriteRelJump(kSaveGamePatchAddr, (UInt32)&SaveGameHook);
	WriteRelCall(kDeleteGamePatchAddr, (UInt32)&DeleteGameHook);
	SafeWrite8(kDeleteGamePatchAddr + 5, 0x90);	// nop out the 6th byte of the patched instruction
	WriteRelCall(kRenameGamePatchAddr, (UInt32)&RenameGameHook);
	WriteRelJump(kPreLoadPatchAddr, (UInt32)&PreLoadGameHook);
	WriteRelJump(kPostLoadPatchAddr, (UInt32)&PostLoadGameHook);
	WriteRelJump(kPostLoadGameFinishedAddr, (UInt32)&FinishLoadGameHook);
}
