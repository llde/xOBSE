#pragma once

#if !OBLIVION
#error GameAPI.h included in editor project
#endif

#define EDITOR_SPECIFIC(X)
#define RUNTIME_SPECIFIC(X) X

#include "GameRTTI.h"
#include "Utilities.h"
#include "NiTypes.h"

class TESForm;
class TESObjectREFR;
class Script;
struct ParamInfo;
class DataHandler;
class MemoryHeap;
class Tile;
class SceneGraph;
class NiNode;
class Tile;
class Menu;
class FormatStringArgs;
struct ScriptLineBuffer;
struct BaseExtraList;
class NiTexturingProperty;
class NiD3DShaderConstantMapEntry;

const UInt32	kMaxSavedIPStack = 20;	// twice the supposed limit
extern bool g_insideUserDefinedFunction;

struct SavedIPInfo
{
	UInt32	ip;
	UInt32	stackDepth;
	UInt32	stack[kMaxSavedIPStack];
};

struct ScriptExecutionState
{
	UInt8	pad[0x20];
	UInt32	stackDepth;
	UInt32	stack[1];
};

// only records individual objects if there's a block that matches it
// ### how can it tell?
struct ScriptEventList
{
	// OnActivate not handled
	// OnAlarm, OnAlarmVictim are weird (multiple possible flags)
	enum
	{
		kEvent_OnAdd			= 0x00000001,
		kEvent_OnEquip			= 0x00000002,
		kEvent_OnActorEquip		= kEvent_OnEquip,	// presumably the game checks the type of the object
		kEvent_OnDrop			= 0x00000004,
		kEvent_OnUnequip		= 0x00000008,
		kEvent_OnActorUnequip	= kEvent_OnUnequip,
		kEvent_OnDeath			= 0x00000010,
		kEvent_OnMurder			= 0x00000020,
		kEvent_OnKnockout		= 0x00000040,
		kEvent_OnHit			= 0x00000080,
		kEvent_OnHitWith		= 0x00000100,		// TESObjectWEAP*
		kEvent_OnPackageStart	= 0x00000200,
		kEvent_OnPackageDone	= 0x00000400,
		kEvent_OnPackageChange	= 0x00000800,
		kEvent_OnLoad			= 0x00001000,
		kEvent_OnMagicEffectHit = 0x00002000,		// EffectSetting*
		kEvent_OnSell			= 0x00004000,
		kEvent_OnStartCombat	= 0x00008000,

		kEvent_OnAlarm_Steal	= 0x00010000,		// crime type 0
		kEvent_OnAlarm_Pickpocket=0x00020000,		// crime type 1
		kEvent_OnAlarm_Trespass = 0x00040000,		// crime type 2
		kEvent_OnAlarm_Attack	= 0x00080000,		// crime type 3
		kEvent_OnAlarm_Murder	= 0x00100000,		// crime type 4

		kEvent_OnTrigger		= 0x10000000,
		kEvent_OnTriggerActor	= 0x20000000,
		kEvent_OnTriggerMob		= 0x40000000,
		kEvent_OnReset			= 0x80000000
	};

	struct Event
	{
		TESForm	* object;
		UInt32	eventMask;
	};

	struct EventEntry
	{
		Event		* event;
		EventEntry	* next;
	};

	struct VarEntry;

	struct Var
	{
		UInt32		id;
		VarEntry	* nextEntry;
		double		data;
	};

	struct VarEntry
	{
		Var			* var;
		VarEntry	* next;
	};

	struct ScriptEffectInfo
	{
		bool	bRunEffectStartBlock;
		bool	bRunEffectFinishBlock;
		UInt8	unk02[2];					// not seen accessed, probably pad
		float	elapsedSeconds;

		// may be more, some simple linked list
	};

	Script		* m_script;						// 00
	UInt32		m_unk1;							// 04
	EventEntry	* m_eventList;					// 08
	VarEntry	* m_vars;						// 0C
	ScriptEffectInfo	* m_scriptEffectInfo;	// 10

	void	Dump(void);
	Var *	GetVariable(UInt32 id);
	UInt32	ResetAllVariables();

	void	Destructor();
};

