#include "GameMagicEffects.h"
#include "GameAPI.h"
#include "GameForms.h"

void ActiveEffect::Remove(bool bRemoveImmediately)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x0068EA10, this, bRemoveImmediately);
#else
#error unsupported oblivion version
#endif
}

bool AssociatedItemEffect::IsBoundItemEffect() const
{ 
	return effectItem && effectItem->setting && effectItem->setting->IsBoundItemEffect(); 
}

bool AssociatedItemEffect::IsSummonEffect() const
{ 
	return effectItem && effectItem->setting && effectItem->setting->IsSummonEffect(); 
}
