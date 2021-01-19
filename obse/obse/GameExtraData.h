#pragma once

#include "GameBSExtraData.h"
#include "GameForms.h"

// ### incomplete

#include "Utilities.h"
#include <vector>

class AlchemyItem;
class TESForm;
class TESObjectREFR;
class TESObjectCELL;
class TESPackage;
class Character;
class NiAVObject;
class TESKey;
class TESFaction;
class TESNPC;
class TESGlobal;
class TESClimate;
class TrespassPackage;
class TESRegionList;
class Script;
struct ScriptEventList;
class NiLines;
class NiNode;
class ActorAnimData;
class bhkWorld;

class ExtraContainerChanges : public BSExtraData
{
public:
	ExtraContainerChanges();
	virtual ~ExtraContainerChanges();

	struct EntryData
	{
		tList<ExtraDataList>* extendData;
		SInt32			      countDelta;
		TESForm*              type;

		static EntryData* Create(SInt32 countDelta, TESForm* type);
		void Cleanup();
		EntryExtendData * Add(EntryExtendData* newData);
		EntryExtendData * Add(ExtraDataList* newList);
		bool Remove(EntryExtendData* toRemove, bool bFree);
	};

	struct Data
	{
		tList<EntryData>*   objList;
		TESObjectREFR*      owner;
		float	            totalWeight;	// of all items in inventory. cached, is -1 if needs to be recalculated
		float			    armorWeight;	// weight of equipped armor. cached as above. Both take armor weight perks into account

		static Data* Create(TESObjectREFR* owner);
		void RunScripts();		// executes scripts attached to any scriptable objects in container
		// thisstdcall float Data::GetArmorWeight(Actor* actor) = 0x00488280
	};

	Data	* data;

	EntryData *	GetByType(TESForm * type);
	void		DebugDump();
	void		Cleanup();	// clean up unneeded extra data from each EntryData
	EntryExtendData * Add(TESForm* form, ExtraDataList* dataList);
	ExtraDataList* SetEquipped(TESForm* obj, bool bEquipped);

	// get EntryData and ExtendData for all equipped objects, return num objects equipped
	UInt32		GetAllEquipped(std::vector<EntryData*>& outEntryData, std::vector<EntryExtendData*>& outExtendData);

	static ExtraContainerChanges* Create();
	static ExtraContainerChanges* GetForRef(TESObjectREFR* refr);
};

typedef Visitor<ExtraContainerChanges::Entry, ExtraContainerChanges::EntryData> ExtraEntryVisitor;
typedef Visitor<ExtraContainerChanges::EntryExtendData, ExtraDataList> ExtendDataVisitor;

// cell and position where the current package was started?
class ExtraPackageStartLocation : public BSExtraData
{
public:
	ExtraPackageStartLocation();
	virtual ~ExtraPackageStartLocation();

	TESForm			* cell;		// 0C can be worldspace or cell
	float			x, y, z;	// 10
	UInt32			pad1C;		// 1C
};

// current package
class ExtraPackage : public BSExtraData
{
public:
	ExtraPackage();
	virtual ~ExtraPackage();

	TESPackage	* package;	// 0C
	UInt32		unk10;		// 10
	UInt32		unk14;		// 14
	UInt32		unk18;		// 18
};

// list of all characters following the owner via a package
class ExtraFollower : public BSExtraData
{
public:
	ExtraFollower();
	virtual ~ExtraFollower();

	struct ListNode
	{
		Character	* character;
		ListNode	* next;

		Character* Info() const { return character; }
		ListNode* Next() const { return next; }
	};

	ListNode	* followers;
};

typedef Visitor<ExtraFollower::ListNode, Character> ExtraFollowerVisitor;

class ExtraHealth : public BSExtraData
{
public:
	ExtraHealth();
	virtual ~ExtraHealth();
	float health;

	static ExtraHealth* Create();
};

class ExtraUses : public BSExtraData
{
public:
	ExtraUses();
	~ExtraUses();
	UInt32 unk0;
	static ExtraUses* Create();
};

class ExtraCharge : public BSExtraData
{
public:
	ExtraCharge();
	~ExtraCharge();
	float charge;
	static ExtraCharge* Create();
};

