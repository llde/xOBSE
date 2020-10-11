#pragma once

#if OBLIVION
#include "obse\GameAPI.h"

#else
#include "obse_editor\EditorAPI.h"
#endif

#include "obse/GameActorValues.h"
#include "obse/GameTypes.h"
#include "obse/GameBSExtraData.h"
#include "obse/NiNodes.h"
#include "Utilities.h"

// ### TODO: this doesn't belong here
struct ConditionEntry
{
	struct Data
	{
		union Param
		{
			float	number;
			TESForm	* form;
		};

		// ### TODO: this
		UInt32		operatorAndFlags;	// 00
		float		comparisonValue;	// 04
		UInt16		functionIndex;		// 08 is opcode & 0x0FFF
		UInt16		unk0A;
		Param		param1;				// 0C
		Param		param2;				// 10
		UInt32		unk14;
	};

	Data			* data;
	ConditionEntry	* next;
};

/****
 *	id	name	size	class
 *	00	NONE
 *	01	TES4
 *	02	GRUP
 *	03	GMST
 *	04	GLOB	28		TESGlobal
 *	05	CLAS	6C		TESClass
 *	06	FACT	44		TESFaction
 *	07	HAIR	4C		TESHair
 *	08	EYES	34		TESEyes
 *	09	RACE	318		TESRace
 *	0A	SOUN	44		TESSound
 *	0B	SKIL	60		TESSkill
 *	0C	MGEF	A8		EffectSetting
 *	0D	SCPT	50		Script
 *	0E	LTEX	34		TESLandTexture
 *	0F	ENCH	44		EnchantmentItem
 *	10	SPEL	44		SpellItem
 *	11	BSGN	4C		BirthSign
 *	12	ACTI	58		TESObjectACTI
 *	13	APPA	7C		TESObjectAPPA
 *	14	ARMO	E8		TESObjectARMO
 *	15	BOOK	8C		TESObjectBOOK
 *	16	CLOT	DC		TESObjectCLOT
 *	17	CONT	7C		TESObjectCONT
 *	18	DOOR	70		TESObjectDOOR
 *	19	INGR	80		IngredientItem
 *	1A	LIGH	90		TESObjectLIGH
 *	1B	MISC	70		TESObjectMISC
 *	1C	STAT	3C		TESObjectSTAT
 *	1D	GRAS	5C		TESGrass
 *	1E	TREE	80		TESObjectTREE
 *	1F	FLOR	64		TESFlora
 *	20	FURN	5C		TESFurniture
 *	21	WEAP	A0		TESObjectWEAP
 *	22	AMMO	84		TESAmmo
 *	23	NPC_	200		TESNPC
 *	24	CREA	138		TESCreature (patch 1.1)
 *				140		TESCreature (patch 1.2) - changed due to TESModelList
 *	25	LVLC	44		TESLevCreature
 *	26	SLGM	74		TESSoulGem
 *	27	KEYM	70		TESKey
 *	28	ALCH	80		AlchemyItem
 *	29	SBSP	30		TESSubSpace
 *	2A	SGST	88		TESSigilStone
 *	2B	LVLI	34		TESLevItem
 *	2C	SNDG
 *	2D	WTHR	148		TESWeather
 *	2E	CLMT	58		TESClimate
 *	2F	REGN	2C		TESRegion
 *	30	CELL	58		TESObjectCELL
 *	31	REFR	58		TESObjectREFR
 *	32	ACHR				used in save file for NPC_ placement
 *	33	ACRE				used in save file for CREA placement
 *	34	PGRD	54		TESPathGrid
 *	35	WRLD	E0		TESWorldSpace
 *	36	LAND	28		TESObjectLAND
 *	37	TLOD
 *	38	ROAD	30		TESRoad
 *	39	DIAL
 *	3A	INFO
 *	3B	QUST	68		TESQuest
 *	3C	IDLE	48		TESIdleForm
 *	3D	PACK	3C		TESPackage
 *	3E	CSTY	98		TESCombatStyle
 *	3F	LSCR	3C		TESLoadScreen
 *	40	LVSP	34		TESLevSpell
 *	41	ANIO	34		TESObjectANIO
 *	42	WATR	AC		TESWaterForm
 *	43	EFSH	110		TESEffectShader
 *	44	TOFT
 ***/

/**** change flags
 *	npc/creature
 *	00000004	CHANGE_ACTOR_BASE_HEALTH
 *	00000008	CHANGE_ACTOR_BASE_ATTRIBUTES
 *	00000010	CHANGE_ACTOR_BASE_DATA
 *	00000020	CHANGE_ACTOR_BASE_SPELLLIST
 *	00000040	CHANGE_ACTOR_BASE_FACTIONS
 *	10000000	CHANGE_ACTOR_BASE_MODIFIERS
 *	00000100	CHANGE_ACTOR_BASE_AIDATA
 *	00000080	CHANGE_ACTOR_BASE_FULLNAME
 *
 *	npc
 *	00000200	CHANGE_NPC_SKILLS
 *	00000400	CHANGE_NPC_COMBATSTYLE
 *	00000800	CHANGE_NPC_FAME
 *
 *	creature
 *	00000200	CHANGE_CREATURE_SKILLS
 *	00000400	CHANGE_CREATURE_COMBATSTYLE
 *
 *	dialog
 *	10000000	CHANGE_TOPIC_SAIDONCE
 *
 *	package
 *	10000000	CHANGE_PACKAGE_NEVER_RUN
 *	08000000	CHANGE_PACKAGE_WAITING
 *
 *	cell
 *	00000008	CHANGE_CELL_FLAGS
 *	00000006	CHANGE_CELL_CREATED	### note: two flags?!
 *	10000000	CHANGE_CELL_SEENDATA
 *	08000000	CHANGE_CELL_DETACHTIME
 *	00000020	CHANGE_CELL_OWNERSHIP
 *	00000010	CHANGE_CELL_FULLNAME
 *	01000000	CHANGE_CELL_PATHGRID_MODIFIED
 *
 *	faction
 *	00000004	CHANGE_FACTION_FLAGS
 *	00000008	CHANGE_FACTION_REACTIONS
 *
 *	book
 *	00000004	CHANGE_BOOK_TEACHES_SKILL
 *
 *	refr (object, character or creature)
 *	00000002	CHANGE_CREATED_NEW_REFERENCE
 *	80000000	CHANGEFLAG_REFR_CELL_CHANGED
 *	00000004	CHANGE_REFR_MOVE
 *	00000008	CHANGE_REFR_HAVOK_MOVE
 *	00000800	CHANGEFLAG_REFR_HAD_HAVOK_MOVE_FLAG
 *	00800000	CHANGE_REFR_OBLIVION_FLAG
 *	02000000	CHANGE_REFR_ANIMATION
 *	00000010	CHANGE_REFR_SCALE
 *	00000020	CHANGE_REFR_ALL_EXTRA
 *	40000000	CHANGE_REFR_DISABLE_STATE
 *	10000000	CHANGE_REFR_EXTRA_LEVELED_CREATURE
 *	08000000	CHANGE_REFR_INVENTORY
 *	04000000	CHANGE_REFR_EXTRA_SCRIPT
 *	01000000	CHANGE_REFR_EXTRA_SAVEDMOVEMENTDATA
 *
 *	object refr
 *	00000400	CHANGE_MAPMARKER_EXTRA_FLAGS
 *	00400000	CHANGE_OBJECT_EXTRA_FURNITURE_MARKER
 *	00200000	CHANGE_OBJECT_EXTRA_MAGIC
 *	00000040	CHANGE_OBJECT_EXTRA_LOCK
 *	00000080	CHANGE_OBJECT_EXTRA_OWNER
 *	00000100	CHANGE_OBJECT_EXTRA_GLOBAL
 *	00000200	CHANGE_OBJECT_EXTRA_RANK
 *	00020000	CHANGEFLAG_OBJECT_DROPPED_NON_QUEST_ITEM
 *	00010000	CHANGE_OBJECT_EMPTY
 *
 *	object refr to DOOR
 *	00100000	CHANGE_DOOR_EXTRA_TELEPORT
 *	00080000	CHANGE_DOOR_OPEN_STATE
 *	00040000	CHANGE_DOOR_OPEN_DEFAULT_STATE
 ***/

enum FormType
{
	kFormType_None = 0,
	kFormType_TES4,
	kFormType_Group,
	kFormType_GMST,
	kFormType_Global,
	kFormType_Class,
	kFormType_Faction,
	kFormType_Hair,
	kFormType_Eyes,
	kFormType_Race,
	kFormType_Sound,
	kFormType_Skill,
	kFormType_Effect,
	kFormType_Script,
	kFormType_LandTexture,
	kFormType_Enchantment,
	kFormType_Spell,		// 10
	kFormType_BirthSign,
	kFormType_Activator,
	kFormType_Apparatus,
	kFormType_Armor,
	kFormType_Book,
	kFormType_Clothing,
	kFormType_Container,
	kFormType_Door,
	kFormType_Ingredient,
	kFormType_Light,
	kFormType_Misc,
	kFormType_Stat,	// ???
	kFormType_Grass,
	kFormType_Tree,
	kFormType_Flora,
	kFormType_Furniture,	// 20
	kFormType_Weapon,
	kFormType_Ammo,
	kFormType_NPC,
	kFormType_Creature,
	kFormType_LeveledCreature,
	kFormType_SoulGem,
	kFormType_Key,
	kFormType_AlchemyItem,
	kFormType_SubSpace,
	kFormType_SigilStone,
	kFormType_LeveledItem,
	kFormType_SNDG,
	kFormType_Weather,
	kFormType_Climate,
	kFormType_Region,
	kFormType_Cell,			// 30
	kFormType_REFR,
	kFormType_ACHR,
	kFormType_ACRE,
	kFormType_PathGrid,
	kFormType_WorldSpace,
	kFormType_Land,
	kFormType_TLOD,	// tile low lod?
	kFormType_Road,
	kFormType_Dialog,
	kFormType_DialogInfo,
	kFormType_Quest,
	kFormType_Idle,
	kFormType_Package,
	kFormType_CombatStyle,
	kFormType_LoadScreen,
	kFormType_LeveledSpell,	// 40
	kFormType_ANIO,
	kFormType_WaterForm,
	kFormType_EffectShader,
	kFormType_TOFT
};

class TESObjectLAND;
class TESPathGrid;
class PlayerCharacter;
class TESWorldSpace;
class EnchantmentItem;
class BSFaceGenNiNode;
class TESObjectLIGH;
class TESEffectShader;
class TESSound;
class TESObjectACTI;
class Script;
class TESCombatStyle;
struct BoundObjectListHead;
class TESNPC;
class TESRace;
class Character;
class FaceGenUndo;
struct ModEntry;
class IngredientItem;
class TESFullName;
class TESObjectANIO;
class TESLoadScreen;
class TESObjectREFR;
class TESIdleForm;
struct ScriptEventList;
struct QuestStageItem;
class Actor;
class TESGrassAreaParam;
class TESWaterForm;

// member fn addresses
#if OBLIVION
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 kTESObjectCELL_SetDetachTimeAddr = 0x004CA9C0;
	static const UInt32 kTESContainer_CtorAddr = 0x00469690;
	static const UInt32 kTESContainer_DtorAddr = 0x0046A010;
	static const UInt32 kTESLeveledList_CalcItemForContainerAddr = 0x0046CDE0;
#else
#error unsupported oblivion version
#endif
#endif

// 004
class BaseFormComponent
{
public:
	BaseFormComponent();
	~BaseFormComponent();

	virtual void	Destructor(void);	// 00
	virtual void	Unk_01(void);
	virtual void	CopyFromBase(BaseFormComponent * component);

	//	void		** _vtbl;	// 000
};

struct TrackingData
{
	UInt16  date;               // 00 low byte is day of month, high byte is number of months with 1 = Jan. 2003 (Decembers are a little weird)
	UInt8   lastUser;           // 02 userID that last had this form checked out
	UInt8   currentUser;        // 03 userID that has this form checked out
};

//Game / Editor
// 018 / 024
class TESForm : public BaseFormComponent
{
public:
	TESForm();
	~TESForm();

	enum
	{
		kModified_FormFlags =	0x00000001,
			// CHANGE_FORM_FLAGS
			// can change 0x00112860 for TESObjectREFR, 0x0C20 for others
			// UInt32	flags;
	};

	enum
	{
		// ### need to figure out which objects these belong to
		kModified_GoldValue = 0x08,
		kModified_Name = 0x80
	};

	// TODO: determine which of these are in BaseFormComponent
	virtual bool	Unk_03(BaseFormComponent * arg);	// return false if equal - compares names, typeID, flags
	virtual void	Destroy(bool Dealloc);	// delete form, pass true to deallocate?
	virtual void	Unk_05(void);	// destroy form components
	virtual void	Unk_06(void);	// call unk01 on form components
	virtual bool	LoadForm(ModEntry* modEntry);
	virtual void	Unk_08(void);	// calls load form
	virtual void	Unk_09(void);	// save form
	virtual void	Unk_0A(void);	// calls save form
	virtual void	Unk_0B(void);
	virtual void	Unk_0C(void);
	virtual void	Unk_0D(void);
	virtual void	Unk_0E(void);
	virtual void	Unk_0F(void);
	virtual void	MarkAsModified(UInt32 mask);	// 10
	virtual void	ClearModified(UInt32 mask);
	virtual void	Unk_12(void);
	virtual UInt32	GetSaveSize(UInt32 modifiedMask);
	virtual void	SaveGame(UInt32 modifiedMask);	// output buffer not passed as param, accessed indirectly via g_createdBaseObjList
	virtual void	LoadGame(UInt32 modifiedMask, UInt32 unk);
	virtual void	Unk_16(void);	// post-load fixup (convert refids in to pointers, etc)
	virtual void	Unk_17(void);
	virtual void	Unk_18(void);
	virtual void	Unk_19(void);
	virtual void	Unk_1A(void);
	virtual void	DoPostFixup(void);	// initialize form after other forms loaded
	virtual void	Unk_1C(void);
	virtual void	GetDescription(BSStringT * dst);
	virtual void	Unk_1E(void);
	virtual void	Unk_1F(void);
	virtual void	Unk_20(void);	// 20
	virtual void	Unk_21(void);
	virtual void	Unk_22(void);
	virtual void	Unk_23(void);
	virtual void	Unk_24(UInt8 arg);
	virtual void	SetQuestItem(bool isQuestItem);
	virtual void	Unk_26(void);
	virtual void	Unk_27(void);
	virtual void	Unk_28(void);
	virtual bool	Unk_29(void);
	virtual bool	Unk_2A(void);	// return true if the form is a reference?
	virtual bool	Unk_2B(void);
	virtual void	Unk_2C(void);
	virtual void	CopyFrom(TESForm * form);
	virtual bool	Compare(TESForm * form);
	virtual void	Unk_2F(void);
	virtual void	Unk_30(void);	// 30
	virtual void	Unk_31(void);
	virtual void	Unk_32(void);
	virtual bool	Unk_33(TESObjectREFR* refr0, TESObjectREFR* refr1, UInt32 unk2); // related to activate, refr1 is activating refr, refr0 is a reference to this TESForm being activated, seen unk2 == 0
	virtual const char *	GetName(void);	// not sure which objects this works on, doesn't seem to work on player or random objects
	virtual const char *	GetEditorName(void);	// returns nothing at run-time
	virtual void	SetName(const char * name);

	struct EditorData
	{
		BSStringT		editorID;
		TrackingData		unk08;
	};

	enum
	{
		kFormFlags_FromMaster           = 0x00000001,   // form is from an esm file
        kFormFlags_FromActiveFile       = 0x00000002,   // form is overriden by active mod or save file
        kFormFlags_Loaded				= 0x00000004,   // ?? (from OBSE ModEntry defininition)
        kFormFlags_Linked               = 0x00000008,   // set after formids have been resolved into TESForm*
        kFormFlags_Deleted              = 0x00000020,   // set on deletion, not saved in CS, probably game as well
        kFormFlags_BorderRegion         = 0x00000040,   // ?? (from TES4Edit)
        kFormFlags_TurnOffFire          = 0x00000080,   // ?? (from TES4Edit)
        kFormFlags_CastShadows          = 0x00000200,   // ?? (from TES4Edit)
		kFormFlags_QuestItem            = 0x00000400,   // aka Quest Item, Persistent Reference, Essenstial Actor
		kFromFlags_Essential            = kFormFlags_QuestItem, 
        kFormFlags_InitiallyDisabled    = 0x00000800,   // ?? (from TES4Edit)
        kFormFlags_Ignored              = 0x00001000,   // record is not loaded by CS, perhaps game as well
        kFormFlags_Temporary            = 0x00004000,   // not saved in CS, probably game as well
        kFormFlags_VisibleWhenDistant   = 0x00008000,   // ?? (from TES4Edit)
        kFormFlags_OffLimits            = 0x00020000,   // ?? (from TES4Edit) //Offlimits for TESObjectCELL
        kFormFlags_Compressed           = 0x00040000,   // ?? (from TES4Edit)
        kFormFlags_CantWait             = 0x00080000,   // ?? (from TES4Edit) //For TESObjectCELL can't wait. Unknown for others TESForm
        kFormFlags_IgnoresFriendlyHits  = 0x00100000,
   		kFormFlags__SavedInRecord       = 0x000A8EE0,    // flag bits copied to record during serialization (from shadeMe)
    };

