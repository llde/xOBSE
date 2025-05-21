#include "GameTiles.h"
#include "GameAPI.h"
#include <string>

#if 0
const char * Tile::StrIDToStr(UInt32 id)
{
// this is going to generate horrible code so only include it in debug builds
// Actually, use game code instead
#ifdef _DEBUG
	switch(id)
	{
		case 0x00000001: return "<nonunique>";
		case 0x00000002: return "<nonunique>";
		case 0x00000004: return "&right;";
		case 0x00000065: return "&click_past;";
		case 0x00000066: return "&no_click_past;";
		case 0x00000067: return "&mixed_menu;";
		case 0x00000068: return "&prev;";
		case 0x00000069: return "&next;";
		case 0x0000006A: return "&first;";
		case 0x0000006B: return "&last;";
		case 0x0000006C: return "&xlist;";
		case 0x0000006D: return "&xitem;";
		case 0x000000FF: return "&scale;";
		case 0x00000385: return "rect";
		case 0x00000386: return "image";
		case 0x00000387: return "text";
		case 0x00000388: return "nif";
		case 0x00000389: return "menu";
		case 0x0000038B: return "window";
		case 0x000003E7: return "template";
		case 0x000003E9: return "&MessageMenu;";
		case 0x000003EA: return "&InventoryMenu;";
		case 0x000003EB: return "&StatsMenu;";
		case 0x000003EC: return "&HUDMainMenu;";
		case 0x000003ED: return "&HUDInfoMenu;";
		case 0x000003EE: return "&HUDReticle;";
		case 0x000003EF: return "&LoadingMenu;";
		case 0x000003F0: return "&ContainerMenu;";
		case 0x000003F1: return "&DialogMenu;";
		case 0x000003F2: return "&HUDSubtitleMenu;";
		case 0x000003F3: return "&GenericMenu;";
		case 0x000003F4: return "&SleepWaitMenu;";
		case 0x000003F5: return "&PauseMenu;";
		case 0x000003F6: return "&LockPickMenu;";
		case 0x000003F7: return "&OptionsMenu;";
		case 0x000003F8: return "&QuantityMenu;";
		case 0x000003F9: return "&AudioMenu;";
		case 0x000003FA: return "&VideoMenu;";
		case 0x000003FB: return "&VideoDisplayMenu;";
		case 0x000003FC: return "&GameplayMenu;";
		case 0x000003FD: return "&ControlsMenu;";
		case 0x000003FE: return "&MagicMenu;";
		case 0x000003FF: return "&MapMenu;";
		case 0x00000400: return "&MagicPopupMenu;";
		case 0x00000401: return "&NegotiateMenu;";
		case 0x00000402: return "&BookMenu;";
		case 0x00000403: return "&LevelUpMenu;";
		case 0x00000404: return "&TrainingMenu;";
		case 0x00000405: return "&BirthSignMenu;";
		case 0x00000406: return "&ClassMenu;";
		case 0x00000408: return "&SkillsMenu;";
		case 0x0000040A: return "&PersuasionMenu;";
		case 0x0000040B: return "&RepairMenu;";
		case 0x0000040C: return "&RaceSexMenu;";
		case 0x0000040D: return "&SpellPurchaseMenu;";
		case 0x0000040E: return "&LoadMenu;";
		case 0x0000040F: return "&SaveMenu;";
		case 0x00000410: return "&AlchemyMenu;";
		case 0x00000411: return "&SpellmakingMenu;";
		case 0x00000412: return "&EnchantmentMenu;";
		case 0x00000413: return "&EffectSettingMenu;";
		case 0x00000414: return "&MainMenu;";
		case 0x00000415: return "&BreathMenu;";
		case 0x00000416: return "&QuickKeysMenu;";
		case 0x00000417: return "&CreditsMenu;";
		case 0x00000418: return "&SigilStoneMenu;";
		case 0x00000419: return "&RechargeMenu;";
		case 0x0000041B: return "&TextEditMenu;";
		case 0x000007D1: return "copy";
		case 0x000007D2: return "add";
		case 0x000007D3: return "sub";
		case 0x000007D4: return "mul";
		case 0x000007D5: return "div";
		case 0x000007D6: return "rand";
		case 0x000007D7: return "user";
		case 0x000007D8: return "gt";
		case 0x000007D9: return "gte";
		case 0x000007DA: return "eq";
		case 0x000007DB: return "lte";
		case 0x000007DC: return "lt";
		case 0x000007DD: return "min";
		case 0x000007DE: return "max";
		case 0x000007DF: return "and";
		case 0x000007E0: return "or";
		case 0x000007E1: return "neq";
		case 0x000007E2: return "mod";
		case 0x000007E3: return "floor";
		case 0x000007E4: return "abs";
		case 0x000007E5: return "onlyif";
		case 0x000007E6: return "onlyifnot";
		case 0x000007E7: return "ln";
		case 0x000007E8: return "log";
		case 0x000007E9: return "ceil";
		case 0x000007EA: return "not";
		case 0x000007EB: return "ref";
		case 0x00000BB9: return "value";
		case 0x00000BBA: return "name";
		case 0x00000BBB: return "src";
		case 0x00000BBC: return "trait";
		case 0x00000FA1: return "visible";
		case 0x00000FA2: return "class";
		case 0x00000FA3: return "listclip";
		case 0x00000FA4: return "clipwindow";
		case 0x00000FA5: return "stackingtype";
		case 0x00000FA6: return "locus";
		case 0x00000FA7: return "alpha";
		case 0x00000FA8: return "id";
		case 0x00000FA9: return "disablefade";
		case 0x00000FAA: return "listindex";
		case 0x00000FAB: return "depth";
		case 0x00000FAC: return "y";
		case 0x00000FAD: return "x";
		case 0x00000FAE: return "user0";
		case 0x00000FAF: return "user1";
		case 0x00000FB0: return "user2";
		case 0x00000FB1: return "user3";
		case 0x00000FB2: return "user4";
		case 0x00000FB3: return "user5";
		case 0x00000FB4: return "user6";
		case 0x00000FB5: return "user7";
		case 0x00000FB6: return "user8";
		case 0x00000FB7: return "user9";
		case 0x00000FB8: return "user10";
		case 0x00000FB9: return "user11";
		case 0x00000FBA: return "user12";
		case 0x00000FBB: return "user13";
		case 0x00000FBC: return "user14";
		case 0x00000FBD: return "user15";
		case 0x00000FBE: return "user16";
		case 0x00000FBF: return "user17";
		case 0x00000FC0: return "user18";
		case 0x00000FC1: return "user19";
		case 0x00000FC2: return "user20";
		case 0x00000FC3: return "user21";
		case 0x00000FC4: return "user22";
		case 0x00000FC5: return "user23";
		case 0x00000FC6: return "user24";
		case 0x00000FC7: return "user25";
		case 0x00000FC8: return "clips";
		case 0x00000FC9: return "target";
		case 0x00000FCA: return "height";
		case 0x00000FCB: return "width";
		case 0x00000FCC: return "red";
		case 0x00000FCD: return "green";
		case 0x00000FCE: return "blue";
		case 0x00000FCF: return "tile";
		case 0x00000FD0: return "childcount";
		case 0x00000FD1: return "justify";
		case 0x00000FD2: return "zoom";
		case 0x00000FD3: return "font";
		case 0x00000FD4: return "wrapwidth";
		case 0x00000FD5: return "wraplimit";
		case 0x00000FD6: return "wraplines";
		case 0x00000FD7: return "pagenum";
		case 0x00000FD8: return "ishtml";
		case 0x00000FD9: return "cropy";
		case 0x00000FDA: return "cropx";
		case 0x00000FDB: return "menufade";
		case 0x00000FDC: return "explorefade";
		case 0x00000FDD: return "mouseover";
		case 0x00000FDE: return "string";
		case 0x00000FDF: return "shiftclicked";
		case 0x00000FE0: return "focusinset";
		case 0x00000FE1: return "clicked";
		case 0x00000FE2: return "clickcountbefore";
		case 0x00000FE3: return "clickcountafter";
		case 0x00000FE4: return "clickedfunc";
		case 0x00000FE5: return "clicksound";
		case 0x00000FE6: return "filename";
		case 0x00000FE7: return "filewidth";
		case 0x00000FE8: return "fileheight";
		case 0x00000FE9: return "repeatvertical";
		case 0x00000FEA: return "repeathorizontal";
		case 0x00000FEB: return "returnvalue";
		case 0x00000FEC: return "animation";
		case 0x00000FED: return "depth3d";
		case 0x00000FEE: return "linecount";
		case 0x00000FEF: return "pagecount";
		case 0x00000FF0: return "xdefault";
		case 0x00000FF1: return "xup";
		case 0x00000FF2: return "xdown";
		case 0x00000FF3: return "xleft";
		case 0x00000FF4: return "xright";
		case 0x00000FF5: return "xscroll";
		case 0x00000FF6: return "xlist";
		case 0x00000FF7: return "xbuttona";
		case 0x00000FF8: return "xbuttonb";
		case 0x00000FF9: return "xbuttonx";
		case 0x00000FFA: return "xbuttony";
		case 0x00000FFB: return "xbuttonlt";
		case 0x00000FFC: return "xbuttonrt";
		case 0x00000FFD: return "xbuttonlb";
		case 0x00000FFE: return "xbuttonrb";
		case 0x00001001: return "xbuttonstart";
		case 0x00001389: return "parent";
		case 0x0000138A: return "me";
		case 0x0000138C: return "sibling";
		case 0x0000138D: return "child";
		case 0x0000138E: return "screen";
		case 0x0000138F: return "strings";
		case 0x00001778: return "&does_not_stack;";
		default: return NULL;
	}
#else
	return NULL;
#endif
}
#endif

