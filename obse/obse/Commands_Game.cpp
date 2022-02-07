#include "obse/Commands_Game.h"
#include "obse/ParamInfos.h"
#include "Script.h"

#ifdef OBLIVION

#include "GameAPI.h"
#include "Hooks_SaveLoad.h"
#include "GameForms.h"
#include "obse/Commands_Input.h"
#include "obse/GameMenus.h"
#include "GameData.h"
#include "GameOSDepend.h"
#include "Hooks_Gameplay.h"
#include "StringVar.h"
#include "ModTable.h"
#include "obse/GameObjects.h"
#include "obse_common/SafeWrite.h"
#include "NiObjects.h"
#include <obse/NiAPI.h>
#include <mbstring.h>
#include <obse/Hooks_Input.h>
#include "ArrayVar.h"
// first character in name mapped to type ID
//	b	0
//	c	1
//	h	2
//	i	3
//	u	4
//	f	5
//	S	6
//	s	6
//	r	7
//	a	8
//	anything else is 9

static bool Cmd_SetNumericGameSetting_Execute(COMMAND_ARGS)
{
	*result = 0;

	char	settingName[256] = { 0 };
	float	settingData = 0;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &settingName, &settingData))
		return true;

	SettingInfo	* setting = NULL;

	if(GetGameSetting(settingName, &setting))
	{
		if(setting && setting->name)
		{
			*result = 1;

			switch(setting->Type())
			{
				case SettingInfo::kSetting_Integer:
					setting->i = settingData;
					break;

				case SettingInfo::kSetting_Float:
					setting->f = settingData;
					break;

				case SettingInfo::kSetting_Unsigned:
					setting->u = settingData;
					break;

				default:
					*result = 0;
					break;
			}
		}
	}

	return true;
}

static INISettingEntry* GetIniSetting(const char* settingName)
{
	IniSettingCollection* settings = IniSettingCollection::GetSingleton ();
	for(INISettingEntry * entry = &settings->settingsList; entry; entry = entry->next)
	{
		INISettingEntry::Data	* data = entry->data;
		if(data && data->name && !_stricmp(settingName, data->name))
			return entry;
	}

	return NULL;
}

static bool Cmd_SetNumericINISetting_Execute(COMMAND_ARGS)
{
	*result = 0;

	char	settingName[256] = { 0 };
	float	settingData = 0;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &settingName, &settingData))
		return true;

	INISettingEntry* entry = GetIniSetting(settingName);
	if (entry)
	{
		INISettingEntry::Data* data = entry->data;
		*result = 1;

		switch(data->name[0])
		{
			case 'i':	// int
				data->i = settingData;
				break;

			case 'f':	// float
				data->f = settingData;
				break;

			case 'u':	// unsigned
				data->u = settingData;
				break;

			case 'b':	// bool
				data->b = (settingData != 0);
				break;

			default:
				*result = 0;
				break;
		}
	}

	return true;
}

bool Cmd_GetNumericINISetting_Execute(COMMAND_ARGS)
{
	*result = 0;

	char	settingName[256] = { 0 };

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &settingName))
		return true;

	INISettingEntry* entry = GetIniSetting(settingName);
	if (entry)
	{
		INISettingEntry::Data	* data = entry->data;
		if(data && data->name && !_stricmp(settingName, data->name))
		{
			switch(data->name[0])
			{
				case 'i':	// int
					*result = data->i;
					break;

				case 'f':	// float
					*result = data->f;
					break;

				case 'u':	// unsigned
					*result = data->u;
					break;

				case 'b':	// bool
					*result = data->b ? 1 : 0;
					break;

				default:
					*result = 0;
					break;
			}
		}
	}

	return true;
}

static bool Cmd_GetStringIniSetting_Execute(COMMAND_ARGS)
{
	char settingName[kMaxMessageLength] = { 0 };
	const char* settingString = "";

	if (ExtractFormatStringArgs(0, settingName, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, SIZEOF_FMT_STRING_PARAMS))
	{
		INISettingEntry* entry = GetIniSetting(settingName);
		if (entry && (entry->data->name[0] == 'S' || entry->data->name[0] == 's'))
			settingString = entry->data->s;
	}

	AssignToStringVar(PASS_COMMAND_ARGS, settingString);
	return true;
}

