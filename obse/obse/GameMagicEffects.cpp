#include "GameMagicEffects.h"
#include "GameAPI.h"
#include "GameForms.h"

void ActiveEffect::Remove(bool bRemoveImmediately)
{
	ThisStdCall(0x0068EA10, this, bRemoveImmediately);
}

bool AssociatedItemEffect::IsBoundItemEffect() const
{ 
	return effectItem && effectItem->setting && effectItem->setting->IsBoundItemEffect(); 
}

bool AssociatedItemEffect::IsSummonEffect() const
{ 
	return effectItem && effectItem->setting && effectItem->setting->IsSummonEffect(); 
}
