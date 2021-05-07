#pragma once

// keeping this in a separate file so we don't need to include dinput/dsound everywhere

#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0800
#include <dinput.h>
#include <dsound.h>

#include "obse/GameTypes.h"
#include "obse/Utilities.h"

class TESGameSound;
class NiAVObject;

static const UInt32 kOSSoundGlobals_PlaySoundAddr = 0x006AE0A0;


// 1BD8
class OSInputGlobals
{
public:
	//Last 10 reserved for the mouse (wheel up, wheel down and up to 8 buttons)
	#define kMaxMacros  266
	#define kControlsMapped 29

	enum
	{
		kFlag_HasJoysticks =	1 << 0,
		kFlag_HasMouse =		1 << 1,
		kFlag_HasKeyboard =		1 << 2,
		kFlag_BackgroundMouse =	1 << 3,
	};

	enum
	{
		kMaxDevices = 8,
	};

	enum
	{
		kDeviceType_KEYBOARD = 1,
		kDeviceType_MOUSE
	};

    //No True Hold key
   enum KeyQuery : UInt8 {
      kKeyQuery_Tap = 0, // key is down
      kKeyQuery_Down = 1, // key just went down
      kKeyQuery_Up = 2, // key just went up
      kKeyQuery_Change = 3, // key just went up or down
      kKeyQuery_Double = 4, // key was double-clicked (for mouse buttons 0 - 7 only)
   };
   enum JoystickControl : UInt8 {
      kJoystickControl_UnkButton00 = 0x00,
      //
      // ... buttons range up to, and do not include, 0x20
      //
      kJoystickControl_UnkControl20 = 0x20,
   };
   enum MouseAxis : UInt8 {
      kMouseAxis_X = 1,
      kMouseAxis_Y = 2,
      kMouseAxis_Scroll = 3,
   };
   enum MouseControl : UInt8 {
      kMouseControl_Left = 0,
      kMouseControl_Right = 1,
      kMouseControl_UnkButton2 = 2,
      kMouseControl_UnkButton3 = 3,
      kMouseControl_UnkButton4 = 4,
      kMouseControl_UnkButton5 = 5,
      kMouseControl_UnkButton6 = 6,
      kMouseControl_UnkButton7 = 7,
      kMouseControl_WheelUp = 8, // could have these two backwards
      kMouseControl_WheelDown = 9, //
   };
   enum MappableControl : UInt8 {
      kControl_Forward = 0x00,
      kControl_Back = 0x01,
      kControl_StrafeLeft = 0x02,
      kControl_StrafeRight = 0x03,
      kControl_UseEquipped = 0x04,
      kControl_Activate = 0x05,
      kControl_Block,
      kControl_Cast,
      kControl_DrawSheatheWeapon,
      kControl_Sneak,
      kControl_Run = 0x0A,
      kControl_AutoRun,
      kControl_AutoMove,
      kControl_Jump,
      kControl_TogglePOV,
      kControl_MenuMode = 0x0F,
      kControl_Rest = 0x10,
      kControl_QuickMenu = 0x11,
      kControl_Quick1 = 0x12,
      kControl_Quick2,
      kControl_Quick3,
      kControl_Quick4,
      kControl_Quick5,
      kControl_Quick6,
      kControl_Quick7,
      kControl_Quick8 = 0x19,
      kControl_QuickSave = 0x1A,
      kControl_QuickLoad = 0x1B,
      kControl_Grab = 0x1C,
      kControl_MAXSTANDARD = kControl_Grab,
      kControl_COUNT,
      //
      // Special indices follow. These are not represented in the Scheme data.
      //
      kControl_SpecialPause = 0x1D, // QueryControlState checks keyboard state directly for DIK_ESCAPE
      kControl_SpecialConsole = 0x1E, // QueryControlState checks keyboard state directly for DIK_GRAVE
      kControl_SpecialPrintScreen = 0x1F  // QueryControlState checks keyboard state directly for DIK_SYSRQ
   };



	OSInputGlobals();
	~OSInputGlobals();

	// 244
	class Joystick
	{
	public:
		Joystick();
		~Joystick();

