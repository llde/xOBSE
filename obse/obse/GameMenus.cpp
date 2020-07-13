#include "Utilities.h"
#include "GameTiles.h"
#include "GameMenus.h"
#include "GameAPI.h"
#include "GameObjects.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1

HUDInfoMenu	** g_HUDInfoMenu = (HUDInfoMenu**)0x00AFC094;
NiTArray<TileMenu*> * g_TileMenuArray = (NiTArray<TileMenu*> *)0x00AD4458;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2

HUDInfoMenu	** g_HUDInfoMenu = (HUDInfoMenu**)0x00B3B33C;
NiTArray<TileMenu*> * g_TileMenuArray = (NiTArray<TileMenu*> *)0x00B13970;

#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416

HUDInfoMenu	** g_HUDInfoMenu = (HUDInfoMenu**)0x00B3B33C;
NiTArray<TileMenu*> * g_TileMenuArray = (NiTArray<TileMenu*> *)0x00B13970;
static const UInt32* g_ContainerMenu_Quantity = (const UInt32*)0x00B13E94;
static NiTListBase<MagicItemAndIndex> * g_MagicMenuMagicItemList = (NiTListBase<MagicItemAndIndex>*)0x00B1435C;

#else

#error unsupported version of oblivion

#endif

Menu* GetMenuByType(UInt32 menuType)
{
	if (menuType >= kMenuType_Message && menuType < kMenuType_Max)
	{
		TileMenu* tileMenu = g_TileMenuArray->data[menuType - kMenuType_Message];
		if (tileMenu)
			return tileMenu->menu;
	}

	return NULL;
}

void Menu::RegisterTile(TileMenu * tileMenu)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	ThisStdCall(0x005779A0, this, tileMenu);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	ThisStdCall(0x00584870, this, tileMenu);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00584880, this, tileMenu);
#else
#error unsupported Oblivion version
#endif
}

void Menu::EnableMenu(bool unk)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	ThisStdCall(0x005785D0, this, unk);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	ThisStdCall(0x00585160, this, unk);
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00585190, this, unk);
#else
#error unsupported Oblivion version
#endif
}

IngredientItem* AlchemyMenu::GetIngredientItem(UInt32 whichIngred)
{
	if (whichIngred < 4 && ingreds[whichIngred])
		return ingreds[whichIngred]->ingredient;
	else
		return NULL;
}

TESObjectAPPA* AlchemyMenu::GetApparatus(UInt32 whichAppa)
{
	if (whichAppa < 4 && apparatus[whichAppa])
		return apparatus[whichAppa]->apparatus;
	else
		return NULL;
}

UInt32 AlchemyMenu::GetIngredientCount(UInt32 whichIngred)
{
	if (whichIngred < 4 && ingreds[whichIngred])
		return ingreds[whichIngred]->count;
	else
		return -1;
}

void BookMenu::UpdateText(const char* newText)
{
	tile->UpdateString(0xFB0, newText);
}

bool MessageMenu::IsScriptMessageBox()
{
	return (buttonCallback == (void*)ShowMessageBox_Callback);
}

Tile* Menu::GetComponentByName(const char* componentPath)
{
	Tokenizer tokens(componentPath, "\\/");
	std::string tok;
	Tile* component = tile;

	while (component && tokens.NextToken(tok) != -1)
		component = component->GetChildByName(tok.c_str());

	return component;
}

const char* MapMenu::GetSelectedMarkerName()
{
	const char* name = NULL;

	if (worldIconPaper)
		worldIconPaper->GetStringValue(kTileValue_string, &name);

	return name ? name : "";
}

TESObjectREFR* MapMenu::GetSelectedMarker()
{
	if (worldIconPaper && InterfaceManager::GetSingleton()->activeTile)
	{
		const char* name = GetSelectedMarkerName();
		if (name && name[0])
		{
			// not very efficient due to the way marker info is stored by map menu
			for (MapMenu::MapMarkerEntry* cur = mapMarkers; cur && cur->mapMarker; cur = cur->next)
			{
				if (!strcmp(GetFullName(cur->mapMarker), name))
				{
					// okay, name matches. Do coords match?
					float markerX = cur->mapMarker->posX;
					markerX -= unk0A0;
					UInt32 diff = unk0A4 - unk0A0;
					markerX /= diff;
					// ###TODO: game code checks if worldMapWidth < 0, why would it be < 0?
					markerX *= worldMapWidth;

					float iconX = 0;
					InterfaceManager::GetSingleton()->activeTile->GetFloatValue(kTileValue_user1, &iconX);
					if (fabs(iconX - markerX) < 0.001)		// reasonable margin of error?
					{
						// ###TODO: check y
						
						return cur->mapMarker;
					}
#if _DEBUG
					_MESSAGE("markerX: %.2f iconX: %.2f", markerX, iconX);
					for (UInt32 i = kTileValue_user0; i < kTileValue_user7; i++)
					{
						worldIconPaper->GetFloatValue(i, &iconX);
						_MESSAGE("  user%d >> %.2f", i, iconX);
					}
#endif
				}
			}
		}
	}

	return NULL;
}

