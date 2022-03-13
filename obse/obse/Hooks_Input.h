#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <queue>
#include "GameOSDepend.h"
#include "GameAPI.h"

static const UInt32 kPollEndHook = 0x00403C81; 
static const UInt32 kPollEndRetn = 0x00403C85;
static const UInt32 kPollEndNoMouseHook = 0x0040483A;
static const UInt32 kPollEndNoMouseRetn = 0x0040483C;
static const UInt32 kLoc_403C40 = 0x00403C40;
static const UInt32 kBufferedKeyHook = 0x00403C90;
static const UInt32 kInputGlobalAllocSize = 0x00404A77;
static const UInt32 kInputInitializeCallAddr = 0x00404A92;
static const UInt32 kInitializeInputGlobals = 0x00404150;


enum  KeyControlState : UInt8 {
	kStateUnmodified = 0,
	kStateDisabled   = 1 << 0,  //The button is disabled (the game will not react to presses)
	kStateSignalled  = 1 << 1,  //The button is signalled (a real press was detected)
	kStateTap		 = 1 << 2,  //The button is tapped (the game react to a single press)
	kStateHolded	 = 1 << 3,  //The button is holded (react to continuos ppesses)
	kStateHammered   = 1 << 4,  //The button is hammered (react to press in odd frames)
	kStateAHammered  = 1 << 5,  //The button is AHammered (react to press in even frames)
	kStatePSignalled = 1 << 6,  //The button was signalled  (real press detected for previous frame)
};
DEFINE_ENUM_FLAG_OPERATORS(KeyControlState)

enum KeyEvent {
	KeyEvent_Down = 0,
	KeyEvent_Up   = 1,
	KeyEvent_Hold = 2,
};

struct DIMOUSESTATEInn {
	LONG    lX;
	LONG    lY;
	LONG    lZ;
	KeyControlState    rgbButtons[8];
};

template <typename T>
class OSNode{
public:
	T val;
	OSNode<T>* next;
};

template <typename T>
class OSQueue{
private:
	OSNode<T>* head = nullptr;
	OSNode<T>* tail = nullptr;
public:
	void AddLast(T item){
		if(tail == nullptr){  //when head is null, tail is null too
			head = (OSNode<T>*) FormHeap_Allocate(sizeof(OSNode<T>));
			if(head == nullptr)
				_ERROR("Can't allocate input");
			tail = head;
			tail->val = item;
			tail->next = nullptr;
		}
		else{
			tail->next = (OSNode<T>*) FormHeap_Allocate(sizeof(OSNode<T>));
			if(tail->next == nullptr) _ERROR("Can't allocate input");
			tail = tail->next;
			tail->next = nullptr;
			tail->val = item;
		}
	}

	T RemoveFirst(){
		if(head == nullptr) {
			_ERROR("Unreachable code reached");
			return T{};   //Should be unreachable
		}
		else{
			OSNode<T>* node = head;
			head = node->next;
			if(head == nullptr) tail = nullptr;
			T val = node->val;
			FormHeap_Free(node);
			return val;
		}
	}


	bool HasElement(){return head != nullptr;}
};

class OSInputGlobalsEx : public OSInputGlobals {
public:
	KeyControlState	KeyMaskState[256];
	DIMOUSESTATEInn MouseMaskState;
	UInt32			EventCode[2];
	float			MouseAxisMovementPerSecond[2]; 
	float			MouseAxisAccumulator[2];
	float			lastFrameLength;
	DWORD			lastFrameTime;
	OSQueue<DIDEVICEOBJECTDATA> fakeBuffered;

	void SetMask(UInt16 keycode);
	void SetUnMask(UInt16 keycode);
	UInt8 GetMaskStatus(UInt16 keycode);
	void SetTap(UInt16 keycode);
	void SetHold(UInt16 keycode);
	void SetUnHold(UInt16 keycode);
	void SetHammer(UInt16 keycode, bool AHammer);
	void SetUnHammer(UInt16 keycode);
	bool IsKeyPressedSimulated(UInt16 keycode);  /*Report simulated keypresses*/
	bool WasKeyPressedSimulated(UInt16 keycode);
	bool IsKeyPressedReal(UInt16 keycode);  /*Report real keypresses*/
	bool WasKeyPressedReal(UInt16 keycode);
	void FakeBufferedKeyTap(UInt32 key);
	void FakeBufferedKeyPress(UInt32 key);
	void FakeBufferedKeyRelease(UInt32 key);

	OSInputGlobalsEx* InitializeEx(IDirectInputDevice8* device);
	void InputPollFakeHandle();
	int GetBufferedKeyStateChangeHook(DIDEVICEOBJECTDATA* data);
private:
	inline void SendKeyEvent(UInt16 idx);
	inline void SendMouseEvent(UInt16 idx);
	inline void SendControlEvents();
	inline void SendWheelEvents();

};

STATIC_ASSERT(sizeof(OSInputGlobalsEx) == sizeof(OSInputGlobals) + sizeof(DIMOUSESTATEInn) + sizeof(KeyControlState[256]) + sizeof(UInt32[2]) + 2*sizeof(float[2]) + sizeof(float) +sizeof(DWORD) +sizeof(OSQueue<DIDEVICEOBJECTDATA>) );

extern OSInputGlobalsEx* g_inputGlobal;

void Hook_Input_Init();

namespace PluginAPI {
	void DisableKey(UInt16 dxCode);
	void EnableKey(UInt16 dxCode);
	void DisableControl(UInt16 controlCode);
	void EnableControl(UInt16 controlCode);
	bool IsKeyPressedReal(UInt16 dxCode);
	bool IsKeyPressedSimulated(UInt16 dxCode);
	bool IsControlPressedReal(UInt16 controlCode);
	bool IsControlPressedSimulated(UInt16 controlCode);
}
