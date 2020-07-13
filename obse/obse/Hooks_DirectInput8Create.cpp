#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Hooks_DirectInput8Create.h"
#include "obse_common/SafeWrite.h"
#include <queue>

static const GUID GUID_SysMouse    = { 0x6F1D2B60, 0xD5A0, 0x11CF, { 0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };
static const GUID GUID_SysKeyboard = { 0x6F1D2B61, 0xD5A0, 0x11CF, { 0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

sDI_data DI_data;
static DWORD DICreate_RealFunc;
static std::queue<DIDEVICEOBJECTDATA> bufferedPresses;

// remove the 'assume 30fps' kludge by calculating the last frame length each time oblivion requests the mouse state
static DWORD fps_LastTime=0;
static float fps_LastFrameLength=0;	// frametime in seconds
static const UInt32 kFrameTimeHistoryLength = 20;
static float fps_FrameTimeHistory[kFrameTimeHistoryLength] = { 0 };
static UInt32 fps_FrameTimeHistoryIdx = 0;
static UInt32 fps_FrameTimeHistoryNum = 0;
static float fps_AverageFrameTime = 0;

sDI_BufferedKeyPress::sDI_BufferedKeyPress(BYTE Key,bool Pressed) {
	key=Key;
	pressed=Pressed;
}

static bool ShouldIgnoreKey(DWORD code)
{
	return (code == DIK_GRAVE) || (code == DIK_ESCAPE);
}

void DInput_FakeBufferedKeyTap(UInt32 key) {
	DIDEVICEOBJECTDATA data;
	data.uAppData=-1;
	data.dwTimeStamp=GetTickCount();
	//If oblivion ever wants to use this we're in trouble. It shouldn't ever need to use it.
	data.dwSequence=0;
	data.dwOfs=key;
	data.dwData=0x80;
	bufferedPresses.push(data);
	data.dwData=0x00;
	bufferedPresses.push(data);
}
void DInput_FakeBufferedKeyPress(UInt32 key) {
	DIDEVICEOBJECTDATA data;
	data.uAppData=-1;
	data.dwTimeStamp=GetTickCount();
	data.dwSequence=0;
	data.dwOfs=key;
	data.dwData=0x80;
	bufferedPresses.push(data);
}
void DInput_FakeBufferedKeyRelease(UInt32 key) {
	DIDEVICEOBJECTDATA data;
	data.uAppData=-1;
	data.dwTimeStamp=GetTickCount();
	data.dwSequence=0;
	data.dwOfs=key;
	data.dwData=0x00;
	bufferedPresses.push(data);
}

class FakeDirectInputDevice : public IDirectInputDevice8 {
private:
    IDirectInputDevice8* RealDevice;
    DWORD DeviceType;
	ULONG Refs;
public:
    /*** Constructor and misc functions ***/
    FakeDirectInputDevice(IDirectInputDevice8* device,DWORD type) {
        RealDevice=device;
        DeviceType=type;
		Refs=1;
    }
    /*** IUnknown methods ***/
    HRESULT _stdcall QueryInterface (REFIID riid, LPVOID * ppvObj) { return RealDevice->QueryInterface(riid,ppvObj); }
    ULONG _stdcall AddRef(void) { return ++Refs; }
    ULONG _stdcall Release(void) {
		if(--Refs==0) {
			RealDevice->Release();
			delete this;
			return 0;
		} else { return Refs; }
	}

    /*** IDirectInputDevice8A methods ***/
    HRESULT _stdcall GetCapabilities(LPDIDEVCAPS a) { return RealDevice->GetCapabilities(a); }
    HRESULT _stdcall EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA a,LPVOID b,DWORD c) { return RealDevice->EnumObjects(a,b,c); }
    HRESULT _stdcall GetProperty(REFGUID a,DIPROPHEADER* b) { return RealDevice->GetProperty(a,b); }
    HRESULT _stdcall SetProperty(REFGUID a,const DIPROPHEADER* b) { return RealDevice->SetProperty(a,b); }
    HRESULT _stdcall Acquire(void) { return RealDevice->Acquire(); }
    HRESULT _stdcall Unacquire(void) { return RealDevice->Unacquire(); }
    HRESULT _stdcall GetDeviceState(DWORD a,LPVOID b) {
        if(DeviceType==kDeviceType_KEYBOARD) {
            //This is a keyboard, so get a list of bytes (Dont forget the mouse too)
            BYTE bytes[kMaxMacros];
            HRESULT hr=RealDevice->GetDeviceState(256,bytes);
            if(hr!=DI_OK) return hr;
            CopyMemory(&bytes[256],&DI_data.MouseOut,10);
			//Get any extra key presses
			DI_data.GlobalHammer = !DI_data.GlobalHammer;

			BYTE	* hammerList = DI_data.GlobalHammer ? DI_data.HammerStates : DI_data.AHammerStates;

			for(DWORD byte=0;byte<kMaxMacros-2;byte++)
			{
				if(!ShouldIgnoreKey(byte))
					bytes[byte] |= hammerList[byte];
			}

			for(DWORD byte=0;byte<256;byte++) {
				if(!ShouldIgnoreKey(byte))
				{
					bytes[byte]|=DI_data.FakeStates[byte];
					bytes[byte]&=DI_data.DisallowStates[byte];
				}

				if(DI_data.TapStates[byte]) {
					bytes[byte]=0x80;
					DI_data.TapStates[byte]=0;
				}
			}

			for(DWORD byte=256;byte<kMaxMacros-2;byte++) bytes[byte]|=DI_data.FakeStates[byte];

			::CopyMemory(b,bytes,a);
			::CopyMemory(DI_data.LastBytes,bytes,kMaxMacros);
			::CopyMemory(DI_data.MouseIn,&bytes[256],10);

			return DI_OK;
		} else {
            //This is a mouse
			
			//measure length of last frame
			DWORD time=GetTickCount();
			fps_LastFrameLength=(float)(time-fps_LastTime)/1000.0f;
			fps_LastTime=time;

			fps_FrameTimeHistory[fps_FrameTimeHistoryIdx] = fps_LastFrameLength;
			if(fps_FrameTimeHistoryIdx < kFrameTimeHistoryLength)
				fps_FrameTimeHistoryIdx++;
			else
				fps_FrameTimeHistoryIdx = 0;

			if(fps_FrameTimeHistoryNum < kFrameTimeHistoryLength)
				fps_FrameTimeHistoryNum++;
			else
			{
				float	total = 0;

				for(UInt32 i = 0; i < kFrameTimeHistoryLength; i++)
					total += fps_FrameTimeHistory[i];

				total /= ((float)kFrameTimeHistoryLength);

				fps_AverageFrameTime = total;
			}

			//Mouse control gunk
            DIMOUSESTATE2* MouseState=(DIMOUSESTATE2*)b;
            HRESULT hr=RealDevice->GetDeviceState(sizeof(DIMOUSESTATE2),MouseState);
            if(hr!=DI_OK) return hr;
            if(MouseState->lZ>0) {
                DI_data.MouseOut[8]=0x80;
                DI_data.MouseOut[9]=0;
            } else if(MouseState->lZ<0) {
                DI_data.MouseOut[8]=0;
                DI_data.MouseOut[9]=0x80;
            } else {
                DI_data.MouseOut[8]=0;
                DI_data.MouseOut[9]=0;
            }
            if(DI_data.MouseDisable) {
                MouseState->lX=0;
                MouseState->lY=0;
            }
            if(DI_data.MouseXMov) {
                MouseState->lX+=DI_data.MouseXMov;
                DI_data.MouseXMov=0;
            }
            if(DI_data.MouseYMov) {
                MouseState->lY+=DI_data.MouseYMov;
                DI_data.MouseYMov=0;
            }
            if(DI_data.MouseXSpeed) {
                float move=DI_data.MouseXSpeed*fps_LastFrameLength;
                MouseState->lX+=(long)move;
                if(DI_data.MouseXSpeed>0) {
                    DI_data.MouseXLeft+=fmodf(move,1.0f);
                    if(DI_data.MouseXLeft>1) {
                        MouseState->lX+=1;
                        DI_data.MouseXLeft-=1;
                    }
                } else {
                    DI_data.MouseXLeft-=fmodf(-move,1.0f);
                    if(DI_data.MouseXLeft<-1) {
                        MouseState->lX-=1;
                        DI_data.MouseXLeft+=1;
                    }
                }
            }
            if(DI_data.MouseYSpeed) {
                float move=DI_data.MouseYSpeed*fps_LastFrameLength;
                MouseState->lY+=(long)move;
                if(DI_data.MouseYSpeed>0) {
                    DI_data.MouseYLeft+=fmodf(move,1.0f);
                    if(DI_data.MouseYLeft>1) {
                        MouseState->lY+=1;
                        DI_data.MouseYLeft-=1;
                    }
                } else {
                    DI_data.MouseYLeft-=fmodf(-move,1.0f);
                    if(DI_data.MouseYLeft<-1) {
                        MouseState->lY-=1;
                        DI_data.MouseYLeft+=1;
                    }
                }
            }
            for(DWORD i=0;i<8;i++) {
                DI_data.MouseOut[i]=MouseState->rgbButtons[i];
                MouseState->rgbButtons[i]|=DI_data.MouseIn[i];
                MouseState->rgbButtons[i]&=DI_data.DisallowStates[i+256];
                if(DI_data.TapStates[i+256]) {
                    MouseState->rgbButtons[i]=0x80;
                    DI_data.TapStates[i+256]=0x00;
                }
            }
            return DI_OK;
        }
    }
	//In morrowind this only got called for keyboards when typing into the console. Probably no point in overriding it
	//And so it turns out that oblivion uses this whever it's in menumode instead of just consoles. Figures.
	HRESULT _stdcall GetDeviceData(DWORD a,DIDEVICEOBJECTDATA* b,DWORD* c,DWORD d) {
		if (bufferedPresses.empty())
			return RealDevice->GetDeviceData(a,b,c,d);
		if(!b) {
			DWORD temp=*c;
			HRESULT hr = RealDevice->GetDeviceData(a,b,c,d);
			if(c) *c=min(bufferedPresses.size(),temp);
			if(!(d|DIGDD_PEEK)) while(!bufferedPresses.empty()) bufferedPresses.pop();
			return hr;
		}
		int count=0;
		while (bufferedPresses.size()) {
			//Stricktly speaking, should return a buffer overflow by here, but if you do it breaks?
			//Presumably, if you could mash your keyboard fast enough, no keypresses would register...
			if(count==*c) return DI_OK; //DI_BUFFEROVERFLOW;
			//This will not work correctly if DIGDD_PEEK is specified. afaik, it's only ever used if b == NULL
			*b=bufferedPresses.front();
			bufferedPresses.pop();
			b+=sizeof(void*);
			count++;
		}
		if(count==*c) return DI_OK;
		//Can probably just return DI_OK here, because afaik *c is only ever 1 unless oblivion is trying to empty the buffer
		*c-=count;
		HRESULT hr=RealDevice->GetDeviceData(a,b,c,d);
		*c+=count;
		return hr;
	}
    HRESULT _stdcall SetDataFormat(const DIDATAFORMAT* a) { return RealDevice->SetDataFormat(a); }
    HRESULT _stdcall SetEventNotification(HANDLE a) { return RealDevice->SetEventNotification(a); }
    HRESULT _stdcall SetCooperativeLevel(HWND a,DWORD b) { return RealDevice->SetCooperativeLevel(a,b); }
    HRESULT _stdcall GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA a,DWORD b,DWORD c) { return RealDevice->GetObjectInfo(a,b,c); }
    HRESULT _stdcall GetDeviceInfo(LPDIDEVICEINSTANCEA a) { return RealDevice->GetDeviceInfo(a); }
    HRESULT _stdcall RunControlPanel(HWND a,DWORD b) { return RealDevice->RunControlPanel(a,b); }
    HRESULT _stdcall Initialize(HINSTANCE a,DWORD b,REFGUID c) { return RealDevice->Initialize(a,b,c); }
    HRESULT _stdcall CreateEffect(REFGUID a,LPCDIEFFECT b,LPDIRECTINPUTEFFECT *c,LPUNKNOWN d) { return RealDevice->CreateEffect(a,b,c,d); }
    HRESULT _stdcall EnumEffects(LPDIENUMEFFECTSCALLBACKA a,LPVOID b,DWORD c) { return RealDevice->EnumEffects(a,b,c); }
    HRESULT _stdcall GetEffectInfo(LPDIEFFECTINFOA a,REFGUID b) { return RealDevice->GetEffectInfo(a,b); }
    HRESULT _stdcall GetForceFeedbackState(LPDWORD a) { return RealDevice->GetForceFeedbackState(a); }
    HRESULT _stdcall SendForceFeedbackCommand(DWORD a) { return RealDevice->SendForceFeedbackCommand(a); }
    HRESULT _stdcall EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK a,LPVOID b,DWORD c) { return RealDevice->EnumCreatedEffectObjects(a,b,c); }
    HRESULT _stdcall Escape(LPDIEFFESCAPE a) { return RealDevice->Escape(a); }
    HRESULT _stdcall Poll(void) { return RealDevice->Poll(); }
    HRESULT _stdcall SendDeviceData(DWORD a,LPCDIDEVICEOBJECTDATA b,LPDWORD c,DWORD d) { return RealDevice->SendDeviceData(a,b,c,d); }
    HRESULT _stdcall EnumEffectsInFile(LPCSTR a,LPDIENUMEFFECTSINFILECALLBACK b,LPVOID c,DWORD d) { return RealDevice->EnumEffectsInFile(a,b,c,d); }
    HRESULT _stdcall WriteEffectToFile(LPCSTR a,DWORD b,LPDIFILEEFFECT c,DWORD d) { return RealDevice->WriteEffectToFile(a,b,c,d); }
    HRESULT _stdcall BuildActionMap(LPDIACTIONFORMATA a,LPCSTR b,DWORD c) { return RealDevice->BuildActionMap(a,b,c); }
    HRESULT _stdcall SetActionMap(LPDIACTIONFORMATA a,LPCSTR b,DWORD c) { return RealDevice->SetActionMap(a,b,c); }
    HRESULT _stdcall GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA a) { return RealDevice->GetImageInfo(a); }
};
class FakeDirectInput : public IDirectInput8A {
private:
    IDirectInput8* RealInput;
	ULONG Refs;
public:
    /*** Constructor ***/
    FakeDirectInput(IDirectInput8* Real) { RealInput=Real; Refs=1; }
    /*** IUnknown methods ***/
    HRESULT _stdcall QueryInterface (REFIID riid, LPVOID* ppvObj) { return RealInput->QueryInterface(riid,ppvObj); }
	ULONG _stdcall AddRef(void) { return ++Refs; }
    ULONG _stdcall Release(void) {
		if(--Refs==0) {
			RealInput->Release();
			delete this;
			return 0;
		} else { return Refs; }
    }
    /*** IDirectInput8A methods ***/
    HRESULT _stdcall CreateDevice(REFGUID r,IDirectInputDevice8A** device,IUnknown* unused) {
        if(r!=GUID_SysKeyboard&&r!=GUID_SysMouse) { 
			return RealInput->CreateDevice(r,device,unused);
		} else {
            DWORD d;
			IDirectInputDevice8A* RealDevice;
			HRESULT hr;

            if(r==GUID_SysKeyboard) d=kDeviceType_KEYBOARD;
            else d=kDeviceType_MOUSE;
            hr=RealInput->CreateDevice(r,&RealDevice,unused);
            if(hr!=DI_OK) return hr;
            *device=new FakeDirectInputDevice(RealDevice,d);
            return DI_OK;
        }
    }
    HRESULT _stdcall EnumDevices(DWORD a,LPDIENUMDEVICESCALLBACKA b,void* c,DWORD d) { return RealInput->EnumDevices(a,b,c,d); }
    HRESULT _stdcall GetDeviceStatus(REFGUID r) { return RealInput->GetDeviceStatus(r); }
    HRESULT _stdcall RunControlPanel(HWND a,DWORD b) { return RealInput->RunControlPanel(a,b); }
    HRESULT _stdcall Initialize(HINSTANCE a,DWORD b) { return RealInput->Initialize(a,b); }
    HRESULT _stdcall FindDevice(REFGUID a,LPCSTR b,LPGUID c) { return RealInput->FindDevice(a,b,c); }
    HRESULT _stdcall EnumDevicesBySemantics(LPCSTR a,LPDIACTIONFORMATA b,LPDIENUMDEVICESBYSEMANTICSCBA c,void* d,DWORD e) { return RealInput->EnumDevicesBySemantics(a,b,c,d,e); }
    HRESULT _stdcall ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK a,LPDICONFIGUREDEVICESPARAMSA b,DWORD c,void* d) { return RealInput->ConfigureDevices(a,b,c,d); }
};

