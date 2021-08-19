#include <list>
#include <stdarg.h>
#include "EventManager.h"
#include "ArrayVar.h"
#include "PluginAPI.h"
#include "GameAPI.h"
#include "ScriptUtils.h"
#include "obse_common/SafeWrite.h"
#include "FunctionScripts.h"
#include "GameObjects.h"
#include "ThreadLocal.h"
#include "common/ICriticalSection.h"
#include "Hooks_Gameplay.h"
#include "GameOSDepend.h"
#include "InventoryReference.h"
#include "GameData.h"

namespace EventManager {

void __stdcall HandleEventForCallingObject(UInt32 id, TESObjectREFR* callingObj, void* arg0, void* arg1);

static ICriticalSection s_criticalSection;

//////////////////////
// Event definitions
/////////////////////

// Hook routines need to be forward declared so they can be used in EventInfo structs.
// ###TODO: Would be nice to move hooks out into a separate file
static void  InstallHook();
static void  InstallActivateHook();
static void  InstallOnVampireFeedHook();
static void  InstallOnSkillUpHook();
static void  InstallModPCSHook();
static void  InstallMapMarkerHook();
static void  InstallOnSpellCastHook();
static void  InstallOnFallImpactHook();
static void  InstallOnDrinkPotionHook();
static void  InstallOnEatIngredientHook();
static void	 InstallOnActorEquipHook();
static void  InstallOnHealthDamageHook();
static void  InstallOnMeleeAttackHook();
static void  InstallOnMeleeReleaseHook();
static void  InstallOnBowAttackHook();
static void  InstallOnBowReleaseHook();
static void  InstallOnBlockHook();
static void  InstallOnRecoilHook();
static void  InstallOnStaggerHook();
static void  InstallOnDodgeHook();
static void  InstallOnSoulTrapHook();
static void	 InstallIniHook();
static void  InstallOnRaceSelectionCompleteHook();
static void  InstallOnQuestCompleteHook();
static void  InstallOnMagicCastHook();
static void  InstallOnMagicApplyHook();
static void  InstallSwimmingBreathHook();

enum {
	kEventMask_OnActivate		= 0xFFFFFFFF,		// special case as OnActivate has no event mask
};

// hook installers
static EventHookInstaller s_MainEventHook = InstallHook;
static EventHookInstaller s_ActivateHook = InstallActivateHook;
static EventHookInstaller s_VampireHook = InstallOnVampireFeedHook;
static EventHookInstaller s_SkillUpHook = InstallOnSkillUpHook;
static EventHookInstaller s_ModPCSHook = InstallModPCSHook;
static EventHookInstaller s_MapMarkerHook = InstallMapMarkerHook;
static EventHookInstaller s_SpellScrollHook = InstallOnSpellCastHook;
static EventHookInstaller s_FallImpactHook = InstallOnFallImpactHook;
static EventHookInstaller s_DrinkHook = InstallOnDrinkPotionHook;
static EventHookInstaller s_EatIngredHook = InstallOnEatIngredientHook;
static EventHookInstaller s_ActorEquipHook = InstallOnActorEquipHook;
static EventHookInstaller s_HealthDamageHook = InstallOnHealthDamageHook;
static EventHookInstaller s_MeleeAttackHook = InstallOnMeleeAttackHook;
static EventHookInstaller s_MeleeReleaseHook = InstallOnMeleeReleaseHook;
static EventHookInstaller s_BowAttackHook = InstallOnBowAttackHook;
static EventHookInstaller s_BowReleaseHook = InstallOnBowReleaseHook;
static EventHookInstaller s_BlockHook = InstallOnBlockHook;
static EventHookInstaller s_RecoilHook = InstallOnRecoilHook;
static EventHookInstaller s_StaggerHook = InstallOnStaggerHook;
static EventHookInstaller s_DodgeHook = InstallOnDodgeHook;
static EventHookInstaller s_SoulTrapHook = InstallOnSoulTrapHook;
static EventHookInstaller s_IniHook = InstallIniHook;
static EventHookInstaller s_OnRaceSelectionCompleteHook = InstallOnRaceSelectionCompleteHook;
static EventHookInstaller s_OnQuestCompleteHook = InstallOnQuestCompleteHook;
static EventHookInstaller s_OnMagicCastHook = InstallOnMagicCastHook;
static EventHookInstaller s_OnMagicApplyHook = InstallOnMagicApplyHook;
static EventHookInstaller s_OnWaterDiveSurfaceHook = InstallSwimmingBreathHook;


// event handler param lists
static UInt8 kEventParams_GameEvent[2] =
{
	Script::eVarType_Ref, Script::eVarType_Ref
};

static UInt8 kEventParams_OneRef[1] =
{
	Script::eVarType_Ref,
};

static UInt8 kEventParams_GameMGEFEvent[2] =
{
	// MGEF gets converted to effect code when passed to scripts
	Script::eVarType_Ref, Script::eVarType_Integer
};

static UInt8 kEventParams_OneString[1] =
{
	Script::eVarType_String
};

static UInt8 kEventParams_OneInteger[1] =
{
	Script::eVarType_Integer
};

static UInt8 kEventParams_TwoIntegers[2] =
{
	Script::eVarType_Integer, Script::eVarType_Integer
};

static UInt8 kEventParams_OneFloat_OneRef[2] =
{
	 Script::eVarType_Float, Script::eVarType_Ref
};

static UInt8 kEventParams_OneRef_OneInt[2] =
{
	Script::eVarType_Ref, Script::eVarType_Integer
};

static UInt8 kEventParams_OneArray[1] =
{
	Script::eVarType_Array
};

///////////////////////////
// internal functions
//////////////////////////

void __stdcall HandleEvent(UInt32 id, void * arg0, void * arg1);
void __stdcall HandleGameEvent(UInt32 eventMask, TESObjectREFR* source, TESForm* object);

static const UInt32 kVtbl_PlayerCharacter = 0x00A73A0C;
static const UInt32 kVtbl_Character = 0x00A6FC9C;
static const UInt32 kVtbl_Creature = 0x00A710F4;
static const UInt32 kVtbl_ArrowProjectile = 0x00A6F08C;
static const UInt32 kVtbl_MagicBallProjectile = 0x00A75944;
static const UInt32 kVtbl_MagicBoltProjectile = 0x00A75BC4;
static const UInt32 kVtbl_MagicFogProjectile = 0x00A75EFC;
static const UInt32 kVtbl_MagicSprayProjectile = 0x00A76594;
static const UInt32 kVtbl_TESObjectREFR = 0x00A46C44;

static const UInt32 kMarkEvent_HookAddr = 0x004FBF90;
static const UInt32 kMarkEvent_RetnAddr = 0x004FBF96;

static const UInt32 kActivate_HookAddr = 0x004DD286;
static const UInt32 kActivate_RetnAddr = 0x004DD28C;

static UInt32 s_PlayerCharacter_SetVampireHasFed_OriginalFn = 0x0066B120;


// cheap check to prevent duplicate events being processed in immediate succession
// (e.g. game marks OnHitWith twice per event, this way we only process it once)
static TESObjectREFR* s_lastObj = NULL;
static TESForm* s_lastTarget = NULL;
static UInt32 s_lastEvent = NULL;

// OnHitWith often gets marked twice per event. If weapon enchanted, may see:
//  OnHitWith >> OnMGEFHit >> ... >> OnHitWith. 
// Prevent OnHitWith being reported more than once by recording OnHitWith events processed
static TESObjectREFR* s_lastOnHitWithActor = NULL;
static TESForm* s_lastOnHitWithWeapon = NULL;

// And it turns out OnHit is annoyingly marked several times per frame for spells/enchanted weapons
static TESObjectREFR* s_lastOnHitVictim = NULL;
static TESForm* s_lastOnHitAttacker = NULL;

//////////////////////////////////
// Hooks
/////////////////////////////////
static TESForm* old_onequip = nullptr;

void __stdcall HandleEventFilter(UInt32 mask , TESObjectREFR* source, TESForm* target) {
//	_MESSAGE("%0X   %08X  %08X  %08X", mask , source, target , old_onequip);
	//Events can be sent with invalid data, as source is not a reference on a OnActorEquip. Why the fuck porcoddio?  
	if (mask == ScriptEventList::kEvent_OnActorEquip && source && source->IsReference() && (old_onequip != nullptr && target->refID == old_onequip->refID)) {
//		_MESSAGE("Event %08X %08X blocked", source->refID, target->refID);
		old_onequip = nullptr;
		return; //the other hook already intercepted the event
	} 
	else if (mask == ScriptEventList::kEvent_OnActorEquip && source && source->IsReference()) {
//		_MESSAGE("Event %08X %08X non blocked", source->refID, target->refID);
		old_onequip = nullptr;
	}
	HandleGameEvent(mask, source, target);
}

static __declspec(naked) void MarkEventHook(void)
{
	// volatile: ecx, edx, eax

	__asm {
		// grab args
		mov	eax, [esp+8]			// ExtraDataList*
		test eax, eax
		jnz	XDataListIsNotNull

		push ebx
		push esi
		mov esi, 0
		jmp [kMarkEvent_RetnAddr]

	XDataListIsNotNull:
		sub eax, 0x44				// TESObjectREFR* thisObj
		mov ecx, [esp+0xC]			// event mask
		mov edx, [esp+4]			// target

		pushad
		push edx
		push eax
		push ecx
		call HandleEventFilter
		popad

		// overwritten code
		push ebx
		push esi
		mov esi, eax		// thisObj
		add esi, 0x44

		jmp [kMarkEvent_RetnAddr]
	}
}		

void InstallHook()
{
	WriteRelJump(kMarkEvent_HookAddr, (UInt32)&MarkEventHook);
}

#if 0
static __declspec(naked) void OnActorEquipHook(void)
{
	// game fails to mark OnActorEquip event reliably
	// this additional hook hooks ExtraContainerChanges::Data::EquipItemForActor to rectify that
	// overwrites a jz rel32 instruction

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 jzAddr = 0x00489D09;
	static const UInt32 jnzAddr = 0x00489C74;
	static const UInt32 argsOffset = 0x30;
#else
#error unsupported Oblivion version
#endif

	__asm {
		// figure out where we're returning to based on zero flag
		jz _JZ
		mov ebp, [jnzAddr]
		jmp DoHook

	_JZ:
		mov ebp, [jzAddr]

	DoHook:
		mov eax, esp

		add eax, [argsOffset]
		mov esi, [eax]			// item being equipped
		add eax, 8
		mov eax, [eax]			// actor equipping

		// make sure args are valid
		test eax, eax
		jz Done
		test esi, esi
		jz Done

		// invoke handler
		pushad
		push esi
		push eax
		push kEventID_OnActorEquip
		call HandleEvent
		popad

	Done:
		jmp	ebp
	}
}
#endif
static __declspec(naked) void OnActorEquipHook(void)
{
	static const UInt32 s_callAddr = 0x00489C30;	// ExtraContainerChanges::Data::EquipForActor()

	static const UInt32 kEventMask = ScriptEventList::kEvent_OnActorEquip;
	__asm {
		mov [old_onequip], edi
		pushad
		push edi
		push ebp
		push [kEventMask]
		call HandleGameEvent
		popad

		jmp	[s_callAddr]
	}
}

static void InstallOnActorEquipHook()
{
	//Non equippable objects send event with this
	if (s_MainEventHook) {
		s_MainEventHook();
		s_MainEventHook = NULL;
	} 
	static const UInt32 patchAddr = 0x00489C6E;
	// below is commented out b/c it reproducibly produces game instability in seemingly unrelated code.
	// WriteRelJump(patchAddr, (UInt32)&OnActorEquipHook);

	// this exhibits same problem
	// The issue is that our Console_Print routine interacts poorly with the game's debug text (turned on with TDT console command)
	// when called from a background thread.
	// So if the handler associated with this event calls Print, PrintC, etc, there is a chance it will crash.
	// ###TODO: fix! 
	//EDIT: The code actually defer the events if happen on the non main thread. This issue should no longer be relevant
	WriteRelCall(0x005F376D, (UInt32)&OnActorEquipHook);
}

static __declspec(naked) void TESObjectREFR_ActivateHook(void)
{
	__asm {
		pushad
		push edi		// activating refr
		push ecx		// this
		mov eax, kEventMask_OnActivate
		push eax
		call HandleGameEvent
		popad

		// overwritten code
		xor bl, bl
		test edi, edi
		mov esi, ecx
		jmp	[kActivate_RetnAddr]
	}
}

void InstallActivateHook()
{
	WriteRelJump(kActivate_HookAddr, (UInt32)&TESObjectREFR_ActivateHook);
}

void __stdcall OnVampireFeedHook(bool bHasFed)
{
	ASSERT(s_PlayerCharacter_SetVampireHasFed_OriginalFn != 0);

	if (bHasFed) {
		HandleEvent(kEventID_OnVampireFeed, NULL, NULL);
	}

	ThisStdCall(s_PlayerCharacter_SetVampireHasFed_OriginalFn, *g_thePlayer, bHasFed);
}

void InstallOnVampireFeedHook()
{
	static const UInt32 vtblEntry = 0x00A73C70;
	SafeWrite32(vtblEntry, (UInt32)OnVampireFeedHook);

}

static __declspec(naked) void OnSkillUpHook(void)
{
	// on entry: edi = TESSkill*
	// retn addr determined by zero flag (we're overwriting a jnz rel32 instruction)
	// ecx, eax volatile
	static const UInt32 s_jnzAddr = 0x00668129;		// if zero flag set
	static const UInt32 s_jzAddr = 0x006680A6;		// if zero flag not set


	__asm {
		jnz	ZeroFlagSet
		mov	ecx, [s_jzAddr]
		jmp DoHook
	ZeroFlagSet:
		mov ecx, [s_jnzAddr]
	DoHook:
		pushad
		push 0
		mov eax, [edi+0x2C]		// skill->skill actor value code
		push eax
		push kEventID_OnSkillUp
		call HandleEvent		// HandleEvent(kEventID_OnSkillUp, (void*)skill->skill, NULL)
		popad
		jmp ecx
	}
}

void InstallOnSkillUpHook()
{
	static const UInt32 hookAddr = 0x006680A0;

	WriteRelJump(hookAddr, (UInt32)&OnSkillUpHook);
}

static __declspec(naked) void ModPCSHook(void)
{
	// on entry: esi = TESSkill*, [esp+0x21C-0x20C] = amount. amount may be zero or negative.
	// hook overwrites a jz instruction following a comparison of amount to zero
	// eax, edx volatile
	static const UInt32 jz_retnAddr = 0x0050D1CA;
	static const UInt32 jnz_retnAddr = 0x0050D0ED;
	static const UInt32 amtStackOffset = 0x10;


	__asm {
		mov edx, esp
		jz _JZ
		mov eax, [jnz_retnAddr]
		jmp DoHook
	_JZ:
		mov eax, [jz_retnAddr]
	DoHook:
		push eax				// retn addr
		pushad

		// grab amt, skill
		mov eax, [amtStackOffset]
		mov eax, [eax+edx]
		push eax				// amount
		mov eax, [esi+0x2C]		// TESSkill* skill->skill
		push eax
		push kEventID_OnScriptedSkillUp
		call HandleEvent		// HandleEvent(kEventID_OnScriptedSkillUp, skill->skill, amount)

		popad
		// esp now points to saved retn addr
		retn
	}
}
	
void InstallModPCSHook()
{
	static const UInt32 hookAddr = 0x0050D0E7;


	WriteRelJump(hookAddr, (UInt32)&ModPCSHook);
}

static __declspec(naked) void OnMapMarkerAddHook(void)
{
	// on entry, we know the marker is being set as visible
	// ecx: ExtraMapMarker::Data* mapMarkerData
	// Only report event if marker was not *already* visible
	// This can be called from 3 locations in game code, 2 of which we're interested in
	static const UInt32 s_HUDMainMenuRetnAddr = 0x005A7058;		// from HUDMainMenu when player discovers a new marker
	static const UInt32 s_ShowMapRetnAddr = 0x0050AD95;			// from Cmd_ShowMap_Execute

	__asm {
		// is marker already visible?
		test byte ptr [ecx+0x0C], 1				// flags, bit 1 is "visible"
		jnz Done

		// not visible, mark it
		or byte ptr [ecx+0x0C], 1

		// get the map marker refr based on calling code
		mov eax, [s_HUDMainMenuRetnAddr]
		cmp [esp], eax
		jnz CheckShowMapRetnAddr
		mov eax, [edi+0x4]
		jmp GotRefr

	CheckShowMapRetnAddr:
		mov eax, [s_ShowMapRetnAddr]
		cmp [esp], eax
		jnz Done			// unknown caller, so don't handle the event since we can't get the refr
		mov eax, [esp+0x0C]
		
	GotRefr:
		// we have a mapmarker refr, report the event
		pushad
		push 0
		push eax			// TESObjectREFR* marker
		push kEventID_OnMapMarkerAdd
		call HandleEvent
		popad

	Done:
		retn 0x4;
	}
}

void InstallMapMarkerHook()
{
	static const UInt32 patchAddr = 0x0042B327;

	WriteRelJump(patchAddr, (UInt32)&OnMapMarkerAddHook);
}

static void DoSpellCastHook(MagicCaster* caster)
{
	MagicItemForm* magicItemForm = OBLIVION_CAST(caster->GetActiveMagicItem(), MagicItem, MagicItemForm);
	TESObjectREFR* casterRef = OBLIVION_CAST(caster, MagicCaster, TESObjectREFR);
	if (magicItemForm && casterRef) {
		UInt32 eventID = OBLIVION_CAST(magicItemForm, MagicItemForm, EnchantmentItem) ? kEventID_OnScrollCast : kEventID_OnSpellCast;
		HandleEvent(eventID, casterRef, magicItemForm);
	}
}

static __declspec(naked) void OnSpellCastHook(void)
{
	// on entry, we know the spell is valid to cast
	// edi: MagicCaster
	// spell can be obtained from MagicCaster::GetQueuedMagicItem()
	static const UInt32 s_retnAddr = 0x005F3F04;

	__asm {
		pushad
		push edi
		call DoSpellCastHook
		pop edi
		popad
		jmp [s_retnAddr]
	}
}

static void InstallOnSpellCastHook()
{
	// overwriting jnz rel32 when MagicCaster->CanCast() returns true
	static const UInt32 s_patchAddr = 0x005F3E71;

	
	WriteRelJnz(s_patchAddr, (UInt32)&OnSpellCastHook);
}

static __declspec(naked) void OnFallImpactHook(void)
{
	static const UInt32 s_retnAddr = 0x005EFD57;

	// on entry: esi=Actor*
	__asm {
		pushad
		push 0
		push esi
		push kEventID_OnFallImpact
		call HandleEvent
		popad

		// overwritten code
		and dword ptr [ecx+0x1F4], 0xFFFFFF7F
		jmp [s_retnAddr]
	}
}

static void InstallOnFallImpactHook()
{
	static const UInt32 s_patchAddr = 0x005EFD4D;

	WriteRelJump(s_patchAddr, (UInt32)&OnFallImpactHook);
}

static __declspec(naked) void OnDrinkPotionHook(void)
{
	static const UInt32 s_arg2StackOffset = 0x18;

	// hooks bool Actor::UseAlchemyItem(AlchemyItem*, UInt32, bool)
	// is called recursively for player - on second call boolean arg is true
	// boolean arg always true for non-player actor
	// returns true if successfully used potion (false e.g. if max potion count exceeded)

	// on entry:
	//	bl: retn value (bool)
	//	esi: Actor*
	//	edi: AlchemyItem*
	
	__asm {
		// make sure retn value is true
		test bl, bl
		jz Done

		// make sure arg2 is true
		mov eax, esp
		add eax, s_arg2StackOffset
		mov al, byte ptr [eax]
		test al, al
		jz Done

		// invoke the handler
		pushad
		push edi
		push esi
		push kEventID_OnDrinkPotion
		call HandleEvent
		popad

	Done:
		// overwritten code
		mov al, bl
		pop ebx
		pop edi
		pop esi
		retn 0x0C
	}
}

static void InstallOnDrinkPotionHook()
{
	static const UInt32 s_hookAddr = 0x005E0968;


	WriteRelJump(s_hookAddr, (UInt32)&OnDrinkPotionHook);
}

static __declspec(naked) void OnEatIngredientHook(void)
{
	static const UInt32 arg2StackOffset = 0x00000024;


	// on entry:
	//	esi: 'eater' refr Actor*
	//	edi: IngredientItem*
	//	boolean arg2 must be true as this is called recursively for player (once with arg=false, then arg=true)

	__asm {
		// check boolean arg, make sure it's true
		mov eax, esp
		add eax, arg2StackOffset
		mov al, byte ptr [eax]
		test al, al
		jz Done

		// handle event
		pushad
		push edi			// ingredient
		push esi			// actor
		push kEventID_OnEatIngredient
		call HandleEvent
		popad

	Done:
		// overwritten code
		jmp MarkBaseExtraListScriptEvent
	}
}

static void InstallOnEatIngredientHook()
{
	static const UInt32 s_hookAddr = 0x005E4515;	// overwrite call to MarkScriptEventList(actor, baseExtraList, kEvent_Equip)

	WriteRelCall(s_hookAddr, (UInt32)&OnEatIngredientHook);
}

static __declspec(naked) void OnHealthDamageHook(void)
{
	// hooks Actor::OnHealthDamage virtual fn
	// only runs if actor is not already dead. Runs *before* damage has a chance to kill actor, so possible to prevent death
	// overwrites a virtual call to Actor::GetCurAV(health)

	// on entry:
	//	edx: virtual fn addr GetCurAV()
	//	esi: Actor* this (actor taking damage)
	//	arg0: Actor* attacker (may be null)
	//	arg1: float damage (has been modified for game difficulty if applicable)

	static const UInt32 argsOffset = 0x00000008;
	static const UInt32 retnAddr = 0x006034D1;

	__asm {
		mov eax, esp
		add eax, [argsOffset]
		pushad
		push [eax]					// attacker
		add eax, 4
		push [eax]					// damage
		push esi					// this
		push kEventID_OnHealthDamage
		call HandleEventForCallingObject
		popad

		// overwritten code
		push 8						// kActorVal_Health
		mov ecx, esi
		call edx

		jmp [retnAddr];
	}
}

static void InstallOnHealthDamageHook()
{
	static const UInt32 patchAddr = 0x006034CB;

	WriteRelJump(patchAddr, (UInt32)&OnHealthDamageHook);
}

// bitfield, set bit (1 << HighProcess::kAction_XXX) for actions which have event handlers registered
static UInt32 s_registeredActions = 0;

static __declspec(naked) void OnActionChangeHook(void)
{
	// overwrites call to HighProcess::SetCurrentAction(UInt16 action, BSAnimGroupSequence*)
	//	esi: Actor*
	//	eax: one of HighProcess::kAction_XXX
	//	ebp: BSAnimGroupSequence*
	//	edx: virtual fn address
	// volatile: ebp, esi (both popped after call), eax

	__asm {
		// -1 == no action
		cmp eax, 0xFFFFFFFF
		jz Done

		push ecx						// actor->process
		add ecx, 0x1F4					// process->currentAction
		mov cl, byte ptr [ecx]
		cmp cl, al						// is new action same as current action?
		jz NotInterested				// if we're interested, we've already reported it, so ignore.
		
		mov ecx, eax					// action
		mov ebp, 1
		shl ebp, cl						// bit for this action
		test [s_registeredActions], ebp	// are we interested in this action?
		jz NotInterested

		// k, we're interested, so invoke the handler
		pushad

		// this supports a linear subset of HighProcess::kAction_XXX, from kAction_Attack through kAction_Dodge
		// so we can calculate the event ID easily
		sub ecx, 2			// kAction_Attack
		add ecx, kEventID_OnMeleeAttack

		push 0
		push esi
		push ecx
		call HandleEvent
		popad

	NotInterested:
		pop ecx

	Done:
		// overwritten code
		call edx
		pop esi
		pop ebp
		retn 8
	}
}

static void InstallOnActionChangeHook(UInt32 action)
{
	static const UInt32 patchAddr = 0x005F01A5;

	ASSERT_STR((action >= HighProcess::kAction_Attack && action <= HighProcess::kAction_Dodge),
		"Invalid action supplied to InstallOnActionChangeHook()");

	// same hook used by multiple events, only install once
	static bool s_installed = false;
	if (!s_installed) {
		WriteRelJump(patchAddr, (UInt32)&OnActionChangeHook);
		s_installed = true;
	}

	// record our interest in this action
	s_registeredActions |= (1 << action);
}

static void InstallOnMeleeAttackHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_Attack);
}

static void InstallOnMeleeReleaseHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_AttackFollowThrough);
}

static void InstallOnBowAttackHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_AttackBow);
}

static void InstallOnBowReleaseHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_AttackBowArrowAttached);
}

static void InstallOnBlockHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_Block);
}

static void InstallOnRecoilHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_Recoil);
}

static void InstallOnStaggerHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_Stagger);
}

static void InstallOnDodgeHook()
{
	InstallOnActionChangeHook(HighProcess::kAction_Dodge);
}

	// when player successfully traps a soul
	static const UInt32 s_soulTrapPatchAddr = 0x006A4EC8;	

	// when an existing EntryExtendData for a soulgem is populated with a newly captured soul
	static const UInt32 s_createExtraSoulPatchAddr1 = 0x00484D14;
	static const UInt32 s_createExtraSoulRetnAddr1 = 0x00484D19;

	// when a new EntryExtendData for a soulgem is created for a newly captured soul
	static const UInt32 s_createExtraSoulPatchAddr2 = 0x00484D47;
	static const UInt32 s_createExtraSoulRetnAddr2 = 0x00484D4D;

	// void tList<T>::AddEntry (T* data), prepends new entry to list
	static const UInt32 s_BSSimpleList_AddEntry = 0x00446CB0;

	// void BaseExtraList::SetExtraSoulLevel (UInt32 soulLevel)
	static const UInt32 s_BaseExtraList_SetExtraSoulLevel = 0x0041EF30;


