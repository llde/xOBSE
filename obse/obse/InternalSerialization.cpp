#include <map>
#include "InternalSerialization.h"
#include "StringVar.h"
#include "ArrayVar.h"
#include "GameData.h"
#include "common/IFileStream.h"
#include <algorithm>
#include <string>
#include "Script.h"
#include "Hooks_Gameplay.h"

void SaveModList(OBSESerializationInterface* obse)
{
	DataHandler* dhand = *g_dataHandler;
	UInt8 modCount = dhand->numLoadedMods;

	obse->OpenRecord('MODS', 0);
	obse->WriteRecordData(&modCount, sizeof(modCount));
	for (UInt32 i = 0; i < modCount; i++)
	{
		UInt16 nameLen = strlen(dhand->modsByID[i]->name);
		obse->WriteRecordData(&nameLen, sizeof(nameLen));
		obse->WriteRecordData(dhand->modsByID[i]->name, nameLen);
	}
}

static UInt8	s_preloadModRefIDs[0xFF];
static UInt8	s_numPreloadMods = 0;

bool ReadModListFromCoSave(OBSESerializationInterface * intfc)
{
	_MESSAGE("Reading mod list from co-save");

	char name[0x104] = { 0 };
	UInt16 nameLen = 0;

	intfc->ReadRecordData(&s_numPreloadMods, sizeof(s_numPreloadMods));
	for (UInt32 i = 0; i < s_numPreloadMods; i++) {
		intfc->ReadRecordData(&nameLen, sizeof(nameLen));
		intfc->ReadRecordData(&name, nameLen);
		name[nameLen] = 0;

		s_preloadModRefIDs[i] = (*g_dataHandler)->GetModIndex(name);
	}
	return true;
}

bool ReadModListFromSaveGame(const char* path)
{
	_MESSAGE("Reading mod list from savegame");

	IFileStream savefile;
	if (!savefile.Open(path)) {
		_MESSAGE("Couldn't open .ess file when attempting to read plugin list");
		return false;
	}
	else {
		static const UInt32 kSaveHeaderSizeOffset = 34;
		savefile.SetOffset(kSaveHeaderSizeOffset);
		UInt32 headerSize = savefile.Read32();
		savefile.SetOffset(headerSize + kSaveHeaderSizeOffset + sizeof(UInt32));

		s_numPreloadMods = savefile.Read8();
		char pluginName[0x100];
		for (UInt32 i = 0; i < s_numPreloadMods; i++) {
			UInt8 nameLen = savefile.Read8();
			savefile.ReadBuf(pluginName, nameLen);
			pluginName[nameLen] = 0;
			_MESSAGE("Save file contains plugin %s", pluginName);
			s_preloadModRefIDs[i] = (*g_dataHandler)->GetModIndex(pluginName);
		}

		savefile.Close();
	}

	return true;
}

UInt8 ResolveModIndexForPreload(UInt8 modIndexIn)
{
	return (modIndexIn < s_numPreloadMods) ? s_preloadModRefIDs[modIndexIn] : 0xFF;
}

// values of obse global data which can be persisted in the co-savefc0d5400  
enum {
	kOBSEGlobal_MovementSpeedModifier,
	kOBSEGlobal_SpellEffectivenessModifier,
	// anything else?

	kOBSEGlobal_MAX
};

// when the game is saved, save any persistent global data
void SaveGlobals (OBSESerializationInterface* obse)
{
	UInt8 globId;
	obse->OpenRecord('GLOB', 0);
	double mvmtSpeedMod = GetPersistentPlayerMovementSpeedModifier ();
	if (0.0 != mvmtSpeedMod)
	{
		globId = kOBSEGlobal_MovementSpeedModifier;
		obse->WriteRecordData (&globId, sizeof (globId));
		obse->WriteRecordData (&mvmtSpeedMod, sizeof(mvmtSpeedMod));
	}

	double effMod = GetPersistentPlayerSpellEffectivenessModifier ();
	if (0.0 != effMod)
	{
		globId = kOBSEGlobal_SpellEffectivenessModifier;
		obse->WriteRecordData (&globId, sizeof (globId));
		obse->WriteRecordData (&effMod, sizeof (effMod));
	}
}

