#pragma once

// Added to remove a cyclic dependency between GameForms.h and GameExtraData.h

/****
 *	id	size	type					Decoded
 *	00	?		
 *	01	10+?	ExtraEditor?		
 *	02	14		ExtraHavok
 *	03	10		ExtraCell3D
 *	04	10		ExtraWaterHeight			*
 *	05	10		ExtraCellWaterType			*
 *	06	14		
 *	07	?		
 *	08	10		ExtraRegionList				*
 *	09	10		ExtraSeenData				*
 *	0A	?		ExtraEditorID
 *	0B	10		ExtraCellMusicType			*
 *	0C	10		ExtraCellClimate			*
 *	0D	?		ExtraProcessMiddleLow
 *	0E	14		ExtraEditorCommonDialogData *		
 *	0F	1C		ExtraCellCanopyShadowMask
 *	10	10		ExtraDetachTime
 *	11	10		ExtraPersistentCell			*
 *	12	14		ExtraScript					*
 *	13	14		ExtraAction					
 *	14	24		ExtraStartingPosition		*
 *	15	10		ExtraAnim
 *	16	10										reference-related		
 *	17	10		ExtraUsedMarkers
 *	18	18		ExtraDistantData
 *	19	10		ExtraRagDollData
 *	1A	10		ExtraContainerChanges		*
 *	1B	C		ExtraWorn					*
 *	1C	C		ExtraWornLeft				*
 *	1D	?		
 *	1E	20		ExtraPackageStartLocation	*
 *	1F	1C		ExtraPackage				*
 *	20	10		ExtraTresPassPackage		*
 *	21	10		ExtraRunOncePacks
 *	22	10		ExtraReferencePointer
 *	23	10		ExtraFollower				*
 *	24	10		ExtraLevCreaModifier
 *	25	C		ExtraGhost
 *	26	10		ExtraOriginalReference		*
 *	27	10		ExtraOwnership				*
 *	28	10		ExtraGlobal					*
 *	29	10		ExtraRank					*
 *	2A	10		ExtraCount					*
 *	2B	10		ExtraHealth					*
 *	2C	10		ExtraUses					*
 *	2D	10		ExtraTimeLeft				*
 *	2E	10		ExtraCharge					*
 *	2F	10		ExtraSoul					*
 *	30	10		ExtraLight					*
 *	31	10		ExtraLock					*
 *	32	?		ExtraTeleport				*
 *	33	?		ExtraMapMarker
 *	34	?										animation-related?
 *	35	C		ExtraLeveledCreature
 *	36	14		ExtraLeveledItem
 *	37	10		ExtraScale					*
 *	38	10		ExtraSeed
 *	39	?		
 *	3A	?		NonActorMagicTarget
 *	3B	10										reference-related		
 *	3C	?		
 *	3D	?		ExtraCrimeGold				*
 *	3E	?		ExtraOblivionEntry
 *	3F	14		ExtraEnableStateParent		*
 *	40	?		ExtraEnableStateChildren	*
 *	41	?		ExtraItemDropper
 *	42	14		ExtraDroppedItemList
 *	43	10		ExtraRandomTeleportMarker	*
 *	44	10		ExtraMerchantContainer		*
 *	45	?		
 *	46	1C		ExtraPersuasionPercent
 *	47	C		ExtraCannotWear
 *	48	10		ExtraPoison					*
 *	49	?										reference-related
 *	4A	?		ExtraLastFinishedSequence
 *	4B	?		ExtraSavedMovementData
 *	4C	10		ExtraNorthRotation			*
 *	4D	10		ExtraXTarget
 *	4E	10		ExtraFriendHitList
 *	4F	10		ExtraHeadingTarget
 *	50	C		ExtraBoundArmor
 *	51	10		ExtraRefractionProperty		*
 *	52	?		ExtraInvestmentGold
 *	53	10		ExtraStartingWorldOrCell
 *	54	?		
 *	55	10		ExtraQuickKey				*
 *	56	14		ExtraEditorRef3DData		*	Editor only		
 *	57	30		ExtraEditorRefMoveData      
 *	58	10		ExtraTravelHorse			*
 *	59	10		ExtraInfoGeneralTopic
 *	5A	10		ExtraHasNoRumors
 *	5B	?		ExtraSound
 *	5C	10		ExtraHaggleAmount
 *	more?
 *	
 *	bits - seems to be linear bitfield
 *	+08	0000000x	00	?
 *	+09	0000000x	08	region list?
 *	+0A	0000000x	10	detach time?
 *	+0B	0000000x	18	distant data?
 *	+0C	0000000x	20	tresspass package?
 *	+0D	0000000x	28	global?
 *	+0E	0000000x	30	light?
 *	+0E	00x00000	35	leveled creature
 *	+0F	0000000x	38	seed?
 *	+10	0000000x	40	enable state children?
 *	+10	x0000000	47	cannot wear
 *	+12	0000000x	50	bound armor
 *	+13	0000000x	58	travel horse
 *	+13	x0000000	5F	?
 **/

