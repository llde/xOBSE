#include "Commands_AI.h"
#include "Script.h"
#include "ParamInfos.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameProcess.h"
#include "InternalSerialization.h"
#include "GameMagicEffects.h"

static bool Cmd_GetNumFollowers_Execute(COMMAND_ARGS)
{
	*result = 0;

	if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Follower);
	if (!xData)
		return true;

	ExtraFollower* xFollower = (ExtraFollower*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraFollower, 0);
	if (xFollower)
		*result = ExtraFollowerVisitor(xFollower->followers).Count();

	return true;
}

static bool Cmd_GetNthFollower_Execute(COMMAND_ARGS)
{
	UInt32 idx = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &idx))
		return true;

	else if (!thisObj)
		return true;

	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Follower);
	if (!xData)
		return true;

	ExtraFollower* xFollowers = (ExtraFollower*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraFollower, 0);
	if (xFollowers)
	{
		Character* follower = ExtraFollowerVisitor(xFollowers->followers).GetNthInfo(idx);
		if (follower)
			*refResult = follower->refID;
	}

	return true;
}

static bool Cmd_GetFollowers_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetFollowers cannot be called within a Set statement.");
		return true;
	}

	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	if (!thisObj)
		return true;

	// get normal followers
	BSExtraData* xData = thisObj->baseExtraList.GetByType(kExtraData_Follower);
	ExtraFollower* xFollowers = OBLIVION_CAST(xData, BSExtraData, ExtraFollower);
	UInt32 arrIdx = 0;
	if (xFollowers)
	{
		for (ExtraFollower::ListNode* cur = xFollowers->followers; cur; cur = cur->next)
		{
			if (!cur->character)
				continue;
			g_ArrayMap.SetElementFormID(arrID, arrIdx++, cur->character->refID);
		}
	}

	// get summons
	MagicTarget* target = thisObj->GetMagicTarget();
	if (target)
	{
		for (MagicTarget::EffectNode* node = target->GetEffectList(); node; node = node->next)
		{
			if (!node->data)
				continue;

			SummonCreatureEffect* summon = OBLIVION_CAST(node->data, ActiveEffect, SummonCreatureEffect);
			if (summon && summon->actor)
				g_ArrayMap.SetElementFormID(arrID, arrIdx++, summon->actor->refID);
		}
	}

	return true;
}

static bool Cmd_GetNumDetectedActors_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = NULL;
	Actor* actor = (Actor*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_Actor, 0);
	if (actor)
		hiProc = (HighProcess*)Oblivion_DynamicCast(actor->process, 0, RTTI_BaseProcess, RTTI_HighProcess, 0);

	if (hiProc && hiProc->detectionList)
		*result = HighProcess::DetectionListVisitor(hiProc->detectionList).Count();

	if (IsConsoleMode())
		Console_Print("Num Detected Actors: %.0f", *result);

	return true;
}

static bool Cmd_GetNthDetectedActor_Execute(COMMAND_ARGS)
{
	UInt32 whichN = 0;
	HighProcess* hiProc = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &whichN))
		return true;

	Actor* actor = (Actor*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_Actor, 0);
	if (actor)
		hiProc = (HighProcess*)Oblivion_DynamicCast(actor->process, 0, RTTI_BaseProcess, RTTI_HighProcess, 0);

	if (hiProc && hiProc->detectionList)
	{
		HighProcess::DetectionList::Data* data = HighProcess::DetectionListVisitor(hiProc->detectionList).GetNthInfo(whichN);
		if (data && data->actor)
		{
			*refResult = data->actor->refID;
			if (IsConsoleMode())
				PrintItemType(data->actor->baseForm);
		}
	}

	return true;
}

class DetectedActorFinder
{
public:
	Actor* m_actorToFind;
	DetectedActorFinder(Actor* actor) : m_actorToFind(actor)
		{ }
	bool Accept(HighProcess::DetectionList::Data* data)
	{
		if (data->actor && data->actor->refID == m_actorToFind->refID)
			return true;

		return false;
	}
};

static bool Cmd_SetDetectionState_Execute(COMMAND_ARGS)
{
	HighProcess* hiProc = NULL;
	Actor* actor = NULL;
	UInt32 detectLevel = 0;
	*result = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &actor, &detectLevel))
		return true;

	Actor* callingActor = (Actor*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_Actor, 0);
	if (callingActor)
		hiProc = (HighProcess*)Oblivion_DynamicCast(callingActor->process, 0, RTTI_BaseProcess, RTTI_HighProcess, 0);

	if (hiProc && hiProc->detectionList && detectLevel < HighProcess::kDetectionState_Max)
	{
		const HighProcess::DetectionList* dList = HighProcess::DetectionListVisitor(hiProc->detectionList).Find(DetectedActorFinder(actor));
		if (dList)
			dList->data->detectionState = detectLevel;
	}

	return true;
}

static bool GetServiceFlag_Execute(COMMAND_ARGS, UInt32 whichService)
{
	TESNPC* npc = NULL;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &npc))
		return false;
	if (!npc && thisObj)
		npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (npc)
		return npc->aiForm.OffersServices(whichService);
	else
		return false;
}

static bool Cmd_OffersWeapons_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Weapons) ? 1 : 0;
	return true;
}

static bool Cmd_OffersArmor_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Armor) ? 1 : 0;
	return true;
}

static bool Cmd_OffersBooks_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Books) ? 1 : 0;
	return true;
}

static bool Cmd_OffersClothing_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Clothing) ? 1 : 0;
	return true;
}

static bool Cmd_OffersIngredients_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Ingredients) ? 1 : 0;
	return true;
}

static bool Cmd_OffersLights_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Lights) ? 1 : 0;
	return true;
}

static bool Cmd_OffersApparatus_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Apparatus) ? 1 : 0;
	return true;
}

static bool Cmd_OffersMiscItems_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Misc) ? 1 : 0;
	return true;
}

static bool Cmd_OffersSpells_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Spells) ? 1 : 0;
	return true;
}

static bool Cmd_OffersMagicItems_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_MagicItems) ? 1 : 0;
	return true;
}

static bool Cmd_OffersPotions_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Potions) ? 1 : 0;
	return true;
}

static bool Cmd_OffersTraining_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Training) ? 1 : 0;
	return true;
}

static bool Cmd_OffersRecharging_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Recharge) ? 1 : 0;
	return true;
}

static bool Cmd_OffersRepair_Execute(COMMAND_ARGS)
{
	*result = GetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Repair) ? 1 : 0;
	return true;
}