class ExtraSoul: public BSExtraData
{
public:
	ExtraSoul();
	~ExtraSoul();
	UInt8 soul;
	UInt8 padding[3];
	static ExtraSoul* Create();
};

// used by torches, etc (implies one light per object?)
class ExtraLight : public BSExtraData
{
public:
	ExtraLight();
	virtual ~ExtraLight();

	struct Data
	{
		NiAVObject	* light;	// probably NiLight
		float		unk4;		// intensity? only seen 1.0f
	};

	Data	* data;	// C
};

class ExtraPoison : public BSExtraData
{
public:
	ExtraPoison();
	~ExtraPoison();

	AlchemyItem* poison;
	static ExtraPoison* Create();
};

class ExtraMerchantContainer : public BSExtraData
{
public:
	ExtraMerchantContainer();
	~ExtraMerchantContainer();
	TESObjectREFR* containerRef;
};

class ExtraWaterHeight : public BSExtraData
{
public:
	ExtraWaterHeight();
	~ExtraWaterHeight();
	float waterHeight;

	static ExtraWaterHeight* Create(float height);
};

class ExtraTravelHorse : public BSExtraData
{
public:
	ExtraTravelHorse();
	~ExtraTravelHorse();

	TESObjectREFR*	horseRef;	// Horse
	static ExtraTravelHorse* Create();
};

class ExtraLock : public BSExtraData
{
public:
	ExtraLock();
	~ExtraLock();

	enum
	{
		kLock_isLocked =	1 << 0,
		//..?
		kLock_Leveled =		1 << 2,
	};

	struct Data
	{
		UInt32	lockLevel;
		TESKey	* key;
		UInt8	flags;
		UInt8	pad[3];
	};

	Data	* data;
	static ExtraLock* Create();
};

class ExtraOwnership : public BSExtraData
{
public:
	ExtraOwnership();
	~ExtraOwnership();

	TESForm*	owner;	//maybe this should be a union {TESFaction*; TESNPC*} but it would be more unwieldy to access and modify
	static ExtraOwnership* Create(TESForm* _owner);
};

class ExtraRank	: public BSExtraData
{								//ownership data, stored separately from ExtraOwnership
public:
	ExtraRank();
	~ExtraRank();

	UInt32	rank;
};

class ExtraGlobal : public BSExtraData
{								//ownership data, stored separately from ExtraOwnership
public:
	ExtraGlobal();
	~ExtraGlobal();

	TESGlobal*	globalVar;
};

class ExtraTeleport : public BSExtraData
{
public:
	ExtraTeleport();
	~ExtraTeleport();

	struct Data
	{
		TESObjectREFR*	linkedDoor;
		float			x; //x, y, z, zRot refer to teleport marker's position and rotation
		float			y;
		float			z;
		float			xRot;		// angles in radians. x generally 0
		float			yRot;		// y generally -0.0, no reason to modify
		float			zRot;
	};

	Data *	data;

	static ExtraTeleport* Create();
};

class ExtraRandomTeleportMarker : public BSExtraData
{
public:
	ExtraRandomTeleportMarker();
	~ExtraRandomTeleportMarker();

	TESObjectREFR *	teleportRef;
};

class ExtraCellClimate : public BSExtraData
{
public:
	ExtraCellClimate();
	~ExtraCellClimate();

	TESClimate* climate;
};

class ExtraQuickKey : public BSExtraData		//turns up in ExtraContainerChanges::EntryExtendData
{												//need to find for spells
public:
	ExtraQuickKey();
	~ExtraQuickKey();

	UInt8 keyID;		//0 thru 7
	UInt8 pad[3];

	static ExtraQuickKey * Create();
};

class ExtraScale : public BSExtraData
{
public:
	ExtraScale();
	~ExtraScale();

	float scale;
};

class ExtraNorthRotation : public BSExtraData
{
public:
	ExtraNorthRotation();
	~ExtraNorthRotation();

	float rotation;		//in radians, not degrees
};

class ExtraTrespassPackage : public BSExtraData
{
public:
	ExtraTrespassPackage();
	~ExtraTrespassPackage();

