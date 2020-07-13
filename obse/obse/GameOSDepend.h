#pragma once

// keeping this in a separate file so we don't need to include dinput/dsound everywhere

#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0800
#include <dinput.h>
#include <dsound.h>

#include "obse/GameTypes.h"

class TESGameSound;
class NiAVObject;

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 kOSSoundGlobals_PlaySoundAddr = 0x006AE0A0;
#else
#error unsupported Oblivion version
#endif

// 1BD8
class OSInputGlobals
{
public:
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

	// 28
	struct Unk1B20
	{
		UInt32	unk00;
		UInt32	unk04;
		UInt32	unk08;
		UInt32	unk0C;
		UInt32	unk10;
		UInt32	unk14;
		UInt32	unk18;
		UInt32	unk1C;
		UInt32	unk20;
		UInt32	unk24;
	};

	UInt32		flags;										// 0000
	IDirectInput8		* dinputInterface;					// 0004
	IDirectInputDevice8	* joystickInterfaces[kMaxDevices];	// 0008 - format is c_dfDIJoystick
	IDirectInputDevice8	* keyboardInterface;				// 0028 - format is c_dfDIKeyboard
	IDirectInputDevice8	* mouseInterface;					// 002C - format is c_dfDIMouse2

	UInt32		unk0030[0x140];								// 0030
	Joystick	devices[kMaxDevices];						// 0530
	DIDEVCAPS	devCaps[kMaxDevices];						// 1750
	JoystickObjectsInfo	devInfo[kMaxDevices];				// 18B0
	UInt32		numJoysticks;								// 18F0
	UInt32		unk18F4[0x80];								// 18F4
	Unk1AF4		unk1AF4;									// 1AF4
	Unk1B20		unk1B20;									// 1B20
	UInt32		oldMouseButtonSwap;							// 1B48 - state of SwapMouseButton when program launched
	UInt32		doubleClickTime;							// 1B4C - GetDoubleClickTime
	UInt32		unk1B50[(0x1B7C - 0x1B50) >> 2];			// 1B50
	UInt8		unk1B7C;									// 1B7C
	UInt8		unk1B7D;									// 1B7D

	// control bindings
	UInt8		unk1B7E[0x1D];								// 1B7E - ini control >> 16
	UInt8		unk1B9B[0x1D];								// 1B8B - ini control >> 8
	UInt8		unk1BB8[0x1E];								// 1BB8 - ini control >> 0
	// 1BD6
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
