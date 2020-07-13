#pragma once
#include "Serialization.h"
#include "GameForms.h"
#include "GameObjects.h"

void Core_PostLoadCallback(bool bLoadSucceeded);
UInt8 ResolveModIndexForPreload(UInt8 modIndexIn);
void Init_CoreSerialization_Callbacks();