#pragma once

#include "CommandTable.h"

extern CommandInfo kCommandInfo_GetAllPathNodes;		// for testing
extern CommandInfo kCommandInfo_IsPathNodeDisabled;
extern CommandInfo kCommandInfo_SetPathNodeDisabled;
extern CommandInfo kCommandInfo_GetPathNodePos;
extern CommandInfo kCommandInfo_SetPathNodePreferred;	// WIP
extern CommandInfo kCommandInfo_GetAllPathEdges;		// for testing
extern CommandInfo kCommandInfo_PathEdgeExists;
extern CommandInfo kCommandInfo_SetPathEdgeExists;		// WIP, may not be practical
extern CommandInfo kCommandInfo_GetPathNodeLinkedRef;

extern CommandInfo kCommandInfo_GetPathNodesInRadius;
extern CommandInfo kCommandInfo_GetPathNodesInRect;