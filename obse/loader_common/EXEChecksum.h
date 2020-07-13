#pragma once

#include <string>

struct ProcHookInfo
{
	UInt32	hookCallAddr;
	UInt32	loadLibAddr;
	bool	steamVersion;
};

// procInfo only fully filled out for CS
bool TestChecksum(const char * procName, std::string * dllSuffix, ProcHookInfo * procInfo);

// stuff that doesn't belong here
void PrintError(const char * fmt, ...);
std::string GetCWD(void);