static bool Cmd_SetStringIniSetting_Execute(COMMAND_ARGS)
{
	char stringArg[kMaxMessageLength] = { 0 };

	if (ExtractFormatStringArgs(0, stringArg, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, SIZEOF_FMT_STRING_PARAMS))
	{
		char* strtokContext = NULL;
		char* settingName = strtok_s(stringArg, GetSeparatorChars(scriptObj), &strtokContext);
		char* settingVal = strtok_s(NULL, GetSeparatorChars(scriptObj), &strtokContext);
		if (settingName && settingVal)
		{
			INISettingEntry* entry = GetIniSetting(settingName);
			if (entry && (entry->data->name[0] == 'S' || entry->data->name[0] == 's'))
			{
				UInt32 valLen = strlen(settingVal) + 1;
				entry->data->s = new char[valLen];				// this leaks. INI string alloc'd where?
				strcpy_s(entry->data->s, valLen, settingVal);
			}
		}
	}

	return true;
}

typedef std::set <UInt32> ScriptSetType;

static bool Cmd_GetGameLoaded_Execute(COMMAND_ARGS)
{
	static ScriptSetType	informedScripts;

	*result = 0;

	// was a game loaded?
	if(g_gameLoaded)
	{
		// yes, clear the list of scripts we've informed and reset the 'game loaded' flag
		informedScripts.clear();

		g_gameLoaded = false;
	}

	if(scriptObj)
	{
		// have we returned 'true' to this script yet?
		if(informedScripts.find(scriptObj->refID) == informedScripts.end())
		{
			// no, return true and add to the set
			*result = 1;

			informedScripts.insert(scriptObj->refID);
		}
	}

	return true;
}

static bool Cmd_GetCellChanged_Execute(COMMAND_ARGS)
{
	static ScriptSetType	informedScripts;
	*result = 0;

	if(GetCellChanged()) {
		informedScripts.clear();
	}

	if(scriptObj) {
		if(informedScripts.find(scriptObj->refID) == informedScripts.end()) {
			*result = 1;
			informedScripts.insert(scriptObj->refID);
		}
	}

	return true;
}

static bool Cmd_GetGameRestarted_Execute(COMMAND_ARGS)
{
	static std::set<UInt32> regScripts;

	*result = 0;

	if (scriptObj && (regScripts.find(scriptObj->refID) == regScripts.end()))
	{
		*result = 1;
		regScripts.insert(scriptObj->refID);
	}

	return true;
}

struct TimeInfo
{
	UInt8	disableCount;	// 00
	UInt8	pad1[3];		// 01
	float	fpsClamp;		// 04 - in seconds
	float	unk08;			// 08
	float	frameTime;		// 0C - in seconds
	float	unk10;			// 10
	UInt32	gameStartTime;	// 14
};

TimeInfo	* g_timeInfo = (TimeInfo *)0x00B33E90;

static bool Cmd_GetFPS_Execute(COMMAND_ARGS)
{
#if 1
	float			frameTime = g_timeInfo->frameTime;
#else
	float			frameTime = GetAverageFrameTime();
#endif

	// clamp to prevent weird behavior
	const float	kFPSCap = 10000.0f;	// 10K FPS ought to be enough for anybody
	const float kMinFrameTime = 1.0f / kFPSCap;

	if(frameTime < kMinFrameTime) frameTime = kMinFrameTime;

	*result = 1.0f / frameTime;

	return true;
}

//Undocumented and deprecated?
static bool Cmd_GetCurrentFrameIndex_Execute(COMMAND_ARGS)
{
	*result = g_inputGlobal->FrameIndex;
	return true;
}

typedef void (* _ToggleGlobalCollision)(void);

UInt8							* g_isCollisionOff = (UInt8 *)0x00B33A34;
const _ToggleGlobalCollision	ToggleGlobalCollision = (_ToggleGlobalCollision)0x004447F0;

bool IsGlobalCollisionDisabled(void)
{
	return *g_isCollisionOff != 0;
}

static bool Cmd_IsGlobalCollisionDisabled_Execute(COMMAND_ARGS)
{
	*result = IsGlobalCollisionDisabled() ? 1 : 0;

	return true;
}

static bool Cmd_SetDisableGlobalCollision_Execute(COMMAND_ARGS)
{
	UInt32	disable = 0;
	UInt32	currentState = IsGlobalCollisionDisabled();

	*result = currentState;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &disable))
		return true;

	if(disable != currentState)
		ToggleGlobalCollision();

	return true;
}

static bool Cmd_GetDebugSelection_Execute(COMMAND_ARGS)
{
	*result = 0;

	InterfaceManager	* interfaceManager = InterfaceManager::GetSingleton();
	if(interfaceManager && interfaceManager->debugSelection)
	{
		UInt32	* refResult = (UInt32 *)result;

		*refResult = interfaceManager->debugSelection->refID;
	}

	return true;
}

static char MessageIconPath[512] = { 0 };
static char MessageSoundID[256] = { 0 };