enum ExtraDataType
{
	kExtraData_Havok =					0x02,
	kExtraData_Cell3D =					0x03,
	kExtraData_WaterHeight =			0x04,
	kExtraData_CellWaterType =			0x05,
	kExtraData_RegionList =				0x08,
	kExtraData_SeenData =				0x09,
	kExtraData_EditorID =				0x0A,
	kExtraData_CellMusicType =			0x0B,
	kExtraData_CellClimate =			0x0C,
	kExtraData_ProcessMiddleLow =		0x0D,
	kExtraData_EditorCommonDialogData = 0x0E,
	kExtraData_CellCanopyShadowMask =	0x0F,
	kExtraData_DetachTime =				0x10,
	kExtraData_PersistentCell =			0x11,
	kExtraData_Script =					0x12,
	kExtraData_Action =					0x13,
	kExtraData_StartingPosition =		0x14,
	kExtraData_Anim =					0x15,

	kExtraData_UsedMarkers =			0x17,
	kExtraData_DistantData =			0x18,
	kExtraData_RagDollData =			0x19,
	kExtraData_ContainerChanges =		0x1A,
	kExtraData_Worn =					0x1B,
	kExtraData_WornLeft =				0x1C,

	kExtraData_StartLocation =			0x1E,
	kExtraData_Package =				0x1F,
	kExtraData_TresPassPackage =		0x20,
	kExtraData_RunOncePacks =			0x21,
	kExtraData_ReferencePointer =		0x22,
	kExtraData_Follower =				0x23,
	kExtraData_LevCreaModifier =		0x24,
	kExtraData_Ghost =					0x25,
	kExtraData_OriginalReference =		0x26,
	kExtraData_Ownership =				0x27,
	kExtraData_Global =					0x28,
	kExtraData_Rank =					0x29,
	kExtraData_Count =					0x2A,
	kExtraData_Health =					0x2B,
	kExtraData_Uses =					0x2C,
	kExtraData_TimeLeft =				0x2D,
	kExtraData_Charge =					0x2E,
	kExtraData_Soul =					0x2F,
	kExtraData_Light =					0x30,
	kExtraData_Lock =					0x31,
	kExtraData_Teleport =				0x32,
	kExtraData_MapMarker =				0x33,

	kExtraData_LeveledCreature =		0x35,
	kExtraData_LeveledItem =			0x36,
	kExtraData_Scale =					0x37,
	kExtraData_Seed =					0x38,
	kExtraData_NonActorMagicCaster =	0x39,

	kExtraData_CrimeGold =				0x3D,
	kExtraData_OblivionEntry =			0x3E,
	kExtraData_EnableStateParent =		0x3F,
	kExtraData_EnableStateChildren =	0x40,
	kExtraData_ItemDropper =			0x41,
	kExtraData_DroppedItemList =		0x42,
	kExtraData_RandomTeleportMarker =	0x43,
	kExtraData_MerchantContainer =		0x44,

	kExtraData_PersuasionPercent =		0x46,
	kExtraData_CannotWear =				0x47,
	kExtraData_Poison =					0x48,

	kExtraData_LastFinishedSequence =	0x4A,
	kExtraData_SavedMovementData =		0x4B,
	kExtraData_NorthRotation =			0x4C,
	kExtraData_XTarget =				0x4D,
	kExtraData_FriendHitList =			0x4E,
	kExtraData_HeadingTarget =			0x4F,
	kExtraData_BoundArmor =				0x50,
	kExtraData_RefractionProperty =		0x51,
	kExtraData_InvestmentGold =			0x52,
	kExtraData_StartingWorldOrCell =	0x53,

	kExtraData_QuickKey =				0x55,

	kExtraData_EditorRefMoveData =		0x57,
	kExtraData_TravelHorse =			0x58,
	kExtraData_InfoGeneralTopic =		0x59,
	kExtraData_HasNoRumors =			0x5A,
	kExtraData_ExtraSound =				0x5B,
	kExtraData_HaggleAmount =			0x5C,
};

class TESForm;

// C+?
class BSExtraData
{
public:
	BSExtraData();
	virtual ~BSExtraData();

	virtual bool	Compare(BSExtraData* compareWith);	// compare type, data, return true if not equal

	static BSExtraData* Create(UInt8 xType, UInt32 size, UInt32 vtbl);

//	void		** _vtbl;	// 000
	UInt8		type;		// 004
	UInt8		pad[3];		// 005
	BSExtraData	* next;		// 008
};

// 014+
struct BaseExtraList
{
	virtual	void Destroy(bool bNoDealloc);	// removes and frees all of m_data

	bool			HasType(UInt32 type) const;
	BSExtraData *	GetByType(UInt32 type) const;

	void			MarkType(UInt32 type, bool bCleared);
	bool			Remove(BSExtraData* toRemove);
	bool			RemoveByType(UInt32 type);
	bool			Add(BSExtraData* toAdd);
	void			RemoveAll();

	bool			MarkScriptEvent(UInt32 eventMask, TESForm* eventTarget);

	void			Copy(BaseExtraList* from);

	void			DebugDump();

	bool			IsWorn();

	// void		** m_vtbl;					// 000
	BSExtraData	* m_data;					// 004
	UInt8		m_presenceBitfield[0x0C];	// 008 - if a bit is set, then the extralist should contain that extradata
											// bits are numbered starting from the lsb
};

struct ExtraDataList : public BaseExtraList
{
	static ExtraDataList * Create();
	//
};