	struct ModReferenceList
	{
		void				* data;		// ###TODO fix cyclic header dependency to use ModEntry::Data* directly
		ModReferenceList	* next;

		void				* Info() const	{ return data; }
		ModReferenceList	* Next() const	{ return next; }
	};

	UInt8	typeID;					// 004
	UInt8	typeIDPad[3];			// 005
	UInt32	flags;					// 008
										// 00000040 - something (TESForm_Fn26)
										// 00000400 - quest item
										// 00004000 - temporary?
										// 00010000 - something (TESForm_Fn27)
										// 00020000 - something (TESForm_Fn28)
										// 00080000 - something (sub_464A60)
										// 00100000 - something (sub_464A30)
	UInt32	refID;					// 00C
	EDITOR_SPECIFIC(EditorData editorData);
	ModReferenceList	modRefList;	// 010 / 01C

	bool	IsQuestItem() const;
	bool	IsCloned() const;
	bool	SupportsSimpleModel() const;
	UInt8	GetModIndex();
	void	MarkAsTemporary(void);
	bool	IsReference()	{	 return typeID >= kFormType_REFR && typeID <= kFormType_ACRE;	}
	TESForm *	TryGetREFRParent(void);
	TESFullName* GetFullName();
	const char* GetEditorID();
	bool	IsInventoryObject() const;
};

typedef Visitor<TESForm::ModReferenceList, TESForm> ModRefListVisitor;

class TESObject : public TESForm
{
public:
	TESObject();
	~TESObject();

	virtual void	Unk_37(void);
	virtual void	Unk_38(void);
	virtual void	Unk_39(void);
	virtual void	Unk_3A(void);
	virtual void	Unk_3B(void);
	virtual void	Unk_3C(void);
	virtual void	Unk_3D(void);
	virtual void	Unk_3E(void);
	virtual void	Unk_3F(void);
	virtual void	Unk_40(void);
	virtual void	Unk_41(void);
	virtual void	Unk_42(void);
	virtual void	Unk_43(void);
	virtual void	Unk_44(void);

	// 018
};

// 24 / 58
class TESBoundObject : public TESObject
{
public:
	TESBoundObject();
	~TESBoundObject();

	virtual void	Unk_45(void);
	virtual void	Unk_46(void);
	virtual void	Unk_47(void);

	BoundObjectListHead*	head;		// 018 / 24
	TESBoundObject*			prev;		// 01C / 28
	TESBoundObject*			next;		// 020 / 2C
#if !OBLIVION
	TESBoundObject*		prevEx;			// / 30
	void*				unk34;			// / 34
	UInt32				unk38;			// / 38
	float				center[3];		// / 3C filled in as requested
	float				extents[3];		// / 48 filled in as requested
	UInt32				unk54;			// / 54
#endif
};

#if !OBLIVION
STATIC_ASSERT(sizeof(TESBoundObject) == 0x58);
#endif

// 24
class TESBoundTreeObject : public TESBoundObject
{
public:
	TESBoundTreeObject();
	~TESBoundTreeObject();
};

// 24
class TESBoundAnimObject : public TESBoundObject
{
public:
	TESBoundAnimObject();
	~TESBoundAnimObject();

	// size limited by TESSound
};

class TESSoundFile : public BaseFormComponent
{
public:
	TESSoundFile();
	~TESSoundFile();

	BSStringT	fileName;	// 004
	BSStringT	editorID;	// 00C
};

/**** components ****/

struct RGBA {
	UInt8	r;
	UInt8	g;
	UInt8	b;
	UInt8	a;

	void Set(UInt8 _r, UInt8 _g, UInt8 _b) {
		r = _r;
		g = _g;
		b = _b;
	}
	RGBA() : r(0), g(0), b(0), a(0) {}
};

// C
class TESFullName : public BaseFormComponent
{
public:
	TESFullName();
	~TESFullName();

	BSStringT	name;		// 004
};

// 18
class TESModel : public BaseFormComponent
{
public:
	TESModel();
	~TESModel();

	virtual void			Unk_03(void);
	virtual void			Unk_04(void);
	virtual const char *	GetModelPath(void);
	virtual void			SetModelPath(const char* path);

	BSStringT	nifPath;	// 004
	float	editorSize;	// 00C 'size' field in editor, from MODB subrecord. Only present for objects defined in Oblivion.esm?
	UInt8	unk10;		// 010
	UInt8	pad11[3];
	void	* unk14;	// 014

	void SetPath(const char* newPath)	{	nifPath.Set(newPath);	}
};

// C
class TESScriptableForm : public BaseFormComponent
{
public:
	TESScriptableForm();
	~TESScriptableForm();

	Script	* script;	// 004
	UInt8	unk1;		// 008
	UInt8	pad[3];		// 009
};

// 008
class TESValueForm : public BaseFormComponent
{
public:
	enum
	{
		kModified_GoldValue =	0x00000008,
			// UInt32	value
	};

	TESValueForm();
	~TESValueForm();

	UInt32	value;
};

// 008
class TESWeightForm : public BaseFormComponent
{
public:
	TESWeightForm();
	~TESWeightForm();

	float	weight;
};

// 008
class TESHealthForm : public BaseFormComponent
{
public:
	TESHealthForm();
	~TESHealthForm();

	UInt32	health;
};

// 008
class TESAttackDamageForm : public BaseFormComponent
{
public:
	TESAttackDamageForm();
	~TESAttackDamageForm();

	UInt16	damage;
	UInt16	unk0;	// bitmask? perhaps 2 UInt8s?
};

// 008 / 010
class TESDescription : public BaseFormComponent
{
public:
	virtual void Unk_3(void);
	virtual const char * GetText(TESForm* parentForm, UInt32 recordCode); // books, classes, and most others: arg0:NULL, arg1:'DESC'
																		  // levelup quotes: arg0:TESSkill, arg1:'(A/J/E/M)NAM'

	TESDescription();
	~TESDescription();

	EDITOR_SPECIFIC(BSStringT description);
	UInt32	formDiskOffset;	// 04 / 0C

	const char* GetDescription() { return GetText(0, 0x43534544); }	// this will not work for skill levelup quotes
};

// 00C / 018
class TESTexture : public BaseFormComponent
{
public:
	TESTexture();
	~TESTexture();

	struct EditorData {
		UInt32	unk00;		// init'd to 0
		UInt32	unk04;		// init'd to 0x414
		UInt32	unk08;		// init'd to 0x422
	};

	BSStringT ddsPath;		// 04
	EDITOR_SPECIFIC(EditorData editorData);
};

// 00C
class TESIcon : public TESTexture
{
public:
	TESIcon();
	~TESIcon();

	void SetPath(const char* newPath)	{	ddsPath.Set(newPath);	}
};

// 10
class TESReactionForm : public BaseFormComponent
{
public:
	enum
	{
		kModified_Reactions =	0x00000008,
			// CHANGE_FACTION_REACTIONS
			//	UInt16	numChanges;
			//	struct
			//	{
			//		UInt32	unk;
			//		UInt32	unk;
			//	} changes[numChanges];
	};

	struct ReactionInfo
	{
		TESForm*	target;
		SInt32	reaction;
	};

	TESReactionForm();
	~TESReactionForm();

	typedef tList<ReactionInfo> TargetList;

	TargetList	targetList;		// 004
	UInt8	unk2;		// 00C
	UInt8	pad[3];		// 00D

	SInt32 ReactionFor(TESForm* pTarget) const;
};

class FindTargetByAddress
{
	void* m_pAddr;
public:
	FindTargetByAddress(void* pAddr) : m_pAddr(pAddr) {}
	bool Accept(void* pTryAddr) {
		if (pTryAddr == m_pAddr) {
			return true;
		} else {
			return false;
		}
	}
};

// 14
class TESSpellList : public BaseFormComponent
{
public:
	enum
	{
		kModified_BaseSpellList =	0x00000020,
			// CHANGE_ACTOR_BASE_SPELLLIST
			//	UInt16	numSpells;
			//	UInt32	spells[numSpells];
	};

	TESSpellList();
	~TESSpellList();

	struct Entry
	{
		TESForm	* type;
		Entry	* next;

		TESForm* Info() const { return type; }
		Entry* Next() const { return next; }

		void SetNext(Entry* nextEntry) { next = nextEntry; }
		void Delete();
		void DeleteHead(Entry* replaceWith);
	};

	Entry	spellList;			// 004
	Entry	leveledSpellList;	// 00C

	// return the nth spell
	TESForm* GetNthSpell(UInt32 whichSpell) const;

	// removes all spells and returns how many spells were removed
	UInt32 RemoveAllSpells();
};

typedef Visitor<TESSpellList::Entry, TESForm> SpellListVisitor;

// C?
class TESAttributes : public BaseFormComponent
{
public:
	enum
	{
		kModified_BaseAttributes =		0x00000008,
			// CHANGE_ACTOR_BASE_ATTRIBUTES
			// UInt8	attr[8];
	};

	TESAttributes();
	~TESAttributes();

	UInt8	attr[8];
};

// 10
class TESContainer : public BaseFormComponent
{
public:
#if OBLIVION
	MEMBER_FN_PREFIX(TESContainer);
	DEFINE_MEMBER_FN(Constructor, void, kTESContainer_CtorAddr, void);
	DEFINE_MEMBER_FN(Destructor, void, kTESContainer_DtorAddr, void);
#endif

	TESContainer();
	~TESContainer();

	struct Data
	{
		SInt32	count;
		TESForm	* type;
	};

	struct Entry
	{
		Data	* data;
		Entry	* next;

		Data* Info() const { return data; }
		Entry* Next() const { return next; }
	};

	UInt8	type;					// 004
	UInt8	typePad[3];				// 005
	Entry	list;					// 008

	Data* DataByType(TESForm* type) const;
};

typedef Visitor<TESContainer::Entry, TESContainer::Data> ContainerVisitor;

class TESFaction;

// 20
class TESActorBaseData : public BaseFormComponent
{
public:
	enum
	{
		kModified_ActorBaseFlags =	0x00000010,
			// CHANGE_ACTOR_BASE_DATA
			// UInt32 flags;

		kModified_BaseFactions =	0x00000040,
			// CHANGE_ACTOR_BASE_FACTIONS
			//	UInt16	numFactions;
			//	struct
			//	{
			//		UInt32	unk0;
			//		UInt32	unk1;
			//	} factions[numFactions];
	};

	TESActorBaseData();
	~TESActorBaseData();

	enum
	{
		kFlag_IsFemale =				0x00000001,	// For NPCs and Characters
		kFlag_IsCreatureBiped =			0x00000001, // For creatures
		kFlag_IsEssential =				0x00000002,
		kFlag_CreatureWeaponAndShield =	0x00000004,	// for creatures
		kFlag_Respawn =					0x00000008,
		kFlag_CreatureSwims =			0x00000010, // for creatures
		kFlag_CreatureFlies =			0x00000020, // for creatures
		kFlag_CreatureWalks =			0x00000040,	// for creatures
		kFlag_PCLevelOffset =			0x00000080,
		kFlag_CreatureHasSounds =		0x00000100,	// for creatures, if not set has a TESCreature* soundBase
		kFlag_NoLowProc =				0x00000200,
		kFlag_NoRumors =				0x00002000,
		kFlag_Summonable =				0x00004000,
		kFlag_NoPersuasion =			0x00008000,
		kFlag_CreatureNoHead =			0x00008000,	// for creatures
		kFlag_CreatureNoRightArm =		0x00010000, // for creatures
		kFlag_CreatureNoLeftArm =		0x00020000, // for creatures
		kFlag_CreatureNoCombatInWater =	0x00040000, // for creatures
		kFlag_CanCorpseCheck =			0x00100000
	};

	struct FactionListData
	{
		TESFaction	* faction;
		UInt8		rank;
		UInt8		pad[3];
	};

	struct FactionListEntry
	{
		FactionListData		* data;
		FactionListEntry	* next;

		FactionListData* Info() const { return data; }
		FactionListEntry* Next() const { return next; }
	};

	UInt32	flags;	// 004 - has flags
						// 00000002 - is essential
	UInt16	baseSpellPts;	// 008
	UInt16	fatigue;	// 00A
	UInt16	barterGold;	// 00C
	SInt16	level;	// 00E absolute level, or level offset if PCLevelOffset flag set
	UInt16	minLevel;	// 010	if PCLevelOffset
	UInt16	maxLevel;	// 012
	UInt32	unk7;	// 014
	FactionListEntry	factionList;	//018

	bool IsFlagSet(UInt32 flag) const {
		return (flags & flag) == flag;
	}

	void SetFlag(UInt32 flag, bool bMod) {
		flags = bMod ? (flags | flag) : (flags & ~flag);
		TESForm* baseForm = (TESForm*)Oblivion_DynamicCast(this, 0, RTTI_BaseFormComponent, RTTI_TESForm, 0);
		baseForm->MarkAsModified(kModified_ActorBaseFlags);
	}

	bool IsEssential() const
		{	return IsFlagSet(kFlag_IsEssential);	}
	void SetEssential(bool bEssential)
		{	SetFlag(kFlag_IsEssential, bEssential);	}
	bool IsFemale() const
		{	return IsFlagSet(kFlag_IsFemale);	}
	void SetFemale(bool bFemale)
		{	SetFlag(kFlag_IsFemale, bFemale);	}
	bool IsRespawning() const
		{	return IsFlagSet(kFlag_Respawn);	}
	void SetRespawning(bool bRespawn)
		{	SetFlag(kFlag_Respawn, bRespawn);	}
	bool IsPCLevelOffset() const
		{	return IsFlagSet(kFlag_PCLevelOffset);	}
	void SetPCLevelOffset(bool bPCLevelOffset, UInt32 min = -1, UInt32 max = -1)
	{
		SetFlag(kFlag_PCLevelOffset, bPCLevelOffset);
		if (bPCLevelOffset)
		{
			minLevel = (min != -1) ? min : minLevel;
			maxLevel = (max != -1) ? max : maxLevel;
		}
	}
	bool HasLowLevelProcessing() const
		{	return !(IsFlagSet(kFlag_NoLowProc));	}
	void SetLowLevelProcessing(bool bLowProc)
		{	SetFlag(kFlag_NoLowProc, !bLowProc);	}
	bool HasNoRumors() const
		{	return IsFlagSet(kFlag_NoRumors);	}
	void SetNoRumors(bool bNoRumors)
		{	SetFlag(kFlag_NoRumors, bNoRumors);	}
	bool IsSummonable() const
		{	return IsFlagSet(kFlag_Summonable);	}
	void SetSummonable(bool bSumm)
		{	SetFlag(kFlag_Summonable, bSumm);	}
	bool HasNoPersuasion() const
		{	return IsFlagSet(kFlag_NoPersuasion);	}
	void SetNoPersuasion(bool bNoPers)
		{	SetFlag(kFlag_NoPersuasion, bNoPers);	}
	bool CanCorpseCheck() const
		{	return IsFlagSet(kFlag_CanCorpseCheck);	}
	void SetCanCorpseCheck(bool bCan)
		{	SetFlag(kFlag_CanCorpseCheck, bCan);	}

	SInt8 GetFactionRank(TESFaction* faction);

