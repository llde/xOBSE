#pragma once

#include "loader_common/EXEChecksum.h"

bool DoInjectDLL_New(PROCESS_INFORMATION * info, const char * dllPath, ProcHookInfo * hookInfo);