typedef UInt32 (* _TileStrToStrID)(const char * str);
typedef const char * (* _TileStrIDToStr)(UInt32 ID);

const _TileStrToStrID TileStrToStrID = (_TileStrToStrID)0x00588EF0;
const _TileStrIDToStr TileStrIDToStr = (_TileStrIDToStr)0x00589080;


UInt32 Tile::StrToStrID(const char * str)
{
	return TileStrToStrID(str);
}

const char* Tile::StrIDToStr(UInt32 id)
{
	const char* str = TileStrIDToStr(id);
	return str ? str : "<unknown>";
}

void Tile::DebugDump()
{
	_MESSAGE("%08X %02X %02X %02X %02X (%s) %08X %08X %08X %08X %s",
		this,
		unk04,
		unk05,
		unk06,
		pad07,
		name.m_data ? name.m_data : "<null>",
		parent,
		unk24,
		unk28,
		flags,
		GetType());

	gLog.Indent();

	if(valueList.numItems)
	{
		_MESSAGE("values:");
		gLog.Indent();

		for(Tile::ValueList::Node * iter = valueList.start; iter; iter = iter->next)
		{
			Tile::Value	* data = iter->data;

			if(data)
			{
				const char	* idStr = StrIDToStr(data->id);
				if(!idStr) idStr = "<unknown>";

				if(data->str.m_data)
					_MESSAGE("%04X %02X: %s = %s", data->id, data->bIsNum, idStr, data->str.m_data);
				else
					_MESSAGE("%04X %02X: %s = %f", data->id, data->bIsNum, idStr, data->num);
			}
		}

		gLog.Outdent();
	}

	if(childList.numItems)
	{
		_MESSAGE("children:");
		gLog.Indent();

		for(Tile::RefList::Node * iter = childList.start; iter; iter = iter->next)
		{
			if(iter->data)
				iter->data->DebugDump();
		}

		gLog.Outdent();
	}

	gLog.Outdent();
}

