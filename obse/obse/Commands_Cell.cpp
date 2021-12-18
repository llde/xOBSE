#include "Commands_Cell.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameExtraData.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "ArrayVar.h"
#include "GameData.h"

static bool Cmd_GetCellMusicType_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectCELL* curCell = (*g_thePlayer)->parentCell;

	BSExtraData* xData = curCell->extraData.GetByType(kExtraData_CellMusicType);
	if (xData)
	{
		ExtraCellMusicType* xMusic = (ExtraCellMusicType*)Oblivion_DynamicCast(xData, 0, RTTI_BSExtraData, RTTI_ExtraCellMusicType, 0);
		if (xMusic)
			*result = xMusic->musicType;
	}

	return true;
}

static bool Cmd_SetCellMusicType_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	UInt32 musicType = -1;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &musicType)) {
		if (cell && musicType < ExtraCellMusicType::kMusicType_MAX) {
			ExtraCellMusicType* xMusic = OBLIVION_CAST(cell->extraData.GetByType(kExtraData_CellMusicType), BSExtraData, ExtraCellMusicType);
			if (xMusic) {
				xMusic->musicType = musicType;
			}
			else {
				xMusic = ExtraCellMusicType::Create(musicType);
				cell->extraData.Add(xMusic);
			}

			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_SetCellWaterHeight_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectCELL* cell = NULL;
	float newHeight = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &newHeight) && cell)
		*result = cell->SetWaterHeight(newHeight) ? 1 : 0;

	return true;
}

static bool Cmd_CellHasWater_Execute(COMMAND_ARGS)
{
	*result = 0;

	if(!thisObj) return true;
	TESObjectCELL* cell = thisObj->parentCell;
	if (!cell) return true;

	*result = cell->HasWater() ? 1 : 0;

	return true;
}

static bool Cmd_GetCellWaterHeight_Execute(COMMAND_ARGS)
{
	*result = 0;
	if(!thisObj) return true;
	TESObjectCELL* cell = thisObj->parentCell;
	if (!cell) return true;

	if (!cell->HasWater()) return true;

	float waterHeight = cell->GetWaterHeight();
	*result = waterHeight;
	return true;
}

static bool Cmd_GetCellDetachTime_Execute(COMMAND_ARGS)
{
	*result = -1;
	TESObjectCELL* cell = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell)
	{
		ExtraDetachTime* detachTime = (ExtraDetachTime*)cell->extraData.GetByType(kExtraData_DetachTime);
		if (detachTime)
			*result = (SInt32)detachTime->detachTime;
	}
	return true;
}

static bool Cmd_GetCellResetHours_Execute(COMMAND_ARGS)
{
	*result = -1;
	TESObjectCELL* cell = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell)
	{
		SInt32 iHoursToRespawn = TimeGlobals::HoursToRespawnCell();
		SInt32 iHoursPassed = TimeGlobals::GameHoursPassed();

		ExtraDetachTime* xDetach = (ExtraDetachTime*)cell->extraData.GetByType(kExtraData_DetachTime);
		if (xDetach)
		{
			SInt32 detachTime = xDetach->detachTime;
			if (xDetach->detachTime == 0xFFFFFFFF)		// ResetInterior sets it to this for immediate respawn
				*result = 0;
			else
			{
				detachTime += iHoursToRespawn;
				if (detachTime <= iHoursPassed)
					*result = 0;
				else
					*result = detachTime - iHoursPassed;
			}
		}
	}

	return true;
}

static bool Cmd_SetCellResetHours_Execute(COMMAND_ARGS)
{
	// specifies # of hours from now until cell reset
	*result = 0;
	TESObjectCELL* cell = NULL;
	UInt32 numHours = -1;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &numHours) && cell && numHours != -1)
	{
		if (cell->IsInterior() && cell != (*g_thePlayer)->parentCell)
		{
			SInt32 iHoursToRespawn = TimeGlobals::HoursToRespawnCell();
			SInt32 iHoursPassed = TimeGlobals::GameHoursPassed();
			SInt32 detachTime = iHoursPassed + numHours - iHoursToRespawn;
			if (detachTime < iHoursPassed)
			{
				*result = 1;
				CALL_MEMBER_FN(cell, SetDetachTime)(detachTime);
				if (IsConsoleMode())
					Console_Print("Current hours passed :%d Detach time: %d", iHoursPassed, detachTime);
			}
			else if (IsConsoleMode())
				Console_Print("Detach time %d cannot be greater than current hours passed %d", detachTime, iHoursPassed);
		}
	}

	return true;
}

