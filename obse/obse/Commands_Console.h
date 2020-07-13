#pragma once

#include "CommandTable.h"

extern CommandInfo kCommandInfo_PrintToConsole;
extern CommandInfo kCommandInfo_PrintToFile;
extern CommandInfo kCommandInfo_RunBatchScript;

extern CommandInfo kCommandInfo_SetDebugMode;
extern CommandInfo kCommandInfo_DebugPrint;
extern CommandInfo kCommandInfo_RunScriptLine;

extern CommandInfo kCommandInfo_DBG_echo;

extern CommandInfo kCommandInfo_PrintD;

extern CommandInfo kCommandInfo_ToggleDebugText2;

bool RunScriptLine(const char * buf);
bool RunScriptLineOnREFR(const char * buf, TESObjectREFR* callingObj, bool bSuppressOutput = false);
