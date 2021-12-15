#include "Hooks_Input.h" 
#include "obse_common/SafeWrite.h"
#include <obse/EventManager.h>

OSInputGlobalsEx* g_inputGlobal = nullptr;

void OSInputGlobalsEx::SetMask(UInt16 keycode) {
	//TODO handle 265 and 266
	if (keycode >= 256 && keycode < kMaxButtons) MouseMaskState.rgbButtons[keycode - 256] |= kStateDisabled;
	else if(keycode < 256) KeyMaskState[keycode] |= kStateDisabled;
}

void OSInputGlobalsEx::SetUnMask(UInt16 keycode) {
	//TODO handle 265 and 266
	if (keycode >= 256 && keycode < kMaxButtons) MouseMaskState.rgbButtons[keycode - 256] &= ~kStateDisabled;
	else if (keycode < 256) KeyMaskState[keycode] &= ~kStateDisabled;
}

UInt8 OSInputGlobalsEx::GetMaskStatus(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) return MouseMaskState.rgbButtons[keycode - 256];
	if (keycode < 256) return KeyMaskState[keycode];
	return 0;
}

void OSInputGlobalsEx::SetTap(UInt16 keycode){
	if (keycode >= 256 && keycode < kMaxButtons) MouseMaskState.rgbButtons[keycode - 256] |= kStateTap;
	else if (keycode < 256) KeyMaskState[keycode] |= kStateTap;
}

void OSInputGlobalsEx::SetHold(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) MouseMaskState.rgbButtons[keycode - 256] |= kStateHolded;
	else if (keycode < 256) KeyMaskState[keycode] |= kStateHolded;
}

void OSInputGlobalsEx::SetUnHold(UInt16 keycode){
	if (keycode >= 256 && keycode < kMaxButtons) MouseMaskState.rgbButtons[keycode - 256] &= ~kStateHolded;
	else if (keycode < 256) KeyMaskState[keycode] &= ~kStateHolded;
}

void OSInputGlobalsEx::SetHammer(UInt16 keycode, bool AHammer){
	if (keycode >= 256 && keycode < kMaxButtons ) MouseMaskState.rgbButtons[keycode - 256] |= (AHammer ? kStateAHammered : kStateHammered);
	else if (keycode < 256) KeyMaskState[keycode] |= (AHammer ?  kStateAHammered : kStateHammered);
}

void OSInputGlobalsEx::SetUnHammer(UInt16 keycode){
	if (keycode >= 256 && keycode < kMaxButtons) {
		MouseMaskState.rgbButtons[keycode - 256] &= ~kStateHammered;
		MouseMaskState.rgbButtons[keycode - 256] &= ~kStateAHammered;
	}
	else if (keycode < 256) {
		KeyMaskState[keycode] &= ~kStateHammered;
		KeyMaskState[keycode] &= ~kStateAHammered;
	}
}

bool OSInputGlobalsEx::IsKeyPressedSimulated(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) return  CurrentMouseState.rgbButtons[keycode - 256] == 0x80;
	else if (keycode == 264) return CurrentMouseState.lZ > 0;
	else if (keycode == 265) return CurrentMouseState.lZ < 0;
	else if(keycode < 256) return CurrentKeyState[keycode] == 0x80;
	return false;
}

bool OSInputGlobalsEx::WasKeyPressedSimulated(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) return  PreviousMouseState.rgbButtons[keycode - 256] == 0x80;
	else if (keycode == 264) return PreviousMouseState.lZ > 0;
	else if (keycode == 265) return PreviousMouseState.lZ < 0;
	else if (keycode < 256) return PreviousKeyState[keycode] == 0x80;
	return false;
}

bool OSInputGlobalsEx::IsKeyPressedReal(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) return (MouseMaskState.rgbButtons[keycode - 256] & kStateSignalled) == kStateSignalled;
	else if (keycode == 264) return CurrentMouseState.lZ > 0;
	else if (keycode == 265) return CurrentMouseState.lZ < 0;
	else if (keycode < 256) return (KeyMaskState[keycode] & kStateSignalled) == kStateSignalled;
	return false;
}