// temp ref (InventoryReference) created for most recently populated soul gem in player's inventory, valid only for a single frame
static TESObjectREFR* s_lastFilledSoulgem = NULL;

static void __stdcall SetLastFilledSoulgem (ExtraContainerChanges::EntryData* entryData, tList<ExtraDataList>::_Node* extendData)
{
	// locate ExtraContainerChanges::Entry for this EntryData
	TESObjectREFR* owner = *g_thePlayer;
	tList<ExtraContainerChanges::EntryData>::Iterator entry = ExtraContainerChanges::GetForRef(*g_thePlayer)->data->objList->Begin();
	while (!entry.End() && *entry != entryData) {
		++entry;
	}
	// create temp InventoryReference for soulgem
	InventoryReference::Data irefData(entryData->type, entry.Get(), extendData->item);
	InventoryReference* iref =  InventoryReference::CreateInventoryRef(owner, irefData, false);
	s_lastFilledSoulgem = iref->GetRef();
}
	
static __declspec(naked) void CreateExtraSoulHook1 (void)
{
	__asm {
		pushad
		push esi		// tList<ExtraDataList>
		push ebp		// EntryData
		call SetLastFilledSoulgem

		popad
		call [s_BaseExtraList_SetExtraSoulLevel]		// overwritten function call
		jmp [s_createExtraSoulRetnAddr1]
	}
}

