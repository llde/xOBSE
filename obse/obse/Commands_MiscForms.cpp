#include "Commands_MiscForms.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION
#include "GameObjects.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "StringVar.h"
#include "Hooks_Gameplay.h"
#include "ScriptUtils.h"
#include "ArrayVar.h"

// game remembers the last TESDescription retrieved from disk and caches its text.
static TESDescription** s_cachedTESDescription = (TESDescription**)0x00B33C04;

enum IngredientMode {
	eMode_Get,
	eMode_Set,
	eMode_GetChance,
	eMode_SetChance
};

static bool IngredientCommand_Execute(COMMAND_ARGS, UInt32 mode)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	TESForm* floraForm = NULL;
	TESForm* ingredientForm = NULL;
	UInt32 whichSeason = 0;
	UInt32 newChance = 0;

	bool bExtracted = false;
	switch (mode)
	{
	case eMode_Get:
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &floraForm);
		break;
	case eMode_Set:
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &ingredientForm, &floraForm);
		break;
	case eMode_GetChance:
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &whichSeason, &floraForm);
		break;
	case eMode_SetChance:
		bExtracted = ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &whichSeason, &newChance, &floraForm);
		break;
	}

	if (!bExtracted)
		return true;
	else if (!floraForm)
	{
		if (thisObj)
			floraForm = thisObj->TryGetREFRParent();
		else
			return true;
	}
	
	TESProduceForm* produceForm = (TESProduceForm*)Oblivion_DynamicCast(floraForm, 0, RTTI_TESForm, RTTI_TESProduceForm, 0);
	IngredientItem* ingredient = NULL;
	if (ingredientForm)
		ingredient = (IngredientItem*)Oblivion_DynamicCast(ingredientForm, 0, RTTI_TESForm, RTTI_IngredientItem, 0);

	if (!produceForm)
		return true;

	switch (mode)
	{
	case eMode_Get:
		if (produceForm->ingredient)
			*refResult = produceForm->ingredient->refID;
		break;
	case eMode_Set:
		produceForm->ingredient = ingredient;
		break;
	case eMode_GetChance:
		if (whichSeason < 4)
			*result = produceForm->harvestChance[whichSeason];
		break;
	case eMode_SetChance:
		if (whichSeason < 4 && newChance <= 100)
			produceForm->harvestChance[whichSeason] = newChance;
		break;
	}

	return true;
}

static bool Cmd_GetIngredient_Execute(COMMAND_ARGS)
{
	return IngredientCommand_Execute(PASS_COMMAND_ARGS, eMode_Get);
}

static bool Cmd_SetIngredient_Execute(COMMAND_ARGS)
{
	return IngredientCommand_Execute(PASS_COMMAND_ARGS, eMode_Set);
}

static bool Cmd_GetIngredientChance_Execute(COMMAND_ARGS)
{
	return IngredientCommand_Execute(PASS_COMMAND_ARGS, eMode_GetChance);
}

static bool Cmd_SetIngredientChance_Execute(COMMAND_ARGS)
{
	return IngredientCommand_Execute(PASS_COMMAND_ARGS, eMode_SetChance);
}

static const UInt32 kGetText_arg1 = 0x43534544;

static bool Cmd_GetBookText_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	const char* text = NULL;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &form))
	{
		//_MESSAGE("Args extracted");
		if (!form)
			form = thisObj->baseForm;

		TESObjectBOOK* book = (TESObjectBOOK*)Oblivion_DynamicCast(form, 0, RTTI_TESForm, RTTI_TESObjectBOOK, 0);
		if (book)
		{
			//_MESSAGE("Got a book");
			text = book->description.GetText(0, kGetText_arg1);
			//_MESSAGE("called gettext");
		}
		else {
			//_MESSAGE("Couldn't geta book");
		}
	}	

	if (!text)
	{
		//_MESSAGE("Couldn't read book text");
		//if (form) {
		//	_MESSAGE("Book = %08x", form->refID);

		text = "";
	}
	else {
		//_MESSAGE("%s", text);
	}
	AssignToStringVar(PASS_COMMAND_ARGS, text);
	
	return true;
}

static bool Cmd_GetEditorID_Execute(COMMAND_ARGS)
{
	// if editorID not available, use formID. if no form, use a bunch of zeroes
	char buf[0x200] = "00000000";
	const char* idStr = buf;

	TESForm* form = NULL;
	UInt32 bNoFormID = 0;	// pass 1 to prevent it returning the formID if no string editorID exists
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form, &bNoFormID) && form)
	{
		const char* edID = form->GetEditorID();
		if (edID)
			idStr = edID;
		else if (bNoFormID)
			idStr = "";
		else
			sprintf_s(buf, sizeof(buf), "%08X", form->refID);
	}

	AssignToStringVar(PASS_COMMAND_ARGS, idStr);
	return true;
}

