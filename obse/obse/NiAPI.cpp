#include "obse/NiAPI.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

NiNode ** g_worldSceneGraph = (NiNode **)0x00AEABEC;
NiNode ** g_interfaceSceneGraph = (NiNode **)0x00AEACE8;
NiNode ** g_interface3DSceneGraph = (NiNode **)0x00AEABB0;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2

NiNode ** g_worldSceneGraph = (NiNode **)0x00B333CC;
NiNode ** g_interfaceSceneGraph = (NiNode **)0x00B333D0;
NiNode ** g_interface3DSceneGraph = (NiNode **)0x00B333D4;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

NiNode ** g_worldSceneGraph = (NiNode **)0x00B333CC;
NiNode ** g_interfaceSceneGraph = (NiNode **)0x00B333D0;
NiNode ** g_interface3DSceneGraph = (NiNode **)0x00B333D4;

#else

#error unsupported version of oblivion

#endif