static __declspec(naked) void CreateExtraSoulHook2 (void)
{
	__asm {
		push esi
		mov esi, ecx						// List
		call [s_BSSimpleList_AddEntry]		// overwritten function call, returns EntryExtendData*
		pushad
		mov ecx, esi
		push ecx		// EntryExtendData
		push ebp		// EntryData
		call SetLastFilledSoulgem

		popad
		jmp [s_createExtraSoulRetnAddr2]
	}
}
		
static __declspec(naked) void OnSoulTrapHook(void)
{
	__asm {
		pushad
		mov eax, [s_lastFilledSoulgem]
		push eax
		push esi		// actor whose soul was captured
		push kEventID_OnSoulTrap
		call HandleEvent
		popad

		// we overwrote a call to QueueUIMessage, jump there and it'll return to hook location
		jmp QueueUIMessage
	}
}

static void InstallOnSoulTrapHook()
{
	WriteRelCall(s_soulTrapPatchAddr, (UInt32)&OnSoulTrapHook);
	WriteRelJump(s_createExtraSoulPatchAddr1, (UInt32)&CreateExtraSoulHook1);

	WriteRelJump(s_createExtraSoulPatchAddr2, (UInt32)&CreateExtraSoulHook2);
}
	
// hook overwrites IniSettingCollection::Write() virtual function
static UInt32 s_IniSettingCollection_Write = 0;
static void SaveIniHook()
{
	// Ini is saved when game exits. We cannot invoke a function script at that time, so check
	OSGlobals* globals = *g_osGlobals;
	if (NULL == globals || 0 != globals->quitGame)
		return;

	// check if ini can be written at this time; if so dispatch pre-save event
	IniSettingCollection* settings = IniSettingCollection::GetSingleton();	// aka 'this', since this is a virtual method
	if (NULL != settings->writeInProgressCollection)
		HandleEvent(kEventID_SaveIni, 0, NULL);

	// just in case I've screwed something up, let the vanilla Write() method run even if we've determined above that it shouldn't
	bool bWritten = ThisStdCall(s_IniSettingCollection_Write, settings) ? true : false;
	
	// if successful, dispatch post-save event
	if (bWritten)
		HandleEvent(kEventID_SaveIni, (void*)1, NULL);
}

static void InstallIniHook()
{
	IniSettingCollection* settings = IniSettingCollection::GetSingleton();
	UInt32* vtbl = *((UInt32**)settings);
	s_IniSettingCollection_Write = vtbl[7];
	SafeWrite32((UInt32)(vtbl+7), (UInt32)(&SaveIniHook));
}

static const UInt32 kChargenPatchAddr = 0x005C2B36;
static const UInt32 kChargenCallAddr  = 0x0066C580;
static const UInt32 kChargenRetnAddr  = 0x005C2B3B;


static __declspec (naked) void OnRaceSelectionCompleteHook (void)
{
	__asm {
		pushad
		push 0
		push 0
		push kEventID_OnRaceSelectionComplete
		call HandleEvent
		popad
		call [kChargenCallAddr]
		jmp  [kChargenRetnAddr]
	}
}

static void InstallOnRaceSelectionCompleteHook()
{
	WriteRelJump (kChargenPatchAddr, (UInt32)&OnRaceSelectionCompleteHook);
}

static const UInt32 kQuestCompletePatchAddr = 0x00529847;
static const UInt32 kQuestCompleteRetnAddr  = 0x00529851;


static __declspec (naked) void OnQuestCompleteHook (void)
{
	__asm {
		pushad
		push 0
		push ecx
		push kEventID_OnQuestComplete
		call HandleEvent
		popad
		
		or	 byte ptr [ecx + 0x3C], 2
		jmp  [kQuestCompleteRetnAddr]
	}
}

static void InstallOnQuestCompleteHook()
{
	WriteRelJump (kQuestCompletePatchAddr, (UInt32)&OnQuestCompleteHook);
}

static const UInt32 kMagicCasterCastMagicItemFnAddr = 0x00699190;
static const UInt32 kMagicCasterCastMagicItemCallSites[13]  = 
{
	0x005020D6, 0x0050212F, 0x00514942, 0x005E4496,
	0x00601439, 0x006033F2, 0x006174C2, 0x0062B3F3,
	0x0062B539, 0x00634FAE, 0x0064AE06, 0x0064D786,
	0x006728FC
};

static const UInt32 kMagicTargetAddEffectFnAddr = 0x006A27F0;
static const UInt32 kMagicTargetAddEffectCallSites[13]  = 
{
	0x005E560F, 0x006A2D7F
};

static bool PerformMagicCasterTargetHook(UInt32 eventID, MagicCaster* caster, MagicItem* magicItem, MagicTarget* target, UInt32 noHitVFX, ActiveEffect* av)
{
	bool result = false;
	
	if (eventID == kEventID_OnMagicCast) {
		result = ThisStdCall(kMagicCasterCastMagicItemFnAddr, caster, magicItem, target, noHitVFX);
	} else {
		result = ThisStdCall(kMagicTargetAddEffectFnAddr, target, caster, magicItem, av);
	}

	if (result) {
		TESObjectREFR* casterRef = OBLIVION_CAST(caster, MagicCaster, TESObjectREFR);
		TESObjectREFR* targetRef = OBLIVION_CAST(target, MagicTarget, TESObjectREFR);
		TESForm* magicItemForm = OBLIVION_CAST(magicItem, MagicItem, TESForm);

		if (casterRef == NULL && caster)
			casterRef = caster->GetParentRefr();

		if (targetRef == NULL && target)
			targetRef = target->GetParent();

		if (magicItemForm) {
			if (eventID == kEventID_OnMagicCast) {
				HandleEventForCallingObject(kEventID_OnMagicCast, casterRef, magicItemForm, targetRef);
			} else {
				HandleEventForCallingObject(kEventID_OnMagicApply, targetRef, magicItemForm, casterRef);
			}
		}
	}

	return result;
}

static bool __stdcall DoOnMagicCastHook(MagicCaster* caster, MagicItem* magicItem, MagicTarget* target, UInt32 noHitVFX)
{
	return PerformMagicCasterTargetHook(kEventID_OnMagicCast, caster, magicItem, target, noHitVFX, NULL);
}

static __declspec(naked) void OnMagicCastHook(void)
{
	__asm {
		push [esp + 0xC]
		push [esp + 0xC]
		push [esp + 0xC]
		push ecx
		xor eax, eax
		call DoOnMagicCastHook
		retn 0xC
	} 
}

static void InstallOnMagicCastHook()
{
	for (int i = 0; i < 13; i++) {
		WriteRelCall(kMagicCasterCastMagicItemCallSites[i], (UInt32)OnMagicCastHook);
	}
}

static bool __stdcall DoOnMagicApplyHook(MagicTarget* target, MagicCaster* caster, MagicItem* magicItem, ActiveEffect* av)
{
	return PerformMagicCasterTargetHook(kEventID_OnMagicApply, caster, magicItem, target, 0, av);
}

static __declspec(naked) void OnMagicApplyHook(void)
{
	__asm {
		push [esp + 0xC]
		push [esp + 0xC]
		push [esp + 0xC]
		push ecx
		xor eax, eax
		call DoOnMagicApplyHook
		retn 0xC
	} 
}

static void InstallOnMagicApplyHook()
{
	for (int i = 0; i < 2; i++) {
		WriteRelCall(kMagicTargetAddEffectCallSites[i], (UInt32)OnMagicApplyHook);
	}
}

//max swimming breath is calculated each frame based on actor's endurance
//hook the two calls to the function that does this
static const UInt32 kActorSwimBreath_CalcMax_CallAddr	= 0x00548960;	// original function for calculating max breath 
static const UInt32 kActorSwimBreath_CalcMax1_PatchAddr = 0x00604559;
static const UInt32 kActorSwimBreath_CalcMax1_RetnAddr	= 0x0060455E;
static const UInt32 kActorSwimBreath_CalcMax2_PatchAddr = 0x005E01C4;
static const UInt32 kActorSwimBreath_CalcMax2_RetnAddr	= 0x005E01C9;

