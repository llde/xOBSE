#pragma once

#include "CommandTable.h"

void Hook_Memory_Init(void);
void Hook_Memory_DeInit(void);

UInt32 GetPoolAllocationSize(void * buf);

void Hook_Memory_CheckAllocs(void);

extern CommandInfo	kCommandInfo_DebugMemDump;