Tile * Tile::ReadXML(const char * xmlPath)
{
	return (Tile *)ThisStdCall(0x00590420, this, xmlPath);

}
Menu* Tile::GetContainingMenu() {
	return (Menu*)ThisStdCall(0x005898F0, this);
}

Tile * Tile::GetRoot(void)
{
	Tile	* traverse = this;

	while(traverse->parent && traverse->parent->parent)
	{
		traverse = traverse->parent;
	}

	return traverse;
}

Tile::Value* Tile::GetValueByType(UInt32 valueType)
{
	for (ValueList::Node* node = valueList.start; node; node = node->next)
	{
	//	DEBUG_PRINT("%04X\t%s\t%.2f", node->data->id, StrIDToStr(node->data->id), node->data->num);
		if (node->data && node->data->id == valueType)
			return node->data;
	}

	return NULL;
}

bool Tile::GetFloatValue(UInt32 valueType, float* out)
{
	Value* val = GetValueByType(valueType);
	if (val)
	{
		*out = val->num;
		return true;
	}
	
	return false;
}

bool Tile::SetFloatValue(UInt32 valueType, float newValue)
{
	Value* val = GetValueByType(valueType);
	if (val)
	{
		val->num = newValue;
		return true;
	}
	
	return false;
}

bool Tile::GetStringValue(UInt32 valueType, const char** out)
{
	Value* val = GetValueByType(valueType);
	if (val)
	{
		*out = val->str.m_data;
		return true;
	}

	return false;
}

bool Tile::SetStringValue(UInt32 valueType, const char* newValue)
{
	Value* val = GetValueByType(valueType);
	if (val)
	{
		val->str.Set(newValue);
		return true;
	}

	return false;
}

bool Tile::DeleteValue(UInt32 valueType)
{
	ValueList::Node* prevNode = NULL;
	ValueList::Node* targetNode = NULL;

	for (ValueList::Node* node = valueList.start; node; node = node->next)
	{
		if (node->data && node->data->id == valueType)
		{
			targetNode = node;
			break;
		}
		else
			prevNode = node;
	}

	if (!targetNode)
		return false;

	if (prevNode)
		prevNode->next = targetNode->next;
	else
		valueList.start = targetNode->next;

	if (!targetNode->next)
		valueList.end = prevNode;

	valueList.FreeNode(targetNode);
	return true;
}