static bool Cmd_GetTrainerSkill_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &npc))
		return true;
	if (!npc && thisObj)
		npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (npc && npc->aiForm.OffersServices(TESAIForm::kService_Training))
		*result = npc->aiForm.trainingSkill + kActorVal_Armorer;

	return true;
}

static bool Cmd_GetTrainerLevel_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &npc))
		return true;
	if (!npc && thisObj)
		npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (npc && npc->aiForm.OffersServices(TESAIForm::kService_Training))
		*result = npc->aiForm.trainingLevel;

	return true;
}

static bool Cmd_OffersServicesC_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	UInt32 serviceMask = 0;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &serviceMask, &npc))
		return true;

	if (!npc && thisObj)
		npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);

	if (serviceMask && npc && npc->aiForm.OffersServices(serviceMask))
		*result = 1;
	else
		*result = 0;

	return true;
}

static void SetServiceFlag_Execute(COMMAND_ARGS, UInt32 whichService)
{
	TESNPC* npc = NULL;
	UInt32 bSetFlag = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &bSetFlag, &npc))
	{
		if (!npc && thisObj)
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);

		if (npc)
		{
			npc->aiForm.SetOffersServices(whichService, bSetFlag ? true : false);
		}
	}
}

static bool Cmd_SetOffersWeapons_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Weapons);
	return true;
}

static bool Cmd_SetOffersArmor_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Armor);
	return true;
}

static bool Cmd_SetOffersClothing_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Clothing);
	return true;
}

static bool Cmd_SetOffersBooks_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Books);
	return true;
}

static bool Cmd_SetOffersIngredients_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Ingredients);
	return true;
}

static bool Cmd_SetOffersLights_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Lights);
	return true;
}

static bool Cmd_SetOffersApparatus_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Apparatus);
	return true;
}

static bool Cmd_SetOffersMiscItems_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Misc);
	return true;
}

static bool Cmd_SetOffersPotions_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Potions);
	return true;
}

static bool Cmd_SetOffersSpells_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Spells);
	return true;
}

static bool Cmd_SetOffersMagicItems_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_MagicItems);
	return true;
}

static bool Cmd_SetOffersTraining_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Training);
	return true;
}

static bool Cmd_SetOffersRecharging_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Recharge);
	return true;
}

static bool Cmd_SetOffersRepair_Execute(COMMAND_ARGS)
{
	SetServiceFlag_Execute(PASS_COMMAND_ARGS, TESAIForm::kService_Repair);
	return true;
}

static bool Cmd_GetServicesMask_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &npc))
	{
		if (!npc && thisObj)
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (npc)
			*result = npc->aiForm.serviceFlags;
	}
	return true;
}

static bool Cmd_SetServicesMask_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	UInt32 serviceMask = 0;

	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &serviceMask, &npc))
	{
		if (!npc && thisObj)
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (npc)
		{
			npc->aiForm.SetOffersServices(serviceMask, true, true);
		}
	}
	return true;
}

static bool Cmd_SetTrainerSkill_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	UInt32 actorVal = 0;

	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorVal, &npc))
	{
		if (!npc && thisObj)
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (npc && actorVal >= kActorVal_Armorer && actorVal <= kActorVal_Speechcraft)
		{
			npc->aiForm.trainingSkill = actorVal - kActorVal_Armorer;
		}
	}
	return true;
}

static bool Cmd_SetTrainerLevel_Execute(COMMAND_ARGS)
{
	TESNPC* npc = NULL;
	UInt32 nuLevel = 0;

	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &nuLevel, &npc))
	{
		if (!npc && thisObj)
			npc = (TESNPC*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESNPC, 0);
		if (npc)
		{
			npc->aiForm.trainingLevel = nuLevel;
		}
	}
	return true;
}

static bool Cmd_GetNumPackages_Execute(COMMAND_ARGS)
{
	TESActorBase* actorBase = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorBase))
	{
		if (!actorBase && thisObj)
			actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
		if (actorBase)
			*result = PackageListVisitor(&actorBase->aiForm.packageList).Count();
	}
	return true;
}

static bool Cmd_GetNthPackage_Execute(COMMAND_ARGS)
{
	TESActorBase* actorBase = NULL;
	UInt32 whichPackage = 0;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &whichPackage, &actorBase))
	{
		if (!actorBase && thisObj)
			actorBase = (TESActorBase*)Oblivion_DynamicCast(thisObj->baseForm, 0, RTTI_TESForm, RTTI_TESActorBase, 0);
		if (actorBase)
		{
			TESPackage* pkg = PackageListVisitor(&actorBase->aiForm.packageList).GetNthInfo(whichPackage);
			if (pkg)
				*refResult = pkg->refID;
		}
	}
	return true;
}

static HighProcess* ExtractHighProcess(TESObjectREFR* thisObj)
{
	HighProcess* hiProc = NULL;
	MobileObject* mob = (MobileObject*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MobileObject, 0);
	if (mob)
		hiProc = (HighProcess*)Oblivion_DynamicCast(mob->process, 0, RTTI_BaseProcess, RTTI_HighProcess, 0);

	return hiProc;
}

static bool Cmd_IsAttacking_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
		*result = hiProc->IsAttacking() ? 1 : 0;

	return true;
}

static bool Cmd_IsBlocking_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
		*result = hiProc->IsBlocking() ? 1 : 0;

	return true;
}

static bool Cmd_IsRecoiling_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
		*result = hiProc->IsRecoiling() ? 1 : 0;

	return true;
}

static bool Cmd_IsDodging_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
		*result = hiProc->IsDodging() ? 1 : 0;

	return true;
}

static bool Cmd_IsStaggered_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
		*result = hiProc->IsStaggered() ? 1 : 0;

	return true;
}

static bool IsMovementFlagSet_Execute(TESObjectREFR* thisObj, UInt32 flag)
{
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc && hiProc->IsMovementFlagSet(flag))
		return true;
	else
		return false;
}

static bool Cmd_IsMovingForward_Execute(COMMAND_ARGS)
{
	*result = IsMovementFlagSet_Execute(thisObj, HighProcess::kMovement_Forward) ? 1 : 0;
	return true;
}

static bool Cmd_IsMovingBackward_Execute(COMMAND_ARGS)
{
	*result = IsMovementFlagSet_Execute(thisObj, HighProcess::kMovement_Backward) ? 1 : 0;
	return true;
}

static bool Cmd_IsMovingLeft_Execute(COMMAND_ARGS)
{
	*result = IsMovementFlagSet_Execute(thisObj, HighProcess::kMovement_Left) ? 1 : 0;
	return true;
}