bool OSInputGlobalsEx::WasKeyPressedReal(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) return (MouseMaskState.rgbButtons[keycode - 256] & kStatePSignalled) == kStatePSignalled;
	else if (keycode == 264) return PreviousMouseState.lZ > 0;
	else if (keycode == 265) return PreviousMouseState.lZ < 0;
	else if (keycode < 256) return (KeyMaskState[keycode] & kStatePSignalled) == kStatePSignalled;
	return false;
}

inline void OSInputGlobalsEx::SendKeyEvent(UInt16 idx) {
	if ((KeyMaskState[idx] & kStateSignalled) == kStateSignalled && (KeyMaskState[idx] & kStatePSignalled) != kStatePSignalled)
		EventManager::HandleEvent(EventCode[0], (void*)idx, (void*)KeyEvent_Down);
	else if ((KeyMaskState[idx] & kStateSignalled) != kStateSignalled && (KeyMaskState[idx] & kStatePSignalled) == kStatePSignalled) {
		EventManager::HandleEvent(EventCode[0], (void*)idx, (void*)KeyEvent_Up);
	}
	else if ((KeyMaskState[idx] & kStateSignalled) == kStateSignalled && (KeyMaskState[idx] & kStatePSignalled) == kStatePSignalled) {
		EventManager::HandleEvent(EventCode[0], (void*)idx, (void*)KeyEvent_Hold);
	}
}

inline void OSInputGlobalsEx::SendMouseEvent(UInt16 idx) {
	void* nomralizedKey = (void*)(idx + 256);
	if ((MouseMaskState.rgbButtons[idx] & kStateSignalled) == kStateSignalled && (MouseMaskState.rgbButtons[idx] & kStatePSignalled) != kStatePSignalled)
		EventManager::HandleEvent(EventCode[0], nomralizedKey, (void*)KeyEvent_Down);
	else if ((MouseMaskState.rgbButtons[idx] & kStateSignalled) != kStateSignalled && (MouseMaskState.rgbButtons[idx] & kStatePSignalled) == kStatePSignalled) {
		EventManager::HandleEvent(EventCode[0], nomralizedKey, (void*)KeyEvent_Up);
	}
	else if ((MouseMaskState.rgbButtons[idx] & kStateSignalled) == kStateSignalled && (MouseMaskState.rgbButtons[idx] & kStatePSignalled) == kStatePSignalled) {
		EventManager::HandleEvent(EventCode[0], nomralizedKey, (void*)KeyEvent_Hold);
	}
}

inline void OSInputGlobalsEx::SendControlEvents() {
	for (UInt8 idx = 0; idx < 29; idx++) {
		UInt8 key = KeyboardInputControls[idx];
		if (key != 0xFF) {
			if ((KeyMaskState[key] & kStateSignalled) == kStateSignalled && (KeyMaskState[key] & kStatePSignalled) != kStatePSignalled)
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Down);
			else if ((KeyMaskState[key] & kStateSignalled) != kStateSignalled && (KeyMaskState[key] & kStatePSignalled) == kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Up);
			}
			else if ((KeyMaskState[key] & kStateSignalled) == kStateSignalled && (KeyMaskState[key] & kStatePSignalled) == kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Hold);
			}
			continue;
		}
		key = MouseInputControls[idx];
		if (key != 0xFF) {
			if ((MouseMaskState.rgbButtons[key] & kStateSignalled) == kStateSignalled && (MouseMaskState.rgbButtons[key] & kStatePSignalled) != kStatePSignalled)
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Down);
			else if ((MouseMaskState.rgbButtons[idx] & kStateSignalled) != kStateSignalled && (MouseMaskState.rgbButtons[key] & kStatePSignalled) == kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Up);
			}
			else if ((MouseMaskState.rgbButtons[key] & kStateSignalled) == kStateSignalled && (MouseMaskState.rgbButtons[key] & kStatePSignalled) == kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Hold);
			}
		}
	}
}