static bool Cmd_GetEditorID2_Execute(COMMAND_ARGS)
{
	// if editorID not available, return empty string
	const char* idStr = "";

	TESForm* form = NULL;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form) && form)
	{
		const char* edID = form->GetEditorID2();
		if (edID)
			idStr = edID;
	}

	AssignToStringVar(PASS_COMMAND_ARGS, idStr);
	return true;
}


static bool Cmd_MatchPotion_Execute(COMMAND_ARGS)
{
	TESForm* potionForm = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &potionForm) && potionForm)
	{
		AlchemyItem* alch = OBLIVION_CAST(potionForm, TESForm, AlchemyItem);
		if (alch)
		{
			alch = MatchPotion(alch);
			if (alch)
				*refResult = alch->refID;
		}
	}

	return true;
}

enum {
	kDescription_Get,
	kDescription_Set,
};

static bool DescriptionFunc_Execute(COMMAND_ARGS, UInt32 mode)
{
	// for skills an optional second param (0-3) indicates which skill level description to return; if omitted use base TESSkill description
	UInt32 minArgs = (mode == kDescription_Set) ? 1 : 0;

	TESDescription* desc = NULL;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs())
	{
		switch (eval.NumArgs() - minArgs) {
			case 0:
				if (thisObj)
					desc = OBLIVION_CAST(thisObj->baseForm, TESForm, TESDescription);
				break;
			case 1:
				desc = OBLIVION_CAST(eval.Arg(minArgs)->GetTESForm(), TESForm, TESDescription);
				break;
			case 2:
			{
				TESSkill* skill = OBLIVION_CAST(eval.Arg(minArgs)->GetTESForm(), TESForm, TESSkill);
				if (skill) {
					UInt32 idx = eval.Arg(minArgs+1)->GetNumber();
					if (idx < 4) {
						if (mode == kDescription_Get && !IsDescriptionModified(&skill->levelQuote[idx])) {
							// oddball, handle here rather than below
							AssignToStringVar(PASS_COMMAND_ARGS, skill->GetLevelQuoteText(idx));
							return true;
						}
						else {
							desc = &skill->levelQuote[idx];
						}
					}
				}
			}
			break;
		}

		if (mode == kDescription_Get) {
			const char* descText = desc ? desc->GetDescription() : "";
			AssignToStringVar(PASS_COMMAND_ARGS, descText);
		}
		else if (mode == kDescription_Set && desc && eval.Arg(0)->CanConvertTo(kTokenType_String)) {
			const char* nuText = eval.Arg(0)->GetString();
			*result = SetDescriptionText(desc, nuText) ? 1.0 : 0.0;
		}
	}
	
	return true;
}

static bool Cmd_GetDescription_Execute(COMMAND_ARGS)
{
	// prevent it from using cache
	*s_cachedTESDescription = NULL;
	return DescriptionFunc_Execute(PASS_COMMAND_ARGS, kDescription_Get);
}		

static bool Cmd_SetDescription_Execute(COMMAND_ARGS)
{
	return DescriptionFunc_Execute(PASS_COMMAND_ARGS, kDescription_Set);
}

static bool Cmd_GetBookLength_Execute(COMMAND_ARGS)
{
	TESForm* bookForm = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &bookForm)) {
		if (!bookForm && thisObj) {
			bookForm = thisObj->baseForm;
		}

		TESObjectBOOK* book = OBLIVION_CAST(bookForm, TESForm, TESObjectBOOK);
		if (book) {
			const char* text = book->description.GetDescription();
			if (text) {
				*result = strlen(text);
			}
		}
	}

	return true;
}

enum {
	kSigilStone_GetUses,
	kSigilStone_SetUses,
	kSigilStone_ModUses
};

static bool SigilStoneFunc_Execute(COMMAND_ARGS, UInt32 mode)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32 intVal = 0;

	bool bExtracted = false;
	switch (mode) {
		case kSigilStone_GetUses:
			bExtracted = ExtractArgs(PASS_EXTRACT_ARGS, &form);
			break;
		case kSigilStone_SetUses:
		case kSigilStone_ModUses:
			bExtracted = ExtractArgs(PASS_EXTRACT_ARGS, &intVal, &form);
			break;
	}

	if (bExtracted) {
		if (!form && thisObj)
			form = thisObj->baseForm;
		TESSigilStone* stone = OBLIVION_CAST(form, TESForm, TESSigilStone);
		if (stone) {
			switch (mode) {
				case kSigilStone_GetUses:
					*result = stone->uses.uses;
					break;
				case kSigilStone_ModUses:
					intVal += stone->uses.uses;
					if (intVal < 0)
						break;
					// fall-through intentional
				case kSigilStone_SetUses:
					stone->uses.uses = intVal;
					*result = 1;	// success
					break;
			}
		}
	}

	return true;
}

