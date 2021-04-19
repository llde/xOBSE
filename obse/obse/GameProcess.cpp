#include "GameProcess.h"
#include <algorithm>

ActorProcessManager * g_actorProcessManager = (ActorProcessManager*)0x00B3BD00;


bhkCharacterController* MiddleHighProcess::GetCharacterController()
{
	bhkCharacterController* ctrlr = (bhkCharacterController*)Oblivion_DynamicCast(charProxy, 0, RTTI_NiObject, RTTI_bhkCharacterController, 0);
	return ctrlr;
}

bool ci_equals(char ch1, char ch2)
{
	return tolower((unsigned char)ch1) == tolower((unsigned char)ch2);
}

bool ActorAnimData::PathsInclude(const char* subString)
{
	bool found = false;
	std::string subStr(subString);

	for (UInt32 idx = 0; idx < 5 && !found; idx++)
	{
		BSAnimGroupSequence* anim = animSequences[idx];
		if (anim && anim->filePath)
		{
			std::string pathStr(anim->filePath);
			found = (std::search(pathStr.begin(), pathStr.end(), subStr.begin(), subStr.end(), ci_equals) != pathStr.end());
		}
	}
	return found;
}

bool ActorAnimData::FindAnimInRange(UInt32 lowBound, UInt32 highBound)
{
	bool found = false;
	if (highBound == -1)
		highBound = lowBound;

	for (UInt32 idx = 0; idx < 5; idx++)
	{
		BSAnimGroupSequence* anim = animSequences[idx];
		found = (anim && anim->animGroup->animGroup >= lowBound && anim->animGroup->animGroup <= highBound);
		if (found)
			break;
	}
	return found;
}

PathBuilder* PathBuilder::GetSingleton()
{
	typedef PathBuilder* (* _GetPathBuilderSingleton)(void);
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static _GetPathBuilderSingleton func = (_GetPathBuilderSingleton)0x00683030;
	return func();
#else
#error unsupported Oblivion version
#endif
}
