#include "obse/Commands_MagicEffect.h"
#include "ParamInfos.h"
#include "Utilities.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"

#include "StringVar.h"
#include "ArrayVar.h"

static bool Cmd_GetMagicEffectName_Execute(COMMAND_ARGS)
{
	std::string name = "";

	EffectSetting* magic = NULL;
	if(ExtractArgs(PASS_EXTRACT_ARGS, &magic))
		if (magic)
			if (magic->fullName.name.m_dataLen > 0)
				name = magic->fullName.name.m_data;

	AssignToStringVar(PASS_COMMAND_ARGS, name.c_str());
	return true;
}

static bool Cmd_GetMagicEffectNameC_Execute(COMMAND_ARGS)
{
	std::string name = "";

	UInt32 effectCode = 0;
	if(ExtractArgs(PASS_EXTRACT_ARGS, &effectCode)) {
		EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
		if (magic) {
			if (magic->fullName.name.m_dataLen > 0) {
				name = magic->fullName.name.m_data;
			}
		}
	}

	AssignToStringVar(PASS_COMMAND_ARGS, name.c_str());
	return true;
}

enum {
	kMagicEffect_EffectCode = 0,
	kMagicEffect_BaseCost,
	kMagicEffect_School,
	kMagicEffect_ProjectileSpeed,
	kMagicEffect_EnchantFactor,
	kMagicEffect_BarterFactor,
	kMagicEffect_EffectShader,
	kMagicEffect_EnchantEffect,
	kMagicEffect_Light,
	kMagicEffect_CastingSound,
	kMagicEffect_BoltSound,
	kMagicEffect_HitSound,
	kMagicEffect_AreaSound,
	kMagicEffect_IsHostile,
	kMagicEffect_CanRecover,
	kMagicEffect_IsDetrimental,
	kMagicEffect_MagnitudePercent,
	kMagicEffect_OnSelfAllowed,
	kMagicEffect_OnTouchAllowed,
	kMagicEffect_OnTargetAllowed,
	kMagicEffect_NoDuration,
	kMagicEffect_NoMagnitude,
	kMagicEffect_NoArea,
	kMagicEffect_FXPersists,
	kMagicEffect_ForSpellmaking,
	kMagicEffect_ForEnchanting,
	kMagicEffect_NoIngredient,
	kMagicEffect_UseWeapon,
	kMagicEffect_UseArmor,
	kMagicEffect_UseCreature,
	kMagicEffect_UseSkill,
	kMagicEffect_UseAttribute,
	kMagicEffect_UseActorValue,
	kMagicEffect_NoHitEffect,
	kMagicEffect_OtherActorValue,
	kMagicEffect_UsedObject,
	kMagicEffect_NumCounters,
	kMagicEffect_ResistValue,
};

static bool GetMagicEffectValue(EffectSetting* effect, UInt32 valueType, double* result)
{
	if (!effect || !result) return true;

	*result = 0;

	switch(valueType) {
		case kMagicEffect_EffectCode:
			{
				*result = (SInt32)(effect->effectCode);
				break;
			}

		case kMagicEffect_BaseCost:
			{
				*result = effect->baseCost;
				break;
			}

		case kMagicEffect_School:
			{
				*result = effect->school;
				break;
			}

		case kMagicEffect_ProjectileSpeed:
			{
				*result = effect->projSpeed;
				break;
			}

		case kMagicEffect_EnchantFactor:
			{
				*result = effect->enchantFactor;
				break;
			}

		case kMagicEffect_BarterFactor:
			{
				*result = effect->barterFactor;
				break;
			}

		case kMagicEffect_IsHostile:
			{
				*result = effect->IsHostile() ? 1 : 0;
				break;
			}

		case kMagicEffect_CanRecover:
			{
				*result = effect->CanRecover() ? 1 : 0;
				break;
			}

		case kMagicEffect_IsDetrimental:
			{
				*result = effect->IsDetrimental() ? 1 : 0;
				break;
			}

		case kMagicEffect_MagnitudePercent:
			{
				*result = effect->MagnitudeIsPercent() ? 1 : 0;
				break;
			}

		case kMagicEffect_OnSelfAllowed:
			{
				*result = effect->OnSelfAllowed() ? 1 : 0;
				break;
			}

		case kMagicEffect_OnTouchAllowed:
			{
				*result = effect->OnTouchAllowed() ? 1 : 0;
				break;
			}

		case kMagicEffect_OnTargetAllowed:
			{
				*result = effect->OnTargetAllowed() ? 1 : 0;
				break;
			}

		case kMagicEffect_NoDuration:
			{
				*result = effect->NoDuration() ? 1 : 0;
				break;
			}

		case kMagicEffect_NoMagnitude:
			{
				*result = effect->NoMagnitude() ? 1 : 0;
				break;
			}

		case kMagicEffect_NoArea:
			{
				*result = effect->NoArea() ? 1 : 0;
				break;
			}

		case kMagicEffect_FXPersists:
			{
				*result = effect->FXPersists() ? 1 : 0;
				break;
			}

		case kMagicEffect_ForSpellmaking:
			{
				*result = effect->ForSpellmaking() ? 1 : 0;
				break;
			}

		case kMagicEffect_ForEnchanting:
			{
				*result = effect->ForEnchanting() ? 1 : 0;
				break;
			}

		case kMagicEffect_NoIngredient:
			{
				*result = effect->NoIngredient() ? 1 : 0;
				break;
			}

		case kMagicEffect_UseWeapon:
			{
				*result = effect->UseWeapon() ? 1 : 0;
				break;
			}

		case kMagicEffect_UseArmor:
			{
				*result = effect->UseArmor() ? 1 : 0;
				break;
			}

		case kMagicEffect_UseCreature:
			{
				*result = effect->UseCreature() ? 1 : 0;
				break;
			}

		case kMagicEffect_UseSkill:
			{
				*result = effect->UseSkill() ? 1 : 0;
				break;
			}

		case kMagicEffect_UseAttribute:
			{
				*result = effect->UseAttribute() ? 1 : 0;
				break;
			}

		case kMagicEffect_UseActorValue:
			{
				*result = effect->UseOtherActorValue() ? 1 : 0;
				break;
			}

		case kMagicEffect_NoHitEffect:
			{
				*result = effect->NoHitEffect() ? 1 : 0;
				break;
			}

		case kMagicEffect_OtherActorValue:
			{
				*result = effect->data;
				break;
			}

		case kMagicEffect_UsedObject:
			{
				UInt32* refResult = (UInt32*)result;
				*refResult = effect->data;
				break;
			}

		case kMagicEffect_EffectShader:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->effectShader)
					*refResult = ((TESForm*)(effect->effectShader))->refID;
				break;
			}

		case kMagicEffect_EnchantEffect:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->enchantEffect)
					*refResult = ((TESForm*)(effect->enchantEffect))->refID;
				break;
			}
		case kMagicEffect_Light:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->light)
					*refResult = effect->light->refID;
				break;
			}

		case kMagicEffect_CastingSound:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->castingSound)
					*refResult = effect->castingSound->refID;
				break;
			}

		case kMagicEffect_BoltSound:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->boltSound)
					*refResult = effect->boltSound->refID;
				break;
			}

		case kMagicEffect_HitSound:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->hitSound)
					*refResult = effect->hitSound->refID;
				break;
			}

		case kMagicEffect_AreaSound:
			{
				UInt32* refResult = (UInt32 *)result;
				if (effect->areaSound)
					*refResult = effect->areaSound->refID;
				break;
			}

		case kMagicEffect_NumCounters:
			{
				if (effect->numCounters)
					*result = effect->numCounters;
				break;
			}

		case kMagicEffect_ResistValue:
			{
				if (effect->resistValue)
					*result = effect->resistValue;
				break;
			}

		default:
			break;
	}
	return true;
}

static bool Cmd_GetMagicEffectValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 whichValue = 0;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &whichValue, &magic)) return true;

	if (magic) {
		return GetMagicEffectValue(magic, whichValue, result);
	}

	return true;
}

static bool GetMagicEffectValue_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	EffectSetting* magic = NULL;
	ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &magic);
	if (magic) {
		return GetMagicEffectValue(magic, whichValue, result);
	}
	return true;
}

static bool GetMagicEffectValueC(UInt32 effectCode, UInt32 whichValue, double* result)
{
	*result = 0;
	EffectSetting *magicEffect = EffectSetting::EffectSettingForC(effectCode);
	if (magicEffect) {
		return GetMagicEffectValue(magicEffect, whichValue, result);
	}
	return true;
}

static bool Cmd_GetMagicEffectCodeValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 whichValue = 0;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &whichValue, &effectCode)) return true;
	return GetMagicEffectValueC(effectCode, whichValue, result);
}

static bool GetMagicEffectValueC_Execute(COMMAND_ARGS, UInt32 whichValue)
{
	*result = 0;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &effectCode)) return true;
	return GetMagicEffectValueC(effectCode, whichValue, result);
}

static bool Cmd_GetMagicEffectCode_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_EffectCode);
}

static bool Cmd_GetMagicEffectBaseCost_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_BaseCost);
}

static bool Cmd_GetMagicEffectSchool_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_School);
}

static bool Cmd_GetMagicEffectProjectileSpeed_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_ProjectileSpeed);
}

static bool Cmd_GetMagicEffectEnchantFactor_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_EnchantFactor);
}

static bool Cmd_GetMagicEffectBarterFactor_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_BarterFactor);
}

static bool Cmd_GetMagicEffectBaseCostC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_BaseCost);
}

