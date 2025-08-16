#pragma once

#include "GameTypes.h"
#include "obse/GameForms.h"
#include "obse/GameExtraData.h"
#include "obse/GameProcess.h"
#include "obse/GameTypes.h"
#include "Utilities.h"
#include <vector>
#include "obse/NiObjects.h"
#include "obse/NiProperties.h"

/*** class hierarchy
 *
 *	this information comes from the RTTI information embedded in the exe
 *	so no, I don't have magical inside information
 *
 *	sadly bethesda decided to use /lots/ of multiple inheritance, so this is
 *	going to be very difficult to access externally.
 *
 *	it'll probably be best to expose all of the rtti structs and the dynamic cast interface I guess
 *
 *	TESObjectREFR can be directly cast to Character
 *
 *													total vtbl size
 *	BaseFormComponent
 *		TESForm										0x37
 *			TESObjectREFR							0x69
 *				MobileObject						0x81
 *					Actor							0xEF
 *						Character					0xEF
 *							PlayerCharacter			0xEF
 *						Creature					0xEF
 *					ArrowProjectile					0x81
 *					MagicProjectile					0x89
 *						MagicBallProjectile			0x89
 *						MagicBoltProjectile			0x89
 *						MagicFogProjectile			0x89
 *						MagicSprayProjectile		0x89
 *			TESPackage
 *				FleePackage
 *				DialogPackage
 *				CombatController
 *				AlarmPackage
 *				SpectatorPackage
 *				TresspassPackage
 *			TESTopicInfo
 *			TESTopic
 *			TESSkill
 *			TESRace
 *			TESQuest
 *			TESObject
 *				TESBoundObject
 *					TESBoundAnimObject
 *						TESActorBase
 *							TESNPC
 *							TESCreature
 *						TESObjectCONT
 *					TESObjectMISC
 *					TESObjectBOOK
 *					TESLevItem
 *			Script
 *		TESFullName
 *			TESTopic
 *			TESRace
 *			TESQuest
 *			TESObject
 *		TESDescription
 *			TESSkill
 *			TESRace
 *		TESTexture
 *			TESSkill
 *			TESIcon
 *				TESQuest
 *		TESSpellList
 *			TESRace
 *			TESObject
 *		TESReactionForm
 *			TESRace
 *		TESScriptableForm
 *			TESQuest
 *			TESObject
 *		TESActorBaseData
 *			TESObject
 *		TESContainer
 *			TESObject
 *		TESAIForm
 *			TESObject
 *		TESHealthForm
 *			TESObject
 *		TESAttributes
 *			TESObject
 *		TESAnimation
 *			TESObject
 *		TESModel
 *			TESObject
 *		TESRaceForm
 *			TESObject
 *		TESGlobal
 *
 *	TESMemContextForm
 *		TESObjectREFR
 *			...
 *
 *	TESChildCell
 *		MobileObject
 *			...
 *
 *	MagicCaster
 *		Actor
 *			...
 *		NonActorMagicCaster
 *			BSExtraData
 *
 *	MagicTarget
 *		Character
 *			...
 *		NonActorMagicTarget
 *			BSExtraData
 *
 *	BaseProcess
 *		LowProcess
 *			MiddleLowProcess
 *				MiddleHighProcess
 *					HighProcess
 *
 *	IOTask
 *		LipTask
 *		SkyTask
 *
 *	NiRefObject
 *		NiObject
 *			NiTimeController
 *				BSPlayerDistanceCheckController
 *				BSDoorHavokController
 *			NiExtraData
 *				Tile::Extra
 *				DebugTextExtraData
 *				BSFaceGenBaseMorphExtraData
 *				BSFaceGenModelExtraData
 *				BSFaceGenAnimationData
 *			BSTempEffect
 *				BSTempEffectParticle
 *				BSTempEffectGeometryDecal
 *				BSTempEffectDecal
 *				MagicHitEffect
 *					MagicModelHitEffect
 *					MagicShaderHitEffect
 *			NiSkinInstance
 *			NiTask
 *				BSTECreateTask
 *			bhkRefObject
 *				bhkSerializable
 *					bhkShape
 *						bhkSphereRepShape
 *							bhkConvexShape
 *								bhkBoxShape
 *								bhkCapsuleShape
 *							bhkMultiSphereShape
 *						bhkTransformShape
 *			NiObjectNET
 *				NiAVObject
 *					NiNode
 *						BSTreeNode
 *						BSFaceGenNiNode
 *			NiCollisionObject
 *				bhkNiCollisionObject
 *					bhkPCollisionObject
 *						bgkSPCollisionObject
 *					bhkCollisionObject
 *						bhkBlendCollisionObject
 *							WeaponObject
 *		BSTreeModel
 *		BSFaceGenMorphDataHair
 *			BSFaceGenMorphDataHead
 *		BSFaceGenModel
 *
 *	BSFaceGenMorph
 *		BSFaceGenMorphStatistical
 *		BSFaceGenMorphDifferential
 *
 *	Menu
 *		VideoDisplayMenu
 *		TrainingMenu
 *		StatsMenu
 *		SpellPurchaseMenu
 *		SpellMakingMenu
 *		SleepWaitMenu
 *		SkillsMenu
 *		SigilStoneMenu
 *		SaveMenu
 *		RepairMenu
 *		RechargeMenu
 *		RaceSexMenu
 *		QuickKeysMenu
 *		QuantityMenu
 *		PersuasionMenu
 *		PauseMenu
 *		OptionsMenu
 *		NegotiateMenu
 *		MessageMenu
 *		MapMenu
 *		MainMenu
 *		MagicPopupMenu
 *		MagicMenu
 *		LockPickMenu
 *		LoadgameMenu
 *		LoadingMenu
 *		LevelUpMenu
 *		InventoryMenu
 *		HUDSubtitleMenu
 *		HUDReticle
 *		HUDMainMenu
 *		HUDInfoMenu
 *		GenericMenu
 *		GameplayMenu
 *		EnchantmentMenu
 *		EffectSettingMenu
 *		DialogMenu
 *		CreditsMenu
 *		ContainerMenu
 *		ClassMenu
 *		BreathMenu
 *		BookMenu
 *		AudioMenu
 *		AlchemyMenu
 *		VideoMenu
 *		TextEditMenu
 *		ControlsMenu
 *
 *	Tile
 *		TileWindow
 *		TileRect
 *			TileMenu
 *		Tile3D
 *		TileText
 *		TileImage
 *
 *	BackgroundLoader
 *		BSFaceGenManager
 *
 *	BSFaceGenKeyframe
 *		BSFaceGenKeyframeMultiple	14
 *
 *	SkyObject
 *		Sun
 *		Stars
 *		Moon
 *		Clouds
 *		Atmosphere
 *
 *	Sky
 *
 *	PathLow
 *		PathMiddleHigh
 *			PathHigh
 *
 *	ActiveEffect
 *		ValueModifierEffect
 *			AbsorbEffect
 *			CalmEffect
 *			ChameleonEffect
 *			DarknessEffect
 *			DetectLifeEffect
 *			FrenzyEffect
 *			InvisibilityEffect
 *			NightEyeEffect
 *			ParalysisEffect
 *			ShieldEffect
 *			TelekinesisEffect
 *		AssociatedItemEffect
 *			BoundItemEffect
 *			SummonCreatureEffect
 *		CommandEffect
 *			CommandCreatureEffect
 *			CommandHumanoidEffect
 *		CureEffect
 *		DemoralizeEffect
 *		DisintegrateArmorEffect
 *		DisintegrateWeaponEffect
 *		DispelEffect
 *		LightEffect
 *		LockEffect
 *		OpenEffect
 *		ReanimateEffect
 *		ScriptEffect
 *		SoulTrapEffect
 *		SunDamageEffect
 *		TurnUndeadEffect
 *		VampirismEffect
 *
 *	[ havok stuff ]
 *		bhkCharacterListenerArrow
 *
 *	Menu vtbl + 0x0C = event handler
 *
 ***/

