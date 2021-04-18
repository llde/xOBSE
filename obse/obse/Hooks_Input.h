#pragma once
#include "GameOSDepend.h"
static UInt32 kPollEndHook = 0x00403C81; 
static UInt32 kPollEndRetn = 0x00403C85;
static UInt32 kPollEndNoMouseHook = 0x0040483A;
static UInt32 kPollEndNoMouseRetn = 0x0040483C;


static UInt8 ControlsState[kControlsMapped] = {0};
static UInt8 KeyState[256] = { 0 };


enum  KeyControlState : UInt8 {
	kStateDisabled =  1 << 0,
	kStateSignalled = 1 << 1
};

void Hook_Input_Init();


void SetMaskKey(UInt16 keycode);
void SetUnmaskKey(UInt16 keycode);
static void SetMaskMouse(UInt8 keycode) {}
static void SetUnmaskMouse(UInt8 keycode) {}
UInt8 GetMaskStatusKey(UInt8 keycode);
static UInt8 GetMaskStatusMouse(UInt8 keycode) { return 0; }

UInt8 GetSignalStatusKey(UInt16 keycode);
UInt8 GetSignalStatusMouse(UInt8 keycode);
UInt8 GetSignalStatus(UInt16 keycode);