		UInt32	unk000[0x244 >> 2];
	};

	struct JoystickObjectsInfo
	{
		enum
		{
			kHasXAxis =		1 << 0,
			kHasYAxis =		1 << 1,
			kHasZAxis =		1 << 2,
			kHasXRotAxis =	1 << 3,
			kHasYRotAxis =	1 << 4,
			kHasZRotAxis =	1 << 5
		};

		UInt32	axis;
		UInt32	buttons;
	};

	// 2C
	struct Unk1AF4
	{
		UInt32	bufLen;
		UInt8	unk04[0x2C - 4];
	};

	struct JoystickDeviceState {
		DIJOYSTATE allStateThisFrame;  //00
		UInt8	   buttonsLastFrame[32];   //50
	};

	UInt32		flags;										// 0000
	IDirectInput8		* dinputInterface;					// 0004
	IDirectInputDevice8	* joystickInterfaces[kMaxDevices];	// 0008 - format is c_dfDIJoystick
	IDirectInputDevice8	* keyboardInterface;				// 0028 - format is c_dfDIKeyboard
	IDirectInputDevice8	* mouseInterface;					// 002C - format is c_dfDIMouse2

	JoystickDeviceState	joystickDeviceState[kMaxDevices];	// 0030
	UInt8       unk03B0[0x530 - 0x3B0];						// 03B0
	DIDEVICEINSTANCE	joystickDevices[kMaxDevices];				// 0530
	DIDEVCAPS	joysticDevCaps[kMaxDevices];				// 1750
	JoystickObjectsInfo	devInfo[kMaxDevices];				// 18B0
	UInt32		numJoysticks;								// 18F0
	UInt8		CurrentKeyState[0x100];						// 18F4
	UInt8       PreviousKeyState[0x100];					// 19F4
	Unk1AF4		unk1AF4;									// 1AF4
	DIMOUSESTATE2 CurrentMouseState;						// 1B20
	DIMOUSESTATE2 PreviousMouseState;						// 1B34
	UInt32		oldMouseButtonSwap;							// 1B48 - state of SwapMouseButton when program launched
	UInt32		doubleClickTime;							// 1B4C - GetDoubleClickTime
	UInt8		unk1B50[8];									// 1B50
	UInt32      unk1B58[8];									// 1B58  see 0x00403C30
	UInt32      unk1B78;									// 1B78
	UInt16		unk1B7C;									// 1B7C

