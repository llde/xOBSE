#include "Hooks_Input.h" 
#include "obse_common/SafeWrite.h"

void SetMaskKey(UInt16 keycode) {
	KeyState[keycode] |= kStateDisabled;
}

void SetUnmaskKey(UInt16 keycode) {
	KeyState[keycode] &= ~kStateDisabled;
}

UInt8 GetMaskStatusKey(UInt8 keycode) {
	return KeyState[keycode];
}

UInt8 GetSignalStatusKey(UInt16 keycode) {
	return (KeyState[keycode] & kStateSignalled) == kStateSignalled;
}
UInt8 GetSignalStatus(UInt16 keycode) {
	return (KeyState[keycode] & kStateSignalled) == kStateSignalled;
}
UInt8 GetSignalStatusMouse(UInt8 keycode) {
	return 0; // (KeyState[keycode] & kStateSignalled) == kStateSignalled;
}


//TODO Tap and Hammer keys
void __fastcall InputPollFakeHandle(OSInputGlobals* input) {
	for (UInt16 idx = 0; idx <= 255; idx++) {
		KeyState[idx] &= ~kStateSignalled;
		if (KeyState[idx] == 0) { 
			continue;
		}
		if(KeyState[idx] == kStateDisabled){
			if (input->CurrentKeyState[idx] != 0) {  //Presses are 0x80 ?
				input->CurrentKeyState[idx] = 0;
				KeyState[idx] |= kStateSignalled;
			}
		}
	}
}

void __fastcall InputPollFakeHandleNoMouse(OSInputGlobals* input) { _MESSAGE("Mache è sto porccoddio"); }

const UInt32 kLoc_403C40 = 0x00403C40;

__declspec(naked) void PollInputHook() {
	__asm {
		jle short loc_cont
		pop edi
		pop esi
		pushad
		call InputPollFakeHandle
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
void Hook_Input_Init() {
	WriteRelJump(kPollEndHook, (UInt32) &PollInputHook);
	WriteRelJump(kPollEndNoMouseHook, (UInt32)&PollInputNoMouseHook); //TODO clean leftovers
}