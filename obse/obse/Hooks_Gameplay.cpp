#include <set>

#include "Hooks_Gameplay.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "obse_common/SafeWrite.h"
#include "Hooks_Memory.h"
#include "Serialization.h"
#include "GameAPI.h"
#include "GameTasks.h"
#include <share.h>
#include <set>
#include "StringVar.h"
#include "ArrayVar.h"
#include "PluginManager.h"
#include "GameOSDepend.h"
#include "GameMenus.h"
#include "InventoryReference.h"
#include "Tasks.h"
#include "EventManager.h"
#include "Hooks_SaveLoad.h"
#include "GameActorValues.h"
#include "ThreadLocal.h"

static void HandleMainLoopHook(void);

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

static const UInt32 kMainLoopHookPatchAddr = 0x0040EC8E;
static const UInt32 kMainLoopHookRetnAddr = 0x0040EC94;

static const UInt32	kRefIDBugfixRetnAddr = 0x00443E6D;

static __declspec(naked) void RefIDBugfix(void)
{
	__asm
	{
		mov	edx, [ebx+0x08B8]		// fetch the last-generated refid
		inc	edx						// increment it
		or	edx, 0xFF000000			// make sure we wrap from FFFFFFFF -> FF000000 instead of 00000000
		jmp	[kRefIDBugfixRetnAddr]	// done
	}
}

static void InstallRefIDBugfix(void)
{
	WriteRelJump(0x00443E66, (UInt32)&RefIDBugfix);
}

static __declspec(naked) void MainLoopHook(void)
{
	__asm
	{
		pushad
		call	HandleMainLoopHook
		popad
		mov		ecx, [edx + 0x280]
		jmp		[kMainLoopHookRetnAddr]
	}
}

static const UInt32	kNewGamePatchAddr = 0x005A7727;

static const UInt32 QUIMsgPatchAddr = 0x0056DF90;
static const UInt8  QUIMsgData = 0x51;	//code overwritten by retn
static const UInt32 QUIMsg_2PatchAddr = 0x0056E0A0;
static const UInt8 QUIMsg_2Data = 0xD9;

static const UInt32	kOriginalLoadCreatedObjectsAddr = 0x00461350;
static const UInt32	kLoadCreatedObjectsHookAddr = 0x0046347B;

static const UInt32 kEnchantItemHookPatchAddr = 0x0059562C;
static const UInt32 kEnchantItemHookRetnAddr =  0x00595638;

static const UInt32 kCreateSpellHookPatchAddr = 0x005C875A;
static const UInt32 kCreateSpellHookRetnAddr =  0x005C8766;

static const UInt32 kCreatePotionHookPatchAddr = 0x00587BD8;
static const UInt32 kCreatePotionHookRetnAddr  = 0x00587BDD;
static const UInt32 kCreatePotionHookCallAddr  = 0x00443EE0;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2

static const UInt32 kMainLoopHookPatchAddr = 0x0040F16D;
static const UInt32 kMainLoopHookRetnAddr = 0x0040F173;

static const UInt32	kRefIDBugfixRetnAddr = 0x00448F76;

static __declspec(naked) void RefIDBugfix(void)
{
	__asm
	{
		inc	dword ptr [ebx+0x8C0]					// increment the current refid
		or	dword ptr [ebx+0x8C0], 0xFF000000		// make sure we wrap from FFFFFFFF -> FF000000 instead of 00000000
		jmp	[kRefIDBugfixRetnAddr]					// done
	}
}

static void InstallRefIDBugfix(void)
{
	WriteRelJump(0x00448F70, (UInt32)&RefIDBugfix);
}

static __declspec(naked) void MainLoopHook(void)
{
	__asm
	{
		pushad
		call	HandleMainLoopHook
		popad
		mov		eax, [edx + 0x280]			// ### check this
		jmp		[kMainLoopHookRetnAddr]
	}
}

static const UInt32	kNewGamePatchAddr = 0x005B5D0D;

static const UInt32 QUIMsgPatchAddr = 0x0057ABC0;
static const UInt8  QUIMsgData = 0x51;	//code overwritten by retn
static const UInt32 QUIMsg_2PatchAddr = 0x0057ACD0;
static const UInt8 QUIMsg_2Data = 0xD9;

static const UInt32	kOriginalLoadCreatedObjectsAddr = 0x00461350;
static const UInt32	kLoadCreatedObjectsHookAddr = 0x0046347B;

static const UInt32 kEnchantItemHookPatchAddr = 0x005A2DCB;
static const UInt32 kEnchantItemHookRetnAddr =  0x005A2DD7;

static const UInt32 kCreateSpellHookPatchAddr = 0x005D7CFE;
static const UInt32 kCreateSpellHookRetnAddr =  0x005D7D0A;

static const UInt32 kCreatePotionHookPatchAddr = 0x00594C5A;
static const UInt32 kCreatePotionHookRetnAddr  = 0x00594C5F;
static const UInt32 kCreatePotionHookCallAddr  = 0x0044A930;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

static const UInt32 kMainLoopHookPatchAddr = 0x0040F19D;
static const UInt32 kMainLoopHookRetnAddr = 0x0040F1A3;

static __declspec(naked) void MainLoopHook(void)
{
	__asm
	{
		pushad
		call	HandleMainLoopHook
		popad
		mov		eax, [edx + 0x280]
		jmp		[kMainLoopHookRetnAddr]
	}
}

static const UInt32	kNewGamePatchAddr = 0x005B5EFD;

static const UInt32	QUIMsgPatchAddr = 0x0057ACC0;
static const UInt8	QUIMsgData = 0x51;	//code overwritten by retn
static const UInt32	QUIMsg_2PatchAddr = 0x0057ADD0;
static const UInt8	QUIMsg_2Data = 0xD9;

static const UInt32	kOriginalLoadCreatedObjectsAddr = 0x00461310;
static const UInt32	kLoadCreatedObjectsHookAddr = 0x0046344B;

static const UInt32 kEnchantItemHookPatchAddr = 0x005A2F1B;
static const UInt32 kEnchantItemHookRetnAddr =  0x005A2F27;

static const UInt32 kCreateSpellHookPatchAddr = 0x005D7F2E;
static const UInt32 kCreateSpellHookRetnAddr =  0x005D7F3A;

static const UInt32 kCreatePotionHookPatchAddr = 0x00594CFA;
static const UInt32 kCreatePotionHookRetnAddr  = 0x00594CFF;
static const UInt32 kCreatePotionHookCallAddr  = 0x0044A750;	// compares passed AlchemyItem with created potions

static const UInt32 kDataHandler_GetCreatedPotion = kCreatePotionHookCallAddr;

static const UInt32 kPlayerBuyHookAddr = 0x0059AE3D;
static const UInt32 kPlayerBuyRetnAddr = 0x0059AE42;

static const UInt32 kPlayerSellHookAddr = 0x0059AB1E;
static const UInt32 kPlayerSellRetnAddr = 0x0059AB23;

static const UInt32* kBuySellQuantity = (const UInt32*)0x00B13E94;

static const UInt32 kSigilStoneCreatePatchAddr = 0x005D4F52;
static const UInt32 kSigilStoneCreateRetnAddr = 0x005D4F57;

static const UInt32 kChangeCellHookPatchAddr = 0x0066765A;
static const UInt32 kChangeCellHookCallAddr = 0x004C97F0;