//
enum
{
	kFormID_DoorMarker			= 0x00000001,
	kFormID_TravelMarker,
	kFormID_NorthMarker,

	kFormID_DivineMarker		= 0x00000005,
	kFormID_TempleMarker,

	kFormID_MapMarker			= 0x00000010,

	kFormID_HorseMarker			= 0x00000012,
	// ...
};

#if OBLIVION
#if OBLIVION_VERSION == OBLIVION_VERSION
	static const UInt32 kTESObjectREFR_IsOffLimitsToPlayerAddr = 0x004DEBF0;
#else
#error unsupported oblivion version
#endif
#endif

class Actor;
class NiNode;
class Atmosphere;
class Stars;
class Sun;
class Clouds;
class Moon;
class Precipitation;
class MagicTarget;
class MagicCaster;
class EffectItem;
class ActiveEffect;
class DialoguePackage;
class Creature;
class BoltShaderProperty;
class TESTopic;
class SpellItem;

// 00C
class MagicCaster
{
public:
	MagicCaster();
	~MagicCaster();

	virtual void	AddAbility(MagicItemForm* ability, bool noHitFX);
	virtual void	AddDisease(MagicItemForm* disease, MagicTarget* target, bool noHitFX);
	virtual void	AddObjectEnchantment(MagicItem* arg0, TESBoundObject* sourceObj, bool noHitFX);
	virtual MagicTarget*	FindTouchTarget(void);
	virtual void	PlayTargettedCastAnim(void);
	virtual void	PlayCastingAnim(void);
	virtual void	ApplyMagicItemCost(MagicItem* ite, bool applyStatChanges);

	// looks like returns true if can use magicItem, filling out type (and arg1 is magicka cost?)
	virtual bool	IsMagicItemUsable(MagicItem* magicItem, float* wortcraftSkill, UInt32* faliureCode, bool useBaseMagicka);
	virtual TESObjectREFR*	GetParentRefr(void);
	virtual NiNode	* GetMagicNode(void);	// looks up "magicnode" node in caster's NiNode
	virtual void	AddEffectToSelf(ActiveEffect* effect);
	virtual float	GetSpellEffectiveness(bool ignoreFatigue, float currentFatigue);	// seen (0, 0)
	virtual MagicItem * GetActiveMagicItem(void);		// calls through to MiddleHighProcess
	virtual void	SetActiveMagicItem(MagicItem* item);
	virtual MagicTarget	GetCastingTarget(void);
	virtual void	SetCastingTarget(MagicTarget* target);
	virtual ActiveEffect*	CreateActiveMagicEffect(MagicItem* magicItem, EffectItem* effect, TESBoundObject* src);	// activate effect?

	enum {
		kState_Inactive			= 0,
		kState_Aim				= 1,
		kState_Cast				= 2,
		kState_FindTargets		= 4,

		// these seem to be considered "errors" by the game
		kState_SpellDisabled	= 5,
		kState_AlreadyCasting	= 6,
		kState_CannotCast		= 7
	};

//	void	** _vtbl;			// 000
	NiNode	* magicNode;		// 004 cached during casting anim
	UInt32	state;				// 008
};

// 008
class MagicTarget
{
public:
	MagicTarget();
	~MagicTarget();

	// 8
	struct EffectNode
	{
		ActiveEffect	* data;
		EffectNode		* next;

		ActiveEffect* Info() const { return data; }
		EffectNode* Next() const { return next; }
	};

	virtual void	Destructor(void);
	virtual TESObjectREFR *	GetParent(void);
	virtual EffectNode *	GetEffectList(void);
	//TODO incomplete VTBL
//	void	** _vtbl;	// 000
	UInt8	unk04;		// 004
	UInt8	pad05[3];
};

typedef Visitor<MagicTarget::EffectNode, ActiveEffect> ActiveEffectVisitor;

class NonActorMagicCaster : public BSExtraData
{
public:
	NonActorMagicCaster();
	~NonActorMagicCaster();

	//base class
	MagicCaster magicCaster;	//00C

	UInt32			unk01;			//018
	UInt32			unk02;			//01C
	TESObjectREFR	* caster;		//020
};

// 20
class NonActorMagicTarget : public BSExtraData
{
public:
	NonActorMagicTarget();
	~NonActorMagicTarget();

	// base
	MagicTarget		magicTarget;	// 00C
	TESObjectREFR	* targetRefr;	// 014 passed to c'tor, is the "nonactor" refr
	UInt32			unk18;			// 018
	UInt32			unk1C;			// 01C
};

STATIC_ASSERT(sizeof(NonActorMagicTarget) == 0x20);

/*

	virtual void	Unk_0(void) = 0;
	virtual void	Unk_1(void) = 0;
	virtual void	Unk_2(void) = 0;
	virtual void	Unk_3(void) = 0;
	virtual void	Unk_4(void) = 0;
	virtual void	Unk_5(void) = 0;
	virtual void	Unk_6(void) = 0;
	virtual void	Unk_7(void) = 0;
	virtual void	Unk_8(void) = 0;
	virtual void	Unk_9(void) = 0;
	virtual void	Unk_A(void) = 0;
	virtual void	Unk_B(void) = 0;
	virtual void	Unk_C(void) = 0;
	virtual void	Unk_D(void) = 0;
	virtual void	Unk_E(void) = 0;
	virtual void	Unk_F(void) = 0;

*/

// 058
class TESObjectREFR : public TESForm
{
public:
#if OBLIVION
	MEMBER_FN_PREFIX(TESObjectREFR);
	DEFINE_MEMBER_FN(IsOffLimitsToPlayer, bool, kTESObjectREFR_IsOffLimitsToPlayerAddr);
#endif

