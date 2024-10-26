#include "Hooks_Memory.h"
#include "GameAPI.h"
#include "obse_common/SafeWrite.h"
#include "common/ICriticalSection.h"
#include <share.h>
#include <set>

// enable everything
#define ENABLE_MEMORY_DEBUG 0 && _DEBUG

// clear new allocs to CDCDCDCD and freed buffers to DEDEDEDE
#define CLEAR_MEMORY 1

// write heap allocations and deallcations to a log file
#define LOG_MEMORY 0

// keep track of allocations, checking for double-frees or assignment to the same address twice
#define TRACK_ALLOCATIONS 0

// use custom heap implemetation allocating a guard page at the end of each selected memory block
#define HEAP_USE_GUARDPAGE 1

// slightly overallocate allocs to add a check value at the start and end
#define BASIC_CHECK_OVERFLOWS 0

static const UInt32 kMemOverflowMarkerStart = 0xDEADBEEF;
static const UInt32 kMemOverflowMarkerEnd = 0xEAC15A55;

MemoryPool **	g_memoryHeap_poolsBySize = (MemoryPool **)0x00B33080;		// size = 0x81
MemoryPool **	g_memoryHeap_poolsByAddress = (MemoryPool **)0x00B32C80;	// size = 0x100

static const UInt32 kMemoryHeap_Allocate_Addr = 0x00401AA7;
static const UInt32 kMemoryHeap_Free_Addr = 0x00401D46;

// note: this function needs to be very selective as it makes each allocation at least 8K
static bool ShouldUseGuardpage(UInt32 size)
{
	if(size == 0x44) return true;

	return false;
}

static MemoryPool * GetAllocationPool(void * buf)
{
	UInt32	buf32 = (UInt32)buf;

	MemoryPool	* pool = g_memoryHeap_poolsByAddress[buf32 >> 24];
	
	if(!pool || !pool->IsMember(buf)) return NULL;
	
	return pool;
}

static UInt32 g_heapAllocCurrent = 0;	// number of non-freed heap allocations

typedef std::map <void *, UInt32>	AllocationInfoMap;

static AllocationInfoMap	g_allocationInfoMap;
static ICriticalSection		g_allocationInfoMapLock;

void Hook_Memory_CheckAllocs(void)
{
	g_allocationInfoMapLock.Enter();

	for(AllocationInfoMap::iterator iter = g_allocationInfoMap.begin(); iter != g_allocationInfoMap.end(); ++iter)
	{
		UInt8	* buf8 = (UInt8 *)iter->first;
		UInt32	size = iter->second;

		ASSERT(*((UInt32 *)buf8) == kMemOverflowMarkerStart);
		ASSERT(*((UInt32 *)(buf8 + 4 + size)) == kMemOverflowMarkerEnd);
	}

	g_allocationInfoMapLock.Leave();
}

#if HEAP_USE_GUARDPAGE
typedef std::set <void *>	GuardpageAllocList;

static GuardpageAllocList	g_guardpageAllocList;
static ICriticalSection		g_guardpageAllocListLock;
#endif

#pragma optimize("", off)

__declspec(naked) void * MemoryHeap::Allocate(UInt32 size, UInt32 unk)
{
	__asm
	{
		push	esi
		mov		esi, ecx
		cmp		dword ptr [esi+0x0C], 0
		jmp		[kMemoryHeap_Allocate_Addr]
	}
}

__declspec(naked) void MemoryHeap::Free(void * ptr)
{
	__asm
	{
		push	esi
		push	edi
		mov		edi, [esp+0x0C]
		jmp		[kMemoryHeap_Free_Addr]
	}
}

FILE	* memLog = NULL;

#pragma warning (disable : 4313)

static const UInt32 kPageSize = 4096;
static const UInt32 kPageSizeMask = kPageSize - 1;

