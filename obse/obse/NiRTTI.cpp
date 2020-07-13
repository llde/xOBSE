#include "obse/NiRTTI.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

#include "NiRTTI_1_1.inl"

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

#include "NiRTTI_1_2_416.inl"

#else

#error unsupported oblivion version

#endif