static bool Cmd_IsMovingRight_Execute(COMMAND_ARGS)
{
	*result = IsMovementFlagSet_Execute(thisObj, HighProcess::kMovement_Right) ? 1 : 0;
	return true;
}

static bool Cmd_IsTurningLeft_Execute(COMMAND_ARGS)
{
	*result = IsMovementFlagSet_Execute(thisObj, HighProcess::kMovement_TurnLeft) ? 1 : 0;
	return true;
}

static bool Cmd_IsTurningRight_Execute(COMMAND_ARGS)
{
	*result = IsMovementFlagSet_Execute(thisObj, HighProcess::kMovement_TurnRight) ? 1 : 0;
	return true;
}

static bool Cmd_IsInAir_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
	{
		bhkCharacterController* ctrlr = hiProc->GetCharacterController();
		if (ctrlr && ctrlr->ctx.IsInAir())
			*result = 1;
	}

	return true;
}

static bool Cmd_IsJumping_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
	{
		bhkCharacterController* ctrlr = hiProc->GetCharacterController();
		if (ctrlr && ctrlr->ctx.IsJumping())
			*result = 1;
	}

	return true;
}

static bool Cmd_IsOnGround_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
	{
		bhkCharacterController* ctrlr = hiProc->GetCharacterController();
		if (ctrlr && ctrlr->ctx.IsOnGround())
			*result = 1;
	}

	return true;
}

static bool Cmd_IsFlying_Execute(COMMAND_ARGS)
{
	*result = 0;
	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
	{
		bhkCharacterController* ctrlr = hiProc->GetCharacterController();
		if (ctrlr && ctrlr->ctx.IsFlying())
			*result = 1;
	}

	return true;
}

static bool Cmd_GetFallTimer_Execute(COMMAND_ARGS)
{
	*result = 0;

	HighProcess* hiProc = ExtractHighProcess(thisObj);
	if (hiProc)
	{
		bhkCharacterController* ctrlr = hiProc->GetCharacterController();
		if (ctrlr)
			*result = ctrlr->fallDamageTimer;
	}

	return true;
}

static MiddleHighProcess* ExtractMiddleHighProcess(TESObjectREFR* thisObj)
{
	MiddleHighProcess* proc = NULL;
	MobileObject* mob = (MobileObject*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MobileObject, 0);
	if (mob)
		proc = (MiddleHighProcess*)Oblivion_DynamicCast(mob->process, 0, RTTI_BaseProcess, RTTI_MiddleHighProcess, 0);

	return proc;
}

ActorAnimData* GetActorAnimData(TESObjectREFR* callingObj)
{
	if (callingObj == *g_thePlayer && (*g_thePlayer)->isThirdPerson == 0)
		return (*g_thePlayer)->firstPersonAnimData;
	else
	{
		MiddleHighProcess* proc = ExtractMiddleHighProcess(callingObj);
		if (proc)
			return proc->animData;
	}

	return NULL;
}

static bool Cmd_IsPowerAttacking_Execute(COMMAND_ARGS)
{
	*result = 0;
	ActorAnimData* animData = GetActorAnimData(thisObj);
	if (animData)
	{
		if (animData->FindAnimInRange(TESAnimGroup::kAnimGroup_AttackPower, TESAnimGroup::kAnimGroup_AttackRightPower))
			*result = 1;
	}
	return true;
}

static bool Cmd_IsCasting_Execute(COMMAND_ARGS)
{
	*result = 0;
	ActorAnimData* animData = GetActorAnimData(thisObj);
	if (animData)
	{
		if (animData->FindAnimInRange(TESAnimGroup::kAnimGroup_CastSelf, TESAnimGroup::kAnimGroup_CastTargetAlt))
			*result = 1;
	}
	return true;
}

static bool Cmd_IsAnimGroupPlaying_Execute(COMMAND_ARGS)
{
	UInt32 whichGroup = -1;
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &whichGroup))
		return true;
	else if (whichGroup > TESAnimGroup::kAnimGroup_Max)
		return true;

	ActorAnimData* animData = GetActorAnimData(thisObj);
	if (animData)
		*result = (animData->FindAnimInRange(whichGroup)) ? 1 : 0;

	return true;
}

static bool Cmd_AnimPathIncludes_Execute(COMMAND_ARGS)
{
	char subStr[512] = { 0 };
	*result = 0;

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &subStr))
		return true;

	ActorAnimData* animData = GetActorAnimData(thisObj);
	if (animData)
		*result = (animData->PathsInclude(subStr)) ? 1 : 0;

	return true;
}

static bool Cmd_GetProcessLevel_Execute(COMMAND_ARGS)
{
	*result = -1;
	if (thisObj)
	{
		MobileObject* mob = (MobileObject*)Oblivion_DynamicCast(thisObj, 0, RTTI_TESObjectREFR, RTTI_MobileObject, 0);
		if (mob && mob->process)
			*result = mob->process->GetProcessLevel();
	}
	return true;
}

static bool Cmd_SetPackageTarget_Execute(COMMAND_ARGS)
{
	// SetPackageTarget package target [count]

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 1)
	{
		TESPackage* pack = OBLIVION_CAST(eval.Arg(0)->GetTESForm(), TESForm, TESPackage);
		if (!pack)
			return true;

		UInt32 count = 1;
		if (eval.Arg(2) && eval.Arg(2)->CanConvertTo(kTokenType_Number))		// did we get an optional count?
			count = eval.Arg(2)->GetNumber();

		TESForm* form = eval.Arg(1)->GetTESForm();
		if (form)
		{
			// is it a base object or a reference?
			TESObjectREFR* refr = OBLIVION_CAST(form, TESForm, TESObjectREFR);
			if (refr)
			{
				if (!refr->IsPersistent())
				{
					ShowRuntimeError(scriptObj, "Non-persistent reference cannot be used as a package target.");
					return true;
				}

				pack->SetTarget(refr);
				return true;
			}

			// it's a base object
			pack->SetTarget(form, count);
			return true;
		}
		else if (eval.Arg(1)->CanConvertTo(kTokenType_Number))		// a type code
			pack->SetTarget(eval.Arg(1)->GetNumber(), count);
	}

	return true;
}