static bool Cmd_GetSigilStoneUses_Execute(COMMAND_ARGS)
{
	return SigilStoneFunc_Execute(PASS_COMMAND_ARGS, kSigilStone_GetUses);
}

static bool Cmd_SetSigilStoneUses_Execute(COMMAND_ARGS)
{
	return SigilStoneFunc_Execute(PASS_COMMAND_ARGS, kSigilStone_SetUses);
}

static bool Cmd_ModSigilStoneUses_Execute(COMMAND_ARGS)
{
	return SigilStoneFunc_Execute(PASS_COMMAND_ARGS, kSigilStone_ModUses);
}

static bool Cmd_GetLightRGB_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* lightForm = NULL;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &lightForm)) {
		if (!lightForm && thisObj) {
			lightForm = thisObj->baseForm;
		}

		TESObjectLIGH* light = OBLIVION_CAST(lightForm, TESForm, TESObjectLIGH);
		if (light) {
			ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
			g_ArrayMap.SetElementNumber(arr, 0.0, light->colorRGB.r);
			g_ArrayMap.SetElementNumber(arr, 1.0, light->colorRGB.g);
			g_ArrayMap.SetElementNumber(arr, 2.0, light->colorRGB.b);
			*result = arr;
		}
	}

	return true;
}

static bool Cmd_SetLightRGB_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0) {
		TESObjectLIGH* light = NULL;
		if (eval.NumArgs() == 2) {			
			light = OBLIVION_CAST(eval.Arg(1)->GetTESForm(), TESForm, TESObjectLIGH);
		}
		else if (thisObj) {
			light = OBLIVION_CAST(thisObj->baseForm, TESForm, TESObjectLIGH);
		}

		if (light) {
			ArrayID arr = eval.Arg(0)->GetArray();
			if (arr && g_ArrayMap.SizeOf(arr) == 3) {
				double red;
				double green;
				double blue;

				if (g_ArrayMap.GetElementNumber(arr, 0.0, &red) && g_ArrayMap.GetElementNumber(arr, 1.0, &green) &&
					g_ArrayMap.GetElementNumber(arr, 2.0, &blue))
				{
					light->colorRGB.Set(red, green, blue);
					*result = 1;
				}
			}
		}
	}

	return true;
}
				
static bool Cmd_GetEditorSize_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = -1.0;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &form)) {
		if (!form && thisObj) {
			form = thisObj->baseForm;
		}

		TESModel* model = OBLIVION_CAST(form, TESForm, TESModel);
		if (!model) {
			TESBipedModelForm* bi = OBLIVION_CAST(form, TESForm, TESBipedModelForm);
			if (bi) {
				model = bi->groundModel;
			}
		}

		if (model) {
			*result = model->editorSize;
		}
	}

	return true;
}
TESObjectLIGH* GetLightArg(TESObjectREFR* refr, TESForm* base)
{
	if (!base && refr) {
		base = refr->baseForm;
	}

	return OBLIVION_CAST(base, TESForm, TESObjectLIGH);
}

static bool Cmd_GetLightDuration_Execute(COMMAND_ARGS)
{
	TESForm* obj = NULL;
	*result = 0.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &obj)) {
		TESObjectLIGH* light = GetLightArg(thisObj, obj);
		if (light) {
			*result = light->time;
		}
	}

	if (IsConsoleMode()) {
		Console_Print("GetLightDuration >> %.2f", *result);
	}

	return true;
}

static bool Cmd_SetLightDuration_Execute(COMMAND_ARGS)
{
	UInt32 duration = 0;
	TESForm* obj = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &duration, &obj)) {
		TESObjectLIGH* light = GetLightArg(thisObj, obj);
		if (light) {
			light->time = duration;
		}
	}

	return true;
}

