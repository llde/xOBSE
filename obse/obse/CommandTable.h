#pragma once

enum ParamType
{
	kParamType_String =				0x00,
	kParamType_Integer =			0x01,
	kParamType_Float =				0x02,
	kParamType_InventoryObject =	0x03,	// GetItemCount				TESForm *, must pass IsInventoryObjectType check
	kParamType_ObjectRef =			0x04,	// Activate					TESObjectREFR *
	kParamType_ActorValue =			0x05,	// ModActorValue			not in switch
	kParamType_Actor =				0x06,	// ToggleAI					TESObjectREFR *, must pass IsActor
	kParamType_SpellItem =			0x07,	// AddSpell					TESForm *, must be either SpellItem or book
	kParamType_Axis =				0x08,	// Rotate					not in switch (X Y Z, passed as char)
	kParamType_Cell =				0x09,	// GetInCell				TESObjectCELL *, must pass sub_4C0780
	kParamType_AnimationGroup =		0x0A,	// PlayGroup				not in switch
	kParamType_MagicItem =			0x0B,	// Cast						MagicItem *
	kParamType_Sound =				0x0C,	// Sound					TESForm *, kFormType_Sound
	kParamType_Topic =				0x0D,	// Say						TESForm *, kFormType_Dialog
	kParamType_Quest =				0x0E,	// ShowQuestVars			TESForm *, kFormType_Quest
	kParamType_Race =				0x0F,	// GetIsRace				TESForm *, kFormType_Race
	kParamType_Class =				0x10,	// GetIsClass				TESForm *, kFormType_Class
	kParamType_Faction =			0x11,	// Faction					TESForm *, kFormType_Faction
	kParamType_Sex =				0x12,	// GetIsSex					not in switch
	kParamType_Global =				0x13,	// GetGlobalValue			TESForm *, kFormType_Global
	kParamType_Furniture =			0x14,	// IsCurrentFurnitureObj	TESForm *, kFormType_Furniture
	kParamType_TESObject =			0x15,	// PlaceAtMe				TESObject *
	kParamType_VariableName =		0x16,	// GetQuestVariable			not in switch
	kParamType_QuestStage =			0x17,	// SetStage					handled like integer
	kParamType_MapMarker =			0x18,	// ShowMap					TESObjectREFR *, base form must be dword_AF36F8
	kParamType_ActorBase =			0x19,	// SetEssential				TESActorBase *
	kParamType_Container =			0x1A,	// RemoveMe					TESObjectREFR *, must pass TESObjectREFR_GetContainer
	kParamType_WorldSpace =			0x1B,	// CenterOnWorld			TESWorldSpace *
	kParamType_CrimeType =			0x1C,	// GetCrimeKnown			not in switch
	kParamType_AIPackage =			0x1D,	// GetIsCurrentPackage		TESPackage *
	kParamType_CombatStyle =		0x1E,	// SetCombatStyle			TESCombatStyle *
	kParamType_MagicEffect =		0x1F,	// HasMagicEffect			EffectSetting *
	kParamType_Birthsign =			0x20,	// GetIsPlayerBirthsign		TESForm *, kFormType_BirthSign
	kParamType_FormType =			0x21,	// GetIsUsedItemType		FormType (not all types supported)
	kParamType_WeatherID =			0x22,	// GetIsCurrentWeather		TESForm *, kFormType_Weather
	kParamType_NPC =				0x23,	// Say						TESForm *, kFormType_NPC
	kParamType_Owner =				0x24,	// IsOwner					TESForm *, kFormType_NPC or kFormType_Faction
	kParamType_EffectShader =		0x25,	// haven't seen used		TESForm *, kFormType_EffectShader

	// custom OBSE types
	kParamType_StringVar =			0x01,
	kParamType_Array =				0x26,	// only usable with compiler override; StandardCompile() will report unrecognized param type
};

/*** IsInventoryObjectType types
 *	APPA ARMO BOOK CLOT
 *	INGR LIGH MISC
 *	WEAP AMMO
 *	SLGM KEYM ALCH
 *	SGST
 ***/