	// control bindings
	UInt8		KeyboardInputControls[29];				    // 1B7E - ini control >> 16
	UInt8		MouseInputControls[29];						// 1B9B - ini control >> 8
	UInt8		JoystickInputControls[29];					// 1BB8 - ini control >> 0 
	UInt8		alwaysRunControlMaybe;						// 1BD5  (relevant to as AlwaysRun, but it's assigned as a button in ResetBinding function as JoystickInputControls[29] = 0xB)
	UInt8		FrameIndex;									// 1BD6  (Repurposed as FrameIndex, originally Unused)
	UInt8		MouseDisabled;								// 1BD7  (Repurposed as MouseDisabled,  originally Unused)
	//Methods
	MEMBER_FN_PREFIX(OSInputGlobals);
    DEFINE_MEMBER_FN(FlushKeyboardBuffer,        void,   0x00403160);
    DEFINE_MEMBER_FN(GetJoystickAxisMovement,    SInt32, 0x00402F50, UInt8 whichJoystick, UInt8 whichAxis); // axis indices: 1, 2, 3, 4, 5, 6: x, y, z, rX, rY, rZ // axes are normalized to the range of [-100, 100]
    DEFINE_MEMBER_FN(GetJoystickButtonCount,     UInt32, 0x004030B0, UInt8 whichJoystick);
    DEFINE_MEMBER_FN(GetJoystickPOVControlCount, UInt32, 0x004030D0, UInt8 whichJoystick);
    DEFINE_MEMBER_FN(GetMouseAxisMovement,       SInt32, 0x00403190, UInt8 whichAxis); // 1, 2, 3: x, y, z; return values are pixels moved since last tick
    DEFINE_MEMBER_FN(LoadControlSettingsFromINI, UInt32, 0x00404540);
    DEFINE_MEMBER_FN(QueryInputState,            UInt32, 0x00403490, UInt8 schemeIndex, UInt8 keycode, KeyQuery query);
    //  - if schemeIndex > 1, then (schemeIndex - 2) is the joystick index to use
    //  - for keyboard and mouse, just calls QueryKeyboardKeyState and QueryMouseKeyState
    //  - if we wanna add gamepad support, we'd need to patch this to query XInput for state
    DEFINE_MEMBER_FN(QueryJoystickButtonState,   bool,   0x00402FC0, UInt8 whichJoystick, UInt8 buttonIndex, KeyQuery query);
    DEFINE_MEMBER_FN(RebindControl,              bool,   0x00403F50, UInt8 whichCtrl, UInt8 whichScheme, UInt8 newButton); // forbids use of Esc, Tilde, Print Screen, F1 - F4, and 1 - 8
    DEFINE_MEMBER_FN(RebindControlMinimalChecks, bool,   0x00403B80, UInt8 whichCtrl, UInt8 whichScheme, UInt8 newButton); // forbids use of Esc, Tilde, and Print Screen
    DEFINE_MEMBER_FN(ResetControlMap,            void,   0x00403960, UInt8 whichScheme); // 0, 1, 2, 3 == keyboard, mouse, unk1BB8, all
    DEFINE_MEMBER_FN(SaveControlSettingsToINI,   void,   0x00404400);
    DEFINE_MEMBER_FN(SendControlPress,           UInt8*, 0x00403380, MappableControl whichControl); // forces the control to be "pressed" for the first scheme in which it is defined: keyboard, mouse, joystick // only works for controls < 0xE, i.e. controls related to movement and gameplay
    DEFINE_MEMBER_FN(SetJoystickDeadzone,        void,   0x004030F0, UInt8 whichJoystick, float deadzonePercent);
    DEFINE_MEMBER_FN(UnbindAllMappedTo,          void,   0x00403B50, UInt8 whichScheme, UInt8 whichKey);
    //
    DEFINE_MEMBER_FN(PollAndUpdateInputState,    void,   0x004046A0);
    //  - called (indirectly) by the game's main loop
    //  - updates the "state this frame" and "state last frame" vars
    //  - also calls a function to update double-click state
    //  - if we wanna add gamepad support, we'd need to patch this to poll XInput


	bool QueryMouseKeyState(UInt8 keycode, KeyQuery query) { return ThisStdCall<UInt32, KeyQuery>(0x004031E0, this, keycode, query); }
	bool QueryKeyboardKeyState(UInt16 keycode, KeyQuery query) { return ThisStdCall<UInt32, KeyQuery>(0x004032D0, this, keycode, query); }

/**  - loops over each scheme and calls QueryInputState for the key the control is mapped to
*    - for special keycodes (1D, 1E, 1F), skips that and uses special fields 
*/
	UInt32 QueryControlState(MappableControl control, KeyQuery query) {return ThisStdCall<UInt32, KeyQuery>(0x00403520, this, control, query);}

	static bool IsKeycodeValid(UInt32 id) { return id < kMaxMacros; } //I don't know why there was a -2 causing the wheels motion to not be picked up.

	//TODO allow passign type of binding
	UInt16 GetControlFromKeycode(UInt16 keycode) {
		UInt8* controlArray = KeyboardInputControls; 
		if (keycode >= 256) {
			controlArray = MouseInputControls;
			keycode -= 256;
		}  
		for (UInt8 idx = 0; idx < kControlsMapped; idx++) {
			if (controlArray[idx] == keycode) return idx;
		}
		return 0xFFFF;
	}
	static UInt8 Plugin_IsKeyPressed(UInt16 keycode) { return false; }
};

STATIC_ASSERT(sizeof(OSInputGlobals) == 0x1BD8);

// 58
class TESGameSound
{
public:
	TESGameSound();
	~TESGameSound();

	enum {
		kType_Voice		= 1 << 2,
		kType_Footstep	= 1 << 3,
	};

