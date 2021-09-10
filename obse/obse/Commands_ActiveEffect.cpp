#include "obse/Commands_ActiveEffect.h"

#include "Utilities.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION
#include "GameObjects.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameMagicEffects.h"

// testing stuff
class AEDumper
{
public:
	bool Accept(ActiveEffect* ae)
	{
		_MESSAGE("***************");
		DumpClass(ae, sizeof(SummonCreatureEffect)/4);
		return true;
	}
};

static bool Cmd_DumpAE_Execute(COMMAND_ARGS)
{
	if (!thisObj)
		return true;

	MagicTarget* target = thisObj->GetMagicTarget();
	if (!target)
		return true;

	ActiveEffectVisitor visitor(target->GetEffectList());
	AEDumper dumper;
	visitor.Visit(dumper);
	return true;
}

enum {
	kAE_EffectCode = 0,
	kAE_Duration,
	kAE_TimeElapsed,
	kAE_MagicItem,
	kAE_Caster,
	kAE_Data,
	kAE_Magnitude,
	kAE_MagicItemIndex,
	kAE_Object,
	kAE_SummonRef,
	kAE_BoundItem,
	kAE_Applied,
	kAE_ActorValue
};

class EffectItemMatcher 
{
	EffectItem* m_pEI;
	UInt32 m_index;
	bool m_bFound;
public:
	EffectItemMatcher(EffectItem* effectItem) : m_pEI(effectItem), m_index(0), m_bFound(false) { }
	
	bool Accept(EffectItem* ei) {
		if (ei != m_pEI) {
			++m_index;
			return true;
		} else {
			m_bFound = true;
			return false;
		}
	}

	bool Found() const { return m_bFound; }
	UInt32 Index() const { return m_index; }
};


static bool GetActiveEffectInfo(ActiveEffect* ae, UInt32 whichVal, double* result)
{
	*result = 0;
	UInt32* refResult = (UInt32 *)result;
	

	if (!ae) return true;

	switch(whichVal) {
		case kAE_Object:
			if (ae->enchantObject)
				*refResult = ae->enchantObject->refID;
			break;
		case kAE_EffectCode:
			{
				*result = (SInt32)(ae->effectItem->effectCode);
				break;
			}
		case kAE_Duration:
			{
				*result = ae->duration;
				break;
			}
		case kAE_TimeElapsed:
			{
				*result = ae->timeElapsed;
				break;
			}

		case kAE_MagicItem:
			{
				TESForm* form = (TESForm*)Oblivion_DynamicCast(ae->item, 0, RTTI_MagicItem, RTTI_TESForm, 0);
				if (form) {
					*refResult = form->refID;
				}	
				break;
			}

		case kAE_Caster:
			{
				TESObjectREFR* caster = (TESObjectREFR *) Oblivion_DynamicCast(ae->caster, 0, RTTI_MagicCaster, RTTI_TESObjectREFR, 0);
				if (caster) {
					*refResult = caster->refID;
				}
				break;
			}

		case kAE_Data:
			{
				AssociatedItemEffect* assEff = OBLIVION_CAST(ae, ActiveEffect, AssociatedItemEffect);
				if (assEff && assEff->item)
					*refResult = assEff->item->refID;
				else if (ae->data)
					*refResult = ae->data->refID;
				else
					*refResult = 0;
				break;
			}
		case kAE_BoundItem:
			{
				AssociatedItemEffect* assEff = OBLIVION_CAST(ae, ActiveEffect, AssociatedItemEffect);
				if (assEff && assEff->item && assEff->IsBoundItemEffect())
					*refResult = assEff->item->refID;
				break;
			}
		case kAE_SummonRef:
			{
				SummonCreatureEffect* sumEff = OBLIVION_CAST(ae, ActiveEffect, SummonCreatureEffect);
				if (sumEff && sumEff->actor)
					*refResult = sumEff->actor->refID;
				break;
			}
		case kAE_Magnitude:
			{
				*result = ae->magnitude;
				break;
			}

		case kAE_MagicItemIndex:
			{
				EffectItemVisitor visitor(&(ae->item->list.effectList));
				EffectItemMatcher matcher(ae->effectItem);
				visitor.Visit(matcher);
				*result = (matcher.Found()) ? matcher.Index() : 255;
				break;
			}
		case kAE_Applied:
			{
				*result = ae->IsApplied() ? 1.0 : 0.0;
				break;
			}
		case kAE_ActorValue:
			{
				ValueModifierEffect* avEff = OBLIVION_CAST(ae, ActiveEffect, ValueModifierEffect);
				*result = avEff ? avEff->actorVal : -1.0;
				break;
			}
		
		default:
			break;
	}
	return true;
}

