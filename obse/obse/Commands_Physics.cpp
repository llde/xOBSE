#include "Commands_Physics.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "ArrayVar.h"
#include "ScriptUtils.h"
#include "Utilities.h"
#include "NiHavok.h"

static bhkWorldM** g_bhkWorldM = (bhkWorldM**)0x00B35C24;


static hkWorld* GethkWorld()
{
	bhkWorld* bWorld = NULL;
	PlayerCharacter* pc = *g_thePlayer;
	if (pc) {
		TESObjectCELL* cell = pc->parentCell;
		if (cell) {
			ExtraHavok* xWorld = (ExtraHavok*)cell->extraData.GetByType(kExtraData_Havok);
			if (xWorld) {
				bWorld = xWorld->world;
			}
		}
	}

	if (!bWorld) {
		bWorld = *g_bhkWorldM;
	}

	return bWorld ? bWorld->GetWorld() : NULL;
}

static bool Cmd_SetLocalGravity_Execute(COMMAND_ARGS)
{
	float grav = 0.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &grav)) {
		hkWorld* world = GethkWorld();
		if (world) {
			world->m_gravity.z = GameUnitsToCM(grav);
		}
	}

	return true;
}

static bool Cmd_SetVelocity_Execute(COMMAND_ARGS)
{
	// currently actors only. bhkCharacterController will reset x/y velocity (and z if not in air)
	// so it needs to be called once with a large magnitude, or repeatedly over several frames, to have a visible/useful effect
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;

	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && ExtractArgs(PASS_EXTRACT_ARGS, &x, &y, &z)) {
		MiddleHighProcess* mhProc = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
		if (mhProc && mhProc->charProxy) {
			ahkCharacterProxy* proxy = (ahkCharacterProxy*)(mhProc->charProxy->hkObj);
			proxy->velocity.x = GameUnitsToCM(x);
			proxy->velocity.y = GameUnitsToCM(y);
			proxy->velocity.z = GameUnitsToCM(z);
		}
	}

	return true;
}

static bool Cmd_SetLocalGravityVector_Execute(COMMAND_ARGS)
{
	// mostly a toy command. Could be useful for e.g. simulating water currents
	hkWorld* world = GethkWorld();
	if (world) {
		float x = 0.0;
		float y = 0.0;
		float z = 0.0;

		if (ExtractArgs(PASS_EXTRACT_ARGS, &x, &y, &z)) {
			world->m_gravity.x = GameUnitsToCM(x);
			world->m_gravity.y = GameUnitsToCM(y);
			world->m_gravity.z = GameUnitsToCM(z);
		}
	}

	return true;
}

static bool Cmd_GetLocalGravity_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	char axis = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &axis)) {
		hkWorld* world = GethkWorld();
		float grav = 0.0;
		if (world) {
			switch (axis) {
				case 'X':
				case 'x':
					grav = world->m_gravity.x;
					break;
				case 'Y':
				case 'y':
					grav = world->m_gravity.y;
					break;
				case 'Z':
				case 'z':
					grav = world->m_gravity.z;
					break;
			}
			*result = CMToGameUnits(grav);
		}
	}

	if (IsConsoleMode()) {
		Console_Print("GetLocalGravity >> %.4f", *result);
	}

	return true;
}

static bool GetActorVelocity(COMMAND_ARGS, char axis)
{
	// actors only
	*result = 0.0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor) {
		MiddleHighProcess* mhProc = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
		if (mhProc && mhProc->charProxy) {
			ahkCharacterProxy* proxy = (ahkCharacterProxy*)(mhProc->charProxy->hkObj);
			float vel = 0.0;
			switch (axis) {
				case 'X':
				case 'x':
					vel = proxy->velocity.x;
					break;
				case 'Y':
				case 'y':
					vel = proxy->velocity.y;
					break;
				case 'Z':
				case 'z':
					vel = proxy->velocity.z;
					break;
			}

			*result = CMToGameUnits(vel);
		}
	}

	return true;
}

static bool Cmd_GetVerticalVelocity_Execute(COMMAND_ARGS)
{
	GetActorVelocity(PASS_COMMAND_ARGS, 'Z');
	if (IsConsoleMode()) {
		Console_Print("GetVerticalVelocity %.4f", *result);
	}

	return true;
}

static bool Cmd_GetVelocity_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	char axis = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &axis)) {
		GetActorVelocity(PASS_COMMAND_ARGS, axis);
	}
	
	if (IsConsoleMode()) {
		Console_Print("GetVelocity >> %.4f", *result);
	}

	return true;
}

static bool Cmd_SetVerticalVelocity_Execute(COMMAND_ARGS)
{
	// only for actors. only effective for HKState_InAir/Jumping.
	*result = 0.0;
	float newVel = 0.0;
	Actor* actor = OBLIVION_CAST(thisObj, TESObjectREFR, Actor);
	if (actor && ExtractArgs(PASS_EXTRACT_ARGS, &newVel)) {
		MiddleHighProcess* mhProc = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
		if (mhProc && mhProc->charProxy) {
			ahkCharacterProxy* proxy = (ahkCharacterProxy*)(mhProc->charProxy->hkObj);
			proxy->velocity.z = GameUnitsToCM(newVel);
			*result = 1.0;
		}
	}

	return true;
}

#endif

DEFINE_COMMAND(SetLocalGravity, sets the gravity for the current cell, 0, 1, kParams_OneFloat);

static ParamInfo kParams_Vec3[3] =
{
	{ "x",	kParamType_Float,	0	},
	{ "y",	kParamType_Float,	0	},
	{ "z",	kParamType_Float,	0	},
};

DEFINE_COMMAND(SetVelocity, sets velocity for an actor, 1, 3, kParams_Vec3);
DEFINE_COMMAND(SetLocalGravityVector, sets the gravity vector for the current cell, 0, 3, kParams_Vec3);

DEFINE_COMMAND(SetVerticalVelocity, sets an actors falling velocity, 1, 1, kParams_OneFloat);
DEFINE_COMMAND(GetVerticalVelocity, returns an actors falling velocity, 1, 0, NULL);

DEFINE_COMMAND(GetVelocity, returns an actors velocity in the specified axis, 1, 1, kParams_OneAxis);
DEFINE_COMMAND(GetLocalGravity, returns gravitational acceleration in the specified axis, 0, 1, kParams_OneAxis);