	enum
	{
		//	TESForm flags
		//	if(!IsActor())
				kChanged_IsEmpty =		0x00010000,
					// CHANGE_OBJECT_EMPTY
					// no data?

		kChanged_Inventory =			0x08000000,
			// CHANGE_REFR_INVENTORY
			// ### todo: see 0048BA40

		//	if((changed & ((version < 0x43) ? 0x177577F0 : 0x177577E0))) || IsActor())
		//	{
		//		// this is all part of its own function
		//
		//	}

		//	if(!IsActor())
				kChanged_Animation =	0x02000000,
					// CHANGE_REFR_ANIMATION
					// UInt16	dataLen;
					// UInt8	data[dataLen];
		kChanged_Move =			0x00000004,
			// TESObjectCELL	cell
			// float			pos[3]
			// float			rot[3]
		kChanged_HavokMove =			0x00000008,
			// CHANGE_REFR_HAVOK_MOVE
		kChanged_MapMarkerFlags =		0x00000400,
			// CHANGE_MAPMARKER_EXTRA_FLAGS
		kChanged_HadHavokMoveFlag =		0x00000800,
			// CHANGEFLAG_REFR_HAD_HAVOK_MOVE_FLAG
			// if either of these are set
			// UInt16	dataLen;
			// UInt8	data[dataLen];

		//	if(version > 0x43)
				kChanged_Scale =		0x00000010,
					// CHANGE_REFR_SCALE
					// float	scale;

		kChanged_DoorOpenDefaultState =	0x00040000,
			// CHANGE_DOOR_OPEN_DEFAULT_STATE
			// no data

		kChanged_DoorOpenState =		0x00080000,
			// CHANGE_DOOR_OPEN_STATE
			// no data

		kChanged_DoorExtraTeleport =	0x00100000,
			// CHANGE_DOOR_EXTRA_TELEPORT

		kChanged_ExtraOwner =			0x00000080,
			// CHANGE_OBJECT_EXTRA_OWNER
	};

	enum
	{
		kFlags_Persistent	= 0x00000400,		//shared bit with kFormFlags_QuestItem
		kFlags_Disabled =	  0x00000800,
		kFlags_Unk00000002	= 0x00000002,		// set when an item reference is picked up by an actor
		kFlags_Deleted		= 0x00000020,		// refr removed from .esp or savegame
		kFlags_Unk128		= 0x80000000,
		kFlags_Temporary	= 0x00004000,

		// both flags are set when an item picked up from gameworld
		// one or the other flag by itself is not sufficient
		kFlags_Taken		= kFlags_Deleted | kFlags_Unk00000002
	};

	TESObjectREFR();
	~TESObjectREFR();

	virtual void	Unk_37(TESTopic* topic, TESObjectREFR* speaker, bool arg2, bool arg3, UInt32 arg4) = 0;  //Say/SayTo????? 
	virtual bool	CanCastShadows(void) = 0;	// 38
	virtual void	SetCanCastShadows(void) = 0;
	virtual void	IsProjectile(void) = 0;
	virtual float	GetScale(void) = 0;
	virtual void	GetStartingAngle(float * pos) = 0;
	virtual void	GetStartingPos(float * pos) = 0;
	virtual void	MoveInitialPosition(float* pos) = 0;
	virtual bool	UpdateLights(void) = 0;
	virtual void	RemoveItem(TESForm* toRemove, ExtraDataList* extraList, UInt32 quantity, UInt32 useContainerOwnership, UInt32 drop, TESObjectREFR* destRef,
		float* dropPos, float* dropRot, UInt32 unk8, UInt8 useExistingEntryData) = 0;	// 40
	virtual void	RemoveItemByType(UInt32 formType, bool useContainerOwnership, UInt32 count) = 0;
	virtual void	Equip(TESForm* toEquip, UInt32 quantity, ExtraDataList* extraList, UInt32 noUnequip ) = 0;  //Quantity used for arrows? noUnequip is ignored
	virtual void	Unequip(TESForm* toUnEquip, UInt32 quantity, ExtraDataList* extraList) = 0;
	virtual void	Unk_44(void) = 0;
	virtual void	AddItem(TESForm* item, ExtraDataList* xDataList, UInt32 count) = 0;
	virtual void	Unk_46(void) = 0;
	virtual void	Unk_47(void) = 0;
	virtual MagicCaster*	GetMagicCaster(void) = 0;
	virtual MagicTarget*	GetMagicTarget(void) = 0;
	virtual TESForm*	GetTemplateForm(void) = 0;  //Reported inlined. No relvant function except for Actor and derived VTBLs
	virtual void	SetTemplateForm(TESForm* form) = 0; //Reported inlined
	virtual BSFaceGenNiNode*	GetFacegenNiNodeBiped(NiNode arg0) = 0;   //arg seem ignored for all 4 methods
	virtual BSFaceGenNiNode* GetFacegenNiNodeSkinned(NiNode arg0) = 0;
	virtual BSFaceGenNiNode* GetFacegenNiNode(NiNode arg0) = 0;  //Call Skinned variant?
	virtual NiAVObject* GetFacegenAnimData(NiNode arg0) = 0;   // return BSFaceGenAnimationDatA*
	virtual bool	MoveToGroundLevel(void) = 0;	// 50
	virtual void	Unk_51(void) = 0;
	virtual void	Unk_52(void) = 0;			// inits animation-related data, and more
	virtual NiNode*	GenerateNiNode(void) = 0;
	virtual void	Set3D(NiNode* niNode);
	virtual NiNode *	GetNiNode(void) = 0;
	virtual void	Unk_56(void) = 0;
	virtual void	Unk_57(UInt32 arg0) = 0;
	virtual void	UpdateNiNode(void) = 0;
	virtual ActorAnimData*	GetAnimData(void) = 0;
	virtual void	Unk_5A(void) = 0;
	virtual void	Unk_5B(void) = 0;
	virtual TESForm *	GetBaseForm(void) = 0;	// returns type this object references
	virtual float *	GetPos(void) = 0;
	virtual void	Unk_5E(void) = 0;
	virtual void	Unk_5F(void) = 0;
	virtual void	Unk_60(UInt32 unk0) = 0;	// 60  //Get SOmetihng from ridden actor
	virtual void	Unk_61(void) = 0;
	virtual bool	IsMobileObject(void) = 0;
	virtual UInt8	GetSitSleepState(void) = 0;
	virtual bool	IsActor(void) = 0;
	virtual void	ChangeCell(TESObjectCELL * newCell) = 0;
	virtual bool	IsDead(bool arg0) = 0;
	virtual UInt8	GetKnockedState(void) = 0;  //Calls Process::GetKnockedState
	virtual bool	HasFatigue(void) = 0; //
	virtual bool	IsParalized(void) = 0;  //