float GetAverageFrameTime(void)
{
	return fps_AverageFrameTime;
}

UInt32 GetCurrentFrameIndex(void)
{
	return fps_FrameTimeHistoryIdx;
}

static HRESULT _stdcall Hook_DirectInput8Create_Execute(HINSTANCE a,DWORD b,REFIID c,void* d,IUnknown* e) {
    IDirectInput8A* dinput;
    HRESULT hr=((DInputProc)DICreate_RealFunc)(a,b,c,&dinput,e);
    if(hr!=DI_OK) return hr;
    *((IDirectInput8A**)d)=new FakeDirectInput(dinput);

    return DI_OK;
}

void Hook_DirectInput8Create_Init() {
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	UInt32	thunkAddress = 0x009FC02C;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	UInt32	thunkAddress = 0x00A2802C;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	UInt32	thunkAddress = 0x00A2802C;
#else
#error unsupported version of oblivion
#endif

	DICreate_RealFunc=*(DWORD*)thunkAddress;
	SafeWrite32(thunkAddress,(DWORD)Hook_DirectInput8Create_Execute);
	ZeroMemory(&DI_data,sizeof(DI_data));
    for(WORD w=0;w<kMaxMacros;w++) DI_data.DisallowStates[w]=0x80;
}

// Plugin API
bool Plugin_IsKeyPressed(UInt32 scancode)
{
	return (DI_data.LastBytes[scancode]) ? true : false;
}
