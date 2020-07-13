#pragma once

void Hook_SaveLoad_Init(void);

extern UInt32	g_gameLoaded;

void __stdcall ToggleScriptExecution(bool bEnable);
bool AreScriptsDisabled();