static bool Cmd_GetMagicEffectSchoolC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_School);
}

static bool Cmd_GetMagicEffectProjectileSpeedC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_ProjectileSpeed);
}

static bool Cmd_GetMagicEffectEnchantFactorC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_EnchantFactor);
}

static bool Cmd_GetMagicEffectBarterFactorC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_BarterFactor);
}

static bool Cmd_IsMagicEffectHostile_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_IsHostile);
}

static bool Cmd_IsMagicEffectHostileC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_IsHostile);
}

static bool Cmd_IsMagicEffectForSpellmaking_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_ForSpellmaking);
}

static bool Cmd_IsMagicEffectForSpellmakingC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_ForSpellmaking);
}

static bool Cmd_IsMagicEffectForEnchanting_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_ForEnchanting);
}

static bool Cmd_IsMagicEffectForEnchantingC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_ForEnchanting);
}

static bool Cmd_IsMagicEffectDetrimental_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_IsDetrimental);
}

static bool Cmd_IsMagicEffectDetrimentalC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_IsDetrimental);
}

static bool Cmd_IsMagicEffectCanRecover_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_CanRecover);
}

static bool Cmd_IsMagicEffectCanRecoverC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_CanRecover);
}

static bool Cmd_IsMagicEffectMagnitudePercent_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_MagnitudePercent);
}

static bool Cmd_IsMagicEffectMagnitudePercentC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_MagnitudePercent);
}

static bool Cmd_MagicEffectFXPersists_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_FXPersists);
}

static bool Cmd_MagicEffectFXPersistsC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_FXPersists);
}

static bool Cmd_IsMagicEffectOnSelfAllowed_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_OnSelfAllowed);
}

static bool Cmd_IsMagicEffectOnSelfAllowedC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_OnSelfAllowed);
}

static bool Cmd_IsMagicEffectOnTouchAllowed_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_OnTouchAllowed);
}

static bool Cmd_IsMagicEffectOnTouchAllowedC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_OnTouchAllowed);
}

static bool Cmd_IsMagicEffectOnTargetAllowed_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_OnTargetAllowed);
}

static bool Cmd_IsMagicEffectOnTargetAllowedC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_OnTargetAllowed);
}

static bool Cmd_MagicEffectHasNoDuration_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoDuration);
}

static bool Cmd_MagicEffectHasNoDurationC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoDuration);
}

static bool Cmd_MagicEffectHasNoMagnitude_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoMagnitude);
}

static bool Cmd_MagicEffectHasNoMagnitudeC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoMagnitude);
}

static bool Cmd_MagicEffectHasNoArea_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoArea);
}

static bool Cmd_MagicEffectHasNoAreaC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoArea);
}

static bool Cmd_MagicEffectHasNoIngredient_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoIngredient);
}

static bool Cmd_MagicEffectHasNoIngredientC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoIngredient);
}

static bool Cmd_MagicEffectHasNoHitEffect_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoHitEffect);
}

static bool Cmd_MagicEffectHasNoHitEffectC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_NoHitEffect);
}

static bool Cmd_MagicEffectUsesWeapon_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseWeapon);
}

static bool Cmd_MagicEffectUsesWeaponC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseWeapon);
}

static bool Cmd_MagicEffectUsesArmor_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseArmor);
}

static bool Cmd_MagicEffectUsesArmorC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseArmor);
}

static bool Cmd_MagicEffectUsesCreature_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseCreature);
}

static bool Cmd_MagicEffectUsesCreatureC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseCreature);
}

static bool Cmd_MagicEffectUsesSkill_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseSkill);
}

static bool Cmd_MagicEffectUsesSkillC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseSkill);
}

static bool Cmd_MagicEffectUsesAttribute_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseAttribute);
}

static bool Cmd_MagicEffectUsesAttributeC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseAttribute);
}

static bool Cmd_MagicEffectUsesOtherActorValue_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseActorValue);
}

static bool Cmd_MagicEffectUsesOtherActorValueC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UseActorValue);
}

static bool Cmd_GetMagicEffectOtherActorValue_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_OtherActorValue);
}

static bool Cmd_GetMagicEffectOtherActorValueC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_OtherActorValue);
}

static bool Cmd_GetMagicEffectUsedObject_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_UsedObject);
}

static bool Cmd_GetMagicEffectUsedObjectC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_UsedObject);
}

static bool Cmd_GetMagicEffectHitShader_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_EffectShader);
}

static bool Cmd_GetMagicEffectHitShaderC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_EffectShader);
}

static bool Cmd_GetMagicEffectEnchantShader_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_EnchantEffect);
}

static bool Cmd_GetMagicEffectEnchantShaderC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_EnchantEffect);
}

static bool Cmd_GetMagicEffectLight_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_Light);
}

static bool Cmd_GetMagicEffectLightC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_Light);
}

static bool Cmd_GetMagicEffectCastingSound_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_CastingSound);
}

static bool Cmd_GetMagicEffectCastingSoundC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_CastingSound);
}

static bool Cmd_GetMagicEffectBoltSound_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_BoltSound);
}

static bool Cmd_GetMagicEffectBoltSoundC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_BoltSound);
}

static bool Cmd_GetMagicEffectHitSound_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_HitSound);
}

static bool Cmd_GetMagicEffectHitSoundC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_HitSound);
}

static bool Cmd_GetMagicEffectAreaSound_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_AreaSound);
}

static bool Cmd_GetMagicEffectAreaSoundC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_AreaSound);
}

static bool Cmd_GetMagicEffectNumCounters_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_NumCounters);
}

static bool Cmd_GetMagicEffectNumCountersC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_NumCounters);
}

static bool Cmd_GetMagicEffectResistValue_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValue_Execute(PASS_COMMAND_ARGS, kMagicEffect_ResistValue);
}

static bool Cmd_GetMagicEffectResistValueC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectValueC_Execute(PASS_COMMAND_ARGS, kMagicEffect_ResistValue);
}

static bool Cmd_GetMagicEffectIcon_Execute(COMMAND_ARGS)
{
	std::string icon = "";

	EffectSetting* magic = NULL;
	if(ExtractArgs(PASS_EXTRACT_ARGS, &magic))
		if (magic)
			if (magic->texture.ddsPath.m_dataLen > 0)
				icon = magic->texture.ddsPath.m_data;

	AssignToStringVar(PASS_COMMAND_ARGS, icon.c_str());
	return true;
}

static bool Cmd_GetMagicEffectIconC_Execute(COMMAND_ARGS)
{
	std::string icon = "";
	EffectSetting* magic = NULL;

	UInt32 effectCode = 0;
	if(ExtractArgs(PASS_EXTRACT_ARGS, &effectCode))
		magic = EffectSetting::EffectSettingForC(effectCode);
		if (magic)
			if (magic->texture.ddsPath.m_dataLen > 0)
				icon = magic->texture.ddsPath.m_data;

	AssignToStringVar(PASS_COMMAND_ARGS, icon.c_str());
	return true;
}

static bool Cmd_GetMagicEffectModel_Execute(COMMAND_ARGS)
{
	std::string model = "";

	EffectSetting* magic = NULL;
	if(ExtractArgs(PASS_EXTRACT_ARGS, &magic))
		if (magic)
			if (magic->model.nifPath.m_dataLen > 0)
				model = magic->model.nifPath.m_data;

	AssignToStringVar(PASS_COMMAND_ARGS, model.c_str());
	return true;
}

static bool Cmd_GetMagicEffectModelC_Execute(COMMAND_ARGS)
{
	std::string model = "";
	EffectSetting* magic = NULL;

	UInt32 effectCode = 0;
	if(ExtractArgs(PASS_EXTRACT_ARGS, &effectCode))
		magic = EffectSetting::EffectSettingForC(effectCode);
		if (magic)
			if (magic->model.nifPath.m_dataLen > 0)
				model = magic->model.nifPath.m_data;

	AssignToStringVar(PASS_COMMAND_ARGS, model.c_str());
	return true;
}

static bool Cmd_GetNthMagicEffectCounter_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 index = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &index, &magic)) return true;

	if (magic) {
		if ( index >= 0 && index < magic->numCounters ) {
			*result = magic->counterArray[index];
		}
	}
	return true;
}

static bool Cmd_GetNthMagicEffectCounterC_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 index = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &index, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if ( index >= 0 && index < magic->numCounters ) {
			*result = magic->counterArray[index];
		}
	}
	return true;
}

static bool Cmd_GetMagicEffectCounters_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetMagicEffectCounterArray must be called within the context of an OBSE expression");
		return false;
	}

	// create temp array to hold the items
	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &magic)) return true;

	if (magic) {
		// store items in array
		for (UInt32 i = 0; i < magic->numCounters; i++)
			g_ArrayMap.SetElementNumber(arrID, i, (double)(magic->counterArray[i]));
	}

	return true;
}

static bool Cmd_GetMagicEffectCountersC_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetMagicEffectCounterArrayC must be called within the context of an OBSE expression");
		return false;
	}

	// create temp array to hold the items
	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		// store items in array
		for (UInt32 i = 0; i < magic->numCounters; i++)
			g_ArrayMap.SetElementNumber(arrID, i, (double)(magic->counterArray[i]));
	}

	return true;
}

