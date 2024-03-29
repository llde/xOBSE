
void SafeWrite8(UInt32 addr, UInt32 data)
{
	UInt32	oldProtect;

	VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt8 *)addr) = data;
	VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

void SafeWrite16(UInt32 addr, UInt32 data)
{
	UInt32	oldProtect;

	VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt16 *)addr) = data;
	VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

void SafeWrite32(UInt32 addr, UInt32 data)
{
	UInt32	oldProtect;

	VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt32 *)addr) = data;
	VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

void SafeWriteBuf(UInt32 addr, void * data, UInt32 len)
{
	UInt32	oldProtect;

	VirtualProtect((void *)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((void *)addr, data, len);
	VirtualProtect((void *)addr, len, oldProtect, &oldProtect);
}

void WriteRelJump(UInt32 jumpSrc, UInt32 jumpTgt)
{
	// jmp rel32
	SafeWrite8(jumpSrc, 0xE9);
	SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}

void WriteRelCall(UInt32 jumpSrc, UInt32 jumpTgt)
{
	// call rel32
	SafeWrite8(jumpSrc, 0xE8);
	SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}

void WriteRelJz(UInt32 jumpSrc, UInt32 jumpTgt)
{
	// jz rel32
	SafeWrite16(jumpSrc, 0x840F);
	SafeWrite32(jumpSrc + 2, jumpTgt - jumpSrc - 2 - 4);
}

void WriteRelJnz(UInt32 jumpSrc, UInt32 jumpTgt)
{
	// jnz rel32
	SafeWrite16(jumpSrc, 0x850F);
	SafeWrite32(jumpSrc + 2, jumpTgt - jumpSrc - 2 - 4);
}

void WriteRelJle(UInt32 jumpSrc, UInt32 jumpTgt)
{
	// jle rel32
	SafeWrite16(jumpSrc, 0x8E0F);
	SafeWrite32(jumpSrc + 2, jumpTgt - jumpSrc - 2 - 4);
}


void WriteNop(UInt32 nopAddr, UInt8 numOfByte) {
	for (UInt8 i = 0; i < numOfByte; i++) {
		SafeWrite8(nopAddr + i, 0x90);
	}
}

void PatchCallsInRange(UInt32 start, UInt32 end, UInt32 CallToPatch, UInt32 HookCall) {
	UInt32	oldProtect;

	VirtualProtect((void*)start , end - start, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (UInt32 current = start; current < end; current++) {
		if (*(UInt8*)current == 0xE8) {
			UInt32 callTarget = *(UInt32*)(current + 1) + current + 1 + 4;
	//		_MESSAGE("%08X", callTarget);
			if (callTarget == CallToPatch) {
	//			_MESSAGE("Call found");
				SafeWrite32(current + 1, HookCall - current - 1 - 4);
			}
		}
	}
	VirtualProtect((void*)start, end - start, oldProtect, &oldProtect);

	/*
	006020FB 054                 db  61h ; a
.text:006020FC 054                 db  59h ; Y
.text:006020FD 054                 db 0EAh ; �
.text:006020FE 054                 db 0FFh ; �
	*/
}