static bool GetNthActiveEffectInfo_Execute(COMMAND_ARGS, UInt32 whichVal) 
{
	*result = 0;
	if (!thisObj) return true;
	MagicTarget * magicTarget = thisObj->GetMagicTarget();
	if (!magicTarget) return true;

	UInt32 whichEffect = 0;
	ExtractArgs(PASS_EXTRACT_ARGS, &whichEffect);

	ActiveEffectVisitor visitor(magicTarget->GetEffectList());
	ActiveEffect* ae = visitor.GetNthInfo(whichEffect);
	if (!ae) return true;

	return GetActiveEffectInfo(ae, whichVal, result);
}

static bool Cmd_GetActiveEffectCount_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (!thisObj) return true;
	MagicTarget * magicTarget = thisObj->GetMagicTarget();
	if (!magicTarget) return true;

	ActiveEffectVisitor visitor(magicTarget->GetEffectList());
	*result = visitor.Count();
	return true;
}

static bool Cmd_GetNthActiveEffectCode_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_EffectCode);
}

static bool Cmd_GetNthActiveEffectDuration_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_Duration);
}

static bool Cmd_GetNthActiveEffectTimeElapsed_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_TimeElapsed);
}

static bool Cmd_GetNthActiveEffectMagicItem_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_MagicItem);
}

static bool Cmd_GetNthActiveEffectCaster_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_Caster);
}

static bool Cmd_GetNthActiveEffectData_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_Data);
}

static bool Cmd_GetNthActiveEffectBoundItem_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_BoundItem);
}

static bool Cmd_GetNthActiveEffectSummonRef_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_SummonRef);
}

static bool Cmd_GetNthActiveEffectMagnitude_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_Magnitude);
}

static bool Cmd_GetNthActiveEffectMagicItemIndex_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_MagicItemIndex);
}

static bool Cmd_IsNthActiveEffectApplied_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_Applied);
}

const bool bForModT = true;
const bool bForModF = false;

static bool ChangeNthActiveEffectValue_Execute(COMMAND_ARGS, UInt32 whichVal, bool bForMod)
{
	*result = 0;
	if (!thisObj) return true;
	MagicTarget * magicTarget = thisObj->GetMagicTarget();
	if (!magicTarget) return true;

	float floatVal = 0.0;
	UInt32 whichEffect = 0;
	ExtractArgs(PASS_EXTRACT_ARGS, &floatVal, &whichEffect);

	ActiveEffectVisitor visitor(magicTarget->GetEffectList());
	ActiveEffect* ae = visitor.GetNthInfo(whichEffect);
	if (!ae) return true;

	if (whichVal == kAE_Magnitude) {
#if 0
		float oldValue = ae->magnitude;
		float nuValue = (bForMod) ? oldValue += floatVal : floatVal;
		float change = (nuValue - oldValue);
		ae->magnitude = nuValue;
		if (thisObj->IsActor() && ae->effectItem) {
			UInt32 av = ae->effectItem->GetActorValue();
			if (av != 256) {
				Actor* actor = (Actor *)thisObj;
				actor->ModActorValue(av, change, 0);
			}
		}
#endif
		if (bForMod) ae->magnitude+= floatVal;
		else ae->magnitude = floatVal;
	}
	return true;
}

static bool Cmd_SetNthActiveEffectMagnitude_Execute(COMMAND_ARGS)
{
	return ChangeNthActiveEffectValue_Execute(PASS_COMMAND_ARGS, kAE_Magnitude, bForModF);
}