static bool GetPackageFlagFunc_Eval(COMMAND_ARGS_EVAL, UInt32 flag)
{
	TESPackage* pkg = (TESPackage*)arg1;
	*result = 0;

	if (!pkg && thisObj)
	{
		Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
		if (actor)
		{
			// try to locate an ExtraPackage first
			// (during dialog/combat editor package overridden by dynamic package and stored in ExtraPackage)
			ExtraPackage* xPack = (ExtraPackage*)actor->baseExtraList.GetByType(kExtraData_Package);
			if (xPack && xPack->package)
				pkg = xPack->package;
			else
				pkg = actor->GetCurrentPackage();
		}
	}

	if (pkg)
		*result = pkg->IsFlagSet(flag) ? 1 : 0;

	return true;
}

static bool GetPackageFlagFunc_Execute(COMMAND_ARGS, UInt32 flag)
{
	TESPackage* pkg = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &pkg))
		return GetPackageFlagFunc_Eval(thisObj, (void*)pkg, NULL, result, flag);

	return true;
}

static bool SetPackageFlagFunc_Execute(COMMAND_ARGS, UInt32 flag)
{
	TESPackage* pkg = NULL;
	UInt32 bSetFlag = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &bSetFlag, &pkg))
	{
		if (!pkg && thisObj)
		{
			Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
			if (actor)
			{
				// try to locate an ExtraPackage first
				// (during dialog/combat editor package overridden by dynamic package and stored in ExtraPackage)
				ExtraPackage* xPack = (ExtraPackage*)actor->baseExtraList.GetByType(kExtraData_Package);
				if (xPack && xPack->package)
					pkg = xPack->package;
				else
					pkg = actor->GetCurrentPackage();
			}
		}

		if (pkg)
			pkg->SetFlag(flag, bSetFlag ? true : false);
	}

	return true;
}

#define DEFINE_PKG_FLAG_FUNC(x) \
	static bool Cmd_GetPackage ## x ## _Eval(COMMAND_ARGS_EVAL) \
{ \
	return GetPackageFlagFunc_Eval(thisObj, arg1, arg2, result, TESPackage::kPackageFlag_ ## x); \
} \
	static bool Cmd_GetPackage ## x ## _Execute(COMMAND_ARGS) \
{ \
	return GetPackageFlagFunc_Execute(PASS_COMMAND_ARGS, TESPackage::kPackageFlag_ ## x); \
} \
	static bool Cmd_SetPackage ## x ## _Execute(COMMAND_ARGS) \
{ \
	return SetPackageFlagFunc_Execute(PASS_COMMAND_ARGS, TESPackage::kPackageFlag_ ## x); \
}

DEFINE_PKG_FLAG_FUNC(OffersServices)
DEFINE_PKG_FLAG_FUNC(MustReachLocation)
DEFINE_PKG_FLAG_FUNC(MustComplete)
DEFINE_PKG_FLAG_FUNC(LockDoorsAtStart)
DEFINE_PKG_FLAG_FUNC(LockDoorsAtEnd)
DEFINE_PKG_FLAG_FUNC(LockDoorsAtLocation)
DEFINE_PKG_FLAG_FUNC(UnlockDoorsAtStart)
DEFINE_PKG_FLAG_FUNC(UnlockDoorsAtEnd)
DEFINE_PKG_FLAG_FUNC(UnlockDoorsAtLocation)
DEFINE_PKG_FLAG_FUNC(ContinueIfPCNear)
DEFINE_PKG_FLAG_FUNC(OncePerDay)
DEFINE_PKG_FLAG_FUNC(SkipFalloutBehavior)
DEFINE_PKG_FLAG_FUNC(AlwaysRun)
DEFINE_PKG_FLAG_FUNC(AlwaysSneak)
DEFINE_PKG_FLAG_FUNC(AllowSwimming)
DEFINE_PKG_FLAG_FUNC(AllowFalls)
DEFINE_PKG_FLAG_FUNC(ArmorUnequipped)
DEFINE_PKG_FLAG_FUNC(WeaponsUnequipped)
DEFINE_PKG_FLAG_FUNC(DefensiveCombat)
DEFINE_PKG_FLAG_FUNC(UseHorse)
DEFINE_PKG_FLAG_FUNC(NoIdleAnims)

static bool Cmd_GetActorPackages_Execute(COMMAND_ARGS)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	TESActorBase* actorBase = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &actorBase))
	{
		if (!actorBase && thisObj)
			actorBase = OBLIVION_CAST(thisObj->baseForm, TESForm, TESActorBase);

		if (actorBase)
		{
			double idx = 0.0;
			for (TESAIForm::PackageEntry* cur = &actorBase->aiForm.packageList; cur && cur->package; cur = cur->next)
			{
				g_ArrayMap.SetElementFormID(arr, idx, cur->package->refID);
				idx += 1;
			}
		}
	}

	return true;
}

static bool GetCurrentPackage_Execute(COMMAND_ARGS, bool bGetEditorPackage)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj && thisObj->IsActor())
	{
		Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
		TESPackage* pkg = NULL;

		if (actor)
		{
			if (bGetEditorPackage)
			{
				ExtraPackage* xPack = (ExtraPackage*)actor->baseExtraList.GetByType(kExtraData_Package);
				if (xPack && xPack->package)
					pkg = xPack->package;
			}

			if (!pkg)
				pkg = actor->GetCurrentPackage();

			if (pkg)
				*refResult = pkg->refID;
		}
	}

	if (IsConsoleMode())
		Console_Print("GetCurrentPackage >> %08X", *refResult);

	return true;
}

static bool Cmd_GetCurrentPackage_Execute(COMMAND_ARGS)
{
	return GetCurrentPackage_Execute(PASS_COMMAND_ARGS, false);
}

static bool Cmd_GetCurrentEditorPackage_Execute(COMMAND_ARGS)
{
	return GetCurrentPackage_Execute(PASS_COMMAND_ARGS, true);
}

/*******************************************************
*
* TESPackage is complex enough to prohibit individual getters/setters for each field, particularly
* because several fields are implemented as unions
* So instead, return the relevant fields in a stringmap, and accept a stringmap as argument for modifying fields
* Prefer strings (matching those used by editor) to numeric type codes
*
* Layout:
*	Schedule:stringmap
*		Day:string
*		Month:string
*		Date:numeric
*		Time:numeric
*		Durtion:numeric
*	Target:stringmap
*		Type:string
*		Radius:string
*		Object:form OR ObjectType:string
*	Location:stringmap
*		Type:string
*		Value:numeric
*		Object:form OR ObjectType:string
*
*******************************************************/

static ArrayID ScheduleForPackage(TESPackage* pkg, Script* scriptObj)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	TESPackage::Time* time = &pkg->time;
	g_ArrayMap.SetElementString(arr, "Day", time->DayForCode(time->weekDay));
	g_ArrayMap.SetElementString(arr, "Month", time->MonthForCode(time->month));
	g_ArrayMap.SetElementNumber(arr, "Date", time->date);
	g_ArrayMap.SetElementNumber(arr, "Time", time->time);
	g_ArrayMap.SetElementNumber(arr, "Duration", time->duration);

	return arr;
}