static bool GetDoorFlag_Execute(COMMAND_ARGS, UInt32 flag)
{
	TESObject* obj = NULL;
	*result = 0.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &obj)) {
		if (!obj && thisObj) {
			obj = OBLIVION_CAST(thisObj->baseForm, TESForm, TESObject);
		}

		TESObjectDOOR* door = OBLIVION_CAST(obj, TESObject, TESObjectDOOR);
		if (door && (door->doorFlags & flag)) {
			*result = 1.0;
		}
	}

	if (IsConsoleMode()) {
		Console_Print("Door flag >> %.0f", *result);
	}

	return true;
}

static bool SetDoorFlag_Execute(COMMAND_ARGS, UInt32 flag)
{
	TESObject* obj = NULL;
	UInt32 bSet = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &bSet, &obj)) {
		if (!obj && thisObj) {
			obj = OBLIVION_CAST(thisObj->baseForm, TESForm, TESObject);
		}

		TESObjectDOOR* door = OBLIVION_CAST(obj, TESObject, TESObjectDOOR);
		if (door) {
			if (bSet) {
				door->doorFlags |= flag;
			}
			else {
				door->doorFlags &= ~flag;
			}
		}
	}

	return true;
}

static bool Cmd_IsOblivionGate_Execute(COMMAND_ARGS)
{
	return GetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_OblivionGate);
}

static bool Cmd_SetIsOblivionGate_Execute(COMMAND_ARGS)
{
	return SetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_OblivionGate);
}

static bool Cmd_IsAutomaticDoor_Execute(COMMAND_ARGS)
{
	return GetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_Automatic);
}

static bool Cmd_SetIsAutomaticDoor_Execute(COMMAND_ARGS)
{
	return SetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_Automatic);
}

static bool Cmd_IsMinimalUseDoor_Execute(COMMAND_ARGS)
{
	return GetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_MinimalUse);
}

static bool Cmd_SetIsMinimalUseDoor_Execute(COMMAND_ARGS)
{
	return SetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_MinimalUse);
}

static bool Cmd_IsHiddenDoor_Execute(COMMAND_ARGS)
{
	return GetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_Hidden);
}

static bool Cmd_SetIsHiddenDoor_Execute(COMMAND_ARGS)
{
	return SetDoorFlag_Execute(PASS_COMMAND_ARGS, TESObjectDOOR::kDoorFlag_Hidden);
}

static bool Cmd_DeleteClonedForm(COMMAND_ARGS) {
	*result = 0;
	TESForm* form = nullptr;
	if (!ExtractArgs(PASS_EXTRACT_ARGS, &form)) return true;
	if (!form || !form->IsCloned()) return true;
	//TODO Check 0x0045C7A0 (refstuff)
	form->Destroy(true);
	*result = 1;
	return true;
}

#endif

static ParamInfo kOBSEParams_OneForm[1] =
{
	{	"object",	kOBSEParamType_Form,	0	},
};


static ParamInfo kParams_GetIngredient[1] =
{
	{	"Flora",	kParamType_InventoryObject,		1	},
};

// 1st param is optional to allow setting ingredient to NULL
static ParamInfo kParams_SetIngredient[2] =
{
	{	"Ingredient",	kParamType_InventoryObject,		1	},		
	{	"Flora",		kParamType_InventoryObject,		1	},
};

static ParamInfo kParams_GetIngredientChance[2] =
{
	{	"Season",		kParamType_Integer,				0	},
	{	"Flora",		kParamType_InventoryObject,		1	},
};

static ParamInfo kParams_SetIngredientChance[3] =
{
	{	"Season",		kParamType_Integer,				0	},
	{	"New Chance",	kParamType_Integer,				0	},
	{	"Flora",		kParamType_InventoryObject,		1	},
};

DEFINE_COMMAND(GetIngredient,
			   returns the ingredient contained by a TESFlora object,
			   0,
			   1,
			   kParams_GetIngredient);

DEFINE_COMMAND(SetIngredient,
			   sets the ingredient contained by a TESFlora object,
			   0,
			   2,
			   kParams_SetIngredient);

DEFINE_COMMAND(GetIngredientChance,
			   returns the seasonal chance of harvesting an ingredient,
			   0,
			   2,
			   kParams_GetIngredientChance);

DEFINE_COMMAND(SetIngredientChance,
			   sets the seasonal chance of harvesting an ingredient,
			   0,
			   3,
			   kParams_SetIngredientChance);

DEFINE_COMMAND(GetBookText,
			   returns the text of a book,
			   0,
			   1,
			   kParams_OneOptionalInventoryObject);

static ParamInfo kParams_GetEditorID[2] =
{
	{ "form",		kParamType_InventoryObject, 0 }, 
	{ "bNoFormID",	kParamType_Integer,			1 },
};