static bool Cmd_ModNthActiveEffectMagnitude_Execute(COMMAND_ARGS)
{
	return ChangeNthActiveEffectValue_Execute(PASS_COMMAND_ARGS, kAE_Magnitude, bForModT);
}

class AEMagnitudeCounter
{
	UInt32 m_effectCode;
	UInt32 m_actorVal;
	float m_magnitude;
	UInt8 m_majorType;
	UInt8 m_minorType;

	enum EFilter{
		eAllowAll = 0,
		eAllowAllButAbilities = 1,
		eAllowOnly = 2,
	};

	UInt8 m_filter;
	bool m_bAppliedEffectsOnly;

	bool MatchesExactly(MagicItem* magicItem) const {
		switch(m_majorType) {
			case MagicItem::kType_Spell:
				{
					SpellItem* spell = (SpellItem*)Oblivion_DynamicCast(magicItem, 0, RTTI_MagicItem, RTTI_SpellItem, 0);
					if (!spell) return false;
					if (m_minorType == SpellItem::kType_All) return true;
					return (spell->spellType == m_minorType);
				}
			default:
				return magicItem->Type() == m_majorType;
		}
	}

public:
	AEMagnitudeCounter(UInt8 majorType = 0, UInt8 minorType = 0) : m_majorType(majorType), m_minorType(minorType), m_magnitude(0), m_effectCode(0), m_actorVal(kActorVal_OblivionMax), m_bAppliedEffectsOnly(false)
	{
		if (m_majorType == 0) {
			m_filter = (m_minorType == SpellItem::kType_Ability) ? eAllowAllButAbilities : eAllowAll;
		} else {
			m_filter = eAllowOnly;
		}
	}

	void SetEffectCode(UInt32 effectCode) { m_effectCode = effectCode; }
	void SetActorVal(UInt32 actorVal) { m_actorVal = actorVal; }
	void SetIgnoreUnappliedEffects(bool bIgnore) { m_bAppliedEffectsOnly = bIgnore; }

	bool Accept(ActiveEffect* ae) {
		if (ae) {
			if (m_bAppliedEffectsOnly && !ae->IsApplied()) return true;
			if (ae->effectItem->effectCode != m_effectCode) return true;
			if (m_actorVal != kActorVal_OblivionMax && ae->effectItem->actorValueOrOther != m_actorVal) return true;
			bool bMatches = false;
			MagicItem* magicItem = ae->item;
			switch(m_filter) {
				case eAllowAll:  
					{
						bMatches = true; 
						break;
					}
				case eAllowAllButAbilities:
					{
						SpellItem* spell = (SpellItem*)Oblivion_DynamicCast(magicItem, 0, RTTI_MagicItem, RTTI_SpellItem, 0);
						bMatches = (spell && spell->spellType == SpellItem::kType_Ability) ? false : true;
						break;
					}
				case eAllowOnly:
					{
						bMatches = MatchesExactly(magicItem);
					}
			}
			if (bMatches) {
				m_magnitude += ae->magnitude;
			}
		}
		return true;
	}

	float Magnitude() const {
		return m_magnitude;
	}
};

static bool GetTotalActiveEffectMagnitude(MagicTarget* magicTarget, AEMagnitudeCounter& counter, UInt32 effectCode, UInt32 actorVal, double* result)
{
	if (!magicTarget) return true;
	counter.SetEffectCode(effectCode);
	if (actorVal != kActorVal_OblivionMax) {
		counter.SetActorVal(actorVal);
	}
	ActiveEffectVisitor visitor(magicTarget->GetEffectList());
	visitor.Visit(counter);
	*result = counter.Magnitude();
	return true;
}

static bool GetTotalActiveEffectMagnitude_Execute(COMMAND_ARGS, AEMagnitudeCounter& counter)
{
	*result = 0;
	if (!thisObj) return true;

	EffectSetting* magicEffect = NULL;
	UInt32 actorVal = kActorVal_OblivionMax;
	UInt32 bIgnoreUnapplied = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &magicEffect, &actorVal, &bIgnoreUnapplied))
		return true;

	if (!magicEffect) return true;
	
	MagicTarget * magicTarget = thisObj->GetMagicTarget();
	if (!magicTarget) return true;

	if (bIgnoreUnapplied)
		counter.SetIgnoreUnappliedEffects(true);

	return GetTotalActiveEffectMagnitude(magicTarget, counter, magicEffect->effectCode, actorVal, result);
}