static ArrayID LocationForPackage(TESPackage* pkg, Script* scriptObj)
{
	TESPackage::LocationData* loc = pkg->location;
	if (!loc)
		return 0;

	ArrayID arr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	g_ArrayMap.SetElementString(arr, "Type", loc->StringForLocationCode(loc->locationType));
	g_ArrayMap.SetElementNumber(arr, "Radius", loc->radius);
	switch (loc->locationType) {
		case loc->kPackLocation_InCell:
		case loc->kPackLocation_NearReference:
		case loc->kPackLocation_ObjectID:
			if (loc->object.form)
				g_ArrayMap.SetElementFormID(arr, "Object", loc->object.form->refID);
			else
				g_ArrayMap.SetElementFormID(arr, "Object", 0);
			break;
		case loc->kPackLocation_ObjectType:
			g_ArrayMap.SetElementString(arr, "ObjectType", TESPackage::StringForObjectCode(loc->object.objectCode));
	}

	return arr;
}

static ArrayID TargetForPackage(TESPackage* pkg, Script* scriptObj)
{
	TESPackage::TargetData* target = pkg->target;
	if (!target)
		return 0;

	ArrayID arr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	g_ArrayMap.SetElementString(arr, "Type", target->StringForTargetCode(target->targetType));
	g_ArrayMap.SetElementNumber(arr, "Value", target->count);
	if (target->targetType == TESPackage::kTargetType_TypeCode) {
		g_ArrayMap.SetElementString(arr, "ObjectType", TESPackage::StringForObjectCode(target->target.objectCode));
	}
	else {
		TESForm* form = target->target.form;
		g_ArrayMap.SetElementFormID(arr, "Object", form ? form->refID : 0);
	}

	return arr;
}

static bool Cmd_GetPackageData_Execute(COMMAND_ARGS)
{
	TESPackage* pkg = NULL;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &pkg) && pkg) {
		ArrayID arr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
		g_ArrayMap.SetElementArray(arr, "Schedule", ScheduleForPackage(pkg, scriptObj));
		g_ArrayMap.SetElementArray(arr, "Location", LocationForPackage(pkg, scriptObj));
		g_ArrayMap.SetElementArray(arr, "Target", TargetForPackage(pkg, scriptObj));
		g_ArrayMap.SetElementString(arr, "Type", TESPackage::StringForPackageType(pkg->type));
		*result = arr;
	}

	return true;
}

static bool Cmd_GetPackageScheduleData_Execute(COMMAND_ARGS)
{
	TESPackage* pkg = NULL;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &pkg) && pkg)
		*result = ScheduleForPackage(pkg, scriptObj);
	return true;
}

static bool Cmd_GetPackageTargetData_Execute(COMMAND_ARGS)
{
	TESPackage* pkg = NULL;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &pkg) && pkg)
		*result = TargetForPackage(pkg, scriptObj);
	return true;
}

static bool Cmd_GetPackageLocationData_Execute(COMMAND_ARGS)
{
	TESPackage* pkg = NULL;
	*result = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &pkg) && pkg)
		*result = LocationForPackage(pkg, scriptObj);
	return true;
}

static bool SetPackageScheduleData(TESPackage* pkg, ArrayID src)
{
	if (!g_ArrayMap.Exists(src) || g_ArrayMap.GetKeyType(src) != kDataType_String)
		return false;

	TESPackage::Time* time = &pkg->time;
	double val = 0;
	std::string str;

	if (g_ArrayMap.GetElementNumber(src, "time", &val) && time->IsValidTime(val))
		time->time = val;

	if (g_ArrayMap.GetElementNumber(src, "date", &val) && time->IsValidDate(val))
		time->date = val;

	if (g_ArrayMap.GetElementNumber(src, "duration", &val))
		time->duration = val;

	if (g_ArrayMap.GetElementString(src, "day", str)) {
		UInt8 day = time->CodeForDay(str.c_str());
		if (time->IsValidDay(day))
			time->weekDay = day;
	}

	if (g_ArrayMap.GetElementString(src, "month", str)) {
		UInt8 month = time->CodeForMonth(str.c_str());
		if (time->IsValidMonth(month))
			time->month = month;
	}

	return true;
}

static bool SetPackageLocationData(TESPackage* pkg, ArrayID src)
{
	if (!g_ArrayMap.Exists(src) || g_ArrayMap.GetKeyType(src) != kDataType_String)
		return false;

	double val = 0;
	std::string str;
	TESForm* form = NULL;

	// first make sure we've got a valid set of values
	if (!g_ArrayMap.GetElementString(src, "type", str))
		return false;

	UInt8 type = TESPackage::LocationData::LocationCodeForString(str.c_str());
	if (!TESPackage::LocationData::IsValidLocationType(type))
		return false;

	g_ArrayMap.GetElementForm(src, "object", &form);
	UInt8 objCode = TESPackage::kObjectType_Max;

	// validate location type matches object
	switch (type) {
		case TESPackage::LocationData::kPackLocation_InCell:
			if (!form || !OBLIVION_CAST(form, TESForm, TESObjectCELL))
				return false;
			break;
		case TESPackage::LocationData::kPackLocation_NearReference:
			if (!form || !OBLIVION_CAST(form, TESForm, TESObjectREFR))
				return false;
			break;
		case TESPackage::LocationData::kPackLocation_ObjectID:
			if (!form)
				return false;
			break;
		case TESPackage::LocationData::kPackLocation_ObjectType:
			if (g_ArrayMap.GetElementString(src, "objecttype", str)) {
				objCode = TESPackage::ObjectCodeForString(str.c_str());
				if (!TESPackage::IsValidObjectCode(objCode))
					return false;
			}
			break;
		default:	// editor/current location
			break;
	}

	// okay, good to go.
	TESPackage::LocationData* loc = pkg->GetLocationData();

	loc->locationType = type;
	if (g_ArrayMap.GetElementNumber(src, "radius", &val))
		loc->radius = val;

	switch (type) {
		case loc->kPackLocation_InCell:
		case loc->kPackLocation_NearReference:
		case loc->kPackLocation_ObjectID:
			loc->object.form = form;
			break;
		case loc->kPackLocation_ObjectType:
			loc->object.objectCode = objCode;
			break;
		default:	// editor/current
			loc->object.form = NULL;
	}

	return true;
}

