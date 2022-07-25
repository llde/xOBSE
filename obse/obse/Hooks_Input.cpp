
#include "Hooks_Input.h" 
#include "obse_common/SafeWrite.h"
#include <obse/EventManager.h>

OSInputGlobalsEx* g_inputGlobal = nullptr;

void OSInputGlobalsEx::SetMask(UInt16 keycode) {
	//TODO handle 265 and 266
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] |= kStateDisabled;
	}
	else if(keycode < 256) KeyMaskState[keycode] |= kStateDisabled;
}

void OSInputGlobalsEx::SetUnMask(UInt16 keycode) {
	//TODO handle 265 and 266
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] &= ~kStateDisabled;
	}
	else if (keycode < 256) KeyMaskState[keycode] &= ~kStateDisabled;
}

UInt8 OSInputGlobalsEx::GetMaskStatus(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		return MouseMaskState.rgbButtons[keycode - 256];
	}
	if (keycode < 256) return KeyMaskState[keycode];
	return 0;
}

void OSInputGlobalsEx::SetTap(UInt16 keycode){
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] |= kStateTap;
	}
	else if (keycode < 256) KeyMaskState[keycode] |= kStateTap;
}

void OSInputGlobalsEx::SetHold(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] |= kStateHolded;
	}
	else if (keycode < 256) KeyMaskState[keycode] |= kStateHolded;
}

void OSInputGlobalsEx::SetUnHold(UInt16 keycode){
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] &= ~kStateHolded;
	}
	else if (keycode < 256) KeyMaskState[keycode] &= ~kStateHolded;
}

void OSInputGlobalsEx::SetHammer(UInt16 keycode, bool AHammer){
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] |= (AHammer ? kStateAHammered : kStateHammered);
	}
	else if (keycode < 256) KeyMaskState[keycode] |= (AHammer ?  kStateAHammered : kStateHammered);
}

void OSInputGlobalsEx::SetUnHammer(UInt16 keycode){
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		MouseMaskState.rgbButtons[keycode - 256] &= ~kStateHammered;
		MouseMaskState.rgbButtons[keycode - 256] &= ~kStateAHammered;
	}
	else if (keycode < 256) {
		KeyMaskState[keycode] &= ~kStateHammered;
		KeyMaskState[keycode] &= ~kStateAHammered;
	}
}

bool OSInputGlobalsEx::IsKeyPressedSimulated(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		return  CurrentMouseState.rgbButtons[keycode - 256] == 0x80;
	}
	else if (keycode == 264) return CurrentMouseState.lZ > 0;
	else if (keycode == 265) return CurrentMouseState.lZ < 0;
	else if(keycode < 256) return CurrentKeyState[keycode] == 0x80;
	return false;
}

bool OSInputGlobalsEx::WasKeyPressedSimulated(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		return  PreviousMouseState.rgbButtons[keycode - 256] == 0x80;
	}
	else if (keycode == 264) return PreviousMouseState.lZ > 0;
	else if (keycode == 265) return PreviousMouseState.lZ < 0;
	else if (keycode < 256) return PreviousKeyState[keycode] == 0x80;
	return false;
}

bool OSInputGlobalsEx::IsKeyPressedReal(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		return (MouseMaskState.rgbButtons[keycode - 256] & kStateSignalled) == kStateSignalled;
	}
	else if (keycode == 264) return CurrentMouseState.lZ > 0;
	else if (keycode == 265) return CurrentMouseState.lZ < 0;
	else if (keycode < 256) return (KeyMaskState[keycode] & kStateSignalled) == kStateSignalled;
	return false;
}

bool OSInputGlobalsEx::WasKeyPressedReal(UInt16 keycode) {
	if (keycode >= 256 && keycode < kMaxButtons) {
		if (this->oldMouseButtonSwap && keycode == 256) keycode = 257;
		else if (this->oldMouseButtonSwap && keycode == 257) keycode = 256;

		return (MouseMaskState.rgbButtons[keycode - 256] & kStatePSignalled) == kStatePSignalled;
	}
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
	UInt16 idxs = idx;
	if (this->oldMouseButtonSwap && idx == 1) idxs = 0;
	else if (this->oldMouseButtonSwap && idx == 0) idxs = 1;  //Reapply swap to forward the correct event
	void* nomralizedKey = (void*)(idx + 256);
//	_MESSAGE("Key %u", nomralizedKey);

	if ((MouseMaskState.rgbButtons[idxs] & kStateSignalled) == kStateSignalled && (MouseMaskState.rgbButtons[idxs] & kStatePSignalled) != kStatePSignalled)
		EventManager::HandleEvent(EventCode[0], nomralizedKey, (void*)KeyEvent_Down);
	else if ((MouseMaskState.rgbButtons[idxs] & kStateSignalled) != kStateSignalled && (MouseMaskState.rgbButtons[idxs] & kStatePSignalled) == kStatePSignalled) {
		EventManager::HandleEvent(EventCode[0], nomralizedKey, (void*)KeyEvent_Up);
	}
	else if ((MouseMaskState.rgbButtons[idxs] & kStateSignalled) == kStateSignalled && (MouseMaskState.rgbButtons[idxs] & kStatePSignalled) == kStatePSignalled) {
		EventManager::HandleEvent(EventCode[0], nomralizedKey, (void*)KeyEvent_Hold);
	}
}

