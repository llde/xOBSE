#include "GameOSDepend.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

OSGlobals ** g_osGlobals = (OSGlobals **)0x00AEAAB8;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2

OSGlobals ** g_osGlobals = (OSGlobals **)0x00B33398;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

OSGlobals ** g_osGlobals = (OSGlobals **)0x00B33398;

#else

#error unsupported version of oblivion

#endif
