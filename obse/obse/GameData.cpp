#include "GameData.h"

#if OBLIVION
#include "GameAPI.h"
#else
#include "obse_editor\EditorAPI.h"
#endif

class LoadedModFinder
{
	const char * m_stringToFind;

public:
	LoadedModFinder(const char * str) : m_stringToFind(str) { }

	bool Accept(ModEntry::Data* data)
	{
		return _stricmp(data->name, m_stringToFind) == 0;
	}
};

const ModEntry * DataHandler::LookupModByName(const char * modName)
{
	return ModEntryVisitor(&modList).Find(LoadedModFinder(modName));
}

const ModEntry ** DataHandler::GetActiveModList()
{
	static const ModEntry* activeModList[0x100] = { 0 };

	if (!(*activeModList))
	{
		UInt8 index = 0;
		for (ModEntry* entry = &(*g_dataHandler)->modList; entry; entry = entry->next)
		{
			if (entry->IsLoaded())
				activeModList[index++] = entry;
		}
	}

	return activeModList;
}

UInt8 DataHandler::GetModIndex(const char* modName)
{
	UInt8 modIndex = 0xFF;
	const ModEntry** activeModList = GetActiveModList();

	for (UInt8 idx = 0; idx < 0x100 && activeModList[idx] && modIndex == 0xFF; idx++)
		if (!_stricmp(activeModList[idx]->data->name, modName))
			modIndex = idx;

	return modIndex;
}

UInt8 DataHandler::GetActiveModCount()
{
	UInt8 count = 0;
	const ModEntry** activeModList = GetActiveModList();

	while (activeModList[count])
		count++;

	return count;
}

const char* DataHandler::GetNthModName(UInt32 modIndex)
{
	const ModEntry** activeModList = GetActiveModList();
	if (modIndex < GetActiveModCount() && activeModList[modIndex]->data)
		return activeModList[modIndex]->data->name;
	else
		return "";
}

TESGlobal* DataHandler::GetGlobalVarByName(const char* varName, UInt32 nameLen)
{
	if (nameLen == -1)
		nameLen = strlen(varName);

	for (tList<TESGlobal>::Iterator Itr = globals.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TESGlobal* item = Itr.Get();

		if (item->name.m_dataLen == nameLen && !_stricmp(item->name.m_data, varName))
			return item;
	}

	return NULL;
}

TESQuest* DataHandler::GetQuestByEditorName(const char* questName, UInt32 nameLen)
{
	if (nameLen == -1)
		nameLen = strlen(questName);

	for (tList<TESQuest>::Iterator Itr = quests.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TESQuest* quest = Itr.Get();
		if (quest->editorName.m_dataLen == nameLen && !_stricmp(quest->editorName.m_data, questName))
			return quest;
	}
	return NULL;
}

// runtime-only stuff
#if OBLIVION

bool DataHandler::ConstructObject(ModEntry::Data* tesFile, bool unk1)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x0044DCF0, this, tesFile, unk1) ? true : false;
#else
#error unsupported oblivion version
#endif
}

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

FileFinder** g_FileFinder = (FileFinder**)0xAEBE0C;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2

FileFinder** g_FileFinder = (FileFinder**)0xB33A04;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

FileFinder** g_FileFinder = (FileFinder**)0xB33A04;
TimeGlobals* g_TimeGlobals = (TimeGlobals*)0x00B332E0;
UInt32* s_iHoursToRespawnCell = (UInt32*)0x00B35C1C;
UInt16* s_firstDayOfMonths = (UInt16*)0x00B06728;
UInt16* s_numDaysPerMonths = (UInt16*)0x00B06710;
#else

#error unsupported Oblivion version

#endif

TimeGlobals* TimeGlobals::Singleton()
{
	return g_TimeGlobals;
}

UInt32 TimeGlobals::GameHoursPassed()
{
	return GameDaysPassed() * 24 + GameHour();
}

UInt32 TimeGlobals::GameDay()
{
	return Singleton()->gameDay->data;
}

UInt32 TimeGlobals::GameYear()
{
	return Singleton()->gameYear->data;
}

UInt32 TimeGlobals::GameMonth()
{
	return Singleton()->gameMonth->data;
}

float TimeGlobals::GameHour()
{
	return Singleton()->gameHour->data;
}

UInt32 TimeGlobals::GameDaysPassed()
{
	return Singleton()->gameDaysPassed->data;
}

float TimeGlobals::TimeScale()
{
	return Singleton()->timeScale->data;
}

UInt32 TimeGlobals::HoursToRespawnCell()
{
	return *s_iHoursToRespawnCell;
}

UInt16 TimeGlobals::GetFirstDayOfMonth(UInt32 monthID)
{
	return (monthID - 1 < 12) ? s_firstDayOfMonths[monthID - 1] : -1;
}

UInt16 TimeGlobals::GetNumDaysInMonth(UInt32 monthID)
{
	return (monthID - 1 < 12) ? s_numDaysPerMonths[monthID - 1] : -1;
}

// Water Shader stuff
struct WaterShaderProperty	{
	const char*	name;
	UInt32		addr;
	bool		bIsPercentage;	// opacity and blend get multiplied by 100 for return value
};

static const UInt32 kNumWaterShaderProperties = 18;

WaterShaderProperty s_WaterShaderProperties[kNumWaterShaderProperties] =
{
	{	"direction",			0x00B45FC0,  false  },
	{	"velocity",				0x00B45FC4,  false  },

	{	"frequency",			0x00B45FD4,  false  },
	{	"amplitude",			0x00B45FD8,  false  },

	{	"fresnel",				0x00B45DC4,  false  },

	{	"reflectivity",			0x00B45E48,  false  },
	{	"opacity",				0x00B45E4C,  true   },
	{	"blend",				0x00B45E50,  true   },
	{	"scrollx",				0x00B45E54,  false  },
	{	"scrolly",				0x00B45E58,  false  },

	{	"rainforce",			0x00B45F58,  false  },
	{	"rainvelocity",			0x00B45F5C,  false  },
	{	"rainfalloff",			0x00B45F60,  false  },
	{	"rainsize",				0x00B45F64,  false  },
	{	"displaceforce",		0x00B45F68,  false  },
	{	"displacevelocity",		0x00B45F6C,  false  },
	{	"displacefalloff",		0x00B45F70,  false  },

	{	"displacedampener",		0x00B45F40,  false  },
};

#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
TES** g_TES = (TES**)0x00B333A0;
#else
#error unsupported oblivion version
#endif

GridCellArray::GridEntry* GridCellArray::GetGridEntry(UInt32 x, UInt32 y)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return (GridEntry*)ThisStdCall(0x00482150, this, x, y);
#else
#error unsupported oblivion version
#endif
}

TES* TES::GetSingleton()
{
	return *g_TES;
}

bool TES::GetTerrainHeight(float* posVec3, float* outHeight)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	return ThisStdCall(0x00440590, this, posVec3, outHeight) ? true : false;
#else
#error unsupported Oblivion version
#endif
}

bool GetWaterShaderProperty(const char* propName, float& out)
{
	bool bFound = false;

	if (propName)
	{
		for (UInt32 i = 0; i < kNumWaterShaderProperties; i++)
		{
			if (!_stricmp(propName, s_WaterShaderProperties[i].name))
			{
				bFound = true;
				out = *(float*)(s_WaterShaderProperties[i].addr);
				if (s_WaterShaderProperties[i].bIsPercentage)
					out *= 100;

				break;
			}
		}
	}

	return bFound;
}

#endif