static bool Cmd_SetMagicEffectName_Execute(COMMAND_ARGS)
{
	*result = 0;

	char newName[kMaxMessageLength];
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newName, &magic)) return true;

	if (magic) {
		if (newName[0] != '\0') {
			magic->fullName.name.Set(newName);
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectNameC_Execute(COMMAND_ARGS)
{
	*result = 0;

	char newName[kMaxMessageLength];
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newName, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newName[0] != '\0') {
			magic->fullName.name.Set(newName);
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectBaseCost_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newBaseCost = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newBaseCost, &magic)) return true;

	if (magic) {
		if (newBaseCost > 0) {
			magic->baseCost = newBaseCost;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectBaseCostC_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newBaseCost = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newBaseCost, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newBaseCost > 0) {
			magic->baseCost = newBaseCost;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectBarterFactor_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newBarterFactor = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newBarterFactor, &magic)) return true;

	if (magic) {
		if (newBarterFactor > 0) {
			magic->barterFactor = newBarterFactor;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectBarterFactorC_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newBarterFactor = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newBarterFactor, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newBarterFactor > 0) {
			magic->barterFactor = newBarterFactor;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectEnchantFactor_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newEnchantFactor = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newEnchantFactor, &magic)) return true;

	if (magic) {
		if (newEnchantFactor > 0) {
			magic->enchantFactor = newEnchantFactor;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectEnchantFactorC_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newEnchantFactor = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newEnchantFactor, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newEnchantFactor > 0) {
			magic->enchantFactor = newEnchantFactor;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectProjectileSpeed_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newProjectileSpeed = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newProjectileSpeed, &magic)) return true;

	if (magic) {
		if (newProjectileSpeed > 0) {
			magic->projSpeed = newProjectileSpeed;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectProjectileSpeedC_Execute(COMMAND_ARGS)
{
	*result = 0;

	float newProjectileSpeed = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newProjectileSpeed, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newProjectileSpeed > 0) {
			magic->projSpeed = newProjectileSpeed;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectSchool_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 newSchool = 0;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newSchool, &magic)) return true;

	if (magic) {
		if ( newSchool >= EffectSetting::kEffect_Alteration && newSchool <= EffectSetting::kEffect_Restoration ) {
			magic->school = newSchool;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectSchoolC_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 newSchool = 0;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newSchool, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if ( newSchool >= EffectSetting::kEffect_Alteration && newSchool <= EffectSetting::kEffect_Restoration ) {
			magic->school = newSchool;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectResistValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 newResist = 0;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newResist, &magic)) return true;

	if (magic) {
		if ((newResist >= kActorVal_Strength && newResist < kActorVal_OblivionMax) || newResist == kActorVal_NoActorValue) { // need to test non-Resist AVs
			if (newResist == kActorVal_NoActorValue)
				magic->resistValue = kActorVal_NoActorValue_Proper;		// special case, as the game expects the correct value
			else
				magic->resistValue = newResist;

			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectResistValueC_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 newResist = 0;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newResist, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if ((newResist >= kActorVal_Strength && newResist < kActorVal_OblivionMax) || newResist == kActorVal_NoActorValue) { // need to test non-Resist AVs
			if (newResist == kActorVal_NoActorValue)
				magic->resistValue = kActorVal_NoActorValue_Proper;		// special case, as the game expects the correct value
			else
				magic->resistValue = newResist;

			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectLight_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESForm* form = NULL;
	EffectSetting* magic = NULL;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &magic)) form = form->TryGetREFRParent();
	if (form) {
		if (magic) {
			TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
			if (light)
				magic->light = light;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectLightC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESForm* form = NULL;
	UInt32 effectCode = 0;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &effectCode)) form = form->TryGetREFRParent();
	if (form) {
		EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
		if (magic) {
			TESObjectLIGH* light = (TESObjectLIGH*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectLIGH, 0);
			if (light)
				magic->light = light;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectCastingSound_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	EffectSetting* magic = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &magic)) return true;

	if (magic) {
		if (sound) {
			magic->castingSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectCastingSoundC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	UInt32 effectCode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (sound) {
			magic->castingSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectBoltSound_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	EffectSetting* magic = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &magic)) return true;

	if (magic) {
		if (sound) {
			magic->boltSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectBoltSoundC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	UInt32 effectCode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (sound) {
			magic->boltSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectHitSound_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	EffectSetting* magic = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &magic)) return true;

	if (magic) {
		if (sound) {
			magic->hitSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectHitSoundC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	UInt32 effectCode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (sound) {
			magic->hitSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectAreaSound_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	EffectSetting* magic = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &magic)) return true;

	if (magic) {
		if (sound) {
			magic->areaSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectAreaSoundC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESSound* sound = NULL;
	UInt32 effectCode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &sound, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (sound) {
			magic->areaSound = sound;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectHitShader_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESEffectShader* shader = NULL;
	EffectSetting* magic = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &shader, &magic)) return true;

	if (magic) {
		if (shader) {
			magic->effectShader = shader;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectHitShaderC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESEffectShader* shader = NULL;
	UInt32 effectCode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &shader, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (shader) {
			magic->effectShader = shader;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectEnchantShader_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESEffectShader* shader = NULL;
	EffectSetting* magic = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &shader, &magic)) return true;

	if (magic) {
		if (shader) {
			magic->enchantEffect = shader;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectEnchantShaderC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESEffectShader* shader = NULL;
	UInt32 effectCode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &shader, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (shader) {
			magic->enchantEffect = shader;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectIcon_Execute(COMMAND_ARGS)
{
	*result = 0;

	char newIcon[kMaxMessageLength];
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newIcon, &magic)) return true;

	if (magic) {
		if (newIcon[0] != '\0') {
			magic->texture.ddsPath.Set(newIcon);
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectIconC_Execute(COMMAND_ARGS)
{
	*result = 0;

	char newIcon[kMaxMessageLength];
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newIcon, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newIcon[0] != '\0') {
			magic->texture.ddsPath.Set(newIcon);
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectModel_Execute(COMMAND_ARGS)
{
	*result = 0;

	char newModel[kMaxMessageLength];
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newModel, &magic)) return true;

	if (magic) {
		if (newModel[0] != '\0') {
			magic->model.nifPath.Set(newModel);
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectModelC_Execute(COMMAND_ARGS)
{
	*result = 0;

	char newModel[kMaxMessageLength];
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newModel, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (newModel[0] != '\0') {
			magic->model.nifPath.Set(newModel);
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectFlag_Execute(COMMAND_ARGS, UInt32 whichFlag)
{
	*result = 0;

	UInt32 newValue = 0;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newValue, &magic)) return true;

	if (magic)
		magic->SetFlag(whichFlag, newValue != 0);
	return true;
}

static bool Cmd_SetMagicEffectFlagC_Execute(COMMAND_ARGS, UInt32 whichFlag)
{
	*result = 0;

	UInt32 newValue = 0;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newValue, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic)
		magic->SetFlag(whichFlag, newValue != 0);
	return true;
}

static bool Cmd_SetMagicEffectIsHostile_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_IsHostile);
}

static bool Cmd_SetMagicEffectIsHostileC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_IsHostile);
}

static bool Cmd_SetMagicEffectCanRecover_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_CanRecover);
}

static bool Cmd_SetMagicEffectCanRecoverC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_CanRecover);
}

static bool Cmd_SetMagicEffectIsDetrimental_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_IsDetrimental);
}

static bool Cmd_SetMagicEffectIsDetrimentalC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_IsDetrimental);
}

static bool Cmd_SetMagicEffectMagnitudePercent_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_MagnitudePercent);
}

static bool Cmd_SetMagicEffectMagnitudePercentC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_MagnitudePercent);
}

static bool Cmd_SetMagicEffectOnSelfAllowed_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_OnSelfAllowed);
}

static bool Cmd_SetMagicEffectOnSelfAllowedC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_OnSelfAllowed);
}

static bool Cmd_SetMagicEffectOnTouchAllowed_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_OnTouchAllowed);
}

static bool Cmd_SetMagicEffectOnTouchAllowedC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_OnTouchAllowed);
}

static bool Cmd_SetMagicEffectOnTargetAllowed_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_OnTargetAllowed);
}

static bool Cmd_SetMagicEffectOnTargetAllowedC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_OnTargetAllowed);
}

static bool Cmd_SetMagicEffectNoDuration_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoDuration);
}

static bool Cmd_SetMagicEffectNoDurationC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoDuration);
}

static bool Cmd_SetMagicEffectNoMagnitude_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoMagnitude);
}

static bool Cmd_SetMagicEffectNoMagnitudeC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoMagnitude);
}

static bool Cmd_SetMagicEffectNoArea_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoArea);
}

static bool Cmd_SetMagicEffectNoAreaC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoArea);
}

static bool Cmd_SetMagicEffectFXPersists_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_FXPersists);
}

static bool Cmd_SetMagicEffectFXPersistsC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_FXPersists);
}

static bool Cmd_SetMagicEffectForSpellmaking_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_ForSpellmaking);
}

static bool Cmd_SetMagicEffectForSpellmakingC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_ForSpellmaking);
}

static bool Cmd_SetMagicEffectForEnchanting_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_ForEnchanting);
}

static bool Cmd_SetMagicEffectForEnchantingC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_ForEnchanting);
}

static bool Cmd_SetMagicEffectNoIngredient_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoIngredient);
}

static bool Cmd_SetMagicEffectNoIngredientC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoIngredient);
}

static bool Cmd_SetMagicEffectUsesWeapon_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseWeapon);
}

static bool Cmd_SetMagicEffectUsesWeaponC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseWeapon);
}

static bool Cmd_SetMagicEffectUsesArmor_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseArmor);
}

static bool Cmd_SetMagicEffectUsesArmorC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseArmor);
}