static __declspec(naked) void Hook_ActorSwimBreath_CalcMax1()
{
	__asm
	{
		pushad
		push	ebp
		call	GetActorMaxSwimBreath
		popad
		jmp		[kActorSwimBreath_CalcMax1_RetnAddr]
	}
}
static __declspec(naked) void Hook_ActorSwimBreath_CalcMax2()
{
	__asm
	{
		pushad
		push	esi
		call	GetActorMaxSwimBreath
		popad
		jmp		[kActorSwimBreath_CalcMax2_RetnAddr]
	}
}

typedef std::map<Actor*,float> MaxBreathOverrideMapT;
MaxBreathOverrideMapT s_MaxSwimmingBreathOverrideMap;

void SetActorMaxSwimBreath(Actor* actor, float nuMax)
{
	static bool s_hooked = false;
	if (!s_hooked)
	{
		s_hooked = true;
		WriteRelJump(kActorSwimBreath_CalcMax1_PatchAddr, (UInt32)Hook_ActorSwimBreath_CalcMax1);
		WriteRelJump(kActorSwimBreath_CalcMax2_PatchAddr, (UInt32)Hook_ActorSwimBreath_CalcMax2);
	}

	MaxBreathOverrideMapT::iterator it = s_MaxSwimmingBreathOverrideMap.find(actor);
	if (nuMax > 0)
	{
		if (it != s_MaxSwimmingBreathOverrideMap.end())
		{
			it->second = nuMax;
		}
		else
		{
			s_MaxSwimmingBreathOverrideMap[ actor ] = nuMax;
		}
	}
	else
	{
		if (it != s_MaxSwimmingBreathOverrideMap.end())
		{
			s_MaxSwimmingBreathOverrideMap.erase(it);
		}
	}
}

float __stdcall GetActorMaxSwimBreath(Actor* actor)
{
	HighProcess* highProcess = (HighProcess*)actor->process;
	MaxBreathOverrideMapT::iterator it = s_MaxSwimmingBreathOverrideMap.find(actor);

	if (it != s_MaxSwimmingBreathOverrideMap.end())
	{
		return it->second;
	}
	else
	{
		typedef float (* _fn)(UInt32 Endurance);
		const _fn fn = (_fn)kActorSwimBreath_CalcMax_CallAddr;
		return fn(actor->GetActorValue(5));
	}
}


static const UInt32 kActorSwimBreath_Override_PatchAddr = 0x006045CA;
static const UInt32 kActorSwimBreath_Override_RetnCanBreathAddr = 0x006045DF; // code for actor that can breath, sets currentBreath to maxBreath
static const UInt32 kActorSwimBreath_Override_RetnNoBreathAddr	= 0x006045F9; // code for actor that cannot breath 
static const UInt32 kActorSwimBreath_Override_RetnNoTickAddr	= 0x00604635; // skips code that ticks the current breath when underwater while keeping the rest
static const UInt32 kActorSwimBreath_Override_RetnSkipAddr		= 0x00604763; // jumps to the end of the breath code
static const UInt32 kActorSwimBreath_Override_RetnSkip2Addr		= 0x00604879; // also skips breathing menu code if actor is player

enum 
{
	kActorSwimBreath_IsUnderWater	= 1,	 // we'll treat the boolean as a flag for more compact code
	// the rest are possible override states  
	kActorSwimBreath_CanBreath		= 1 << 1, // forces code to think the actor can breath, no other changes so standard behaviour of setting 'curBreath' to 'maxBreath' applies
	kActorSwimBreath_NoBreath		= 2 << 1, // forces the code to think the actor cannot breath, no other changes so standard behaviour applies
	kActorSwimBreath_NoTick			= 3 << 1, // stops the game from changing 'curBreath' each frame (when underwater) but still causes health damage when 'curBreath' is set below 0
	kActorSwimBreath_SkipBreath		= 4 << 1, // completely skips breath code (BreathMenu not included)
	kActorSwimBreath_SkipBreath2	= 5 << 1, // completely skips breath code (BreathMenu included)
};
typedef std::map<Actor*,UInt32> ActorSwimmingBreathMapT;
ActorSwimmingBreathMapT s_ActorSwimmingBreathMap;

bool SetActorSwimBreathOverride(Actor* actor, UInt32 state)
{
	if (state >= 0 && state < 4)
	{
		ActorSwimmingBreathMapT::iterator it = s_ActorSwimmingBreathMap.find(actor);
		if (it != s_ActorSwimmingBreathMap.end())
		{
			it->second = ((it->second & kActorSwimBreath_IsUnderWater) | (state << 1));
			return true;
		}
		s_ActorSwimmingBreathMap.insert(std::map<Actor*,UInt32>::value_type(actor, state << 1));
		return true;
	}
	return false;
}

UInt32 __stdcall HandleActorSwimBreath(Actor* actor, HighProcess* process, bool canBreath, bool isUnderWater, float* curBreath, float* maxBreath)
{
	UInt32 retnAddr = 0;
	//ensure curBreath is initialized
	*curBreath = process->swimBreath;

	//find & update state
	ActorSwimmingBreathMapT::iterator it = s_ActorSwimmingBreathMap.find(actor);
	//only fire events when actor is already registered
	if (it != s_ActorSwimmingBreathMap.end())
	{
		if ( (it->second & kActorSwimBreath_IsUnderWater) != isUnderWater )
		{
			//Console_Print("OnWater%s for (%08X)", isUnderWater ? "Dive" : "Surface", actor->refID);
			//_MESSAGE("OnWater%s for '%s' (%08X)", isUnderWater ? "Dive" : "Surface", GetActorFullName(actor), actor->refID);
			HandleEvent(EventManager::kEventID_OnWaterSurface + isUnderWater, actor, NULL);
		}
	}
	UInt32 breathState = s_ActorSwimmingBreathMap[actor];
	breathState = (breathState & ~kActorSwimBreath_IsUnderWater) | (isUnderWater != 0);
	s_ActorSwimmingBreathMap[actor] = breathState;

	if ( actor == *g_thePlayer && IsGodMode() )
	{
		// GodMode overrides everything
		retnAddr = kActorSwimBreath_Override_RetnCanBreathAddr;
	}
	else if ( breathState > 1 )
	{
		// override is in place
		switch ( breathState & ~kActorSwimBreath_IsUnderWater )
		{
		case kActorSwimBreath_CanBreath:
			retnAddr = kActorSwimBreath_Override_RetnCanBreathAddr;
			break;
		case kActorSwimBreath_NoBreath:
			retnAddr = kActorSwimBreath_Override_RetnNoBreathAddr;
			break;
		case kActorSwimBreath_NoTick:
			retnAddr = kActorSwimBreath_Override_RetnNoTickAddr;
			break;
		case kActorSwimBreath_SkipBreath:
			retnAddr = kActorSwimBreath_Override_RetnSkipAddr;
			break;
		default:
			//_MESSAGE("Invalid swim breath override for '%s' (%08X)", GetActorFullName(actor), actor->refID);
			retnAddr = canBreath ? kActorSwimBreath_Override_RetnCanBreathAddr : kActorSwimBreath_Override_RetnNoBreathAddr;
			break;
		}
	}
	else
	{
		retnAddr = canBreath ? kActorSwimBreath_Override_RetnCanBreathAddr : kActorSwimBreath_Override_RetnNoBreathAddr;
	}

	// update stack variables in case SetActor(Max)SwimBreath was called inside any of the event handlers
	*curBreath = process->swimBreath;
	*maxBreath = GetActorMaxSwimBreath(actor);

	//ASSERT(retnAddr != NULL);

	return retnAddr;
}
static __declspec(naked) void Hook_ActorSwimBreath_Override()
{
	//TESObjectREFR::IsUnderWater(Vector3& pos, TESObjectCELL* cell, float thresholdFactor); =0x005E06C0
	static UInt32 retnAddr;
	__asm
	{
		// ebx = canBreath (bool)				determined based on being underwater or not and if the actor breaths air or water
		// esp+17h = isSwimming (bool)			from TESObjectREFR::IsUnderWater(...) w/ threshold of 70%
		// esp+15h = isUnderWater (bool)		from TESObjectREFR::IsUnderWater(...) w/ threshold of 87.5%
		// esp+28h = currentBreath (float)		not yet set at this point, used to change/update stack variable with our handler (if needed)
		// esp+24h = maxBreath (float)			already calculated at this point (possibly by our other hook), may also need to be changed/updated with our handler
		// 
		mov		al, [esp+15h]
		lea		edx, [esp+24h]
		lea		ecx, [esp+28h]
		pushad
		push	edx		// float*
		push	ecx		// float*
		push	eax		// bool
		push	ebx		// bool
		mov		eax, [ebp+58h]
		push	eax		// HighProcess*
		push	ebp		// Actor*
		call	HandleActorSwimBreath
		mov		[retnAddr], eax
		popad
		mov		eax, [retnAddr]
		jmp		eax
	}
}