/*** kParamType_AnimationGroup is converted to a UInt16 code when compiled
Doc'ed here for future ref
AnimGroup		  Code (0x)
---------		  ---------
Idle 				00
DynamicIdle			01
SpecialIdle			02
Forward				03
Backward			04
Left				05
Right				06
FastForward 		07
FastBackward 		08
FastLeft 			09
FastRight 			0A
DodgeForward		0B
DodgeBack 			0C
DodgeLeft 			0D
DodgeRight			0E
TurnLeft			0F
TurnRight			10
Equip 				11
Unequip				12
AttackBow 			13
AttackLeft 			14
AttackRight			15
AttackPower 		16
AttackForwardPower	17
AttackBackPower		18
AttackLeftPower		19
AttackRightPower	1A
BlockIdle 			1B
BlockHit 			1C
BlockAttack			1D
Recoil				1E
Stagger				1F
Death				20
TorchIdle			21
CastSelf 			22
CastTouch 			23
CastTarget			24
CastSelfAlt 		25
CastTouchAlt		26
CastTargetAlt 		27
JumpStart 			28
JumpLoop			29
JumpLand 			2A
*/

struct ParamInfo
{
	const char	* typeStr;
	UInt32		typeID;		// ParamType
	UInt32		isOptional;	// do other bits do things?
};

struct ScriptEventList;
class TESObjectREFR;
class Script;
struct ScriptBuffer;
struct ScriptLineBuffer;

#define COMMAND_ARGS	ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr
#define PASS_COMMAND_ARGS paramInfo, arg1, thisObj, arg3, scriptObj, eventList, result, opcodeOffsetPtr
#define EXTRACT_ARGS	paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList
#define COMMAND_ARGS_EVAL TESObjectREFR * thisObj, void * arg1, void * arg2, double * result

//Macro to make CommandInfo definitions a bit less tedious