static void * __stdcall MemoryHeap_Allocate_Hook(UInt32 size, UInt32 unk)
{
	void	* result;
	
#if HEAP_USE_GUARDPAGE

	size = (size + 3) & ~3;

	if(ShouldUseGuardpage(size))
	{
		UInt32	sizeInPages = (size + kPageSizeMask) & ~kPageSizeMask;
		UInt32	alignSize = sizeInPages - size;

		UInt8	* base = (UInt8 *)VirtualAlloc(NULL, sizeInPages + kPageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		ASSERT(base);

		VirtualProtect(base + sizeInPages, kPageSize, PAGE_NOACCESS, NULL);

		result = base + alignSize;

		g_guardpageAllocListLock.Enter();
		g_guardpageAllocList.insert(result);
		g_guardpageAllocListLock.Leave();
	}
	else
	{
		result = g_formHeap->Allocate(size, unk);
	}
#else

#if BASIC_CHECK_OVERFLOWS

	{
		result = g_formHeap->Allocate(size + 8, unk);

		g_allocationInfoMapLock.Enter();
		g_allocationInfoMap[result] = size;
		g_allocationInfoMapLock.Leave();

		UInt8	* buf8 = (UInt8 *)result;

		*((UInt32 *)buf8) = kMemOverflowMarkerStart;
		result = buf8 + 4;
		*((UInt32 *)(buf8 + 4 + size)) = kMemOverflowMarkerEnd;
	}

#else
	result = g_formHeap->Allocate(size, unk);
#endif

#endif

	if(g_formHeap->IsMainHeapAllocation(result))
	{
		g_heapAllocCurrent++;

#if LOG_MEMORY
		if(unk != 1)
			fprintf(memLog, "a %08X %08X %08X\n", result, size, unk);
		else
			fprintf(memLog, "a %08X %08X\n", result, size);
#endif

#if TRACK_ALLOCATIONS
		{
			g_allocationInfoMapLock.Enter();

			AllocationInfoMap::iterator	iter = g_allocationInfoMap.find(result);
			if(iter == g_allocationInfoMap.end())
			{
				g_allocationInfoMap[result] = size;
			}
			else
			{
				fprintf(memLog, "### double allocation at address %08X (old size %08X new size %08X)\n",
					result, g_allocationInfoMap[result], size);
				fflush(memLog);
			}

			g_allocationInfoMapLock.Leave();
		}
#endif
	}

#if CLEAR_MEMORY
	if(result)
	{
		memset(result, 0xCDCDCDCD, size);
	}
#endif

	return result;
}

static void __stdcall MemoryHeap_Free_Hook(void * ptr)
{
#if BASIC_CHECK_OVERFLOWS
	{
		ptr = ((UInt8 *)ptr) - 4;
		ASSERT(*((UInt32 *)ptr) == kMemOverflowMarkerStart);

		g_allocationInfoMapLock.Enter();
		AllocationInfoMap::iterator	iter = g_allocationInfoMap.find(ptr);
		ASSERT(iter != g_allocationInfoMap.end());

		UInt32	size = iter->second;
		g_allocationInfoMap.erase(iter);
		g_allocationInfoMapLock.Leave();

		ASSERT(*((UInt32 *)(((UInt8 *)ptr) + 4 + size)) == kMemOverflowMarkerEnd);
	}
#endif

	if(g_formHeap->IsMainHeapAllocation(ptr))
	{
#if LOG_MEMORY
		fprintf(memLog, "d %08X\n", ptr);
#endif

		g_heapAllocCurrent--;
		if(g_heapAllocCurrent == 0xFFFFFFFF)
		{
			fprintf(memLog, "%x double free\n", (UInt32) ptr);
			fflush(memLog);

			g_heapAllocCurrent = 0;
		}

#if TRACK_ALLOCATIONS
		{
			g_allocationInfoMapLock.Enter();

			AllocationInfoMap::iterator	iter = g_allocationInfoMap.find(ptr);
			if(iter == g_allocationInfoMap.end())
			{
				fprintf(memLog, "### bad free of %08X\n", ptr);
				fflush(memLog);
			}
			else
			{
				g_allocationInfoMap.erase(iter);
			}

			g_allocationInfoMapLock.Leave();
		}
#endif
	}

	if(ptr)
	{
#if CLEAR_MEMORY
		{
			MemoryPool	* pool = GetAllocationPool(ptr);
			if(pool)
				memset(ptr, 0xDEDEDEDE, pool->field_100);
		}
#endif

#if HEAP_USE_GUARDPAGE
		bool	isGuardpageAlloc = false;

		g_guardpageAllocListLock.Enter();
		GuardpageAllocList::iterator	iter = g_guardpageAllocList.find(ptr);
		if(iter != g_guardpageAllocList.end())
		{
			isGuardpageAlloc = true;
			g_guardpageAllocList.erase(iter);
		}
		g_guardpageAllocListLock.Leave();

		if(isGuardpageAlloc)
		{
			ptr = (void *)(((UInt32)ptr) & ~kPageSizeMask);

			VirtualFree(ptr, 0, MEM_RELEASE);
		}
		else
		{
			g_formHeap->Free(ptr);
		}
#else
		g_formHeap->Free(ptr);
#endif
	}
}

#pragma optimize("", on)

void Hook_Memory_Init(void)
{
#if ENABLE_MEMORY_DEBUG

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	WriteRelJump(0x00401AA0, (UInt32)MemoryHeap_Allocate_Hook);
	WriteRelJump(0x00401D40, (UInt32)MemoryHeap_Free_Hook);
#endif

	memLog = _fsopen("h:\\memlog.txt", "w", _SH_DENYWR);

#endif
}

void Hook_Memory_DeInit()
{
#if ENABLE_MEMORY_DEBUG

#if TRACK_ALLOCATIONS
	if(!g_allocationInfoMap.empty())
	{
		fprintf(memLog, "%d leaked allocations\n", g_allocationInfoMap.size());

		for(AllocationInfoMap::iterator iter = g_allocationInfoMap.begin(); iter != g_allocationInfoMap.end(); ++iter)
		{
			fprintf(memLog, "@%08X size %08X\n", iter->first, iter->second);
		}
	}
#endif

	fclose(memLog);
	memLog = NULL;

#endif
}

UInt32 GetPoolAllocationSize(void * buf)
{
	UInt32	result = 0;
	UInt32	buf32 = (UInt32)buf;

	MemoryPool	* pool = g_memoryHeap_poolsByAddress[buf32 >> 24];
	if(pool)
		result = pool->field_100;

	return result;
}

bool Cmd_DebugMemDump_Execute(COMMAND_ARGS)
{
	for(UInt32 i = 0; i <= 0x81; i++)
	{
		MemoryPool	* pool = g_memoryHeap_poolsBySize[i];

		_MESSAGE("%03X: %08X", i << 2, pool);
		gLog.Indent();

		if(pool)
		{
			_MESSAGE("name = %s buffer = %08X size = %08X entry size = %08X page index size = %08X",
				pool->m_name,
				pool->field_040,
				pool->field_110,
				pool->field_100,
				pool->field_10C);

			gLog.Indent();

			for(UInt32 j = 0; j < pool->field_10C; j++)
			{
				_MESSAGE("%08X: %04X", j, pool->field_108[j]);
			}

			gLog.Outdent();
		}

		gLog.Outdent();
	}

	return true;
}

CommandInfo	kCommandInfo_DebugMemDump =
{
	"DebugMemDump",
	"",
	0,
	"",
	0,
	0,
	NULL,
	HANDLER(Cmd_DebugMemDump_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