static bool Cmd_GetWorldSpaceParentWorldSpace_Execute(COMMAND_ARGS)
{
	TESWorldSpace* space = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &space) && space && space->parentWorldspace)
		*refResult = space->parentWorldspace->refID;

	return true;
}

static bool Cmd_GetCellBehavesAsExterior_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell)
		if (cell->BehavesLikeExterior())
			*result = 1;

	if (IsConsoleMode())
		Console_Print("GetCellBehavesAsExterior >> %.0f", *result);

	return true;
}

ArrayID RGBAToArray(RGBA& rgba, Script* script)
{
	ArrayID id = g_ArrayMap.Create(kDataType_Numeric, true, script->GetModIndex());
	g_ArrayMap.SetElementNumber(id, 0.0, rgba.r);
	g_ArrayMap.SetElementNumber(id, 1.0, rgba.g);
	g_ArrayMap.SetElementNumber(id, 2.0, rgba.b);
	return id;
}

static bool Cmd_GetCellLighting_Execute(COMMAND_ARGS)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	TESObjectCELL* cell = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell && cell->IsInterior()) {
		TESObjectCELL::LightingData* data = cell->lighting;
		if (data) {
			g_ArrayMap.SetElementArray(arr, "ambient", RGBAToArray(data->ambient, scriptObj));
			g_ArrayMap.SetElementArray(arr, "directional", RGBAToArray(data->directional, scriptObj));
			g_ArrayMap.SetElementArray(arr, "fog", RGBAToArray(data->fog, scriptObj));
			g_ArrayMap.SetElementNumber(arr, "rotxy", data->rotXY);
			g_ArrayMap.SetElementNumber(arr, "rotz", data->rotZ);
			g_ArrayMap.SetElementNumber(arr, "fognear", data->fogNear);
			g_ArrayMap.SetElementNumber(arr, "fogfar", data->fogFar);
			g_ArrayMap.SetElementNumber(arr, "clip", data->fogClipDistance);
			g_ArrayMap.SetElementNumber(arr, "fade", data->directionalFade);
		}
	}

	*result = arr;
	return true;
}

static bool SetRGBFromArray(ArrayID arr, RGBA& outRGB)
{
	ArrayVar* var = g_ArrayMap.Get(arr);
	if (var && var->IsPacked() && var->Size() == 3) {
		double rgb[3];
		for (UInt32 i=0; i < 3; i++) {
			if (g_ArrayMap.GetElementNumber(arr, i, &rgb[i])) {
				rgb[i] = min(max(0.0, rgb[i]), 255.0);
			}
			else {
				return false;
			}
		}
		outRGB.Set(rgb[0], rgb[1], rgb[2]);
		outRGB.a = 0;
		return true;
	}

	return false;
}