static bool Cmd_SetMagicEffectUsesCreature_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseCreature);
}

static bool Cmd_SetMagicEffectUsesCreatureC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseCreature);
}

static bool Cmd_SetMagicEffectUsesSkill_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseSkill);
}

static bool Cmd_SetMagicEffectUsesSkillC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseSkill);
}

static bool Cmd_SetMagicEffectUsesAttribute_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseAttribute);
}

static bool Cmd_SetMagicEffectUsesAttributeC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseAttribute);
}

static bool Cmd_SetMagicEffectUsesActorValue_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseActorValue);
}

static bool Cmd_SetMagicEffectUsesActorValueC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_UseActorValue);
}

static bool Cmd_SetMagicEffectNoHitEffect_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlag_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoHitEffect);
}

static bool Cmd_SetMagicEffectNoHitEffectC_Execute(COMMAND_ARGS)
{
	return Cmd_SetMagicEffectFlagC_Execute(PASS_COMMAND_ARGS, EffectSetting::kEffect_NoHitEffect);
}

static bool Cmd_AddMagicEffectCounter_Execute(COMMAND_ARGS)
{
	*result = 0;

	EffectSetting* newCounter = NULL;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newCounter, &magic)) return true;

	if (magic) {
		if (newCounter) {
			*result = magic->numCounters+1;
			UInt32 * newCounters = (UInt32 *)FormHeap_Allocate((*result) * sizeof(UInt32));
			for ( int i = 0; i < magic->numCounters; ++i )
				newCounters[i] = magic->counterArray[i];
			newCounters[magic->numCounters] = newCounter->effectCode;
			FormHeap_Free(magic->counterArray);
			magic->counterArray = newCounters;
			magic->numCounters = *result;
		}
	}
	return true;
}

static bool Cmd_AddMagicEffectCounterC_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 newCounterCode = 0;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &newCounterCode, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (EffectSetting::EffectSettingForC(newCounterCode)) {
			*result = magic->numCounters+1;
			UInt32 * newCounters = (UInt32 *)FormHeap_Allocate((*result) * sizeof(UInt32));
			for ( int i = 0; i < magic->numCounters; ++i )
				newCounters[i] = magic->counterArray[i];
			newCounters[magic->numCounters] = newCounterCode;
			FormHeap_Free(magic->counterArray);
			magic->counterArray = newCounters;
			magic->numCounters = *result;
		}
	}
	return true;
}

static bool Cmd_RemoveNthMagicEffectCounter_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 index = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &index, &magic)) return true;

	if (magic) {
		if (index >= 0 && index < magic->numCounters) {
			*result = magic->numCounters-1;
			UInt32 * newCounters = (UInt32 *)FormHeap_Allocate((*result) * sizeof(UInt32));
			for ( int i = 0; i < magic->numCounters; ++i ) {
				if ( i < index )
					newCounters[ i ] = magic->counterArray[i];
				else if ( i > index )
					newCounters[i-1] = magic->counterArray[i];
			}
			FormHeap_Free(magic->counterArray);
			magic->counterArray = newCounters;
			magic->numCounters = *result;
		}
	}
	return true;
}

static bool Cmd_RemoveNthMagicEffectCounterC_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 index = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &index, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if (index >= 0 && index < magic->numCounters) {
			*result = magic->numCounters-1;
			UInt32 * newCounters = (UInt32 *)FormHeap_Allocate((*result) * sizeof(UInt32));
			for ( int i = 0; i < magic->numCounters; ++i ) {
				if ( i < index )
					newCounters[ i ] = magic->counterArray[i];
				else if ( i > index )
					newCounters[i-1] = magic->counterArray[i];
			}
			FormHeap_Free(magic->counterArray);
			magic->counterArray = newCounters;
			magic->numCounters = *result;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectCounters_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if ( eval.ExtractArgs() && eval.NumArgs() == 2 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Form))
	{
		EffectSetting* magic = OBLIVION_CAST(eval.Arg(1)->GetTESForm(), TESForm, EffectSetting);

		if (!magic)
			return true;

		 ArrayID arrayID = eval.Arg(0)->GetArray();
		if ( g_ArrayMap.GetKeyType(arrayID) == kDataType_Numeric )
		{
			FormHeap_Free(magic->counterArray);
			magic->numCounters = g_ArrayMap.SizeOf(arrayID);
			magic->counterArray = (UInt32 *)FormHeap_Allocate((magic->numCounters) * sizeof(UInt32));
			for (UInt32 i = 0; i < magic->numCounters; ++i)
			{
				double counterEffectCode = 0;
				if ( g_ArrayMap.GetElementNumber(arrayID, i, &counterEffectCode) )
					magic->counterArray[i] = counterEffectCode;
			}
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectCountersC_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if ( eval.ExtractArgs() && eval.NumArgs() == 2 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Number) )
	{
		EffectSetting* magic = EffectSetting::EffectSettingForC((UInt32)eval.Arg(1)->GetNumber());
		if (!magic)
			return true;

		ArrayID arrayID = eval.Arg(0)->GetArray();
		if ( g_ArrayMap.GetKeyType(arrayID) == kDataType_Numeric )
		{
			FormHeap_Free(magic->counterArray);
			magic->numCounters = g_ArrayMap.SizeOf(arrayID);
			magic->counterArray = (UInt32 *)FormHeap_Allocate((magic->numCounters) * sizeof(UInt32));
			for (UInt32 i = 0; i < magic->numCounters; ++i)
			{
				double counterEffectCode = 0;
				if ( g_ArrayMap.GetElementNumber(arrayID, i, &counterEffectCode) )
					magic->counterArray[i] = counterEffectCode;
			}
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectOtherActorValue_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 actorVal = -1;
	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &actorVal, &magic)) return true;

	if (magic) {
		if ( actorVal >= kActorVal_Strength && actorVal < kActorVal_OblivionMax ) {
			magic->data = actorVal;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectOtherActorValueC_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 actorVal = -1;
	UInt32 effectCode = 0;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &actorVal, &effectCode)) return true;

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic) {
		if ( actorVal >= kActorVal_Strength && actorVal < kActorVal_OblivionMax ) {
			magic->data = actorVal;
			*result = 1;
		}
	}
	return true;
}

static bool Cmd_SetMagicEffectUsedObject_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESForm* form = NULL;
	EffectSetting* magic = NULL;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &magic)) form = form->TryGetREFRParent();

	if (magic && form) {
		magic->data = form->refID;
		*result = 1;
	}
	return true;
}

static bool Cmd_SetMagicEffectUsedObjectC_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESForm* form = NULL;
	UInt32 effectCode = 0;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &effectCode)) form = form->TryGetREFRParent();

	EffectSetting* magic = EffectSetting::EffectSettingForC(effectCode);
	if (magic && form) {
		magic->data = form->refID;
		*result = 1;
	}
	return true;
}

static bool Cmd_MagicEffectFromCode_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	UInt32 code = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &code) && code)
	{
		EffectSetting* eff = EffectSetting::EffectSettingForC(code);
		if (eff)
			*refResult = eff->refID;
	}

	return true;
}

enum {
	kMGEF_Ref,
	kMGEF_Code
};

static bool MagicEffectFromChars_Execute(COMMAND_ARGS, UInt32 which)
{
	char str[kMaxMessageLength] = { 0 };
	UInt32* refResult = (UInt32*)result;
	*result = 0;

	if (which == kMGEF_Ref) {
		*refResult = 0;
	}

	if (ExtractArgs(PASS_EXTRACT_ARGS, str) && strlen(str) == 4)
	{
		UInt32 code = *((UInt32*)str);
		EffectSetting* eff = EffectSetting::EffectSettingForC(code);
		if (eff)
		{
			if (which == kMGEF_Ref) {
				*refResult = eff->refID;
			}
			else {
				*result = (SInt32)eff->effectCode;
			}
		}
	}

	return true;
}

static bool Cmd_MagicEffectFromChars_Execute(COMMAND_ARGS)
{
	return MagicEffectFromChars_Execute(PASS_COMMAND_ARGS, kMGEF_Ref);
}

static bool Cmd_MagicEffectCodeFromChars_Execute(COMMAND_ARGS)
{
	return MagicEffectFromChars_Execute(PASS_COMMAND_ARGS, kMGEF_Code);
}

static bool GetMagicEffectChars_Execute(COMMAND_ARGS, bool bUseCode)
{
	UInt32 effCode = 0;
	char effChars[5] = { '0' };
	EffectSetting* eff = NULL;
	if (bUseCode) {
		if (ExtractArgs(PASS_EXTRACT_ARGS, &effCode)) {
			eff = EffectSetting::EffectSettingForC(effCode);
		}
	}
	else {
		ExtractArgs(PASS_EXTRACT_ARGS, &eff);
	}

	if (eff) {
		eff->GetEffectChars(effChars);
	}

	AssignToStringVar(PASS_COMMAND_ARGS, effChars);
	return true;
}

static bool Cmd_GetMagicEffectChars_Execute(COMMAND_ARGS)
{
	return GetMagicEffectChars_Execute(PASS_COMMAND_ARGS, false);
}

static bool Cmd_GetMagicEffectCharsC_Execute(COMMAND_ARGS)
{
	return GetMagicEffectChars_Execute(PASS_COMMAND_ARGS, true);
}

