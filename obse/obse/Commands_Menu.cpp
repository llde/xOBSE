#include "Commands_Menu.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameObjects.h"
#include "GameTiles.h"
#include "GameMenus.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "StringVar.h"
#include "Hooks_Gameplay.h"
#include "GameData.h"

typedef void (* _CloseAllMenus)(void);

static _CloseAllMenus CloseAllMenus = (_CloseAllMenus)0x00579770;

const _ShowMessageBox_Callback ContainerMenuCallback =	(_ShowMessageBox_Callback)0x00597B10;
const _ShowMessageBox_Callback SpellPurchaseCallback =	(_ShowMessageBox_Callback)0x005D9730;
const _ShowMessageBox_Callback PoisonConfirmCallback =	(_ShowMessageBox_Callback)0x00666890;
const _ShowMessageBox_Callback OverwriteSaveGameCallback = (_ShowMessageBox_Callback)0x005D3390;
const _ShowMessageBox_Callback LoadGameCallback =		(_ShowMessageBox_Callback)0x005AE190;
const _ShowMessageBox_Callback MissingContentCallback = (_ShowMessageBox_Callback)0x00578DC0;

static bool Cmd_GetActiveMenuMode_Execute(COMMAND_ARGS)
{
	*result = 0;
	InterfaceManager* intfc = InterfaceManager::GetSingleton();
	if (!intfc) return true;
	if (intfc->activeMenu) {
		*result = intfc->activeMenu->id;
		return true;
	}
	//Try to use the tile containing menu, as activeMenu is null using keyboard
	Tile* activeTile = intfc->activeTile ? intfc->activeTile : intfc->altActiveTile;
	if (activeTile)
		*result = activeTile->GetContainingMenu()->id;

	return true;
}

enum eMenuValue {
	//no params
	kMenu_Soulgem,
	kMenu_EnchantItem,
	kMenu_Barter,
	kMenu_ContainerView,

	//one optional int param
	kMenu_Selection,
	kMenu_Object,
	kMenu_Ref,
	kMenu_Filter,

	//one int param
	kMenu_Ingredient,
	kMenu_IngredientCount,
	kMenu_Apparatus
};

union MenuInfo {
	const TESForm* form;
	UInt32	 integer;
};

static bool GetActiveMenuElement(COMMAND_ARGS, eMenuValue whichValue, MenuInfo* out, UInt32 whichMenu = 0)
{
	InterfaceManager* intfc = InterfaceManager::GetSingleton();
	//activeTile is used when using mouse selection, is NULL when using keyboard. altActiveTile contain the keyboard selection, is NULL using mouse 
	Tile* activeTile = intfc->activeTile ? intfc->activeTile : intfc->altActiveTile;
	//ActiveMenu is NULL when using keyboard, get the menu form the tile
	Menu* activeMenu = intfc->activeMenu ? intfc->activeMenu : activeTile->GetContainingMenu();

	bool gotValue = false;
	UInt32 intArg = -1;

	//Extract arguments
	if (whichValue < kMenu_Ingredient && whichValue >= kMenu_Selection)	//optional int param specifies menu type
	{
		ExtractArgs(PASS_EXTRACT_ARGS, &intArg);
		if (intArg != -1)
			activeMenu = GetMenuByType(intArg);
	}
	else if (whichValue >= kMenu_Ingredient)
	{
		ExtractArgs(PASS_EXTRACT_ARGS, &intArg);
		if (intArg == -1)
			return false;
	}

	if (whichMenu)	//specific menu, so look it up directly
		activeMenu = GetMenuByType(whichMenu);

	if (!activeMenu)
		return false;

	//get element based on menu type
	switch (activeMenu->id)
	{
	case kMenuType_Message:
		{
			MessageMenu* msgMenu = (MessageMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Object:
				if (ShowMessageBox_pScriptRefID && msgMenu->IsScriptMessageBox())
				{
					out->form = LookupFormByID(*ShowMessageBox_pScriptRefID);
					gotValue = true;
				}
				else
					out->form = 0;
			default:
				break;
			}
		}
		break;
	case kMenuType_Alchemy:
		{
			AlchemyMenu* menu = (AlchemyMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Object:
				out->form = menu->potion;
				gotValue = true;
				break;
			case kMenu_Ingredient:
				out->form = menu->GetIngredientItem(intArg);
				gotValue = true;
				break;
			case kMenu_IngredientCount:
				out->integer = menu->GetIngredientCount(intArg);
				gotValue = true;
				break;
			case kMenu_Apparatus:
				out->form = menu->GetApparatus(intArg);
				gotValue = true;
				break;
			default:
				break;
			}
		}
		break;
	case kMenuType_Container:
		{
			ContainerMenu* menu = (ContainerMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_ContainerView:
				if (menu->isContainerContents)
					out->integer = 0;
				else
					out->integer = 1;
				gotValue = true;
				break;
			case kMenu_Ref:
				{
					out->form = menu->refr;
					gotValue = true;
					break;
				}
			case kMenu_Filter:
				out->integer = menu->filterType;
				gotValue = true;
				break;
			case kMenu_Selection:
				{
					if (activeTile)
					{
						float fIndex;
						if (activeTile->GetFloatValue(kTileValue_user11, &fIndex))
						{
							UInt32 index = fIndex;
							if (menu->isContainerContents)
								out->form = menu->refr->GetInventoryItem(index, menu->isBarter);
							else
								out->form = (*g_thePlayer)->GetInventoryItem(index, 0);
							gotValue = true;
						}
					}
				}
				break;
			case kMenu_Barter:
				out->integer = menu->isBarter;
				gotValue = true;
				break;
			default:
				break;
			}
			break;
		}
	case kMenuType_Magic:
		{
			MagicMenu* menu = (MagicMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Selection:
				{
					if (activeTile && menu->filterType != MagicMenu::kFilter_ActiveEffects)
					{
						float fIndex;
						if (activeTile->GetFloatValue(kTileValue_user13, &fIndex)) {
							DEBUG_PRINT("Index of active magic item: %.0f", fIndex);
							UInt32 index = fIndex;
							float fObjType;
							if (activeTile->GetFloatValue(kTileValue_user7, &fObjType)) {
								if (fObjType == 8)	{	// a scroll
									TESForm* form = menu->GetMagicItemForIndex(index);
									if (form) {
										out->form = form;
										gotValue = true;
									}
								}
								else {	// a spell
									out->form = MenuSpellListVisitor(&menu->spells).GetNthInfo(index-1);
									gotValue = true;
								}
							}
						}
					}
				}
				break;
			case kMenu_Filter:
				out->integer = menu->filterType;
				gotValue = true;
				break;
			default:
				break;
			}
			break;
		}
	case kMenuType_SpellPurchase:
		{
			SpellPurchaseMenu* menu = (SpellPurchaseMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Selection:
				{
					if (activeTile)
					{
						float fIndex;
						//if (activeTile->GetFloatValue(kTileValue_user11, &fIndex))
						if (activeTile->GetFloatValue(kTileValue_user0, &fIndex))
						{
							UInt32 index = fIndex;
							out->form = MenuSpellListVisitor(&menu->spells).GetNthInfo(index);
							gotValue = true;
						}
					}
					break;
				}
			case kMenu_Ref:
				{
					out->form = menu->spellMerchant;
					gotValue = true;
					break;
				}
			default:
				break;
			}
		}
		break;
	case kMenuType_Enchantment:
		{
			EnchantmentMenu* menu = (EnchantmentMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Soulgem:
				if (menu->soulGemInfo)
				{
					out->form = menu->soulGemInfo->form;
					gotValue = true;
				}
				break;
			case kMenu_EnchantItem:
				out->form = menu->enchantItem;
				gotValue = true;
				break;
			default:
				break;
			}
		}
		break;
	case kMenuType_Book:
		{
			BookMenu* menu = (BookMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Ref:
				out->form = menu->bookRef;
				gotValue = true;
				break;
			case kMenu_Object:
				out->form = menu->book;
				gotValue = true;
				break;
			default:
				break;
			}
		}
		break;
	case kMenuType_Inventory:
		{
			InventoryMenu* menu = (InventoryMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Selection:
				{
					if (activeTile)
					{
						float fIndex;
						if (activeTile->GetFloatValue(kTileValue_user11, &fIndex))
						{
							UInt32 index = fIndex;
							out->form = (*g_thePlayer)->GetInventoryItem(index, 0);
							gotValue = true;
						}
					}
				}
				break;
			case kMenu_Filter:
				out->integer = menu->filterType;
				gotValue = true;
				break;
			default:
				break;
			}
		}
		break;
	case kMenuType_Dialog:
		{
			DialogMenu* menu = (DialogMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Ref:
				out->form = menu->speaker;
				gotValue = true;
				break;
			default:
				break;
			}
		}
		break;
	case kMenuType_Recharge:
		{
			RechargeMenu* menu = (RechargeMenu*)activeMenu;
			switch (whichValue)
			{
			case kMenu_Ref:
				out->form = menu->recharger;
				gotValue = true;
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return gotValue;
}

static bool Cmd_GetEnchMenuEnchItem_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_EnchantItem, &info, kMenuType_Enchantment) && info.form)
		*refResult = info.form->refID;

	return true;
}