static bool Cmd_SetCellLighting_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 2) {
		TESObjectCELL* cell = OBLIVION_CAST(eval.Arg(0)->GetTESForm(), TESForm, TESObjectCELL);
		ArrayID data = eval.Arg(1)->GetArray();
		if (cell && data && cell->IsInterior() && cell->lighting && g_ArrayMap.GetKeyType(data) == kDataType_String) {
			ArrayID arr;
			double num;

			if (g_ArrayMap.GetElementArray(data, "ambient", &arr)) {
				SetRGBFromArray(arr, cell->lighting->ambient);
			}
			if (g_ArrayMap.GetElementArray(data, "directional", &arr)) {
				SetRGBFromArray(arr, cell->lighting->directional);
			}
			if (g_ArrayMap.GetElementArray(data, "fog", &arr)) {
				SetRGBFromArray(arr, cell->lighting->fog);
			}
			if (g_ArrayMap.GetElementNumber(data, "rotxy", &num)) {
				// ###TODO: limits
				cell->lighting->rotXY = num;
			}
			if (g_ArrayMap.GetElementNumber(data, "rotz", &num)) {
				// ###TODO: limits
				cell->lighting->rotZ = num;
			}
			if (g_ArrayMap.GetElementNumber(data, "fognear", &num)) {
				cell->lighting->fogNear = num;
			}
			if (g_ArrayMap.GetElementNumber(data, "fogfar", &num)) {
				cell->lighting->fogFar = num;
			}
			if (g_ArrayMap.GetElementNumber(data, "clip", &num)) {
				cell->lighting->fogClipDistance = num;
			}
			if (g_ArrayMap.GetElementNumber(data, "fade", &num)) {
				cell->lighting->directionalFade = num;
			}

			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_GetCellClimate_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = (*g_thePlayer)->parentCell;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
		ExtraCellClimate* xClim = (ExtraCellClimate*)cell->extraData.GetByType(kExtraData_CellClimate);
		if (xClim && xClim->climate) {
			*refResult = xClim->climate->refID;
		}
	}

	return true;
}

static bool Cmd_SetCellClimate_Execute(COMMAND_ARGS)
{
	*result = 0.0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 2) {
		TESObjectCELL* cell = OBLIVION_CAST(eval.Arg(0)->GetTESForm(), TESForm, TESObjectCELL);
		TESClimate* clim = OBLIVION_CAST(eval.Arg(1)->GetTESForm(), TESForm, TESClimate);
		if (clim && cell) {	
			if (cell->IsInterior() && cell != (*g_thePlayer)->parentCell) {
				cell->SetInteriorClimate(clim);
				*result = 1.0;
			}
		}
	}

	return true;
}

static bool Cmd_SetCellBehavesAsExterior_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	UInt32 bSet = 0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &bSet) && cell) {
		if (cell->IsInterior() && cell != (*g_thePlayer)->parentCell) {
			cell->SetBehavesLikeExterior(bSet ? true : false);
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_SetCellHasWater_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	UInt32 bHas = 0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS,&cell, &bHas) && cell) {
		if (cell->IsInterior() && cell != (*g_thePlayer)->parentCell) {
			cell->SetHasWater(bHas ? true : false);
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_SetCellIsPublic_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	UInt32 bPublic = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &bPublic) && cell) {
		cell->SetIsPublic(bPublic ? true : false);
	}

	return true;
}

static bool Cmd_IsCellPublic_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	*result = 0.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
		if (cell->GetIsPublic()) {
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_GetTerrainHeight_Execute(COMMAND_ARGS)
{
	float pos[3] = { 0.0, 0.0, 0.0 };
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &pos[0], &pos[1])) {
		float height = 0.0;
		TES* tes = TES::GetSingleton();
		if (tes->GetTerrainHeight(pos, &height)) {
			*result = height;
		}
	}

	return true;
}

static bool Cmd_IsOblivionInterior_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
		if (cell->IsOblivionInterior()) {
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_IsOblivionWorld_Execute(COMMAND_ARGS)
{
	TESWorldSpace* world = NULL;
	*result = 0.0;
	
	if (ExtractArgs(PASS_EXTRACT_ARGS, &world) && world) {
		if (world->IsOblivionWorld()) {
			*result = 1.0;
		}
	}

	return true;
}

static bool Cmd_CanFastTravelFromWorld_Execute(COMMAND_ARGS)
{
	TESWorldSpace* world = NULL;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &world) && world && world->CanFastTravel()) {
		*result = 1.0;
	}

	return true;
}

static bool Cmd_IsInvertFastTravel_Execute(COMMAND_ARGS)
{
    TESObjectCELL* cell = NULL;
    *result = 0.0;

    if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
        if (cell->IsInvertFastTravel()) {
            *result = 1.0;
        }
    }

    return true;
}

static bool Cmd_SetInvertFastTravel_Execute(COMMAND_ARGS)
{
    TESObjectCELL* cell = NULL;
    UInt32 act = NULL;
    *result = 0.0;

    if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &act) && cell) {
        cell->SetInvertFastTravel(act ? true : false);
        *result = 1.0;
    }

    return true;
}

