#include "NiNodes.h"

void TextureFormat::InitFromD3DFMT(UInt32 fmt)
{
	typedef void (* _D3DFMTToTextureFormat)(UInt32 d3dfmt, TextureFormat * dst);
	_D3DFMTToTextureFormat D3DFMTToTextureFormat = (_D3DFMTToTextureFormat)0x0076C3B0;

	D3DFMTToTextureFormat(fmt, this);
}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static const UInt32 kNiObjectNET_SetNameAddr = 0x006FF420;

// an array of structs describing each of the game's anim groups
static const TESAnimGroup::AnimGroupInfo* s_animGroupInfos = (const TESAnimGroup::AnimGroupInfo*)0x00B102E0;
#else
#unsupported Oblivion version
#endif

void NiObjectNET::SetName(const char* newName)
{
#if OBLIVION
	ThisStdCall(kNiObjectNET_SetNameAddr, this, newName);
#endif
}

#if OBLIVION
const char* TESAnimGroup::StringForAnimGroupCode(UInt32 groupCode)
{
	return (groupCode < TESAnimGroup::kAnimGroup_Max) ? s_animGroupInfos[groupCode].name : NULL;
}

UInt32 TESAnimGroup::AnimGroupForString(const char* groupName)
{
	for (UInt32 i = 0; i < TESAnimGroup::kAnimGroup_Max; i++) {
		if (!_stricmp(s_animGroupInfos[i].name, groupName)) {
			return i;
		}
	}

	return TESAnimGroup::kAnimGroup_Max;
}
#endif