	bool CreatureWalks() const
		{	return IsFlagSet(kFlag_CreatureWalks);	}
	void SetCreatureWalks(bool bWalks)
		{ SetFlag(kFlag_CreatureWalks, bWalks); }
	bool CreatureFlies() const
		{ return IsFlagSet(kFlag_CreatureFlies); }
	void SetCreatureFlies(bool bFlies)
		{ SetFlag(kFlag_CreatureFlies, bFlies); }
	bool CreatureSwims() const
		{ return IsFlagSet(kFlag_CreatureSwims); }
	bool IsCreatureBiped() const
		{ return IsFlagSet(kFlag_IsCreatureBiped); }
	bool CreatureHasNoMovement() const
		{ return IsCreatureBiped() && !CreatureWalks() && !CreatureFlies() && !CreatureSwims(); }
	bool CreatureHasWeaponAndShield() const
		{ return IsFlagSet(kFlag_CreatureWeaponAndShield); }
	void SetCreatureHasWeaponAndShield(bool bUseWeapon)
		{ SetFlag(kFlag_CreatureWeaponAndShield, bUseWeapon); }
	bool CreatureHasNoHead() const
		{ return IsFlagSet(kFlag_CreatureNoHead); }
	void SetCreatureHasNoHead(bool bNoHead)
		{ return SetFlag(kFlag_CreatureNoHead, bNoHead); }
	bool CreatureHasNoLeftArm() const
		{ return IsFlagSet(kFlag_CreatureNoLeftArm); }
	void SetCreatureHasNoLeftArm(bool bNoLeftArm)
		{ SetFlag(kFlag_CreatureNoLeftArm, bNoLeftArm); }
	bool CreatureHasNoRightArm() const
		{ return IsFlagSet(kFlag_CreatureNoRightArm); }
	void SetCreatureHasNoRightArm(bool bNoRightArm)
		{ SetFlag(kFlag_CreatureNoRightArm, bNoRightArm); }
	bool CreatureNoCombatInWater() const
		{ return IsFlagSet(kFlag_CreatureNoCombatInWater); }
	void SetCreatureNoCombatInWater(bool bNoCombatInWater)
		{ SetFlag(kFlag_CreatureNoCombatInWater, bNoCombatInWater); }
};

typedef Visitor<TESActorBaseData::FactionListEntry, TESActorBaseData::FactionListData> FactionListVisitor;

class TESPackage;

// 018
class TESAIForm : public BaseFormComponent
{
public:
	enum
	{
		kModified_BaseAIData =	0x00000100,
			// CHANGE_ACTOR_BASE_AIDATA
			// UInt8	unk0;	// +0x4
			// UInt8	unk1;	// +0x5
			// UInt8	unk2;	// +0x6
			// UInt8	unk3;	// +0x7
	};

	TESAIForm();
	~TESAIForm();

	struct PackageEntry
	{
		TESPackage		* package;
		PackageEntry	* next;

		TESPackage * Info() const	{ return package; }
		PackageEntry * Next() const	{ return next; }
	};

	enum{
		kAISetting_Aggression = 0,
		kAISetting_Confidence,
		kAISetting_Energy,
		kAISetting_Responsibility,

		kAISetting_Max,
	};

	enum{
		kService_Weapons	= 1 << 0,
		kService_Armor		= 1 << 1,
		kService_Clothing	= 1 << 2,
		kService_Books		= 1 << 3,
		kService_Ingredients= 1 << 4,

		kService_Lights		= 1 << 7,
		kService_Apparatus	= 1 << 8,

		kService_Misc		= 1 << 10,
		kService_Spells		= 1 << 11,
		kService_MagicItems = 1 << 12,
		kService_Potions	= 1 << 13,
		kService_Training	= 1 << 14,

		kService_Recharge	= 1 << 16,
		kService_Repair		= 1 << 17,
	};

	UInt8	AISettings[4];			//004
	UInt32	serviceFlags;			//008
	UInt8	trainingSkill;			//00C .. = kActorVal_XXX - kActorVal_Armorer
	UInt8	trainingLevel;			//00D
	UInt16	unk00E;					//00E
	PackageEntry	packageList;	// 010

	bool OffersServices(UInt32 serviceMask)
		{	return (serviceFlags & serviceMask) == serviceMask;	}
	void SetOffersServices(UInt32 serviceMask, bool bOffersService, bool bClearServices = false)
	{			//changes aren't saved
		if (bClearServices)
			serviceFlags = 0;
		if (bOffersService)
			serviceFlags |= serviceMask;
		else
			serviceFlags &= ~serviceMask;
	}
};

typedef Visitor<TESAIForm::PackageEntry, TESPackage> PackageListVisitor;

// C
class TESAnimation : public BaseFormComponent
{
public:
	TESAnimation();
	~TESAnimation();

	struct AnimationNode {
		char* animationName;	// 04
		AnimationNode* next;	// 08

		char* Info() const { return animationName; }
		AnimationNode* Next() const { return next; }

		void SetNext(AnimationNode* nextEntry) { next = nextEntry; }
		void Delete();
		void DeleteHead(AnimationNode* replaceWith);
	};

	AnimationNode data;
};

typedef Visitor<TESAnimation::AnimationNode, char> AnimationVisitor;

// 00C (1.1)
// 014 (1.2)
class TESModelList : public BaseFormComponent
{
public:
	TESModelList();
	~TESModelList();

	struct Entry
	{
		char	* nifPath;
		Entry	* next;

		char  * Info() const	{	return nifPath;	}
		Entry * Next() const	{	return next;	}
	};

	Entry	modelList;	// 004

#if OBLIVION_VERSION >= OBLIVION_VERSION_1_2
	UInt32	unk0C;	// 00C
	UInt32	unk10;	// 010
#endif

	const Entry* FindNifPath(char* path);
	bool RemoveEntry(char* nifToRemove);
	bool AddEntry(char* nifToAdd);
};

typedef Visitor<TESModelList::Entry, char*> ModelListVisitor;

// 8
class TESRaceForm : public BaseFormComponent
{
public:
	TESRaceForm();
	~TESRaceForm();

	TESRace* race;	// 004
};

// 8
class TESQualityForm : public BaseFormComponent
{
public:
	TESQualityForm();
	~TESQualityForm();

	float	quality;	// 004
};

// 10
class TESEnchantableForm : public BaseFormComponent
{
public:
	TESEnchantableForm();
	~TESEnchantableForm();

	EnchantmentItem* enchantItem;	// 04
	UInt16	enchantment;			// 08 - the maximum charge, valid only for weapons
	UInt16	pad0A;					// 0A
	UInt32	castingType;			// 0C - init by derived class
};

// C
class TESProduceForm : public BaseFormComponent
{
public:
	TESProduceForm();
	~TESProduceForm();

	enum {
		kSeason_Spring,
		kSeason_Summer,
		kSeason_Fall,
		kSeason_Winter
	};

	IngredientItem	* ingredient;		// 004
	UInt8			harvestChance[4];	//008
};

// 10
class TESLeveledList : public BaseFormComponent
{
public:
#if OBLIVION
	MEMBER_FN_PREFIX(TESLeveledList);
	// this calcs item(s) based on level and adds them to the passed TESContainer, which is usually a temporary object
	DEFINE_MEMBER_FN(CalcItemForContainer, void, kTESLeveledList_CalcItemForContainerAddr, UInt16 level, SInt16 count, TESContainer* container);
#endif

	TESLeveledList();
	~TESLeveledList();

	enum
	{
		kFlags_CalcAllLevels=1,
		kFlags_CalcEachInCount
	};

	struct ListData
	{
		UInt16		level;
		UInt16		pad;
		union
		{
			TESForm*	form;
			UInt32		formID;	// only valid during load
		};
		UInt16		count;
	};

	struct ListEntry
	{
		ListData*	data;
		ListEntry*	next;

		ListData* Info() const { return data; }
		ListEntry* Next() const { return next; }
		void Delete();
		void DeleteHead(ListEntry* replaceWith);
		void SetNext(ListEntry* nextEntry) { next = nextEntry; }
	};

	ListEntry	list;		//004
	UInt8		chanceNone;	//00C
	UInt8		flags;		//00D
	UInt16		pad00E;

	ListData*	CreateData(TESForm* form, UInt16 level, UInt16 count);
	ListEntry*	CreateEntry(ListData* data);
	void		AddItem(TESForm* form, UInt16 level, UInt16 count);
		//adds an element to the list (sorted in ascending order by level)
	UInt32		RemoveItem(TESForm* form);
		//returns num elements removed
	void		Dump();
		//output contents to console
	TESForm*	CalcElement(UInt32 cLevel, bool useChanceNone, UInt32 levelDiff, bool noRecurse = false);
		//returns a random element from the list
	TESForm*	GetElementByLevel(UInt32 whichLevel);
		//returns first form matching whichLevel
	UInt32		RemoveByLevel(UInt32 whichLevel);
		//returns num removed
	bool		RemoveNthItem(UInt32 itemIndex);
	UInt32		GetItemIndexByForm(TESForm* form);
	UInt32		GetItemIndexByLevel(UInt32 level);
};

typedef Visitor<TESLeveledList::ListEntry, TESLeveledList::ListData> LeveledListVisitor;

/**** bases with components ****/

// 080
class TESBipedModelForm : public BaseFormComponent
{
public:
	TESBipedModelForm();
	~TESBipedModelForm();

	static UInt32 SlotForMask(UInt32 mask);
	static UInt32 MaskForSlot(UInt32 slot);

	// bit indices starting from lsb
	enum
	{
		kPart_Head =	0,
		kPart_Hair,
		kPart_UpperBody,
		kPart_LowerBody,
		kPart_Hand,
		kPart_Foot,
		kPart_RightRing,
		kPart_LeftRing,
		kPart_Amulet,
		kPart_Weapon,
		kPart_BackWeapon,
		kPart_SideWeapon,
		kPart_Quiver,
		kPart_Shield,
		kPart_Torch,
		kPart_Tail,

		kPart_Max
	};
	enum
	{
		kFlags_HidesRings = 0,
		kFlags_HidesAmulets,
		kFlags_Unk2,
		kFlags_Unk3,
		kFlags_Unk4,
		kFlags_Unk5,
		kFlags_NotPlayable,
		kFlags_HeavyArmor,
		kFlags_Unk8,
	};

	enum {
		kPath_Biped,
		kPath_Ground,
		kPath_Icon,
		kPath_Max
	};

	UInt16		partMask;			// 004
	UInt16		flags;				// 006

	// male and female models and icons
	TESModel	bipedModel[2];		// 008
	TESModel	groundModel[2];		// 038
	TESIcon		icon[2];			// 068

	UInt32 GetSlot() const;
	void SetSlot(UInt32 slot);
	bool		IsPlayable() const;
	void		SetPlayable(bool bPlayable);
	void  SetPath(const char* newPath, UInt32 whichPath, bool bfemalePath);
	const char* GetPath(UInt32 whichPath, bool bFemalePath);
};

// 0E4
class TESActorBase : public TESBoundAnimObject
{
public:
	enum
	{
		//	if(version >= 0x6D)
		//		TESForm flags
		//	TESAttributes flags
		//	TESActorBaseData flags
		//	TESSpellList flags
		//	TESAIForm flags

		kModified_BaseHealth =		0x00000004,
			// CHANGE_ACTOR_BASE_HEALTH
			// UInt32	health;

		kModified_BaseModifiers =	0x10000000,
			// CHANGE_ACTOR_BASE_MODIFIERS
			// stuff with +0xD0

		kModified_FullName =		0x00000080,
			// CHANGE_ACTOR_BASE_FULLNAME
			//	UInt8	strLen;
			//	char	name[strLen];
	};

	TESActorBase();
	~TESActorBase();

	virtual void	Unk_48(void) = 0;
	virtual void	Unk_49(void) = 0;
	virtual UInt32	GetActorValue(UInt32 avCode) = 0;
	virtual void	Unk_4B(void) = 0;
	virtual void	Unk_4C(void) = 0;
	virtual void	Unk_4D(void) = 0;
	virtual void	Unk_4E(void) = 0;
	virtual void	Unk_4F(void) = 0;

	// base classes
	TESActorBaseData	actorBaseData;	// 024
	TESContainer		container;		// 044
	TESSpellList		spellList;		// 054
	TESAIForm			aiForm;			// 068
	TESHealthForm		health;			// 080
	TESAttributes		attributes;		// 088
	TESAnimation		animation;		// 094
	TESFullName			fullName;		// 0A0
	TESModel			model;			// 0AC
	TESScriptableForm	scriptable;		// 0C4

	// 8
	struct Unk2
	{
		UInt8	unk0;	// 000
		UInt8	pad[3];	// 001
		UInt32	unk1;	// 004
	};

	struct Unk
	{
		UInt32	unk0;	// 000
		UInt32	unk1;	// 004
		Unk2	* unk2;	// 008 - alloc'd in ctor
		Unk2	* unk3;	// 00C - alloc'd in ctor
		UInt32	unk4;	// 010
	};

	// Unk::unk2 and Unk::unk3 may not be the same type, they're just the same size and have the same member sizes

	// members
	Unk		unk0;	// 0D0
};

// A8
class EffectSetting : public TESForm
{
public:
	// no changed flags (TESForm flags)

	EffectSetting();
	~EffectSetting();

	// bases
	TESModel			model;			// 018
	TESDescription		description;	// 030
	TESFullName			fullName;		// 038
	TESIcon				texture;		// 044
	UInt32				unk0[2];		// 050
	UInt32				effectFlags;	// 058
	float				baseCost;		// 05C
	UInt32				data;			// 060 bound weapon, actor value
	UInt32				school;			// 064
	UInt32				resistValue;	// 068 an actor value i.e. kActorVal_ResistFire
	UInt16				numCounters;	// 06C count of counter effects, is size of counterArray
	UInt16				pad06E;
	TESObjectLIGH*		light;			// 070
	float				projSpeed;		// 074
	TESEffectShader*	effectShader;	// 078
	TESEffectShader*	enchantEffect;	// 07C
	TESSound*			castingSound;	// 080
	TESSound*			boltSound;		// 084
	TESSound*			hitSound;		// 088
	TESSound*			areaSound;		// 08C
	float				enchantFactor;	// 090
	float				barterFactor;	// 094
	UInt32				effectCode;		// 098
	UInt32*				counterArray;	// 09C counters stored as dynamic array[numCounters] of effect codes
										//		i.e. { 1380340562, 1213420102, 1195657542 }
	UInt32				unk4[2];		// 0A0
	// 0A8

	enum {
		kEffect_IsHostile =			0x1,
		kEffect_CanRecover =		0x1 << 1,
		kEffect_IsDetrimental =		0x1 << 2,
		kEffect_MagnitudePercent =	0x1 << 3,
		kEffect_OnSelfAllowed =		0x1 << 4,
		kEffect_OnTouchAllowed =	0x1 << 5,
		kEffect_OnTargetAllowed =	0x1 << 6,
		kEffect_NoDuration =		0x1 << 7,
		kEffect_NoMagnitude =		0x1 << 8,
		kEffect_NoArea =			0x1 << 9,
		kEffect_FXPersists =		0x1 << 10,
		kEffect_ForSpellmaking =	0x1 << 11,
		kEffect_ForEnchanting =		0x1 << 12,
		kEffect_NoIngredient =		0x1 << 13,
		kEffect_UnknownF =			0x1 << 14,
		kEffect_UnknownG =			0x1 << 15,
		kEffect_UseWeapon =			0x1 << 16,
		kEffect_UseArmor =			0x1 << 17,
		kEffect_UseCreature =		0x1 << 18,
		kEffect_UseSkill =			0x1 << 19,
		kEffect_UseAttribute =		0x1 << 20,
		//Looks like: M=Ball, N=Bolt, O=Fog?
		//(DW: I don't think so - everything except SHDG [bolt] and FRDG [fog] use ball,
		//     plus there's a fourth unused type, spray)
		kEffect_UnknownM =			0x1 << 21, // ABHE, FIDG, FOAT, FOMA, FRDG, OPEN, RSHE, RSMA, SEFF, LISH, SHLD
		// DW test: (did not test 0-form-id effects) (still not every effect that uses Ball)
		/* ABAT, ABHE, ABFA, ABSK, ABSP, BABO, BACU, BAGR, BAHE, BASH, BRDN, BWAX, CALM, CHML,
		   CHRM, COCR, COHU, CUDI, CUPA, CUPO, DEMO, DGAT, DGFA, DGHE, DIAR, DIWE, DRSP, DSPL,
		   DTCT, FIDG, FOMM, FOSP, FRDG, REHE, RSMA, SABS, SEFF, SHDG, SHLD, STMA, TURN, ZSKE */
		kEffect_UnknownN =			0x1 << 22, // none
		kEffect_UnknownO =			0x1 << 23, // POSN
		kEffect_UseActorValue =		0x1 << 24,
		kEffect_UnknownQ =			0x1 << 25, // FRDG
		kEffect_UnknownR =			0x1 << 26, // FRDG, SEFF
		kEffect_NoHitEffect =		0x1 << 27,
		kEffect_UnknownT =			0x1 << 28, // REAN, VAMP - uh... makes things undead?
		kEffect_UnknownU =			0x1 << 29, // DGHE, FIDG, LIDG - with FRDG, would be health damage, but FRDG does not have this
		kEffect_UnknownV =			0x1 << 30, // CALM, COCR, COHU, DEMO, FRNZ, TURN - AI effects? but not all of them...
											   // (also, I did not get FRNZ)
		kEffect_UnknownW =			0x1 << 31, // DARK, DTCT, LGHT, TELE - other than TELE, light effects?
//		kEffect_NoRecast,
	};

