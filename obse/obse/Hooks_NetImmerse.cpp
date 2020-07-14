#include "Hooks_NetImmerse.h"
#include "NiObjects.h"
#include "obse_common/SafeWrite.h"
#include "common/ICriticalSection.h"
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#include <hash_map>

#define ENABLE_NETIMMERSE_DEBUG 0
#define ENABLE_INTERLOCKED_HOOKS 0
#define ENABLE_CTOR_HOOKS 0

struct ObjectInfo
{
	ObjectInfo()
		:flags(0), type(NULL) { }

	enum
	{
		kFlag_PrintedType =	1 << 0,
	};
	
	UInt32	flags;
	NiRTTI	* type;
};

typedef stdext::hash_map <NiObjectNET *, ObjectInfo>	NiObjectList;

static ICriticalSection	g_lock;
FILE					* g_log = NULL;
NiObjectList			g_objs;

static void Log(const char * fmt, ...)
{
	va_list	args;

	va_start(args, fmt);
	vfprintf(g_log, fmt, args);
	va_end(args);

	fflush(g_log);
}

static void CheckObject(NiObjectList::iterator * iter)
{
	NiObjectNET	* obj = (*iter)->first;
	ObjectInfo	* info = &(*iter)->second;

	if(!(info->flags & ObjectInfo::kFlag_PrintedType))
	{
		info->type = obj->GetType();
		if(info->type && info->type->name)
			Log("t %08X %s\n", obj, info->type->name);

		info->flags |= ObjectInfo::kFlag_PrintedType;
	}
}

static void __stdcall NiObjectNET_ctor_Hook(NiObjectNET * obj)
{
#if ENABLE_CTOR_HOOKS
	g_lock.Enter();

	Log("c %08X\n", obj);

	if(g_objs.find(obj) == g_objs.end())
		g_objs[obj] = ObjectInfo();
	else
		Log("### duplicate\n");

	g_lock.Leave();
#endif
}

static void __stdcall NiObjectNET_dtor_Hook(NiObjectNET * obj)
{
#if ENABLE_CTOR_HOOKS
	g_lock.Enter();

	NiObjectList::iterator	iter = g_objs.find(obj);

	if(iter == g_objs.end())
	{
//		Log("### not in list\n");
		Log("d %08X ### not in list\n", obj);
	}
	else
	{
		Log("d %08X%s%s%s\n", obj,
			iter->second.flags & ObjectInfo::kFlag_PrintedType ? "" : " missed_type",
			((UInt32)obj) < 0x00BAC200 ? " static_alloc" : "",
			obj->m_uiRefCount ? " has_refs" : "");

		g_objs.erase(iter);
	}

	g_lock.Leave();
#endif
}

static LONG WINAPI InterlockedIncrement_Hook(LONG volatile * ptr)
{
#if ENABLE_INTERLOCKED_HOOKS
	NiObjectNET	* obj = (NiObjectNET *)(ptr - 1);

	g_lock.Enter();

	NiObjectList::iterator	iter = g_objs.find(obj);
	if(iter != g_objs.end())
	{
		Log("+ %08X %X\n", obj, *ptr);
		CheckObject(&iter);
	}

	g_lock.Leave();
#endif

	return InterlockedIncrement(ptr);
}

static LONG WINAPI InterlockedDecrement_Hook(LONG volatile * ptr)
{
#if ENABLE_INTERLOCKED_HOOKS
	NiObjectNET	* obj = (NiObjectNET *)(ptr - 1);

	g_lock.Enter();

	NiObjectList::iterator	iter = g_objs.find(obj);
	if(iter != g_objs.end())
	{
		Log("- %08X %X\n", obj, *ptr);
		CheckObject(&iter);
	}

	g_lock.Leave();
#endif

	return InterlockedDecrement(ptr);
}

static const UInt32	kNiObjectNET_ctor_Hook = (UInt32)&NiObjectNET_ctor_Hook;
static const UInt32 kNiObject_ctor = 0x007005D0;

__declspec(naked) void _NiObjectNET_ctor_Hook(void)
{
	__asm
	{
		pushad
		push	ecx
		call	[kNiObjectNET_ctor_Hook]
		popad

		jmp		[kNiObject_ctor]
	}
}

static const UInt32 kNiObjectNET_dtor_Hook = (UInt32)&NiObjectNET_dtor_Hook;
static const UInt32 kNiObjectNET_dtor_Hook_retn = 0x006FFD77;

__declspec(naked) void _NiObjectNET_dtor_Hook(void)
{
	__asm
	{
		pushad
		push	ecx
		call	[kNiObjectNET_dtor_Hook]
		popad

		push	0xFFFFFFFF
		push	0x009C9333
		jmp		[kNiObjectNET_dtor_Hook_retn]
	}
}

void Hook_NetImmerse_Init(void)
{
#if ENABLE_NETIMMERSE_DEBUG
	g_log = fopen("c:\\ni.txt", "w");

	WriteRelCall(0x006FFD33, (UInt32)&_NiObjectNET_ctor_Hook);
	WriteRelJump(0x006FFD70, (UInt32)&_NiObjectNET_dtor_Hook);
	SafeWrite32(0x00A28078, (UInt32)&InterlockedIncrement_Hook);
	SafeWrite32(0x00A2807C, (UInt32)&InterlockedDecrement_Hook);
#endif
}

void Hook_NetImmerse_DeInit(void)
{
#if ENABLE_NETIMMERSE_DEBUG
	for(NiObjectList::iterator iter = g_objs.begin(); iter != g_objs.end(); ++iter)
	{
		NiObjectNET	* obj = iter->first;
		ObjectInfo	* info = &iter->second;

		Log("leaked %08X (%X %s)\n", obj, obj->m_uiRefCount,
			info->type ? info->type->name : "<no rtti>");
	}

	fclose(g_log);
#endif
}