	TrespassPackage* package;
};

class ExtraRegionList : public BSExtraData
{
public:
	ExtraRegionList();
	~ExtraRegionList();

	TESRegionList* regionList;
};

class ExtraSeenData : public BSExtraData
{
public:
	ExtraSeenData();
	~ExtraSeenData();

	void* unk1;			//pointer to SeenData or IntSeenData, neither exposed yet
};

class ExtraPersistentCell : public BSExtraData
{
public:
	ExtraPersistentCell();
	~ExtraPersistentCell();

	TESObjectCELL* cell;
};

class ExtraCellMusicType : public BSExtraData
{
public:
	ExtraCellMusicType();
	~ExtraCellMusicType();

	enum
	{
		kMusicType_Default = 0,
		kMusicType_Public = 1,
		kMusicType_Dungeon = 2,

		kMusicType_MAX
	};

	UInt8 musicType;
	UInt8 pad[3];

	static ExtraCellMusicType* Create(UInt32 _musicType);
};

class ExtraCrimeGold : public BSExtraData
{
public:
	ExtraCrimeGold();
	~ExtraCrimeGold();

	float crimeGold;
};

class ExtraEnableStateParent : public BSExtraData
{
public:
	ExtraEnableStateParent();
	~ExtraEnableStateParent();

	TESObjectREFR	* parent;
	UInt8			oppositeState;		//is 1 if enable state set to opposite of parent's
	UInt8			padOppState[3];
};

class ExtraEnableStateChildren : public BSExtraData
{
public:
	ExtraEnableStateChildren();
	~ExtraEnableStateChildren();

	struct Entry
	{
		TESObjectREFR	* child;
		Entry			* next;

		TESObjectREFR* Info() const { return child; }
		Entry* Next() const { return next; }
	};

	Entry childList;
};

typedef Visitor<ExtraEnableStateChildren::Entry, TESObjectREFR> EnableStateChildrenVisitor;

class ExtraScript : public BSExtraData
{
public:
	ExtraScript();
	~ExtraScript();

	Script			* script;
	ScriptEventList	* eventList;

	static ExtraScript* Create(Script* scr);
};

class ExtraCount : public BSExtraData
{
public:
	ExtraCount();
	~ExtraCount();

	SInt16	count;
	UInt8	pad[2];

	static ExtraCount* Create();
	static ExtraCount* Create(SInt16 count);
};

UInt32 GetCountForExtraDataList(ExtraDataList* list);

class ExtraMapMarker : public BSExtraData
{
public:
	ExtraMapMarker();
	~ExtraMapMarker();

	enum {
		kFlag_Visible	= 1 << 0,
		kFlag_CanTravel	= 1 << 1
	};

	enum {
		kType_Unk0 = 0,
		kType_Camp,
		kType_Unk2,
		kType_City,
		//...

		kType_Max
	};

	struct Data {
		TESFullName	fullName;
		UInt16		flags;
		UInt16		type;	// possibly only 8 bits, haven't checked yet
	};

	Data	* data;

	TESFullName* GetFullName();
	const char* GetName();
	bool IsVisible()	{	return (data->flags & kFlag_Visible) == kFlag_Visible;	}
	bool CanTravelTo()	{	return (data->flags & kFlag_CanTravel) == kFlag_CanTravel;	}
	void SetVisible(bool bVisible)	{
		data->flags = (bVisible) ? (data->flags | kFlag_Visible) : (data->flags & ~kFlag_Visible);	}
	void SetCanTravelTo(bool bCanTravel) {
		data->flags = (bCanTravel) ? (data->flags | kFlag_CanTravel) : (data->flags & ~kFlag_CanTravel);	}
};

class ExtraSound : public BSExtraData
{
public:
	ExtraSound();
	~ExtraSound();

	UInt32	* unk01;
};

// 14
class ExtraAction : public BSExtraData
{
public:
	ExtraAction();
	~ExtraAction();

	enum
	{
		kFlag_NormalActivation			= 1 << 0,		// if not set, TESObjectREFR::Activate will execute the attached script
		kFlag_RunOnActivateBlock		= 1 << 1,		// checked by the OnActivate block's handler
		kFlag_Unk03						= 1 << 2,
		kFlag_Unk04						= 1 << 3,
	};

