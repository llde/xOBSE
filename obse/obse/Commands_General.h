#pragma once

#include "CommandTable.h"

extern CommandInfo kCommandInfo_Let;
extern CommandInfo kCommandInfo_eval;
extern CommandInfo kCommandInfo_While;
extern CommandInfo kCommandInfo_Loop;
extern CommandInfo kCommandInfo_ForEach;
extern CommandInfo kCommandInfo_Continue;
extern CommandInfo kCommandInfo_Break;
extern CommandInfo kCommandInfo_ToString;
extern CommandInfo kCommandInfo_Print;
extern CommandInfo kCommandInfo_testexpr;
extern CommandInfo kCommandInfo_TypeOf;

extern CommandInfo kCommandInfo_Function;
extern CommandInfo kCommandInfo_Call;
extern CommandInfo kCommandInfo_SetFunctionValue;

extern CommandInfo kCommandInfo_GetUserTime;

extern CommandInfo kCommandInfo_GetModLocalData;
extern CommandInfo kCommandInfo_SetModLocalData;
extern CommandInfo kCommandInfo_ModLocalDataExists;
extern CommandInfo kCommandInfo_RemoveModLocalData;

extern CommandInfo kCommandInfo_GetAllModLocalData;

extern CommandInfo kCommandInfo_Internal_PushExecutionContext;
extern CommandInfo kCommandInfo_Internal_PopExecutionContext;