static bool Cmd_IsCantWait_Execute(COMMAND_ARGS)
{
    TESObjectCELL* cell = NULL;
    *result = 0.0;

    if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
        if (cell->IsCantWait()) {
            *result = 1.0;
        }
    }

    return true;
}

static bool Cmd_SetCantWait_Execute(COMMAND_ARGS)
{
    TESObjectCELL* cell = NULL;
    UInt32 act = NULL;
    *result = 0.0;

    if (ExtractArgs(PASS_EXTRACT_ARGS, &cell, &act) && cell) {
        cell->SetCantWait(act ? true : false);
        *result = 1.0;
    }

    return true;
}

static bool Cmd_SetCanFastTravelFromWorld_Execute(COMMAND_ARGS)
{
	TESWorldSpace* world = NULL;
	UInt32 bCanTravel = -1;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &world, &bCanTravel) && world) {
		world->SetCanFastTravel(bCanTravel ? true : false);
	}

	return true;
}

static bool Cmd_GetCellWaterType_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	TESObjectCELL* cell = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
		TESWaterForm* water = cell->GetWaterType();
		if (water) {
			*refResult = water->refID;
		}
	}

	return true;
}

static bool Cmd_SetCellWaterType_Execute(COMMAND_ARGS)
{
	TESObjectCELL* cell = NULL;
	TESForm* waterForm = NULL;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &cell, &waterForm) && cell && waterForm) {
		TESWaterForm* water = OBLIVION_CAST(waterForm, TESForm, TESWaterForm);
		if (water) {
			cell->SetWaterType(water);
		}
	}

	return true;
}

static bool Cmd_GetCellNorthRotation_Execute(COMMAND_ARGS)
{
	// negative because ExtraNorthRotation is negative of north marker ref's Z rotation
	// easier for scripts to deal with rotation as defined in editor
	static const double mult = -180.0/3.14159265358979;

	TESObjectCELL* cell = NULL;
	*result = 0.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &cell) && cell) {
		ExtraNorthRotation* rot = (ExtraNorthRotation*)cell->extraData.GetByType(kExtraData_NorthRotation);
		if (rot) {
			// radians -> degrees
			*result = rot->rotation * mult;
		}
	}

	return true;
}

#endif

