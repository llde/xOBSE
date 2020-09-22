#pragma once

#include "CommandTable.h"

extern CommandInfo kCommandInfo_IsScripted;
extern CommandInfo kCommandInfo_GetScript;
extern CommandInfo kCommandInfo_RemoveScript;
extern CommandInfo kCommandInfo_SetScript;

extern CommandInfo kCommandInfo_IsFormValid;
extern CommandInfo kCommandInfo_IsReference;
extern CommandInfo kCommandInfo_HasVariable;
extern CommandInfo kCommandInfo_GetVariable;
extern CommandInfo kCommandInfo_GetRefVariable;

extern CommandInfo kCommandInfo_CompareScripts;

extern CommandInfo kCommandInfo_ResetAllVariables;
extern CommandInfo kCommandInfo_GetFormFromMod;

extern CommandInfo kCommandInfo_GetNumExplicitRefs;
extern CommandInfo kCommandInfo_GetNthExplicitRef;

extern CommandInfo kCommandInfo_RunScripts;

extern CommandInfo kCommandInfo_RunScript;

extern CommandInfo kCommandInfo_GetArrayVariable;

extern CommandInfo kCommandInfo_SetEventHandler;
extern CommandInfo kCommandInfo_RemoveEventHandler;
extern CommandInfo kCommandInfo_EventHandlerExist;

extern CommandInfo kCommandInfo_GetCurrentEventName;

extern CommandInfo kCommandInfo_GetCurrentScript;
extern CommandInfo kCommandInfo_GetCallingScript;

extern CommandInfo kCommandInfo_DispatchEvent;