// after a game is loaded, read and apply any persistent global data
void ReadGlobals (OBSESerializationInterface* obse, UInt32 dataLen)
{
	_MESSAGE ("Reading globals\n");
	while (dataLen > 0) {
		UInt8 globId;
		obse->ReadRecordData (&globId, sizeof (globId));
		dataLen -= sizeof (globId);

		switch (globId) {
			case kOBSEGlobal_MovementSpeedModifier:
				{
					double mod;
					obse->ReadRecordData (&mod, sizeof (mod));
					dataLen -= sizeof (mod);
					ModPlayerMovementSpeed (mod, true);
				}
				break;
			case kOBSEGlobal_SpellEffectivenessModifier:
				{
					double mod;
					obse->ReadRecordData (&mod, sizeof (mod));
					dataLen -= sizeof (mod);
					ModPlayerSpellEffectiveness (mod, true);
				}
				break;
			default:
				_MESSAGE ("ReadGlobals >> Unexpected global ID %d\n", globId);
				break;
		}
	}
}

// before a game is loaded, reset any persistent global data
void ResetGlobals ()
{
	double mod = GetPersistentPlayerMovementSpeedModifier ();
	if (0.0 != mod)
	{
		// only apply if non-zero, to avoid needlessly enabling the hook
		ModPlayerMovementSpeed (-1.0 * mod, true);
	}

	mod = GetPersistentPlayerSpellEffectivenessModifier ();
	if (0.0 != mod)
		ModPlayerSpellEffectiveness (-1.0 * mod, true);
}

/*******************************
*	Callbacks
*******************************/
void Core_SaveCallback(void * reserved)
{
	SaveModList(&g_OBSESerializationInterface);
	g_StringMap.Save(&g_OBSESerializationInterface);
	g_ArrayMap.Save(&g_OBSESerializationInterface);
	SaveGlobals (&g_OBSESerializationInterface);
}

void Core_LoadCallback(void * reserved)
{
	ResetGlobals ();

	OBSESerializationInterface* intfc = &g_OBSESerializationInterface;
	UInt32 type, version, length;

	while (intfc->GetNextRecordInfo(&type, &version, &length))
	{
		switch (type)
		{
		case 'STVS':
		case 'STVR':
		case 'STVE':
		case 'ARVS':
		case 'ARVR':
		case 'ARVE':
		case 'MODS':	
			break;		// processed during preload
		case 'GLOB':
			ReadGlobals (intfc, length);
			break;
		default:
			_MESSAGE("Unhandled chunk type in LoadCallback: %d", type);
			continue;
		}
	}
}

void Core_NewGameCallback(void * reserved)
{
	ResetGlobals ();
	g_ArrayMap.Clean();
	g_StringMap.Clean();

	// below are commented out because it is possible for quest scripts to create string/array vars during
	// main menu - in which case they would get obliterated when a new game is started.

	//g_StringMap.Reset(&g_OBSESerializationInterface);
	//g_ArrayMap.Reset(&g_OBSESerializationInterface);
}

void Core_PostLoadCallback(bool bLoadSucceeded)
{
	g_ArrayMap.PostLoad(bLoadSucceeded);
	g_StringMap.PostLoad(bLoadSucceeded);
}

void Core_PreloadCallback(void * reserved)
{
	// this is invoked only if at least one other plugin registers a preload callback
	
	// reset refID fixup table. if save made prior to 0019, this will remain empty
	s_numPreloadMods = 0;	// no need to zero out table - unloaded mods will be set to 0xFF below

	OBSESerializationInterface* intfc = &g_OBSESerializationInterface;

	g_ArrayMap.Preload();
	g_StringMap.Preload();

	UInt32 type, version, length;

	while (intfc->GetNextRecordInfo(&type, &version, &length)) {
		switch (type) {
			case 'MODS':
				// as of 0019 mod list stored in co-save
				ReadModListFromCoSave(intfc);
				break;
			case 'STVS':
				if (!s_numPreloadMods) {
					// pre-0019 co-save doesn't contain mod list, read from .ess instead
					ReadModListFromSaveGame((const char*)reserved);
				}

				g_StringMap.Load(intfc);
				break;
			case 'ARVS':
				if (!s_numPreloadMods) {
					// pre-0019 co-save doesn't contain mod list, read from .ess instead
					ReadModListFromSaveGame((const char*)reserved);
				}

				g_ArrayMap.Load(intfc);
				break;
			default:
				break;
		}
	}
}


void Init_CoreSerialization_Callbacks()
{
	Serialization::InternalSetSaveCallback(0, Core_SaveCallback);
	Serialization::InternalSetLoadCallback(0, Core_LoadCallback);
	Serialization::InternalSetNewGameCallback(0, Core_NewGameCallback);
	Serialization::InternalSetPreloadCallback(0, Core_PreloadCallback);
}