// boolean, used by ExtraDataList::IsExtraDefaultForContainer() to determine if ExtraOwnership should be treated
// as 'non-default' for an inventory object. Is 0 in vanilla, set to 1 to make ownership NOT treated as default
static const UInt32 kExtraOwnershipDefaultSetting = 0x0041FE59;
static const UInt32 kExtraOwnershipDefaultSetting2 = 0x0041FE0D;

static const UInt32 kConsoleManager_PrintAddr = 0x00585C90;

static const UInt32 kContainerMenuDanglingPointerPatchAddr = 0x00597D26;
static const UInt32 kContainerMenuSecondDanglingPointerPatchAddr = 0x00599B41;
static const UInt32 kContainerMenuSecondDanglingPointerRetnAddr = 0x00599B47;

#else
#error unsupported oblivion version
#endif

// this stores a pointer to the most recently enchanted item at the moment it is created
TESForm* g_LastEnchantedItem = 0;
static const UInt32 kEnchantEventID = EventManager::kEventID_OnEnchant;	// workaround for use in inline asm

static __declspec(naked) void EnchantItemHook(void)
{
	__asm
	{
		pushad								// save the registers
		mov		g_LastEnchantedItem, edi	// checked: all versions use edi
	}

	AddFormToCreatedBaseObjectsList(*g_createdBaseObjList, g_LastEnchantedItem);

	__asm
	{
		// invoke event handler
		mov edi, [g_LastEnchantedItem]
		push 0
		push edi
		push [kEnchantEventID]
		call EventManager::HandleEvent

		popad								// restore the registers
		jmp [kEnchantItemHookRetnAddr]
	}
}

// this stores a pointer to the most recently created spell
TESForm* g_LastCreatedSpell = 0;
static const UInt32 kSpellEventID = EventManager::kEventID_OnCreateSpell;

static __declspec(naked) void CreateSpellHook(void)
{
	__asm
	{
		pushad
		mov		g_LastCreatedSpell, edx		// all versions use edx
	}

	AddFormToCreatedBaseObjectsList(*g_createdBaseObjList, g_LastCreatedSpell);

	__asm
	{
		// invoke event handler
		mov edx, [g_LastCreatedSpell]
		push 0
		push edx
		push kSpellEventID
		call EventManager::HandleEvent

		popad
		jmp	[kCreateSpellHookRetnAddr]
	}
}

// Pointers to the most recently created potion, and the most recently created unique potion
TESForm* g_LastCreatedPotion = 0;
TESForm* g_LastUniqueCreatedPotion = 0;
static const UInt32 kOnCreatePotionEventID = EventManager::kEventID_OnCreatePotion;

static __declspec(naked) void CreatePotionHook(void)
{
	__asm
	{
			call [kCreatePotionHookCallAddr]
			pushad
			xor ecx, ecx
			test eax, eax		// is AlchemyItem* if potion has been previously created
			jnz EndHook

			// new base potion object
			mov ecx, 1
			mov eax, [edi+0x94]			// AlchemyMenu::potion
			mov g_LastUniqueCreatedPotion, eax
	EndHook:
			mov g_LastCreatedPotion, eax

			// invoke event handler
			push ecx						// 1 if created potion is a new base form, 0 if already existed
			push eax						// potion
			push kOnCreatePotionEventID
			call EventManager::HandleEvent

			popad
			jmp [kCreatePotionHookRetnAddr]
	}
}

// set of references' refIDs flagged for deletion by DeleteReference command
// using refID rather than ref pointer to avoid potential (if unlikely) issues with unloaded forms
static std::set<UInt32> deletedREFRs;

void QueueRefForDeletion(TESObjectREFR* refr)
{
	if (refr->flags & 0x4000)	// don't axe a temporary object, though it should never reach this point anyway.
		return;

	deletedREFRs.insert(refr->refID);
}

static void DoDeferredDelete()
{
	IOManager* ioMan = IOManager::GetSingleton();
	if (!ioMan)
		return;

	for (std::set<UInt32>::iterator iter = deletedREFRs.begin(); iter != deletedREFRs.end(); )
	{
		TESForm* refForm = LookupFormByID(*iter);
		if (refForm)
		{
			TESObjectREFR* refr = OBLIVION_CAST(refForm, TESForm, TESObjectREFR);
			if (!ioMan->IsInQueue(refr))		// only delete if no tasks are queued for reference
			{
				refr->Destroy(false);
				deletedREFRs.erase(iter++);
			}
			else
				++iter;
		}
		else			// refForm not found, remove from set
			deletedREFRs.erase(iter++);
	}
}

static void HandleNewGameHook(void)
{
	_MESSAGE("HandleNewGameHook");

	// event sent from here because Serialization::HandleNewGame() can be invoked when loading a save with no
	// associated co-save - which should not be reported as a New Game event
	EventManager::HandleEvent(EventManager::kEventID_OnNewGame, NULL, NULL);

	// Updated v0020: Make GetGameLoaded return true whenever user starts a new game
	// (previously returned true on new game only if user started new game immediately after launching Oblivion)
	g_gameLoaded = 1;

	Serialization::HandleNewGame();
}

DWORD g_mainThreadID = 0;

static void HandleMainLoopHook(void)
{
	static bool s_recordedMainThreadID = false;
	if (!s_recordedMainThreadID) {
		s_recordedMainThreadID = true;
		g_mainThreadID = GetCurrentThreadId();
	}

	// Hook_Memory_CheckAllocs(); not currently used
	// DoDeferredEnable(); not currently used

	// clean up any temp arrays/strings
	g_ArrayMap.Clean();
	g_StringMap.Clean();

	// delete any refs queued for deletion by DeleteReference command
	// ###TODO: make this a Task
	DoDeferredDelete();

	// if any temporary references to inventory objects exist, clean them up
	if (InventoryReference::HasData())
		InventoryReference::Clean();

	// currently unused
	//if (TaskManager::HasTasks())
	//	TaskManager::Run();

	// Tick event manager
	EventManager::Tick();
}

// workaround for inability to take address of __thiscall functions
static __declspec(naked) void _TESSaveLoadGame_LoadCreatedObjectsHook(void)
{
	__asm
	{
		jmp	TESSaveLoadGame::LoadCreatedObjectsHook
	}
}

class CallPostFixup
{
public:
	bool	Accept(UInt32 formID)
	{
		TESForm	* form = LookupFormByID(formID);

		if(form)
		{
			// this could be done with dynamic_cast, but this is faster
			switch(form->typeID)
			{
				case 0x25:	// TESLevCreature
				{
					TESLevCreature	* _form = (TESLevCreature *)form;

					FixupModIDs(&_form->leveledList);
				}
				break;

				case 0x2B:	// TESLevItem
				{
					TESLevItem	* _form = (TESLevItem *)form;

					FixupModIDs(&_form->leveledList);
				}
				break;

				case 0x40:	// TESLevSpell
				{
					TESLevSpell	* _form = (TESLevSpell *)form;

					FixupModIDs(&_form->leveledList);
				}
				break;
			}

			form->DoPostFixup();
		}

		return true;
	}

private:
	class FixupModID
	{
	public:
		bool	Accept(TESLeveledList::ListData * data)
		{
			UInt32	newRefID;

			if(!Serialization::ResolveRefID(data->formID, &newRefID))
				newRefID = 0;	// invalid refid

			data->formID = newRefID;

			return true;
		}
	};

	void FixupModIDs(TESLeveledList * levList)
	{
		LeveledListVisitor	visitor(&levList->list);

		visitor.Visit(FixupModID());
	}
};