CommandInfo kCommandInfo_GetCellMusicType =
{
	"GetCellMusicType", "GetMusicType", 0,
	"returns the music type associated with the player's current cell",
	0, 0, NULL,
	HANDLER(Cmd_GetCellMusicType_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_SetWaterHeight[] =
{
	{	"cell",		kParamType_Cell,	0	},
	{	"height",	kParamType_Float,	0	},
};

DEFINE_COMMAND(SetCellWaterHeight, changes the height of water in a cell, 0, 2, kParams_SetWaterHeight);

CommandInfo kCommandInfo_ParentCellHasWater =
{
	"ParentCellHasWater",
	"HasWater",
	0,
	"returns whether the cell has water",
	1,
	0,
	NULL,
	HANDLER(Cmd_CellHasWater_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GetParentCellWaterHeight =
{
	"GetParentCellWaterHeight",
	"GetWaterHeight",
	0,
	"returns the height of the cell's water",
	1,
	0,
	NULL,
	HANDLER(Cmd_GetCellWaterHeight_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneCell[1] =
{
	{	"cell",	kParamType_Cell,	0	},
};

static ParamInfo kParams_OneCellOneInt[2] =
{
	{	"cell",	kParamType_Cell,	0	},
	{	"int",	kParamType_Integer,	0	},
};

DEFINE_COMMAND(GetCellDetachTime, returns the cell detach time, 0, 1, kParams_OneCell);
DEFINE_COMMAND(GetCellResetHours, returns the number of hours until a cell resets, 0, 1, kParams_OneCell);
DEFINE_COMMAND(SetCellResetHours, sets the number of hours until a cell resets, 0, 2, kParams_OneCellOneInt);

DEFINE_CMD_ALT(GetWorldSpaceParentWorldSpace, GetWorldParentWorld, returns a worldspaces parent worldspace, 0, kParams_OneWorldSpace);
DEFINE_COMMAND(GetCellBehavesAsExterior, returns 1 if the cell is flagged as behaving as an exterior, 0, 1, kParams_OneCell);

DEFINE_COMMAND(GetCellLighting, returns the lighting data for an interior cell, 0, 1, kParams_OneCell);

static ParamInfo kOBSEParams_SetCellLighting[2] =
{
	{	"cell", kOBSEParamType_Form, 0 },
	{	"data", kOBSEParamType_Array, 0 },
};

CommandInfo kCommandInfo_SetCellLighting =
{
	"SetCellLighting", "", 0,
	"sets the lighting data for an interior cell",
	0, 2, kOBSEParams_SetCellLighting,
	HANDLER(Cmd_SetCellLighting_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};

static ParamInfo kOBSEParams_SetCellClimate[2] =
{
	{	"cell",		kOBSEParamType_Form,	0	},
	{	"climate",	kOBSEParamType_Form,	0	},
};

DEFINE_COMMAND(GetCellClimate, returns the climate for the interior cell, 0, 1, kParams_OneCell);
DEFINE_COMMAND(SetCellBehavesAsExterior, sets the flag for the cell, 0, 2, kParams_OneCellOneInt);
DEFINE_COMMAND(SetCellHasWater, sets the flag for the cell, 0, 2, kParams_OneCellOneInt);

CommandInfo kCommandInfo_SetCellClimate = {
	"SetCellClimate", "", 0,
	"sets the climate for the interior cell",
	0, 2, kOBSEParams_SetCellClimate,
	HANDLER(Cmd_SetCellClimate_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};

DEFINE_COMMAND(SetCellIsPublic, sets the public flag, 0, 2, kParams_OneCellOneInt);
DEFINE_COMMAND(IsCellPublic, returns true if the cell is public, 0, 1, kParams_OneCell);

static ParamInfo kParams_GetTerrainHeight[2] =
{
	{	"x",	kParamType_Float,	0	},
	{	"y",	kParamType_Float,	0	},
};

DEFINE_COMMAND(GetTerrainHeight, returns height of terrain at specified x and y position, 0, 2, kParams_GetTerrainHeight);

DEFINE_COMMAND(IsOblivionInterior, returns 1 if the cell is an oblivion interior, 0, 1, kParams_OneCell);
DEFINE_COMMAND(IsOblivionWorld, returns 1 if the worldspace is in Oblivion, 0, 1, kParams_OneWorldSpace);
DEFINE_COMMAND(CanFastTravelFromWorld, returns 1 if the player can fast travel from the world space, 0, 1, kParams_OneWorldSpace);

static ParamInfo kParams_OneWorld_OneInt[2] =
{
	{	"worldspace",	kParamType_WorldSpace,	0	},
	{	"int",			kParamType_Integer,		0	},
};

DEFINE_COMMAND(SetCanFastTravelFromWorld, enables or disables fast travel from the world space, 0, 2, kParams_OneWorld_OneInt);

DEFINE_COMMAND(GetCellWaterType, returns the cells water type, 0, 1, kParams_OneCell);

static ParamInfo kParams_OneCell_OneInventoryObject[2] =
{
	{ "cell",	kParamType_Cell,			0	},
	{ "form",	kParamType_InventoryObject,	0	},
};

DEFINE_COMMAND(SetCellWaterType, sets the cell water type, 0, 2, kParams_OneCell_OneInventoryObject);
DEFINE_COMMAND(GetCellNorthRotation, returns the rotation of the north marker for the specified cell, 0, 1, kParams_OneCell);

DEFINE_COMMAND(SetCellMusicType, sets the music type for a cell, 0, 2, kParams_OneCellOneInt);

DEFINE_COMMAND(SetInvertFastTravel, sets the InvertFastTravel flag, 0, 2, kParams_OneCellOneInt);
DEFINE_COMMAND(IsInvertFastTravel, returns true if the cell has InvertFastTravel flag, 0, 1, kParams_OneCell);

DEFINE_COMMAND(SetCantWait, sets the CantWait flag, 0, 2, kParams_OneCellOneInt);
DEFINE_COMMAND(IsCantWait, returns true if the cell has CantWait flag, 0, 1, kParams_OneCell);