	TESChildCell	childCell;		// 018
	TESForm	* baseForm;				// 01C
										// U8(typeInfo + 4) == 0x23 is true if the object is a character
	float	rotX, rotY, rotZ;		// 020 - either public or accessed via simple inline accessor common to all child classes
	float	posX, posY, posZ;		// 02C - seems to be private
	float	scale;					// 038
	NiNode	* niNode;				// 03C
	TESObjectCELL	* parentCell;	// 040
	ExtraDataList	baseExtraList;	// 044

	ScriptEventList* GetEventList() const;
	bool IsTaken() const		{	return ((flags & kFlags_Taken) == kFlags_Taken) ? true : false;	}
	bool IsDeleted() const;
	void SetTaken(bool bTaken) {
		flags = (bTaken) ? (flags | kFlags_Taken) : (flags & ~kFlags_Taken);
	}
	bool	IsDisabled() { return flags & kFlags_Disabled ? true : false; }
	void	SetDisabled(bool bDisabled) {
		flags = bDisabled ? (flags | kFlags_Disabled) : (flags & ~kFlags_Disabled);
	}
	bool IsPersistent() { return (flags & kFlags_Persistent) ? true : false; }
	bool IsTemporary() { return (flags & kFlags_Temporary) ? true : false; }
	TESForm * GetInventoryItem(UInt32 itemIndex, bool bGetWares);
	void Disable();
	void Enable();
	bool RunScripts();		// runs magic effect and object scripts plus any scripts on items in inventory

	bool GetTeleportCellName(BSStringT* outName);
	bool Update3D();

	TESContainer* GetContainer();
	bool IsMapMarker();
	float GetDistance(TESObjectREFR* other, bool bIncludeDisabled);

	ExtraTeleport::Data* GetExtraTeleportData();

	static TESObjectREFR* Create(bool bTemp = false);
};

// 05C+
class MobileObject : public TESObjectREFR
{
public:
	enum
	{
		//	UInt8	processLevel;
		//		FF - none
		//		00 - high
		//		01 - medium high
		//		02 - medium low
		//		03 - low
	};

	MobileObject();
	~MobileObject();

	virtual void	Unk_6A(void) = 0;	// 6A
	virtual void	Unk_6B(void) = 0;  //Relevant to switch of processing? Seen destroying/creating  Processes
	virtual void	Unk_6C(void) = 0;
	virtual void	Move(float arg0, float* pos, UInt32 arg2) = 0;  //TODO check. This function is a mess
//	virtual void	Jump(void) = 0;	// jump?
	virtual void  	FallImpact(float a2, float a3, Actor* a4, int a5) = 0; //At least seems relevant to fall and/or pain sounds
	virtual void	Unk_6F(void) = 0;  //SOmething for bhkCharacterProxy
	virtual void	Unk_70(void) = 0;	// 70
	virtual void	Unk_71(void) = 0;
	virtual void	Unk_72(void) = 0;
	virtual void	Unk_73(void) = 0;
	virtual void	SetPosition(float* pos) = 0;  //Call TESObjectREFR::SetPosition + some stuffs with the havok for actors
	virtual void	Unk_75(void) = 0;
	virtual void	Unk_76(void) = 0;
	virtual void	Unk_77(void) = 0;
	virtual float	GetZRotation(void) = 0;
	virtual void	Unk_79(void) = 0;
	virtual void	Unk_7A(void) = 0;
	virtual void	Unk_7B(void) = 0;
	virtual void	Unk_7C(void) = 0;
	virtual float	GetJumpScale(void) = 0; //Return Acrobatics AAV / 100.0 
	virtual bool	IsDead(void) = 0;   //For Actors return True if DeadState == 1
	virtual void	Unk_7F(void) = 0;
	virtual void	Unk_80(void) = 0;	// 80

	BaseProcess	* process;			// 058
};

typedef std::vector<TESForm*> EquippedItemsList;
typedef std::vector<ExtraContainerChanges::EntryData*> ExtraContainerDataList;

// 104+
class Actor : public MobileObject
{
public:
	Actor();
	~Actor();

	virtual SInt32	GetFame(void) = 0; // 81
	virtual SInt32	GetInfamy(void) = 0;	// 82
	virtual void	Resurrect(UInt32 unk1, UInt8 unk2, bool unk3) = 0; //If unk3 is 1, it try to use the bhkProxyController. MAybe relative to animations? unk3 == 0, recreate the processes
	virtual void	Unk_84(void) = 0;
	virtual void	Unk_85(void) = 0;
	virtual void	Unk_86(void) = 0;
	virtual void	Unk_87(void) = 0;