void TESSaveLoadGame::LoadCreatedObjectsHook(UInt32 unk0)
{
	// run the original code
	ThisStdCall(kOriginalLoadCreatedObjectsAddr, this, unk0);

	// iterate the linked list, call DoPostFixup
	CreatedObjectVisitor	visitor(&createdObjectList);

	visitor.Visit(CallPostFixup());
}

bool TESSaveLoadGame::LoadGame(const char* filename)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	return (bool)ThisStdCall(0x0045EA60, this, 0, filename, 0);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	return (bool)ThisStdCall(0x00465760, this, 0, filename, 0);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	bool bFileFound = ThisStdCall(0x00465860, this, 0, filename, 0) ? true : false;
	return bFileFound;
#else
#error unsupported Oblivion version
#endif
}

UInt32 TESSaveLoadGame::ResetObject(TESForm* object, UInt32 changeFlags, UInt32 unk2)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x000045BDE0, this, object, changeFlags, unk2);
#else
#error unsupported oblivion version
#endif
}

#define DEBUG_PRINT_CHANNEL(idx)								\
																\
static UInt32 __stdcall DebugPrint##idx(const char * str)		\
{																\
	static FILE	* dst = NULL;									\
	if(!dst) dst = _fsopen("obse_debugprint" #idx ".log", "w", _SH_DENYWR);	\
	if(dst) fputs(str, dst);									\
	return 0;													\
}

DEBUG_PRINT_CHANNEL(0)	// used to exit
DEBUG_PRINT_CHANNEL(1)	// ignored
DEBUG_PRINT_CHANNEL(2)	// ignored
// 3 - program flow
DEBUG_PRINT_CHANNEL(4)	// ignored
// 5 - stack trace?
DEBUG_PRINT_CHANNEL(6)	// ignored
// 7 - ingame
// 8 - ingame

// these are all ignored in-game
static void Hook_DebugPrint(void)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	const UInt32	kMessageHandlerVtblBase = 0x00A0A468;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	const UInt32	kMessageHandlerVtblBase = 0x00A3DB18;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	const UInt32	kMessageHandlerVtblBase = 0x00A3DA08;
#else
#error unsupported oblivion version
#endif

	SafeWrite32(kMessageHandlerVtblBase + (0 * 4), (UInt32)DebugPrint0);
	SafeWrite32(kMessageHandlerVtblBase + (1 * 4), (UInt32)DebugPrint1);
	SafeWrite32(kMessageHandlerVtblBase + (2 * 4), (UInt32)DebugPrint2);
	SafeWrite32(kMessageHandlerVtblBase + (4 * 4), (UInt32)DebugPrint4);
	SafeWrite32(kMessageHandlerVtblBase + (6 * 4), (UInt32)DebugPrint6);
}

//toggle messages on or off in upper left  corner of screen
//meant to be toggled off immediately before calling a spam-generating function and toggled back on again
static ICriticalSection s_UICritSection;
static SInt32 s_UIToggleCount = 0;	// how many times have msgs been toggled off
void ToggleUIMessages(bool enableSpam)
{
	ScopedLock lock(s_UICritSection);
	if (!enableSpam) {
		s_UIToggleCount++;
		ASSERT_STR(s_UIToggleCount > 0, "Overflow in ToggleUIMessages");
	}
	else {
		ASSERT_STR(s_UIToggleCount > 0, "Underflow in ToggleUIMessages");
		s_UIToggleCount--;
	}

	if (s_UIToggleCount == 0)
	{
		// restore overwritten code to enable UI msgs again
		SafeWrite8(QUIMsgPatchAddr, QUIMsgData);
		SafeWrite8(QUIMsg_2PatchAddr, QUIMsg_2Data);
	}
	else if (s_UIToggleCount == 1) {
		// if count > 1 we've already patched
		SafeWrite8(QUIMsgPatchAddr, 0xC3);		//write immediate retn at function entry
		SafeWrite8(QUIMsg_2PatchAddr, 0xC3);
	}
}

void ToggleConsoleOutput(bool enable)
{
	static bool s_bEnabled = true;
	if (enable != s_bEnabled) {
		s_bEnabled = enable;
		if (enable) {
			// original code: 'push 0xFFFFFFFF; push ...'
			SafeWrite8(kConsoleManager_PrintAddr, 0x6A);
			SafeWrite8(kConsoleManager_PrintAddr+1, 0xFF);
			SafeWrite8(kConsoleManager_PrintAddr+2, 0x68);
		}
		else {
			// 'retn 8'
			SafeWrite8(kConsoleManager_PrintAddr, 0xC2);
			SafeWrite8(kConsoleManager_PrintAddr+1, 0x08);
			SafeWrite8(kConsoleManager_PrintAddr+2, 0x00);
		}
	}
}

static __declspec(naked) bool DummyKeyboardInputHandler(char inputChar)
{
	__asm {
		mov al, 1;
		retn 4;
	}
}

//Menu::HandleKeyboardInput() takes an ASCII code and returns true only if it handles it as a keyboard shortcut
//This temporarily overwrites that virtual function with a dummy handler which always returns true
//Tricks the calling function into thinking we've handled the input, thus disabling all keyboard shortcuts for that menu
//NOTE: Only supports toggling for one menu at a time
//		Menu* param not required to enable
bool ToggleMenuShortcutKeys(bool bEnable, Menu* menu)
{
	static UInt32* handlerAddr = 0;			//vtbl offset of handler function
	static UInt32 originalHandler = 0;		//address of "real" handler function

	if (bEnable)
	{
		if (handlerAddr && originalHandler)
		{
			SafeWrite32((UInt32)handlerAddr, originalHandler);	//restore original function
			handlerAddr = 0;
			originalHandler = 0;
			return true;
		}
		else
			return false;
	}
	else
	{
		if (menu && !handlerAddr && !originalHandler)
		{
			UInt32* vtblAddr = *((UInt32**)menu);
			handlerAddr = (vtblAddr + 0x0C);
			originalHandler = *(handlerAddr);
			SafeWrite32((UInt32)handlerAddr, (UInt32)DummyKeyboardInputHandler);
			return true;
		}
		else
			return false;
	}
}

// Hook GetIsRace function to allow scripters to define aliases for mod-added races
// Only apply hook if SetRaceAlias command is actually called
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
static const UInt32 kGetIsRacePatchAddr = 0x004EB460;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
static const UInt32 kGetIsRacePatchAddr = 0x004F6FD0;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static const UInt32 kGetIsRacePatchAddr = 0x004F6F40;
#else
#error unsupported oblivion version
#endif

// key is refID of aliased race, data is set of refID's of races using that race as an alias
typedef std::set<UInt32> RefIDSet;
std::map<UInt32, RefIDSet> s_raceAliases;

