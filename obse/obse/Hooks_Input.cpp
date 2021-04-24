#include "Hooks_Input.h" 
#include "obse_common/SafeWrite.h"

OSInputGlobalsEx* g_inputGlobal = nullptr;

void OSInputGlobalsEx::SetMaskKey(UInt16 keycode) {
	if (keycode >= 256) SetMaskMouse(keycode - 256);
	else KeyMaskState[keycode] |= kStateDisabled;
}

void OSInputGlobalsEx::SetMaskMouse(UInt8 keycode) {
	if (keycode >= 8 ) return;
	MouseMaskState.rgbButtons[keycode] |= kStateDisabled;
}

void OSInputGlobalsEx::SetUnmaskKey(UInt16 keycode) {
	if (keycode >= 256) SetUnmaskMouse(keycode - 256);
	else KeyMaskState[keycode] &= ~kStateDisabled;
}

void OSInputGlobalsEx::SetUnmaskMouse(UInt8 keycode) {
	if (keycode >= 8) return;
	MouseMaskState.rgbButtons[keycode] &= ~kStateDisabled;
}

UInt8 OSInputGlobalsEx::GetMaskStatusKey(UInt16 keycode) {
	if (keycode >= 256) return GetMaskStatusMouse(keycode - 256);
	return KeyMaskState[keycode];
}

UInt8 OSInputGlobalsEx::GetMaskStatusMouse(UInt8 keycode) {
	if (keycode >= 8) return 0;
	return MouseMaskState.rgbButtons[keycode];
}


UInt8 OSInputGlobalsEx::GetSignalStatusKey(UInt16 keycode) {
	if (keycode >= 256) return GetSignalStatusMouse(keycode - 256);
	return (KeyMaskState[keycode] & kStateSignalled) == kStateSignalled;
}

UInt8 OSInputGlobalsEx::GetSignalStatusMouse(UInt8 keycode) {
	if (keycode >= 8) return 0;
	return (MouseMaskState.rgbButtons[keycode] & kStateSignalled) == kStateSignalled;
}

void OSInputGlobalsEx::SetTapKey(UInt16 keycode){
	if (keycode >= 256) SetTapMouse(keycode - 256);
	else KeyMaskState[keycode] |= kStateTapped;
}

void OSInputGlobalsEx::SetTapMouse(UInt8 keycode){
	if (keycode >= 8) return;
	MouseMaskState.rgbButtons[keycode] |= kStateTapped;
}

void OSInputGlobalsEx::SetHoldKey(UInt16 keycode) {
	if (keycode >= 256) SetHoldMouse(keycode - 256);
	else KeyMaskState[keycode] |= kStateHolded;
}

void OSInputGlobalsEx::SetHoldMouse(UInt8 keycode){
	if (keycode >= 8) return;
	MouseMaskState.rgbButtons[keycode] |= kStateHolded;
}

void OSInputGlobalsEx::SetUnHoldKey(UInt16 keycode){
	if (keycode >= 256) SetUnHoldMouse(keycode - 256);
	else KeyMaskState[keycode] &= ~kStateHolded;

}

void OSInputGlobalsEx::SetUnHoldMouse(UInt8 keycode){
	if (keycode >= 8) return;
	MouseMaskState.rgbButtons[keycode] &= ~kStateHolded;
}

void OSInputGlobalsEx::SetHammerKey(UInt16 keycode, bool AHammer){
	if (keycode >= 256) SetHammerMouse(keycode - 256, AHammer);
	else KeyMaskState[keycode] |= (AHammer ?  kStateAHammered : kStateHammered);
}

void OSInputGlobalsEx::SetHammerMouse(UInt8 keycode, bool AHammer){
	if (keycode >= 8) return;
	MouseMaskState.rgbButtons[keycode] |= (AHammer ? kStateAHammered : kStateHammered);
}

void OSInputGlobalsEx::SetUnHammerKey(UInt16 keycode){
	if (keycode >= 256) SetUnHammerMouse(keycode - 256);
	else {
		KeyMaskState[keycode] &= ~kStateHammered;
		KeyMaskState[keycode] &= ~kStateAHammered;
	}
}