ScriptEventList* EventListFromForm(TESForm* form);

typedef void (* _Console_Print)(const char * buf, ...);
extern const _Console_Print Console_Print;

typedef bool (* _MarkBaseExtraListScriptEvent)(TESForm* target, BaseExtraList* extraList, UInt32 eventMask);
extern const _MarkBaseExtraListScriptEvent MarkBaseExtraListScriptEvent;

typedef bool (* _ExtractArgs)(ParamInfo * paramInfo, void * arg1, UInt32 * arg2, TESObjectREFR * thisObj, TESObjectREFR* contObj, Script * script, ScriptEventList * eventList, ...);
extern const _ExtractArgs ExtractArgs;

// convenience macro for common arguments to ExtractArgs

#if OBSE_CORE
bool ExtractArgsEx(ParamInfo * paramInfo, void * scriptData, UInt32 * scriptDataOffset, Script * scriptObj, ScriptEventList * eventList, ...);
bool ExtractFormatStringArgs(UInt32 fmtStringPos, char* buffer, ParamInfo * paramInfo, void * scriptDataIn, UInt32 * scriptDataOffset, Script * scriptObj, ScriptEventList * eventList, UInt32 maxParams, ...);
#endif

bool ExtractSetStatementVar(Script* script, ScriptEventList* eventList, void* scriptDataIn, double* outVarData, UInt8* outModIndex = NULL);
bool ExtractFormattedString(FormatStringArgs& args, char* buffer);

// Problem: plugins may want to use %z specifier in format strings, but don't have access to StringVarMap
// Could change params to ExtractFormatStringArgs to include an OBSEStringVarInterface* but
//  this would break existing plugins
// Instead allow plugins to register their OBSEStringVarInterface for use
// I'm sure there is a better way to do this but I haven't found it
struct OBSEStringVarInterface;
void RegisterStringVarInterface(OBSEStringVarInterface* intfc);

typedef TESForm * (* _CreateFormInstance)(UInt8 type);
extern const _CreateFormInstance CreateFormInstance;

typedef TESForm * (* _LookupFormByID)(UInt32 id);
extern const _LookupFormByID LookupFormByID;

typedef void * (* _FormHeap_Allocate)(UInt32 size);
extern const _FormHeap_Allocate FormHeap_Allocate;

typedef void (* _FormHeap_Free)(void * ptr);
extern const _FormHeap_Free FormHeap_Free;

typedef void * (* _GetGlobalScriptStateObj)(void);
extern const _GetGlobalScriptStateObj GetGlobalScriptStateObj;

typedef void (* _ShowMessageBox_Callback)(void);
extern const _ShowMessageBox_Callback ShowMessageBox_Callback;

// message is messageBox string, unk2 is ID of first button, var args are char* for buttons (args terminated by NULL pointer)
// pass at least one button string or the messagebox can't be closed
typedef bool (* _ShowMessageBox)(const char * message, _ShowMessageBox_Callback callback, UInt32 baseButtonIndex, ...);
extern const _ShowMessageBox ShowMessageBox;

// set to scriptObj->refID after calling ShowMessageBox()
// GetButtonPressed checks this before returning a value, if it doesn't match it returns -1
typedef UInt32* _ShowMessageBox_pScriptRefID;
extern const _ShowMessageBox_pScriptRefID ShowMessageBox_pScriptRefID;
typedef UInt8* _ShowMessageBox_button;
extern const _ShowMessageBox_button ShowMessageBox_button;

// unk1 = 0, unk2 = 1
typedef bool (* _QueueUIMessage)(const char * string, UInt32 unk1, UInt32 unk2, float duration);
extern const _QueueUIMessage QueueUIMessage;
const UInt32 kMaxMessageLength = 0x4000;

//displays icon and plays sound (used by Additem, Addspell, etc...)
//ddsPath relative to Textures\Menus\...  soundID as defined in the CS
typedef bool (* _QueueUIMessage_2)(const char * string, float duration, const char * ddsPath, const char * soundID);
extern const _QueueUIMessage_2 QueueUIMessage_2;

typedef bool (* _IsGodMode)(void);
extern const _IsGodMode IsGodMode;