void InstallSwimmingBreathHook()
{
	WriteRelJump(kActorSwimBreath_Override_PatchAddr, (UInt32)Hook_ActorSwimBreath_Override);
}

UInt32 EventIDForMask(UInt32 eventMask)
{
	switch (eventMask) {
		case kEventMask_OnActivate:
			return kEventID_OnActivate;
		case ScriptEventList::kEvent_OnHit:
			return kEventID_OnHit;
		case ScriptEventList::kEvent_OnHitWith:
			return kEventID_OnHitWith;
		case ScriptEventList::kEvent_OnMagicEffectHit:
			return kEventID_OnMagicEffectHit;
		case ScriptEventList::kEvent_OnActorEquip:
			return kEventID_OnActorEquip;
		case ScriptEventList::kEvent_OnDeath:
			return kEventID_OnDeath;
		case ScriptEventList::kEvent_OnMurder:
			return kEventID_OnMurder;
		case ScriptEventList::kEvent_OnKnockout:
			return kEventID_OnKnockout;
		case ScriptEventList::kEvent_OnActorUnequip:
			return kEventID_OnActorUnequip;
		case ScriptEventList::kEvent_OnAlarm_Trespass:
			return kEventID_OnAlarm_Trespass;
		case ScriptEventList::kEvent_OnAlarm_Steal:
			return kEventID_OnAlarm_Steal;
		case ScriptEventList::kEvent_OnAlarm_Attack:
			return kEventID_OnAlarm_Attack;
		case ScriptEventList::kEvent_OnAlarm_Pickpocket:
			return kEventID_OnAlarm_Pickpocket;
		case ScriptEventList::kEvent_OnAlarm_Murder:
			return kEventID_OnAlarm_Murder;
		case ScriptEventList::kEvent_OnPackageStart:
			return kEventID_OnPackageStart;
		case ScriptEventList::kEvent_OnPackageDone:
			return kEventID_OnPackageDone;
		case ScriptEventList::kEvent_OnPackageChange:
			return kEventID_OnPackageChange;
		case ScriptEventList::kEvent_OnStartCombat:
			return kEventID_OnStartCombat;
		default:
			return kEventID_INVALID;
	}
}

UInt32 EventIDForMessage(UInt32 msgID)
{
	switch (msgID) {
		case OBSEMessagingInterface::kMessage_LoadGame:
			return kEventID_LoadGame;
		case OBSEMessagingInterface::kMessage_SaveGame:
			return kEventID_SaveGame;
		case OBSEMessagingInterface::kMessage_ExitGame:
			return kEventID_ExitGame;
		case OBSEMessagingInterface::kMessage_ExitGame_Console:
			return kEventID_QQQ;
		case OBSEMessagingInterface::kMessage_ExitToMainMenu:
			return kEventID_ExitToMainMenu;
		case OBSEMessagingInterface::kMessage_PostLoadGame:
			return kEventID_PostLoadGame;
		default:
			return kEventID_INVALID;
	}
}

typedef std::vector<EventInfo*> EventInfoList;
static EventInfoList s_eventInfos;

UInt32 EventManager::EventIDForString(const char* eventStr)
{
	std::string name(eventStr);
	MakeLower(name);
	eventStr = name.c_str();
	UInt32 numEventInfos = s_eventInfos.size ();
	for (UInt32 i = 0; i < numEventInfos; i++) {
		if (!strcmp(eventStr, s_eventInfos[i]->name.c_str())) {
			return i;
		}
	}

	return kEventID_INVALID;
}

bool EventCallback::Equals(const EventCallback& rhs) const
{
	return (script == rhs.script &&
		object == rhs.object &&
		source == rhs.source &&
		callingObj == rhs.callingObj);
}

typedef std::list<EventCallback>	CallbackList;

bool RemoveHandler(UInt32 id, EventCallback& handler);
bool RemoveHandler(UInt32 id, Script* fnScript);

class EventHandlerCaller : public InternalFunctionCaller
{
public:
	EventHandlerCaller(Script* script, EventInfo* eventInfo, void* arg0, void* arg1, TESObjectREFR* callingObj = NULL)
		: InternalFunctionCaller(script, callingObj), m_eventInfo(eventInfo)
	{
		UInt8 numArgs = 2;
		if (!arg1)
			numArgs = 1;
		if (!arg0)
			numArgs = 0;

		SetArgs(numArgs, arg0, arg1);
	}

	virtual bool ValidateParam(UserFunctionParam* param, UInt8 paramIndex)
	{
		return param->varType == m_eventInfo->paramTypes[paramIndex];
	}

	virtual bool PopulateArgs(ScriptEventList* eventList, FunctionInfo* info) {
		// make sure we've got the same # of args as expected by event handler
		DynamicParamInfo& dParams = info->ParamInfo();
		if (dParams.NumParams() != m_eventInfo->numParams || dParams.NumParams() > 2) {
			ShowRuntimeError(m_script, "Number of arguments to function script does not match those expected for event");
			return false;
		}

		return InternalFunctionCaller::PopulateArgs(eventList, info);
	}

private:
	EventInfo		* m_eventInfo;
};

// stack of event names pushed when handler invoked, popped when handler returns
// used by GetCurrentEventName
std::stack<std::string> s_eventStack;

// some events are best deferred until Tick() invoked rather than being handled immediately
// this stores info about such an event. Currently unused.
struct DeferredCallback
{
	DeferredCallback(CallbackList::iterator& _iter, TESObjectREFR* _callingObj, void* _arg0, void* _arg1, EventInfo* _eventInfo)
		: iterator(_iter), callingObj(_callingObj), arg0(_arg0), arg1(_arg1), eventInfo(_eventInfo) { }

	CallbackList::iterator	iterator;
	TESObjectREFR			* callingObj;
	void					* arg0;
	void					* arg1;
	EventInfo				* eventInfo;
};

std::list<DeferredCallback> s_deferredCallbacks;

void __stdcall HandleEventForCallingObject(UInt32 id, TESObjectREFR* callingObj, void* arg0, void* arg1)
{
	ScopedLock lock(s_criticalSection);

	EventInfo* eventInfo = s_eventInfos[id];
	if (eventInfo->callbacks) {
		for (CallbackList::iterator iter = eventInfo->callbacks->begin(); iter != eventInfo->callbacks->end(); ) {
			if (iter->IsRemoved()) {
				if (!iter->IsInUse()) {
					iter = eventInfo->callbacks->erase(iter);
				}
				else {
					++iter;
				}

				continue;
			}
			if (id == kEventID_EventKey || id == kEventID_EventControl) {
				if (arg0 != iter->source || arg1 != iter->object) {
					++iter;
					continue;
				}
			}
			else {
				// Check filters
				if (iter->source) {
					// special-case - check the source filter against the second arg, the attacker
					if (id == kEventID_OnHealthDamage) {
						if ((TESObjectREFR*)arg1 != iter->source) {
							++iter;
							continue;
						}
					}
					else if (!((TESObjectREFR*)arg0 == iter->source)) {
						++iter;
						continue;
					}
				}

				if (iter->callingObj && !(callingObj == iter->callingObj)) {
					++iter;
					continue;
				}

				if (iter->object) {
					if (id == kEventID_OnMagicEffectHit) {
						EffectSetting* setting = OBLIVION_CAST(iter->object, TESForm, EffectSetting);
						if (setting && setting->effectCode != (UInt32)arg1) {
							++iter;
							continue;
						}
					}
					else if (!(iter->object == (TESForm*)arg1)) {
						++iter;
						continue;
					}
				}
			}
			if (GetCurrentThreadId() != g_mainThreadID) {
				// avoid potential issues with invoking handlers outside of main thread by deferring event handling
				if (!iter->IsRemoved()) {
					iter->SetInUse(true);
					s_deferredCallbacks.push_back(DeferredCallback(iter, callingObj, arg0, arg1, eventInfo));
					++iter;
				}
			}
			else {
				// handle immediately
				bool bWasInUse = iter->IsInUse();
				iter->SetInUse(true);
				s_eventStack.push(eventInfo->name);
				if (iter->script) {
					ScriptToken* result = UserFunctionManager::Call(EventHandlerCaller(iter->script, eventInfo, arg0, arg1, callingObj));
					// result is unused
					delete result;
				}
				else {
					iter->eventFunction(arg0,arg1, callingObj); //TODO there is no validation of parameters. Add it.
					//TODO actually restructure this entire mess.
				}

				s_eventStack.pop();
				iter->SetInUse(bWasInUse);

				// it's possible the handler decided to remove itself, so take care of that, being careful
				// not to remove a callback that is needed for deferred invocation
				if (!bWasInUse && iter->IsRemoved()) {
					iter = eventInfo->callbacks->erase(iter);
				}
				else {
					++iter;
				}

			}
		}
	}
}