static bool Cmd_GetTotalActiveEffectMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter;
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAENonAbilityMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(0, SpellItem::kType_Ability);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEAbilityMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Ability);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAESpellMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Spell);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEDiseaseMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Disease);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAELesserPowerMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_LesserPower);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEPowerMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Power);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEAllSpellsMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_All);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEEnchantmentMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Enchantment);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEAlchemyMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Alchemy);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEIngredientMagnitude_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Ingredient);
	return GetTotalActiveEffectMagnitude_Execute(PASS_COMMAND_ARGS, counter);
}

static bool GetTotalActiveEffectMagnitudeC_Execute(COMMAND_ARGS, AEMagnitudeCounter& counter)
{
	*result = 0;
	if (!thisObj) return true;

	UInt32 effectCode = 0;
	UInt32 actorVal = kActorVal_OblivionMax;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &effectCode, &actorVal))
		return true;

	if (effectCode == 0)
		return true;
	
	MagicTarget * magicTarget = thisObj->GetMagicTarget();
	if (!magicTarget) return true;

	return GetTotalActiveEffectMagnitude(magicTarget, counter, effectCode, actorVal, result);
}

static bool Cmd_GetTotalActiveEffectMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter;
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAENonAbilityMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(0, SpellItem::kType_Ability);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEAbilityMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Ability);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAESpellMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Spell);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEDiseaseMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Disease);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAELesserPowerMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_LesserPower);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEPowerMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_Power);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEAllSpellsMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Spell, SpellItem::kType_All);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEEnchantmentMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Enchantment);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEAlchemyMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Alchemy);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}

static bool Cmd_GetTotalAEIngredientMagnitudeC_Execute(COMMAND_ARGS)
{
	AEMagnitudeCounter counter(MagicItem::kType_Ingredient);
	return GetTotalActiveEffectMagnitudeC_Execute(PASS_COMMAND_ARGS, counter);
}




class ScriptEffectAEFinder
{
	Script* m_pScript;
	ActiveEffect* m_pAE;
	UInt32 m_index;
public:
	ScriptEffectAEFinder(Script* pScript) : m_pScript(pScript), m_pAE(NULL), m_index(0) {}

	bool Accept(ActiveEffect* ae) {
		if (ae && ae->effectItem->IsScriptedEffect() && ae->effectItem->scriptEffectInfo->scriptRefID == m_pScript->refID) {
			m_pAE = ae;
			// stop if we find our match
			return false;
		} else {
			++m_index;
			return true;
		}
	}

	ActiveEffect* Found() const { return m_pAE; }
	UInt32 Index() const { return (m_pAE) ? m_index : -1; }
};

static bool Cmd_GetScriptActiveEffectIndex_Execute(COMMAND_ARGS)
{
	// default return value changed from 0 to -1 in OBSE 0019
	// otherwise no means to distinguish 'not found' or general failure from AE index 0
    *result = -1.0;

	if (thisObj && eventList) {
		MagicTarget * magicTarget = thisObj->GetMagicTarget();
		if (magicTarget)  {
			UInt32 idx = 0;
			for (MagicTarget::EffectNode* node = magicTarget->GetEffectList(); node; node = node->next)
			{
				ScriptEffect* effect = (ScriptEffect*)Oblivion_DynamicCast(node->data, 0, RTTI_ActiveEffect, RTTI_ScriptEffect,0);
				if (effect && effect->eventList == eventList)
				{
					*result = idx;
					break;
				}
				idx++;
			}
		}
	}

	return true;
}

