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



UInt8 OSInputGlobalsEx::GetPreSignalStatusKey(UInt16 keycode) {
	if (keycode >= 256) return GetPreSignalStatusMouse(keycode - 256);
	return (KeyMaskState[keycode] & kStatePSignalled) == kStatePSignalled;
}

UInt8 OSInputGlobalsEx::GetPreSignalStatusMouse(UInt8 keycode) {
	if (keycode >= 8) return 0;
	return (MouseMaskState.rgbButtons[keycode] & kStatePSignalled) == kStatePSignalled;
}

void OSInputGlobalsEx::SetTapKey(UInt16 keycode){
	if (keycode >= 256) SetTapMouse(keycode - 256);
	else KeyMaskState[keycode] |= kStateTap;
}

void OSInputGlobalsEx::SetTapMouse(UInt8 keycode){
	if (keycode >= 8) return;
	MouseMaskState.rgbButtons[keycode] |= kStateTap;
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
	MouseMaskState.rgbButtons[keycode] &= ~kStateHammered;
	MouseMaskState.rgbButtons[keycode] &= ~kStateAHammered;

}

bool OSInputGlobalsEx::IsKeyPressedKeyboard(UInt16 keycode) {
	if ((KeyMaskState[keycode] & kStateDisabled) == kStateDisabled) {
		//Don't report tap presses, so check only for signal state
		return (KeyMaskState[keycode] & kStateSignalled) == kStateSignalled;
	}
	else {
		//also here don't report tapped keys
		 return (CurrentKeyState[keycode] != 0)  && ((KeyMaskState[keycode] & kStateTapped ) != kStateTapped);
	}
}

bool OSInputGlobalsEx::IsKeyPressedMouse(UInt8 keycode) {
	if ((MouseMaskState.rgbButtons[keycode] & kStateDisabled) == kStateDisabled) {
		//Don't report tap presses, so check only for signal state
		return (MouseMaskState.rgbButtons[keycode] & kStateSignalled) == kStateSignalled;
	}
	else {
		//TODO also here don't report tapped keys (require moving the clear of tap status in the hooked loop)
		return (CurrentMouseState.rgbButtons[keycode] != 0) && ((MouseMaskState.rgbButtons[keycode] & kStateTapped) != kStateTapped);
	}

}
bool OSInputGlobalsEx::IsKeyPressed(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxMacros) {
		return  IsKeyPressedMouse(keycode - 256);
	}
	return IsKeyPressedKeyboard(keycode);

}

//TODO what happens if key is tapped while is pressed? Should probably be reported
bool OSInputGlobalsEx::WasKeyPressedMouse(UInt8 keycode) {
	return ((MouseMaskState.rgbButtons[keycode] & kStatePTapped) != kStatePTapped) && (PreviousMouseState.rgbButtons[keycode] != 0);
}
bool OSInputGlobalsEx::WasKeyPressedKeyboard(UInt16 keycode) {
	return ((KeyMaskState[keycode] & kStatePTapped) != kStatePTapped)  && (PreviousKeyState[keycode] != 0);
}