static bool Cmd_MessageEX_Execute(COMMAND_ARGS)
{
	*result = 0;
	char buffer[kMaxMessageLength];
	// updated 0021: kyoma's MenuQue plugin causes UI messages to take duration into account
	// so we accept a duration now
	float duration = 2.0;
	if (ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_MessageEX.numParams, &duration))
	{
		*result = 1;
		if (*MessageIconPath || *MessageSoundID)
		{
			QueueUIMessage_2(buffer, duration, MessageIconPath, MessageSoundID);
			*MessageIconPath = 0;
			*MessageSoundID = 0;
		}
		else
			QueueUIMessage(buffer, 0, 1, duration);
	}

	return true;
}

static bool Cmd_SetMessageIcon_Execute(COMMAND_ARGS)
{
	ExtractArgs(PASS_EXTRACT_ARGS, &MessageIconPath);
	return true;
}

static bool Cmd_SetMessageSound_Execute(COMMAND_ARGS)
{
	ExtractArgs(PASS_EXTRACT_ARGS, &MessageSoundID);
	return true;
}

static bool Cmd_MessageBoxEX_Execute(COMMAND_ARGS)
{
	*result = 0;
	char buffer[kMaxMessageLength];

	if (!ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_MessageBoxEX.numParams))
		return true;

	//extract the buttons
	const char* b[10] = {0};
	UInt32 btnIdx = 0;
	UInt32 mb_length = 0;

	for (char* ch = buffer; *ch && btnIdx < 10; ch++)
	{
		if (mb_length > 0) {
			mb_length--;
			continue;		// bytes in multibyte characters are not considered as SeparatorChar
		} else if (strlen(ch) > 1 && _mbsbtype((const unsigned char*)(ch), 1) == 2) {
			mb_length = _mbclen((const unsigned char*)(ch)) - 1;		// get the length of a multibyte character from its first byte.
			continue;
		}

		if (*ch == GetSeparatorChar(scriptObj))
		{
			*ch = '\0';
			b[btnIdx++] = ch + 1;
		}

	}

	if (!btnIdx)				//supply default OK button
		b[0] = "Ok";

	if (thisObj && !(thisObj->flags & 0x00004000))		//if not temporary object and not quest script
		*ShowMessageBox_pScriptRefID = thisObj->refID;
	else
		*ShowMessageBox_pScriptRefID = scriptObj->refID;

	*ShowMessageBox_button = 0xFF;	//overwrite any previously pressed button
	ShowMessageBox(buffer, ShowMessageBox_Callback, 0, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], 0);

	return true;
}

static bool Cmd_GetCrosshairRef_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!(*g_HUDInfoMenu))
		return true;

	TESObjectREFR* xRef = ((*g_HUDInfoMenu)->crosshairRef);
	if (xRef)
		if (Oblivion_DynamicCast(xRef, 0, RTTI_TESObjectREFR, RTTI_TESObjectREFR, 0))
			*refResult = xRef->refID;

	return true;
}

static bool Cmd_IsModLoaded_Execute(COMMAND_ARGS)
{
	char modName[512];
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &modName))
		return true;

	if (ModTable::Get().IsModLoaded (modName))
		*result = 1.0;

	if (IsConsoleMode())
	{
		if (*result)
			Console_Print("Mod Loaded");
		else
			Console_Print("Mod not loaded");
	}

	return true;
}

static bool Cmd_GetModIndex_Execute(COMMAND_ARGS)
{
	char modName[512];
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &modName))
		return true;

	UInt32 modIndex = ModTable::Get().GetModIndex (modName);
	*result = modIndex;
	if (IsConsoleMode())
		Console_Print("Mod Index: %02X", modIndex);

	return true;
}

static bool Cmd_ResolveModIndex_Execute(COMMAND_ARGS)
{
	UInt32 storedModIndex = -1;
	*result = -1.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &storedModIndex)) {
		if (storedModIndex >= 0 && storedModIndex < 0xFF) {
			UInt32 fixedRefID = 0;
			if (Serialization::ResolveRefID(storedModIndex << 24, &fixedRefID)) {
				fixedRefID >>= 24;
				if (fixedRefID != 0xFF) {
					*result = fixedRefID;
				}
			}
		}
	}

	if (IsConsoleMode()) {
		Console_Print("ResolveModIndex %d >> %.0f", storedModIndex, *result);
	}

	return true;
}

static bool Cmd_GetNumLoadedMods_Execute(COMMAND_ARGS)
{
	// note this doesn't care about 'aliased' mods in ModTable, and probably shouldn't
	*result = (*g_dataHandler)->GetActiveModCount();
	return true;
}