static bool Cmd_DumpMagicEffectUnknowns_Execute(COMMAND_ARGS)
{
	*result = 0;

	EffectSetting* magic = NULL;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &magic)) return true;

	if (magic) {
		std::string MEdump = "'";
		MEdump+=magic->fullName.name.m_data;
		MEdump+="'";
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownF)?" F":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownG)?" G":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownM)?" M":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownN)?" N":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownO)?" O":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownQ)?" Q":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownR)?" R":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownT)?" T":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownU)?" U":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownV)?" V":"  ");
		MEdump+=(magic->IsFlagSet(EffectSetting::kEffect_UnknownW)?" W":"  ");
		MEdump+=(magic->unk0[0]);
		MEdump+="  ";
		MEdump+=(magic->unk0[1]);
		MEdump+="  ";
		MEdump+=(magic->pad06E);
		MEdump+="  ";
		MEdump+=(magic->unk4[0]);
		MEdump+="  ";
		MEdump+=(magic->unk4[1]);
		_MESSAGE(MEdump.c_str());
		*result = 1;
	}
	return true;
}

#endif

CommandInfo kCommandInfo_GetMagicEffectName =
{
	"GetMagicEffectName",
	"GetMEName",
	0,
	"returns the name of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectNameC =
{
	"GetMagicEffectNameC",
	"GetMENameC",
	0,
	"returns the name of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectNameC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectCode =
{
	"GetMagicEffectCode",
	"GetMECode",
	0,
	"returns the effect code for the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectCode_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectBaseCost =
{
	"GetMagicEffectBaseCost",
	"GetMEBaseCost",
	0,
	"returns the specified base cost for the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectBaseCost_Execute),
	Cmd_Default_Parse,NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectSchool =
{
	"GetMagicEffectSchool",
	"GetMESchool",
	0,
	"returns the magic school for the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectSchool_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectProjectileSpeed =
{
	"GetMagicEffectProjectileSpeed",
	"GetMEProjSpeed",
	0,
	"returns the projectile speed for the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectProjectileSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectEnchantFactor =
{
	"GetMagicEffectEnchantFactor",
	"GetMEEnchant",
	0,
	"returns the enchantment factor for the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectEnchantFactor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectBarterFactor =
{
	"GetMagicEffectBarterFactor",
	"GetMEBarter",
	0,
	"returns the barter factor for the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectBarterFactor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectBaseCostC =
{
	"GetMagicEffectBaseCostC",
	"GetMEBaseCostC",
	0,
	"returns the specified base cost for the given magic effect code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectBaseCostC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectSchoolC =
{
	"GetMagicEffectSchoolC",
	"GetMESchoolC",
	0,
	"returns the magic school for the given magic effect code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectSchoolC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectProjectileSpeedC =
{
	"GetMagicEffectProjectileSpeedC",
	"GetMEProjSpeedC",
	0,
	"returns the projectile speed for the given magic effect code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectProjectileSpeedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectEnchantFactorC =
{
	"GetMagicEffectEnchantFactorC",
	"GetMEEnchantC",
	0,
	"returns the enchantment factor for the given magic effect code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectEnchantFactorC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectBarterFactorC =
{
	"GetMagicEffectBarterFactorC",
	"GetMEBarterC",
	0,
	"returns the barter factor for the given magic effect code",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectBarterFactorC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectHostile =
{
	"IsMagicEffectHostile",
	"IsMEHostile",
	0,
	"returns 1 if the passed magic effect is hostile",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectHostile_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectHostileC =
{
	"IsMagicEffectHostileC",
	"IsMEHostileC",
	0,
	"returns 1 if the passed magic effect is hostile",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectHostileC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectForSpellmaking =
{
	"IsMagicEffectForSpellmaking",
	"IsMEForSpellmaking",
	0,
	"returns 1 if the passed magic effect is allowed for spellmaking",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectForSpellmaking_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectForSpellmakingC =
{
	"IsMagicEffectForSpellmakingC",
	"IsMEForSpell",
	0,
	"returns 1 if the passed magic effect is allowed for spellmaking",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectForSpellmakingC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectForEnchanting =
{
	"IsMagicEffectForEnchanting",
	"IsMEForEnchant",
	0,
	"returns 1 if the passed magic effect is allowed for enchanting",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectForEnchanting_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectForEnchantingC =
{
	"IsMagicEffectForEnchantingC",
	"IsMEForEnchantC",
	0,
	"returns 1 if the passed magic effect is allowed for enchanting",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectForEnchantingC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectDetrimental =
{
	"IsMagicEffectDetrmimental",
	"IsMEDetrimental",
	0,
	"returns 1 if the passed magic effect is detrimental",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectDetrimental_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectDetrimentalC =
{
	"IsMagicEffectDetrimentalC",
	"IsMEDetrimentalC",
	0,
	"returns 1 if the passed magic effect detrimental",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectDetrimentalC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectCanRecover =
{
	"IsMagicEffectCanRecover",
	"IsMECanRecover",
	0,
	"returns 1 if the passed magic effect is marked can recover",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectCanRecover_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectCanRecoverC =
{
	"IsMagicEffectCanRecoverC",
	"IsMECanRecoverC",
	0,
	"returns 1 if the passed magic effect is marked can recover",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectCanRecoverC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectMagnitudePercent =
{
	"IsMagicEffectMagnitudePercent",
	"IsMEMagnitudePercent",
	0,
	"returns 1 if the passed magic effect is marked can recover",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectMagnitudePercent_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectMagnitudePercentC =
{
	"IsMagicEffectMagnitudePercentC",
	"IsMEMagnitudePercentC",
	0,
	"returns 1 if the passed magic effect is marked can recover",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectMagnitudePercentC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectFXPersists =
{
	"MagicEffectFXPersists",
	"MEFXPersists",
	0,
	"returns 1 if the passed magic effect's graphic effects persist",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectFXPersists_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectFXPersistsC =
{
	"MagicEffectFXPersistsC",
	"MEFXPersistsC",
	0,
	"returns 1 if the passed magic effect's graphic effects persist",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectFXPersistsC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectOnSelfAllowed =
{
	"IsMagicEffectOnSelfAllowed",
	"IsMEOnSelfAllowed",
	0,
	"returns 1 if the passed magic effect can target self",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectOnSelfAllowed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectOnSelfAllowedC =
{
	"IsMagicEffectOnSelfAllowedC",
	"IsMEOnSelfAllowedC",
	0,
	"returns 1 if the passed magic effect can target self",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectOnSelfAllowedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectOnTouchAllowed =
{
	"IsMagicEffectOnTouchAllowed",
	"IsMEOnTouchAllowed",
	0,
	"returns 1 if the passed magic effect can target touch",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectOnTouchAllowed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectOnTouchAllowedC =
{
	"IsMagicEffectOnTouchAllowedC",
	"IsMEOnTouchAllowedC",
	0,
	"returns 1 if the passed magic effect can target ouch",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectOnTouchAllowedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectOnTargetAllowed =
{
	"IsMagicEffectOnTargetAllowed",
	"IsMEOnTargetAllowed",
	0,
	"returns 1 if the passed magic effect can target target",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_IsMagicEffectOnTargetAllowed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMagicEffectOnTargetAllowedC =
{
	"IsMagicEffectOnTargetAllowedC",
	"IsMEOnTargetAllowedC",
	0,
	"returns 1 if the passed magic effect can target arget",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_IsMagicEffectOnTargetAllowedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoDuration =
{
	"MagicEffectHasNoDuration",
	"MEHasNoDuration",
	0,
	"returns 1 if the passed magic effect has no duration",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectHasNoDuration_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoDurationC =
{
	"MagicEffectHasNoDurationC",
	"MEHasNoDurationC",
	0,
	"returns 1 if the passed magic effect has no duration",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectHasNoDurationC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoMagnitude =
{
	"MagicEffectHasNoMagnitude",
	"MEHasNoMagnitude",
	0,
	"returns 1 if the passed magic effect has no magnitude",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectHasNoMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoMagnitudeC =
{
	"MagicEffectHasNoMagnitudeC",
	"MEHasNoMagnitudeC",
	0,
	"returns 1 if the passed magic effect has no magnitude",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectHasNoMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoArea =
{
	"MagicEffectHasNoArea",
	"MEHasNoArea",
	0,
	"returns 1 if the passed magic effect has no area",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectHasNoArea_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoAreaC =
{
	"MagicEffectHasNoAreaC",
	"MEHasNoAreaC",
	0,
	"returns 1 if the passed magic effect has no area",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectHasNoAreaC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoIngredient =
{
	"MagicEffectHasNoIngredient",
	"MEHasNoIngredient",
	0,
	"returns 1 if the passed magic effect has no ingredient",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectHasNoIngredient_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoIngredientC =
{
	"MagicEffectHasNoIngredientC",
	"MEHasNoIngredientC",
	0,
	"returns 1 if the passed magic effect has no ingredient",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectHasNoIngredientC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoHitEffect =
{
	"MagicEffectHasNoHitEffect",
	"MEHasNoHitEffect",
	0,
	"returns 1 if the passed magic effect has no hit effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectHasNoHitEffect_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectHasNoHitEffectC =
{
	"MagicEffectHasNoHitEffectC",
	"MEHasNoHitEffectC",
	0,
	"returns 1 if the passed magic effect has no hit effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectHasNoHitEffectC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesWeapon =
{
	"MagicEffectUsesWeapon",
	"MEUsesWeapon",
	0,
	"returns 1 if the passed magic effect uses a weapon",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectUsesWeapon_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesWeaponC =
{
	"MagicEffectUsesWeaponC",
	"MEUsesWeaponC",
	0,
	"returns 1 if the passed magic effect uses a weapon",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectUsesWeaponC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesArmor =
{
	"MagicEffectUsesArmor",
	"MEUsesArmor",
	0,
	"returns 1 if the passed magic effect uses armor",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectUsesArmor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesArmorC =
{
	"MagicEffectUsesArmorC",
	"MEUsesArmorC",
	0,
	"returns 1 if the passed magic effect uses armor",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectUsesArmorC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesCreature =
{
	"MagicEffectUsesCreature",
	"MEUsesCreature",
	0,
	"returns 1 if the passed magic effect uses a creature",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectUsesCreature_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesCreatureC =
{
	"MagicEffectUsesCreatureC",
	"MEUsesCreatureC",
	0,
	"returns 1 if the passed magic effect uses a creature",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectUsesCreatureC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesSkill =
{
	"MagicEffectUsesSkill",
	"MEUsesSkill",
	0,
	"returns 1 if the passed magic effect uses a Skill",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectUsesSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesSkillC =
{
	"MagicEffectUsesSkillC",
	"MEUsesSkillC",
	0,
	"returns 1 if the passed magic effect uses a Skill",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectUsesSkillC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesAttribute =
{
	"MagicEffectUsesAttribute",
	"MEUsesAttribute",
	0,
	"returns 1 if the passed magic effect uses an attribute",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectUsesAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesAttributeC =
{
	"MagicEffectUsesAttributeC",
	"MEUsesAttributeC",
	0,
	"returns 1 if the passed magic effect uses an attribute",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectUsesAttributeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesOtherActorValue =
{
	"MagicEffectUsesOtherActorValue",
	"MEUsesOtherAV",
	0,
	"returns 1 if the passed magic effect uses another actor value",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_MagicEffectUsesOtherActorValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MagicEffectUsesOtherActorValueC =
{
	"MagicEffectUsesOtherActorValueC",
	"MEUsesOtherAVC",
	0,
	"returns 1 if the passed magic effect uses another actor value",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_MagicEffectUsesOtherActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectOtherActorValue =
{
	"GetMagicEffectOtherActorValue",
	"GetMEOtherAV",
	0,
	"returns the magic effect's other actor value",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectOtherActorValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectOtherActorValueC =
{
	"GetMagicEffectOtherActorValueC",
	"GetMEOtherAVC",
	0,
	"returns the magic effect's other actor value",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectOtherActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectUsedObject =
{
	"GetMagicEffectUsedObject",
	"GetMEUsedObject",
	0,
	"returns the magic effect's used object",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectUsedObject_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectUsedObjectC =
{
	"GetMagicEffectUsedObjectC",
	"GetMEUsedObjectC",
	0,
	"returns the magic effect's used object",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectUsedObjectC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetMagicEffectValue[2] =
{
	{	"value", kParamType_Integer, 0 },
	{	"magic effect", kParamType_MagicEffect, 0 },
};

CommandInfo kCommandInfo_GetMagicEffectValue =
{
	"GetMagicEffectValue",
	"GetMEV",
	0,
	"returns the specified value for the given magic effect",
	0,
	2,
	kParams_GetMagicEffectValue,
	HANDLER(Cmd_GetMagicEffectValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetMagicEffectCodeValue[2] =
{
	{	"value", kParamType_Integer, 0 },
	{	"effect code", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_GetMagicEffectCodeValue =
{
	"GetMagicEffectCodeValue",
	"GetMECV",
	0,
	"returns the specified value for the given magic effect code",
	0,
	2,
	kParams_GetMagicEffectCodeValue,
	HANDLER(Cmd_GetMagicEffectCodeValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectHitShader =
{
	"GetMagicEffectHitShader",
	"GetMEHitShader",
	0,
	"returns the Effect Shader of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectHitShader_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectHitShaderC =
{
	"GetMagicEffectHitShaderC",
	"GetMEHitShaderC",
	0,
	"returns the Effect Shader of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectHitShaderC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectEnchantShader =
{
	"GetMagicEffectEnchantShader",
	"GetMEEnchantShader",
	0,
	"returns the Enchant Effect of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectEnchantShader_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectEnchantShaderC =
{
	"GetMagicEffectEnchantShaderC",
	"GetMEEnchantShaderC",
	0,
	"returns the Enchant Effect of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectEnchantShaderC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectLight =
{
	"GetMagicEffectLight",
	"GetMELight",
	0,
	"returns the Light of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectLight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectLightC =
{
	"GetMagicEffectLightC",
	"GetMELightC",
	0,
	"returns the Light of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectLightC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectCastingSound =
{
	"GetMagicEffectCastingSound",
	"GetMECastingSound",
	0,
	"returns the Casting Sound of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectCastingSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectCastingSoundC =
{
	"GetMagicEffectCastingSoundC",
	"GetMECastingSoundC",
	0,
	"returns the Casting Sound of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectCastingSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectBoltSound =
{
	"GetMagicEffectBoltSound",
	"GetMEBoltSound",
	0,
	"returns the Bolt Sound of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectBoltSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectBoltSoundC =
{
	"GetMagicEffectBoltSoundC",
	"GetMEBoltSoundC",
	0,
	"returns the Bolt Sound of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectBoltSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectHitSound =
{
	"GetMagicEffectHitSound",
	"GetMEHitSound",
	0,
	"returns the Hit Sound of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectHitSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectHitSoundC =
{
	"GetMagicEffectHitSoundC",
	"GetMEHitSoundC",
	0,
	"returns the Hit Sound of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectHitSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectAreaSound =
{
	"GetMagicEffectAreaSound",
	"GetMEAreaSound",
	0,
	"returns the Area Sound of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectAreaSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectAreaSoundC =
{
	"GetMagicEffectAreaSoundC",
	"GetMEAreaSoundC",
	0,
	"returns the Area Sound of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectAreaSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectNumCounters =
{
	"GetMagicEffectNumCounters",
	"GetMENumCounters",
	0,
	"returns the number of counters to the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectNumCounters_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectNumCountersC =
{
	"GetMagicEffectNumCountersC",
	"GetMENumCountersC",
	0,
	"returns the number of counters to the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectNumCountersC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectResistValue =
{
	"GetMagicEffectResistValue",
	"GetMEResistValue",
	0,
	"returns the Resist Value of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectResistValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectResistValueC =
{
	"GetMagicEffectResistValueC",
	"GetMEResistValueC",
	0,
	"returns the Resist Value of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectResistValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneInt_OneMagicEffect[2] =
{
	{	"int",			kParamType_Integer,		0	},
	{	"magic effect",	kParamType_MagicEffect,	0	},
};

CommandInfo kCommandInfo_GetNthMagicEffectCounter =
{
	"GetNthMagicEffectCounter",
	"GetNthMECounter",
	0,
	"returns the Nth counter to the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_GetNthMagicEffectCounter_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthMagicEffectCounterC =
{
	"GetNthMagicEffectCounterC",
	"GetNthMECounterC",
	0,
	"returns the Nth counter to the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_GetNthMagicEffectCounterC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectCounters =
{
	"GetMagicEffectCounters",
	"GetMECounters",
	0,
	"returns an array of the counters to the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectCounters_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectCountersC =
{
	"GetMagicEffectCountersC",
	"GetMECountersC",
	0,
	"returns an array of the counters to the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectCountersC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectIcon =
{
	"GetMagicEffectIcon",
	"GetMEIcon",
	0,
	"returns the file path to the icon of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectIcon_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectIconC =
{
	"GetMagicEffectIconC",
	"GetMEIconC",
	0,
	"returns the file path to the icon of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectIconC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectModel =
{
	"GetMagicEffectModel",
	"GetMEModel",
	0,
	"returns the file path to the model of the given magic effect",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_GetMagicEffectModel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetMagicEffectModelC =
{
	"GetMagicEffectModelC",
	"GetMEModelC",
	0,
	"returns the file path to the model of the given magic effect",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetMagicEffectModelC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectIsHostile =
{
	"SetMagicEffectIsHostile",
	"SetMEIsHostile",
	0,
	"sets the Is Hostile flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectIsHostile_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectIsHostileC =
{
	"SetMagicEffectIsHostileC",
	"SetMEIsHostileC",
	0,
	"sets the Is Hostile flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectIsHostileC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectCanRecover =
{
	"SetMagicEffectCanRecover",
	"SetMECanRecover",
	0,
	"sets the Can Recover flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectCanRecover_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectCanRecoverC =
{
	"SetMagicEffectCanRecoverC",
	"SetMECanRecoverC",
	0,
	"sets the Can Recover flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectCanRecoverC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectIsDetrimental =
{
	"SetMagicEffectIsDetrimental",
	"SetMEIsDetrimental",
	0,
	"sets the IsDetrimental flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectIsDetrimental_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectIsDetrimentalC =
{
	"SetMagicEffectIsDetrimentalC",
	"SetMEIsDetrimentalC",
	0,
	"sets the Is Detrimental flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectIsDetrimentalC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectMagnitudePercent =
{
	"SetMagicEffectMagnitudePercent",
	"SetMEMagnitudePercent",
	0,
	"sets the Magnitude Percent flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectMagnitudePercent_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectMagnitudePercentC =
{
	"SetMagicEffectMagnitudePercentC",
	"SetMEMagnitudePercentC",
	0,
	"sets the Magnitude Percent flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectMagnitudePercentC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOnSelfAllowed =
{
	"SetMagicEffectOnSelfAllowed",
	"SetMEOnSelfAllowed",
	0,
	"sets the On Self Allowed flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectOnSelfAllowed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOnSelfAllowedC =
{
	"SetMagicEffectOnSelfAllowedC",
	"SetMEOnSelfAllowedC",
	0,
	"sets the On Self Allowed flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectOnSelfAllowedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOnTouchAllowed =
{
	"SetMagicEffectOnTouchAllowed",
	"SetMEOnTouchAllowed",
	0,
	"sets the On Touch Allowed flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectOnTouchAllowed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOnTouchAllowedC =
{
	"SetMagicEffectOnTouchAllowedC",
	"SetMEOnTouchAllowedC",
	0,
	"sets the On Touch Allowed flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectOnTouchAllowedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOnTargetAllowed =
{
	"SetMagicEffectOnTargetAllowed",
	"SetMEOnTargetAllowed",
	0,
	"sets the On Target Allowed flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectOnTargetAllowed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOnTargetAllowedC =
{
	"SetMagicEffectOnTargetAllowedC",
	"SetMEOnTargetAllowedC",
	0,
	"sets the On Target Allowed flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectOnTargetAllowedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoDuration =
{
	"SetMagicEffectNoDuration",
	"SetMENoDuration",
	0,
	"sets the No Duration flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectNoDuration_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoDurationC =
{
	"SetMagicEffectNoDurationC",
	"SetMENoDurationC",
	0,
	"sets the No Duration flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectNoDurationC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoMagnitude =
{
	"SetMagicEffectNoMagnitude",
	"SetMENoMagnitude",
	0,
	"sets the No Magnitude flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectNoMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoMagnitudeC =
{
	"SetMagicEffectNoMagnitudeC",
	"SetMENoMagnitudeC",
	0,
	"sets the No Magnitude flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectNoMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoArea =
{
	"SetMagicEffectNoArea",
	"SetMENoArea",
	0,
	"sets the No Area flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectNoArea_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoAreaC =
{
	"SetMagicEffectNoAreaC",
	"SetMENoAreaC",
	0,
	"sets the No Area flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectNoAreaC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectFXPersists =
{
	"SetMagicEffectFXPersists",
	"SetMEFXPersists",
	0,
	"sets the FX Persists flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectFXPersists_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectFXPersistsC =
{
	"SetMagicEffectFXPersistsC",
	"SetMEFXPersistsC",
	0,
	"sets the FX Persists flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectFXPersistsC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectForSpellmaking =
{
	"SetMagicEffectForSpellmaking",
	"SetMEForSpellmaking",
	0,
	"sets the For Spellmaking flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectForSpellmaking_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectForSpellmakingC =
{
	"SetMagicEffectForSpellmakingC",
	"SetMEForSpellmakingC",
	0,
	"sets the For Spellmaking flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectForSpellmakingC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectForEnchanting =
{
	"SetMagicEffectForEnchanting",
	"SetMEForEnchant",
	0,
	"sets the For Enchanting flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectForEnchanting_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectForEnchantingC =
{
	"SetMagicEffectForEnchantingC",
	"SetMEForEnchantC",
	0,
	"sets the For Enchanting flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectForEnchantingC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoIngredient =
{
	"SetMagicEffectNoIngredient",
	"SetMENoIngredient",
	0,
	"sets the No Ingredient flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectNoIngredient_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoIngredientC =
{
	"SetMagicEffectNoIngredientC",
	"SetMENoIngredientC",
	0,
	"sets the No Ingredient flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectNoIngredientC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesWeapon =
{
	"SetMagicEffectUsesWeapon",
	"SetMEUsesWeapon",
	0,
	"sets the Use Weapon flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsesWeapon_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesWeaponC =
{
	"SetMagicEffectUsesWeaponC",
	"SetMEUsesWeaponC",
	0,
	"sets the Use Weapon flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectUsesWeaponC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesArmor =
{
	"SetMagicEffectUsesArmor",
	"SetMEUsesArmor",
	0,
	"sets the Use Armor flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsesArmor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesArmorC =
{
	"SetMagicEffectUsesArmorC",
	"SetMEUsesArmorC",
	0,
	"sets the Use Armor flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectUsesArmorC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesCreature =
{
	"SetMagicEffectUsesCreature",
	"SetMEUsesCreature",
	0,
	"sets the Use Creature flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsesCreature_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesCreatureC =
{
	"SetMagicEffectUsesCreatureC",
	"SetMEUsesCreatureC",
	0,
	"sets the Use Creature flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectUsesCreatureC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesSkill =
{
	"SetMagicEffectUsesSkill",
	"SetMEUsesSkill",
	0,
	"sets the Use Skill flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsesSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesSkillC =
{
	"SetMagicEffectUsesSkillC",
	"SetMEUsesSkillC",
	0,
	"sets the Use Skill flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectUsesSkillC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesAttribute =
{
	"SetMagicEffectUsesAttribute",
	"SetMEUsesAttribute",
	0,
	"sets the Use Attribute flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsesAttribute_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesAttributeC =
{
	"SetMagicEffectUsesAttributeC",
	"SetMEUsesAttributeC",
	0,
	"sets the Use Attribute flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectUsesAttributeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesActorValue =
{
	"SetMagicEffectUseActorValue",
	"SetMEUseActorValue",
	0,
	"sets the Use Actor Value flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsesActorValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsesActorValueC =
{
	"SetMagicEffectUseActorValueC",
	"SetMEUseActorValueC",
	0,
	"sets the Use Actor Value flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectUsesActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoHitEffect =
{
	"SetMagicEffectNoHitEffect",
	"SetMENoHitEffect",
	0,
	"sets the No Hit Effect flag of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectNoHitEffect_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNoHitEffectC =
{
	"SetMagicEffectNoHitEffectC",
	"SetMENoHitEffectC",
	0,
	"sets the No Hit Effect flag of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectNoHitEffectC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneString_OneMagicEffect[2] =
{
	{	"string",		kParamType_String,		0 },
	{	"magic effect",	kParamType_MagicEffect,	0 },
};

CommandInfo kCommandInfo_SetMagicEffectName =
{
	"SetMagicEffectName",
	"SetMEName",
	0,
	"sets the Name of the given magic effect",
	0,
	2,
	kParams_OneString_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectName_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectNameC =
{
	"SetMagicEffectNameC",
	"SetMENameC",
	0,
	"sets the Name of the given magic effect",
	0,
	2,
	kParams_OneString_OneInt,
	HANDLER(Cmd_SetMagicEffectNameC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectIcon =
{
	"SetMagicEffectIcon",
	"SetMEIcon",
	0,
	"sets the Icon of the given magic effect",
	0,
	2,
	kParams_OneString_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectIcon_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectIconC =
{
	"SetMagicEffectIconC",
	"SetMEIconC",
	0,
	"sets the Icon of the given magic effect",
	0,
	2,
	kParams_OneString_OneInt,
	HANDLER(Cmd_SetMagicEffectIconC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectModel =
{
	"SetMagicEffectModel",
	"SetMEModel",
	0,
	"sets the Model of the given magic effect",
	0,
	2,
	kParams_OneString_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectModel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectModelC =
{
	"SetMagicEffectModelC",
	"SetMEModelC",
	0,
	"sets the Model of the given magic effect",
	0,
	2,
	kParams_OneString_OneInt,
	HANDLER(Cmd_SetMagicEffectModelC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneFloat_OneMagicEffect[2] =
{
	{	"float",		kParamType_Float,		0 },
	{	"magic effect",	kParamType_MagicEffect,	0 },
};

CommandInfo kCommandInfo_SetMagicEffectBaseCost =
{
	"SetMagicEffectBaseCost",
	"SetMEBaseCost",
	0,
	"sets the Base Cost of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectBaseCost_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectBaseCostC =
{
	"SetMagicEffectBaseCostC",
	"SetMEBaseCostC",
	0,
	"sets the Base Cost of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneInt,
	HANDLER(Cmd_SetMagicEffectBaseCostC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectBarterFactor =
{
	"SetMagicEffectBarterFactor",
	"SetMEBarterFactor",
	0,
	"sets the Barter Factor of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectBarterFactor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectBarterFactorC =
{
	"SetMagicEffectBarterFactorC",
	"SetMEBarterFactorC",
	0,
	"sets the Barter Factor of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneInt,
	HANDLER(Cmd_SetMagicEffectBarterFactorC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectEnchantFactor =
{
	"SetMagicEffectEnchantFactor",
	"SetMEEnchantFactor",
	0,
	"sets the Enchant Factor of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectEnchantFactor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectEnchantFactorC =
{
	"SetMagicEffectEnchantFactorC",
	"SetMEEnchantFactorC",
	0,
	"sets the Enchant Factor of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneInt,
	HANDLER(Cmd_SetMagicEffectEnchantFactorC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectProjectileSpeed =
{
	"SetMagicEffectProjectileSpeed",
	"SetMEProjSpeed",
	0,
	"sets the Projectile Speed of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectProjectileSpeed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectProjectileSpeedC =
{
	"SetMagicEffectProjectileSpeedC",
	"SetMEProjSpeedC",
	0,
	"sets the Projectile Speed of the given magic effect",
	0,
	2,
	kParams_OneFloat_OneInt,
	HANDLER(Cmd_SetMagicEffectProjectileSpeedC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectSchool =
{
	"SetMagicEffectSchool",
	"SetMESchool",
	0,
	"sets the School of the given magic effect",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectSchool_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectSchoolC =
{
	"SetMagicEffectSchoolC",
	"SetMESchoolC",
	0,
	"sets the School of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectSchoolC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneActorValue_OneMagicEffect[2] =
{
	{ "actor value",	kParamType_ActorValue,	0 },
	{ "magic effect",	kParamType_MagicEffect,	0 },
};

CommandInfo kCommandInfo_SetMagicEffectResistValue =
{
	"SetMagicEffectResistValue",
	"SetMEResistValue",
	0,
	"sets the Resist Value of the given magic effect",
	0,
	2,
	kParams_OneActorValue_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectResistValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectResistValueC =
{
	"SetMagicEffectResistValueC",
	"SetMEResistValueC",
	0,
	"sets the Resist Value of the given magic effect",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectResistValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneInventoryObject_OneMagicEffect[2] =
{
	{ "item",			kParamType_InventoryObject,	0 },
	{ "magic effect",	kParamType_MagicEffect,		0 },
};

CommandInfo kCommandInfo_SetMagicEffectLight =
{
	"SetMagicEffectLight",
	"SetMELight",
	0,
	"sets the Light of the given magic effect",
	0,
	2,
	kParams_OneInventoryObject_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectLight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectLightC =
{
	"SetMagicEffectLightC",
	"SetMELightC",
	0,
	"sets the Light of the given magic effect",
	0,
	2,
	kParams_OneInventoryObject_OneInt,
	HANDLER(Cmd_SetMagicEffectLightC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneSound_OneMagicEffect[2] =
{
	{	"sound",		kParamType_Sound,		0 },
	{	"magic effect",	kParamType_MagicEffect,	0 },
};

CommandInfo kCommandInfo_SetMagicEffectCastingSound =
{
	"SetMagicEffectCastingSound",
	"SetMECastingSound",
	0,
	"sets the Casting Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectCastingSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectCastingSoundC =
{
	"SetMagicEffectCastingSoundC",
	"SetMECastingSoundC",
	0,
	"sets the Casting Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneInt,
	HANDLER(Cmd_SetMagicEffectCastingSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectBoltSound =
{
	"SetMagicEffectBoltSound",
	"SetMEBoltSound",
	0,
	"sets the Bolt Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectBoltSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectBoltSoundC =
{
	"SetMagicEffectBoltSoundC",
	"SetMEBoltSoundC",
	0,
	"sets the Bolt Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneInt,
	HANDLER(Cmd_SetMagicEffectBoltSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectHitSound =
{
	"SetMagicEffectHitSound",
	"SetMEHitSound",
	0,
	"sets the Hit Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectHitSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectHitSoundC =
{
	"SetMagicEffectHitSoundC",
	"SetMEHitSoundC",
	0,
	"sets the Hit Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneInt,
	HANDLER(Cmd_SetMagicEffectHitSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectAreaSound =
{
	"SetMagicEffectAreaSound",
	"SetMEAreaSound",
	0,
	"sets the Area Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectAreaSound_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectAreaSoundC =
{
	"SetMagicEffectAreaSoundC",
	"SetMEAreaSoundC",
	0,
	"sets the Area Sound of the given magic effect",
	0,
	2,
	kParams_OneSound_OneInt,
	HANDLER(Cmd_SetMagicEffectAreaSoundC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneEffectShader_OneMagicEffect[2] =
{
	{	"effect shader",	kParamType_EffectShader,	0 },
	{	"magic effect",		kParamType_MagicEffect,		0 },
};

CommandInfo kCommandInfo_SetMagicEffectHitShader =
{
	"SetMagicEffectHitShader",
	"SetMEHitShader",
	0,
	"sets the Hit Shader of the given magic effect",
	0,
	2,
	kParams_OneEffectShader_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectHitShader_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectHitShaderC =
{
	"SetMagicEffectHitShaderC",
	"SetMEHitShaderC",
	0,
	"sets the Hit Shader of the given magic effect",
	0,
	2,
	kParams_OneEffectShader_OneInt,
	HANDLER(Cmd_SetMagicEffectHitShaderC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectEnchantShader =
{
	"SetMagicEffectEnchantShader",
	"SetMEEnchantShader",
	0,
	"sets the Enchant Shader of the given magic effect",
	0,
	2,
	kParams_OneEffectShader_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectEnchantShader_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectEnchantShaderC =
{
	"SetMagicEffectEnchantShaderC",
	"SetMEEnchantShaderC",
	0,
	"sets the Enchant Shader of the given magic effect",
	0,
	2,
	kParams_OneEffectShader_OneInt,
	HANDLER(Cmd_SetMagicEffectEnchantShaderC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_TwoMagicEffects[2] =
{
	{	"magic effect",	kParamType_MagicEffect,	0	},
	{	"magic effect",	kParamType_MagicEffect,	0	},
};

CommandInfo kCommandInfo_AddMagicEffectCounter =
{
	"AddMagicEffectCounter",
	"AddMECounter",
	0,
	"adds an entry to the given magic effect's Counter list",
	0,
	2,
	kParams_TwoMagicEffects,
	HANDLER(Cmd_AddMagicEffectCounter_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_AddMagicEffectCounterC =
{
	"AddMagicEffectCounterC",
	"AddMECounterC",
	0,
	"adds an entry to the given magic effect's Counter list",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_AddMagicEffectCounterC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveNthMagicEffectCounter =
{
	"RemoveNthMagicEffectCounter",
	"RemoveNthMECounter",
	0,
	"removes an entry to the given magic effect's Counter list",
	0,
	2,
	kParams_OneInt_OneMagicEffect,
	HANDLER(Cmd_RemoveNthMagicEffectCounter_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RemoveNthMagicEffectCounterC =
{
	"RemoveNthMagicEffectCounterC",
	"RemoveNthMECounterC",
	0,
	"removes an entry to the given magic effect's Counter list",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_RemoveNthMagicEffectCounterC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneArray_OneMagicEffect[2] =
{
	{ "array",			kOBSEParamType_Array,	0 },
	{ "effect setting",	kOBSEParamType_Form,	0 },
};

CommandInfo kCommandInfo_SetMagicEffectCounters =
{
	"SetMagicEffectCounters",
	"SetMECounters",
	0,
	"takes an array and makes it the Counter Array of the given magic effect",
	0,
	2,
	kParams_OneArray_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectCounters_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneArray_OneInt[2] =
{
	{ "array",	kOBSEParamType_Array,	0 },
	{ "int",	kOBSEParamType_Number,	0 },
};

CommandInfo kCommandInfo_SetMagicEffectCountersC =
{
	"SetMagicEffectCountersC",
	"SetMECountersC",
	0,
	"takes an array and makes it the Counter Array of the given magic effect",
	0,
	2,
	kParams_OneArray_OneInt,
	HANDLER(Cmd_SetMagicEffectCountersC_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOtherActorValue =
{
	"SetMagicEffectOtherActorValue",
	"SetMEOtherAV",
	0,
	"sets the other actor value of the given magic effect to the given actor value",
	0,
	2,
	kParams_OneActorValue_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectOtherActorValue_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectOtherActorValueC =
{
	"SetMagicEffectOtherActorValueC",
	"SetMEOtherAVC",
	0,
	"sets the other actor value of the given magic effect to the given actor value",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_SetMagicEffectOtherActorValueC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsedObject =
{
	"SetMagicEffectUsedObject",
	"SetMEUsedObject",
	0,
	"sets the item or creature associated with the given magic effect",
	0,
	2,
	kParams_OneInventoryObject_OneMagicEffect,
	HANDLER(Cmd_SetMagicEffectUsedObject_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetMagicEffectUsedObjectC =
{
	"SetMagicEffectUsedObjectC",
	"SetMEUsedObjectC",
	0,
	"sets the item or creature associated with the given magic effect",
	0,
	2,
	kParams_OneInventoryObject_OneInt,
	HANDLER(Cmd_SetMagicEffectUsedObjectC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_DumpMagicEffectUnknowns =
{
	"DumpMagicEffectUnknowns",
	"DumpMEUnknowns",
	0,
	"dumps the unknown flags of a given magic effec to obse.log",
	0,
	1,
	kParams_OneMagicEffect,
	HANDLER(Cmd_DumpMagicEffectUnknowns_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(MagicEffectFromCode, returns an effect setting given its numeric code, 0, 1, kParams_OneInt);
DEFINE_COMMAND(MagicEffectFromChars, returns an effect setting given its 4-letter code, 0, 1, kParams_OneString);

DEFINE_COMMAND(GetMagicEffectCharsC, returns the 4-character code for a magic effect, 0, 1, kParams_OneInt);
DEFINE_COMMAND(GetMagicEffectChars, returns the 4-character code for a magic effect, 0, 1, kParams_OneMagicEffect);

CommandInfo kCommandInfo_MagicEffectCodeFromChars =
{
	"MagicEffectCodeFromChars", "MECodeFromChars", 0,
	"returns an effect code for a magic effect given its 4-letter code",
	0, 1, kParams_OneString,
	HANDLER(Cmd_MagicEffectCodeFromChars_Execute), Cmd_Default_Parse, NULL, 0
};