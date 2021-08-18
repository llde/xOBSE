#pragma once

#include "Utilities.h"
#include "GameForms.h"

struct ScriptEventList;
struct ScriptBuffer;

#if OBLIVION

#define SCRIPT_SIZE 0x50
static const UInt32 kScript_ExecuteFnAddr = 0x004FBE00;


#else

#define SCRIPT_SIZE 0x54
#if CS_VERSION == CS_VERSION_1_2
	static const UInt32 kScript_SetTextFnAddr = 0x004FC6C0;
#else
#error unsupported CS version
#endif

#endif

// 50 / 54
class Script : public TESForm
{
public:
	// no changed flags (TESForm flags)
	MEMBER_FN_PREFIX(Script);

	Script();
	~Script();

	// members

	struct RefVariable
	{
		BSStringT	name;		// variable name/editorID (not used at run-time)
		TESForm	* form;
		UInt32	varIdx;		// always zero in editor

		void	Resolve(ScriptEventList * eventList);
	};

	struct RefListEntry
	{
		RefVariable		* var;
		RefListEntry	* next;

		RefVariable* Info() const { return var; }
		RefListEntry* Next() const { return next; }
		void SetNext(RefListEntry* nextEntry) { next = nextEntry; }
		RefVariable* GetRefVariableByName(const char* name);
		UInt32 GetIndex(RefVariable* refVar);
	};

	typedef Visitor<RefListEntry, RefVariable> RefListVisitor;

	enum {
		eVarType_Float = 0,			//ref is also zero
		eVarType_Integer,

		// OBSE, return values only
		eVarType_String,
		eVarType_Array,
		eVarType_Ref,

		eVarType_Invalid
	};

	struct VariableInfo
	{
		UInt32			idx;		// 00
		UInt32			pad04;		// 04
		double			data;		// 08
		UInt8			type;		// 10
		UInt8			pad11[3];	// 11
		UInt32			unk14;		// 14
		BSStringT			name;		// 18
	};

	struct VarInfoEntry
	{
		VariableInfo	* data;
		VarInfoEntry	* next;

		VariableInfo* Info() const { return data; }
		VarInfoEntry* Next() const { return next; }

		VariableInfo* GetVariableByName(const char* name);
	};
	typedef Visitor<VarInfoEntry, VariableInfo> VarListVisitor;

	// 14
	struct ScriptInfo
	{
		UInt32	unk0;		// 00
		UInt32	numRefs;	// 04
		UInt32	dataLength;	// 08
		UInt32	varCount;	// 0C
		UInt32	type;		// 10
	};

	enum {
		eType_Object = 0,
		eType_Quest = 1,
		eType_Magic = 0x100
	};

	ScriptInfo		info;					// 018 / 024
	char			* text;					// 02C / 038
	void			* data;					// 030 / 03C
#if OBLIVION
	UInt32			unk34;					// 034
	float			questDelayTimeCounter;	// 038      - init'd to fQuestDelayTime, decremented by frametime each frame
	float			secondsPassed;			// 03C      - only if you've modified fQuestDelayTime
#endif
	RefListEntry	refList;				// 040 / 040 - ref variables and immediates
	VarInfoEntry	varList;				// 048 / 048 - local variable list
#if !OBLIVION
	UInt8			unk50;					//	   / 050
	UInt8			pad51[3];
#endif

	RefVariable *	GetVariable(UInt32 reqIdx);
	VariableInfo*	GetVariableInfo(UInt32 idx);

	UInt32			AddVariable(TESForm * form);
	void			CleanupVariables(void);

	UInt32			Type() const { return info.type; }
	bool			IsObjectScript() const {return info.type == eType_Object; }
	bool			IsQuestScript() const { return info.type == eType_Quest; }
	bool			IsMagicScript() const { return info.type == eType_Magic; }
	VariableInfo*	GetVariableByName(const char* varName);
	UInt32			GetVariableType(VariableInfo* var);

#if OBLIVION
	// arg3 appears to be true for result scripts (runs script even if dataLength <= 4)
	DEFINE_MEMBER_FN(Execute, bool, kScript_ExecuteFnAddr, TESObjectREFR* thisObj, ScriptEventList* eventList, TESObjectREFR* containingObj, bool arg3);
	void			Constructor(void);
	void			StaticDestructor(void);
	void			SetText(const char * buf);
	ScriptEventList	* CreateEventList();
	bool			CompileAndRun(void * unk0, UInt32 unk1, void * unk2);
#else
	DEFINE_MEMBER_FN(SetText, void, kScript_SetTextFnAddr, const char* newText);
#endif
};