static bool Cmd_GetSourceModIndex_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = -1;

	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form))
		return true;

	if (!form)
		form = thisObj;

	if (form)
	{
		if (form->IsCloned())
			*result = 0xFF;
		else
			*result = (UInt8)(form->refID >> 24);
	}

	return true;
}

static bool Cmd_GetGodMode_Execute(COMMAND_ARGS)
{
	*result = (IsGodMode()) ? 1 : 0;
	return true;
}

static bool WildcardCompare(const char * String, const char * WildcardComp)
{
	const char *	RollbackString = NULL;
	const char *	RollbackCompare = NULL;

	// exit if either string's pointer is NULL.
	if (!String || !WildcardComp)
		return false;

	do
	{
		if(*WildcardComp == '*')
		{
			// skip multiple *s in comparison string
			while(WildcardComp[1] == '*')
				WildcardComp++;

			// set up the rollback string pointers so they skip any wildcard characters
			RollbackCompare = WildcardComp++;
			RollbackString = String;
		}

		// looking for a single wildcard character but run out of characters in String to compare it to
		if(*WildcardComp == '?'&& !*String)
			return false;

		// the current characters don't match and are not wildcarded
		if(*WildcardComp != '?' && (tolower(*WildcardComp) != tolower(*String)))
		{
			// haven't come across any multiple match wildcards yet so the strings don't match
			if(RollbackCompare == NULL)
				return false;

			// compare string is multiply wildcarded
			// characters don't match, so rollback the two strings and move to the next character in RollbackString
			WildcardComp = RollbackCompare;
			String = RollbackString++;
		}

		// if the Wildcard string hasn't reached the end, move to the next character
		if(*WildcardComp)
			WildcardComp++;

		// move to the next character in String and loop if we aren't at the end
	}
	while(*String++);

	// remove any trailing multiple wildcards
	while(*WildcardComp == '*')
		WildcardComp++;

	// if the comparison string is at the end of it's string then the two strings match
	return !*WildcardComp;
}

static float DistanceSquared(
	float x1, float y1, float z1,
	float x2, float y2, float z2)
{
	float	dx = x1 - x2;
	float	dy = y1 - y2;
	float	dz = z1 - z2;

	return dx * dx + dy * dy + dz * dz;
}

static bool IsDeadCastingSound(TESGameSound* sound)
{
	// special-case casting sounds - they tend to persist long after they cease being audible
	// returns true if sound is a dead casting sound, false otherwise
	// ###TODO: figure out what first two bytes of TESGameSound mean to solve this more generally
	if (WildcardCompare(sound->name, "SPL*Cast")) {
		DEBUG_PRINT("Casting sound");
		return sound->unk01 != 2;
	}

	return false;
}