	TESObjectREFR	* actionRef;
	UInt8			typeFlags;
	UInt8			unk11[3];
};

class ExtraEditorID : public BSExtraData
{
public:
	ExtraEditorID();
	~ExtraEditorID();

	BSStringT		editorID;
};

class ExtraDetachTime : public BSExtraData
{
public:
	ExtraDetachTime();
	~ExtraDetachTime();

	UInt32	detachTime;		// # of hours at which player exited cell. When (hours passed - detachTime) > iHoursToRespawnCell, cell resets
							// set by game to (GameDaysPassed * 24) + (int)GameHour
};

class ExtraReferencePointer : public BSExtraData
{
public:
	ExtraReferencePointer();
	~ExtraReferencePointer();

	TESObjectREFR*	reference;	// a persistent reference. ExtraReferencePointer marks an item in an inventory that originated
								// by activating a persistent reference. When the item is dropped, moves the original reference rather
								// than create a dynamic ref
};

class ExtraCannotWear : public BSExtraData
{
public:
	ExtraCannotWear();
	~ExtraCannotWear();

	// no data
	static ExtraCannotWear * Create();
};

class ExtraEditorRefMoveData : public BSExtraData
{
public:
	ExtraEditorRefMoveData();
	~ExtraEditorRefMoveData();

	float			posX, posY, posZ;
	TESChildCell*	childCell;
	UInt32			unk1C;		// initialized to childCell->unk04
	UInt32			unk20;		// initialized to childCell->unk08
	TESChildCell*	unk24;		// copy of childCell
	UInt32			unk28;		// copy of unk20
	UInt32			unk2C;		// copy of unk24
};

class ExtraEditorCommonDialogData : public BSExtraData	// editor only. added to the window's extraList (which is its userdata)
{
public:
	ExtraEditorCommonDialogData();
	~ExtraEditorCommonDialogData();

	HWND			windowHandle;
	TESForm*		parentForm;
};

class ExtraOriginalReference : public BSExtraData
{
public:
	ExtraOriginalReference();
	~ExtraOriginalReference();

	TESForm*		originalForm;
};

class ExtraRefractionProperty : public BSExtraData
{
public:
	ExtraRefractionProperty();
	~ExtraRefractionProperty();

	float		refractionAmount;		// range of 0-1
};

class ExtraDistantData : public BSExtraData
{
public:
	ExtraDistantData();
	~ExtraDistantData();

	float		unk0C,
				unk10,
				unk14;		// initialized to 0.9700000286102295
};

class ExtraRagDollData : public BSExtraData
{
public:
	ExtraRagDollData();
	~ExtraRagDollData();

	struct Data
	{
		UInt32		unk00;
		UInt32		unk04;
	};

	Data*			data;
};

class ExtraCellWaterType : public BSExtraData
{
public:
	ExtraCellWaterType();
	~ExtraCellWaterType();

	TESWaterForm*		waterType;

	static ExtraCellWaterType* Create(TESWaterForm* water);
};

class ExtraEditorRef3DData : public BSExtraData
{
public:
	ExtraEditorRef3DData();
	~ExtraEditorRef3DData();

	NiNode		* niNode;		// 0C
	NiLines		* selectionBox;	// 10 present when selected in editor
};

class ExtraAnim : public BSExtraData
{
public:
	ExtraAnim();
	~ExtraAnim();

	ActorAnimData*		animData;
};

class ExtraTimeLeft : public BSExtraData
{
public:
	ExtraTimeLeft();
	~ExtraTimeLeft();

	float		time;

	static ExtraTimeLeft* Create(float timeLeft);
};

// attached to TESObjectCELL
class ExtraHavok : public BSExtraData
{
public:
	ExtraHavok();
	~ExtraHavok();

	bhkWorld		* world;
	UInt32			unk10;
};

// 24
class ExtraStartingPosition : public BSExtraData
{
public:
	ExtraStartingPosition();
	~ExtraStartingPosition();

	float x, y, z;			// 0C
	float rotX, rotY, rotZ;	// 18
};