static bool Cmd_DispelNthActiveEffect_Execute(COMMAND_ARGS)
{
	// sets time elapsed equal to duration, to force effect to be removed next frame
	// returns 1 on success, 0 if not dispelled
	// ###TODO: doesn't dispel abilities, some (?) bound item effects. works on summons

	*result = 0;
	if (!thisObj) 
		return true;

	MagicTarget * magicTarget = thisObj->GetMagicTarget();
	if (!magicTarget) 
		return true;

	UInt32 whichEffect = 0;
	ExtractArgs(PASS_EXTRACT_ARGS, &whichEffect);

	ActiveEffectVisitor visitor(magicTarget->GetEffectList());
	ActiveEffect* ae = visitor.GetNthInfo(whichEffect);
	if (ae)
	{
		ae->Remove(false);
		*result = 1;
	}

	return true;
}

enum eAEDataType
{
	kAEData_EffectCode,
	kAEData_Caster
};

static bool GetActiveEffectArray_Execute(COMMAND_ARGS, eAEDataType type)
{
	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	if (thisObj)
	{
		MagicTarget* target = thisObj->GetMagicTarget();
		if (target)
		{
			UInt32 idx = 0;
			for (MagicTarget::EffectNode* cur = target->GetEffectList(); cur && cur->data; cur = cur->next)
			{
				switch (type)
				{
				case kAEData_EffectCode:
					g_ArrayMap.SetElementNumber(arrID, ArrayKey(idx), cur->data->effectItem ? cur->data->effectItem->effectCode : 0);
					break;
				case kAEData_Caster:
					{
						TESObjectREFR* caster = OBLIVION_CAST(cur->data->caster, MagicCaster, TESObjectREFR);
						UInt32 casterID = caster ? caster->refID : 0;
						g_ArrayMap.SetElementFormID(arrID, ArrayKey(idx), casterID);
						break;
					}
				}

				idx++;
			}
		}
	}

	return true;
}

static bool Cmd_GetActiveEffectCodes_Execute(COMMAND_ARGS)
{
	return GetActiveEffectArray_Execute(PASS_COMMAND_ARGS, kAEData_EffectCode);
}

static bool Cmd_GetActiveEffectCasters_Execute(COMMAND_ARGS)
{
	return GetActiveEffectArray_Execute(PASS_COMMAND_ARGS, kAEData_Caster);
}

class ActiveEffectCodeMatcher
{
	UInt32 m_effectCode;
	const ActiveEffect* m_foundAE;
public:
	ActiveEffectCodeMatcher(UInt32 effCode) : m_effectCode(effCode), m_foundAE(NULL)
	{	}

	bool Accept (const ActiveEffect* ae)
	{
		if (ae->effectItem && ae->effectItem->effectCode == m_effectCode)
		{
			m_foundAE = ae;
			return false;
		}

		return true;
	}

	const ActiveEffect* FoundAE() { return m_foundAE; }
};

static bool Cmd_GetTelekinesisRef_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MagicTarget* target = (*g_thePlayer)->GetMagicTarget();
	if (target)
	{
		MagicTarget::EffectNode* effList = target->GetEffectList();
		if (effList)
		{
			ActiveEffectVisitor visitor(effList);
			ActiveEffectCodeMatcher matcher(MACRO_SWAP32('TELE'));
			visitor.Visit(matcher);
			if (matcher.FoundAE())
			{
				TelekinesisEffect* tele = OBLIVION_CAST(matcher.FoundAE(), ActiveEffect, TelekinesisEffect);
				if (tele && tele->target)
					*refResult = tele->target->refID;
			}
		}
	}

	if (IsConsoleMode())
		Console_Print("GetTelekinesisRef >> %08x", *refResult);

	return true;
}

static bool Cmd_GetNthActiveEffectEnchantObject_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_Object);
}

static bool Cmd_GetNthActiveEffectActorValue_Execute(COMMAND_ARGS)
{
	return GetNthActiveEffectInfo_Execute(PASS_COMMAND_ARGS, kAE_ActorValue);
}

#endif


