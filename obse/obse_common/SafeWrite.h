#pragma once

void SafeWrite8(UInt32 addr, UInt32 data);
void SafeWrite16(UInt32 addr, UInt32 data);
void SafeWrite32(UInt32 addr, UInt32 data);
void SafeWriteBuf(UInt32 addr, void * data, UInt32 len);

// 5 bytes
void WriteRelJump(UInt32 jumpSrc, UInt32 jumpTgt);
void WriteRelCall(UInt32 jumpSrc, UInt32 jumpTgt);

// 6 bytes
void WriteRelJz(UInt32 jumpSrc, UInt32 jumpTgt);
void WriteRelJnz(UInt32 jumpSrc, UInt32 jumpTgt);
void WriteRelJle(UInt32 jumpSrc, UInt32 jumpTgt);

//Write arbitrary nop
void WriteNop(UInt32 nopAddr, UInt8 numOfByte);
//PAtch calls to address in a certain range
void PatchCallsInRange(UInt32 start, UInt32 end, UInt32 CallToPatch, UInt32 HookCall);