void __stdcall HandleEvent(UInt32 id, void * arg0, void * arg1)
{
	// initial implementation didn't support a calling object; pass through to impl which does
	HandleEventForCallingObject(id, NULL, arg0, arg1);
}

////////////////
// public API
///////////////

std::string GetCurrentEventName()
{
	ScopedLock lock(s_criticalSection);

	return s_eventStack.empty() ? "" : s_eventStack.top();
}

bool SetHandler(const char* eventName, EventCallback& handler)
{
	ScopedLock lock(s_criticalSection);

	UInt32 id = EventIDForString (eventName);
	if (kEventID_INVALID == id)
	{
		// have to assume registering for a user-defined event which has not been used before this point
		id = s_eventInfos.size();
		s_eventInfos.push_back (new EventInfo (eventName, kEventParams_OneArray, 1));
	}

	if (id < s_eventInfos.size()) {
		EventInfo* info = s_eventInfos[id];
		// is hook installed for this event type?
		if (info->installHook) {
			if (*(info->installHook)) {
				// if this hook is used by more than one event type, prevent it being installed a second time
				(*info->installHook)();
				*(info->installHook) = NULL;
			}
			// mark event as having had its hook installed
			info->installHook = NULL;
		}

		if (!info->callbacks) {
			info->callbacks = new CallbackList();
		}
		else {
			// if an existing handler matches this one exactly, don't duplicate it
			for (CallbackList::iterator iter = info->callbacks->begin(); iter != info->callbacks->end(); ++iter) {
				if (iter->Equals(handler)) {
					// may be re-adding a previously removed handler, so clear the Removed flag
					iter->SetRemoved(false);
					return false;
				}
			}
		}
		info->callbacks->push_back(handler);
		return true;
	}
	else {
		return false; 
	}
}

bool EventHandlerExist(const char*  ev, EventCallback& handler){
	ScopedLock lock(s_criticalSection);

	UInt32 eventType = EventIDForString(ev);
	bool found = false;
	if (eventType < s_eventInfos.size() && s_eventInfos[eventType]->callbacks) {
		CallbackList* callbacks = s_eventInfos[eventType]->callbacks;
		for (CallbackList::iterator iter = callbacks->begin(); iter != callbacks->end(); ++iter ) {
			if (iter->Equals(handler)) {
				found = true;
				break;
			}
		}
	}
	return found;
}


bool RemoveHandler(const char* id, EventCallback& handler)
{
	ScopedLock lock(s_criticalSection);
	//TODO remove event type if custom and not having an handler anymore
	UInt32 eventType = EventIDForString(id);
	bool bRemovedAtLeastOne = false;
	if (eventType < s_eventInfos.size() && s_eventInfos[eventType]->callbacks) {
		CallbackList* callbacks = s_eventInfos[eventType]->callbacks;
		for (CallbackList::iterator iter = callbacks->begin(); iter != callbacks->end(); ) {
			if (iter->script == handler.script) {
				bool bMatches = true;
				if (eventType == kEventID_OnHealthDamage) { 
					if (handler.callingObj && handler.callingObj != iter->object) {	// OnHealthDamage special-casing
						bMatches = false;
					}
				}
				else if (handler.object && handler.object != iter->object) {
					bMatches = false;
				}
				
				if (handler.source && handler.source != iter->source) {
					bMatches = false;
				}

				if (bMatches) {
					if (iter->IsInUse()) {
						// this handler is currently active, flag it for later removal
						iter->SetRemoved(true);
						++iter;
					}
					else {
						iter = callbacks->erase(iter);
					}

					bRemovedAtLeastOne = true;
				}
				else {
					++iter;
				}
			}
			else {
				++iter;
			}
		}
	}
	
	return bRemovedAtLeastOne;
}