CommandInfo kCommandInfo_GetActiveEffectCount =
{
	"GetActiveEffectCount",
	"GetAECount",
	0,
	"returns the number of active effects on the reference",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetActiveEffectCount_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectCode = 
{
	"GetNthActiveEffectCode",
	"GetNthAECode",
	0,
	"returns the effect code of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectCode_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectMagnitude =
{
	"GetNthActiveEffectMagnitude",
	"GetNthAEMagnitude",
	0,
	"returns the magnitude of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectDuration =
{
	"GetNthActiveEffectDuration",
	"GetNthAEDuration",
	0,
	"returns the duration of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectDuration_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectTimeElapsed =
{
	"GetNthActiveEffectTimeElapsed",
	"GetNthAETime",
	0,
	"returns the time elapsed of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectTimeElapsed_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectMagicItem =
{
	"GetNthActiveEffectMagicItem",
	"GetNthAEMagicItem",
	0,
	"returns the magic item of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectMagicItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectCaster =
{
	"GetNthActiveEffectCaster",
	"GetNthAECaster",
	0,
	"returns the caster of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectCaster_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectData =
{
	"GetNthActiveEffectData",
	"GetNthAEData",
	0,
	"returns the data of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectData_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectBoundItem =
{
	"GetNthActiveEffectBoundItem",
	"GetNthAEBoundItem",
	0,
	"returns the bound item of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectBoundItem_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectSummonRef =
{
	"GetNthActiveEffectSummonRef",
	"GetNthAESummonRef",
	0,
	"returns a reference to the summoned creature associated with the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectSummonRef_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthActiveEffectMagicItemIndex =
{
	"GetNthActiveEffectMagicItemIndex",
	"GetNthAEIndex",
	0,
	"returns the index in the MagicItem of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectMagicItemIndex_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_CMD_ALT(IsNthActiveEffectApplied, IsNthAEApplied, returns 1 if the active effect has been applied to the target,
			   1, kParams_OneInt);

static ParamInfo kParams_SetNthAEFloat[2] =
{
	{	"value", kParamType_Float, 0 },
	{	"which", kParamType_Integer, 0 },
};

CommandInfo kCommandInfo_SetNthActiveEffectMagnitude =
{
	"SetNthActiveEffectMagnitude",
	"SetNthAEMagnitude",
	0,
	"sets the magnitude of the Nth active effect",
	1,
	2,
	kParams_SetNthAEFloat,
	HANDLER(Cmd_SetNthActiveEffectMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ModNthActiveEffectMagnitude =
{
	"ModNthActiveEffectMagnitude",
	"ModNthAEMagnitude",
	0,
	"mods the magnitude of the Nth active effect",
	1,
	2,
	kParams_SetNthAEFloat,
	HANDLER(Cmd_ModNthActiveEffectMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetTotalAE[3] =
{
	{	"magic effect", kParamType_MagicEffect, 0 },
	{	"actor value", kParamType_ActorValue, 1},
	{	"bIgnoreUnappliedEffects", kParamType_Integer, 1},
};


CommandInfo kCommandInfo_GetTotalActiveEffectMagnitude =
{
	"GetTotalActiveEffectMagnitude",
	"GetTotalAEMagnitude",
	0,
	"returns the magnitude of the all active effect",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalActiveEffectMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAENonAbilityMagnitude =
{
	"GetTotalAENonAbilityMagnitude",
	"",
	0,
	"returns the magnitude of the all active effects except abilities",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAENonAbilityMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEAbilityMagnitude =
{
	"GetTotalAEAbilityMagnitude",
	"",
	0,
	"returns the magnitude of the all active abilities",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEAbilityMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAESpellMagnitude =
{
	"GetTotalAESpellMagnitude",
	"",
	0,
	"returns the magnitude of the all active spells",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAESpellMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEDiseaseMagnitude =
{
	"GetTotalAEDiseaseMagnitude",
	"",
	0,
	"returns the magnitude of the all active diseases",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEDiseaseMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAELesserPowerMagnitude =
{
	"GetTotalAELesserPowerMagnitude",
	"",
	0,
	"returns the magnitude of the all active lesser powers",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAELesserPowerMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEPowerMagnitude =
{
	"GetTotalAEPowerMagnitude",
	"",
	0,
	"returns the magnitude of the all active greater powers",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEPowerMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEAllSpellsMagnitude =
{
	"GetTotalAEAllSpellsMagnitude",
	"",
	0,
	"returns the magnitude of the all active spells",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEAllSpellsMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEEnchantmentMagnitude =
{
	"GetTotalAEEnchantmentMagnitude",
	"",
	0,
	"returns the magnitude of the all active enchantments",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEEnchantmentMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


CommandInfo kCommandInfo_GetTotalAEAlchemyMagnitude =
{
	"GetTotalAEAlchemyMagnitude",
	"",
	0,
	"returns the magnitude of the all active alchemy items",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEAlchemyMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEIngredientMagnitude =
{
	"GetTotalAEIngredientMagnitude",
	"",
	0,
	"returns the magnitude of the all active ingredients",
	1,
	3,
	kParams_GetTotalAE,
	HANDLER(Cmd_GetTotalAEIngredientMagnitude_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_GetTotalAEC[3] =
{
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 1 },
	{	"bIgnoreUnappliedEffects", kParamType_Integer, 1},
};

CommandInfo kCommandInfo_GetTotalActiveEffectMagnitudeC =
{
	"GetTotalActiveEffectMagnitudeC",
	"GetTotalAEMagnitudeC",
	0,
	"returns the magnitude of the all active effect",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalActiveEffectMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAENonAbilityMagnitudeC =
{
	"GetTotalAENonAbilityMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active effects except abilities",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAENonAbilityMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEAbilityMagnitudeC =
{
	"GetTotalAEAbilityMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active abilities",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEAbilityMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAESpellMagnitudeC =
{
	"GetTotalAESpellMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active spells",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAESpellMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEDiseaseMagnitudeC =
{
	"GetTotalAEDiseaseMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active diseases",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEDiseaseMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAELesserPowerMagnitudeC =
{
	"GetTotalAELesserPowerMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active lesser powers",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAELesserPowerMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEPowerMagnitudeC =
{
	"GetTotalAEPowerMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active greater powers",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEPowerMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEAllSpellsMagnitudeC =
{
	"GetTotalAEAllSpellsMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active spells",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEAllSpellsMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEEnchantmentMagnitudeC =
{
	"GetTotalAEEnchantmentMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active enchantments",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEEnchantmentMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


CommandInfo kCommandInfo_GetTotalAEAlchemyMagnitudeC =
{
	"GetTotalAEAlchemyMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active alchemy items",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEAlchemyMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTotalAEIngredientMagnitudeC =
{
	"GetTotalAEIngredientMagnitudeC",
	"",
	0,
	"returns the magnitude of the all active ingredients",
	1,
	3,
	kParams_GetTotalAEC,
	HANDLER(Cmd_GetTotalAEIngredientMagnitudeC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetScriptActiveEffectIndex =
{
	"GetScriptActiveEffectIndex",
	"GetSAEIndex",
	0,
	"returns the ActiveEffect index of the running script effect",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetScriptActiveEffectIndex_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(DumpAE, nothing, 1, 0, NULL);

CommandInfo kCommandInfo_DispelNthActiveEffect = 
{
	"DispelNthActiveEffect",
	"DispelNthAE",
	0,
	"terminates the specified active effect on the calling actor",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_DispelNthActiveEffect_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetActiveEffectCodes, returns an array containing the codes of the actors active effects, 1, 0, NULL);
DEFINE_COMMAND(GetActiveEffectCasters, returns an array containing the casters of the actors active effects, 1, 0, NULL);
DEFINE_COMMAND(GetTelekinesisRef, returns the object currently targeted by telekinesis, 0, 0, NULL);

CommandInfo kCommandInfo_GetNthActiveEffectEnchantObject =
{
	"GetNthActiveEffectEnchantObject",
	"GetNthAEEnchantObject",
	0,
	"returns the enchanted object of the nth active effect",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthActiveEffectEnchantObject_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

DEFINE_CMD_ALT(GetNthActiveEffectActorValue, GetNthAEAV, returns the actor value associated with the active effect,
			   1, kParams_OneInt);