	bool ForSpellmaking() const;
	bool ForEnchanting() const;
	bool IsHostile() const;
	bool IsDetrimental() const;
	bool CanRecover() const;
	bool MagnitudeIsPercent() const;
	bool FXPersists() const;
	bool OnSelfAllowed() const;
	bool OnTouchAllowed() const;
	bool OnTargetAllowed() const;
	bool NoDuration() const;
	bool NoMagnitude() const;
	bool NoArea() const;
	bool NoIngredient() const;
	bool UseWeapon() const;
	bool UseArmor() const;
	bool UseCreature() const;
	bool UseSkill() const;
	bool UseAttribute() const;
	bool UseOtherActorValue() const;
	bool NoRecast() const;
	bool NoHitEffect() const;

	void SetFlag(UInt32 flag, bool bMod) {
		effectFlags = bMod ? (effectFlags | flag) : (effectFlags & ~flag);	}

	bool IsFlagSet(UInt32 flag) {
		return (effectFlags & flag) == flag;	}

	bool MatchesType(const std::string& typeStr) const;
	bool IsBoundItemEffect() const;
	bool IsSummonEffect() const;

	static UInt32 RefIdForC(UInt32 effectCode);
	static EffectSetting* EffectSettingForC(UInt32 effectCode);

	bool GetEffectChars(char* out) const;

	enum {
		kEffect_Alteration = 0,
		kEffect_Conjuration,
		kEffect_Destruction,
		kEffect_Illusion,
		kEffect_Mysticism,
		kEffect_Restoration
	};
};

// 24
class EffectItem {
public:
	EffectItem();
	~EffectItem();

	enum {
		kRange_Self = 0,
		kRange_Touch,
		kRange_Target,
	};

	struct ScriptEffectInfo
	{
		UInt32 scriptRefID;
		UInt32 school;
		BSStringT effectName;
		UInt32 visualEffectCode;
		UInt8 isHostile;
		UInt8 padIsHostile[3];

		void SetName(const char* name);
		void SetSchool(UInt32 school);
		void SetVisualEffectCode(UInt32 code);
		void SetIsHostile(bool bIsHostile);
		bool IsHostile() const;
		void SetScriptRefID(UInt32 refID);

		ScriptEffectInfo* Clone() const;
		void CopyFrom(const ScriptEffectInfo* from);
		static ScriptEffectInfo* Create();
	};

	// mising flags
	UInt32	effectCode;			// 00
	UInt32	magnitude;			// 04
	UInt32	area;				// 08
	UInt32	duration;			// 0C
	UInt32	range;				// 10
	UInt32	actorValueOrOther;	// 14
	ScriptEffectInfo	* scriptEffectInfo;	// 18
	EffectSetting	* setting;	// 1C
	float	cost;				// 20 on autocalc items this seems to be the cost

	bool HasActorValue() const;
	UInt32 GetActorValue() const;
	bool IsValidActorValue(UInt32 actorValue) const;
	void SetActorValue(UInt32 actorValue);

	bool IsScriptedEffect() const;
	UInt32 ScriptEffectRefId() const;
	UInt32 ScriptEffectSchool() const;
	UInt32 ScriptEffectVisualEffectCode() const;
	bool IsScriptEffectHostile() const;

	void GetQualifiedName(char* outBuf);		// i.e. "Absorb Strength" instead of "Absorb Attribute"
	EffectItem* Clone() const;
	void CopyFrom(const EffectItem* from);
	static EffectItem* Create(UInt32 mgefCode = 0);
	static EffectItem* ProxyEffectItemFor(UInt32 effectCode);

	bool operator<(EffectItem*rhs) const;
	// return the magicka cost of this effect item
	// adjust for skill level if actorCasting is used
	float MagickaCost(TESForm* actorCasting = NULL) const;

	void SetMagnitude(UInt32 magnitude);
	void ModMagnitude(float modBy);
	void SetArea(UInt32 area);
	void ModArea(float modBy);
	void SetDuration(UInt32 duration);
	void ModDuration(float modBy);
	void SetRange(UInt32 range);
	bool IsHostile() const;
};

// 10
class EffectItemList
{
public:
	EffectItemList();
	~EffectItemList();

	struct Entry {
		EffectItem* effectItem;
		Entry* next;

		EffectItem* Info() const { return effectItem; }
		Entry* Next() const { return next; }
	};

	virtual float	GetMagickaCost(Actor* actor);
	virtual UInt32	Unk_01(void);					// returns NULL in base class impl
	virtual UInt32	GetMasteryLevel(void);

	// void			** _vtbl;				// 000
	Entry			effectList;				// 004 BSSimpleList<EffectItem>, from which this class is derived
	UInt32			hostileEffectCount;		// 00C missing from original OBSE class definition

	EffectItem* ItemAt(UInt32 whichItem);
	UInt32 AddItem(EffectItem* effectItem);
	UInt32 AddItemCopy(EffectItem* effectItem);
	UInt32 CopyItemFrom(EffectItemList& fromList, UInt32 whichItem);
	bool RemoveItem(UInt32 whichItem);
	bool HasNonHostileItem() const;
	bool HasHostileItem() const;
	UInt32 CountItems() const;
	UInt32 CountHostileItems() const;
	EffectItem* ItemWithHighestMagickaCost() const;
	UInt32 GetMagickaCost(TESForm* form = NULL) const;
	const char* GetNthEIName(UInt32 whichEffect) const;
	void RemoveAllItems();
	float CalcGoldValue();
	UInt32 GetSchoolSkillAV();
	UInt32 GetSchoolCode();		// returns one of EffectSetting::kEffect_XXX (values 0 thru 5)
};

typedef Visitor<EffectItemList::Entry, EffectItem> EffectItemVisitor;
EffectItemList* GetEffectList(TESForm* form);

// 1C
class MagicItem : public TESFullName
{
public:
	MagicItem();
	~MagicItem();

	EffectItemList	list;	// 00C

	enum EType{
		kType_None = 0,
		kType_Spell = 1,
		kType_Enchantment = 2,
		kType_Alchemy = 3,
		kType_Ingredient = 4,
	};
	EType Type() const;
};

// 040
class MagicItemObject : public TESBoundObject
{
public:
	MagicItemObject();
	~MagicItemObject();

	// base
	MagicItem	magicItem;	// 024
};

// 034
class MagicItemForm : public TESForm
{
public:
	MagicItemForm();
	~MagicItemForm();

	// base
	MagicItem	magicItem;	// 018
};

/**** forms ****/

class TESGlobal : public TESForm
{
public:
	// no changed flags (TESForm flags)

	TESGlobal();
	~TESGlobal();

	enum
	{
		kType_Short =	's',
		kType_Long =	'l',
		kType_Float =	'f'
	};

	BSStringT	name;		// 018
	UInt8	type;		// 020
	UInt8	pad21[3];	// 021
	float	data;		// 024
};

class TESClass : public TESForm
{
public:
	// no changed flags (TESForm flags)

	enum
	{
		kFlag_Playable =	1 << 0,
		kFlag_Guard =		1 << 1,
	};

	enum
	{
		kBuySell_Weapons =		1 << 0,
		kBuySell_Armor =		1 << 1,
		kBuySell_Clothing =		1 << 2,
		kBuySell_Books =		1 << 3,
		kBuySell_Ingredients =	1 << 4,
		kBuySell_Unk5 =			1 << 5,
		kBuySell_Unk6 =			1 << 6,
		kBuySell_Lights =		1 << 7,
		kBuySell_Apparatus =	1 << 8,
		kBuySell_Unk9 =			1 << 9,
		kBuySell_Misc =			1 << 10,
		kBuySell_Spells =		1 << 11,
		kBuySell_MagicItems =	1 << 12,
		kBuySell_Potions =		1 << 13,

		kService_Training =		1 << 14,
		kService_Unk15 =		1 << 15,
		kService_Recharge =		1 << 16,
		kService_Repair =		1 << 17,
	};

	enum {
		eSpec_Combat = 0,
		eSpec_Magic,
		eSpec_Stealth,

		eSpec_MAX
	};

	TESClass();
	~TESClass();

	// child classes
	TESFullName			fullName;			// 018
	TESDescription		description;		// 024
	TESTexture			texture;			// 02C

	// members
	UInt32				attributes[2];		// 038
	UInt32				specialization;		// 040
	UInt32				majorSkills[7];		// 044
	UInt32				classFlags;				// 060
	UInt32				buySellServices;	// 064
	UInt8				skillTrained;		// 068
	UInt8				trainingLevel;		// 069
	UInt8				pad6A[2];			// 06A

	bool IsMajorSkill(UInt32 skill) const;
	bool IsPlayable() const { return (classFlags & kFlag_Playable) == kFlag_Playable; }
};

class TESFaction : public TESForm
{
public:
	enum
	{
		// TESForm flags

		// TESReactionForm flags

		kModified_FactionFlags =	0x00000004
			// CHANGE_FACTION_FLAGS
			// UInt8	flags;
	};

	enum
	{
		kFactionFlags_HiddenFromPC =	0x00000001,
		kFactionFlags_Evil =			0x00000002,
		kFactionFlags_SpecialCombat =	0x00000004
	};

	struct RankData
	{
		BSStringT		maleRank;
		BSStringT		femaleRank;
		TESTexture	insignia;
	};

	struct RankEntry
	{
		RankData	* data;
		RankEntry	* next;

		RankData  * Info() const	{ return data; }
		RankEntry * Next() const	{ return next; }
	};

	TESFaction();
	~TESFaction();

	// child classes
	TESFullName		fullName;	// 018
	TESReactionForm	reaction;	// 024

	// members
	UInt8		factionFlags;		// 034
	UInt8		pad35[3];			// 035
	float		crimeGoldMultiplier;// 038
	RankEntry	ranks;				// 03C

	bool IsFlagSet(UInt32 flag) {
		return (factionFlags & flag) != 0;
	}
	void SetFlag(UInt32 flag, bool bMod) {
		factionFlags = bMod ? (factionFlags | flag) : (factionFlags & ~flag);
		MarkAsModified(kModified_FactionFlags);
	}
	bool IsHidden()
		{	return IsFlagSet(kFactionFlags_HiddenFromPC);	}
	bool IsEvil()
		{	return IsFlagSet(kFactionFlags_Evil);	}
	bool HasSpecialCombat()
		{	return IsFlagSet(kFactionFlags_SpecialCombat);	}
	void SetHidden(bool bHidden)
		{	SetFlag(kFactionFlags_HiddenFromPC, bHidden);	}
	void SetEvil(bool bEvil)
		{	SetFlag(kFactionFlags_Evil, bEvil);	}
	void SetSpecialCombat(bool bSpec)
		{	SetFlag(kFactionFlags_SpecialCombat, bSpec);	}
	const char* GetNthRankName(UInt32 whichRank, bool bFemale = false);
	void SetNthRankName(const char* newName, UInt32 whichRank, bool bFemale);
};

typedef Visitor<TESFaction::RankEntry, TESFaction::RankData> FactionRankVisitor;

// 04C
class TESHair : public TESForm
{
public:
	// no changed flags (TESForm flags)

	enum
	{
		kFlag_Playable =	1 << 0,
		kFlag_NotMale =		1 << 1,
		kFlag_NotFemale =	1 << 2,
		kFlag_FixedColor =	1 << 3,
	};

	TESHair();
	~TESHair();

	// child classes
	TESFullName	fullName;	// 018
	TESModel	model;		// 024
	TESTexture	texture;	// 03C

	// members
	UInt8	flags;	// 048
	UInt8	pad[3];	// 049
};

// 34
class TESEyes : public TESForm
{
public:
	// no changed flags (TESForm flags)

	enum
	{
		kFlag_Playable =	1 << 0
	};

	TESEyes();
	~TESEyes();

	// child classes
	TESFullName	fullName;	// 018
	TESTexture	texture;	// 024

	// members
	UInt8	flags;	// 030
	UInt8	pad[3];	// 031
};

// 318
class TESRace : public TESForm
{
public:
	// no changed flags (TESForm flags)

	TESRace();
	~TESRace();

	// child classes
	TESFullName		fullName;	// 018
	TESDescription	desc;		// 024
	TESSpellList	spells;		// 02C
	TESReactionForm	reaction;	// 040

	struct Unk
	{
		UInt32	unk0;
		UInt32	unk1;
		UInt32	unk2;
		UInt32	unk3;
		UInt32	unk4;
		UInt32	unk5;
	};
	struct BonusSkillInfo
	{
		UInt8	skill;
		UInt8	bonus;
	};

	// members
	BonusSkillInfo	bonusSkills[7];			// 050
	UInt16			pad0;
	float			maleScale;				// 060
	float			femaleScale;			// 064
	float			maleWeight;				// 068
	float			femaleWeight;			// 06C
	UInt8			isPlayable;				// 070
	UInt8			unk1[3];
	TESAttributes	maleAttr;				// 074
	TESAttributes	femaleAttr;				// 080
	tList<TESHair>	hairs;					// 08C
	TESHair			* defaultHair[2];		// 094 0-male, 1-female
	UInt32			unk09C[3];				// 09C
	tList<TESEyes>	eyes;					// 0A8
	TESModel		tails[2];				// 0B0 0-male 1-female
	TESModel		unk9[9];				// 0E0
	TESTexture		unk10[9];				// 1B8
	TESTexture		unk11[10];				// 224
	Unk				unk12[4];				// 29C
	UInt32			unk13;					// 2FC
	TESRace			* voiceRaces[2];		//300 - 0=Male, 1=Female
	UInt32			unk13_2;				//308
	UInt32			unk14;					// 30C
	UInt16			unk15;					// 310
	UInt16			unk16;					// 312
	UInt16			unk17;					// 314
	UInt16			pad;					// 316

	UInt32 GetBaseAttribute(UInt32 attribute, bool bForFemale) const;
	UInt32 GetSkillBonus(UInt32 skill) const;
	bool IsBonusSkill(UInt32 skill) const;
	UInt32 GetNthBonusSkill(UInt32 n) const;
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESRace) == 0x318);
#endif

// 44
class TESSound : public TESBoundAnimObject
{
public:
	// no changed flags (TESForm flags)

	TESSound();
	~TESSound();

	enum {
		kFlags_RandomFrequencyShift = 1,
		//...
		kFlags_Loop					= 1 << 4,
		kFlags_MenuSound			= 1 << 5,
		kFlags_2D					= 1 << 6,
		//...
	};

	// bases
	TESSoundFile	soundFile;	// 024

	// members
	UInt8	minAttenuation;		// 038 not taken directly from CS
	UInt8	maxAttenuation;		// 039 likewise. Different ratios for each
	UInt8	frequencyAdjust;	// 03A
	UInt8	unk9;				// 03B
	UInt32	flags;				// 03C
	UInt16	staticAttenuation;	// 040 - CS value * -100
	UInt16	unk12;				// 042 related to start/end times
};

// 60 / A0
class TESSkill : public TESForm
{
public:
	// no changed flags (TESForm flags)

	enum
	{
		kSkill_Armorer =	0x0C,
		kSkill_Athletics,
		kSkill_Blade,
		kSkill_Block,
		kSkill_Blunt,		// 10
		kSkill_HandToHand,
		kSkill_HeavyArmor,
		kSkill_Alchemy,
		kSkill_Alteration,
		kSkill_Conjuration,
		kSkill_Destruction,
		kSkill_Illusion,
		kSkill_Mysticism,	// 18
		kSkill_Restoration,
		kSkill_Acrobatics,
		kSkill_LightArmor,
		kSkill_Marksman,
		kSkill_Mercantile,
		kSkill_Security,
		kSkill_Sneak,
		kSkill_Speechcraft,	// 20
	};

	TESSkill();
	~TESSkill();

	// bases
	TESDescription	description;	// 018 / 24
	TESTexture		texture;		// 020 / 34

	// members
	UInt32			skill;			// 02C / 4C
	UInt32			attribute;		// 030 / 50
	UInt32			specialization;	// 034 / 54
	float			useValue0;		// 038 / 58 - amount to learn on successful action
	float			useValue1;		// 03C / 5C - action is different for each skill
	TESDescription	levelQuote[4];	// 040 / 60