DEFINE_COMMAND_DEPRECATED(GetEditorID, returns the editorID of a form if possible, 0, 2, kParams_GetEditorID);

DEFINE_COMMAND(GetEditorID2, returns the editorID of a form if possible, 0, 1, kParams_OneInventoryObject);

DEFINE_COMMAND(MatchPotion, returns a potion matching the effects of the passed potion if any, 0, 1, kParams_OneInventoryObject);

static ParamInfo kOBSEParams_GetDescription[2] =
{
	{	"FormOrSkill",	kOBSEParamType_FormOrNumber,	1	},
	{	"SkillLevel",	kOBSEParamType_Number,			1	},
};

static ParamInfo kOBSEParams_SetDescription[3] = 
{
	{	"text",			kOBSEParamType_String,			0	},
	{	"FormOrSkill",	kOBSEParamType_FormOrNumber,	1	},
	{	"SkillLevel",	kOBSEParamType_Number,			1	},
};

CommandInfo kCommandInfo_GetDescription =
{
	"GetDescription",
	"",
	0,
	"returns the description text for a form or skill level",
	0,
	2,
	kOBSEParams_GetDescription,
	HANDLER(Cmd_GetDescription_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_SetDescription =
{
	"SetDescription", "", 0,
	"modifies the text of a TESDescription (e.g. book, class, skill, skill level, etc)",
	0, 3, kOBSEParams_SetDescription,
	HANDLER(Cmd_SetDescription_Execute),
	Cmd_Expression_Parse, NULL, 0
};

DEFINE_COMMAND(GetSigilStoneUses, returns the number of uses for a sigil stone, 0, 1, kParams_OneOptionalInventoryObject);
DEFINE_COMMAND(SetSigilStoneUses, sets the number of uses for a sigil stone, 0, 2, kParams_OneInt_OneOptionalInventoryObject);
DEFINE_COMMAND(ModSigilStoneUses, mods the numer of uses for a sigil stone, 0, 2, kParams_OneInt_OneOptionalInventoryObject);
DEFINE_COMMAND(GetBookLength, returns the length of the text of a book, 0, 1, kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetLightRGB, returns the RGB color value of a light as an array, 0, 1, kParams_OneOptionalInventoryObject);

static ParamInfo kParams_SetLightRGB[2] =
{
	{	"RGB",	kOBSEParamType_Array,	0	},
	{	"light",kOBSEParamType_Form,	1	},
};

CommandInfo kCommandInfo_SetLightRGB =
{
	"SetLightRGB", "", 0,
	"sets the RGB color value of a light",
	0, 2, kParams_SetLightRGB,
	HANDLER(Cmd_SetLightRGB_Execute),
	Cmd_Expression_Parse, NULL, 0
};

DEFINE_COMMAND(GetEditorSize, returns the editor size of a bound object, 0, 1, kParams_OneOptionalInventoryObject);

DEFINE_COMMAND(GetLightDuration, returns the duration of the light, 0, 1, kParams_OneOptionalInventoryObject);
DEFINE_COMMAND(SetLightDuration, sets the duration of the light, 0, 2, kParams_OneInt_OneOptionalInventoryObject);

static ParamInfo kParams_OneOptionalObject[1] =
{
	{ "object", kParamType_TESObject, 1 }
};

static ParamInfo kParams_OneInt_OneOptionalObject[2] =
{
	{ "int",	kParamType_Integer,		0	},
	{ "object",	kParamType_TESObject,	1	}
};

DEFINE_COMMAND(IsOblivionGate, returns 1 if the door is an oblivion gate, 0, 1, kParams_OneOptionalObject);
DEFINE_COMMAND(IsAutomaticDoor, returns 1 if the door is automatic, 0, 1, kParams_OneOptionalObject);
DEFINE_COMMAND(IsHiddenDoor, returns 1 if the door is hidden, 0, 1, kParams_OneOptionalObject);
DEFINE_COMMAND(IsMinimalUseDoor, returns 1 if the door is minimal use, 0, 1, kParams_OneOptionalObject);

DEFINE_COMMAND(SetIsOblivionGate, sets the oblivion gate flag, 0, 2, kParams_OneInt_OneOptionalObject);
DEFINE_COMMAND(SetIsAutomaticDoor, sets the automatic flag, 0, 2, kParams_OneInt_OneOptionalObject);
DEFINE_COMMAND(SetIsHiddenDoor, sets the hidden flag, 0, 2, kParams_OneInt_OneOptionalObject);
DEFINE_COMMAND(SetIsMinimalUseDoor, sets the minimal use flag, 0, 2, kParams_OneInt_OneOptionalObject);