void MapMenu::UpdateMarkerName(TESObjectREFR* mapMarker, const char* newName)
{
	// when changing the name of the mapmarker refr need to update name in menu as well
	// multiple mapmarkers can share the same name so we need to check all of them until the correct match is found

	if (!mapMarker || !newName || !mapMarker->baseForm || mapMarker->baseForm->refID != kFormID_MapMarker)
		return;

	Tile* root = GetComponentByName("map_background\\map_page_2\\map_world_layout\\map_world_map_window\\map_world_map");
	if (!root)
	{
		DEBUG_PRINT("Could not locate root in MapMenu::UpdateMarkerName");
		return;
	}

	TESFullName* fullName = mapMarker->GetFullName();
	if (!fullName || !fullName->name.m_data)
		return;

	const char* markerFullName = fullName->name.m_data;
	float markerX = mapMarker->posX;

	// tile names replace space characters with underscores, adjust marker name to match
	char markerName[0x200];
	UInt32 i;
	for (i = 0; i < sizeof(markerName) && markerFullName[i]; i++)
	{
		if (markerFullName[i] == ' ')
			markerName[i] = '_';
		else
			markerName[i] = markerFullName[i];
	}
	markerName[i] = 0;

	for (Tile::RefList::Node* curNode = root->childList.start; curNode; curNode = curNode->next)
	{
		Tile* elem = curNode->data;
		if (elem && !_stricmp(elem->name.m_data, markerName))
		{
			// do coords match? ###TODO factor this out into a separate routine
			markerX = mapMarker->posX;
			markerX -= unk0A0;
			UInt32 diff = unk0A4 - unk0A0;
			markerX /= diff;
			// ###TODO: game code checks if worldMapWidth < 0, why would it be < 0?
			markerX *= worldMapWidth;

			float iconX = 0;
			if (elem->GetFloatValue(kTileValue_user1, &iconX))
			{
				float diff = fabs(iconX - markerX);
				if (diff < 0.001)
				{
					// got a match, now rename it
					elem->SetStringValue(kTileValue_user4, newName);
					
					// rename the tile itself so name can be changed again without exiting menu. must replace space with underscore
					char tileName[0x200];
					UInt32 i;
					for (i = 0; i < sizeof(tileName) && newName[i]; i++)
					{
						if (newName[i] == ' ')
							tileName[i] = '_';
						else
							tileName[i] = newName[i];
					}
					tileName[i] = 0;

					elem->name.Set(tileName);
					return;
				}
			}
		}
	}
}

UInt32 ContainerMenu::GetQuantity()
{
	UInt32 quantity = *g_ContainerMenu_Quantity;
	return (quantity == -1) ? 1 : quantity;
}

void ContainerMenu::Update()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	typedef void (* _Func)();
	static const _Func Func = (_Func)0x005992C0;
#else
#error unsupported Oblivion version
#endif

	Func();
}

UInt32 ContainerMenu::GetItemIndex()
{
	if (selectedItemTile) {
		Tile::Value* idxVal = selectedItemTile->GetValueByType(kTileValue_user11);
		if (idxVal) {
			return idxVal->num;
		}
	}

	return -1;
}

TESForm* ContainerMenu::GetItem()
{
	UInt32 idx = GetItemIndex();
	if (idx != -1) {
		TESObjectREFR* cont = isContainerContents ? refr : *g_thePlayer;
		if (cont) {
			return cont->GetInventoryItem(idx, (isBarter && isContainerContents));
		}
	}

	return NULL;
}

NiTListBase<MagicItemAndIndex> * MagicMenu::GetMagicItemList()
{
	return g_MagicMenuMagicItemList;
}

TESForm* MagicMenu::GetMagicItemForIndex(UInt32 idx)
{
	NiTListBase<MagicItemAndIndex> * list = GetMagicItemList();
	for (NiTListBase<MagicItemAndIndex>::Node* node = list->start; node && node->data; node = node->next) {
		if (node->data->index == idx) {
			return node->data->data ? node->data->data->object : NULL;
		}
	}

	return NULL;
}

void SpellPurchaseMenu::Update()
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x005D90E0, this);
#else
#error unsupported Oblivion version
#endif
}