	static TESSkill* SkillForActorVal(UInt32 actorVal);
	const char* GetLevelQuoteText(UInt32 level);
};

// 34
class TESLandTexture : public TESForm
{
public:
	// no changed flags (TESForm flags)

	TESLandTexture();
	~TESLandTexture();

	// bases
	TESTexture	texture;	// 018

	// members
	UInt32	unk0;	// 024
	UInt8	unk1;	// 028
	UInt8	unk2;	// 029
	UInt8	unk3;	// 02A
	UInt8	unk4;	// 02B
	UInt32	unk5;	// 02C
	UInt32	unk6;	// 030
};

// 44
class EnchantmentItem : public MagicItemForm
{
public:
	// no changed flags (TESForm flags)

	EnchantmentItem();
	~EnchantmentItem();

	enum {
		kEnchant_Scroll = 0,
		kEnchant_Staff,
		kEnchant_Weapon,
		kEnchant_Apparel
	};

	enum {
		kEnchant_NoAutoCalc = 0x1,
	};

	// members
	UInt32	enchantType;	// 034 - initialized to kEnchant_Weapon
	UInt32	charge;			// 03C - initialized to -1
	UInt32	cost;			// 040 - initialized to -1
	UInt8	flags040;		// 040
	UInt8	pad041[3];

	bool MatchesType(TESForm* form);
	bool IsAutoCalc() const;
	void SetAutoCalc(bool bAutoCalc);
};

// 44
class SpellItem : public MagicItemForm
{
public:
	// no changed flags (TESForm flags)

	SpellItem();
	~SpellItem();

	enum {
		kType_Spell = 0,
		kType_Disease,
		kType_Power,
		kType_LesserPower,
		kType_Ability,
		kType_All,
	};

	enum {
		kLevel_Novice = 0,
		kLevel_Apprentice,
		kLevel_Journeyman,
		kLevel_Expert,
		kLevel_Master,
	};

	enum {
		kFlag_NoAutoCalc =					0x1,
		kFlag_PCStart =						0x4,
		kFlag_ImmuneToSilence =				0xA,	// ### 2 bits?
		kFlag_AreaEffectIgnoresLOS =		0x10,
		kFlag_ScriptEffectAlwaysApplies =	0x20,
		kFlag_DisallowAbsorbReflect =		0x40,
		kFlag_TouchExplodesWithNoTarget =	0x80,
	};

	// members
	UInt32	spellType;		// 038 - init'd to FFFFFFFF
	UInt32	magickaCost;	// 03C
	UInt32	masteryLevel;	// 040
	UInt32	spellFlags;		// 044

	bool IsAutoCalc() const;
	void SetAutoCalc(bool bAutoCalc);
	UInt32 GetSchool() const;
	UInt32 GetMagickaCost(TESForm* form = NULL) const;
	UInt32 GetMasteryLevel() const;
	bool TouchExplodesWithNoTarget() const;
	void SetTouchExplodes(bool bExplodesWithNoTarget);
	bool IsHostile();
	void SetHostile(bool bHostile);
};

// 4C
class BirthSign : public TESForm
{
public:
	// no changed flags (TESForm flags)

	BirthSign();
	~BirthSign();

	// bases
	TESFullName		fullName;	// 018
	TESTexture		texture;	// 024
	TESDescription	desc;		// 030
	TESSpellList	spellList;	// 038
};

// 58
class TESObjectACTI : public TESBoundAnimObject
{
public:
	// no changed flags (TESForm flags)

	TESObjectACTI();
	~TESObjectACTI();

	// bases
	TESFullName			fullName;	// 024
	TESModel			model;		// 030
	TESScriptableForm	scriptable;	// 048

	// members
	TESSound*			loopSound;	// 054
};

// 7C
class TESObjectAPPA : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESObjectAPPA();
	~TESObjectAPPA();

	// bases
	TESFullName			fullName;	// 024
	TESModel			model;		// 030
	TESIcon				icon;		// 048
	TESScriptableForm	scriptable;	// 054
	TESValueForm		value;		// 060
	TESWeightForm		weight;		// 068
	TESQualityForm		quality;	// 070

	// members
	enum {
		eApparatus_MortarPestle = 0,
		eApparatus_Alembic,
		eApparatus_Calcinator,
		eApparatus_Retort,
	};
	UInt8	appaType;	// 078
	UInt8	pad[3];	// 079

	void SetType(UInt8 appaType);
};

// E8
class TESObjectARMO : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESObjectARMO();
	~TESObjectARMO();

	// children
	TESFullName			fullName;		// 024
	TESScriptableForm	scriptable;		// 030
	TESEnchantableForm	enchantable;	// 03C
	TESValueForm		value;			// 04C
	TESWeightForm		weight;			// 054
	TESHealthForm		health;			// 05C
	TESBipedModelForm	bipedModel;		// 064

	// members
	UInt16	armorRating;			// 0E4
	UInt16	pad;					// 0E6

	bool IsHeavyArmor() const;
	void SetHeavyArmor(bool bHeavyArmor);
};

// 8C
class TESObjectBOOK : public TESBoundObject
{
public:
	enum
	{
		// TESForm flags

		// TESValueForm flags

		kModified_TeachesSkill =	0x00000004,
			// CHANGE_BOOK_TEACHES_SKILL
			// UInt8	teachesSkill
	};

	TESObjectBOOK();
	~TESObjectBOOK();

	// children
	TESFullName			fullName;		// 024
	TESModel			model;			// 030
	TESIcon				icon;			// 048
	TESScriptableForm	scriptable;		// 054
	TESEnchantableForm	enchantable;	// 060
	TESValueForm		value;			// 070
	TESWeightForm		weight;			// 078
	TESDescription		description;	// 080

	// members
	enum {
		kBook_IsScroll = 1,
		kBook_CantBeTaken = 1 << 1,
	};

	UInt8				bookFlags;		// 088
	UInt8				teachesSkill;	// 089
	UInt8	pad[2];	// 08A

	void Constructor(void);
	bool CantBeTaken() const;
	void SetCantBeTaken(bool bCantBeTaken);
	bool IsScroll() const;
	void SetIsScroll(bool bIsScroll);
	float Teaches() const;
	void SetTeaches(UInt32 skill);
};

// DC
class TESObjectCLOT : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESObjectCLOT();
	~TESObjectCLOT();

	// children
	TESFullName			fullName;		// 024
	TESScriptableForm	scriptable;		// 030
	TESEnchantableForm	enchantable;	// 03C
	TESValueForm		value;			// 04C
	TESWeightForm		weight;			// 054
	TESBipedModelForm	bipedModel;		// 05C
};

// 7C
class TESObjectCONT : public TESBoundAnimObject
{
public:
	// no changed flags (TESForm flags)

	TESObjectCONT();
	~TESObjectCONT();

	enum {
		eFlags_Respawning = 0x2
	};

	// child classes
	TESContainer		container;	// 024
	TESFullName			fullName;	// 034
	TESModel			model;		// 040
	TESScriptableForm	scriptable;	// 058
	TESWeightForm		weight;		// 064
	UInt32				unk0;		// 06C
	TESSound*			animSounds[2];	// 070 0=open, 1=close
	UInt8				flags078;	// 078
	UInt8				pad[3];		// 079

	bool IsRespawning() {
		return (flags078 & eFlags_Respawning) != 0;
	}

	void SetRespawning(bool bRespawn) {
		if (bRespawn) {
			flags078 |= eFlags_Respawning;
		} else {
			flags078 &= ~eFlags_Respawning;
		}
	}
};

// 70
class TESObjectDOOR : public TESBoundAnimObject
{
public:
	// no changed flags (TESForm flags)

	TESObjectDOOR();
	~TESObjectDOOR();

	enum {
		kDoorFlag_OblivionGate		= 1 << 0,
		kDoorFlag_Automatic			= 1 << 1,
		kDoorFlag_Hidden			= 1 << 2,
		kDoorFlag_MinimalUse		= 1 << 3
	};

	struct RandomTeleportEntry
	{
		TESWorldSpace*		destination; //can also be TESObjectCELL
		RandomTeleportEntry	* next;
	};

	// bases
	TESFullName			fullName;	// 024
	TESModel			model;		// 030
	TESScriptableForm	scriptable;	// 048
	// 054 - TESMagicCasterForm
	// 055 - TESMagicTargetForm
	UInt32				basePad;

	// members
	TESSound*			animSounds[3];		// 058 0=open, 1=close, 2=loop
	UInt8				doorFlags;			// 064
	UInt8				pad[3];				// 065
	RandomTeleportEntry randomTeleport;		// 068
};

// 80
class IngredientItem : public MagicItemObject
{
public:
	// no changed flags (TESForm flags)

	IngredientItem();
	~IngredientItem();
	// bases
	TESModel			model;		// 040
	TESIcon				icon;		// 058
	TESScriptableForm	scriptable;	// 064
	TESWeightForm		weight;		// 070

	// members
	UInt32	value;	// 078 - init'd to FFFFFFFF
	enum {
		kIngred_NoAutocalc = 0x1,
		kIngred_Food = 0x2,
	};
	UInt32	ingredFlags;

	bool IsFood() const { return _IsFlagSet(kIngred_Food); }
	void SetIsFood(bool bFood) { _SetFlag(kIngred_Food, bFood); }
	bool IsAutoCalc() const { return !_IsFlagSet(kIngred_NoAutocalc); }
	void SetAutoCalc(bool bAutocalc) { _SetFlag(kIngred_NoAutocalc, !bAutocalc); }
private:
	bool _IsFlagSet(UInt32 mask) const;
	void _SetFlag(UInt32 flag, bool bSet);
};

// 90
class TESObjectLIGH : public TESBoundAnimObject
{
public:
	// TESForm flags
	// TESValueForm flags

	enum
	{
		kLightFlags_Dynamic =		0x001,
		kLightFlags_CanCarry =		0x002,
		kLightFlags_Negative =		0x004,
		kLightFlags_Flicker =		0x008,
		kLightFlags_OffByDefault =	0x020,
		kLightFlags_FlickerSlow =	0x040,
		kLightFlags_Pulse =			0x080,
		kLightFlags_PulseSlow =		0x100,
		kLightFlags_SpotLight =		0x200,
		kLightFlags_SpotShadow =	0x400
	};

	TESObjectLIGH();
	~TESObjectLIGH();

	// bases
	TESFullName			fullName;	// 024
	TESModel			model;		// 030
	TESIcon				icon;		// 048
	TESScriptableForm	scriptable;	// 054
	TESWeightForm		weight;		// 060
	TESValueForm		value;		// 068

	// members

	// 018
	UInt32	time;		// 070
	UInt32	radius;		// 074
	RGBA	colorRGB;	// 078
	UInt32	lightFlags;	// 07C
	float	fallOff;	// 080
	float	FOV;		// 084
	float	fade;		// 088
	TESSound * loopSound;	// 08C

	UInt32 GetRadius()
		{	return radius;	}
	void SetRadius(UInt32 newRadius)
		{	radius = newRadius;	}
	bool IsCarriable()
		{	return (lightFlags & kLightFlags_CanCarry) ? true : false;	}
};

// 70
class TESObjectMISC : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESObjectMISC();
	~TESObjectMISC();

	// bases
	TESFullName			fullName;	// 024
	TESModel			model;		// 030
	TESTexture			texture;	// 048
	TESScriptableForm	scriptable;	// 054
	TESValueForm		value;		// 060
	TESWeightForm		weight;		// 068
};

// 3C
class TESObjectSTAT : public TESBoundObject
{
public:
	// no changed flags (TESForm flags)

	TESObjectSTAT();
	~TESObjectSTAT();

	// children
	TESModel	model;	// 024
};

// 5C
class TESGrass : public TESBoundObject
{
public:
	// no changed flags (TESForm flags)

	TESGrass();
	~TESGrass();

	// bases
	TESModel	model;		// 024

	// members
	UInt8		unk03C;		// 03C
	UInt8		unk03D;		// 03D
	UInt8		unk03E;		// 03E
	UInt8		pad03F;		// 03F
	UInt16		unk040;		// 040
	UInt16		pad042;		// 042
	UInt32		unk044;		// 044
	float		unk048;		// 048
	float		unk04C;		// 04C
	float		unk050;		// 050
	float		unk054;		// 054
	UInt8		unk058;		// 058
	UInt8		pad059[3];	// 059
};

// 80
class TESObjectTREE : public TESBoundTreeObject
{
public:
	// no changed flags (TESForm flags)

	TESObjectTREE();
	~TESObjectTREE();

	// bases
	TESModel	model;		// 024
	TESTexture	texture;	// 03C

	// members
	// NiTArray <uint>
	NiTArray <UInt32>	unk048;		// 048 - verify type
	UInt16		unk050;		// 050
	UInt16		unk052;		// 052
	UInt16		unk054;		// 054
	UInt16		unk056;		// 056
	float		unk058;		// 058
	float		unk05C;		// 05C
	float		unk060;		// 060
	float		unk064;		// 064
	float		unk068;		// 068
	UInt32		pad06C;		// 06C
	float		unk070;		// 070
	float		unk074;		// 074
	void		* unk078;	// 078
	void		* unk07C;	// 07C
};

// 64
class TESFlora : public TESProduceForm
{
public:
	// no changed flags (no TESForm)

	enum
	{
		kFloraFlags_Harvested = 0x2000,		//On reference, not base
		kModified_Empty =		0x10000		//Ditto
	};

	TESFlora();
	~TESFlora();

	// bases
	TESObjectACTI	activator;			// 00C
};

// 5C
class TESFurniture : public TESObjectACTI
{
public:
	// no changed flags (TESForm flags)

	TESFurniture();
	~TESFurniture();

	// members
	UInt32	unk058;	// 058
};

class TESObjectWEAP : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESObjectWEAP();
	~TESObjectWEAP();

	// child classes (exposed for easy access)
	TESFullName			fullName;		// 024
	TESModel			model;			// 030
	TESIcon				icon;			// 048
	TESScriptableForm	scriptable;		// 054
	TESEnchantableForm	enchantable;	// 060
	TESValueForm		value;			// 070
	TESWeightForm		weight;			// 078
	TESHealthForm		health;			// 080
	TESAttackDamageForm	attackDmg;		// 088
	// 090

	enum
	{
		kType_BladeOneHand = 0,
		kType_BladeTwoHand,
		kType_BluntOneHand,
		kType_BluntTwoHand,
		kType_Staff,
		kType_Bow,

		kType_Max,
	};

	// member vars
	UInt32	type;						// 090
	float	speed;						// 094
	float	reach;						// 098
	UInt32	ignoreNormalWeaponResist;	// 09C
};

// 9E
class TESAmmo : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESAmmo();
	~TESAmmo();
	// child classes (exposed for easy access)
	TESFullName			fullName;					// 024
	TESModel			model;						// 030
	TESIcon				icon;						// 048
	TESEnchantableForm	enchantable;				// 054
	TESValueForm		value;						// 064
	TESWeightForm		weight;						// 072
	TESAttackDamageForm	attackDmg;					// 080
	float				speed;						// 088
	UInt32				ignoreNormalWeaponResist;	// 092
	UInt32				unk1;						// 096	// seems to be a pointer
	UInt32				unk2;						// 09A	// seems to be flags?  values 0x84 and 0x200
};

// 200
class TESNPC : public TESActorBase
{
public:
	enum
	{
		// TESActorBase flags

		kChanged_TESNPC_EC_15 =	0x00000200,
			// unknown, 0x15 bytes at +0xEC
			// CHANGE_NPC_SKILLS?

		kChanged_TESNPC_1E4 =	0x00000400,
			// calls through the other unknown write/read fn with something at +0x1E4
			// CHANGE_NPC_COMBATSTYLE?
	};

	TESNPC();
	~TESNPC();

	// base classes
	TESRaceForm		race;	// 0E4
	// 0EC

	// 1C
	struct Unk
	{
		UInt32	unk0;	// 000
		UInt32	unk1;	// 004
		UInt32	unk2;	// 008
		UInt32	unk3;	// 00C
		UInt32	unk4;	// 010
		UInt32	unk5;	// 014
	};

	// members
	UInt8		skillLevels[0x15];	// 0EC
	UInt8		pad101[3];			// 101
	TESClass*	npcClass;			// 104
	Unk			unk1[4];			// 108
	Unk			unk2[4];			// 168
	TESHair*	hair;				// 1C8
	UInt32		hairLength;			// 1CC
	TESEyes*	eyes;				// 1D0
	BSFaceGenNiNode* face0;			// 1D4 male and female unknown order
	BSFaceGenNiNode* face1;			// 1D8
	UInt32		unk6;				// 1DC
	UInt32		unk7;				// 1E0
	TESCombatStyle	* combatStyle;	// 1E4 is NULL if set as DEFAULT in CS
	UInt8		hairColorRGB[4];	//1E8