// this replaces the smaller version of the GetIsRace command with args already extracted
// checks if character's race is found in set of aliases for aliasRace
bool Cmd_GetIsRace_2_Execute(Character* character, TESRace* aliasRace, UInt32 unk2, double* result)
{
	*result = 0;

	if (!character || !aliasRace)
		return true;

	TESNPC* baseForm = (TESNPC*)(character->GetBaseForm());
	if (!baseForm || baseForm->typeID != kFormType_NPC)
		return true;

	TESRace* charRace = baseForm->race.race;
	if (!charRace)
		return true;
	else if (charRace == aliasRace) {
		// is actually of the specified race, no need to check aliases
		*result = 1;
		return true;
	}

	if (s_raceAliases.empty() || s_raceAliases.find(aliasRace->refID) == s_raceAliases.end())
		*result = (charRace == aliasRace) ? 1 : 0;	// no alias defined for this race
	else
	{
		RefIDSet* aliasSet = &s_raceAliases[aliasRace->refID];

		if (aliasSet->find(charRace->refID) != aliasSet->end())
			*result = 1;	// race is defined as alias for aliasRace
	}

	if (IsConsoleMode())
		Console_Print("GetIsRace >> %.2f", *result);

	return true;
}

void SetRaceAlias(TESRace* race, TESRace* alias, bool bEnableAlias)
{
	static bool bPatchApplied = false;

	// On first call to this function, overwrite original GetIsRace with our version
	if (!bPatchApplied)
	{
		WriteRelJump(kGetIsRacePatchAddr, (UInt32)Cmd_GetIsRace_2_Execute);
		bPatchApplied = true;
	}

	if (bEnableAlias)
	{
		RefIDSet* aliasSet = &s_raceAliases[alias->refID];
		aliasSet->insert(race->refID);
	}
	else
	{
		if (s_raceAliases.find(alias->refID) != s_raceAliases.end())	// do any aliases exist for this race?
		{
			RefIDSet* aliasSet = &s_raceAliases[alias->refID];
			aliasSet->erase(race->refID);
			if (aliasSet->empty())		// if no more aliases defined, get rid of the set
				s_raceAliases.erase(alias->refID);
		}
	}
}

// SpellEffectiveness hook: Allows scripters to modify the player's spell effectiveness
// Overwrites GetSpellEffectiveness virtual func in MagicCaster class
// Calculation remains the same (performed by game code); the modifier is just added to the result
static bool bSpellEffectivenessPatchApplied = false;
static double s_playerSpellEffectivenessModifier = 0;		// modifier to add to base value
static double s_recordedPlayerSpellEffectivenessModifier = 0;

static UInt32 s_GetPlayerSpellEffectivenessAddr = 0;		// address of original GetSpellEffectiveness func

bool DoGetSpellEffectivenessHook();

float __stdcall GetPlayerSpellEffectiveness(UInt32 arg0, UInt32 arg1)
{
	float baseEffectiveness = 0;
	PlayerCharacter* pc = *g_thePlayer;

	MagicCaster* pcCaster = (MagicCaster*)Oblivion_DynamicCast(pc, 0, RTTI_TESObjectREFR, RTTI_MagicCaster, 0);
	if (pcCaster)
	{
		// let the game calculate the base value
		ThisStdCall(s_GetPlayerSpellEffectivenessAddr, pcCaster, arg0, arg1);

		// return value is float so pop it
		__asm {
			fstp	[baseEffectiveness]
		}

		DEBUG_PRINT("baseEffectiveness = %.2f, modded = %.2f", baseEffectiveness,
					(baseEffectiveness + s_playerSpellEffectivenessModifier));
	}

	// add script modifier and return
	return baseEffectiveness + s_playerSpellEffectivenessModifier;
}

bool DoGetSpellEffectivenessHook()
{
	PlayerCharacter* pc = *g_thePlayer;
	if (pc)
	{
		UInt32* pcCasterVtblPtr = (UInt32*)Oblivion_DynamicCast(pc, 0, RTTI_TESObjectREFR, RTTI_MagicCaster, 0);
		if (pcCasterVtblPtr)
		{
			// Get address of vtbl pointer to original GetSpellEffectiveness()
			UInt32* originalFuncAddr = (UInt32*)((*pcCasterVtblPtr) + 0x2C);

			// Store original function address for later use
			s_GetPlayerSpellEffectivenessAddr = *originalFuncAddr;

			//Overwrite ptr to original func with address of new func
			SafeWrite32((UInt32)originalFuncAddr, (UInt32)GetPlayerSpellEffectiveness);

			bSpellEffectivenessPatchApplied = true;
		}
	}

	return bSpellEffectivenessPatchApplied;
}

void ModPlayerSpellEffectiveness(double modBy, bool recordChange)
{
	if (!bSpellEffectivenessPatchApplied)
		DoGetSpellEffectivenessHook();

	s_playerSpellEffectivenessModifier += modBy;
	if (recordChange)
		s_recordedPlayerSpellEffectivenessModifier += modBy;
}

double GetPlayerSpellEffectivenessModifier()
{
	return s_playerSpellEffectivenessModifier;
}

double GetPersistentPlayerSpellEffectivenessModifier ()
{
	return s_recordedPlayerSpellEffectivenessModifier;
}

AlchemyItem* MatchPotion(AlchemyItem* toMatch)
{
	if (!toMatch)
		return NULL;

	return (AlchemyItem*)ThisStdCall(kDataHandler_GetCreatedPotion, *g_dataHandler, toMatch);
}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32		kPlayer_GetActorValueAddr = 0x0065E030;
	static const UInt32		kPlayer_GetActorValueVtblAddr = 0x00A73C90;
	static const UInt32		kPlayer_GetActorValueRetnAddresses[] =
	{	0x005E3818,		// run speed
		0x005E39C7,		// swim speed
		0x005E3B77,		// fast swim speed
		0x005E363E,		// walk speed
	};
#else
#error unsupported oblivion version
#endif

static double s_pcSpeedModifier = 0;
static double s_recordedPCSpeedModifier = 0;

UInt32 __stdcall Player_GetActorValue(UInt32 actorVal)
{
	UInt32 retnAddr;

	// grab the address of instruction following call to this function
	__asm {
		mov		ecx, [ebp + 4]
		mov		[retnAddr], ecx
	};

	// get the actor value from the original function
	UInt32 result = ThisStdCall(kPlayer_GetActorValueAddr, *g_thePlayer, actorVal);

	// were we called from one of the movement-related functions?
	bool bApplyModifier = false;
	if (actorVal == kActorVal_Speed)
	{
		for (UInt32 i = 0; i < sizeof(kPlayer_GetActorValueRetnAddresses); i++)
			if (retnAddr == kPlayer_GetActorValueRetnAddresses[i])
			{
				bApplyModifier = true;
				break;
			}
	}

	// apply modifier if appropriate, don't return a value < 0
	if (bApplyModifier)
		result = (result + s_pcSpeedModifier > 0) ? result + s_pcSpeedModifier : 0;

	return result;
}

void ModPlayerMovementSpeed(double modBy, bool recordChange)
{
	static bool bHooked = false;
	if (!bHooked)
	{
		SafeWrite32(kPlayer_GetActorValueVtblAddr, (UInt32)&Player_GetActorValue);
		bHooked = true;
	}

	s_pcSpeedModifier += modBy;
	if (recordChange)
		s_recordedPCSpeedModifier += modBy;
}

double GetPlayerMovementSpeedModifier()
{
	return s_pcSpeedModifier;
}

double GetPersistentPlayerMovementSpeedModifier()
{
	return s_recordedPCSpeedModifier;
}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 kCreateReferenceCallAddr		= 0x0048FBC0;
	static const UInt32 kCreateDroppedReferenceHookAddr = 0x004D87C7;
	static const UInt32 kCreateDroppedReferenceRetnAddr = 0x004D87CC;
#else
#error unsupported Oblivion version
#endif

