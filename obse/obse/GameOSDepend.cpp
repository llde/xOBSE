#include "GameOSDepend.h"

OSGlobals** g_osGlobals = (OSGlobals**)0x00B33398;


OSInputGlobals* OSInputGlobals::GetInstance() { return (*g_osGlobals)->input; }
