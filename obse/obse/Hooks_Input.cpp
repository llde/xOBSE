#include "Hooks_Input.h" 
#include "obse_common/SafeWrite.h"

OSInputGlobalsEx* g_inputGlobal = nullptr;

void OSInputGlobalsEx::SetMaskKey(UInt16 keycode) {
	KeyMaskState[keycode] |= kStateDisabled;
}

void OSInputGlobalsEx::SetUnmaskKey(UInt16 keycode) {
	KeyMaskState[keycode] &= ~kStateDisabled;
}

UInt8 OSInputGlobalsEx::GetMaskStatusKey(UInt8 keycode) {
	return KeyMaskState[keycode];
}

UInt8 OSInputGlobalsEx::GetSignalStatusKey(UInt16 keycode) {
	return (KeyMaskState[keycode] & kStateSignalled) == kStateSignalled;
}
UInt8 OSInputGlobalsEx::GetSignalStatus(UInt16 keycode) {
	return (KeyMaskState[keycode] & kStateSignalled) == kStateSignalled;
}
UInt8 OSInputGlobalsEx::GetSignalStatusMouse(UInt8 keycode) {
	return 0; 
}


//TODO Tap and Hammer keys
void OSInputGlobalsEx::InputPollFakeHandle() {
	for (UInt16 idx = 0; idx <= 255; idx++) {
		KeyMaskState[idx] &= ~kStateSignalled;
		if (KeyMaskState[idx] == 0) {
			continue;
		}
		if(KeyMaskState[idx] == kStateDisabled){
			if (CurrentKeyState[idx] != 0) {  //Presses are 0x80 ?
				CurrentKeyState[idx] = 0;
				KeyMaskState[idx] |= kStateSignalled;
			}
		}
	}

}

void __fastcall InputPollFakeHandleNoMouse(OSInputGlobals* input) { _MESSAGE("Macche è sto porccoddio"); }


__declspec(naked) void PollInputHook() {
	__asm {
		jle short loc_cont
		pop edi
		pop esi
		pushad
		call OSInputGlobalsEx::InputPollFakeHandle
		popad
		retn

	loc_cont:
		jmp [kLoc_403C40]
	}

}
__declspec(naked) void PollInputNoMouseHook() {
	__asm {
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp , 8
		pushad 
		mov ecx, ebx //check the InputGlobal should be on ebx before this
		call InputPollFakeHandleNoMouse
		popad
		retn
	}

}
OSInputGlobalsEx* __thiscall OSInputGlobalsEx::InitializeEx(IDirectInputDevice8* device) {
	ThisStdCall(kInitializeInputGlobals, this, device);
	ZeroMemory(this + sizeof(OSInputGlobals), sizeof(DIMOUSESTATE2) + 256);
	g_inputGlobal = this;
	return this;
}

void Hook_Input_Init() {
	SafeWrite32(kInputGlobalAllocSize, sizeof(OSInputGlobals) + sizeof(UInt8[256]) + sizeof(DIMOUSESTATE2));
	OSInputGlobalsEx* (__thiscall OSInputGlobalsEx::* InitializeExCall)(IDirectInputDevice8*) = &OSInputGlobalsEx::InitializeEx;
	WriteRelCall(kInputInitializeCallAddr, (UInt32) *((void**) &InitializeExCall));
	WriteRelJump(kPollEndHook, (UInt32) &PollInputHook);
	WriteRelJump(kPollEndNoMouseHook, (UInt32)&PollInputNoMouseHook); //TODO clean leftovers
}


