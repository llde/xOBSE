// Precompiled header
//	>> Include headers with stable code
#pragma once
// 4018 - signed/unsigned mismatch
// 4244 - loss of data by assignment
// 4267 - possible loss of data (truncation)
// 4305 - truncation by assignment
// 4288 - disable warning for crap Microsoft extension screwing up the scope of variables defined in for loops
// 4311 - pointer truncation
// 4312 - pointer extension
#pragma warning(disable: 4018 4244 4267 4305 4288 4312 4311 4800)

#define DPSAPI_VERSION	1

#define _CRT_NO_VA_START_VALIDATION
// WIN32
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <atltypes.h>
#include <commctrl.h>
#include <richedit.h>
#include <shlobj.h>
#include <Rpc.h>
#include <Dbghelp.h>
#include <uxtheme.h>
#include <Objbase.h>
#include <Psapi.h>

// CRT
#include <time.h>
#include <intrin.h>
#include <errno.h>
#include <crtdefs.h>
#include <malloc.h>
#include <stdio.h>

// STL
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <cmath>
#include <cwchar>
#include <ctime>
#include <algorithm>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <fstream>
#include <stdexcept>
#include <exception>
#include <sstream>
#include <memory>
#include <iostream>
#include <complex>
#include <iomanip>
#include <numeric>
#include <functional>

// RPC
#include <Rpc.h>

// DIRECTX
#include <d3d9.h>
#include <d3d9types.h>

// xSE Common
#include "common\ITypes.h"
#include "common\IErrors.h"
#include "common\IDynamicCreate.h"
#include "common\ISingleton.h"
#include "common\IDirectoryIterator.h"
#include "common\IFileStream.h"
#include "common\IDebugLog.h"