#define DEFINE_COMMAND(name, description, refRequired, numParams, paramInfo) \
	CommandInfo (kCommandInfo_ ## name) = { \
	#name, \
	"", \
	0, \
	#description, \
	refRequired, \
	numParams, \
	paramInfo, \
	HANDLER(Cmd_ ## name ## _Execute), \
	Cmd_Default_Parse, \
	NULL, \
	0 \
	};

#define DEFINE_COMMAND_CONDITIONAL(name ,description ,refRequired ,numParams ,paramInfo)\
	CommandInfo (kCommandInfo_ ## name) = { \
	#name, \
	"", \
	0, \
	#description, \
	refRequired, \
	numParams, \
	paramInfo, \
	HANDLER(Cmd_ ## name ## _Execute), \
	Cmd_Default_Parse, \
	HANDLER_EVAL(Cmd_ ## name ## _Eval), \
	0 \
	};

#define DEFINE_COMMAND(name, description, refRequired, numParams, paramInfo) \
	CommandInfo (kCommandInfo_ ## name) = { \
	#name, \
	"", \
	0, \
	#description, \
	refRequired, \
	numParams, \
	paramInfo, \
	HANDLER(Cmd_ ## name ## _Execute), \
	Cmd_Default_Parse, \
	NULL, \
	0 \
	};

#define DEFINE_CMD_COND(name, description, refRequired, paramInfo) \
	CommandInfo (kCommandInfo_ ## name) = { \
	#name, \
	"",	\
	0,	\
	#description, \
	refRequired, \
	SIZEOF_ARRAY(paramInfo, ParamInfo), \
	paramInfo, \
	HANDLER(Cmd_ ## name ## _Execute), \
	Cmd_Default_Parse, \
	HANDLER_EVAL(Cmd_ ## name ## _Eval), \
	1 \
	};

#define DEFINE_COMMAND_PLUGIN(name, description, refRequired, numParams, paramInfo) \
	CommandInfo (kCommandInfo_ ## name) = { \
	#name, \
	"", \
	0, \
	#description, \
	refRequired, \
	numParams, \
	paramInfo, \
	HANDLER(Cmd_ ## name ## _Execute), \
	NULL, \
	NULL, \
	0 \
	};

#define DEFINE_CMD_ALT(name, altName, description, refRequired, paramInfo) \
	CommandInfo (kCommandInfo_ ## name) = { \
	#name, \
	#altName, \
	0, \
	#description, \
	refRequired, \
	SIZEOF_ARRAY(paramInfo, ParamInfo), \
	paramInfo, \
	HANDLER(Cmd_ ## name ## _Execute), \
	Cmd_Default_Parse, \
	NULL, \
	0 \
	};

typedef bool (* Cmd_Execute)(COMMAND_ARGS);
bool Cmd_Default_Execute(COMMAND_ARGS);

typedef bool (* Cmd_Parse)(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf);
bool Cmd_Default_Parse(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf);

typedef bool (* Cmd_Eval)(TESObjectREFR * thisObj, void * arg1, void * arg2, double * result);
bool Cmd_Default_Eval(COMMAND_ARGS_EVAL);

#ifdef OBLIVION
#define HANDLER(x)	x
#define HANDLER_PARSE(x) Cmd_Default_Parse
#define HANDLER_EVAL(x)	x
#else
#define HANDLER(x)	Cmd_Default_Execute
#define HANDLER_PARSE(x) x
#define HANDLER_EVAL(x)	Cmd_Default_Eval
#endif

// unk3 args:
// TESObjectREFR * thisObj
// TESForm * param
// TESForm * param2
// double * result

struct CommandInfo
{
	const char	* longName;		//  0
	const char	* shortName;	//  4
	UInt32		opcode;			//  8
	const char	* helpText;		//  C
	UInt16		needsParent;	// 10
	UInt16		numParams;		// 12
	ParamInfo	* params;		// 14

	// handlers
	Cmd_Execute	execute;		// 18
	Cmd_Parse	parse;			// 1C
	Cmd_Eval	eval;			// 20 - smaller version of Cmd_Execute with arg extracted already, used for dialog condition evaluation

	UInt32	flags;				// 24

	void	DumpFunctionDef() const;
	void	DumpDocs() const;
	void	DumpXML(std::ofstream& out);
};

// annoyingly we're going to have to record return type info for vanilla cmds too
// as we can't tell at run-time whether the command returned a form or a float
enum CommandReturnType : UInt8
{
	kRetnType_Default,
	kRetnType_Form,
	kRetnType_String,
	kRetnType_Array,
	kRetnType_ArrayIndex,
	kRetnType_Ambiguous,

	kRetnType_Max
};

const UInt32 kObseOpCodeStart = 0x1400;

struct PluginInfo;
class CommandTable
{
public:
	CommandTable();
	~CommandTable();

	static void	Init(void);

	void	Read(CommandInfo * start, CommandInfo * end);
	void	Add(CommandInfo * info, CommandReturnType retnType = kRetnType_Default, UInt32 parentPluginOpcodeBase = 0);
	void	PadTo(UInt32 id, CommandInfo * info = NULL);
	bool	Replace(UInt32 opcodeToReplace, CommandInfo* replaceWith);
	void	SetBaseID(UInt32 id)	{ m_baseID = id; m_curID = id; }

	void	Dump(void);

	CommandInfo *	GetStart(void);
	CommandInfo *	GetEnd(void);

	CommandInfo *	GetByName(const char * name);
	CommandInfo *	GetByOpcode(UInt32 opcode);

	UInt32	GetMaxID(void)		{ return m_baseID + m_commands.size(); }
	void	SetCurID(UInt32 id)	{ m_curID = id; }
	UInt32	GetCurID(void)		{ return m_curID; }

	void	DumpAlternateCommandNames(void);
	void	DumpCommandDocumentation(UInt32 startWithID = kObseOpCodeStart);
	void	DumpCommandXML(UInt32 startWithID = kObseOpCodeStart);

	CommandReturnType	GetReturnType(const CommandInfo * cmd);
	void				SetReturnType(UInt32 opcode, CommandReturnType retnType);

	UInt32				GetRequiredOBSEVersion(const CommandInfo * cmd);
	PluginInfo *		GetParentPlugin(const CommandInfo * cmd);

private:
	typedef std::vector <CommandInfo>				CommandList;
	typedef std::map <UInt32, CommandReturnType>	OpcodeReturnTypeMap;
	typedef std::map <UInt32, UInt32>				OpcodeToPluginMap;

	CommandList	m_commands;

	UInt32		m_baseID;
	UInt32		m_curID;

	// todo: combine these in to a single struct
	OpcodeReturnTypeMap	m_returnTypes;		// maps opcode to return type, only string/array-returning cmds included
	OpcodeToPluginMap	m_opcodesByPlugin;	// maps opcode to owning plugin opcode base

	std::vector<UInt32>	m_opcodesByRelease;	// maps an OBSE major version # to opcode of first command added to that release, beginning with v0008

	void	RecordReleaseVersion(void);
	void	RemoveDisabledPlugins(void);
};

extern CommandTable	g_consoleCommands;
extern CommandTable	g_scriptCommands;

typedef bool (* _Cmd_Execute)(COMMAND_ARGS);

namespace PluginAPI {
	const CommandInfo* GetCmdTblStart();
	const CommandInfo* GetCmdTblEnd();
	const CommandInfo* GetCmdByOpcode(UInt32 opcode);
	const CommandInfo* GetCmdByName(const char* name);
	UInt32 GetCmdRetnType(const CommandInfo* cmd);
	UInt32 GetReqVersion(const CommandInfo* cmd);
	const PluginInfo* GetCmdParentPlugin(const CommandInfo* cmd);
}
