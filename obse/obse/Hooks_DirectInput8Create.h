#pragma once

typedef HRESULT (_stdcall *DInputProc)(HINSTANCE,DWORD,REFIID,LPVOID,LPUNKNOWN);

void Hook_DirectInput8Create_Init();

void DInput_FakeBufferedKeyTap(UInt32 key);
void DInput_FakeBufferedKeyPress(UInt32 key);
void DInput_FakeBufferedKeyRelease(UInt32 key);

enum
{
	kDeviceType_KEYBOARD = 1,
	kDeviceType_MOUSE
};

//Last 10 reserved for the mouse (wheel up, wheel down and up to 8 buttons)
static const UInt32 kMaxMacros = 266;

struct sDI_data {
	bool GlobalHammer;

	BYTE LastBytes[kMaxMacros];     //Stores which keys were pressed last GetData call
	BYTE FakeStates[kMaxMacros];    //Stores which keys are currently permenently down
	BYTE HammerStates[kMaxMacros];  //Stores which keys are currently being hammered
	BYTE AHammerStates[kMaxMacros]; //Stores the keys that are currently being ahammered
	BYTE TapStates[kMaxMacros];     //Stores the keys that need to be tapped next frame
	BYTE DisallowStates[kMaxMacros];//Stores which keys are disallowed

	BYTE MouseIn[10];      //Used to transfer keypresses to the mouse
	BYTE MouseOut[10];     //Used to transfer keypresses back from the mouse
	int MouseXMov;
	int MouseYMov;
	float MouseXSpeed;
	float MouseYSpeed;
	float MouseXLeft;
	float MouseYLeft;
	bool MouseDisable;
};
extern sDI_data DI_data;

struct sDI_BufferedKeyPress {
	BYTE key;
	bool pressed;

	sDI_BufferedKeyPress(BYTE,bool);
};

float GetAverageFrameTime(void);
UInt32 GetCurrentFrameIndex(void);

// Plugin API
bool Plugin_IsKeyPressed(UInt32 scancode);