void Tile::UpdateFloat(UInt32 valueType, float newValue)
{
	//	ThisStdCall() doesn't handle floating point args correctly
	// do it in asm

	static UInt32 callAddr = 0x0058CEB0;
	__asm
	{
		fld		[newValue]
		push	ecx
		fstp	[esp+0]
		mov		ecx, this
		push	[valueType]
		call	[callAddr]
	}
}

void Tile::UpdateString(UInt32 valueType, const char* newValue)
{
	ThisStdCall(0x58CED0, this, valueType, newValue);
}

void Tile::DoActionEnumeration()
{
	ThisStdCall(0x58BEE0, this, 1);
}

Tile * Tile::GetChildByName(const char * name)
{
	Tile * requestedTile = NULL;
	for (RefList::Node* node = childList.start; node; node = node->next)
	{
		if (node->data)
		{
//			DEBUG_PRINT("%s", node->data->name.m_data);
			if (!_stricmp(name, node->data->name.m_data))
			{
				requestedTile = node->data;
				break;
			}
		}
	}
	
	return requestedTile;
}

Tile::Value * Tile::GetValueByName(char* name)
{
	char* strtokContext = NULL;
	char * childName = strtok_s(name, "\\/", &strtokContext);
	char* nextName = NULL;
	Tile * parentTile = this;

	while (childName && parentTile)
	{
		nextName = strtok_s(NULL, "\\/", &strtokContext);
		if (!nextName)
			break;
	
		parentTile = parentTile->GetChildByName(childName);
		childName = nextName;
	}

	if (childName && !nextName && parentTile)	// childName is now name of value to retrieve
		return parentTile->GetValueByType(StrToStrID(childName));

	return NULL;
}

Tile::Value * Tile::GetValueByNameAndListIndex(char* name, UInt32 listIndex)
{
	char* strtokContext = NULL;
	char * childName = strtok_s(name, "\\/", &strtokContext);
	char* nextName = NULL;
	Tile * parentTile = this;

	while (childName && parentTile)
	{
		nextName = strtok_s(NULL, "\\/", &strtokContext);
		if (!nextName)
		{
			parentTile = parentTile->GetChildByListIndexTrait(listIndex);
			break;
		}
	
		parentTile = parentTile->GetChildByName(childName);
		childName = nextName;
	}

	if (childName && !nextName && parentTile)	// childName is now name of value to retrieve
		return parentTile->GetValueByType(StrToStrID(childName));

	return NULL;
}

// this is currently very slow due to the # of tiles and values that need to be searched
// The game probably caches the ID trait somewhere on the tile, investigate...
Tile  * Tile::GetChildByIDTrait(UInt32 idToMatch)
{
	// search children recursively
	for (RefList::Node* node = childList.start; node; node = node->next)
	{
		if (node->data)
		{
			Tile* match = node->data->GetChildByIDTrait(idToMatch);
			if (match)
				return match;
		}
	}

	// check this tile
	Tile::Value* idVal = GetValueByType(kTileValue_id);
	if (idVal && idVal->num == idToMatch)
		return this;
	else
		return NULL;
}

// should only be used on tiles that have children with listindex trait
Tile  * Tile::GetChildByListIndexTrait(UInt32 indexToMatch)
{
	// search children recursively
	for (RefList::Node* node = childList.start; node; node = node->next)
	{
		if (node->data)
		{
			Tile* match = node->data->GetChildByListIndexTrait(indexToMatch);
			if (match)
				return match;
		}
	}

	// check this tile
	Tile::Value* idVal = GetValueByType(kTileValue_listindex);
	if (idVal && idVal->num == indexToMatch)
		return this;
	else
		return NULL;
}

std::string Tile::GetQualifiedName()
{
	std::string qualifiedName;
	float parentClass;
	if (parent && !parent->GetFloatValue(kTileValue_class, &parentClass))	// i.e., parent is not a menu
		qualifiedName = parent->GetQualifiedName() + "\\";

	qualifiedName += name.m_data;

	return qualifiedName;
}

void Tile::Value::DumpExpressionList()
{
	_MESSAGE("Tile %08x %s %s", parentTile, parentTile->name.m_data, parentTile->GetType());
	gLog.Indent();

	Expression*  expr = exprList.info;
	while (expr)
	{
		_MESSAGE("%08x: prev %08x next %08x Operand: %08x Opcode: %s %04x %08x %04x (%s) ",
			expr,
			expr->prev,
			expr->next,
			expr->operand,
			StrIDToStr(expr->opcode),
			expr->unkE,
			expr->src,
			expr->src ? expr->src->id : 0,
			expr->src ? StrIDToStr(expr->src->id) : "NULL");

		expr = expr->next;
	}

	gLog.Outdent();
}