static TESForm* s_lastDroppedItem = NULL;
static TESObjectREFR* s_lastDroppedItemRef = NULL;
static UInt32 s_lastDroppedItemRefID = 0;

static void __stdcall HandleDroppedItem(TESObjectREFR* dropper)
{
	TESObjectREFR* ref = s_lastDroppedItemRef;
	if (ref)
	{
		s_lastDroppedItem = ref->baseForm;
		s_lastDroppedItemRefID = ref->refID;
		EventManager::HandleEvent(EventManager::kEventID_OnActorDrop, (void*)dropper, (void*)ref);
	}
}

static __declspec(naked) void DroppedItemHook(void)
{
	__asm
	{
		call	[kCreateReferenceCallAddr]
		pushad
		mov		[s_lastDroppedItemRef], eax

		push	esi				// actor who dropped the item
		call	HandleDroppedItem

		popad
		mov		eax, [s_lastDroppedItemRef]
		jmp		[kCreateDroppedReferenceRetnAddr]
	}
}

UInt32 GetPCLastDroppedItemRef()
{
	return s_lastDroppedItemRefID;
}

TESForm* GetPCLastDroppedItem()
{
	return s_lastDroppedItem;
}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 kExitGameFromIngameMenuPatchAddr = 0x005BDE60;
	static const UInt32 kExitGameFromIngameMenuRetnAddr  =  0x005BDE66;

	static const UInt32 kMainMenuFromIngameMenuPatchAddr = 0x005BDE23;
	static const UInt32 kMainMenuFromIngameMenuRetnAddr  = 0x005BDE29;

	static const UInt32 kExitGameFromMainMenuPatchAddr   = 0x005B5A0D;
	static const UInt32 kExitGameFromMainMenuRetnAddr    = 0x005B5A12;

	static const UInt32 kExitGameViaQQQPatchAddr		 = 0x005077F2;
	static const UInt32 kExitGameViaQQQRetnAddr			 = 0x005077F7;
#else
#error unsupported oblivion version
#endif

enum QuitGameMessage
{
	kQuit_ToMainMenu,
	kQuit_ToWindows,
	kQuit_QQQ,
};

void __stdcall SendQuitGameMessage(QuitGameMessage msg)
{
	UInt32 msgToSend = OBSEMessagingInterface::kMessage_ExitGame;
	if (msg == kQuit_ToMainMenu)
		msgToSend = OBSEMessagingInterface::kMessage_ExitToMainMenu;
	else if (msg == kQuit_QQQ)
		msgToSend = OBSEMessagingInterface::kMessage_ExitGame_Console;

	PluginManager::Dispatch_Message(0, msgToSend, NULL, 0, NULL);
	EventManager::HandleOBSEMessage(msgToSend, NULL);
}

static __declspec(naked) void ExitGameFromIngameMenuHook(void)
{
	__asm {
		pushad

		push	kQuit_ToWindows
		call	SendQuitGameMessage

		popad
		mov		edx,	[g_osGlobals]
		mov		edx,	[edx]
		jmp		[kExitGameFromIngameMenuRetnAddr]
	}
}

static __declspec(naked) void ExitGameFromMainMenuHook(void)
{
	__asm {
		pushad

		push	kQuit_ToWindows
		call	SendQuitGameMessage

		popad
		mov		eax,	[g_osGlobals]
		mov		eax,	[eax]
		jmp		[kExitGameFromMainMenuRetnAddr]
	}
}

static __declspec(naked) void ExitGameViaQQQHook(void)
{
	__asm {
		pushad

		push	kQuit_QQQ
		call	SendQuitGameMessage

		popad
		add	esp, 4
		mov	al, 1
		jmp		[kExitGameViaQQQRetnAddr]
	}
}

static __declspec(naked) void MainMenuFromIngameMenuHook(void)
{
	__asm {
		pushad

		push	kQuit_ToMainMenu
		call	SendQuitGameMessage

		popad
		mov		ecx,	[g_osGlobals]
		mov		ecx,	[ecx]
		jmp		[kMainMenuFromIngameMenuRetnAddr]
	}
}

static TESForm* s_lastTransactionItem = NULL;
static UInt32 s_lastTransactionQuantity = 0;

// set of scripts that have been informed about the most recent transaction
static std::set<UInt32>	s_transactionInformedScripts[2];