static bool Cmd_GetSoundPlaying_Execute(COMMAND_ARGS)
{
	char	soundName[512] = { 0 };
	float	radiusPickSize = 0;

	*result = 0;

	if(!*g_osGlobals) return true;

	OSSoundGlobals	* soundGlobals = (*g_osGlobals)->sound;
	if(!soundGlobals || !soundGlobals->gameSoundMap || !soundGlobals->niObjectMap) return true;

	if(!ExtractArgs(PASS_EXTRACT_ARGS, &soundName, &radiusPickSize)) return true;

	UInt32	matchCount = 0;

	if(!soundName[0])
	{
		// dump sound info

		_MESSAGE("TESGameSound:");
		gLog.Indent();

		for(OSSoundGlobals::TESGameSoundMap::Iterator iter(soundGlobals->gameSoundMap); !iter.Done(); iter.Next())
		{
			UInt32			key = iter.GetKey();
			TESGameSound	* data = iter.Get();

			_MESSAGE("%08X: %08X (%f %f %f) %s", key, data,
				data->x, data->y, data->z, data->name);
			Console_Print("%08X: %08X (%f %f %f) %s", key, data,
				data->x, data->y, data->z, data->name);
		}

		gLog.Outdent();

		_MESSAGE("NiAVObject:");
		gLog.Indent();

		for(OSSoundGlobals::NiAVObjectMap::Iterator iter(soundGlobals->niObjectMap); !iter.Done(); iter.Next())
		{
			UInt32			key = iter.GetKey();
			NiAVObject		* data = iter.Get();

			_MESSAGE("%08X: %08X (%s)", key, data, data->m_pcName);
			Console_Print("%08X: %08X (%s)", key, data, data->m_pcName);
		}

		gLog.Outdent();
	}
	else
	{
		NiNode	* targetNiNode = NULL;

		if(thisObj)
		{
			if(thisObj == *g_thePlayer)
				targetNiNode = thisObj->niNode;	// special-casing the player as it returns a different node in first-person view
			else
				targetNiNode = thisObj->GetNiNode();
		}

		// searching based on a NiNode?
		if(targetNiNode)
		{
			// iterate through NiNode map, find matches and corresponding hash key (may occur multiple times)
			for(OSSoundGlobals::NiAVObjectMap::Iterator iter(soundGlobals->niObjectMap); !iter.Done(); iter.Next())
			{
				if(iter.Get() == targetNiNode)
				{
					TESGameSound	* gameSound = soundGlobals->gameSoundMap->Lookup(iter.GetKey());
					if(gameSound)
					{
						if(WildcardCompare(gameSound->name, soundName))
						{
							// special-case casting sounds
							if (!IsDeadCastingSound(gameSound)) {
								matchCount++;
							}
						}
					}
				}
			}
		}

		// searching based on proximity?
		if((radiusPickSize > 0) && thisObj)
		{
			float	pickSizeSquared = radiusPickSize * radiusPickSize;

			for(OSSoundGlobals::TESGameSoundMap::Iterator iter(soundGlobals->gameSoundMap); !iter.Done(); iter.Next())
			{
				TESGameSound	* gameSound = iter.Get();

				// radius matched?
				if(DistanceSquared(
					thisObj->posX, thisObj->posY, thisObj->posZ,
					gameSound->x, gameSound->y, gameSound->z) < pickSizeSquared)
				{
					if(WildcardCompare(gameSound->name, soundName))
					{
						if (!IsDeadCastingSound(gameSound)) {
							matchCount++;
						}
					}
				}
			}
		}

		// searching based on name alone?
		if(!thisObj)
		{
			for(OSSoundGlobals::TESGameSoundMap::Iterator iter(soundGlobals->gameSoundMap); !iter.Done(); iter.Next())
			{
				TESGameSound	* gameSound = iter.Get();

				if(WildcardCompare(gameSound->name, soundName))
				{
					if (!IsDeadCastingSound(gameSound)) {
						matchCount++;
					}
				}
			}
		}

#ifdef _DEBUG
		Console_Print("matchCount = %d", matchCount);
#endif
	}

	*result = matchCount;

	return true;
}

static bool Cmd_GetLastEnchantedItem_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (g_LastEnchantedItem)
		*refResult = g_LastEnchantedItem->refID;

	return true;
}

static bool Cmd_GetLastCreatedSpell_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (g_LastCreatedSpell)
		*refResult = g_LastCreatedSpell->refID;

	return true;
}

static bool Cmd_GetLastCreatedPotion_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (g_LastCreatedPotion)
		*refResult = g_LastCreatedPotion->refID;

	return true;
}

static bool Cmd_GetLastUniqueCreatedPotion_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (g_LastUniqueCreatedPotion)
		*refResult = g_LastUniqueCreatedPotion->refID;

	return true;
}

static bool Cmd_IsConsoleOpen_Execute(COMMAND_ARGS)
{
	*result = (IsConsoleOpen()) ? 1 : 0;
	return true;
}

static bool Cmd_LoadGameEx_Execute(COMMAND_ARGS)
{
	char filename[kMaxMessageLength] = { 0 };
	*result = 0;

	if (ExtractFormatStringArgs(0, filename, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_LoadGameEx.numParams))
	{
		TESSaveLoadGame	* game = *g_createdBaseObjList;
		if (game)
		{
			if (game->LoadGame(filename))
				*result = 1;
			else
				Console_Print("Error: failed to load game %s", filename);
		}
	}

	return true;
}

static bool Cmd_GetGameDifficulty_Execute(COMMAND_ARGS)
{
	*result = (*g_thePlayer)->gameDifficultyLevel;
	return true;
}

static bool Cmd_SetGameDifficulty_Execute(COMMAND_ARGS)
{
	float newDiff = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &newDiff) && newDiff >= -1 && newDiff <= 1)
		(*g_thePlayer)->gameDifficultyLevel = newDiff;

	return true;
}

static bool Cmd_GetWaterShader_Execute(COMMAND_ARGS)
{
	*result = -1.0;	// all properties must be positive so this indicates failure

	char propName[0x200] = { 0 };
	if (ExtractArgs(PASS_EXTRACT_ARGS, propName) && propName)
	{
		float propVal;
		if (GetWaterShaderProperty(propName, propVal))
			*result = propVal;
	}

	return true;
}