typedef char (__stdcall * _ScancodeToChar)(UInt32 scanCode, UInt32 bUppercase);
extern const _ScancodeToChar ScancodeToChar;

extern MemoryHeap	* g_formHeap;

UInt32 AddFormToDataHandler(DataHandler * dataHandler, TESForm * form);
extern DataHandler ** g_dataHandler;

typedef  const char* (__cdecl * _GetFormModelPath)(TESForm*);
extern const _GetFormModelPath GetFormModelPath;

class NiFormArray;
// BSTCaseInsensitiveStringMap<IDLE_ANIM_ROOT *> / NiTPointerMap<const char*, IDLE_ANIM_ROOT *>
extern const NiTMapBase<const char*, NiFormArray*>**  g_IdleAnimationMap;

// 1C8 (different between 1.1, 1.2)
class TESSaveLoadGame
{
public:
	TESSaveLoadGame();
	~TESSaveLoadGame();

	UInt32	unk000[0x28 >> 2];	// 000 (1.2)

	struct CreatedObject
	{
		UInt32			formID;
		CreatedObject	* next;

		UInt32			Info() const	{ return formID; }
		CreatedObject *	Next() const	{ return next; }
	};

	typedef Visitor <CreatedObject, UInt32>	CreatedObjectVisitor;

	CreatedObject	createdObjectList;				// 028 (1.2) 02C (1.1)
	UInt32			unk030[(0x048 - 0x030) >> 2];	// 030 (1.2) 034 (1.1)

	UInt8	numMods;			// 048 (1.2) 04C (1.1)
	UInt8	pad049[3];			// 049 (1.2) 04D (1.1)
	UInt8	* modRefIDTable;	// 04C (1.2) 050 (1.1)
								// table mapping stored mod indices to loaded mod indices

	void	LoadCreatedObjectsHook(UInt32 unk0);
	bool	LoadGame(const char* filename);

	// returns number of mod references to object. unk2 seen 0, is compared to 0xC
	UInt32	ResetObject(TESForm* object, UInt32 changeFlags, UInt32 unk2);
};

STATIC_ASSERT(offsetof(TESSaveLoadGame, numMods) ==  0x48);

void AddFormToCreatedBaseObjectsList(TESSaveLoadGame * objList, TESForm * form);
extern TESSaveLoadGame ** g_createdBaseObjList;	// a TESSaveLoadGame
// TESSaveLoadGame + 0x14 is a void * to the current save/load buffer
// TESSaveLoadGame + 0x80 stores a UInt8 containing the version of the save?

UInt32 NiTPointerMap_Lookup(void * map, void * key, void ** data);
extern void * g_gameSettingsTable;

extern const bool * g_bConsoleMode;
bool IsConsoleMode();

extern const bool * g_bIsConsoleOpen;
bool IsConsoleOpen();

const char * GetObjectClassName(void * obj);

const char * GetFullName(TESForm * baseForm);

extern char*** g_baseActorValueNames;		//those with an associated game setting string
extern char** g_extraActorValueNames;		//MagickaMultiplier .. ResistWaterDamage (unchangeable)
extern char** g_scriptActorValueNames;		// names as used in scripts

const char* GetActorValueString(UInt32 actorValue);
UInt32 GetActorValueForString(const char* strActorVal, bool bForScript = false);
bool IsValidActorValue(UInt32 actorValue);

UInt32 SafeModUInt32(UInt32 originalVal, float modBy);
float SafeChangeFloat(float originalVal, float changeVal, bool bMod, bool bNegativeAllowed);

extern CRITICAL_SECTION * g_extraListMutex;
extern CRITICAL_SECTION * g_pathingMutex;

struct SettingInfo
{
	union
	{
		bool	b;
		float	f;
		int		i;
		char	* s;
		UInt32	u;
	};
	
	char	* name;
	
	enum EType {
		kSetting_Bool = 0,
		kSetting_c,
		kSetting_h,
		kSetting_Integer,
		kSetting_Unsigned,
		kSetting_Float,
		kSetting_String,
		kSetting_r,
		kSetting_a,
		kSetting_Other
	};

	EType Type() const;

	void Set(const char* str);
};