	// applies damage based on game difficulty modifier. Difficulty only applies if attacker != NULL
	// invoked for fall damage (attacker == NULL), melee attacks, not spell damage.
	virtual void	ApplyDamage(float hpDamage, float staminaDamage, Actor* attacker) = 0;
	virtual void	GetDisposition(Actor* with, UInt32 unk1) = 0; //
 	virtual void	ProcessControl(void) = 0;	// handles input for PlayerCharacter
	virtual void	Unk_8B(void) = 0;
	virtual void	SetPackageDismount(void) = 0;
	virtual void	Unk_8D(void) = 0;
	virtual void	Unk_8E(void) = 0;
	virtual void	Unk_8F(void) = 0;
	virtual void	OnAlarmAttack(Actor* attackVictim, UInt32 arg1) = 0;	// 90 tries to send alarm when 'this' attacks victim
	virtual void	Unk_91(void) = 0;
	virtual void	Unk_92(void) = 0;	// SendTrespassAlarm
	virtual void	Unk_93(void) = 0;
	virtual void	Unk_94(void) = 0;
	virtual void	Unk_95(void) = 0;
	virtual void	Unk_96(void) = 0;
	virtual bool	IsOverEncoumbered(void) = 0;   //Possible incorrect decompilation. 
	virtual bool	HasVampireFed(void) = 0;
	virtual void	SetVampireHasFed(bool bFed) = 0;
	virtual void	Unk_9A(void) = 0;  //GetBirthsign?
	virtual void	GetHandReachDistance(void) = 0;   
	virtual void	SetTransparency(bool on, float amount) = 0;  //Can create or delete ExtraRefractionProperty, set something on the NiAvObject, call a base object vtbl function
	virtual void	Unk_9D(void) = 0;
	virtual void	Unk_9E(void) = 0;
	virtual void	Unk_9F(void) = 0;
	virtual void	Unk_A0(void) = 0;	// A0
	virtual SInt32	GetActorValue(UInt32 avCode) = 0;								// current, cumulative value
	virtual float	GetAV_F(UInt32 avCode) = 0;
	virtual void	SetAV_F(UInt32 avCode, float val) = 0;							// base value
	virtual void	SetActorValue(UInt32 avCode, UInt32 val) = 0;
	virtual void	ModMaxAV_F(UInt32 avCode, float amt, Actor* arg2) = 0;
	virtual void	ModMaxAV(UInt32 avCode, SInt32 amt, Actor* arg2) = 0;
	virtual void	ApplyScriptAVMod_F(UInt32 avCode, float amt, UInt32 arg2) = 0;	// script cmds Mod/ForceAV
	virtual void	ApplyScriptAVMod(UInt32 avCode, SInt32 amt, Actor* arg2) = 0;
	virtual void	DamageAV_F(UInt32 avCode, float amt, Actor* arg2) = 0;			// modifier <= 0, console ModAV cmd, damage health, etc
	virtual void	DamageAV(UInt32 value, UInt32 amount, UInt32 unk) = 0;
	virtual void	ModBaseAV_F(UInt32 avCode, float amt) = 0;
	virtual void	ModBaseAV(UInt32 avCode, SInt32 amt) = 0;
	virtual void	Unk_AD(void) = 0;
	virtual void	Unk_AE(void) = 0;
	virtual void	Unk_AF(void) = 0;
	virtual void	Unk_B0(void) = 0;	// B0
	virtual void	Unk_B1(void) = 0;
	virtual void	Unk_B2(void) = 0;
	virtual void	Unk_B3(TESObjectREFR* activatedRefr, UInt32 quantity) = 0; // called after Activate by TESForm::Unk33()
	virtual void	Unk_B4(void) = 0;
	virtual void	Unk_B5(void) = 0;
	virtual void	Unk_B6(void) = 0;
	virtual void	Unk_B7(void) = 0;	// AddSpell?
	virtual void	Unk_B8(void) = 0;	// RemoveSpell?
	virtual void	Unk_B9(void) = 0;
	virtual void	Unk_BA(void) = 0;
	virtual void	Unk_BB(void) = 0;
	virtual void	Unk_BC(void) = 0;
	virtual void	Unk_BD(void) = 0;
	virtual void	Unk_BE(void) = 0;
	virtual void	Unk_BF(void) = 0;
	virtual void	Unk_C0(void) = 0;	// C0
	virtual void	Unk_C1(void) = 0;
	virtual void	Unk_C2(void) = 0;
	virtual void	Unk_C3(void) = 0;
	virtual void	ManageAlarm(void) = 0;  //TODO check
	virtual void	Unk_C5(void) = 0;
	virtual void	Unk_C6(void) = 0;
	virtual void	Unk_C7(void) = 0;
	virtual void	AddPackageWakeUp(void) = 0;
	virtual void	Unk_C9(void) = 0;
	virtual void	Unk_CA(void) = 0;
	virtual void	Unk_CB(void) = 0;
	virtual CombatController*	GetCombatController(void) = 0;
	virtual bool	IsInCombat(bool unk) = 0;
	virtual TESForm *	GetCombatTarget(void) = 0;
	virtual void	Unk_CF(void) = 0;
	virtual void	Unk_D0(void) = 0;	// D0
	virtual void	Unk_D1(void) = 0;
	virtual void	Unk_D2(void) = 0;
	virtual void	Unk_D3(void) = 0;
	virtual bool	IsYielding(void) = 0;
	virtual void	Unk_D5(void) = 0;
	virtual void	Unk_D6(void) = 0;
	virtual void	Unk_D7(void) = 0;
	virtual void	Unk_D8(void) = 0;
	virtual void	Unk_D9(void) = 0;
	virtual void	Unk_DA(void) = 0;
	virtual void	Unk_DB(void) = 0;
	virtual void	Unk_DC(void) = 0;
	virtual void	Unk_DD(void) = 0;
	virtual void	Unk_DE(void) = 0;
	virtual void	Unk_DF(void) = 0;
	virtual Creature * GetMountedHorse(void) = 0;	// E0 returns this->horseOrRider, only for Character
	virtual void	Unk_E1(void) = 0;
	virtual void	Unk_E2(void) = 0;
	virtual void	Unk_E3(void) = 0;
	virtual void	Unk_E4(void) = 0;
	virtual void	Unk_E5(void) = 0;
	virtual void	Unk_E6(void) = 0;
	virtual void	ModExperience(UInt32 actorValue, UInt32 scaleIndex, float baseDelta) = 0;
	virtual void	Unk_E8(void) = 0;
	virtual void	Unk_E9(void) = 0;
	virtual void	Unk_EA(void) = 0;
	virtual void	AttackHandling(UInt32 unused, TESObjectREFR* arrowRef, TESObjectREFR * target) = 0;	// args all null for melee attacks
	virtual void	Unk_EC(void) = 0;
	virtual void	Unk_ED(void) = 0;	// something with blocking
	virtual void	OnHealthDamage(Actor* attacker, float damage) = 0;

	// unk1 looks like quantity, usu. 1; ignored for ammo (equips entire stack)
	// itemExtraList is NULL as the container changes entry is not resolved before the call
	void				EquipItem(TESForm * objType, UInt32 unk1, ExtraDataList* itemExtraList, UInt32 unk3, bool lockEquip);
	void				UnequipItem(TESForm* objType, UInt32 unk1, ExtraDataList* itemExtraList, UInt32 unk3, bool lockUnequip, UInt32 unk5);
	void				UnequipItemSilent(TESForm* objType, UInt32 unk1, ExtraDataList* itemExtraList, UInt32 unk3, bool lockUnequip, UInt32 unk5);

	UInt32				GetBaseActorValue(UInt32 value);
	EquippedItemsList	GetEquippedItems();
	ExtraContainerDataList	GetEquippedEntryDataList();
	bool				CanCastGreaterPower(SpellItem* power);
	void				SetCanUseGreaterPower(SpellItem* power, bool bAllowUse, float timer = -1);
	void				UnequipAllItems();

	// 8
	struct PowerListData {
		SpellItem	* power;
		float		timer;		// init'ed to (3600 / TimeScale) * 24 <- TimeScale is a TESGlobal
	};

	struct PowerListEntry {
		PowerListData	* data;
		PowerListEntry  * next;

		PowerListData * Info() const	{ return data; }
		PowerListEntry * Next() const	{ return next; }
		void SetNext(PowerListEntry* nuNext) { next = nuNext; }
		void Delete();
		void DeleteHead(PowerListEntry* replaceWith);
	};

	// 8
	struct DispositionModifier
	{
		SInt32			  modifier;					// 00 - Can be positive or negative. Linear based on previous interactions
		TESObjectREFR	* refr;						// 04 - This is always the player
	};

	// bases
	MagicCaster		magicCaster;					// 05C
	MagicTarget		magicTarget;					// 068