static bool SetPackageTargetData(TESPackage* pkg, ArrayID src)
{
	if (!g_ArrayMap.Exists(src) || g_ArrayMap.GetKeyType(src) != kDataType_String)
		return false;

	double val = 0;
	std::string str;
	TESForm* form = NULL;
	TESPackage::TargetData* target = NULL;

	// make sure passed data is consistent
	if (!g_ArrayMap.GetElementString(src, "type", str))
		return false;

	UInt8 type = TESPackage::TargetData::TargetCodeForString(str.c_str());
	if (!TESPackage::TargetData::IsValidTargetCode(type))
		return false;

	g_ArrayMap.GetElementForm(src, "object", &form);
	switch (type) {
		case TESPackage::kTargetType_TypeCode:
			if (g_ArrayMap.GetElementString(src, "objecttype", str)) {
				UInt8 objCode = TESPackage::ObjectCodeForString(str.c_str());
				if (!TESPackage::IsValidObjectCode(objCode))
					return false;

				// good to go
				target = pkg->GetTargetData();
				target->target.objectCode = objCode;
			}
			else
				return false;
			break;
		case TESPackage::kTargetType_Refr:
			// make sure it IS a reference
			if (!form || !OBLIVION_CAST(form, TESForm, TESObjectREFR))
				return false;
		case TESPackage::kTargetType_BaseObject:	// fall-through intentional
			if (!form)
				return false;
			// ok, good
			target = pkg->GetTargetData();
			target->target.form = form;
	}

	if (target) {
		if (g_ArrayMap.GetElementNumber(src, "value", &val))
			target->count = val;
		target->targetType = type;
		return true;
	}

	return false;
}

enum {
	kPackData_All,
	kPackData_Schedule,
	kPackData_Location,
	kPackData_Target
};

static bool SetPackageData_Execute(COMMAND_ARGS, UInt32 whichData)
{
	*result = 0;
	bool bResult = false;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (!eval.ExtractArgs() || eval.NumArgs() != 2)
		return true;

	TESPackage* pkg = (TESPackage*)OBLIVION_CAST(eval.Arg(0)->GetTESForm(), TESForm, TESPackage);
	ArrayID arr = eval.Arg(1)->GetArray();
	if (pkg && arr) {
		switch (whichData) {
			case kPackData_Schedule:
				bResult = SetPackageScheduleData(pkg, arr);
				break;
			case kPackData_Location:
				bResult = SetPackageLocationData(pkg, arr);
				break;
			case kPackData_Target:
				bResult = SetPackageTargetData(pkg, arr);
			case kPackData_All:
				{
					ArrayID sub = 0;
					bResult = true;
					// user can pass any or all of the following sub-arrays
					if (g_ArrayMap.GetElementArray(arr, "schedule", &sub) && !SetPackageScheduleData(pkg, sub))
						bResult = false;
					if (g_ArrayMap.GetElementArray(arr, "target", &sub) && !SetPackageTargetData(pkg, sub))
						bResult = false;
					if (g_ArrayMap.GetElementArray(arr, "location", &sub) && !SetPackageLocationData(pkg, sub))
						bResult = false;
				}
				break;
		}
	}

	*result = bResult ? 1.0 : 0.0;
	return true;
}

static bool Cmd_SetPackageData_Execute(COMMAND_ARGS)
{
	return SetPackageData_Execute(PASS_COMMAND_ARGS, kPackData_All);
}

static bool Cmd_SetPackageScheduleData_Execute(COMMAND_ARGS)
{
	return SetPackageData_Execute(PASS_COMMAND_ARGS, kPackData_Schedule);
}

static bool Cmd_SetPackageLocationData_Execute(COMMAND_ARGS)
{
	return SetPackageData_Execute(PASS_COMMAND_ARGS, kPackData_Location);
}

static bool Cmd_SetPackageTargetData_Execute(COMMAND_ARGS)
{
	return SetPackageData_Execute(PASS_COMMAND_ARGS, kPackData_Target);
}

static bool Cmd_GetPackageType_Execute(COMMAND_ARGS)
{
	const char* typeStr = "";
	TESPackage* pkg = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &pkg) && pkg) {
		typeStr = TESPackage::StringForPackageType(pkg->type);
	}

	AssignToStringVar(PASS_COMMAND_ARGS, typeStr);
	return true;
}

static bool Cmd_GetCurrentPackageProcedure_Execute(COMMAND_ARGS)
{
	const char* procName = "NONE";
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && actor->process && actor->process->GetCurrentPackage()) {
		const char* name = TESPackage::StringForProcedureCode((TESPackage::eProcedure)actor->process->GetCurrentPackProcedure());
		if (name) {
			procName = name;
		}
	}

	AssignToStringVar(PASS_COMMAND_ARGS, procName);
	if (IsConsoleMode()) {
		Console_Print("GetCurrentPackageProcedure >> %s", procName);
	}

	return true;
}

static bool Cmd_GetGroundSurfaceMaterial_Execute(COMMAND_ARGS)
{
	*result = -1;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && actor->process)
	{
		MiddleHighProcess* midHigProcess = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
		if (midHigProcess)
		{
			bhkCharacterController* hkController = midHigProcess->GetCharacterController();
			if (hkController)
			{
				*result = hkController->listener.groundSurfaceMaterial;
				if (IsConsoleMode())
					Console_Print("GetGroundSurfaceMaterial >> %d", hkController->listener.groundSurfaceMaterial);
			}
		}
	}

	return true;
}

#endif

static ParamInfo kParams_OneNPC[1] =
{
	{	"NPC",	kParamType_NPC,	1	},
};

static ParamInfo kParams_OneIntOneOptionalNPC[2] =
{
	{	"RGB value",	kParamType_Integer,	0	},
	{	"NPC",			kParamType_NPC,		1	},
};