	UInt32		unk8;				// 1EC
	NiTArray <FaceGenUndo *>	faceGenUndo;	// 1F0
};

// 138 (1.1)
// 140 (1.2) - changed due to TESModelList
class TESCreature : public TESActorBase
{
public:
	enum
	{
		// TESActorBase flags

		kChanged_CreatureSkills =	0x00000200,
			// CHANGE_CREATURE_SKILLS
			// UInt8	combatSkill;
			// UInt8	magicSkill;
			// UInt8	stealthSkill;

		kChanged_CombatStyle =		0x00000400,
			// SaveGameData2 +0x110
			// CHANGE_CREATURE_COMBATSTYLE
			// UInt32	refIdx;
	};

	TESCreature();
	~TESCreature();

	enum {
		eCreatureType_Creature = 0,
		eCreatureType_Daedra,
		eCreatureType_Undead,
		eCreatureType_Humanoid,
		eCreatureType_Horse,
		eCreatureType_Giant,

		eCreatureType_MAX
	};

	struct CreatureSound {
		TESSound	* sound;
		UInt8		chance;
		UInt8		pad[3];
	};

	struct CreatureSoundEntry {
		CreatureSound	* data;
		UInt32			unk01;	//only seen zero - perhaps used to be next* in linked list?
	};

	union SoundData {
		CreatureSoundEntry	** sounds;		// pointer to array of 10 CreatureSoundEntries for non-inherited sounds
		TESCreature			*  soundBase;	//TESCreature* for inherited sounds
	};

	enum {
		eCreatureSound_LeftFoot = 0,
		eCreatureSound_RightFoot,
		eCreatureSound_LeftBack,
		eCreatureSound_RightBack,
		eCreatureSound_Idle,
		eCreatureSound_Aware,
		eCreatureSound_Attack,
		eCreatureSound_Hit,
		eCreatureSound_Death,
		eCreatureSound_Weapon,
		eCreatureSound_MAX,
	};

	// base classes
	TESAttackDamageForm	attackDamage;	// 0E4
	TESModelList		modelList;		// 0EC - changed size in 1.2

	// members
	SoundData	soundData;				// 0F8 / 100
	UInt8		type;					// 0FC / 104
	UInt8		combatSkill;			// 0FD / 105
	UInt8		magicSkill;				// 0FE / 106
	UInt8		stealthSkill;			// 0FF / 107
	UInt8		soulLevel;				// 100 / 108
	UInt8		unkB;
	UInt8		attackReach;
	UInt8		unkD;
	float		turningSpeed;			// 104 / 10C
	float		footWeight;				// 108 / 110
	float		baseScale;				// 10C / 114
	TESCombatStyle*	combatStyle;		// 110 / 118

	TESModel	bloodSpray;				// 114 / 11C
	TESTexture	bloodDecal;				// 12C / 134

	TESCreature*	GetSoundBase();
	TESSound*	GetSound(UInt32 whichSound);
	UInt32		GetSoundChance(UInt32 whichSound);
	bool		HasSounds();
	bool		SetSoundBase(TESCreature* base);
};

// 44
class TESLevCreature : public TESBoundObject
{
public:
	// no changed flags (TESForm flags)

	TESLevCreature();
	~TESLevCreature();

	// bases
	TESLeveledList		leveledList;	// 024
	TESScriptableForm	scriptable;		// 034

	// members
	TESActorBase*		templateForm;	//044
};

class TESSoulGem : public TESBoundObject
{
public:
	// TESForm flags
	// TESValueForm flags

	TESSoulGem();
	~TESSoulGem();

	TESFullName			fullName;					// 024
	TESModel			model;						// 030
	TESIcon				icon;						// 048
	TESScriptableForm	scriptable;		// 054
	TESValueForm		value;						// 064
	TESWeightForm		weight;						// 072

	enum {
		kSoul_None = 0,
		kSoul_Petty,
		kSoul_Lesser,
		kSoul_Common,
		kSoul_Greater,
		kSoul_Grand,
	};

	UInt8				soul;
	UInt8				capacity;
	UInt8				padding[2];
};

// 70
class TESKey : public TESObjectMISC
{
public:
	// TESForm flags
	// TESValueForm flags

	TESKey();
	~TESKey();
};

class AlchemyItem : public MagicItemObject
{
public:
	// no changed flags (TESForm flags)

	AlchemyItem();
	~AlchemyItem();

	enum {
		kAlchemy_NoAutocalc = 0x1,
		kAlchemy_IsFood = 0x2,	// in moreFlags
	};

	TESModel			model;
	TESIcon				icon;
	TESScriptableForm	scriptable;
	TESWeightForm		weight;
	UInt32				goldValue;

	UInt32				moreFlags;

	bool IsPoison() const;
	bool IsFood() const  { return _IsFlagSet(kAlchemy_IsFood); }
	void SetIsFood(bool bFood) { _SetFlag(kAlchemy_IsFood, bFood); }
	bool IsAutoCalc() const { return !_IsFlagSet(kAlchemy_NoAutocalc); }
	void SetAutoCalc(bool bSet) { return _SetFlag(kAlchemy_NoAutocalc, !bSet); }
	float GetGoldValue();
	void SetGoldValue(UInt32 newValue);
private:
	bool _IsFlagSet(UInt32 flag) const;
	void _SetFlag(UInt32 flag, bool bSet);
};

// TESSubSpace

// 008
class TESUsesForm : public BaseFormComponent
{
public:
	TESUsesForm();
	~TESUsesForm();
	UInt32 uses;
	// 008
};

// 88
class TESSigilStone : public TESBoundObject
{
public:
	TESSigilStone();
	~TESSigilStone();

	TESFullName			name;		// 024
	TESModel			model;		// 030
	TESIcon				icon;		// 048
	TESScriptableForm	scriptable;	// 054
	TESValueForm		value;		// 060
	TESWeightForm		weight;		// 068
	TESUsesForm			uses;		// 070
	EffectItemList		effectList;	// 078
};

// 34
class TESLevItem : public TESBoundObject
{
public:
	TESLevItem();
	~TESLevItem();

	// bases
	TESLeveledList	leveledList;	// 24
};

// TESLevItem
// SNDG

// 148
class TESWeather : public TESForm
{
public:
	TESWeather();
	~TESWeather();

	typedef ::RGBA RGBA;

	struct FogInfo {
		float nearFog;
		float farFog;
	};

	enum {
		eHDR_EyeAdpat = 0,
		eHDR_BlurRadius,
		eHDR_BlurPasses,
		eHDR_EmissiveMult,
		eHDR_TargetLUM,
		eHDR_UpperLUMClamp,
		eHDR_BrightScale,
		eHDR_BrightClamp,
		eHDR_LUMRampNoTex,
		eHDR_LUMRampMin,
		eHDR_LUMRampMax,
		eHDR_SunlightDimmer,
		eHDR_GrassDimmer,
		eHDR_TreeDimmer,
		eHDR_Last = eHDR_TreeDimmer,
	};

	enum {
		eColor_SkyUpper = 0,
		eColor_Fog,
		eColor_CloudsLower,
		eColor_Ambient,
		eColor_Sunlight,
		eColor_Sun,
		eColor_Stars,
		eColor_SkyLower,
		eColor_Horizon,
		eColor_CloudsUpper,
		eColor_Lightning,
		eColor_Last = eColor_Lightning,
	};

	enum {
		eTime_Sunrise = 0,
		eTime_Day,
		eTime_Sunset,
		eTime_Night,
	};

	struct ColorData {
		RGBA colors[4];
	};

	struct SoundData {
		UInt32	refID;
		UInt32	count;
	};

	enum {
		kType_None = 0,
		kType_Pleasant = 1,
		kType_Cloudy = 2,
		kType_Rainy = 3,
		kType_Snow = 4
	};

	TESTexture	upperLayer;		// 018
	TESTexture	lowerLayer;		// 024
	TESModel	model;			// 030
	UInt8		windSpeed;
	UInt8		cloudSpeedLower;
	UInt8		cloudSpeedUpper;
	UInt8		transDelta;
	UInt8		sunGlare;
	UInt8		sunDamage;
	UInt8		pad[2];
	UInt8		lightningBeginFadeIn;
	UInt8		lightningEndFadeOut;
	UInt8		lightningFrequency;
	UInt8		precipType;
	RGBA		lightningColor;
	FogInfo		fogDay;			// 058
	FogInfo		fogNight;		// 060
	ColorData	colors[10];
	SoundData*	unk108;	// these are the bits and pieces of the sound data
	SoundData*	unk10C;	// in the one decoded case there are only 2 - so this may be a head/tail interface
	float		hdrInfo[14];		// 110
	// 148

	float GetHDRValue(UInt32 eWhich) const {
		return (eWhich > eHDR_Last) ? 0.0 : hdrInfo[eWhich];
	}

	float SetHDRValue(UInt32 eWhich, float nuVal) {
		if (eWhich > eHDR_Last) return 0.0;
		float oldVal = hdrInfo[eWhich];
		hdrInfo[eWhich] = nuVal;
		return oldVal;
	}

	RGBA& GetColor(UInt32 whichColor, UInt8 time);
};

// 058
class TESClimate : public TESForm
{
public:
	TESClimate();
	~TESClimate();

	struct WeatherInfo {
		TESWeather* weather;
		UInt32 chance;
	};

	struct WeatherEntry {
		WeatherInfo* entry;
		WeatherEntry* next;
		WeatherInfo* Info() const { return entry; }
		WeatherEntry* Next() const { return next; }
	};

	TESModel		nightSky;		// 018
	WeatherEntry	list;
	TESTexture		sun;			// 038
	TESTexture		sunGlare;		// 044
	UInt8			sunriseBegin;	// 050
	UInt8			sunriseEnd;		// these are the number of 10 minute increments past midnight
	UInt8			sunsetBegin;	// 6 increments per hour
	UInt8			sunsetEnd;
	UInt8			volatility;		// 054
	UInt8			moonInfo;
	UInt8			pad[2];
	// 058

	enum {
		kClimate_Masser = 0x80,
		kClimate_Secunda = 0x40,
		kClimate_PhaseLengthMask = 0x3F,
	};
	// moonInfo bits 0-6 define the moon length
	UInt8 GetPhaseLength() const;
	bool HasMasser() const; // moonInfo bit 8
	bool HasSecunda() const; // moonInfo bit 7

	void SetPhaseLength(UInt8 nuVal);
	void SetHasMasser(bool bHasMasser);
	void SetHasSecunda(bool bHasSecunda);

	void SetSunriseBegin(UInt8 nuVal);
	void SetSunriseEnd(UInt8 nuVal);
	void SetSunsetBegin(UInt8 nuVal);
	void SetSunsetEnd(UInt8 nuVal);
};

typedef Visitor<TESClimate::WeatherEntry, TESClimate::WeatherInfo> WeatherVisitor;

class TESRegionData : public BaseFormComponent {
public:
	TESRegionData();
	~TESRegionData();

	UInt8 OverrideFlag;  //1 if Override flag setted, 0 otherwise
	UInt8 unk2;  //Always seen 0
	UInt8 Priority;
	UInt8 unk4;   //Spurious, seems to be 7 or 8, maybe unused?
};

class TESRegionDataMap : public TESRegionData {
public:
	TESRegionDataMap();
	~TESRegionDataMap();

	BSStringT regionName;
};

// 2C
class TESRegion : public TESForm
{
public:
	TESRegion();
	~TESRegion();

	struct RegionDataEntry {
		TESRegionData* data;
		RegionDataEntry* next;
	};
	struct RegionDataList {
		RegionDataEntry entry;
		UInt8 unk8;   //Always 1
		UInt8 pad9[3];  // Zero most of the times, 13-3-0 once
	};
    
	struct Unk01C
	{
		UInt32	unk0;
		Unk01C	* next;
	};

	RegionDataList* dataList;			// 018
	Unk01C	* areaList;				// 01C
	TESWorldSpace	* worldSpace;	// 020
	TESWeather		* weather;		// 024
	float	unk028;		
};

// 58
class TESObjectCELL : public TESForm
{
public:
#if OBLIVION
	MEMBER_FN_PREFIX(TESObjectCELL);
	DEFINE_MEMBER_FN(SetDetachTime, void, kTESObjectCELL_SetDetachTimeAddr, UInt32 detachTime);
#endif

	enum
	{
		kModified_DetachTime = 0x08000000,
			// only present if version >= 0x5A
			// UInt32	detachTime;	// maybe float
			// stored as ExtraData chunk 0x10

		// TESForm bits

		kModified_CellFlags =	0x00000008,
			// CHANGE_CELL_FLAGS
			// UInt8	flags;
			// bits 0x60 stored in flags0
			// bits 0x9F stored in flags1

		kModified_SeenData =	0x10000000,
			// CHANGE_CELL_SEENDATA
			// no data, just adds a ExtraSeenData, then calls a virtual function passing 0xFFFF

		//	if(version < 0x5A)
		//		kModified_DetachTime
		//			(as above)

		kModified_FullName =	0x00000010,
			// CHANGE_CELL_FULLNAME
			// UInt8	strLen;
			// char		name[strLen];

		kModified_Ownership =	0x00000020,
			// CHANGE_CELL_OWNERSHIP
			// UInt32	formIdx;

		kModified_Pathgrid =	0x01000000,
			// CHANGE_CELL_PATHGRID_MODIFIED
			// ### todo: data
	};

	enum
	{
		kFlags0_Interior =				    1 << 0,
		kFlags0_HasWater =				    1 << 1,
		kFlags0_InvertFastTravelBeheviour =	1 << 2,
		kFlags0_ForceHideLand =			    1 << 3,	// shared bit - for exterior
		kFlags0_OblivionInterior =		    1 << 3,	// shared bit - for interior
		kFlags0_Unk4 =					    1 << 4,
		kFlags0_Public =				    1 << 5,
		kFlags0_HandChanged =			    1 << 6,
		kFlags0_BehaveLikeExterior =	    1 << 7,
	};

	TESObjectCELL();
	~TESObjectCELL();

	// base class
	TESFullName	fullName;	// 018

	struct ObjectListEntry
	{
		TESObjectREFR		* refr;
		ObjectListEntry		* next;

		TESObjectREFR*		Info() const	{ return refr; }
		ObjectListEntry*	Next() const	{ return next; }
	};

	struct CellCoordinates
	{
		SInt32	x;
		SInt32	y;
	};

	struct LightingData
	{
		RGBA		ambient;
		RGBA		directional;
		RGBA		fog;
		float		fogNear;
		float		fogFar;
		UInt32		rotXY;
		UInt32		rotZ;
		float		directionalFade;
		float		fogClipDistance;
	};

	// members
	UInt8			flags0;			// 024
	UInt8			flags1;			// 025
	UInt8			flags2;			// 026
	UInt8			pad27;			// 027
	ExtraDataList	extraData;		// 028 includes ExtraEditorID
	union {
		CellCoordinates * coords;		// if exterior
		LightingData	* lighting;		// if interior
	};								// 3C
	TESObjectLAND	* land;			// 040
	TESPathGrid		* pathGrid;		// 044
	ObjectListEntry	objectList;		//048
	TESWorldSpace	* worldSpace;	// 050
	NiNode			* unk6;			// 054

	bool IsInterior() const;
	bool HasWater() const;
	void SetHasWater(bool bHas);
	float GetWaterHeight() const;
	bool SetWaterHeight(float newHeight);
	TESWaterForm* GetWaterType() const;
	bool SetWaterType(TESWaterForm* type);
	bool BehavesLikeExterior() const { return flags0 & kFlags0_BehaveLikeExterior ? true : false; }
	void SetBehavesLikeExterior(bool bSet);
	void SetInteriorClimate(TESClimate* climate);
	bool GetIsPublic() const { return IsInterior() && (flags0 & kFlags0_Public); }
	void SetIsPublic(bool bSet);
	bool IsOblivionInterior() const { return IsInterior() && (flags0 & kFlags0_OblivionInterior); }
};

typedef Visitor<TESObjectCELL::ObjectListEntry, TESObjectREFR> CellListVisitor;

// TESObjectREFR
// ACHR
// ACHE

// 2C
struct TESPathGridPoint
{
public:
	TESPathGridPoint();
	~TESPathGridPoint();