//TODO Speed(Ma davvero vogliamo sto' bordello?)
//TODO wheel to button translation
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
	DWORD time = GetTickCount();
	lastFrameLength = (float)(time - lastFrameTime) / 1000.0f;
	lastFrameTime = time;

	CurrentMouseState.lX += MouseMaskState.lX;
	CurrentMouseState.lY += MouseMaskState.lY;
	MouseMaskState.lX = 0;
	MouseMaskState.lY = 0;

	if (MouseAxisMovementPerSecond[0]) {  //Move the mouse in X
		float move = MouseAxisMovementPerSecond[0] * lastFrameLength;
		CurrentMouseState.lX += move;
		float rem = fmodf(move, 1.0f);
		MouseAxisAccumulator[0] += rem;
		if (rem > 0) {
			if (MouseAxisAccumulator[0] > 1) {
				CurrentMouseState.lX += 1;
				MouseAxisAccumulator[0] -= 1;
			}
		}
		else if(rem < 0) {
			if (MouseAxisAccumulator[0] < -1) {
				CurrentMouseState.lX -= 1;
				MouseAxisAccumulator[0] += 1;
			}
		}
	}
	if (MouseAxisMovementPerSecond[1]) {  //Move the mouse in y
		float move = MouseAxisMovementPerSecond[1] * lastFrameLength;
		CurrentMouseState.lY += move;
		float rem = fmodf(move, 1.0f);
		MouseAxisAccumulator[1] += rem;
		if (rem > 0) {
			if (MouseAxisAccumulator[1] > 1) {
				CurrentMouseState.lY += 1;
				MouseAxisAccumulator[1] -= 1;
			}
		}
		else if (rem < 0) {
			if (MouseAxisAccumulator[1] < -1) {
				CurrentMouseState.lY -= 1;
				MouseAxisAccumulator[1] += 1;
			}
		}
	}

	for (UInt16 idx = 0; idx <= 255; idx++) {
		if (idx < 8) {
			if ((MouseMaskState.rgbButtons[idx] & kStateSignalled) == kStateSignalled) MouseMaskState.rgbButtons[idx] |= kStatePSignalled;
			else MouseMaskState.rgbButtons[idx] &= ~kStatePSignalled;

			if (CurrentMouseState.rgbButtons[idx] == 0x80) MouseMaskState.rgbButtons[idx] |= kStateSignalled;
			else MouseMaskState.rgbButtons[idx] &= ~kStateSignalled;

			if (FrameIndex == 0 && (MouseMaskState.rgbButtons[idx] & kStateHammered) == kStateHammered) CurrentMouseState.rgbButtons[idx] = 0x80;
			
			if (FrameIndex == 1 && (MouseMaskState.rgbButtons[idx] & kStateAHammered) == kStateAHammered) CurrentMouseState.rgbButtons[idx] = 0x80;
			
			if ((MouseMaskState.rgbButtons[idx] & kStateHolded) == kStateHolded) CurrentMouseState.rgbButtons[idx] = 0x80;
			
			if ((MouseMaskState.rgbButtons[idx] & kStateDisabled) == kStateDisabled) CurrentMouseState.rgbButtons[idx] = 0;
		
			if ((MouseMaskState.rgbButtons[idx] & kStateTap) == kStateTap) CurrentMouseState.rgbButtons[idx] = 0x80;
			MouseMaskState.rgbButtons[idx] &= ~kStateTap;

			SendMouseEvent(idx);
		}
		if ((KeyMaskState[idx] & kStateSignalled) == kStateSignalled) KeyMaskState[idx] |= kStatePSignalled;
		else KeyMaskState[idx] &= ~kStatePSignalled;

		if (CurrentKeyState[idx] == 0x80) KeyMaskState[idx] |= kStateSignalled;
		else KeyMaskState[idx] &= ~kStateSignalled;

		if (FrameIndex == 0 && (KeyMaskState[idx] & kStateHammered) == kStateHammered) CurrentKeyState[idx] = 0x80;

		if (FrameIndex == 1 && (KeyMaskState[idx] & kStateAHammered) == kStateAHammered) CurrentKeyState[idx] = 0x80;

		if ((KeyMaskState[idx] & kStateHolded) == kStateHolded) CurrentKeyState[idx] = 0x80;

		if ((KeyMaskState[idx] & kStateDisabled) == kStateDisabled) CurrentKeyState[idx] = 0;

		if ((KeyMaskState[idx] & kStateTap) == kStateTap ) CurrentKeyState[idx] = 0x80;
		KeyMaskState[idx] &= ~kStateTap;
		SendKeyEvent(idx);

	}
	SendControlEvents();
	FrameIndex = (FrameIndex + 1) % 2;

}