bool TryGetReference(TESObjectREFR* refr)
{
	// ### HACK HACK HACK
	// MarkEventList() may have been called for a BaseExtraList not associated with a TESObjectREFR
	bool bIsRefr = false;
	__try 
	{
		switch (*((UInt32*)refr)) {
			case kVtbl_PlayerCharacter:
			case kVtbl_Character:
			case kVtbl_Creature:
			case kVtbl_ArrowProjectile:
			case kVtbl_MagicBallProjectile:
			case kVtbl_MagicBoltProjectile:
			case kVtbl_MagicFogProjectile:
			case kVtbl_MagicSprayProjectile:
			case kVtbl_TESObjectREFR:
				bIsRefr = true;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		bIsRefr = false;
	}

	return bIsRefr;
}

void __stdcall HandleGameEvent(UInt32 eventMask, TESObjectREFR* source, TESForm* object)
{
	if (!TryGetReference(source)) {
		return;
	}

	ScopedLock lock(s_criticalSection);

	// ScriptEventList can be marked more than once per event, cheap check to prevent sending duplicate events to scripts
	if (source != s_lastObj || object != s_lastTarget || eventMask != s_lastEvent) {
		s_lastObj = source;
		s_lastEvent = eventMask;
		s_lastTarget = object;
	}
	else {
		// duplicate event, ignore it
		return;
	}

	UInt32 eventID = EventIDForMask(eventMask);
	if (eventID != kEventID_INVALID) {
		// special-case OnMagicEffectHit
		if (eventID == kEventID_OnMagicEffectHit) {
			EffectSetting* setting = OBLIVION_CAST(object, TESForm, EffectSetting);
			HandleEvent(eventID, source, setting ? (void*)setting->effectCode : 0);
			// also send OnMagicEffectHit2, for OBME plugin support
			HandleEvent(kEventID_OnMagicEffectHit2, source, setting);
		}
		else if (eventID == kEventID_OnHitWith) {
			// special check for OnHitWith, since it gets called redundantly
			if (source != s_lastOnHitWithActor || object != s_lastOnHitWithWeapon) {
				s_lastOnHitWithActor = source;
				s_lastOnHitWithWeapon = object;
				HandleEvent(eventID, source, object);
			}
		}
		else if (eventID == kEventID_OnHit) {
			if (source != s_lastOnHitVictim || object != s_lastOnHitAttacker) {
				s_lastOnHitVictim = source;
				s_lastOnHitAttacker = object;
				HandleEvent(eventID, source, object);
			}
		}
		else
			HandleEvent(eventID, source, object);
	}
}

void HandleOBSEMessage(UInt32 msgID, void* data)
{
	UInt32 eventID = EventIDForMessage(msgID);
	if (eventID != kEventID_INVALID)
		HandleEvent(eventID, data, NULL);
}

bool DispatchUserDefinedEvent (const char* eventName, Script* sender, UInt32 argsArrayId, const char* senderName)
{
	ScopedLock lock(s_criticalSection);

	// does an EventInfo entry already exist for this event?
	UInt32 eventID = EventIDForString (eventName);
	if (eventID < kEventID_UserDefinedMIN) {
		_MESSAGE("User Dispatch for internal event not supported");
		return false;
	}
	if (kEventID_INVALID == eventID)
	{
		// The event isn't found, event should not be created by Dispatch but only on Set. 
		//TODO maybe defer the dispatch for unregistered events to avoid possible loss of events 
		_MESSAGE("Dispatch on unregistered event %s. Ignoring", eventName);
		return false;
	}

	// get or create args array
	if (argsArrayId == 0) //Still should be avoided to be deleted. 
		argsArrayId = g_ArrayMap.Create (kDataType_String, false, sender ?  sender->GetModIndex () : 0xFF);
	else if (!g_ArrayMap.Exists (argsArrayId) || g_ArrayMap.GetKeyType (argsArrayId) != kDataType_String)
		return false;

	// populate args array
	g_ArrayMap.SetElementString (argsArrayId, "eventName", eventName);
	if (nullptr == senderName  && sender != nullptr)
		senderName = (*g_dataHandler)->GetNthModName(sender->GetModIndex ());
	else if (senderName == nullptr) {
		senderName = "OBSE";
	}
	g_ArrayMap.SetElementString (argsArrayId, "eventSender", senderName);

	// dispatch
	HandleEvent (eventID, (void*)argsArrayId, NULL);
	return true;
}

EventInfo::~EventInfo()
{
	if (callbacks) {
		delete callbacks;
		callbacks = NULL;
	}
}

void Tick()
{
	ScopedLock lock(s_criticalSection);

	// handle deferred events
	if (s_deferredCallbacks.size()) {
		std::list< std::list<DeferredCallback>::iterator > s_removedCallbacks;

		std::list<DeferredCallback>::iterator iter = s_deferredCallbacks.begin();
		while (iter != s_deferredCallbacks.end()) {
			if (!iter->iterator->IsRemoved()) {
				s_eventStack.push(iter->eventInfo->name);
				if (iter->iterator->script) {
					ScriptToken* result = UserFunctionManager::Call(
						EventHandlerCaller(iter->iterator->script, iter->eventInfo, iter->arg0, iter->arg1, iter->callingObj));
					// result is unused
					delete result;
				}
				else {
					iter->iterator->eventFunction(iter->arg0, iter->arg1, iter->callingObj); //TODO param validation
				}
				s_eventStack.pop();

				if (iter->iterator->IsRemoved()) {
					s_removedCallbacks.push_back(iter);
					++iter;
				}
				else {
					iter = s_deferredCallbacks.erase(iter);
				}
			}
		}

		// get rid of any handlers removed during processing above
		while (s_removedCallbacks.size()) {
			(*s_removedCallbacks.begin())->eventInfo->callbacks->erase(iter->iterator);
			s_removedCallbacks.pop_front();
		}

		s_deferredCallbacks.clear();
	}


	// cleanup temporary hook data
	for (MaxBreathOverrideMapT::iterator itr = s_MaxSwimmingBreathOverrideMap.begin(); itr != s_MaxSwimmingBreathOverrideMap.end();)
	{
		if (g_actorProcessManager->highActors.IndexOf(itr->first) == -1 && itr->first != (*g_thePlayer))
			itr = s_MaxSwimmingBreathOverrideMap.erase(itr);						// remove the actor from the map if not in high process
		else
			itr++;
	}

	for (ActorSwimmingBreathMapT::iterator itr = s_ActorSwimmingBreathMap.begin(); itr != s_ActorSwimmingBreathMap.end();)
	{
		if (g_actorProcessManager->highActors.IndexOf(itr->first) == -1 && itr->first != (*g_thePlayer))
			itr = s_ActorSwimmingBreathMap.erase(itr);
		else
			itr++;
	}

	s_lastObj = NULL;
	s_lastTarget = NULL;
	s_lastEvent = NULL;
	s_lastOnHitWithActor = NULL;
	s_lastOnHitWithWeapon = NULL;
	s_lastOnHitVictim = NULL;
	s_lastOnHitAttacker = NULL;
}

void Init()
{
#define EVENT_INFO(name, params, hookInstaller) s_eventInfos.push_back (new EventInfo (name, params, params ? sizeof(params) : 0, hookInstaller));

	EVENT_INFO("onhit", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onhitwith", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onmagiceffecthit", kEventParams_GameMGEFEvent, &s_MainEventHook)
	EVENT_INFO("onactorequip", kEventParams_GameEvent, &s_ActorEquipHook)
	EVENT_INFO("ondeath", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onmurder", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onknockout", kEventParams_OneRef, &s_MainEventHook)
	EVENT_INFO("onactorunequip", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onalarm trespass", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onalarm steal", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onalarm attack", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onalarm pickpocket", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onalarm murder", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onpackagechange", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onpackagestart", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onpackagedone", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onstartcombat", kEventParams_GameEvent, &s_MainEventHook)
	EVENT_INFO("onmagiceffecthit2", kEventParams_GameEvent, &s_MainEventHook)

	EVENT_INFO("onactivate", kEventParams_GameEvent, &s_ActivateHook)
	EVENT_INFO("onvampirefeed", NULL, &s_VampireHook)
	EVENT_INFO("onskillup", kEventParams_OneInteger, &s_SkillUpHook)
	EVENT_INFO("onscriptedskillup", kEventParams_TwoIntegers, &s_ModPCSHook)
	EVENT_INFO("onmapmarkeradd", kEventParams_OneRef, &s_MapMarkerHook)
	EVENT_INFO("onspellcast", kEventParams_GameEvent, &s_SpellScrollHook)
	EVENT_INFO("onscrollcast", kEventParams_GameEvent, &s_SpellScrollHook)
	EVENT_INFO("onfallimpact", kEventParams_OneRef, &s_FallImpactHook)
	EVENT_INFO("onactordrop", kEventParams_GameEvent, NULL)
	EVENT_INFO("ondrinkpotion", kEventParams_GameEvent, &s_DrinkHook)
	EVENT_INFO("oneatingredient", kEventParams_GameEvent, &s_EatIngredHook)
	EVENT_INFO("onnewgame", NULL, NULL)
	EVENT_INFO("onhealthdamage", kEventParams_OneFloat_OneRef, &s_HealthDamageHook)
	EVENT_INFO("onsoultrap", kEventParams_GameEvent, &s_SoulTrapHook)
	EVENT_INFO("onraceselectioncomplete", NULL, &s_OnRaceSelectionCompleteHook)

	EVENT_INFO("onattack", kEventParams_OneRef, &s_MeleeAttackHook)
	EVENT_INFO("onrelease", kEventParams_OneRef, &s_MeleeReleaseHook)
	EVENT_INFO("onbowattack", kEventParams_OneRef, &s_BowAttackHook)
	EVENT_INFO("onbowarrowattach", kEventParams_OneRef, &s_BowReleaseHook)	// undocumented, not hugely useful
	EVENT_INFO("onblock", kEventParams_OneRef, &s_BlockHook)
	EVENT_INFO("onrecoil", kEventParams_OneRef, &s_RecoilHook)
	EVENT_INFO("onstagger", kEventParams_OneRef, &s_StaggerHook)
	EVENT_INFO("ondodge", kEventParams_OneRef, &s_DodgeHook)

	EVENT_INFO("onenchant", kEventParams_OneRef, NULL)
	EVENT_INFO("oncreatespell", kEventParams_OneRef, NULL)
	EVENT_INFO("oncreatepotion", kEventParams_OneRef_OneInt, NULL)

	EVENT_INFO("onquestcomplete", kEventParams_OneRef, &s_OnQuestCompleteHook)
	EVENT_INFO("onmagiccast", kEventParams_GameEvent, &s_OnMagicCastHook)
	EVENT_INFO("onmagicapply", kEventParams_GameEvent, &s_OnMagicApplyHook)
	EVENT_INFO("onwatersurface", kEventParams_OneRef, &s_OnWaterDiveSurfaceHook)
	EVENT_INFO("onwaterdive", kEventParams_OneRef, &s_OnWaterDiveSurfaceHook)

	EVENT_INFO("loadgame", kEventParams_OneString, NULL)
	EVENT_INFO("savegame", kEventParams_OneString, NULL)
	EVENT_INFO("exitgame", NULL, NULL)
	EVENT_INFO("mainmenu", NULL, NULL)
	EVENT_INFO("qqq", NULL, NULL)
	EVENT_INFO("postloadgame", kEventParams_OneInteger, NULL)
	EVENT_INFO("saveini", kEventParams_OneInteger, &s_IniHook)
	EVENT_INFO("OnKeyEvent", kEventParams_TwoIntegers, nullptr)
	EVENT_INFO("OnControlEvent", kEventParams_TwoIntegers, nullptr)
	ASSERT (kEventID_InternalMAX == s_eventInfos.size());

#undef EVENT_INFO
}


};	// namespace

namespace PluginAPI {
	bool DispatchEvent(const char* eventName, const char* sender, UInt32 arrayId) {
		if (EventManager::s_eventInfos.empty()) return false;
		return EventManager::DispatchUserDefinedEvent(eventName, nullptr, arrayId, sender);
	}
	/*
		If you want to register an handler for a native event, register the event info with  RegisterEventNative before calling this or registering the event in scripts
	*/
	bool RegisterEvent(const char* eventName, EventManager::EventFunc func, void* arg0 , void* arg1, TESObjectREFR* refr) {
		if (EventManager::s_eventInfos.empty()) return false;
		if (eventName== nullptr || func == nullptr) return false;
		EventManager::EventCallback event(func, (TESObjectREFR*)arg0, (TESObjectREFR*)arg1, refr);
		return  EventManager::SetHandler(eventName, event);
	}
	bool UnRegisterEvent(const char* eventName, EventManager::EventFunc func, void* arg0, void* arg1, TESObjectREFR* refr) { 
		if (EventManager::s_eventInfos.empty()) return false;
		if (eventName == nullptr || func == nullptr) return false;
		EventManager::EventCallback event(func, (TESObjectREFR*)arg0, (TESObjectREFR*)arg1, refr);
		return  EventManager::RemoveHandler(eventName, event);
	}
	bool IsEventRegistered(const char* eventName, EventManager::EventFunc func, void* arg0, void* arg1, TESObjectREFR* refr) {
		if (EventManager::s_eventInfos.empty()) return false;
		if (eventName == nullptr || func == nullptr) return false;
		EventManager::EventCallback event(func, (TESObjectREFR*)arg0, (TESObjectREFR*)arg1, refr);
		return  EventManager::EventHandlerExist(eventName, event);

	}
	
	HandleEventFunc  GetHandleGameEventFuncAddress() {
		return EventManager::HandleGameEvent;
	}

	bool RegisterEventNative(PluginEventInfo* info) {
		if (EventManager::s_eventInfos.empty()) return false;
		EventManager::EventInfo* event = new EventManager::EventInfo(info->name, info->paramTypes, info->numParams, &info->installer);
		EventManager::s_eventInfos.push_back(event);
		return true;
	}
}