STATIC_ASSERT(sizeof(Script) == SCRIPT_SIZE);

// 6C
struct QuestStageItem
{
	struct LogDate {
		UInt16	dayOfYear;
		UInt16	year;

		void Get(UInt16& d, UInt32& m, UInt16& y);
		bool Set(UInt32 d, UInt32 m, UInt32 y);
		bool Set(const LogDate& date);

		static LogDate Create(UInt32 d, UInt32 m, UInt32 y);
	};

	UInt32			unk00;			// 00
	ConditionEntry	conditionList;	// 04
	Script			resultScript;	// 0C
	UInt32			unk5C;			// 5C disk offset to log text records? consistent within a single quest
	UInt8			index;			// 60 sequential
	UInt8			unk61;			// 61 previously 'hasLogText', which is incorrect
	UInt8			unk62[2];		// 62 pad?
	LogDate			* logDate;		// 64 alloc'ed when added to log. (Why use a 32-bit pointer to a 32-bit structure?)
	TESQuest		* owningQuest;	// 68

	const char* GetLogText() const;
	bool SetLogDate(const LogDate& date);	// only supported if date already set (quest stage item is known to player)
};

#if OBLIVION
STATIC_ASSERT(sizeof(QuestStageItem) == 0x6C);
#endif

// 41C
struct ScriptLineBuffer
{
	static const UInt32	kBufferSize = 0x200;

	UInt32				lineNumber;			// 000 counts blank lines too
	char				paramText[0x200];	// 004 portion of line text following command
	UInt32				paramTextLen;		// 204
	UInt32				lineOffset;			// 208
	UInt8				dataBuf[0x200];		// 20C
	UInt32				dataOffset;			// 40C
	UInt32				cmdOpcode;			// 410 not initialized. Opcode of command being parsed
	UInt32				callingRefIndex;	// 414 not initialized. Zero if cmd not invoked with dot syntax
	UInt32				unk418;				// 418

	// these write data and update dataOffset
	bool Write(const void* buf, UInt32 bufsize);
	bool WriteFloat(double buf);
	bool WriteString(const char* buf);
	bool Write32(UInt32 buf);
	bool Write16(UInt16 buf);
	bool WriteByte(UInt8 buf);
};

// size 0x58? Nothing initialized beyond 0x50.
struct ScriptBuffer
{
	template <typename tData> struct Node
	{
		tData		* data;
		Node<tData>	* next;
	};

	char			* scriptText;		// 000
	UInt32			textOffset;			// 004
	UInt32			unk008;				// 008
	BSStringT		scriptName;			// 00C
	UInt32			unk014;				// 014
	UInt8			scriptFragment;		// 018 set to 1 when compiling results scripts in quest stages, dialogue topics and AI packages
	UInt8			pad019;				// 019
	UInt16			unk01A;				// 01A
	UInt32			curLineNumber;		// 01C
	UInt8			* scriptData;		// 020 pointer to 0x4000-byte array
	UInt32			dataOffset;			// 024
	UInt32			unk028;				// 028
	UInt32			numRefs;			// 02C
	UInt32			unk030;				// 030
	UInt32			varCount;			// 034 script->varCount
	UInt8			scriptType;			// 038 script->type
	UInt8			unk039;				// 039 script->unk35
	UInt8			unk03A[2];
	Script::VarInfoEntry	vars;		// 03C
	Script::RefListEntry	refVars;	// 044 probably ref vars
	UInt32			unk04C;				// 04C num lines?
	Node<ScriptLineBuffer>	lines;		// 050
	// nothing else initialized

	// convert a variable or form to a RefVar, add to refList if necessary
	Script::RefVariable* ResolveRef(const char* refName);
	UInt32	GetRefIdx(Script::RefVariable* ref);
	UInt32	GetVariableType(Script::VariableInfo* varInfo, Script::RefVariable* refVar);
};

UInt32 GetDeclaredVariableType(const char* varName, const char* scriptText);	// parses scriptText to determine var type
Script* GetScriptFromForm(TESForm* form);