bool GetGameSetting(const char *settingName, SettingInfo** setting);

struct INISettingEntry
{
	typedef	SettingInfo	Data;

	Data			* data;
	INISettingEntry	* next;
};

// 114
// ###TODO: inheritance hierarchy
class IniSettingCollection
{
public:
	virtual void AddSetting (SettingInfo* info) = 0;
	virtual void RemoveSetting (SettingInfo* info) = 0;
	virtual void Unk_02 (void* arg0) = 0;
	virtual bool SaveSetting (SettingInfo* info) = 0;		// individual setting to file
	virtual bool LoadSetting (SettingInfo* info) = 0;		// individual setting from file
	virtual bool PrepareToWrite () = 0;						// called before WriteToFile ()
	virtual bool FinishWrite () = 0;						// called after WriteToFile ()
	virtual bool WriteToFile () = 0;
	virtual bool ReadFromFile () = 0;						// requires INISettingEntry list has already been populated, refreshes values from file

	IniSettingCollection ();
	~IniSettingCollection ();

	// vtbl													// 000
	char					iniFilePath[(0x108-0x004)];		// 004
	IniSettingCollection	* writeInProgressCollection;	// 108 set to 'this' in PrepareToWrite(), to null in FinishWrite(), checked before read/write operations
	INISettingEntry			settingsList;					// 10C

	static IniSettingCollection* GetSingleton ();
};

STATIC_ASSERT (sizeof (IniSettingCollection) == 0x114);

// 134
class InterfaceManager
{
public:
	InterfaceManager();
	~InterfaceManager();

	static InterfaceManager *	GetSingleton(void);

	enum {				// special values for IsActiveMenuMode()
		kMenuMode_GameMode = 0,
		kMenuMode_BigFour,
		kMenuMode_Any,
		kMenuMode_Console,
	};

	SceneGraph*		unk000;							// 000
	SceneGraph*		unk004;							// 004
	UInt32			unk008[(0x018 - 0x008) >> 2];	// 008
	void*			unk018;							// 018 NiDirectionalLight *
	Tile			* cursor;						// 01C
	UInt32			unk020[(0x050 - 0x020) >> 2];	// 020
	bool			debugTextOn;					// 050
	UInt8			unk051[3];
	NiNode*			unk054[(0x064 - 0x054) >> 2];	// 054
	NiNode*			unk064;							// 064 ShadowSceneNode *
	Tile			* menuRoot;						// 068
	Tile			* strings;						// 06C
	NiNode*			unk070;							// 070
	UInt32			unk074;							// 074
	void*			unk078;							// 078 NiAlphaProperty*
	UInt32			unk07C;							// 07C
	Tile			* hudReticule;					// 080
	UInt32			unk084;							// 084
	Tile			* altActiveTile;				// 088 appears to be active tile when activeTile is null and keyboard navigation used
	UInt32			unk08C;							// 08C
	UInt32			unk090;							// 090
	UInt32			unk094;							// 094
	Tile			* activeTile;					// 098 - moused-over tile
	Menu			* activeMenu;					// 09C - menu over which the mouse cursor is placed
	UInt32			unk0A0;							// 0A0
	UInt32			unk0A4;							// 0A4
	UInt32			unk0A8;							// 0A8
	UInt32			unk0AC;							// 0AC
	UInt8			msgBoxButtonPressed;			// 0B0
	UInt8			unk0B1[3];						// 0B1
	void			* unk0B4;						// 0B4 - stores callback for ShowMessageBox() (anything else?)
	UInt32			unk0B8;							// 0B8	
	TESObjectREFR	* debugSelection;				// 0BC
	UInt32			unk0C0[(0x134 - 0x0C0) >> 2];	// 0C0

	bool CreateTextEditMenu(const char* promptText, const char* defaultText);
	float GetDepth();
	bool MenuModeHasFocus(UInt32 menuType);		// returns true if menuType is on top (has focus)
	bool IsGameMode();

	static void ToggleDebugText();

};

STATIC_ASSERT(offsetof(InterfaceManager, activeMenu) == 0x09C);
STATIC_ASSERT(sizeof(InterfaceManager) == 0x134);