void __fastcall InputPollFakeHandleNoMouse(OSInputGlobals* input) { _MESSAGE("Can't still use the new input system without a mouse "); }


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
	EventCode[0] = EventManager::EventIDForString("OnKeyEvent");
	EventCode[1] = EventManager::EventIDForString("OnControlEvent");
	this->MouseAxisMovementPerSecond[0] = 0;
	this->MouseAxisMovementPerSecond[1] = 0;
	this->lastFrameTime = GetTickCount();  //TODO QueryPerformanceCounter? Internal GetTickCount 
	this->MouseAxisAccumulator[0] = 0;
	this->MouseAxisAccumulator[1] = 0;
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



namespace PluginAPI {
	void DisableKey(UInt16 dxCode) { g_inputGlobal->SetMask(dxCode); }
	void EnableKey(UInt16 dxCode) { g_inputGlobal->SetUnMask(dxCode); }
	void DisableControl(UInt16 controlCode) {
		UInt8 keyCode = g_inputGlobal->KeyboardInputControls[controlCode];
		if (keyCode != 0xFF) g_inputGlobal->SetMask(keyCode);
		keyCode = g_inputGlobal->MouseInputControls[controlCode];
		if (keyCode != 0xFF) g_inputGlobal->SetMask(keyCode + 256);
	}
	void EnableControl(UInt16 controlCode) {
		UInt8 keyCode = g_inputGlobal->KeyboardInputControls[controlCode];
		if (keyCode != 0xFF) g_inputGlobal->SetUnMask(keyCode);
		keyCode = g_inputGlobal->MouseInputControls[controlCode];
		if (keyCode != 0xFF) g_inputGlobal->SetUnMask(keyCode + 256);
	}
	bool IsKeyPressedReal(UInt16 dxCode) { return g_inputGlobal->IsKeyPressedReal(dxCode); }
	bool IsKeyPressedSimulated(UInt16 dxCode) { return g_inputGlobal->IsKeyPressedSimulated(dxCode); }
	bool IsControlPressedReal(UInt16 controlCode) {
		UInt8 keyCode = g_inputGlobal->KeyboardInputControls[controlCode];
		bool result = false;
		if (keyCode != 0xFF) result = g_inputGlobal->IsKeyPressedReal(keyCode);
		keyCode = g_inputGlobal->MouseInputControls[controlCode];
		if (keyCode != 0xFF && !result) result = g_inputGlobal->IsKeyPressedReal(keyCode + 256);
		return result;
	}
	bool IsControlPressedSimulated(UInt16 controlCode) {
		UInt8 keyCode = g_inputGlobal->KeyboardInputControls[controlCode];
		bool result = false;
		if (keyCode != 0xFF) result = g_inputGlobal->IsKeyPressedSimulated(keyCode);
		keyCode = g_inputGlobal->MouseInputControls[controlCode];
		if (keyCode != 0xFF && !result) result = g_inputGlobal->IsKeyPressedSimulated(keyCode + 256);
		return result;
	}
}