static UInt32* kuGridsToLoadAddr = (UInt32*)0x00B06A2C;
static Cmd_Execute Cmd_OutputLocalMapPictures_Execute = (Cmd_Execute)0x0050E6A0;
static const UInt32 kOLMPPatchAddr = 0x0050E763;	// call GridCellArray::GetGridEntry(x, y)
static const UInt32 kOLMPRetnAddr = 0x0050E768;
static const UInt32 kGridCellArray_GetGridEntry = 0x00482150;

static UInt32 uGridsToLoad_Override = -1;

static __declspec(naked) void GridCellArray_GetGridEntry_Hook(void)
{
	static UInt32 x;
	static UInt32 y;
	static GridCellArray* _this = NULL;
	static GridCellArray::GridEntry* retnValue = NULL;

	__asm
	{
		mov _this,	ecx					// grab this
		mov ecx,	[esp]				// grab args
		mov x,		ecx
		mov ecx,	[esp+4]
		mov y,		ecx

		pushad

		mov	eax,	[kuGridsToLoadAddr]
		mov	eax,	[eax]				// this will be the *modified* value if overridden by SetOLMPGrids

		mov ecx,	[_this]
		mov edx,	[ecx+0x0C]			// GridCellArray::size
		sub	edx,	eax					// difference between uGridsToLoad and override
		shr edx,	1					// divide by two to get offset

		// add offset to x and y
		mov	eax,	[x]
		add eax,	edx
		mov ebx,	[y]
		add ebx,	edx

		// look up grid cell entry
		mov	edx,	[ecx+0x0C]			// GridCellArray::size
		imul edx,	eax					// multiply by x
		add	edx,	ebx					// add y
		mov	ecx,	[ecx+0x10]			// GridCellArray::grid
		lea eax,	[ecx+edx*8]			// GridEntry*
		mov	[retnValue],	eax

		popad

		// pop the args
		pop	eax
		pop eax

		// return value
		mov eax,	[retnValue]
		jmp	[kOLMPRetnAddr]
	}
}

static bool Cmd_SetOLMPGrids_Execute(COMMAND_ARGS)
{
	SInt32 grids = 0;
	*result = 0;

	// uGrids override must be <= original value and odd
	if (ExtractArgs(PASS_EXTRACT_ARGS, &grids) && grids > 0 && grids <= *kuGridsToLoadAddr && (grids & 1))
	{
		uGridsToLoad_Override = grids;
		*result = 1;
	}

	return true;
}

static bool Cmd_OLMPOR_Execute(COMMAND_ARGS)
{
	UInt32 oldGrids = *kuGridsToLoadAddr;

	bool bDoHook = (uGridsToLoad_Override != -1);
	if (bDoHook)
	{
		// temporarily modify uGridToLoad
		*kuGridsToLoadAddr = uGridsToLoad_Override;

		// install hook
		WriteRelJump(kOLMPPatchAddr, (UInt32)&GridCellArray_GetGridEntry_Hook);
	}

	Cmd_OutputLocalMapPictures_Execute(PASS_COMMAND_ARGS);

	if (bDoHook)
	{
		// restore original uGridsToLoad
		*kuGridsToLoadAddr = oldGrids;

		// uninstall hook
		WriteRelCall(kOLMPPatchAddr, kGridCellArray_GetGridEntry);
	}

	return true;
}

static bool Cmd_GetGridsToLoad_Execute(COMMAND_ARGS)
{
	*result = (double)(*kuGridsToLoadAddr);
	return true;
}

static bool Cmd_GlobalVariableExists_Execute(COMMAND_ARGS)
{
	char varName[0x200] = { 0 };
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &varName) && varName)
		*result = (*g_dataHandler)->GetGlobalVarByName(varName, strlen(varName)) ? 1 : 0;

	return true;
}

static bool Cmd_SetModAlias_Execute (COMMAND_ARGS)
{
	char name[0x100] = "", alias[0x100] = "";

	if (ExtractArgs(PASS_EXTRACT_ARGS, name, alias) && *name && *alias)
		*result = ModTable::Get().SetAlias (name, ModTable::Get().GetModIndex (alias)) ? 1.0 : 0.0;

	DEBUG_PRINT ("SetModAlias >> %.0f", *result);
	return true;
}