	UInt32			unk070[(0x07C - 0x070) >> 2];	// 070
	Actor			* unk07C;						// 07C
	UInt32			unk080[(0x088 - 0x080) >> 2];	// 080
	ActorValues		avModifiers;					// 088
	PowerListEntry  greaterPowerList;				// 09C
	DispositionModifier * dispositionModifier;      // 0A4 - This is created after the player has increased/decreased the Actor's disposition
	UInt32          unk0A8;                         // 0A8	
	float			unk0AC;							// 0AC
	UInt32			DeadState;						// 0B0
	UInt32			unk0B4[(0x0CC - 0x0B4) >> 2];	// 0B4
	TESObjectREFR	* unk0CC;						// 0CC
	UInt32			unk0D0;							// 0D0
	Actor			* horseOrRider;					// 0D4 - For Character, currently ridden horse
														 //- For horse (Creature), currently riding Character
	UInt32			unk0D8[(0x0E4 - 0x0D8) >> 2];	// 0D8
	Actor			* unk0E4;						// 0E4
	UInt32			unk0E8[(0x104 - 0x0E8) >> 2];	// 0E8
	// 104

	TESPackage* GetCurrentPackage();
	bool IsObjectEquipped(TESForm* object);
	float GetAVModifier(eAVModifier mod, UInt32 avCode);
	float GetCalculatedBaseAV(UInt32 avCode);
	bool IsAlerted();

	void SetAlerted(bool bAlerted);
	void EvaluatePackage();
//	bool IsTalking();
};

#if OBLIVION
STATIC_ASSERT(sizeof(Actor) == 0x104);
#endif

class Creature : public Actor {
public:
	UInt32		unk104;				// 104  should be similar to ActorSkinInfo
};
#if OBLIVION
STATIC_ASSERT(sizeof(Creature) == 0x108);
#endif

class Character : public Actor {
public:
	ActorSkinInfo*	ActorSkinInfo;						// 104
	UInt32		unk108;								// 108
};
#if OBLIVION
STATIC_ASSERT(sizeof(Character) == 0x10C);
#endif

// 800
class PlayerCharacter : public Character
{
public:
	enum
	{
		kMiscStat_DaysInPrison = 0,
		kMiscStat_DaysPassed,
		kMiscStat_SkillIncreases,
		kMiscStat_TrainingSessions,
		kMiscStat_LargestBounty,
		kMiscStat_CreaturesKilled,
		kMiscStat_PeopleKilled,
		kMiscStat_PlacesDiscovered,
		kMiscStat_LocksPicked,
		kMiscStat_LockpicksBroken,
		kMiscStat_SoulsTrapped,	// 10
		kMiscStat_IngredientsEaten,
		kMiscStat_PotionsMade,
		kMiscStat_OblivionGatesShut,
		kMiscStat_HorsesOwned,
		kMiscStat_HousesOwned,
		kMiscStat_StoresInvestedIn,
		kMiscStat_BooksRead,
		kMiscStat_SkillBooksRead,
		kMiscStat_ArtifactsFound,
		kMiscStat_HoursSlept,	// 20
		kMiscStat_HoursWaited,
		kMiscStat_DaysAsAVampire,
		kMiscStat_LastDayAsAVampire,
		kMiscStat_PeopleFedOn,
		kMiscStat_JokesTold,
		kMiscStat_DiseasesContracted,
		kMiscStat_NirnrootsFound,
		kMiscStat_ItemsStolen,
		kMiscStat_ItemsPickpocketed,
		kMiscStat_Trespasses,	// 30
		kMiscStat_Assaults,
		kMiscStat_Murders,
		kMiscStat_HorsesStolen,

		kMiscStat_Max			// 34
	};

	struct TopicList {
		TESTopic	* topic;
		TopicList	* next;
	};

	PlayerCharacter();
	~PlayerCharacter();

	// [ vtbl ]
	// +000 = PlayerCharacter
	// +018 = TESChildCell
	// +05C = MagicCaster
	// +068 = MagicTarget
	// +784 = NiTMapBase

	// [ objects ]
	// +01C TESNPC *
	// +03C BSFadeNode *
	// +040 TESObjectCELL *
	// +048 ExtraContainerChanges *
	// +058 HighProcess *
	// +1F4 hkAllCdPointCollector *
	// +570 TESObjectREFR *
	// +5E4 TESTopic *
	// +5F4 TESQuest *
	// +614    float amountFenced
	// +624 SpellItem *
	// +644 BirthSign *
	// +650 TESClass *
	// +6E8 TESRegion *
	// +700 TESObjectREFR *
	// +728 TESWorldSpace *
	// +740 TESWorldSpace *

	// [ data ]
	// +11C haggle amount?
	// +588 UInt8, bit 0x01 is true if we're in third person?
	// +590 UInt8, is time passing?
	// +5A9 UInt8, fast travel disabled
	// +658	UInt32, misc stat array
	// +70C	'initial state' buffer

