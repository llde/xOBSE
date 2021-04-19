#include "GameActorValues.h"
#include "Utilities.h"
#include "GameAPI.h"
#include "PluginManager.h"

float ActorValues::GetAV(UInt32 avCode)
{
	float result = 0;
	ThisStdCall(0x0065CB80, this, avCode);
	__asm { fstp [result] }
	return result;
}

void ActorValues::ModAV(UInt32 avCode, float modBy, bool bAllowPositive)
{
	ThisStdCall(0x0065CA60, this, avCode, modBy, bAllowPositive);
}

float GetLuckModifiedSkill(SInt32 skill, SInt32 luck, bool capped)
{
	typedef float (* _fn)(SInt32 skill, SInt32 luck);
	static _fn fn = (_fn)0x00547B90;


	if (capped == false)
		return fn(skill, luck);

	float result = 0.f;
	SettingInfo* luckMultiplier = NULL, *luckBase = NULL;
	GetGameSetting("fActorLuckSkillMult", &luckMultiplier);
	GetGameSetting("iActorLuckSkillBase", &luckBase);

	if (luckMultiplier && luckBase)
	{
		result = luck * luckMultiplier->f;
		result += (luckBase->i + skill) * 1.0f;

		if (capped)
		{
			if (result > 100)
				result = 100;
			else if (result < 0)
				result = 0;
		}
	}

	return result;
}

UInt32 GetSkillMasteryLevel( UInt32 level )
{
	typedef UInt32 (* _fn)(UInt32 level);
	static _fn fn = (_fn)0x0056A300;

	return fn(level);

}
