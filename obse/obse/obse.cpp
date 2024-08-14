#include "obse.h"
#include "CommandTable.h"
#include "Hooks_Input.h"
#include "Hooks_Gameplay.h"
#include "Hooks_Memory.h"
#include "Hooks_SaveLoad.h"
#include "Hooks_Script.h"
#include "Commands_Math.h"
#include "PluginManager.h"
#include "InternalSerialization.h"
#include "HavokReflection.h"
#include "Hooks_NetImmerse.h"
#include "ThreadLocal.h"
#include "EventManager.h"
#include "Settings.h"

#include <winternl.h>
#include <shellapi.h>

#include "Detours\detours.h"
IDebugLog	gLog("obse.log");

std::string SteamExePath;
std::string BadSteamExePath;

std::wstring SteamExePathW;
std::wstring BadSteamExePathW;
/*Otherwise SteamStub bail out and try different paths, all invalids*/
void(__stdcall* Real_RtlDosPathNameToNtPathName_U)(const WCHAR*, UNICODE_STRING*, WCHAR**, void* );
void __stdcall Hooked_RtlDosPathNameToNtPathName_U(const WCHAR* DosName, UNICODE_STRING* NtName, WCHAR** PartName, void* relativeName) {
	if (_wcsicmp(DosName, BadSteamExePathW.c_str()) == 0  && DosName && NtName) {
		_MESSAGE("%S   %S", DosName, SteamExePathW.c_str());
		lstrcpyW((WCHAR*)DosName, SteamExePathW.c_str());
		return Real_RtlDosPathNameToNtPathName_U(SteamExePathW.c_str(), NtName ,  PartName, relativeName);
	}
	return Real_RtlDosPathNameToNtPathName_U(DosName, NtName, PartName, relativeName);
}

/*Allow a proper creation of the steam process*/
HINSTANCE (__cdecl* Real_ShellExecuteA)(HWND, char*, const char*, char*, char*, int);
HINSTANCE Hooked_ShellExecuteA(HWND   hwnd, char* lpOperation, const char* lpFile, char* lpParameters, char* lpDirectory, int nShowCmd) {
	if (_stricmp(lpFile, BadSteamExePath.c_str()) == 0) {
		return Real_ShellExecuteA(hwnd, lpOperation, SteamExePath.c_str(), lpParameters, lpDirectory, nShowCmd);
	}
	return Real_ShellExecuteA(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}


static void SanitizeSteam() {
	HKEY ActiveProcess;
	LSTATUS status = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		"Software\\Valve\\Steam\\ActiveProcess",
		0,
		KEY_READ,
		&ActiveProcess
	);
	if (status != ERROR_SUCCESS) {
		_MESSAGE("Cannot open ActiveProcess Steam key. Steam may be not installed properly, or wasn't run.");
		return;
	}
	char clientPath[MAX_PATH];
	UInt32 size = sizeof(clientPath);
	if (RegGetValue(ActiveProcess, NULL, "SteamClientDll", RRF_RT_REG_SZ, NULL, &clientPath, &size) != ERROR_SUCCESS) {
		_MESSAGE("Cannot get SteamClientDll path from registry. Steam may be not installed properly, or wasn't run.");
		return;
	}

	std::string path = clientPath;
	UInt32 idx = path.rfind('\\');
	path = path.substr(0, idx + 1).append("steam.exe");
	_MESSAGE("Steam Path %s", path.c_str());

	char	oblivionPathBuf[MAX_PATH];
	UInt32	oblivionPathLength = GetModuleFileName(GetModuleHandle(NULL), oblivionPathBuf, sizeof(oblivionPathBuf));
	std::string	runtimePath = oblivionPathBuf;
	std::string::size_type	lastSlash = runtimePath.rfind('\\');
	runtimePath = runtimePath.substr(0, lastSlash + 1).append("steam.exe");
	BadSteamExePathW = std::wstring(runtimePath.begin(), runtimePath.end());
	SteamExePathW = std::wstring(path.begin(), path.end());
	BadSteamExePath = runtimePath;
	SteamExePath = path;


	Real_RtlDosPathNameToNtPathName_U = (void(__stdcall *)(const WCHAR*, UNICODE_STRING*, WCHAR**, void*)) GetProcAddress(GetModuleHandle("ntdll"), "RtlDosPathNameToNtPathName_U");
	Real_ShellExecuteA = (HINSTANCE(__cdecl*)(HWND, char*, const char*, char*, char*, int)) GetProcAddress(GetModuleHandle("shell32"), "ShellExecuteA");

	if (Real_RtlDosPathNameToNtPathName_U == nullptr || Real_ShellExecuteA == nullptr) {
		_MESSAGE("No Hook found");
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)Real_RtlDosPathNameToNtPathName_U, Hooked_RtlDosPathNameToNtPathName_U);
	DetourAttach(&(PVOID&)Real_ShellExecuteA, Hooked_ShellExecuteA);
	DetourTransactionCommit();
}


extern "C" {
void OBSE_Initialize(void)
{
#ifndef _DEBUG


	__try {
#endif
		_MESSAGE("OBSE: initialize (version = %d.%d %08X)", OBSE_VERSION_INTEGER, OBSE_VERSION_INTEGER_MINOR, OBLIVION_VERSION_1_2_416);

#ifdef _DEBUG
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif

#if 0
		while(!IsDebuggerPresent())
		{
			Sleep(10);
		}

		Sleep(1000 * 2);
#endif
		InitializeSettings();
		if (installCrashdump)
			g_OriginalTopLevelExceptionFilter = SetUnhandledExceptionFilter(OBSEUnhandledExceptionFilter);

		SanitizeSteam();

		MersenneTwister::init_genrand(GetTickCount());

		CommandTable::Init();
		Hook_Input_Init();
		Hook_Gameplay_Init();
//		Hook_Memory_Init();
		Hook_SaveLoad_Init();
		Hook_Script_Init();
//		Hook_NetImmerse_Init();

//		HavokReflection_Init();

		Init_CoreSerialization_Callbacks();

		FlushInstructionCache(GetCurrentProcess(), NULL, 0);

		ThreadLocalData::Init();

		EventManager::Init();

#ifndef _DEBUG
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		_ERROR("exception");
	}
#endif
}

void OBSE_DeInitialize(void)
{
	_MESSAGE("OBSE: deinitialize");

	g_pluginManager.DeInit();

//	Hook_NetImmerse_DeInit();
//	Hook_Memory_DeInit();

	ThreadLocalData::DeInit();
}
};

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
	)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		OBSE_Initialize();
		break;

	case DLL_PROCESS_DETACH:
		OBSE_DeInitialize();
		break;
	};

	return TRUE;
}