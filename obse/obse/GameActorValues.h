#pragma once

#include "GameTypes.h"

enum {
	kActorVal_Strength = 0,		// 0x00
	kActorVal_Intelligence,
	kActorVal_Willpower,
	kActorVal_Agility,
	kActorVal_Speed,
	kActorVal_Endurance,		//0x05
	kActorVal_Personality,
	kActorVal_Luck,
	kActorVal_Health,
	kActorVal_Magicka,
	kActorVal_Fatigue,			// 0x0a
	kActorVal_Encumbrance,

	kActorVal_Armorer,
	kActorVal_Athletics,
	kActorVal_Blade,
	kActorVal_Block,			// 0x0f
	kActorVal_Blunt,			// 0x10
	kActorVal_HandToHand,
	kActorVal_HeavyArmor,

	kActorVal_Alchemy,			
	kActorVal_Alteration,
	kActorVal_Conjuration,		// 0x15
	kActorVal_Destruction,
	kActorVal_Illusion,
	kActorVal_Mysticism,
	kActorVal_Restoration,

	kActorVal_Acrobatics,		// 0x1a
	kActorVal_LightArmor,
	kActorVal_Marksman,
	kActorVal_Mercantile,
	kActorVal_Security,
	kActorVal_Sneak,			// 0x1f
	kActorVal_Speechcraft,		// 0x20

	kActorVal_Aggression,
	kActorVal_Confidence,
	kActorVal_Energy,
	kActorVal_Responsibility,
	kActorVal_Bounty,			// 0x25
	kActorVal_Fame,
	kActorVal_Infamy,
	kActorVal_MagickaMultiplier,
	kActorVal_NightEyeBonus,
	kActorVal_AttackBonus,		// 0x2a
	kActorVal_DefendBonus,
	kActorVal_CastingPenalty,
	kActorVal_Blindness,
	kActorVal_Chameleon,
	kActorVal_Invisibility,		// 0x2f
	kActorVal_Paralysis,		// 0x30
	kActorVal_Silence,
	kActorVal_Confusion,
	kActorVal_DetectItemRange,
	kActorVal_SpellAbsorbChance,
	kActorVal_SpellReflectChance,// 0x35
	kActorVal_SwimSpeedMultiplier,
	kActorVal_WaterBreathing,
	kActorVal_WaterWalking,
	kActorVal_StuntedMagicka,
	kActorVal_DetectLifeRange,	// 0x3a
	kActorVal_ReflectDamage,
	kActorVal_Telekinesis,
	kActorVal_ResistFire,
	kActorVal_ResistFrost,
	kActorVal_ResistDisease,	// 0x3f
	kActorVal_ResistMagic,		// 0x40
	kActorVal_ResistNormalWeapons,
	kActorVal_ResistParalysis,
	kActorVal_ResistPoison,
	kActorVal_ResistShock,
	kActorVal_Vampirism,		// 0x45
	kActorVal_Darkness,
	kActorVal_ResistWaterDamage,
	///
	kActorVal_OblivionMax,
	kActorVal_NoActorValue = 256,					// ### incorrect definition, actually 255
	kActorVal_NoActorValue_Proper = 0xFF,			// changing the above value will break existing mods that use it
													// so special-case it for (MGEF, more?)commands that expect the correct value
};

enum {
	kMasteryLevel_Novice = 0,
	kMasteryLevel_Apprentice = 1,
	kMasteryLevel_Journeyman = 2,
	kMasteryLevel_Expert = 3,
	kMasteryLevel_Master = 4,

	kMasteryLevel_MAX = 5
};

enum eAVModifier
{
	kAVModifier_Max         = 0x0,		// e.g. Fortify, Drain, Feather effects
	kAVModifier_Offset      = 0x1,		// script modifier e.g. script cmds Mod/ForceAV
	kAVModifier_Damage      = 0x2,		// console cmds Mod/ForceAV, damage health, etc

	kAVModifier_Invalid		= 0xFF
};

inline bool IsSkill(UInt32 actorVal) {
	return !(actorVal < kActorVal_Armorer || actorVal > kActorVal_Speechcraft);
}

inline bool IsCombatSkill(UInt32 av) { return av >= kActorVal_Armorer && av <= kActorVal_HeavyArmor; }
inline bool IsMagicSkill(UInt32 av) { return av >= kActorVal_Alchemy && av <= kActorVal_Restoration; }
inline bool IsStealthSkill(UInt32 av) { return av >= kActorVal_Acrobatics && av <= kActorVal_Speechcraft; }

// 14
class ActorValues
{
public:
	ActorValues();
	~ActorValues();

	struct Entry {
		UInt32		avCode;
		float		value;
	};

	tList<Entry>	avList;			// 00
	Entry			* magicka;		// 08
	Entry			* fatigue;		// 0C
	Entry			** avArray;		// 10 array of more AV modifiers, size 0x12?

	float	GetAV(UInt32 avCode);
	void	ModAV(UInt32 avCode, float modBy, bool bAllowPositive);
};

STATIC_ASSERT(sizeof(ActorValues) == 0x14);

float GetLuckModifiedSkill(SInt32 skill, SInt32 luck, bool capped = true);
UInt32 GetSkillMasteryLevel(UInt32 level);