static bool Cmd_GetEnchMenuSoulgem_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Soulgem, &info, kMenuType_Enchantment) && info.form)
		*refResult = info.form->refID;

	return true;
}

static bool Cmd_GetActiveMenuSelection_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Selection, &info) && info.form)
	{
		*refResult = info.form->refID;
		if (IsConsoleMode())
			Console_Print("GetActiveMenuSelection >> %s (%08x)", GetFullName(const_cast<TESForm*>(info.form)), info.form->refID);
	}

	return true;
}

static bool Cmd_GetActiveMenuObject_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Object, &info) && info.form)
		*refResult = info.form->refID;

	return true;
}

static bool Cmd_GetActiveMenuRef_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Ref, &info) && info.form)
		*refResult = info.form->refID;

	return true;
}

static bool Cmd_GetActiveMenuFilter_Execute(COMMAND_ARGS)
{
	*result = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Filter, &info))
		*result = info.integer;

	return true;
}

static bool Cmd_IsBarterMenuActive_Execute(COMMAND_ARGS)
{
	*result = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Barter, &info))
		*result = (info.integer) ? 1 : 0;

	return true;
}

static bool Cmd_GetAlchMenuIngredient_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Ingredient, &info, kMenuType_Alchemy) && info.form)
		*refResult = info.form->refID;

	return true;
}

static bool Cmd_GetAlchMenuIngredientCount_Execute(COMMAND_ARGS)
{
	*result = -1;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_IngredientCount, &info, kMenuType_Alchemy))
		*result = info.integer;

	return true;
}

static bool Cmd_GetAlchMenuApparatus_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_Apparatus, &info, kMenuType_Alchemy) && info.form)
		*refResult = info.form->refID;

	return true;
}

/*	Need to figure out what BaseExtraList* and unk4 do in ApparatusInfo struct first
static bool Cmd_SetAlchMenuApparatus_Execute(COMMAND_ARGS)
{
	UInt32 whichAppa = 0;
	TESForm* newAppa = NULL;
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &whichAppa, &newAppa))
	{
		Menu* menu = GetMenuByType(kMenuType_Alchemy);
		if (alchMenu)
		{
			AlchemyMenu* alchMenu = (AlchemyMenu*)menu;
			alchMenu->
*/