CommandInfo kCommandInfo_GetNumFollowers =
{
	"GetNumFollowers", "", 0,
	"returns the number of characters following the calling actor",
	1, 0, NULL,
	HANDLER(Cmd_GetNumFollowers_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthFollower =
{
	"GetNthFollower", "", 0,
	"returns the nth actor following the calling actor",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthFollower_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthDetectedActor =
{
	"GetNthDetectedActor",
	"",
	0,
	"returns the nth actor detected by the calling actor",
	1,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GetNthDetectedActor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNumDetectedActors =
{
	"GetNumDetectedActors",
	"",
	0,
	"returns the number of actors detected by the calling actor",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetNumDetectedActors_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetDetectionState[2] =
{
	{	"actor",	kParamType_Actor,	0	},
	{	"level",	kParamType_Integer,	0	},
};

CommandInfo kCommandInfo_SetDetectionState =
{
	"SetDetectionState", "",
	0,
	"sets the level at which the calling actor detects the specified actor",
	1, 2, kParams_SetDetectionState,
	HANDLER(Cmd_SetDetectionState_Execute),
	Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_OffersWeapons =
{
	"OffersWeapons",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersWeapons_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersArmor =
{
	"OffersArmor",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersArmor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersBooks =
{
	"OffersBooks",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersBooks_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersClothing =
{
	"OffersClothing",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersClothing_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersIngredients =
{
	"OffersIngredients",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersIngredients_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersLights =
{
	"OffersLights",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersLights_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersApparatus =
{
	"OffersApparatus",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersApparatus_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersMiscItems =
{
	"OffersMiscItems",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersMiscItems_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersMagicItems =
{
	"OffersMagicItems",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersMagicItems_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersSpells =
{
	"OffersSpells",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersSpells_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersRecharging =
{
	"OffersRecharging",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersRecharging_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersTraining =
{
	"OffersTraining",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersTraining_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersPotions =
{
	"OffersPotions",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersPotions_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersRepair =
{
	"OffersRepair",
	"",
	0,
	"returns 1 if the calling actor offers the specified service",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_OffersRepair_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTrainerSkill =
{
	"GetTrainerSkill",
	"",
	0,
	"returns the skill in which the actor offers training",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_GetTrainerSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetTrainerLevel =
{
	"GetTrainerLevel",
	"",
	0,
	"returns the level at which the actor offers training",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_GetTrainerLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_OffersServicesC =
{
	"OffersServicesC",
	"",
	0,
	"returns 1 if the calling actor offers the specified services",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_OffersServicesC_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersWeapons =
{
	"SetOffersWeapons",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersWeapons_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersArmor =
{
	"SetOffersArmor",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersArmor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersClothing =
{
	"SetOffersClothing",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersClothing_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersBooks =
{
	"SetOffersBooks",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersBooks_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersLights =
{
	"SetOffersLights",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersLights_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersPotions =
{
	"SetOffersPotions",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersPotions_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersSpells =
{
	"SetOffersSpells",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersSpells_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersIngredients =
{
	"SetOffersIngredients",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersIngredients_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersApparatus =
{
	"SetOffersApparatus",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersApparatus_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersMiscItems =
{
	"SetOffersMiscItems",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersMiscItems_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersMagicItems =
{
	"SetOffersMagicItems",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersMagicItems_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersTraining =
{
	"SetOffersTraining",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersTraining_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersRepair =
{
	"SetOffersRepair",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersRepair_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersRecharging =
{
	"SetOffersRecharging",
	"",
	0,
	"sets the specified offers services flag for the NPC",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetOffersRecharging_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetTrainerLevel =
{
	"SetTrainerLevel",
	"",
	0,
	"sets the level at which the NPC offers training",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetTrainerLevel_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetTrainerSkill[2] =
{
	{	"skill",	kParamType_ActorValue,	0	},
	{	"npc",		kParamType_NPC,			1	},
};

CommandInfo kCommandInfo_SetTrainerSkill =
{
	"SetTrainerSkill",
	"",
	0,
	"sets the skill in which the NPC offers training",
	0,
	2,
	kParams_SetTrainerSkill,
	HANDLER(Cmd_SetTrainerSkill_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetOffersServicesC =
{
	"SetServicesMask",
	"SetOffersServicesC",
	0,
	"sets the services offered by the npc",
	0,
	2,
	kParams_OneIntOneOptionalNPC,
	HANDLER(Cmd_SetServicesMask_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetServicesMask =
{
	"GetServicesMask",
	"GetServicesC",
	0,
	"returns the code for all services offered by the npc",
	0,
	1,
	kParams_OneNPC,
	HANDLER(Cmd_GetServicesMask_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneIntOneOptionalActorBase[2] =
{
	{	"int",		kParamType_Integer,		0	},
	{	"actor",	kParamType_ActorBase,	1	},
};

CommandInfo kCommandInfo_GetNumPackages =
{
	"GetNumPackages",
	"",
	0,
	"returns the number of packages in the actor's package list",
	0,
	1,
	kParams_OneOptionalActorBase,
	HANDLER(Cmd_GetNumPackages_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetNthPackage =
{
	"GetNthPackage",
	"",
	0,
	"returns the nth package in the actor's package list",
	0,
	2,
	kParams_OneIntOneOptionalActorBase,
	HANDLER(Cmd_GetNthPackage_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsAttacking =
{
	"IsAttacking",
	"",
	0,
	"returns true if the calling actor is attacking",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsAttacking_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsBlocking =
{
	"IsBlocking",
	"",
	0,
	"returns true if the calling actor is Blocking",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsBlocking_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsDodging =
{
	"IsDodging",
	"",
	0,
	"returns true if the calling actor is Dodging",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsDodging_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsRecoiling =
{
	"IsRecoiling",
	"",
	0,
	"returns true if the calling actor is Recoiling",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsRecoiling_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsStaggered =
{
	"IsStaggered",
	"",
	0,
	"returns true if the calling actor is Staggered",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsStaggered_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMovingForward =
{
	"IsMovingForward",
	"",
	0,
	"returns 1 if the calling actor is performing the specified movement",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsMovingForward_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMovingBackward =
{
	"IsMovingBackward",
	"",
	0,
	"returns 1 if the calling actor is performing the specified movement",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsMovingBackward_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMovingLeft =
{
	"IsMovingLeft",
	"",
	0,
	"returns 1 if the calling actor is performing the specified movement",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsMovingLeft_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsMovingRight =
{
	"IsMovingRight",
	"",
	0,
	"returns 1 if the calling actor is performing the specified movement",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsMovingRight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsTurningLeft =
{
	"IsTurningLeft",
	"",
	0,
	"returns 1 if the calling actor is performing the specified movement",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsTurningLeft_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsTurningRight =
{
	"IsTurningRight",
	"",
	0,
	"returns 1 if the calling actor is performing the specified movement",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsTurningRight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsInAir =
{
	"IsInAir",
	"",
	0,
	"returns 1 if the calling actor is in the air",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsInAir_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsJumping =
{
	"IsJumping",
	"",
	0,
	"returns 1 if the calling actor is jumping",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsJumping_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsOnGround =
{
	"IsOnGround",
	"",
	0,
	"returns 1 if the calling actor is on the ground",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsOnGround_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsFlying =
{
	"IsFlying",
	"",
	0,
	"returns 1 if the calling actor is flying",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsFlying_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetFallTimer =
{
	"GetFallTimer",
	"GetFallDamageTimer",
	0,
	"returns the length of time for which the calling actor has been falling",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetFallTimer_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_IsCasting =
{
	"IsCasting",
	"",
	0,
	"returns true if the calling actor is casting a spell",
	1,
	0,
	NULL,
	HANDLER(Cmd_IsCasting_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneAnimGroup[1] =
{
	{	"anim group",	kParamType_AnimationGroup,	0	},
};

DEFINE_COMMAND(IsPowerAttacking,
			   returns true if the calling actor is executing a power attack,
			   1,
			   0,
			   NULL);

DEFINE_COMMAND(IsAnimGroupPlaying,
			   returns true if the actor is playing the specified anim group,
			   1,
			   1,
			   kParams_OneAnimGroup);

DEFINE_COMMAND(AnimPathIncludes,
			   returns true if the actor is playing an anim containing the substring,
			   1,
			   1,
			   kParams_OneString);

static ParamInfo kParams_Package[1] =
{
	{	"package",	kParamType_AIPackage,	0	},
};

DEFINE_COMMAND(GetProcessLevel,
			   returns the process level of the calling reference,
			   1,
			   0,
			   NULL);

CommandInfo kCommandInfo_GetFollowers =
{
	"GetFollowers",
	"",
	0,
	"returns an array containing all actors currently following the calling ref",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetFollowers_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_SetPackageTarget[3] =
{
	{	"package",	kOBSEParamType_Form,		0	},
	{	"target",	kOBSEParamType_FormOrNumber,0	},
	{	"count",	kOBSEParamType_Number,		1	},
};

CommandInfo kCommandInfo_SetPackageTarget =
{
	"SetPackageTarget",
	"",
	0,
	"sets the target of a package",
	0,
	3,
	kOBSEParams_SetPackageTarget,
	HANDLER(Cmd_SetPackageTarget_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneOptionalPackage[1] =
{
	{	"package",	kParamType_AIPackage,	1	},
};

static ParamInfo kParams_OneInt_OneOptionalPackage[2] =
{
	{	"int",		kParamType_Integer,		0	},
	{	"package",	kParamType_AIPackage,	1	},
};

DEFINE_CMD_COND(GetPackageOffersServices, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageMustReachLocation, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageMustComplete, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageLockDoorsAtStart, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageLockDoorsAtEnd, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageLockDoorsAtLocation, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageUnlockDoorsAtStart, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageUnlockDoorsAtEnd, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageUnlockDoorsAtLocation, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageContinueIfPCNear, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageOncePerDay, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageSkipFalloutBehavior, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageAlwaysRun, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageAlwaysSneak, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageAllowSwimming, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageAllowFalls, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageArmorUnequipped, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageWeaponsUnequipped, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageDefensiveCombat, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageUseHorse, gets a package flag, 0, kParams_OneOptionalPackage);
DEFINE_CMD_COND(GetPackageNoIdleAnims, gets a package flag, 0, kParams_OneOptionalPackage);

DEFINE_COMMAND(SetPackageOffersServices, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageMustReachLocation, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageMustComplete, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageLockDoorsAtStart, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageLockDoorsAtEnd, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageLockDoorsAtLocation, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageUnlockDoorsAtStart, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageUnlockDoorsAtEnd, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageUnlockDoorsAtLocation, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageContinueIfPCNear, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageOncePerDay, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageSkipFalloutBehavior, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageAlwaysRun, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageAlwaysSneak, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageAllowSwimming, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageAllowFalls, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageArmorUnequipped, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageWeaponsUnequipped, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageDefensiveCombat, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageUseHorse, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);
DEFINE_COMMAND(SetPackageNoIdleAnims, sets a package flag, 0, 2, kParams_OneInt_OneOptionalPackage);

DEFINE_COMMAND(GetActorPackages, returns the actors package list as an array, 0, 1, kParams_OneOptionalActorBase);
DEFINE_COMMAND(GetCurrentPackage, returns the actor refs currently executing package, 1, 0, NULL);
DEFINE_COMMAND(GetCurrentEditorPackage, returns the actor refs executing non-dynamic editor package, 1, 0, NULL);

DEFINE_COMMAND(GetPackageData, returns a stringmap containing info about a package, 0, 1, kParams_OnePackage);
DEFINE_COMMAND(GetPackageScheduleData, returns a stringmap containing info about a package schedule, 0, 1, kParams_OnePackage);
DEFINE_COMMAND(GetPackageLocationData, returns a stringmap containing info about a package location, 0, 1, kParams_OnePackage);
DEFINE_COMMAND(GetPackageTargetData, returns a stringmap containing info about a package target, 0, 1, kParams_OnePackage);

static ParamInfo kOBSEParams_SetPackageData[2] =
{
	{	"package",	kOBSEParamType_Form,		0	},
	{	"target",	kOBSEParamType_Array,		0	},
};

CommandInfo kCommandInfo_SetPackageData =
{
	"SetPackageData",
	"",
	0,
	"sets the target, schedule, and/or location of a package",
	0,
	2,
	kOBSEParams_SetPackageData,
	HANDLER(Cmd_SetPackageData_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetPackageTargetData =
{
	"SetPackageTargetData",
	"",
	0,
	"sets the target of a package",
	0,
	2,
	kOBSEParams_SetPackageData,
	HANDLER(Cmd_SetPackageTargetData_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetPackageScheduleData =
{
	"SetPackageScheduleData",
	"",
	0,
	"sets the schedule of a package",
	0,
	2,
	kOBSEParams_SetPackageData,
	HANDLER(Cmd_SetPackageScheduleData_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetPackageLocationData =
{
	"SetPackageLocationData",
	"",
	0,
	"sets the location of a package",
	0,
	2,
	kOBSEParams_SetPackageData,
	HANDLER(Cmd_SetPackageLocationData_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(GetPackageType, returns the type of a package as a string, 0, 1, kParams_OnePackage);

DEFINE_COMMAND(GetCurrentPackageProcedure, returns the actors current package procedure as a string, 1, 0, NULL);
DEFINE_COMMAND(GetGroundSurfaceMaterial, returns the surface type the actor is currently standing on, 1, 0, NULL);