TransactionInfo s_transactionHistories[2] =
{
	{ NULL, NULL, NULL, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

void __stdcall DoBuySellHook(eTransactionType type, ContainerMenu* menu, TESForm* item, Actor* seller, UInt32 price)
{
	// these are used by GetLastTransactionItem/Quantity
	s_lastTransactionItem = item;
	s_lastTransactionQuantity = menu->GetQuantity();

	// used by more specific transaction-related commands
	TESObjectREFR* buyer = (type == kPC_Buy) ? *g_thePlayer : menu->refr;
	TransactionInfo* info = &s_transactionHistories[type];
	info->buyer = buyer;
	info->seller = seller;
	info->item = item;
	info->price = price;
	info->quantity = menu->GetQuantity();

	// a new transaction exists, so clear informed scripts
	s_transactionInformedScripts[type].clear();

	menu->selectedItemTile = NULL;
}

static __declspec(naked) void PlayerBuyHook(void)
{;
	__asm {
		pushad

		push	edi				// cost
		push	ebp				// merchant
		mov		esi, [esi+8]	// ExtraContainerChanges::EntryData::type for item being purchased
		push	esi
		push	ebx				// ContainerMenu
		push	kPC_Buy
		call	DoBuySellHook

		popad

		call	MarkBaseExtraListScriptEvent
		jmp		[kPlayerBuyRetnAddr]
	}
}

static __declspec(naked) void PlayerSellHook(void)
{
	__asm {
		pushad

		push	edi				// cost
		push	eax				// seller (player)
		mov		esi, [esi+8]
		push	esi				// item being sold
		push	ebx				// ContainerMenu
		push	kPC_Sell
		call	DoBuySellHook

		popad

		call	MarkBaseExtraListScriptEvent
		jmp		[kPlayerSellRetnAddr]
	}
}

bool GetLastTransactionInfo(TESForm** form, UInt32* quantity)
{
	if (form)
		*form = s_lastTransactionItem;
	if (quantity)
		*quantity = s_lastTransactionQuantity;
	return true;
}

const TransactionInfo* GetLastTransactionInfo(eTransactionType type, UInt32 callingScriptRefID)
{
	const TransactionInfo* info = NULL;
	std::set<UInt32>& informedScripts = s_transactionInformedScripts[type];
	if (!callingScriptRefID || informedScripts.find(callingScriptRefID) == informedScripts.end()) {
		info =  &s_transactionHistories[type];
		if (!info->quantity) {
			// indicates no transaction has yet taken place during this game session
			info = NULL;
		}
		else if (callingScriptRefID) {
			informedScripts.insert(callingScriptRefID);
		}
	}

	return info;
}

static TESForm*	s_lastUsedSigilStone = NULL;
static TESForm*			s_lastEnchantedSigilStoneItem = NULL;
static TESForm*			s_lastCreatedSigilStoneItem = NULL;

static __declspec(naked) void SigilStoneCreateHook(void)
{
	__asm {
		pushad

		mov ecx, [esi+0x28]
		mov	[s_lastUsedSigilStone], ecx
		mov	s_lastCreatedSigilStoneItem, eax
		mov	eax, [esi+0x2C]
		mov ecx, [eax+0x08]
		mov	[s_lastEnchantedSigilStoneItem], ecx

		popad

		mov	ecx, [esi+0x2C]
		mov	edi, eax
		jmp	[kSigilStoneCreateRetnAddr]
	}
}

bool GetLastSigilStoneInfo(TESForm** outStone, TESForm** outOldItem, TESForm** outCreatedItem)
{
	ASSERT(outStone && outOldItem && outCreatedItem);
	if (!s_lastUsedSigilStone)
		return false;

	*outStone = s_lastUsedSigilStone;
	*outOldItem = s_lastEnchantedSigilStoneItem;
	*outCreatedItem = s_lastCreatedSigilStoneItem;
	return true;
}

static UInt32 g_cellChanged = false;
bool GetCellChanged()
{
	bool changed = g_cellChanged ? true : false;
	g_cellChanged = 0;
	return changed;
}

static __declspec(naked) void ChangeCellHook(void)
{
	// hooks PlayerCharacter::ChangeCell()
	// at this point we know:
	//	-newCell is non-null
	//	-ebx == newCell
	// however it's possible that newCell is equal to the previous cell, so check for that

	static TESObjectCELL* s_lastCell = NULL;
	__asm {
		push eax
		mov eax, [s_lastCell]
		cmp eax, ebx			// if (newCell == s_lastCell)
		jz done
		test eax, eax			// if null, game just started - don't bother treating it as a cell change
		jz recordCell
		mov	[g_cellChanged], 1

	recordCell:
		mov	[s_lastCell], ebx

	done:
		pop	eax
		jmp	[kChangeCellHookCallAddr]
	}
}

static __declspec(naked) void Hook_ContainerMenuDanglingPointer(void)
{
	// edi = TileRect* which is about to be destroyed and its memory freed.
	// ContainerMenu::selectedItemTile points to this memory and is not reset at this time.
	// so yeah, that's a problem.

	// on entry:
	//	edi: TileRect*
	//	edx: TileRect::Destroy
	//	ebx: ContainerMenu*
	//	ecx, eax volatile

	__asm {
		// reset dangling pointer
		mov ecx, ebx	// ContainerMenu* contMenu
		add ecx, 0x3C	// contMenu->selectedItemTile
		xor eax, eax
		mov [ecx], eax	// = NULL

		// overwritten code
		push 1
		mov ecx, edi
		call edx
		jmp [kContainerMenuSecondDanglingPointerRetnAddr]
	}
}

// RemoveAllItems cmd and SendPlayerToJail function remove all items from player, but if a removed quest item was equipped and enchanted, the enchantment remains on the player
// fix by unequipping everything before removing all items
// In both cases overwrite a call to BaseExtraList::RemoveAllItems (TESObjectREFR* from, TESObjectREFR* to, UInt32 unk2, bool bRetainOwnership, UInt32 unk4)
// Could reasonably just patch that function instead, but don't want to mess with other code that may call it
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static const UInt32 kRemoveAllItems_CallAddr	= 0x00492E70;
static const UInt32 kRemoveAllItems_PatchAddr	= 0x00507578;
static const UInt32 kGotoJail_PatchAddr			= 0x00670328;
#else
#error unsupported Oblivion version
#endif

static void __stdcall DoUnequipAllItems(TESObjectREFR* refr)
{
	Actor* actor = OBLIVION_CAST (refr, TESObjectREFR, Actor);
	if (actor)
		actor->UnequipAllItems();
}

static __declspec(naked) void RemoveAllItemsHook(void)
{
	__asm {
		pushad
		push esi
		call DoUnequipAllItems
		popad
		jmp	[kRemoveAllItems_CallAddr]
	}
}

// AddSpell cmd will trigger a CTD if an effectitem-less spellitem is added to the player
// caused by the handler dereferencing a NULL effectitem ptr to get its icon path for the notification message
static const UInt32 kAddSpellPlayer_PatchAddr = 0x00514A98;
static const UInt32 kAddSpellPlayer_CallAddr = 0x004152C0;
static const UInt32 kAddSpellPlayer_RetnAddr = 0x00514A9D;
static const UInt32 kAddSpellPlayer_JumpAddr = 0x00514AA7;

static __declspec(naked) void AddSpellPlayerHook(void)
{
	__asm
	{
		call [kAddSpellPlayer_CallAddr]
		test eax, eax		// returns a EffectItem*
		jz SKIP

		jmp	[kAddSpellPlayer_RetnAddr]
	SKIP:
		jmp	[kAddSpellPlayer_JumpAddr]
	}
}

// scripted objects (containers in particular) fail to correctly activate when accessed by NPCs
// in the case of containers, there's a loss of parametric data when the Activate command is called inside a script
// so we save the parameters before activation and restore them inside the Activate command's handler
struct ReferenceActivationContext
{
	// thisObj
	TESObjectREFR*		activatedRef;
	// args
	TESObjectREFR*		activatingRef;
	UInt32				arg2;				// seen as either 0 or 1
	TESForm*			arg3;
	UInt32				arg4;				// seen as either 0 or 1, mostly the latter
};

static std::stack<ReferenceActivationContext> s_RefActivationContexts;
//###TODO is this thread-safe enough?
static ICriticalSection s_RefActivationCS;

//###HACK - call the militia!
#define DEFINE_REFHOOKFN(hookaddr, handler)									\
	void __declspec(naked) RefActivation##hookaddr##Hook(void)				\
	{																		\
		static UInt32 RetnAddr = hookaddr## + 5;							\
		{																	\
		__asm	push	ecx													\
		__asm	call	handler												\
		__asm	jmp		RetnAddr											\
		}																	\
	}
#define PATCH_REFHOOKFN(hookaddr)			WriteRelJump(hookaddr, (UInt32)RefActivation##hookaddr##Hook)

bool __stdcall DoRefActivationCallSiteHook(TESObjectREFR* thisObj, TESObjectREFR* arg1, UInt32 arg2, TESForm* arg3, UInt32 arg4)
{
	ReferenceActivationContext context = {0};
	context.activatedRef = thisObj;
	context.activatingRef = arg1;
	context.arg2 = arg2;
	context.arg3 = arg3;
	context.arg4 = arg4;

	s_RefActivationContexts.push(context);
	bool result = ThisStdCall(0x004DD260, thisObj, arg1, arg2, arg3, arg4);
	s_RefActivationContexts.pop();

	return result;
}

bool __stdcall DoRefActivationCmdHandlerHook(TESObjectREFR* thisObj, TESObjectREFR* arg1, UInt32 arg2, TESForm* arg3, UInt32 arg4)
{
	ScopedLock lock(s_RefActivationCS);

	bool result = false;

	if (s_RefActivationContexts.size())
	{
		const ReferenceActivationContext& current = s_RefActivationContexts.top();

		if (current.activatedRef == thisObj && current.activatingRef == arg1)
		{
			arg2 = current.arg2;
			arg3 = current.arg3;
			arg4 = current.arg4;
		}
	}

	result = ThisStdCall(0x004DD260, thisObj, arg1, arg2, arg3, arg4);

	return result;
}

DEFINE_REFHOOKFN(0x00507705, DoRefActivationCmdHandlerHook)						// Activate command handler

DEFINE_REFHOOKFN(0x00637C31, DoRefActivationCallSiteHook)						// called when activating a container while searching for food

#if 0
DEFINE_REFHOOKFN(0x0062EED9, DoRefActivationCallSiteHook)						// something to do with trespassing
DEFINE_REFHOOKFN(0x006319E2, DoRefActivationCallSiteHook)
DEFINE_REFHOOKFN(0x00631AE7, DoRefActivationCallSiteHook)
DEFINE_REFHOOKFN(0x00637D51, DoRefActivationCallSiteHook)						// called when handling Find packages
DEFINE_REFHOOKFN(0x0063802A, DoRefActivationCallSiteHook)
DEFINE_REFHOOKFN(0x00638286, DoRefActivationCallSiteHook)
DEFINE_REFHOOKFN(0x00645F71, DoRefActivationCallSiteHook)
DEFINE_REFHOOKFN(0x0064FD39, DoRefActivationCallSiteHook)
DEFINE_REFHOOKFN(0x006529A2, DoRefActivationCallSiteHook)
#endif

void Init_RefActivationPatch(void)
{
	PATCH_REFHOOKFN(0x00507705);

	PATCH_REFHOOKFN(0x00637C31);
#if 0
	PATCH_REFHOOKFN(0x0062EED9);
	PATCH_REFHOOKFN(0x006319E2);
	PATCH_REFHOOKFN(0x00631AE7);
	PATCH_REFHOOKFN(0x00637D51);
	PATCH_REFHOOKFN(0x0063802A);
	PATCH_REFHOOKFN(0x00638286);
	PATCH_REFHOOKFN(0x00645F71);
	PATCH_REFHOOKFN(0x0064FD39);
	PATCH_REFHOOKFN(0x006529A2);
#endif

	// calling Activate on an unscripted object prevents subsequent default activation of that object
	// we fix it be patching the handler to stop it from removing the normal activation flag from the ref's ExtraAction extradata
	WriteRelJump(0x00507713, 0x0050771C);
}

void Hook_Gameplay_Init(void)
{
	// game main loop
	// this address was chosen because it is only run when oblivion is in the foreground
	WriteRelJump(kMainLoopHookPatchAddr, (UInt32)&MainLoopHook);

	// patch enchanted cloned item check
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	SafeWrite8(0x004590ED + 1, 0x20);	// more accurate to branch to 0045DED7
	WriteRelJump(0x004591EC, 0x00459275);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	SafeWrite8(0x0045DEBD + 1, 0x20);	// more accurate to branch to 0045DED7
	WriteRelJump(0x0045DFBF, 0x0045E04A);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	SafeWrite8(0x0045DEAD + 1, 0x20);	// more accurate to branch to 0045DED7
	WriteRelJump(0x0045DFAF, 0x0045E03A);
#else
#error unsupported oblivion version
#endif

#if OBLIVION_VERSION < OBLIVION_VERSION_1_2_410
	// patch dynamic refid rollover bug
	InstallRefIDBugfix();
#endif

	WriteRelCall(kNewGamePatchAddr, (UInt32)&HandleNewGameHook);

	// objects in the 'created objects' list are not loaded in the same way as other objects
	// normally they are created, loaded from disk, then once they have all been loaded and added
	// to the form table a post-load callback is called to allow refids to be turned in to pointers
	// the 'created objects' code calls the post-load callback immediately after loading the form
	// so bugs occur when the callback expects to be able to find a cloned object that hasn't been
	// created yet
	//
	// to fix this, we hook the function loading the created objects and disable the call to the
	// post-fixup function. once the function is completed, we walk the created objects linked list
	// and call the callback ourselves

	// nop out post-load callback
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	SafeWrite8(0x0045B27F + 0, 0x90);
	SafeWrite8(0x0045B27F + 1, 0x90);
	SafeWrite8(0x0045B27F + 2, 0x90);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	SafeWrite8(0x004614F9 + 0, 0x90);
	SafeWrite8(0x004614F9 + 1, 0x90);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	SafeWrite8(0x004614B9 + 0, 0x90);
	SafeWrite8(0x004614B9 + 1, 0x90);
#else
#error unsupported oblivion version
#endif

	// hook the loader function
	WriteRelCall(kLoadCreatedObjectsHookAddr, (UInt32)&_TESSaveLoadGame_LoadCreatedObjectsHook);

	// hook EnchantmentMenu::CreateEnchantedItem() to grab a pointer to the newly enchanted item
	WriteRelJump(kEnchantItemHookPatchAddr, (UInt32)&EnchantItemHook);

	// hook SpellmakingMenu::CreateSpell() to grab pointer to the newly created spell
	WriteRelJump(kCreateSpellHookPatchAddr, (UInt32)&CreateSpellHook);

	// hook AlchemyMenu::CreatePotion() to grab pointer to newly created potion
	WriteRelJump(kCreatePotionHookPatchAddr, (UInt32)&CreatePotionHook);

	// hook code that creates a new reference to an item dropped by the player
	WriteRelJump(kCreateDroppedReferenceHookAddr, (UInt32)&DroppedItemHook);

	// hook exit to main menu or to windows
	WriteRelJump(kExitGameFromIngameMenuPatchAddr, (UInt32)&ExitGameFromIngameMenuHook);
	WriteRelJump(kExitGameFromMainMenuPatchAddr, (UInt32)&ExitGameFromMainMenuHook);
	WriteRelJump(kMainMenuFromIngameMenuPatchAddr, (UInt32)&MainMenuFromIngameMenuHook);
	WriteRelJump(kExitGameViaQQQPatchAddr, (UInt32)&ExitGameViaQQQHook);

	// hook code that executes when player confirms buy/sell in ContainerMenu
	WriteRelJump(kPlayerBuyHookAddr, (UInt32)&PlayerBuyHook);
	WriteRelJump(kPlayerSellHookAddr, (UInt32)&PlayerSellHook);

	// hook code that executes immediately after constructing a newly SigilStone-enchanted item
	WriteRelJump(kSigilStoneCreatePatchAddr, (UInt32)&SigilStoneCreateHook);

	// hook PlayerCharacter::ChangeCell() to detect cell changes
	WriteRelCall(kChangeCellHookPatchAddr, (UInt32)&ChangeCellHook);

	// Fix dangling pointer in ContainerMenu
	SafeWrite32(kContainerMenuDanglingPointerPatchAddr, 0x90909090);
	SafeWrite16(kContainerMenuDanglingPointerPatchAddr+4, 0x9090);
	// and fix another reference to the same dangling pointer elsewhere
	WriteRelJump(kContainerMenuSecondDanglingPointerPatchAddr, (UInt32)&Hook_ContainerMenuDanglingPointer);

	// patch enchantments from equipped quest items not being removed when RemoveAllItems cmd used or player sent to jail
	WriteRelCall (kRemoveAllItems_PatchAddr, (UInt32)&RemoveAllItemsHook);
	WriteRelCall (kGotoJail_PatchAddr, (UInt32)&RemoveAllItemsHook);

	// fix AddSpell command CTD
	// the CustomSpellIcons plugin fixes this bug, so we'll shut the fudge up when it's loaded
	if (g_pluginManager.LookupHandleFromName("CustomSpellIcons") == kPluginHandle_Invalid)
		WriteRelJump(kAddSpellPlayer_PatchAddr, (UInt32)&AddSpellPlayerHook);

	// fix the various Activate related stuff
	Init_RefActivationPatch();

	// patch the fly camera update function
	Init_PlayerFlyCamPatch();

	// this seems stable and helps in debugging, but it makes large files during gameplay
#if defined(_DEBUG) && 0
	Hook_DebugPrint();
#endif
}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static TESDescription** s_LastRetrievedDescription = (TESDescription**)0x00B33C04;
static BSStringT*			s_LastRetrievedDescriptionText = (BSStringT*)0x00B33C08;
static const UInt32		kTESDescription_GetText_Addr = 0x0046A710;
static const UInt32		kTESDescriptionHook_RetnAddr = 0x0046A715;
static const UInt32		kTlsIndex = 0x00BA9DE4;
#else
#error unsupported Oblivion version
#endif

static std::map<TESDescription*, std::string> s_descriptionChanges;
static bool s_bHookInstalled = false;

const char* GetDescription(TESDescription* desc)
{
	if (desc) {
		std::map<TESDescription*, std::string>::iterator found = s_descriptionChanges.find(desc);
		if (found != s_descriptionChanges.end()) {
			return found->second.c_str();
		}
	}

	return NULL;
}

static void __stdcall DoTESDescriptionHook(TESDescription* desc)
{
	const char* text = GetDescription(desc);
	if (text) {
		s_LastRetrievedDescriptionText->Set(text);
		*s_LastRetrievedDescription = desc;
	}
}

static __declspec(naked) void TESDescription_GetTextHook(void)
{
	static TESDescription* _this = NULL;

	__asm {
		pushad
		mov	_this, ecx
		push _this
		call DoTESDescriptionHook
		popad
		mov	eax, kTlsIndex
		mov	eax, [eax]
		jmp kTESDescriptionHook_RetnAddr
	}
}

void AddDescription(TESDescription* desc, const char* newText) {
	if (!s_bHookInstalled) {
		WriteRelJump(kTESDescription_GetText_Addr, (UInt32)&TESDescription_GetTextHook);
		s_bHookInstalled = true;
	}
	s_descriptionChanges[desc] = newText;

	// clear the cache in case the data for the form being modified is cached
	*s_LastRetrievedDescription = NULL;
	s_LastRetrievedDescriptionText->Set(NULL);
}

bool SetDescriptionTextForForm(TESForm* form, const char* newText, UInt8 skillIndex)
{
	TESDescription* desc = NULL;
	if (skillIndex != -1) {
		TESSkill* skill = OBLIVION_CAST(form, TESForm, TESSkill);
		if (skill && skillIndex < 4) {
			desc = &skill->levelQuote[skillIndex];
		}
	}
	else {
		TESDescription* desc = OBLIVION_CAST(form, TESForm, TESDescription);
	}

	return SetDescriptionText(desc, newText);
}

bool SetDescriptionText(TESDescription* desc, const char* newText)
{
	if (desc && newText) {
		AddDescription(desc, newText);
		return true;
	}

	return false;
}

bool IsDescriptionModified(TESDescription* desc)
{
	return s_descriptionChanges.find(desc) != s_descriptionChanges.end();
}

void SetRetainExtraOwnership(bool bRetain)
{
	UInt8 retain = bRetain ? 1 : 0;
	SafeWrite8(kExtraOwnershipDefaultSetting, retain);
	SafeWrite8(kExtraOwnershipDefaultSetting2, retain);
}

bool ToggleBlockPerk(UInt32 mastery, bool bEnable)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 kJMPatchAddr = 0x005F5C67;
	static const UInt16 kJMOverwrittenBytes = 0x6D7F;
#else
#error unsupported Oblivion version
#endif

	switch (mastery) {
		case kMasteryLevel_Journeyman:
			if (bEnable) {
				SafeWrite16(kJMPatchAddr, kJMOverwrittenBytes);
			}
			else {
				SafeWrite16(kJMPatchAddr, 0x9090);
			}
			return true;
	}

	return false;
}

bool ToggleMercantilePerk (UInt32 mastery, bool bEnable)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	// for both, we replace a short 'jl' instruction with a 'jmp' to turn off the perk
	static const UInt32 kJMPatchAddr = 0x00485627;
	static const UInt32 kMSPatchAddr = 0x00488F81;
	// master perk: extra 500 barter gold, replace jnz with jmp to toggle off
	static const UInt32 kMSPatchAddr2 = 0x005FAAC5;
#else
#error unsupported Oblivion version
#endif

	switch (mastery) {
		case kMasteryLevel_Journeyman:
			SafeWrite8 (kJMPatchAddr, bEnable ? 0x7C : 0xEB);
			return true;
		case kMasteryLevel_Master:
			SafeWrite8 (kMSPatchAddr, bEnable ? 0x7C : 0xEB);
			SafeWrite8 (kMSPatchAddr2, bEnable ? 0x75 : 0xEB);
			return true;
	}

	return false;
}