inline void OSInputGlobalsEx::SendControlEvents() {
	for (UInt8 idx = 0; idx < 29; idx++) {
		UInt8 key = KeyboardInputControls[idx];
		if (key != 0xFF) {
			if ((KeyMaskState[key] & kStateSignalled) == kStateSignalled && (KeyMaskState[key] & kStatePSignalled) != kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Down);
				continue;
			}
			else if ((KeyMaskState[key] & kStateSignalled) != kStateSignalled && (KeyMaskState[key] & kStatePSignalled) == kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Up);
				continue;
			}
			else if ((KeyMaskState[key] & kStateSignalled) == kStateSignalled && (KeyMaskState[key] & kStatePSignalled) == kStatePSignalled) {
				EventManager::HandleEvent(EventCode[1], (void*)idx, (void*)KeyEvent_Hold);
				continue;
			}
		}
		key = MouseInputControls[idx];
		if (key != 0xFF) {
			if (this->oldMouseButtonSwap && key == 0) key = 1;
			else if (this->oldMouseButtonSwap && key == 1) key = 0;
//			_MESSAGE("Control %u, Key %u", idx, key);

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

inline void OSInputGlobalsEx::SendWheelEvents() {
	if (CurrentMouseState.lZ > 0  && PreviousMouseState.lZ > 0) {//264
		EventManager::HandleEvent(EventCode[0], (void*)264, (void*)KeyEvent_Hold);
	}
	else if (CurrentMouseState.lZ > 0 && PreviousMouseState.lZ == 0) {
		EventManager::HandleEvent(EventCode[0], (void*)264, (void*)KeyEvent_Down);
	}
	else if (CurrentMouseState.lZ > 0 && PreviousMouseState.lZ < 0) {
		EventManager::HandleEvent(EventCode[0], (void*)265, (void*)KeyEvent_Up);
		EventManager::HandleEvent(EventCode[0], (void*)264, (void*)KeyEvent_Down);

	}
	else if (CurrentMouseState.lZ < 0 && PreviousMouseState.lZ < 0) {//265
		EventManager::HandleEvent(EventCode[0], (void*)265, (void*)KeyEvent_Hold);
	}
	else if (CurrentMouseState.lZ < 0 && PreviousMouseState.lZ == 0) {
		EventManager::HandleEvent(EventCode[0], (void*)265, (void*)KeyEvent_Down);
	}
	else if (CurrentMouseState.lZ < 0 && PreviousMouseState.lZ > 0) {
		EventManager::HandleEvent(EventCode[0], (void*)264, (void*)KeyEvent_Up);
		EventManager::HandleEvent(EventCode[0], (void*)265, (void*)KeyEvent_Down);
	}
	else if (CurrentMouseState.lZ == 0 && PreviousMouseState.lZ < 0) {
		EventManager::HandleEvent(EventCode[0], (void*)265, (void*)KeyEvent_Up);
	}
	else if (CurrentMouseState.lZ == 0 && PreviousMouseState.lZ > 0) {
		EventManager::HandleEvent(EventCode[0], (void*)264, (void*)KeyEvent_Up);
	}
}

void OSInputGlobalsEx::FakeBufferedKeyTap(UInt32 key){
	DIDEVICEOBJECTDATA data;
	data.uAppData=-1;
	data.dwTimeStamp=GetTickCount();
	data.dwSequence=0;
	data.dwOfs=key;
	data.dwData=0x80;
	fakeBuffered.AddLast(data);
	data.dwData=0x00;
	fakeBuffered.AddLast(data);
}
void OSInputGlobalsEx::FakeBufferedKeyPress(UInt32 key){
	DIDEVICEOBJECTDATA data;
	data.uAppData=-1;
	data.dwTimeStamp=GetTickCount();
	data.dwSequence=0;
	data.dwOfs=key;
	data.dwData=0x80;
	fakeBuffered.AddLast(data);
}
void OSInputGlobalsEx::FakeBufferedKeyRelease(UInt32 key){
	DIDEVICEOBJECTDATA data;
	data.uAppData=-1;
	data.dwTimeStamp=GetTickCount();
	data.dwSequence=0;
	data.dwOfs=key;
	data.dwData=0x00;
	fakeBuffered.AddLast(data);
}

UInt16 OSInputGlobalsEx::GetMouseControlKey(UInt16 ctrl)
{
	UInt16 key = MouseInputControls[ctrl];
	if (this->oldMouseButtonSwap && key == 1) key = 0;
	else if (this->oldMouseButtonSwap && key == 0) key = 1;
	return key;
}

int OSInputGlobalsEx::GetBufferedKeyStateChangeHook(DIDEVICEOBJECTDATA* data) {
	DIDEVICEOBJECTDATA temp = {};
	if(!this->keyboardInterface) return 0;
 	IDirectInputDevice8* keyInterface = this->keyboardInterface;
	UInt32 number = 1;
	if(fakeBuffered.HasElement()) temp = fakeBuffered.RemoveFirst();
	if(keyInterface->GetDeviceData(0x14, &temp, &number, 0) != DI_OK || number == 0) return 0;
	data->dwOfs = temp.dwOfs;
	data->dwData = temp.dwData;
	data->dwTimeStamp = temp.dwTimeStamp;
	return 2 - ((temp.dwData & 0x80) == 0x80);
}
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
			UInt16 idxs = idx;
			if (this->oldMouseButtonSwap && idx == 1) idxs = 0;
			else if (this->oldMouseButtonSwap && idx == 0) idxs = 1;
			
			if ((MouseMaskState.rgbButtons[idxs] & kStateSignalled) == kStateSignalled) MouseMaskState.rgbButtons[idxs] |= kStatePSignalled;
			else MouseMaskState.rgbButtons[idxs] &= ~kStatePSignalled;

			if (CurrentMouseState.rgbButtons[idxs] == 0x80) MouseMaskState.rgbButtons[idxs] |= kStateSignalled;
			else MouseMaskState.rgbButtons[idxs] &= ~kStateSignalled;

			if (FrameIndex == 0 && (MouseMaskState.rgbButtons[idxs] & kStateHammered) == kStateHammered) CurrentMouseState.rgbButtons[idxs] = 0x80;
			
			if (FrameIndex == 1 && (MouseMaskState.rgbButtons[idxs] & kStateAHammered) == kStateAHammered) CurrentMouseState.rgbButtons[idxs] = 0x80;
			
			if ((MouseMaskState.rgbButtons[idxs] & kStateHolded) == kStateHolded) CurrentMouseState.rgbButtons[idxs] = 0x80;
			
			if ((MouseMaskState.rgbButtons[idxs] & kStateDisabled) == kStateDisabled) CurrentMouseState.rgbButtons[idxs] = 0;
		
			if ((MouseMaskState.rgbButtons[idxs] & kStateTap) == kStateTap) CurrentMouseState.rgbButtons[idxs] = 0x80;
			MouseMaskState.rgbButtons[idxs] &= ~kStateTap;

			SendMouseEvent(idxs); //Invert the swap inside 
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
	SendWheelEvents();
	FrameIndex = (FrameIndex + 1) % 2;

}

void __fastcall InputPollFakeHandleNoMouse(OSInputGlobals* input) { _MESSAGE("Can't still use the new input system without a mouse "); }

__declspec(naked) void FakeBufferedKeyHook(){
	__asm{
		 jmp OSInputGlobalsEx::GetBufferedKeyStateChangeHook
	}
}

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
	fakeBuffered = {};
	g_inputGlobal = this;
//	_MESSAGE("Swap mouse state %u", this->oldMouseButtonSwap);
	return this;
}

void Hook_Input_Init() {
	SafeWrite32(kInputGlobalAllocSize, sizeof(OSInputGlobalsEx));
	OSInputGlobalsEx* (__thiscall OSInputGlobalsEx::* InitializeExCall)(IDirectInputDevice8*) = &OSInputGlobalsEx::InitializeEx;
	WriteRelCall(kInputInitializeCallAddr, (UInt32) *((void**) &InitializeExCall));
	WriteRelJump(kPollEndHook, (UInt32) &PollInputHook);
	WriteRelJump(kPollEndNoMouseHook, (UInt32)&PollInputNoMouseHook); //TODO clean leftovers
	WriteRelJump(kBufferedKeyHook, (UInt32)&FakeBufferedKeyHook);

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