	enum {
		kFlag_Disabled =		1 << 5,
	};

	UInt32					unk00[4];		// 00
	UInt8					flags;			// 10
	UInt8					pad11[3];
	float					x, y, z;		// 14
	tList<TESPathGridPoint>	edges;			// 20
	NiNode					* niNode;		// 28

	bool IsDisabled() { return flags & kFlag_Disabled ? true : false; }
	void SetDisabled(bool bDisabled) {
		if (bDisabled)	flags |= kFlag_Disabled;
		else			flags &= ~kFlag_Disabled;
	}
	bool SetEdgeEnabled(TESPathGridPoint* target, bool bEnable);

	static TESPathGridPoint* Create(float _x, float _y, float _z);
};

STATIC_ASSERT(sizeof(TESPathGridPoint) == 0x2C);

// 54
class TESPathGrid : public TESForm
{
	bool SetExternalEdge(UInt16 localID, TESPathGridPoint* target, bool bEnableEdge);
public:
	TESPathGrid();
	~TESPathGrid();

	typedef tList<TESPathGridPoint> PointList;

	struct ExternalEdge {
		// used for edges which cross exterior cell boundaries - maps local node to coordinates of external node
		UInt16		localNodeID;		// 00
		UInt16		pad02;
		float		x, y, z;			// 04

		static ExternalEdge* Create(UInt16 id, TESPathGridPoint* target);
	};

	typedef tListBase<ExternalEdge, true>	ExternalEdgeList;

	// bases
	TESChildCell	childCell;		// 018

	// members
	NiNode						* niNode;			// 01C
	TESObjectCELL				* theChildCell;		// 020
	NiTArray<TESPathGridPoint*>	* nodes;			// 024
	void						* unk028;			// 028 exterior-specific?
	tListPtr<ExternalEdge>		externalEdges;		// 02C exterior-specific
	UInt16						nodeCount;			// 030
	UInt16						pad032;

	NiTPointerMap <PointList>	linkedPathPoints;	// 034 key is TESObjectREFR*
	NiTPointerMap <PointList>	pointsByPartition;	// 44
	/* each cell subdivided into 512x512 partitions for quick lookup of points in a particular area.
		key to pointsByArea based on (x,y) position in worldspace where x and y are cast to signed 16-bit integer:
			key = ((x >> 9) << 16) | (y >> 9)
		points marked as 'preferred' (blue) in editor always listed first
		if a partition contains no points there is no entry for it
	*/

	TESPathGridPoint* GetByIndex(UInt16 index);
	UInt16 IndexOf(TESPathGridPoint* pt);
	void SetLinkedPointsEnabled(TESObjectREFR* linkedRefr, bool bEnablePoints);
	bool SetPointDisabled(UInt16 index, bool bDisabled);
	bool SetPointPreferred(UInt16 index, bool bPreferred);
	PointList* GetListForPosition(float x, float y, bool bCreateIfNeeded = false);
	static UInt32 GetPartitionKey(float x, float y);
	bool SetEdge(UInt16 localID, UInt16 targetID, bool bEnableEdge, TESPathGrid* targetGrid = NULL);
	UInt16 AddNode(float x, float y, float z, bool bPreferred);	// returns new node index
	TESObjectREFR* GetLinkedRef(TESPathGridPoint* pt);
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESPathGrid) == 0x54);
#endif

// E0
class TESWorldSpace : public TESForm
{
public:
	TESWorldSpace();
	~TESWorldSpace();

	enum {
		// "Can't Wait" flag does not appear to be saved by editor
		kFlag_SmallWorld		= 1 << 0,
		kFlag_NoFastTravel		= 1 << 1,
		kFlag_OblivionWorld		= 1 << 2,
		kFlag_NoLODWater		= 1 << 4,
	};

	// bases
	TESFullName	fullName;	// 018
	TESTexture	texture;	// 024

	// members
	NiTPointerMap<TESObjectCELL>	* cellMap;	// 030 - key is coordinates of cell: (x << 16 + y)
	TESObjectCELL	* unk034;			// 034 - pointer to TESObjectCELL (always zero so far)
	NiTPointerMap <void>	map;		// 038 - NiTPointerMap<int, class TESTerrainLODQuadRoot *>
	TESWorldSpace	* worldSpace048;	// 048
	UInt32			unk04C[(0x58 - 0x4C) >> 2];	// 04C
	TESClimate		* climate;			// 058
	UInt32			worldFlags;			// 05C
	UInt32			unk060;				// 060
	NiTPointerMap <void> map064;		// 064 - $NiTPointerMap@IPAV?$BSSimpleList@PAVTESObjectREFR@@@@@@
	Character		* character;		// 074
	void			* ptr078;			// 078
	TESWorldSpace	* parentWorldspace;	// 07C
	UInt32			unk080[(0xC0 - 0x80) >> 2];	// 080
	BSStringT			editorID;			// 0C0
	NiTPointerMap <void> map0C8;		// 0C8 - ?$NiTPointerMap@I_N@@
	UInt32			unk0D8[(0xE0 - 0xD8) >> 2];	// 0D8

	TESObjectCELL * LookupCell(SInt32 x, SInt32 y) const;
	bool IsOblivionWorld() const { return (worldFlags & kFlag_OblivionWorld) ? true : false; }
	bool CanFastTravel() const { return (worldFlags & kFlag_NoFastTravel) ? false : true; }
	void SetCanFastTravel(bool bCan) { if (bCan) worldFlags &= ~kFlag_NoFastTravel; else worldFlags |= kFlag_NoFastTravel; }
};

// 28
class TESObjectLAND : public TESForm
{
public:
	TESObjectLAND();
	~TESObjectLAND();

	// A0
	struct Data
	{
		// 10
		struct Heightmap {
			struct Vert {
				// x, y range from -2048 to 2048
				float	x, y, z;
			};

			struct Quadrant {
				Vert	verts[17][17];
			};

			// order: LL, LR, UL, UR
			// 128 game units between each vert
			Quadrant	* quadrants[4];
		};

		// 10
		struct Unk0C {
			UInt8	data[0x1210][4];
		};

		// 10
		struct Unk10 {
			UInt8	data[0x124][4];	// alloc request size 121 padded to 124
		};

		// 20
		struct Unk30 {
			UInt8	data[0x20];
		};

		// 484
		struct Unk40 {
			UInt8	data[0x484];
		};

		UInt32				unk0;			// 00 ptr to some struct containing NiNode*
		Heightmap			* heightmap;	// 04
		void				* unk08;		// 08 normal data, 4 ptrs to 0xD8C bytes
		Unk0C				* unk0C;		// 0C vertex color data
		Unk10				* unk10;		// 10 vertex texture data
		UInt32				unk14;			// 14
		float				unk18;			// 18 init'd to FLT_MAX
		float				unk1C;			// 1C init'd to -FLT_MAX
		TESLandTexture		* textures[4];	// 20
		Unk30				* unk30[4];		// 30
		Unk40				* unk40[4];		// 40
		UInt32				unk50;			// 50
		NiTPointerMap<TESGrassAreaParam>	grassAreaParams[4];	// 54, key is UInt32
		UInt32				unk94;			// 94
		SInt32				cellCoordX;		// 98
		SInt32				cellCoordY;		// 9C
	};

	// base class
	TESChildCell	childCell;	// 018

	// members
	UInt8			landFlags;	// 1C
	UInt8			unk1D[3];
	TESObjectCELL	* cell;		// 020
	Data			* data;		// 024
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESObjectLAND) == 0x28);
STATIC_ASSERT(sizeof(TESObjectLAND::Data) == 0xA0);
#endif

// TLOD
// TESRoad
// DIAL
// INFO

// 68
class TESQuest : public TESForm
{
public:
	enum
	{
		// ### order may be wrong (haven't checked code yet)

		kModified_QuestFlags =	0x00000004,
			// CHANGE_QUEST_FLAGS

		kModified_QuestStage =	0x10000000,
			// CHANGE_QUEST_STAGES

		kModified_QuestScript =	0x08000000,
			// CHANGE_QUEST_SCRIPT
	};

	TESQuest();
	~TESQuest();

	// base classes
	TESScriptableForm	scriptable;	// 018
	TESIcon				icon;		// 024
	TESFullName			fullName;	// 030

	typedef tList<QuestStageItem> StageItemList;

	struct StageEntry
	{
		enum
		{
			kFlag_CompleteQuest	= 1 << 0,
		};

		UInt8			index;	// as defined in CS, not necessarily sequential
		UInt8			flags;
		UInt8			pad[2];
		StageItemList	itemList;
	};

	typedef tList<StageEntry> StageEntryList;

	struct TargetEntry
	{
		UInt32			unk00;			// 00 flags?
		ConditionEntry	conditionList;	// 04
		TESObjectREFR	* target;		// 0C
		TESObjectREFR	* doorRef;		// 10 door leading to this target if in another cell
		// possibly 1 more 32-bit member
	};

	typedef tList<TargetEntry> TargetEntryList;

	struct Unk50
	{
		UInt32	unk0;	// 000
		UInt32	unk1;	// 004
	};

	enum {
		kQuestFlag_Active			= 1 << 0,
		kQuestFlag_Completed		= 1 << 1,
		kQuestFlag_RepeatConv		= 1 << 2,
		kQuestFlag_RepeatStages		= 1 << 3
	};

	// members
	UInt8		questFlags;	// 03C
	UInt8		priority;	// 03D
	UInt8		pad0[2];	// 03E
	StageEntryList	stageList;	// 040
	TargetEntryList	targetList;	// 048
	Unk50		unk4;		// 050
	ScriptEventList	* scriptEventList;	// 058
	UInt8		stageIndex;	// 05C
	UInt8		pad1[3];	// 05D
	BSStringT		editorName;	// 060

	void SetCompleted(bool bComplete) { questFlags = bComplete ? questFlags | kQuestFlag_Completed : questFlags & ~kQuestFlag_Completed; }
	bool IsCompleted() const { return (questFlags & kQuestFlag_Completed) ? true : false; }
	StageEntry* GetStageEntry(UInt32 index);
};

// TESIdleForm

// 03C
class TESPackage : public TESForm
{
public:
	TESPackage();
	~TESPackage();

	enum
	{
		kPackageFlag_OffersServices =			1 << 0,
		kPackageFlag_MustReachLocation =		1 << 1,
		kPackageFlag_MustComplete =				1 << 2,
		kPackageFlag_LockDoorsAtStart =			1 << 3,
		kPackageFlag_LockDoorsAtEnd =			1 << 4,
		kPackageFlag_LockDoorsAtLocation =		1 << 5,
		kPackageFlag_UnlockDoorsAtStart =		1 << 6,
		kPackageFlag_UnlockDoorsAtEnd =			1 << 7,
		kPackageFlag_UnlockDoorsAtLocation =	1 << 8,
		kPackageFlag_ContinueIfPCNear =			1 << 9,
		kPackageFlag_OncePerDay =				1 << 10,
		kPackageFlag_Unk11 =					1 << 11,
		kPackageFlag_SkipFalloutBehavior =		1 << 12,
		kPackageFlag_AlwaysRun =				1 << 13,
		kPackageFlag_Unk14 =					1 << 14,
		kPackageFlag_Unk15 =					1 << 15,
		kPackageFlag_Unk16 =					1 << 16,
		kPackageFlag_AlwaysSneak =				1 << 17,
		kPackageFlag_AllowSwimming =			1 << 18,
		kPackageFlag_AllowFalls =				1 << 19,
		kPackageFlag_ArmorUnequipped =			1 << 20,
		kPackageFlag_WeaponsUnequipped =		1 << 21,
		kPackageFlag_DefensiveCombat =			1 << 22,
		kPackageFlag_UseHorse =					1 << 23,
		kPackageFlag_NoIdleAnims =				1 << 24,
		kPackageFlag_Unk25 =					1 << 25,
		kPackageFlag_Unk26 =					1 << 26,
		kPackageFlag_Unk27 =					1 << 27,
		kPackageFlag_Unk28 =					1 << 28,
		kPackageFlag_Unk29 =					1 << 29,
		kPackageFlag_Unk30 =					1 << 30,
		kPackageFlag_Unk31 =					1 << 31
	};

	enum
	{
		kPackageType_Find =	0,
		kPackageType_Follow,
		kPackageType_Escort,
		kPackageType_Eat,
		kPackageType_Sleep,
		kPackageType_Wander,
		kPackageType_Travel,
		kPackageType_Accompany,
		kPackageType_UseItemAt,
		kPackageType_Ambush,
		kPackageType_FleeNotCombat,
		kPackageType_CastMagic,
		kPackageType_CombatController,	// Actor::GetCombatController returns package only if type matches this
		kPackType_Unk0D,				// also related to dynamic combat packages?

		kPackType_MAX
	};

	// 8
	struct Time
	{
		enum
		{
			kDay_Any = 0,
			kTime_Any = 0xFF,
		};

		enum
		{
			kMonth_January = 0,
			kMonth_February,
			kMonth_March,
			kMonth_April,
			kMonth_May,
			kMonth_June,
			kMonth_July,
			kMonth_August,
			kMonth_September,
			kMonth_October,
			kMonth_November,
			kMonth_December,
			kMonth_Spring,	// march, april, may
			kMonth_Summer,	// june, july, august
			kMonth_Fall,	// september, august, november
			kMonth_Winter,	// december, january, february

			kMonth_Any = 0xFF,
		};

		enum
		{
			kWeekday_Sundas = 0,
			kWeekday_Morndas,
			kWeekday_Tirdas,
			kWeekday_Middas,
			kWeekday_Turdas,
			kWeekday_Fredas,
			kWeekday_Loredas,
			kWeekday_Weekdays,
			kWeekday_Weekends,
			kWeekday_MWF,
			kWeekday_TT,

			kWeekday_Any = 0xFF
		};

		UInt8	month;
		UInt8	weekDay;
		UInt8	date;
		UInt8	time;
		UInt32	duration;

		static const char* MonthForCode(UInt8 monthCode);
		static const char* DayForCode(UInt8 dayCode);
		static UInt8 CodeForMonth(const char* monthStr);
		static UInt8 CodeForDay(const char* dayStr);
		static bool IsValidMonth(UInt8 m) { return (m+1) <= kMonth_Winter; }
		static bool IsValidTime(UInt8 t) { return (t+1) <= 24; }
		static bool IsValidDay(UInt8 d) { return (d+1) <= kWeekday_TT; }
		static bool IsValidDate(UInt8 d) { return d <= 31; }
	};

	union ObjectType
	{
		TESForm			* form;
		TESObjectREFR	* refr;
		UInt32			objectCode;
	};

	// order only somewhat related to kFormType_XXX (values off by 17, 20, or 21)
	enum
	{
		kObjectType_Activator		= 0x01,
		kObjectType_Apparatus,
		kObjectType_Armor,
		kObjectType_Book,
		kObjectType_Clothing,
		kObjectType_Container,
		kObjectType_Door,
		kObjectType_Ingredient,
		kObjectType_Light,
		kObjectType_Misc,
		kObjectType_Flora,
		kObjectType_Furniture,
		kObjectType_WeaponAny,
		kObjectType_Ammo,
		kObjectType_NPC,
		kObjectType_Creature,		// 10
		kObjectType_Soulgem,
		kObjectType_Key,
		kObjectType_Alchemy,
		kObjectType_Food,
		kObjectType_CombatWearable,
		kObjectType_Wearable,
		kObjectType_WeaponNone,
		kObjectType_WeaponMelee,
		kObjectType_WeaponRanged,
		kObjectType_SpellsAny,
		kObjectType_SpellsTarget,
		kObjectType_SpellsTouch,
		kObjectType_SpellsSelf,
		kObjectType_SpellsAlteration,
		kObjectType_SpellsConjuration,
		kObjectType_SpellsDestruction,	// 20
		kObjectType_SpellsIllusion,
		kObjectType_SpellsMysticism,
		kObjectType_SpellsRestoration,

		kObjectType_Max				= 0x24,
	};

	struct LocationData
	{
		enum {
			kPackLocation_NearReference		= 0,
			kPackLocation_InCell			= 1,
			kPackLocation_CurrentLocation	= 2,
			kPackLocation_EditorLocation	= 3,
			kPackLocation_ObjectID			= 4,
			kPackLocation_ObjectType		= 5,

			kPackLocation_Max,
		};

		UInt8		locationType;
		UInt8		pad[3];
		UInt32		radius;
		ObjectType  object;

		static LocationData* Create();
		static const char* StringForLocationCode(UInt8 locCode);
		static UInt8 LocationCodeForString(const char* locStr);
		static bool IsValidLocationType(UInt8 locCode) { return locCode < kPackLocation_Max; }
	};