bool OSInputGlobalsEx::WasKeyPressed(UInt16 keycode) {
	if (keycode >= 256) return WasKeyPressedMouse(keycode - 256);
	return WasKeyPressedKeyboard(keycode);
}
//TODO Speed(Ma davvero vogliamo sto' bordello?)
//TODO wheel to button translation
//TODO add a way to separate proper presses to simulated presses
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
	MouseMaskState.lX = 0;
	MouseMaskState.lY = 0;

	for (UInt16 idx = 0; idx <= 255; idx++) {
		if (idx < 8) {
			if ((MouseMaskState.rgbButtons[idx] & kStateSignalled) == kStateSignalled) {
				MouseMaskState.rgbButtons[idx] &= ~kStateSignalled;
				MouseMaskState.rgbButtons[idx] |= kStatePSignalled;
//				_MESSAGE("Mouse key %0X  was pressed, set previous frame state", idx);
			}
			else { MouseMaskState.rgbButtons[idx] &= ~kStatePSignalled; }
			if ((MouseMaskState.rgbButtons[idx] & kStateTapped) == kStateTapped) {
				MouseMaskState.rgbButtons[idx] &= ~kStateTapped;
				MouseMaskState.rgbButtons[idx] |= kStatePTapped;
//				_MESSAGE("Mouse key %0X  was tapped, set previous frame state", idx);

			}
			else { MouseMaskState.rgbButtons[idx] &= ~kStatePTapped; }
			//	_MESSAGE("%u mouse key mask  %0X, status %0X", idx,  MouseMaskState.rgbButtons[idx]  , CurrentMouseState.rgbButtons[idx]);
			if (FrameIndex == 0 && (MouseMaskState.rgbButtons[idx] & kStateHammered) == kStateHammered) {
				CurrentMouseState.rgbButtons[idx] = 0x80; 
	//			_MESSAGE("Mouse Key %0X was setted as hammered, simulate press", idx);
			}
			if (FrameIndex == 1 && (MouseMaskState.rgbButtons[idx] & kStateAHammered) == kStateAHammered) {
				CurrentMouseState.rgbButtons[idx] = 0x80; 
	//			_MESSAGE("Mouse Key %0X was setted as Ahammered, simulate press", idx);
			}
			if ((MouseMaskState.rgbButtons[idx] & kStateHolded) == kStateHolded) {
	//			_MESSAGE("Mouse Key %0X was setted as holded, simulate press", idx);
				CurrentMouseState.rgbButtons[idx] = 0x80;
			}
			if ((MouseMaskState.rgbButtons[idx] & kStateDisabled) == kStateDisabled) {
	//			_MESSAGE("Mouse key %0X disabled", idx);
				if (CurrentMouseState.rgbButtons[idx] != 0) {  //Presses are 0x80 
	//				_MESSAGE("Mouse key %0X was pressed, signal ", idx);
					CurrentMouseState.rgbButtons[idx] = 0;
					MouseMaskState.rgbButtons[idx] |= kStateSignalled;
				}
			}
			if ((MouseMaskState.rgbButtons[idx] & kStateTap) == kStateTap   && CurrentMouseState.rgbButtons[idx] == 0) {
				CurrentMouseState.rgbButtons[idx] = 0x80; 
	//			_MESSAGE("Mouse Key %0X tapped, simulate press", idx);
				CurrentMouseState.rgbButtons[idx] |= kStateTapped;
			}
			CurrentMouseState.rgbButtons[idx] &= ~kStateTap;
		}
		if ((KeyMaskState[idx] & kStateSignalled) == kStateSignalled) {
			KeyMaskState[idx] &= ~kStateSignalled;
			KeyMaskState[idx] |= kStatePSignalled;
	//		_MESSAGE("key %0X  was pressed, set previous frame state", idx);
		}
		else { KeyMaskState[idx] &= ~kStatePSignalled; }
		if ((KeyMaskState[idx] & kStateTapped) == kStateTapped) {
			KeyMaskState[idx] &= ~kStateTapped;
			KeyMaskState[idx] |= kStatePTapped;
	//		_MESSAGE("key %0X  was tapped, set previous frame state", idx);
		}
		else { KeyMaskState[idx] &= ~kStatePTapped; }
		if (FrameIndex == 0 && (KeyMaskState[idx] & kStateHammered) == kStateHammered) {
			CurrentKeyState[idx] = 0x80;
	//		_MESSAGE("Key %0X was setted as hammered, simulate press", idx);
		}
		if (FrameIndex == 1 && (KeyMaskState[idx] & kStateAHammered) == kStateAHammered) {
			CurrentKeyState[idx] = 0x80;
	//		_MESSAGE("Key %0X was setted as Ahammered, simulate press", idx);
		}
		if ((KeyMaskState[idx] & kStateHolded) == kStateHolded) {
			CurrentKeyState[idx] = 0x80;
	//		_MESSAGE("Key %0X was setted as holded, simulate press", idx);
		}
		if((KeyMaskState[idx] & kStateDisabled) == kStateDisabled){
	//		_MESSAGE("key %0X disabled", idx);
			if (CurrentKeyState[idx] != 0) {
	//			_MESSAGE("key %0X was pressed, signal ", idx);
				CurrentKeyState[idx] = 0;
				KeyMaskState[idx] |= kStateSignalled;
			}
		}
		if ((KeyMaskState[idx] & kStateTap) == kStateTap  && CurrentKeyState[idx] == 0) {
			CurrentKeyState[idx] = 0x80;
	//		_MESSAGE("Key %0X tapped, simulate press", idx);
			KeyMaskState[idx] |= kStateTapped;
		}
		KeyMaskState[idx] &= ~kStateTap;
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
	ZeroMemory(this->KeyMaskState, 256 * sizeof(KeyControlState));
	ZeroMemory(this->MouseMaskState.rgbButtons, 8 * sizeof(KeyControlState));
	this->MouseMaskState.lX = 0;
	this->MouseMaskState.lY = 0;
	this->MouseMaskState.lZ = 0;
	this->MouseDisabled = 0;
	this->FrameIndex = 0;
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