	UInt32		unk10C[(0x118 - 0x10C) >> 2];				// 10C
	DialoguePackage	* dialoguePackage;						// 118
	UInt32		unk11C[(0x130 - 0x11C) >> 2];				// 11C
	float		skillExp[21];								// 130	current experience for each skill
	UInt32		majorSkillAdvances;							// 184
	UInt32		skillAdv[21];								// 188 number of advances for each skill
	UInt8		bCanLevelUp;								// 1DC
	UInt8		unk1DD[3];									// 1DD
	Creature	* lastRiddenHorse;							// 1E0
	UInt32 unk1E4;
	UInt32 unk1E8;
	UInt32 unk1EC;
	UInt32 unk1F0;
	UInt32 unk1F4;
	BSSimpleList<AlchemyItem*>* alchemyItemList;  //Updated in Player_EquipPotion, Queried in Player_GetNumberActivePotions (somewhere else too?)
	UInt32 unk1FC;
	UInt32 unk200;
	float		maxAVModifiers[kActorVal_OblivionMax];		// 204
	float		scriptAVModifiers[kActorVal_OblivionMax];	// 324
    float       health;                                    // 444
    float       magicka;                                   // 448
    float       stamina;                                   // 44C
    UInt32      unk450[(0x570 - 0x450) >> 2];             // 450
    TESObjectREFR	* lastActivatedLoadDoor;	// 570 - most recently activated load door
	UInt32		unk574[(0x588 - 0x574) >> 2];	// 574
	UInt8		isThirdPerson;					// 588
	UInt8		pad589[2];						// 589
	UInt8       isRunning;                      // 58B According to EchoEclipse, who named it shouldAutoRun
	UInt32		unk58C[(0x5B0 - 0x58C) >> 2];	// 58C
	void		* unk5B0;						// 5B0 - freed when running SetInCharGen 0
	UInt8		** attributeBonuses;			// 5B4
	UInt16		unk5B8;							// 5B8
	UInt8		unk5BA;
	UInt8		unk5BB;
	UInt32		trainingSessionsUsed;			// 5BC reset on level-up
	UInt32		unk5C0;							// 5C0
	UInt32		unk5C4;							// 5C4
	UInt32		unk5C8;							// 5C8
	ActorAnimData	* firstPersonAnimData;		// 5CC
	NiNode		* firstPersonNiNode;			// 5D0
	float		unk5D4;							// 5D4
	UInt32		unk5D8;							// 5D8
	UInt32		unk5DC;							// 5DC
	UInt32		unk5E0;							// 5E0
	TESTopic	* unk5E4;						// 5E4
	UInt32		unk5E8;							// 5E8
	tList<QuestStageItem> knownQuestStageItems;	// 5EC
	TESQuest	* activeQuest;					// 5F4
	tList<TESObjectREFR*> activeQuestTargets;	// 5F8 targets whose conditions evaluate to true, updated each frame by HUDMainMenu::Update()
	UInt32		unk600[(0x610 - 0x600) >> 2];	// 600
	UInt8		unk610;							// 610
	UInt8		isAMurderer;					// 611
	UInt8		pad612[2];						// 612
	UInt32		unk614[(0x624 - 0x614) >> 2];	// 614
	MagicItem	* activeMagicItem;				// 624
	TESObjectBOOK	* book;						// 628 //Last acitvated/read book?
	UInt32		unk62C[(0x644 - 0x62C) >> 2];	// 62C
	BirthSign	* birthSign;					// 644
	UInt32		unk648[(0x650 - 0x648) >> 2];	// 648
	TESClass	* wtfClass;						// 650 - this is not the player class! use OBLIVION_CAST(this, TESForm, TESNPC)->npcClass
	UInt32		unk654;							// 654
	UInt32		miscStats[kMiscStat_Max];		// 658
	AlchemyItem	* alchemyItem;					// 6E0
	UInt8		bVampireHasFed;					// 6E4 returned by vtbl+260, set by vtbl+264
	UInt8		isInCharGen;					// 6E5
	UInt8		pad6E6[2];						// 6E6
	TESRegion	* region;						// 6E8
	UInt32		unk6EC[(0x734 - 0x6EC) >> 2];	// 6EC
	float		gameDifficultyLevel;			// 734 ranges from -1 to 1
	UInt32		unk738[(0x7A4 - 0x738) >> 2];	// 738
	float		requiredSkillExp[21];			// 7A4 total amt of exp needed to increase each skill
	UInt32		unk7F8;							// 7F8
	UInt32		unk7FC;							// 7FC
	//  float unk800;  //Defined into ida pro definition. Probably saw this used somewhere, structure is probably 804 size not 800 (but it's the leaf of the hyerarchy so it doesn't matter)
	// 800

	bool	SetActiveSpell(MagicItem * item);
	UInt8	GetAttributeBonus(UInt32 whichAttribute) {
		return whichAttribute < kActorVal_Luck ? (*attributeBonuses)[whichAttribute] : -1;
	}

	void	SetAttributeBonus(UInt32 whichAttr, UInt8 newVal) {
		if (whichAttr < kActorVal_Luck)	(*attributeBonuses)[whichAttr] = newVal;
	}

	UInt32 GetSkillAdvanceCount(UInt32 valSkill) {
		return IsSkill(valSkill) ? skillAdv[valSkill - kActorVal_Armorer] : 0;
	}

	void SetSkillAdvanceCount(UInt32 valSkill, UInt32 val) {
		if (IsSkill(valSkill)) {
			skillAdv[valSkill - kActorVal_Armorer] = val;
		}
	}

	UInt32 ModSkillAdvanceCount(UInt32 valSkill, SInt32 mod);
	UInt32 ModMajorSkillAdvanceCount(SInt32 mod) {
		SInt32 adjustedVal = majorSkillAdvances + mod;
		majorSkillAdvances = (adjustedVal > 0) ? adjustedVal : 0;
		return majorSkillAdvances;
	}

	MagicItem* GetActiveMagicItem();
	bool IsThirdPerson() { return isThirdPerson ? true : false; }
	void TogglePOV(bool bFirstPerson);
	void SetBirthSign(BirthSign* birthSign);
	void ChangeExperience(UInt32 actorValue, UInt32 scaleIndex, float baseDelta);
	void ChangeExperience(UInt32 actorValue, float amount);
	float ExperienceNeeded(UInt32 skill, UInt32 atLevel);

	TESClass* GetPlayerClass() const;

	bool SetSkeletonPath(const char* newPath);

	static void UpdateHead(void);	// TODO: investigate further
};

#if OBLIVION
STATIC_ASSERT(sizeof(PlayerCharacter) == 0x800);
#endif

extern PlayerCharacter ** g_thePlayer;

class SkyObject {
public:
	virtual NiNode* GetObjectNode();
	virtual void	Initialize(UInt32 u1);
	virtual void	func_03(UInt32 u1, UInt32 u2);

	NiNode* RootNode;						// 04	
};
STATIC_ASSERT(sizeof(SkyObject) == 0x008);

class Sun : public SkyObject {
public:
	NiBillboardNode* SunBillboard;			// 08
	NiBillboardNode* SunGlareBillboard;		// 0C
	NiGeometry* SunGeometry;			// 10 NiTriShape*
	NiGeometry* SunGlareGeometry;		// 14 NiTriShape*
	NiTArray<NiPick*>* SunPickList;			// 18 NiPick::Record
	NiDirectionalLight* SunDirLight;			// 1C
	float				unk20;					// 20
	UInt8				unk24;					// 24
	UInt8				pad25[3];				// 25
};
STATIC_ASSERT(sizeof(Sun) == 0x028);

class Atmosphere : public SkyObject {
public:
	NiAVObject*    Mesh;					// 08
	BSFogProperty * fogProperty;			// 0C
	UInt32			unk10;					// 10
	NiNode* Quad;					// 14
	UInt8			unk18;					// 18
	UInt8			pad18[3];
};
STATIC_ASSERT(sizeof(Atmosphere) == 0x01C);

class Stars : public SkyObject {
public:
	UInt32			unk08;					// 08
	float			unk0C;					// 0C
};
STATIC_ASSERT(sizeof(Stars) == 0x010);

class Clouds : public SkyObject {
public:
	UInt32			unk08;					// 08
	UInt32			unk0C;					// 0C
	UInt32			unk10;					// 10
	UInt32			unk14;					// 14
};
STATIC_ASSERT(sizeof(Clouds) == 0x018);