static bool Cmd_CloseAllMenus_Execute(COMMAND_ARGS)
{
	CloseAllMenus();
	return true;
}

static bool Cmd_GetContainerMenuView_Execute(COMMAND_ARGS)
{
	*result = -1;

	MenuInfo info;
	if (GetActiveMenuElement(PASS_COMMAND_ARGS, kMenu_ContainerView, &info, kMenuType_Container))
		*result = info.integer;

	return true;
}

enum MenuValueType{
	kGetFloat,
	kSetFloat,
	kGetString,
	kSetString,
	kExists
};

static bool GetSetMenuValue_Execute(COMMAND_ARGS, UInt32 mode)
{
	const char* separatorChar = GetSeparatorChars(scriptObj);
	char stringArg[kMaxMessageLength] = { 0 };
	UInt32 menuType = 0;
	float newFloatVal = 0;
	char* newStringVal = NULL;

	char* componentPath = stringArg;

	bool bExtracted = false;
	switch (mode)
	{
	case kGetFloat:
	case kGetString:
	case kExists:
		bExtracted = ExtractFormatStringArgs(0, stringArg, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_GetMenuFloatValue.numParams, &menuType);
		break;
	case kSetFloat:
		bExtracted = ExtractFormatStringArgs(0, stringArg, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_SetMenuFloatValue.numParams, &menuType, &newFloatVal);
		break;
	case kSetString:
		{
			bExtracted = ExtractFormatStringArgs(0, stringArg, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_GetMenuFloatValue.numParams, &menuType);
			// extract new value from format string
			char* context = NULL;
			componentPath = strtok_s(stringArg, separatorChar, &context);
			newStringVal = strtok_s(NULL, separatorChar, &context);
			bExtracted = (bExtracted && componentPath && newStringVal);
		}
		break;
	}

	const char* strToAssign = "";

	if (bExtracted)
	{
		Menu* menu = GetMenuByType(menuType);
		if (menu && menu->tile)
		{
			Tile::Value* val = menu->tile->GetValueByName(componentPath);
			if (val)
			{
#if _DEBUG && 0
				val->DumpExpressionList();
#endif
				switch (mode)
				{
				case kExists:
					*result = 1;
					break;
				case kGetFloat:
					if (val->IsNum())
						*result = val->num;
					break;
				case kSetFloat:
					val->bIsNum = 1;
					val->parentTile->UpdateFloat(val->id, newFloatVal);
					break;
				case kGetString:
					if (val->IsString())
						strToAssign = val->str.m_data;
					break;
				case kSetString:
					val->bIsNum = 0;
					val->parentTile->UpdateString(val->id, newStringVal);
					val->parentTile->SetStringValue(val->id, newStringVal);
					val->parentTile->UpdateField(val->id, 0, newStringVal);

					break;
				}
			}
		}
	}

	if (mode == kGetString)		// need to assign even if errors occur during arg extraction/value retrieval
		AssignToStringVar(PASS_COMMAND_ARGS, strToAssign);

	return true;
}

static bool Cmd_GetMenuFloatValue_Execute(COMMAND_ARGS)
{
	return GetSetMenuValue_Execute(PASS_COMMAND_ARGS, kGetFloat);
}

static bool Cmd_GetMenuStringValue_Execute(COMMAND_ARGS)
{
	return GetSetMenuValue_Execute(PASS_COMMAND_ARGS, kGetString);
}

static bool Cmd_SetMenuFloatValue_Execute(COMMAND_ARGS)
{
	return GetSetMenuValue_Execute(PASS_COMMAND_ARGS, kSetFloat);
}

static bool Cmd_SetMenuStringValue_Execute(COMMAND_ARGS)
{
	return GetSetMenuValue_Execute(PASS_COMMAND_ARGS, kSetString);
}

static bool Cmd_GetMenuHasTrait_Execute(COMMAND_ARGS)
{
	*result = 0;
	return GetSetMenuValue_Execute(PASS_COMMAND_ARGS, kExists);
}

static bool Cmd_SetButtonPressed_Execute(COMMAND_ARGS)
{
	UInt32 button = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &button))
	{
		MessageMenu* menu = (MessageMenu*)GetMenuByType(kMenuType_Message);
		if (menu)
			menu->HandleClick(menu->kButtonID_Button1 + button, 0);
	}

	return true;
}

static bool Cmd_GetActiveUIComponentName_Execute(COMMAND_ARGS)
{
	const char* name = "";

	InterfaceManager* intfc = InterfaceManager::GetSingleton();
	Tile* actTile = intfc->activeTile ? intfc->activeTile : intfc->altActiveTile;
	if (actTile) {
		name = actTile->name.m_data;
	}
	AssignToStringVar(PASS_COMMAND_ARGS, name);
	return true;
}

static bool Cmd_GetActiveUIComponentFullName_Execute(COMMAND_ARGS)
{
	const char* name = "";
	std::string nameStr;

	InterfaceManager* intfc = InterfaceManager::GetSingleton();
	Tile* actTile = intfc->activeTile ? intfc->activeTile : intfc->altActiveTile;
	if (actTile) {
		nameStr = actTile->GetQualifiedName();
		name = nameStr.c_str();
	}

	AssignToStringVar(PASS_COMMAND_ARGS, name);
	return true;
}

static bool Cmd_GetActiveUIComponentID_Execute(COMMAND_ARGS)
{
	*result = -1;

	InterfaceManager* intfc = InterfaceManager::GetSingleton();
	Tile* actTile = intfc->activeTile ? intfc->activeTile : intfc->altActiveTile;

	if (actTile)
	{
		float id;
		if (actTile->GetFloatValue(kTileValue_id, &id))
			*result = id;
	}

	return true;
}