	UInt8			typeFlags;	// 00	for calculating volume based on OSSoundGlobals::foot/voice/effectVolume
								//      effectVolume used by default
	UInt8			unk01;
	UInt8			unk02[2];
	UInt32			unk04[2];	// 04
	UInt32			hashKey;	// 0C
	UInt32			unk10[4];	// 10
	float			x;			// 20
	float			y;			// 24
	float			z;			// 28
	UInt32			unk2C[4];	// 2C
	float			unk3C;		// 3C
	UInt32			unk40[3];	// 40
	const char *	name;		// 4C
	UInt32			unk50;		// 50
	UInt32			unk54;		// 54
};

// 328
class OSSoundGlobals
{
public:
	OSSoundGlobals();
	~OSSoundGlobals();

	enum
	{
		kFlags_HasDSound =		1 << 0,
		kFlags_HasHardware3D =	1 << 2,
	};
	
	typedef NiTPointerMap <TESGameSound>	TESGameSoundMap;
	typedef NiTPointerMap <NiAVObject>		NiAVObjectMap;

	UInt32					unk000;						// 000
	UInt32					unk004;						// 004
	IDirectSound8			* dsoundInterface;			// 008
	IDirectSoundBuffer8		* primaryBufferInterface;	// 00C
	DSCAPS					soundCaps;					// 010
	UInt32					unk070;						// 070
	UInt32					unk074;						// 074
	IDirectSound3DListener	* listenerInterface;		// 078
	UInt32					unk07C[(0x0A4-0x07C) >> 2];	// 07C
	UInt8					unk0A4;						// 0A4
	UInt8					unk0A5;						// 0A5
	UInt8					unk0A6;						// 0A6
	UInt8					pad0A7;						// 0A7
	UInt32					unk0A8;						// 0A8
	UInt32					flags;						// 0AC - flags?
	UInt32					unk0B0;						// 0B0
	float					unk0B4;						// 0B4
	float					masterVolume;				// 0B8
	float					footVolume;					// 0BC
	float					voiceVolume;				// 0C0
	float					effectsVolume;				// 0C4
	UInt32					unk0C8;						// 0C8 - time
	UInt32					unk0CC;						// 0CC - time
	UInt32					unk0D0;						// 0D0 - time
	UInt32					unk0D4[(0x0DC-0x0D4) >> 2];	// 0D4
	UInt32					unk0DC;						// 0DC
	UInt32					unk0E0[(0x2F0-0x0E0) >> 2];	// 0E0
	float					musicVolume;				// 2F0
	UInt32					unk2F4;						// 2F4
	float					musicVolume2;				// 2F8
	UInt32					unk2FC;						// 2FC
	TESGameSoundMap			* gameSoundMap;				// 300
	NiAVObjectMap			* niObjectMap;				// 304
	NiTPointerList <void>	* soundMessageMap;			// 308 - AudioManager::SoundMessage *
	UInt32					unk30C[(0x320-0x30C) >> 2];	// 30C
	void					* soundMessageList;			// 320
	UInt32					unk324;						// 324

	// flags: seen 0x101 (non-locational), 0x102 (locational)
	DEFINE_MEMBER_FN_LONG(OSSoundGlobals, PlaySound, void, kOSSoundGlobals_PlaySoundAddr, UInt32 soundRefID, UInt32 flags, bool arg2);
};

STATIC_ASSERT(sizeof(OSSoundGlobals) == 0x328);

// 28
class OSGlobals
{
public:
	OSGlobals();
	~OSGlobals();

	UInt8			quitGame;			// 00
	UInt8			exitToMainMenu;		// 01
	UInt8			unk02;				// 02
	UInt8			unk03;				// 03
	UInt8			unk04;				// 04
	UInt8			pad05[3];			// 05
	HWND			window;				// 08
	HINSTANCE		procInstance;		// 0C
	UInt32			mainThreadID;		// 10
	HANDLE			mainThreadHandle;	// 14
	UInt32			unk18;				// 18
	UInt32			unk1C;				// 1C
	OSInputGlobals	* input;			// 20
	OSSoundGlobals	* sound;			// 24
};

extern OSGlobals ** g_osGlobals;
