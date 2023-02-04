#pragma once

// not exposing entire class hierarchies here
// just the minimum needed to interface with the script compiler
// some class fields are only present in editor or only at run-time
// easier/cleaner to redefine them here than to muddy up the run-time defs with a bunch of #if's

#include "obse\GameTypes.h"
#include "obse\GameBSExtraData.h"

class Script;
class TESObjectCELL;
class NiNode;

class BaseFormComponent
{
public:
	BaseFormComponent();
	~BaseFormComponent();

	virtual void	Destructor(void);	// 00
	// ...

	//	void		** _vtbl;	// 000
};

//24
class TESForm : public BaseFormComponent
{
public:
	TESForm();
	~TESForm();

	virtual void Unk01(void);
	//...

	struct Unk {
		void	* unk00;
		void	* unk04;
	};

	//void		** vtbl;		//00
	UInt8		typeID;			//04
	UInt8		pad04[3];		//05 .. 07
	UInt32		unk08;			//08 - flags?
	UInt32		refID;			//0C
	String		editorID;		//10 .. 17
	UInt32		unk18;			//18
	Unk			unk1C;			//1C .. 23
};

class TESScriptableForm : public BaseFormComponent
{
public:
	TESScriptableForm();
	~TESScriptableForm();

	Script	* script;	// 004
	UInt8	unk1;		// 008
	UInt8	pad[3];		// 009
};

// 064
class TESObjectREFR : public TESForm
{
public:
	TESObjectREFR();
	~TESObjectREFR();

	UInt32	childCell;				// 024
	TESForm	* baseForm;				// 028
	float	rotX, rotY, rotZ;		// 02C 
	float	posX, posY, posZ;		// 038 
	float	scale;					// 044 
	NiNode	* niNode;				// 048
	TESObjectCELL	* parentCell;	// 04C
	ExtraDataList	baseExtraList;	// 050
};


// 054
class Script : public TESForm		
{
public:
	Script();
	~Script();

	enum {
		eVarType_Float = 0,			//ref is also zero
		eVarType_Integer,

		// OBSE, return values only
		eVarType_String,
		eVarType_Array,
		eVarType_Ref,

		eVarType_Invalid
	};

	struct RefVariable
	{
		string	name;		// variable name/editorID (not used at run-time)
		TESForm	* form;
		UInt32	varIdx;		// always zero in editor? refs stored in order

		void	Resolve(ScriptEventList * eventList);
	};

	struct RefListEntry
	{
		RefVariable		* var;
		RefListEntry	* next;

		RefVariable* Info() const { return var; }
		RefListEntry* Next() const { return next; }
	};

	typedef Visitor<RefListEntry, RefVariable> RefListVisitor;


	struct VariableInfo
	{
		UInt32			idx;		// 00
		UInt32			pad04;		// 04
		double			data;		// 08
		UInt8			type;		// 10
		UInt8			pad11[3];	// 11
		UInt32			unk14;		// 14
		String			name;		// 18
	};

	struct VarInfoEntry
	{
		VariableInfo	* data;
		VarInfoEntry	* next;

		VariableInfo* GetVariableByName(const char* varName)
	};

	UInt32			unk0;		// 24
	UInt32			numRefs;	// 28
	UInt32			dataLen;	// 2C
	UInt32			varCount;	// 30 
	UInt8			type;		// 34 
	UInt8			unk35;		// 35
	UInt8			unk36[2];
	char			* text;		//38
	void			* data;		//3C
	RefListEntry	refVars;	//40 .. 47
	VarInfoEntry	vars;		//48 .. 4F
	UInt8			unk50;		//50
	UInt8			pad51[3];	//51 .. 53

	VariableInfo* GetVariableInfo(UInt32 idx);
};