	enum
	{
		kTargetType_Refr		= 0,
		kTargetType_BaseObject	= 1,
		kTargetType_TypeCode	= 2,
	};

	struct TargetData
	{
		UInt8		targetType;
		UInt8		pad[3];
		ObjectType	target;
		UInt32		count;

		static TargetData* Create();
		static const char* StringForTargetCode(UInt8 targetCode);
		static UInt8 TargetCodeForString(const char* targetStr);
		static bool IsValidTargetCode(UInt8 c) { return c <= TESPackage::kTargetType_TypeCode; }
	};

	enum eProcedure {			// UInt32
		kProcedure_TRAVEL = 0,
		kProcedure_WANDER,
		kProcedure_ACTIVATE,
		kProcedure_AQUIRE,	// sic
		kProcedure_SLEEP,
		kProcedure_EAT,
		kProcedure_FOLLOW,
		kProcedure_ESCORT,
		kProcedure_ALARM,
		kProcedure_COMBAT,
		kProcedure_FLEE,
		kProcedure_YIELD,
		kProcedure_DIALOGUE,
		kProcedure_WAIT,
		kProcedure_TRAVEL_TARGET,
		kProcedure_PURSUE,
		kProcedure_GREET,					// 0x10
		kProcedure_CREATE_FOLLOW,
		kProcedure_OBSERVE_COMBAT,
		kProcedure_OBSERVE_DIALOGUE,
		kProcedure_GREET_DEAD,
		kProcedure_WARN,
		kProcedure_GET_UP,
		kProcedure_MOUNT_HORSE,
		kProcedure_DISMOUNT_HORSE,
		kProcedure_DO_NOTHING,
		kProcedure_CAST_SPELL,
		kProcedure_AIM,
		kProcedure_NOTIFY,
		kProcedure_ACCOMPANY,
		kProcedure_USE_ITEM_AT,
		kProcedure_FEED,
		kProcedure_AMBUSH_WAIT,				// 0x20
		kProcedure_SURFACE,
		kProcedure_WAIT_FOR_SPELL,
		kProcedure_CHOOSE_CAST,
		kProcedure_FLEE_NON_COMBAT,
		kProcedure_REMOVE_WORN_ITEMS,
		kProcedure_SEARCH,
		kProcedure_CLEAR_MOUNT_POSITION,
		kProcedure_SUMMON_CREATURE_DEFEND,
		kProcedure_MOVEMENT_BLOCKED,
		kProcedure_UNEQUIP_ARMOR,
		kProcedure_DONE,					// 0x2B

		kProcedure_MAX						// 0x2C
	};

	UInt32			procedureArrayIndex;	// 018 index into array of array of eProcedure terminated by 0x2C.
											//	   -1 if no procedure array exists for package type.
	UInt32			packageFlags;			// 01C
	UInt8			type;					// 020
	UInt8			pad021[3];				// 021
	LocationData	* location;				// 024
	TargetData		* target;				// 028
	Time			time;					// 02C
	ConditionEntry	conditionList;			// 034

	void SetTarget(TESObjectREFR* refr);
	void SetTarget(TESForm* baseForm, UInt32 count);
	void SetTarget(UInt8 typeCode, UInt32 count);
	TargetData* GetTargetData();
	LocationData* GetLocationData();

	bool IsFlagSet(UInt32 flag);
	void SetFlag(UInt32 flag, bool bSet);

	static const char* StringForPackageType(UInt32 pkgType);
	static const char* StringForObjectCode(UInt8 objCode);
	static UInt8 ObjectCodeForString(const char* objString);
	static bool IsValidObjectCode(UInt8 o) { return o < kObjectType_Max; }
	static const char* StringForProcedureCode(eProcedure proc, bool bRemovePrefix = true);
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESPackage) == 0x3C);
#endif

// 98
class TESCombatStyle : public TESForm
{
public:
	TESCombatStyle();
	~TESCombatStyle();

	// ###TODO: decode flags

	enum {
		kAttackChoice_Normal,
		kAttackChoice_Forward,
		kAttackChoice_Back,
		kAttackChoice_Left,
		kAttackChoice_Right
	};

	// 54
	struct ExtraSettings
	{
		float	dodgeFatigueModMult;
		float	dodgeFatigueModBase;
		float	encumSpeedModBase;
		float	encumSpeedModMult;
		float	dodgeUnderAttackMult;
		float	dodgeNotUnderAttackMult;
		float	dodgeBackUnderAttackMult;
		float	dodgeBackNotUnderAttackMult;
		float	dodgeFWAttackingMult;
		float	dodgeFWNotAttackingMult;
		float	blockSkillModMult;
		float	blockSkillModBase;
		float	blockUnderAttackMult;
		float	blockNotUnderAttackMult;
		float	attackSkillModMult;
		float	attackSkillModBase;
		float	attackUnderAttackMult;
		float	attackNotUnderAttackMult;
		float	attackDuringBlockMult;
		float	powerAttackFatigueModBase;
		float	powerAttackFatigueModMult;
	};

	// Do Not Acquire flag doesn't appear to exist
	enum
	{
		kFlag_HasAdvancedData		= 0x01,
		kFlag_IgnoreAlliesInArea	= 0x04,
		kFlag_WillYield				= 0x08,
		kFlag_RejectsYields			= 0x10,
		kFlag_FleeingDisabled		= 0x20,
		kFlag_PrefersRanged			= 0x40,
		kFlag_MeleeAlertOK			= 0x80
	};

	UInt8		dodgeChance;			// 18
	UInt8		LRChance;
	UInt8		pad1A[2];
	float		dodgeLRTimerMin;		// 1C
	float		dodgeLRTimerMax;		// 20
	float		dodgeFWTimerMin;		// 24
	float		dodgeFWTimerMax;		// 28
	float		dodgeBackTimerMin;		// 2C
	float		dodgeBackTimerMax;		// 30
	float		idleTimerMin;			// 34
	float		idleTimerMax;			// 38
	UInt8		blockChance;			// 3C
	UInt8		attackChance;
	UInt8		pad3E[2];
	float		staggerBonusToAttack;	// 40
	float		KOBonusToAttack;		// 44
	float		H2HBonusToAttack;		// 48
	UInt8		powerAttackChance;		// 4C
	UInt8		pad4D[3];
	float		staggerBonusToPower;	// 50
	float		KOBonusToPower;			// 54
	UInt8		attackChoiceChances[5];	// 58
	UInt8		pad5D[3];
	float		holdTimerMin;			// 60
	float		holdTimerMax;			// 64
	UInt8		styleFlags;				// 68
	UInt8		acroDodgeChance;
	UInt8		pad6A[2];
	float		rangeMultOptimal;		// 6C
	float		rangeMultMax;			// 70
	float		switchDistMelee;		// 74
	float		switchDistRanged;		// 78
	float		buffStandoffDist;		// 7C
	float		rangedStandoffDist;		// 80
	float		groupStandoffDist;		// 84
	UInt8		rushAttackChance;		// 88
	UInt8		pad89[3];
	float		rushAttackDistMult;		// 8C
	UInt32		unk90;					// 90
	ExtraSettings	* extraSettings;	// 94
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESCombatStyle) == 0x98);
STATIC_ASSERT(sizeof(TESCombatStyle::ExtraSettings) == 0x54);
#endif

// 3C
class TESLoadScreen : public TESForm
{
public:
	TESLoadScreen();
	~TESLoadScreen();

	// bases
	TESTexture		texture;			// 18
	TESDescription	description;		// 24

	// members
	UInt32			unk28[3];			// 28
	BSStringT			unk34;				// 34
};

// 34
class TESLevSpell : public TESBoundObject
{
public:
	TESLevSpell();
	~TESLevSpell();

	TESLeveledList	leveledList;	// 24
};

// AC
class TESWaterForm : public TESForm
{
public:
	TESWaterForm();
	~TESWaterForm();

	enum {								// use to index into simValues array
		kWaterVal_WindVelocity = 0,
		kWaterVal_WindDirection,
		kWaterVal_WaveAmplitude,
		kWaterVal_WaveFrequency,
		kWaterVal_SunPower,
		kWaterVal_Reflectivity,
		kWaterVal_FresnelAmount,
		kWaterVal_ScrollX,
		kWaterVal_ScrollY,
		kWaterVal_FogDistNear,
		kWaterVal_FogDistFar,

		kWaterVal_Max
	};

	TESAttackDamageForm	damageForm;			// 18
	TESTexture			texture;			// 20

	UInt8				opacity;			// 2C init'ed to 0x4B
	UInt8				unk2D;				//    looks like flags
	UInt8				pad2E[2];
	UInt32				unk30;				// 30
	UInt16				unk34;				// 34
	UInt16				unk36;
	TESSound			* loopSound;		// 38
	float				waterSimVals[11];	// 3C .. 64
	UInt32				shallowColorRGB;	// 68
	UInt32				deepColorRGB;		// 6C
	UInt32				reflectColorRGB;	// 70
	UInt32				textureBlend;		// 74
	float				rainSimVals[5];		// 78 .. 88
	float				displacementSimVals[5];	// 8C .. 9C
	UInt32				unkA0[3];			// A0 .. A8 look like pointers to day/night/underwater water forms
};
// global TESWaterForm* at 0x00B360AC in v1.2.416 - default water?

// 34
class TESObjectANIO : public TESForm
{
public:
	TESObjectANIO();
	~TESObjectANIO();

	// bases
	TESModel		model;		// 18

	// members
	TESIdleForm		* idleForm;	// 30
};

// 110
class TESEffectShader : public TESForm
{
public:
	TESEffectShader();
	~TESEffectShader();

	struct BlendData
	{
		// ###TODO: order, enum values
		UInt32		unk00[4];
	};

	struct MembraneData
	{
		RGBA	color;
		float	alphaFadeInTime;
		float	fullAlphaTime;
		float	alphaFadeOutTime;
		float	persistentAlphaRatio;
		float	alphaPulseAmplitude;
		float	alphaPulseFrequency;
	};

	struct Vec3
	{
		float x, y, z;
	};

	struct ParticleData
	{
		float	birthRampUpTime;
		float	birthFullTime;
		float	birthRampDownTime;
		float	birthFullRatio;
		float	birthPersistentRatio;
		float	lifeTime;
		float	lifeTimeDelta;
		float	initialSpeedAlongNormal;
		float	accelerationAlongNormal;
		float	initialVelocity[3];
		float	acceleration[3];
		float	scaleKeys[2];
		float	scaleKeyTimes[2];
		RGBA	colorKeys[3];
		float	colorAlpha[3];
		float	colorKeyTimes[3];
	};

	BlendData			membraneBlendData;		// 18
	MembraneData		skinMembraneShader;		// 28
	float				textureAnimSpeedU;		// 44
	float				textureAnimSpeedV;		// 48

	float				fallOff;				// 4C
	MembraneData		edgeMembraneShader;		// 50

	float				skinFullAlphaRatio;		// 6C
	float				edgeFullAlphaRatio;		// 70

	BlendData			particleBlendData;		// 74
	UInt32				unk84;					// 84
	ParticleData		particleShader;			// 88

	TESTexture			texture0F8;				// F8 ###TODO order
	TESTexture			texture104;				// 104
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESEffectShader) == 0x110);
#endif

class TESTopicInfo;

// 24
class TopicInfoArray : public NiTLargeArray<TESTopicInfo*>
{
public:
	TopicInfoArray();
	~TopicInfoArray();

	virtual void		Destructor(bool Destroy);
};

// 3C
class TESTopic : public  TESForm
{
public:
	TESTopic();
	~TESTopic();

	struct QuestInfoData
	{
		TESQuest*			parentQuest;
		TopicInfoArray		infoList;
	};

	struct QuestInfoEntry
	{
		QuestInfoData*		data;
		QuestInfoEntry*		next;
	};

	 enum
	 {
		kTopicType_Topic = 0,
		kTopicType_Conversation,
		kTopicType_Combat,
		kTopicType_Persuasion,
		kTopicType_Detection,
		kTopicType_Service,
		kTopicType_Miscellaneous
	 };

	// bases
	TESFullName			fullname;			// 18 name of topic as seen in-game

	// members
	UInt8				topicType;			// 24
	UInt8				pad25[3];			// 25
	QuestInfoData*		firstEntry;			// 28
	QuestInfoEntry*		questInfoList;		// 2C
	void*				unk30;				// 30 subrecord type XIDX saved here. seen NULL most of the time
	BSStringT				editorID;			// 34
};

bool IsClonedForm(UInt32 formID);
TESForm* CloneForm(TESForm* formToClone);

// issues with run-time size and with header dependencies need to be resolved before this is usable
#if 0

// 38 / A0
class TESTopicInfo : public TESForm
{
public:
	TESTopicInfo();
	~TESTopicInfo();

	struct LinkedTopics
	{
		tList<TESTopic>		topicsLinkedFrom;
		tList<TESTopic>		topicsLinkedTo;		// doubles as choices for appropriate infotype
	};

	 enum
	 {
		kInfoType_Topic = 0,
		kInfoType_Conversation,
		kInfoType_Combat,
		kInfoType_Persuasion,
		kInfoType_Detection,
		kInfoType_Service,
		kInfoType_Miscellaneous
	 };

	 enum
	 {
		kFlags_Goodbye = 0x0001,
		kFlags_Random = 0x0002,
		kFlags_SayOnce = 0x0004,
		kFlags_Unk008 = 0x0008,
		kFlags_InfoRefusal = 0x0010,
		kFlags_RandomEnd = 0x0020,
		kFlags_RunforRumors = 0x0040
	 };

	struct ResponseEntry
	{
		// 18 / 24
		struct Data
		{
			enum
			{
				kEmotionType_Neutral = 0,
				kEmotionType_Anger,
				kEmotionType_Disgust,
				kEmotionType_Fear,
				kEmotionType_Sad,
				kEmotionType_Happy,
				kEmotionType_Surprise,
			};

			UInt32			emotionType;				// 00
			UInt32			emotionValue;				// 04
			UInt32			unk08;						// 08
			UInt32			unk0C;						// 0C
			String			responseText;				// 10
#ifndef OBLIVION
			String			actorNotes;					// 18
			UInt32			unk20;						// 20
#endif
		};

		Data*				data;
		ResponseEntry*		next;
	};

#ifdef OBLIVION

	ConditionEntry			conditions;		// 18
	UInt16					unk20;			// 20 -	init to -1
	UInt8					unk22;			// 22
	UInt16					infotype;		// 23
	UInt8					flags0;			// 25
	UInt8					pad26[2];		// 26
	tList<TESTopic>			addedTopics;	// 28
	LinkedTopics*			linkedTopics;	// 30
	UInt32					unk34;			// 34 - init to parent plugin's TESFile::unk25C.
											// appears to be a file offset that can be passed to TESFile::SetFilePointer()
#else

	TESTopic*			unk24;			// 24 - always NULL ?
	ConditionEntry		conditions;		// 28
	UInt16				unk30;			// 30 - init to -1. used to determine previous info
	UInt16				infotype;		// 32
	UInt8				flags0;			// 34
	UInt8				pad35[3];		// 35
	TopicListEntry		addedTopics;	// 38
	LinkedTopics*		linkedTopics;	// 40
	ResponseEntry		responses;		// 44
	Script				resultScript;	// 4C

#endif
};

#if OBLIVION
STATIC_ASSERT(sizeof(TESTopicInfo) == 0x38);	// this is wrong
#else
STATIC_ASSERT(sizeof(TESTopicInfo) == 0xA0);
#endif

#endif

// 18
class TESModelAnim : public TESModel
{
public:
	TESModelAnim();
	~TESModelAnim();
};

// 18+?
class NiFormArray : public NiTLargeArray<class TESForm *>
{
public:
	NiFormArray();
	~NiFormArray();
};

// 48 / 68
class TESIdleForm : public TESForm
{
public:
	TESIdleForm();
	~TESIdleForm();

	enum
	{
		kFlags_LowerBody = 0,
		kFlags_LeftArm,
		kFlags_LeftHand,
		kFlags_RightArm,
		kFlags_SpecialIdle,
		kFlags_WholeBody,
		kFlags_UpperBody,

		kFlags_MustReturnFile = 0x7F
	};

	// bases
	TESModelAnim	animModel;		// 18

	// members
	ConditionEntry	conditions;		// 30
	UInt32			animFlags;		// 38
	NiFormArray*	children;		// 3C
	TESIdleForm*	parent;			// 40
	TESIdleForm*	previous;		// 44
};