static bool Cmd_ClickMenuButton_Execute(COMMAND_ARGS)
{
	UInt32 menuType;
	char name[kMaxMessageLength] = { 0 };

	if (ExtractFormatStringArgs(0, name, paramInfo, scriptData, opcodeOffsetPtr, scriptObj, eventList, kCommandInfo_GetMenuFloatValue.numParams, &menuType))
	{
		Menu* menu = GetMenuByType(menuType);
		if (menu && name)
		{
			UInt32 buttonID = 0;
			Tile* parentTile = NULL;

			if (name[0] == '#')		// component specified by ID rather than name
			{
				buttonID = atoi(name + 1);
				parentTile = menu->tile->GetChildByIDTrait(buttonID);
			}
			else		// roundabout way of getting a button tile - look up the <id> trait then take the parent tile
			{
				UInt32 nameLen = strlen(name);
				name[nameLen] = '\\';
				name[nameLen+1] = 'i';
				name[nameLen+2] = 'd';
				name[nameLen+3] = '\0';

				Tile::Value* val = menu->tile->GetValueByName(name);
				if (val)
				{
					buttonID = val->num;
					parentTile = val->parentTile;
				}
			}

			//DEBUG_PRINT("Parent Tile: %08x, ID: %d", parentTile, buttonID);
			if (parentTile)
				menu->HandleClick(buttonID, parentTile);
		}
	}

	return true;
}

static bool Cmd_IsGameMessageBox_Execute(COMMAND_ARGS)
{
	*result = 0;
	MessageMenu* menu = (MessageMenu*)GetMenuByType(kMenuType_Message);
	if (menu)	// if not game generated, expression is true
		*result = (menu->buttonCallback == ShowMessageBox_Callback) ? 0 : 1;

	return true;
}

enum MsgBoxType{
	kMsgBox_Unknown,
	kMsgBox_Script,
	kMsgBox_BuyItem,
	kMsgBox_SellItem,
	kMsgBox_GiveItem,
	kMsgBox_BuySpell,
	kMsgBox_PoisonWeapon,
	kMsgBox_OverwriteGame,
	kMsgBox_LoadGame,
	kMsgBox_MissingContent,
	kMsgBox_NoRepairMagic,

	kMsgBox_Max
};