bool ToggleSkillPerk(UInt32 actorVal, UInt32 mastery, bool bEnable)
{
	// currently this supports only the Journeyman Block perk
	// may be extended in the future to support other perks
	if (mastery < kMasteryLevel_MAX) {
		switch (actorVal) {
			case kActorVal_Block:
				return ToggleBlockPerk(mastery, bEnable);
			case kActorVal_Mercantile:
				return ToggleMercantilePerk (mastery, bEnable);
		}
	}

	return false;
}

// quest log text is read from disk as needed
// SetQuestStageText cmd allows changing the text, so we have to
// hook QuestStageItem::GetLogText() to support that
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static const UInt32 kQuestStageItem_GetLogText_RetnAddr = 0x0052AF46;
static const UInt32 kQuestStageItem_GetLogText_PatchAddr = 0x0052AF40;
#else
#error unsupported Oblivion version
#endif

static std::map<QuestStageItem*, std::string> s_questStageTextMap;

const char* __stdcall GetQuestStageItemText(QuestStageItem* item)
{
	const char* text = NULL;
	std::map<QuestStageItem*, std::string>::iterator iter = s_questStageTextMap.find(item);
	if (iter != s_questStageTextMap.end()) {
		text = iter->second.c_str();
	}

	return text;
}

static __declspec(naked) void Hook_QuestStageItem_GetLogText(void)
{
	static UInt32 s_stageText = NULL;

	__asm {
		pushad

		push ecx
		call GetQuestStageItemText
		test eax, eax
		jz NotFound
		mov [s_stageText], eax
		popad
		mov eax, [s_stageText]
		retn 4

	NotFound:
		popad
		// overwritten code
		push ebp
		mov ebp, esp
		sub esp, 0xC
		jmp [kQuestStageItem_GetLogText_RetnAddr]
	}
}

void SetQuestStageItemText(QuestStageItem* item, const char* text)
{
	static bool s_hookInstalled = false;
	if (!s_hookInstalled) {
		WriteRelJump(kQuestStageItem_GetLogText_PatchAddr, (UInt32)&Hook_QuestStageItem_GetLogText);
		s_hookInstalled = true;
	}

	if (item && text) {
		s_questStageTextMap[item] = text;
	}
}

void UnsetQuestStageItemText(QuestStageItem* item)
{
	std::map<QuestStageItem*, std::string>::iterator iter = s_questStageTextMap.find(item);
	if (iter != s_questStageTextMap.end())
		s_questStageTextMap.erase(iter);
}

long double g_PlayerFlyCamSpeed = 10.f;

void Init_PlayerFlyCamPatch( void )
{
	static const UInt32	kPatchLocation[] =
	{
		0x00664470,
		0x0066448D,
		0x006644AA,
		0x006644C7
	};

	for (int i = 0; i < 4; i++)
		SafeWrite32(kPatchLocation[i] + 2, (UInt32)&g_PlayerFlyCamSpeed);
}