// 18
class FontManager
{
public:
	FontManager();
	~FontManager();

	// 3C
	struct FontInfo {
		FontInfo();
		~FontInfo();

		UInt8						bLoadedSuccessfully;		// 00
		UInt8						pad01[3];
		char						* path;						// 04 "Data\\Fonts\\XXX.fnt"
		UInt16						ID;							// 08 1-5 for default game fonts
		UInt16						pad0A;
		NiTexturingProperty			* textureProperty;			// 0C
		UInt32						unk10[(0x2C-0x10) >> 2];	// 10
		float						unk2C;						// 2C
		float						unk30;						// 30
		NiD3DShaderConstantMapEntry	* unk34;					// 34
		void						* unk38;					// 38 size == 0x3928. looks like file buffer
																//	  +4 == num textures (max 8)

		static FontInfo * Load(const char* path, UInt32 ID);
		bool GetName(char* out);	// filename stripped of path and extension
	};

	FontInfo	* fontInfos[5];		// 00 indexed by FontInfo::ID - 1; access inlined at each point in code where font requested
	UInt8		unk14;				// 14
	UInt8		pad15[3];

	static FontManager * GetSingleton();
};

STATIC_ASSERT(sizeof(FontManager) == 0x18);
STATIC_ASSERT(sizeof(FontManager::FontInfo) == 0x3C);

bool SCRIPT_ASSERT(bool expr, Script* script, const char * errorMsg, ...);

//A4
class ScriptRunner
{
public:
	static ScriptRunner *	GetSingleton;

	UInt32					unk00;						//00
	UInt32					unk04;						//04
	ScriptEventList			* eventList;				//08
	UInt32					unk0C;						//0C
	UInt32					unk10;						//10
	Script					* script;					//14
	UInt32					unk18[ (0xA0 - 0x18) >> 2];	//18..9F
	UInt8					unkA0;						//A0
	UInt8					unkA1;						//A1
	UInt8					unkA2;						//A2
	UInt8					unkA3;						//A3

	//unk4 = offset or length
	bool ExecuteLine(Script* scriptObj, UInt16 opcode, UInt32 unk2, UInt16* unk3, UInt16 unk4, UInt16* unk5, 
					 UInt32 currentLine, UInt32 unk7, UInt32 unk8);
	bool Run(Script* scriptObj, UInt32 unk1, ScriptEventList* eventList, UInt32 unk3, UInt32 unkt4, UInt32 unk5,
		UInt32 unk6, float unk7);
};

STATIC_ASSERT(sizeof(ScriptRunner) == 0xA4);

// A plugin author requested the ability to use OBSE format specifiers to format strings with the args
// coming from a source other than script.
// So changed ExtractFormattedString to take an object derived from following class, containing the args
// Probably doesn't belong in GameAPI.h but utilizes a bunch of stuff defined here and can't think of a better place for it
class FormatStringArgs
{
public:
	enum argType {
		kArgType_Float,
		kArgType_Form		// TESForm*
	};

	virtual bool Arg(argType asType, void * outResult) = 0;	// retrieve next arg
	virtual bool SkipArgs(UInt32 numToSkip) = 0;			// skip specified # of args
	virtual bool HasMoreArgs() = 0;
	virtual std::string GetFormatString() = 0;						// return format string
};

// concrete class used for extracting script args
class ScriptFormatStringArgs : public FormatStringArgs
{
public:
	virtual bool Arg(argType asType, void* outResult);
	virtual bool SkipArgs(UInt32 numToSkip);
	virtual bool HasMoreArgs();
	virtual std::string GetFormatString();

	ScriptFormatStringArgs(UInt32 _numArgs, UInt8* _scriptData, Script* _scriptObj, ScriptEventList* _eventList);
	UInt32 GetNumArgs();
	UInt8* GetScriptData();

private:
	UInt32			numArgs;
	UInt8			* scriptData;
	Script			* scriptObj;
	ScriptEventList	* eventList;
	std::string fmtString;
};

void ShowCompilerError(ScriptLineBuffer* lineBuf, const char* fmt, ...);

Script* GetCurrentExecutingScript(void);
ScriptEventList* GetCurrentExecutingScriptEventList(void);