static bool Cmd_GetMessageBoxType_Execute(COMMAND_ARGS)
{
	*result = kMsgBox_Unknown;

	MessageMenu* menu = (MessageMenu*)GetMenuByType(kMenuType_Message);
	if (!menu || !menu->buttonCallback || !menu->messageText)
		return true;

	const char* msg = NULL;
	if (!menu->messageText->GetStringValue(kTileValue_string, &msg) || !msg)
		return true;

	std::string msgText(msg);
	SettingInfo* setting = NULL;

	if (menu->buttonCallback == NULL)
	{
		if (GetGameSetting("sNoRepairMagic", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_NoRepairMagic;
	}
	else if (menu->buttonCallback == ShowMessageBox_Callback)
	{
		*result = kMsgBox_Script;
	}
	else if (menu->buttonCallback == ContainerMenuCallback)
	{
		if (GetGameSetting("sBuy", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_BuyItem;
		else if (GetGameSetting("sSell", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_SellItem;
		else if (GetGameSetting("sGiveAway", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_GiveItem;
	}
	else if (menu->buttonCallback == SpellPurchaseCallback)
	{
		if (GetGameSetting("sConfirmBuySpell", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_BuySpell;
	}
	else if (menu->buttonCallback == PoisonConfirmCallback)
	{
		if (GetGameSetting("sPoisonConfirmMessage", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_PoisonWeapon;
		else if (GetGameSetting("sPoisonBowConfirmMessage", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_PoisonWeapon;
	}
	else if (menu->buttonCallback == OverwriteSaveGameCallback)
	{
		if (GetGameSetting("sSaveOverSaveGame", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_OverwriteGame;
	}
	else if (menu->buttonCallback == LoadGameCallback)
	{
		if (GetGameSetting("sLoadFromMainMenu", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_LoadGame;
		else if (GetGameSetting("sLoadWhilePlaying", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_LoadGame;
	}
	else if (menu->buttonCallback == MissingContentCallback)
	{
		if (GetGameSetting("sSaveGameContentIsMissing", &setting) && msgText.find(setting->s) == 0)
			*result = kMsgBox_MissingContent;
	}

	if (IsConsoleMode())
		Console_Print("GetMessageBoxType >> %.0f", *result);

	return true;
}

static Tile* ExtractMenuComponent(COMMAND_ARGS, const char* componentNameBuffer)
{
	UInt32 menuType = 0;
	Tile* tile = NULL;

	if (ExtractArgs(PASS_EXTRACT_ARGS, componentNameBuffer, &menuType))
	{
		Menu* menu = GetMenuByType(menuType);
		if (menu)
			tile = menu->GetComponentByName(componentNameBuffer);
	}

	return tile;
}

static bool Cmd_GetTileTraits_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetTileTraits must be called within an OBSE expression.");
		return true;
	}

	char buf[kMaxMessageLength] = { 0 };
	UInt32 arrID = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	*result = arrID;

	Tile* tile = ExtractMenuComponent(PASS_COMMAND_ARGS, buf);
	if (tile)
	{
		for (Tile::ValueList::Node* node = tile->valueList.start; node && node->data; node = node->next)
		{
			const char* traitName = Tile::StrIDToStr(node->data->id);
			if (node->data->bIsNum)
				g_ArrayMap.SetElementNumber(arrID, traitName, node->data->num);
			else
				g_ArrayMap.SetElementString(arrID, traitName, node->data->str.m_data);
		}
	}

	return true;
}

static bool Cmd_GetTileChildren_Execute(COMMAND_ARGS)
{
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "GetTileChildren must be called within an OBSE expression.");
		return true;
	}

	char buf[kMaxMessageLength] = { 0 };
	UInt32 arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	Tile* tile = ExtractMenuComponent(PASS_COMMAND_ARGS, buf);
	if (tile)
	{
		UInt32 idx = 0;
		for (Tile::RefList::Node* node = tile->childList.start; node && node->data; node = node->next)
		{
			g_ArrayMap.SetElementString(arrID, idx, node->data->name.m_data);
			idx++;
		}
	}

	return true;
}

static bool PrintTileInfo(const char* componentPath, Tile* tile)
{
	Console_Print("PrintTileInfo %s", componentPath);
	_MESSAGE("PrintTileInfo %s", componentPath);
	if (!tile)
	{
		Console_Print("   Component not found");
		_MESSAGE("   Component not found");
		return true;
	}

	Console_Print("  Traits:");
	_MESSAGE("  Traits:");
	std::string output;
	for (Tile::ValueList::Node* node = tile->valueList.start; node && node->data; node = node->next)
	{
		output = "   ";
		output += Tile::StrIDToStr(node->data->id);
		output += ": ";
		if (node->data->bIsNum)
		{
			char num[0x30];
			sprintf_s(num, sizeof(num), "%.4f", node->data->num);
			output += num;
		}
		else
			output += node->data->str.m_data;

		Console_Print(output.c_str());
		_MESSAGE(output.c_str());
	}

	Console_Print("  Children:");
	_MESSAGE("  Children:");
	for (Tile::RefList::Node* node = tile->childList.start; node && node->data; node = node->next)
	{
		output = "   ";
		const char* nodeName = node->data->name.m_data;
		output += nodeName ? nodeName : "<UNNAMED NODE>";

		Console_Print(output.c_str());
		_MESSAGE(output.c_str());
	}

	return true;
}

static bool Cmd_PrintTileInfo_Execute(COMMAND_ARGS)
{
	char componentPath[kMaxMessageLength] = { 0 };
	Tile* tile = ExtractMenuComponent(PASS_COMMAND_ARGS, componentPath);
	return PrintTileInfo(componentPath, tile);
}

static bool Cmd_PrintActiveTileInfo_Execute(COMMAND_ARGS)
{
	Tile* activeTile = InterfaceManager::GetSingleton()->activeTile ? InterfaceManager::GetSingleton()->activeTile : InterfaceManager::GetSingleton()->altActiveTile;
	return PrintTileInfo("< Active Tile >", activeTile);
}

static bool Cmd_GetMapMenuMarkerName_Execute(COMMAND_ARGS)
{
	const char* name = "";

	MapMenu* mapMenu = (MapMenu*)GetMenuByType(kMenuType_Map);
	if (mapMenu)
		name = mapMenu->GetSelectedMarkerName();

	AssignToStringVar(PASS_COMMAND_ARGS, name);
	return true;
}

static bool Cmd_GetMapMenuMarkerRef_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	MapMenu* mapMenu = (MapMenu*)GetMenuByType(kMenuType_Map);
	if (mapMenu)
	{
		TESObjectREFR* markerRef = mapMenu->GetSelectedMarker();
		if (markerRef)
		{
			*refResult = markerRef->refID;
			if (IsConsoleMode())
				Console_Print("GetMapMenuMarkerRef >> %s (%08X)", GetFullName(markerRef), markerRef->refID);
		}
	}

	return true;
}

static bool Cmd_GetBarterItem_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	ContainerMenu* contMenu = (ContainerMenu*)GetMenuByType(kMenuType_Container);
	if (contMenu) {
		TESForm* item = contMenu->GetItem();
		if (item) {
			*refResult = item->refID;
		}
	}

	return true;
}

static bool Cmd_GetBarterItemQuantity_Execute(COMMAND_ARGS)
{
	UInt32 quantity = ContainerMenu::GetQuantity();
	*result = (quantity == -1) ? -1.0 : quantity;
	DEBUG_PRINT("Quantity: %.2f", *result);
	return true;
}

static bool Cmd_GetLastTransactionQuantity_Execute(COMMAND_ARGS)
{
	UInt32 quantity = 0;
	GetLastTransactionInfo(NULL, &quantity);
	*result = quantity;
	DEBUG_PRINT("GetLastTransactionQuantity >> %.0f", *result);
	return true;
}

static bool Cmd_GetLastTransactionItem_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	GetLastTransactionInfo(&form, NULL);
	UInt32* refResult = (UInt32*)result;
	*refResult = form ? form->refID : 0;
	DEBUG_PRINT("GetLastTransactionItem >> %s", GetFullName(form));
	return true;
}

enum {
	kQuantityMenu_Max,
	kQuantityMenu_Cur,
	kQuantityMenu_Item,
};

static bool GetQuantityMenuInfo_Execute(COMMAND_ARGS, UInt32 which)
{
	QuantityMenu* menu = (QuantityMenu*)GetMenuByType(kMenuType_Quantity);
	if (menu) {
		switch (which) {
			case kQuantityMenu_Max:
				*result = menu->maxQuantity;
				DEBUG_PRINT("GetQMMaximum >> %d", menu->maxQuantity);
				return true;
			case kQuantityMenu_Cur:
				{
					if (menu->quantity_scroll) {
						Tile::Value* val = menu->quantity_scroll->GetValueByType(kTileValue_user7);
						if (val) {
							*result = val->num;
							DEBUG_PRINT("GetQMCurrent >> %.0f", val->num);
						}
					}
				}
				return true;
			case kQuantityMenu_Item:
				{
					TESForm* item = NULL;
					UInt32* refResult = (UInt32*)result;
					*refResult = 0;

					if (menu->itemTile) {
						Tile::Value* val = menu->itemTile->GetValueByType(kTileValue_user11);
						if (val) {
							UInt32 idx = val->num;
							// are we bartering? and what container is open?
							bool bMerchant = false;
							TESObjectREFR* container = *g_thePlayer;
							if (menu->unk44 == 0x33) {
								ContainerMenu* contMenu = (ContainerMenu*)GetMenuByType(kMenuType_Container);
								container = contMenu->isContainerContents ? contMenu->refr : container;
								bMerchant = contMenu && contMenu->isContainerContents && contMenu->isBarter;
							}

							item = container->GetInventoryItem(idx, bMerchant);
							*refResult = item ? item->refID : 0;
							DEBUG_PRINT("GetQMItem >> %s", GetFullName(item));
						}
					}
				}
				return true;
		}
	}
	return true;
}

static bool Cmd_GetQuantityMenuItem_Execute(COMMAND_ARGS)
{
	return GetQuantityMenuInfo_Execute(PASS_COMMAND_ARGS, kQuantityMenu_Item);
}

static bool Cmd_GetQuantityMenuCurrentQuantity_Execute(COMMAND_ARGS)
{
	return GetQuantityMenuInfo_Execute(PASS_COMMAND_ARGS, kQuantityMenu_Cur);
}

static bool Cmd_GetQuantityMenuMaximumQuantity_Execute(COMMAND_ARGS)
{
	return GetQuantityMenuInfo_Execute(PASS_COMMAND_ARGS, kQuantityMenu_Max);
}

enum {
	kSSInfo_Stone	= 0,
	kSSInfo_Old		= 1,
	kSSInfo_New		= 2
};

static bool GetSigilStoneInfo_Execute(COMMAND_ARGS, UInt32 which)
{
	ASSERT(which <= kSSInfo_New);

	TESForm* info[3] = { NULL, NULL, NULL };
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (GetLastSigilStoneInfo(info, &info[1], &info[2]) && info[which]) {
		*refResult = info[which]->refID;
	}

	return true;
}

static bool Cmd_GetLastUsedSigilStone_Execute(COMMAND_ARGS)
{
	return GetSigilStoneInfo_Execute(PASS_COMMAND_ARGS, kSSInfo_Stone);
}

static bool Cmd_GetLastSigilStoneEnchantedItem_Execute(COMMAND_ARGS)
{
	return GetSigilStoneInfo_Execute(PASS_COMMAND_ARGS, kSSInfo_Old);
}

static bool Cmd_GetLastSigilStoneCreatedItem_Execute(COMMAND_ARGS)
{
	return GetSigilStoneInfo_Execute(PASS_COMMAND_ARGS, kSSInfo_New);
}

static bool Cmd_GetClassMenuSelectedClass_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	ClassMenu* theMenu = (ClassMenu*)GetMenuByType(kMenuType_Class);
	if (theMenu){
		TESClass* theClass = theMenu->selectedClass;
		if (theClass) {
			*refResult = theClass->refID;
		}
	}

	if(IsConsoleMode())
		Console_Print("GetSelectedClass >> (%08X)", *refResult);

	return true;
}

static bool Cmd_GetClassMenuHighlightedClass_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	// make sure the user is actually doing class selection
	ClassMenu* theMenu = (ClassMenu*)GetMenuByType(kMenuType_Class);
	if (!theMenu){
		if(IsConsoleMode())
			Console_Print("GetHighlightedClass >> Class menu not open");
		return true;
	}
	InterfaceManager* intfc = InterfaceManager::GetSingleton();
	Tile* tile = intfc->activeTile ? intfc->activeTile : intfc->altActiveTile;
	if (!tile){
		if(IsConsoleMode())
			Console_Print("GetHighlightedClass >> Not selecting a class");
		return true;
	}

	if (strcmp(tile->parent->GetQualifiedName().c_str(), "class_background\\class_list_window\\class_list_pane")){
		if(IsConsoleMode())
			Console_Print("GetHighlightedClass >> Not selecting a class");
		return true;
	}

	// get the name of the highlighted class
	const char* className;
	tile->GetStringValue(kTileValue_user1, &className);

	// ClassMenu doesn't have a convenient pointer to the highlighted class?
	// So grab the class info from the menu.
	float specialization;
	theMenu->tile->GetFloatValue(kTileValue_user18, &specialization);

	float attributes[2];
	theMenu->tile->GetFloatValue(kTileValue_user19, &attributes[0]);
	theMenu->tile->GetFloatValue(kTileValue_user20, &attributes[1]);

	float skills[7];
	theMenu->tile->GetFloatValue(kTileValue_user11, &skills[0]);
	theMenu->tile->GetFloatValue(kTileValue_user12, &skills[1]);
	theMenu->tile->GetFloatValue(kTileValue_user13, &skills[2]);
	theMenu->tile->GetFloatValue(kTileValue_user14, &skills[3]);
	theMenu->tile->GetFloatValue(kTileValue_user15, &skills[4]);
	theMenu->tile->GetFloatValue(kTileValue_user16, &skills[5]);
	theMenu->tile->GetFloatValue(kTileValue_user17, &skills[6]);

	// iterate over all classes in the game to find one that matches
	for (tList<TESClass>::Iterator Itr = (*g_dataHandler)->classes.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TESClass* theClass = Itr.Get();

		if(theClass->IsPlayable())
		{
			if(!strcmp(theClass->GetFullName()->name.m_data, className)){
				if(theClass->specialization == (UInt32)specialization){
					if(theClass->attributes[0] == (UInt32)attributes[0] && theClass->attributes[1] == (UInt32)attributes[1]){
						bool bAttributesMatch = true;
						for (UInt32 i = 0; i < 7; i++) {
							if (theClass->majorSkills[i] != skills[i]) {
								bAttributesMatch = false;
								break;
							}
						}
						if (bAttributesMatch) {
							*refResult = theClass->refID;
							break;
						}
					}
				}
			}
		}
	}

	if(IsConsoleMode())
		Console_Print("GetHighlightedClass >> (%08X)", *refResult);

	return true;
}

static bool Cmd_GetEnchMenuBaseItem_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	EnchantmentMenu* theMenu = (EnchantmentMenu*)GetMenuByType(kMenuType_Enchantment);
	if (theMenu && theMenu->enchantableInfo){
		TESForm* item = theMenu->enchantableInfo->form;
		if(item){
			*refResult = item->refID;
		}
	}

	if(IsConsoleMode())
		Console_Print("GetEnchMenuBaseItem >> (%08x)", *refResult);

	return true;
}

static bool Cmd_GetMapMarkers_Execute(COMMAND_ARGS)
{
	UInt32 inclHidden = 1;
	UInt32 markerType = 0;

	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if(ExtractArgs(PASS_EXTRACT_ARGS, &inclHidden, &markerType))
	{
		std::vector<TESObjectREFR*> vec;
		MapMenu* mapMenu = (MapMenu*)GetMenuByType(kMenuType_Map);
		for (MapMenu::MapMarkerEntry* cur = mapMenu->mapMarkers; cur && cur->mapMarker; cur = cur->next){
			ExtraMapMarker* markerExtra = (ExtraMapMarker*)cur->mapMarker->baseExtraList.GetByType(kExtraData_MapMarker);
			UInt32 isVisible = markerExtra->IsVisible();
			UInt32 canTravel = markerExtra->CanTravelTo();
			if(!markerType || markerType == markerExtra->data->type){
				if ((inclHidden == 0 && isVisible && canTravel) || (inclHidden == 1 && isVisible) || inclHidden == 2){
					//It seems that disabled mapmarkers are considered visible so an extra check is needed
					if(!cur->mapMarker->IsDisabled() || inclHidden == 2){
						vec.push_back(cur->mapMarker);
					}
				}
			}
		}
		//We wanna reverse the array to make it easier to combine with the GetMenuChildXXXValue functions
		std::reverse(vec.begin(), vec.end());
		for (UInt32 i = 0; i < vec.size(); i++) {
			g_ArrayMap.SetElementFormID(arr, (double)i, vec[i]->refID);
		}
	}

	return true;
}

static bool Cmd_UpdateContainerMenu_Execute(COMMAND_ARGS)
{
	// we could check if the container menu is actually open, but Update() will do nothing if it isn't
	ContainerMenu::Update();
	*result = 1.0;
	return true;
}

static bool Cmd_UpdateSpellPurchaseMenu_Execute(COMMAND_ARGS)
{
	SpellPurchaseMenu* menu = OBLIVION_CAST(GetMenuByType(kMenuType_SpellPurchase), Menu, SpellPurchaseMenu);
	if (menu) {
		menu->Update();
		*result = 1.0;
	}

	return true;
}

static bool Cmd_scrwtf_Execute(COMMAND_ARGS)
{
	return true;
}

#endif

DEFINE_COMMAND(GetActiveMenuMode,
			   returns the code for the menu over which the mouse is positioned,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetEnchMenuEnchItem,
			   returns the selected enchantment item,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetEnchMenuSoulgem,
			   returns the selected soulgem,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetActiveMenuObject,
			   returns the base object from which the active menu is derived,
			   0,
			   1,
			   kParams_OneOptionalInt);

DEFINE_COMMAND(GetActiveMenuRef,
			   returns the ref from which the active menu is derived,
			   0,
			   1,
			   kParams_OneOptionalInt);

DEFINE_COMMAND(GetActiveMenuSelection,
			   returns the selected item in the active menu,
			   0,
			   1,
			   kParams_OneOptionalInt);

DEFINE_COMMAND(GetActiveMenuFilter,
			   returns the filter code for the active menu,
			   0,
			   1,
			   kParams_OneOptionalInt);

DEFINE_COMMAND(IsBarterMenuActive,
			   returns 1 if the barter menu is active,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetAlchMenuIngredient,
			   returns the nth ingredient selected in the Alchemy interface,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetAlchMenuIngredientCount,
			   returns the count of the nth ingredient selected in the Alchemy interface,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetAlchMenuApparatus,
			   returns the nth apparatus selected in the alchemy interface,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetContainerMenuView,
			   returns 1 if the player is looking at his own inventory,
			   0,
			   0,
			   NULL);

static ParamInfo kParams_SetAlchMenuApparatus[2] =
{
	{	"int",		 kParamType_Integer,		 0	},
	{	"apparatus", kParamType_InventoryObject, 0	},
};

/*
DEFINE_COMMAND(SetAlchMenuApparatus,
			   sets the apparatus being used for brewing potions,
			   0,
			   2,
			   kParams_SetAlchMenuApparatus);
*/

DEFINE_COMMAND(CloseAllMenus,
			   closes all open menus,
			   0,
			   0,
			   NULL);

static ParamInfo kParams_GetMenuValue[SIZEOF_FMT_STRING_PARAMS + 1] =
{
	FORMAT_STRING_PARAMS,
	{	"menuType",	kParamType_Integer,	0	},
};

DEFINE_COMMAND(GetMenuFloatValue,
			   returns a float value for the specified menu component,
			   0,
			   NUM_PARAMS(kParams_GetMenuValue),
			   kParams_GetMenuValue);

DEFINE_COMMAND(GetMenuStringValue,
			   returns a string value for the specified menu component,
			   0,
			   NUM_PARAMS(kParams_GetMenuValue),
			   kParams_GetMenuValue);

static ParamInfo kParams_SetMenuFloatValue[SIZEOF_FMT_STRING_PARAMS + 2] =
{
	FORMAT_STRING_PARAMS,
	{	"menuType",		kParamType_Integer,	0	},
	{	"new value",	kParamType_Float,	0	},
};

DEFINE_COMMAND(SetMenuFloatValue,
			   sets a float value for the specified menu component,
			   0,
			   NUM_PARAMS(kParams_SetMenuFloatValue),
			   kParams_SetMenuFloatValue);

DEFINE_COMMAND(SetMenuStringValue,
			   sets a string value for the specified menu component,
			   0,
			   NUM_PARAMS(kParams_GetMenuValue),
			   kParams_GetMenuValue);

DEFINE_COMMAND(GetMenuHasTrait,
			   returns 1 if the specified trait exists,
			   0,
			   NUM_PARAMS(kParams_GetMenuValue),
			   kParams_GetMenuValue);

DEFINE_COMMAND(SetButtonPressed,
			   forces a messagebox button press,
			   0,
			   1,
			   kParams_OneInt);

DEFINE_COMMAND(GetActiveUIComponentName,
			   returns the name of the highlighted UI component such as a button,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetActiveUIComponentFullName,
			   returns the qualified name of the highlighted UI component,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetActiveUIComponentID,
			   returns the ID of the highlighted UI component as defined in the menu XML,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(ClickMenuButton,
			   simulates a mouse click of the specified button,
			   0,
			   NUM_PARAMS(kParams_GetMenuValue),
			   kParams_GetMenuValue);

DEFINE_COMMAND(IsGameMessageBox, returns 1 if a game-generated messagebox is currently displayed, 0, 0, NULL);
DEFINE_COMMAND(GetMessageBoxType, returns the type of the messagebox, 0, 0, NULL);

DEFINE_COMMAND(GetTileTraits, returns an array of traits for the specified UI component, 0, 2, kParams_OneString_OneInt);
DEFINE_COMMAND(GetTileChildren, returns an array of children for the specified UI component, 0, 2, kParams_OneString_OneInt);
DEFINE_COMMAND(PrintTileInfo, debug command for printing traits and values, 0, 2, kParams_OneString_OneInt);

DEFINE_COMMAND(GetMapMenuMarkerName, returns the name of the currently selected map marker, 0, 0, NULL);
DEFINE_COMMAND(GetMapMenuMarkerRef, returns a reference to the currently selected map marker, 0, 0, NULL);

DEFINE_COMMAND(GetBarterItem, returns the active item in a container menu, 0, 0, NULL);
DEFINE_COMMAND(GetBarterItemQuantity, returns the selected quantity of the active item in a container menu, 0, 0, NULL);
DEFINE_COMMAND(GetLastTransactionItem, returns the item most recently bought or sold by the player, 0, 0, NULL);
DEFINE_COMMAND(GetLastTransactionQuantity, returns the quantity of the most recent buy/sell transaction, 0, 0, NULL);

CommandInfo kCommandInfo_GetQMItem =
{
	"GetQMItem", "GetQuantityMenuItem", 0,
	"returns the active item in the quantity menu",
	0, 0, NULL,
	HANDLER(Cmd_GetQuantityMenuItem_Execute), Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_GetQMCurrent =
{
	"GetQMCurrent", "GetQuantityMenuCurrentQuantity", 0,
	"returns the current quantity in the quantity menu",
	0, 0, NULL,
	HANDLER(Cmd_GetQuantityMenuCurrentQuantity_Execute), Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_GetQMMaximum =
{
	"GetQMMaximum", "GetQuantityMenuMaximumQuantity", 0,
	"returns the maximum quantity in the quantity menu",
	0, 0, NULL,
	HANDLER(Cmd_GetQuantityMenuMaximumQuantity_Execute), Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_GetLastUsedSigilStone =
{
	"GetLastUsedSigilStone", "GetLastSS", 0,
	"returns the sigil stone most recently used to enchant an item during the current game session",
	0, 0, NULL,
	HANDLER(Cmd_GetLastUsedSigilStone_Execute), Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_GetLastSigilStoneEnchantedItem =
{
	"GetLastSigilStoneEnchantedItem", "GetLastSSItem", 0,
	"returns the unenchanted item most recently enchanted with a sigil stone during the current game session",
	0, 0, NULL,
	HANDLER(Cmd_GetLastSigilStoneEnchantedItem_Execute), Cmd_Default_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_GetLastSigilStoneCreatedItem =
{
	"GetLastSigilStoneCreatedItem", "GetLastSSCreated", 0,
	"returns the enchanted item most recently created with a sigil stone during the current game session",
	0, 0, NULL,
	HANDLER(Cmd_GetLastSigilStoneCreatedItem_Execute), Cmd_Default_Parse,
	NULL, 0
};

DEFINE_COMMAND(GetClassMenuSelectedClass, returns the class currently selected by the player in the class menu, 0, 0, NULL);
DEFINE_COMMAND(GetClassMenuHighlightedClass, returns the class currently under the cursor in the class menu, 0, 0, NULL);
DEFINE_COMMAND(GetEnchMenuBaseItem, returns the unenchanted item the player has selected for enchanting, 0, 0, NULL);

DEFINE_COMMAND(PrintActiveTileInfo, a debug command for printing the attributes and children of the current active tile,
			   0, 0, NULL);

static ParamInfo kParams_TwoOptionalInts[2] =
{
	{	"int",	kParamType_Integer,	1	},
	{	"int",	kParamType_Integer,	1	},
};

DEFINE_COMMAND(GetMapMarkers, returns an array of mapmarkers for the current world, 0, 2, kParams_TwoOptionalInts);

DEFINE_COMMAND(scrwtf, testing, 0, 1, kParams_OneQuest);

DEFINE_COMMAND(UpdateContainerMenu, updates list of items displayed in the container menu, 0, 0, NULL);
DEFINE_COMMAND(UpdateSpellPurchaseMenu, updates list of spells displayed in spell purchase menu, 0, 0, NULL);