class Moon : public SkyObject {
public:
	NiNode* MoonNode;				// 08
	NiNode* ShadowNode;				// 0C
	NiTriShape* MoonMesh;				// 10
	NiTriShape* ShadowMesh;				// 14
	char* texture_full;			// 18
	UInt32			unk1C;					// 1C
	char* texture_three_wan;		// 20
	UInt32			unk24;					// 24
	char* texture_half_wan;		// 28
	UInt32			unk2C;					// 2C
	char* texture_one_wan;		// 30
	UInt32			unk34;					// 34
	UInt32			unk38;					// 38
	UInt32			unk3C;					// 3C
	char* texture_one_wax;		// 40
	UInt32			unk44;					// 44
	char* texture_half_wax;		// 48
	UInt32			unk4C;					// 4C
	char* texture_three_wax;		// 50
	UInt32			unk54;					// 54
	float			unk58;					// 58
	float			unk5C;					// 5C
	float			unk60;					// 60
	float			unk64;					// 64
	float			unk68;					// 68
	UInt32			unk6C;					// 6C
	UInt32			unk70;					// 70
	float			unk74;					// 74
	float			unk78;					// 78
};
STATIC_ASSERT(sizeof(Moon) == 0x07C);

// 104
class Sky
{
public:
	Sky();
	~Sky();

	virtual void	Destructor(void);
	// no more virtual functions

	static Sky *	GetSingleton(void);

	void	RefreshClimate(TESClimate * climate, UInt32 unk1);	// pass 1 for unk1 to pick new random weather etc

//	void		** _vtbl;						// 000
	NiNode*		nodeSkyRoot;					// 004
	NiNode*		nodeMoonRoot;					// 008
	TESClimate	* firstClimate;					// 00C
	TESWeather	* firstWeather;					// 010
	TESWeather* weather014;						// 014  //tansition?
	TESWeather*	weather018;						// 018
	TESWeather* weatherOverride;				// 01C
	Atmosphere* atmosphere;						// 020
	Stars*		stars;							// 024
	Sun*		sun;							// 028
	Clouds*		clouds;							// 02C
	Moon*		masserMoon;						// 030
	Moon*		secundaMoon;					// 034
	Precipitation* precipitation;				// 038
	UInt32		unk03C[(0x104 - 0x03C) >> 2];	// 03C
};

enum
{
	kProjectileType_Arrow,
	kProjectileType_Ball,
	kProjectileType_Fog,
	kProjectileType_Bolt,
};							//arbitrary

//78
class MagicProjectile : public MobileObject
{
public:
	/*
	.rdata:00A76488                     dd offset nullsub_returnvVoid_1arg
.rdata:00A7648C                     dd offset nullsub_26
.rdata:00A76490                     dd offset nullsub_returnVoid_2arg
.rdata:00A76494                     dd offset MagicProjectile_NotDeleted
.rdata:00A76498                     dd offset ?ClearComponentReferences@TESTexture@@UAEXXZ?
.rdata:00A7649C                     dd offset MagicProjectile_RemoveCaster
.rdata:00A764A0                     dd offset sub_738500
.rdata:00A764A4                     dd offset nullsub_returnTrue_0arg
	*/


	MagicProjectile();
	~MagicProjectile();
	virtual void __stdcall	       Unk81(int a1) = 0;
	virtual void __stdcall		   Unk82(int a1, int a2, int a3, int a4, int a5, int a6) = 0;
	virtual void  __stdcall		   Unk83(int a1, int a2) = 0;
	virtual void		   NotDeleted(void);  //in Jroush IDB
	virtual void		   Unk85(void) = 0;
	virtual void		   RemoveCaster(MagicCaster* caster);
	virtual void __stdcall		   Unk87(int a1, int a2) = 0;
	virtual bool		   Unk88(void) = 0;


	float			speed;				// 5C base speed * GMST fMagicProjectileBaseSpeed
	float			distanceTraveled;	// 60 speed * elapsedTime while in flight
	float			elapsedTime;		// 64 length of time projectile has existed
	MagicCaster		* caster;			// 68 whoever/whatever cast the spell
										//    For NonActorMagicCaster, != casting reference
	MagicItem		* magicItem;		// 6C can always cast to SpellItem? NO - can be EnchantmentItem for staves
	UInt32			effectCode;			// 70 i.e. 'SEFF'
	EffectSetting	* effectSetting;	// 74



};

//90
class MagicBallProjectile : public MagicProjectile
{
public:
	MagicBallProjectile();
	~MagicBallProjectile();

	float				unk078;				//078
	UInt32				unk07C;				//07C
	UInt32				unk080;				//080 - looks like flags - (1 in flight, 2 hit target?)
	float				unk084;				//084 - value changes after projectile hits something
	UInt32				unk088;				//088
	UInt32				unk08C;				//08C
};

//9C
class MagicFogProjectile : public MagicProjectile
{
public:
	MagicFogProjectile();
	~MagicFogProjectile();

	float				unk078;				//078
	float				unk07C;				//07C
	float				unk080;				//080
	float				unk084;				//084
	UInt32				unk088;				//088 - looks like flags - (0 in flight, 1 hit target?)
	float				unk08C;				//08C
	UInt32				unk090;				//090 - pointer?
	UInt32				unk094;				//094
	UInt32				unk098;				//098 - pointer?
};

//A4
class MagicBoltProjectile : public MagicProjectile
{
public:
	MagicBoltProjectile();
	~MagicBoltProjectile();

	float				unk078;				//078
	BoltShaderProperty	* boltShaderProperty;//07C
	UInt32				unk080;				//080
	UInt32				unk084;				//084
	NiNode				* niNode088;		//088
	UInt32				unk08C;				//08C
	UInt32				unk090;				//090
	NiNode				* niNode094;		//094
	UInt32				unk098;				//098
	UInt32				unk09C;				//09C - pointer?
	UInt32				unk0A0;				//0A0
};

//9C
class ArrowProjectile : public MobileObject
{
public:
	ArrowProjectile();
	~ArrowProjectile();

	// 54
	struct CollisionData
	{
		// not sure if this data is generated for collisions with immobile objects too, or if struct is unique to ArrowProjectile (probably not)
		float			unk00[9];	// 00 presumably a matrix or set of 3 3-dimensional vectors
		TESObjectREFR	* refr;		// 24 what it hit
		NiNode			* ninode;	// 28 seen "Bip01 Spine" for Creature
		float			unk2C[9];	// 2C again
	};

	CollisionData	* unk05C;		//05C
	UInt32			unk060;			//060
	float			unk064;			//064
	float			elapsedTime;	//068
	float			speed;			//06C - base speed * GMST fArrowSpeedMult
	float			unk070;			//070
	float			unk074;			//074
	Actor			* shooter;		//078;
	EnchantmentItem	* arrowEnch;	//07C
	EnchantmentItem	* bowEnch;		//080
	AlchemyItem		* poison;		//084
	float			unk088;			//088
	float			unk08C;			//08C
	float			unk090;			//090
	UInt32			unk094;			//094
	UInt32			unk098;			//098
};