void OSInputGlobalsEx::SetUnHammerMouse(UInt8 keycode){
	if (keycode >= 8) return;
	KeyMaskState[keycode] &= ~kStateHammered;
	KeyMaskState[keycode] &= ~kStateAHammered;

}


//TODO Speed and wheel to button translation
/*
*  The old DInput hook use this order: Hammer (depending on the frame), Hold (DI_data.FakeState) , Disabled, Tap 
*  So a Disabled key win on hold and hamer but it lose on Tap. So Tap is seen as a proper input
*  Hammer is used for even frames, AHammer for odd ones.
*  THIS IS MADNESS, Even undocumented PORCODDIO.
*/
void OSInputGlobalsEx::InputPollFakeHandle() {
	if (MouseDisabled) {
		CurrentMouseState.lX = 0;
		CurrentMouseState.lY = 0;
	}
	CurrentMouseState.lX += MouseMaskState.lX;
	CurrentMouseState.lY += MouseMaskState.lY;

	for (UInt16 idx = 0; idx <= 255; idx++) {
		if (idx < 8) {
			MouseMaskState.rgbButtons[idx] &= ~kStateSignalled;
			if (FrameIndex == 0 && (MouseMaskState.rgbButtons[idx] & kStateHammered) == kStateHammered) MouseMaskState.rgbButtons[idx] = 0x80;
			if (FrameIndex == 1 && (MouseMaskState.rgbButtons[idx] & kStateAHammered) == kStateAHammered) MouseMaskState.rgbButtons[idx] = 0x80;
			if ((MouseMaskState.rgbButtons[idx] & kStateHolded) == kStateHolded) MouseMaskState.rgbButtons[idx] = 0x80;
			if ((MouseMaskState.rgbButtons[idx] & kStateDisabled) == kStateDisabled) {
				if (CurrentMouseState.rgbButtons[idx] != 0) {  //Presses are 0x80 
					CurrentMouseState.rgbButtons[idx] = 0;
					MouseMaskState.rgbButtons[idx] |= kStateSignalled;
				}
			}
			if ((MouseMaskState.rgbButtons[idx] & kStateTapped) == kStateTapped) {
				CurrentMouseState.rgbButtons[idx] = 0x80;   
				MouseMaskState.rgbButtons[idx] &= ~kStateTapped;
			}
		}
		KeyMaskState[idx] &= ~kStateSignalled;
		if (FrameIndex == 0 && (KeyMaskState[idx] & kStateHammered) == kStateHammered) CurrentKeyState[idx] = 0x80;
		if (FrameIndex == 1 && (KeyMaskState[idx] & kStateAHammered) == kStateAHammered) CurrentKeyState[idx] = 0x80;
		if ((KeyMaskState[idx] & kStateHolded) == kStateHolded) CurrentKeyState[idx] = 0x80;
		if((KeyMaskState[idx] & kStateDisabled) == kStateDisabled){
			if (CurrentKeyState[idx] != 0) {
				CurrentKeyState[idx] = 0;
				KeyMaskState[idx] |= kStateSignalled;
			}
		}
		if ((KeyMaskState[idx] & kStateTapped) == kStateTapped) {
			CurrentKeyState[idx] = 0x80;
			KeyMaskState[idx] &= ~kStateTapped;
		}
	}
	FrameIndex = (FrameIndex + 1) % 2;

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
	ZeroMemory(this->KeyMaskState, 256);
	ZeroMemory(this->MouseMaskState.rgbButtons, 8);
	this->MouseMaskState.lX = 0;
	this->MouseMaskState.lY = 0;
	this->MouseMaskState.lZ = 0;
	g_inputGlobal = this;
	return this;
}

void Hook_Input_Init() {
	SafeWrite32(kInputGlobalAllocSize, sizeof(OSInputGlobalsEx));
	OSInputGlobalsEx* (__thiscall OSInputGlobalsEx::* InitializeExCall)(IDirectInputDevice8*) = &OSInputGlobalsEx::InitializeEx;
	WriteRelCall(kInputInitializeCallAddr, (UInt32) *((void**) &InitializeExCall));
	WriteRelJump(kPollEndHook, (UInt32) &PollInputHook);
	WriteRelJump(kPollEndNoMouseHook, (UInt32)&PollInputNoMouseHook); //TODO clean leftovers
}