static bool Cmd_GetModAlias_Execute (COMMAND_ARGS)
{
	char name[0x100] = "";
	std::string alias;

	if (ExtractArgs(PASS_EXTRACT_ARGS, name) && *name)
		alias = ModTable::Get().GetAlias (name);

	AssignToStringVar (PASS_COMMAND_ARGS, alias.c_str ());

	DEBUG_PRINT ("GetModAlias >> %s", alias.c_str ());
	return true;
}
static bool Cmd_SetCameraFOV2_Execute(COMMAND_ARGS) {
	SInt32 FOV = 0;
	float FOVA = 0.0f;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &FOV)) {
		if (FOV < 0) FOVA = (1.0f / (-FOV));
		else FOVA = FOV;
		(*g_worldSceneGraph)->SetCameraFOV(FOVA, 0.0f);
		(*g_worldSceneGraph)->UpdateParticleFOV(FOV);
	}
	return true;
}

void GetLoadedType(UInt8 formType, int index, ArrayID arr){
    if(formType <= kFormType_TOFT){
        UInt32 idx = 0;
        NiTPointerMap<TESForm>::Iterator iter(g_formTable);
        for (NiTPointerMap<TESForm>::Entry* entry = iter.Current(); entry != NULL ; entry = iter.Next()) {
			TESForm* form = entry->data;
            if(form->typeID == formType && (index == -1 || (UInt8)index == (form->refID >> 24))){
                g_ArrayMap.SetElementFormID(arr, idx, form->refID);
                idx++;
            }
        }
        if(idx  == 0) _MESSAGE("No form found. It's possible that this type isn't available with the Form map, and must be taken directly from the data handler. Open an issue on github");
    }
    else{
        _MESSAGE("Invalid form type (If this is an error and the form is indeed valid, open an issue on github)");
    }
}

bool Cmd_GetLoadedTypeArray_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 formType;
	int index = -1;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &formType, &index)){
        ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
		GetLoadedType(formType, index, arr);
        *result = arr;
	}
	return true;
}
#endif

static ParamInfo kParams_SetNumericGameSetting[] =
{
	{	"string",	kParamType_String,	0 },
	{	"float",	kParamType_Float,	0 },
};

CommandInfo kCommandInfo_SetNumericGameSetting =
{
	"SetNumericGameSetting",
	"setngs",
	0,
	"Set a game setting from a variable",
	0,
	2,
	kParams_SetNumericGameSetting,
	HANDLER(Cmd_SetNumericGameSetting_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetNumericINISetting =
{
	"SetNumericINISetting",
	"setnis",
	0,
	"Set an INI setting from a variable",
	0,
	2,
	kParams_SetNumericGameSetting,
	HANDLER(Cmd_SetNumericINISetting_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumericINISetting =
{
	"GetNumericINISetting",
	"getnis",
	0,
	"Get an INI setting",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_GetNumericINISetting_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetGameLoaded =
{
	"GetGameLoaded",
	"",
	0,
	"Returns true the first time the command is called (per script) after a game is loaded",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetGameLoaded_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetGameRestarted =
{
	"GetGameRestarted",
	"OnRestart",
	0,
	"returns true once each time game is restarted",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetGameRestarted_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetFPS =
{
	"GetFPS",
	"",
	0,
	"Returns the current rendering FPS",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetFPS_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsGlobalCollisionDisabled =
{
	"IsGlobalCollisionDisabled",
	"",
	0,
	"Returns whether or not global collision is disabled (via the \'tcl\' console command)",
	0,
	0,
	NULL,
	HANDLER(Cmd_IsGlobalCollisionDisabled_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetDisableGlobalCollision =
{
	"SetDisableGlobalCollision",
	"",
	0,
	"Enables or disables global collision, returns the previous state",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetDisableGlobalCollision_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetDebugSelection =
{
	"GetDebugSelection",
	"",
	0,
	"returns the current selected object in the console",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetDebugSelection_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_Message[21] =
{
	{"format string",	kParamType_String, 0},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
};

static ParamInfo kParams_MessageEx[22] =
{
	{"format string",	kParamType_String, 0},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"variable",		kParamType_Float, 1},
	{"duration",		kParamType_Float, 1}
};

CommandInfo kCommandInfo_MessageEX =
{
	"MessageEX", "MsgEX",
	0,
	"Prints a formatted message",
	0, 22, kParams_MessageEx,
	HANDLER(Cmd_MessageEX_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MessageBoxEX =
{
	"MessageBoxEX", "MBoxEX",
	0,
	"Displays a formatted messagebox",
	0, 21, kParams_Message,
	HANDLER(Cmd_MessageBoxEX_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetCrosshairRef =
{
	"GetCrosshairRef", "GetXRef",
	0,
	"returns the reference currently under the crosshair",
	0, 0, NULL,
	HANDLER(Cmd_GetCrosshairRef_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsModLoaded =
{
	"IsModLoaded", "",
	0,
	"returns 1 if the specified mod is currently loaded",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_IsModLoaded_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetModIndex =
{
	"GetModIndex", "",
	0,
	"returns the index of the specified mod",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_GetModIndex_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetSourceModIndex =
{
	"GetSourceModIndex", "",
	0,
	"returns the mod index of the mod from which the object originates",
	0,
	1,
	kParams_OneOptionalInventoryObject,
	HANDLER(Cmd_GetSourceModIndex_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumLoadedMods =
{
	"GetNumLoadedMods",
	"GetNumMods",
	0,
	"",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetNumLoadedMods_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMessageIcon =
{
	"SetMessageIcon",
	"",
	0,
	"sets the .dds path of the icon to be used by the next MessageEX call",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_SetMessageIcon_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMessageSound =
{
	"SetMessageSound",
	"",
	0,
	"sets the sound ID of the sound to be played by the next MessageEX call",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_SetMessageSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetGodMode =
{
	"GetGodMode",
	"",
	0,
	"returns true if god mode is enabled",
	0,
	0,
	NULL,
	HANDLER(Cmd_GetGodMode_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetSoundPlaying[] =
{
	{	"string",	kParamType_String,	1 },	// sound name (empty to dump sound data to obse.log)
	{	"float",	kParamType_Float,	1 },	// distance pick radius (zero to only receive exact matches)
};

DEFINE_COMMAND(GetSoundPlaying, Returns the number of times the specified sound is playing, 0, 2, kParams_GetSoundPlaying)

DEFINE_COMMAND(GetCurrentFrameIndex,
			   returns current frame index used for calculating FPS,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetLastEnchantedItem,
			   returns the last item enchanted by the player,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetLastCreatedSpell,
			   returns the last spell created by the player,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetLastCreatedPotion,
			   returns the last potion created by the player,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetLastUniqueCreatedPotion,
			   returns the last unique potion created by the player,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(IsConsoleOpen,
			   returns 1 if the console window is open,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(LoadGameEx,
			   loads a saved game,
			   0,
			   SIZEOF_FMT_STRING_PARAMS,
			   kParams_FormatString);

DEFINE_COMMAND(GetStringIniSetting,
			   returns a string ini setting,
			   0,
			   SIZEOF_FMT_STRING_PARAMS,
			   kParams_FormatString);

DEFINE_COMMAND(SetStringIniSetting,
			   sets a string ini setting,
			   0,
			   SIZEOF_FMT_STRING_PARAMS,
			   kParams_FormatString);

DEFINE_COMMAND(GetGameDifficulty, returns the difficulty level of the game, 0, 0, NULL);
DEFINE_COMMAND(SetGameDifficulty, sets the difficulty level of the game from -1 to 1, 0, 1, kParams_OneFloat);

DEFINE_COMMAND(GetWaterShader, returns the value of a water shader property, 0, 1, kParams_OneString);

CommandInfo kCommandInfo_SetOLMPGrids =
{
	"SetOutputLocalMapPicturesGrids",
	"SetOLMPGrids",
	0,
	"sets the value with which to override uGridsToLoad when generating local maps with OLMPOR",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetOLMPGrids_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OLMPOR =
{
	"OutputLocalMapPicturesOverride",
	"OLMPOR",
	0,
	"identical to the OutputLocalMapPictures console command, but overrides the uGridsToLoad ini setting",
	0,
	0,
	NULL,
	HANDLER(Cmd_OLMPOR_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetGridsToLoad, returns the effective value of the uGridsToLoad ini setting, 0, 0, NULL);
DEFINE_CMD_ALT(GlobalVariableExists, GlobalExists, returns 1 if a global var exists with the specified editorID, 0, kParams_OneString);
DEFINE_COMMAND(GetCellChanged, returns 1 for one frame per calling script when the player enters a new interior or exterior cell, 0, 0, NULL);

DEFINE_COMMAND(ResolveModIndex,
			   "given a mod index referring to a mod stored in the current savegame, returns that mod's current mod index",
			   0, 1, kParams_OneInt);

DEFINE_COMMAND (SetModAlias, sets the alias for a mod name, 0, 2, kParams_TwoStrings);
DEFINE_COMMAND (GetModAlias, retrieves the alias for a mod name, 0, 1, kParams_OneString);

CommandInfo kCommandInfo_SetCameraFOV2 =
{
	"SetCameraFOV2",
	"SetFOV2",
	0,
	"Set the FOV for the camera. It's similar of con_etCameraFOV but it's unbounded",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_SetCameraFOV2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_CMD_ALT(GetLoadedTypeArray, GLTA, "Return an array with all Loaded form of type and optionally with specific mod index" , 0, kParams_OneInt_OneOptionalInt);
