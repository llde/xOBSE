#pragma once
#include "GameTiles.h"

class TESObjectREFR;
class EnchantmentItem;
class TESForm;
class TESObjectBOOK;
class TileMenu;
class TileText;
class TileImage;
class TileRect;
class SpellItem;
class Character;
class ValueModifierEffect;
struct BaseExtraList;
class TESClass;
class TESDescription;

enum {
	kMenuType_None = 0,			// for gamemode
	kMenuType_BigFour,			// F1 - F4 menus

	kMenuType_Message = 0x3E9,
	kMenuType_Inventory,
	kMenuType_Stats,
	kMenuType_HUDMain,
	kMenuType_HUDInfo,
	kMenuType_HUDReticle,
	kMenuType_Loading,
	kMenuType_Container,
	kMenuType_Dialog,
	kMenuType_HUDSubtitle,
	kMenuType_Generic,
	kMenuType_SleepWait,
	kMenuType_Pause,
	kMenuType_LockPick,
	kMenuType_Options,
	kMenuType_Quantity,
	kMenuType_Audio,
	kMenuType_Video,
	kMenuType_VideoDisplay,
	kMenuType_Gameplay,
	kMenuType_Controls,
	kMenuType_Magic,
	kMenuType_Map,
	kMenuType_MagicPopup,
	kMenuType_Negotiate,
	kMenuType_Book,
	kMenuType_LevelUp,
	kMenuType_Training,
	kMenuType_BirthSign,
	kMenuType_Class,
	kMenuType_unk407,
	kMenuType_Skills,
	kMenuType_unk409,
	kMenuType_Persuasion,
	kMenuType_Repair,
	kMenuType_RaceSex,
	kMenuType_SpellPurchase,
	kMenuType_Load,
	kMenuType_Save,
	kMenuType_Alchemy,
	kMenuType_Spellmaking,
	kMenuType_Enchantment,
	kMenuType_EffectSetting,
	kMenuType_Main,
	kMenuType_Breath,
	kMenuType_QuickKeys,
	kMenuType_Credits,
	kMenuType_SigilStone,
	kMenuType_Recharge,
	kMenuType_unk41A,
	kMenuType_TextEdit,

	kMenuType_Max
};

extern NiTArray<TileMenu*> * g_TileMenuArray;
Menu * GetMenuByType(UInt32 menuType);

// 028
class Menu
{
public:
	Menu();
	~Menu();

	virtual void	Destructor(bool arg0);		// pass false to free memory
	virtual void	SetField(UInt32 idx, Tile* value);
	virtual void	Unk_02(UInt32 arg0, UInt32 arg1);	// show menu?
	virtual void	HandleClick(UInt32 buttonID, Tile* clickedButton); // buttonID = <id> trait defined in XML
	virtual void	HandleMouseover(UInt32 arg0, Tile * activeTile);	//called on mouseover, activeTile is moused-over Tile
	virtual void	Unk_05(UInt32 arg0, UInt32 arg1);
	virtual void	Unk_06(UInt32 arg0, UInt32 arg1, UInt32 arg2);
	virtual void	Unk_07(UInt32 arg0, UInt32 arg1, UInt32 arg2);
	virtual void	Unk_08(UInt32 arg0, UInt32 arg1);
	virtual void	Unk_09(UInt32 arg0, UInt32 arg1);
	virtual void	Unk_0A(UInt32 arg0, UInt32 arg1);
	virtual void	Unk_0B(void);
	virtual bool	HandleKeyboardInput(char inputChar);	//for keyboard shortcuts, return true if handled
	virtual UInt32	GetID(void);
	virtual bool	Unk_0E(UInt32 arg0, UInt32 arg1);

//	void	** _vtbl;	// 00
	TileMenu* tile;		// 04
	UInt32	unk08;		// 08
	UInt32	unk0C;		// 0C
	UInt32	unk10;		// 10
	UInt32	unk14;		// 14
	UInt32	unk18;		// 18
	UInt32	unk1C;		// 1C - initialized to 1
	UInt32	id;			// 20 - uninitialized
	UInt32	unk24;		// 24 - initialized to 4, is 8 if enabled?

	void	RegisterTile(TileMenu * tileMenu);
	void	EnableMenu(bool unk);
	Tile *  GetComponentByName(const char* componentPath);
};

// 64
class MessageMenu : public Menu
{
public:
	MessageMenu();
	~MessageMenu();

	enum {
		kButtonID_Button1 =		0x04,
		//	...
		kButtonID_Button10 =	0x0C,
		kButtonID_Close	=		0x1F	// not actually a button
	};

	TileRect		* backGround;		// 28
	TileText		* messageText;		// 2C
	TileRect		* focusBox;			// 30
	TileImage		* buttons[10];		// 34..58 each has a child TileText
	void			* buttonCallback;	// 5C
	UInt8			minButtonIndex;		// 60
	UInt8			pad61[3];			// 61

	bool IsScriptMessageBox();
};

// 58
class QuantityMenu : public Menu
{
public:
	QuantityMenu();
	~QuantityMenu();

	TileRect		* background;		// 28
	TileText		* display_text;		// 2C
	TileImage		* quantity_scroll;	// 30 user7 trait of this tile holds currently selected quantity
	TileImage		* horizontal_scroll_marker;	// 34
	TileImage		* button_okay;		// 38
	TileImage		* button_cancel;	// 3C
	UInt32			maxQuantity;		// 40
	UInt32			unk44;				// 44 is 0x33 if created from ContainerMenu (hard-coded value) - why 0x33?
	UInt32			unk48;				// 48 always == maxQuantity? Not currently selected quantity
	Tile			* itemTile;			// 4C Tile of selected item in container menu
	UInt32			unk50;				// 50 init'd to arg3 of CreateQuantityMenu()
	UInt32			unk54;				// 54
};

enum {
	kFilterInv_Weapons	= 1 << 0,
	kFilterInv_Armor	= 1 << 1,
	kFilterInv_Alchemy	= 1 << 2,
	kFilterInv_Misc		= 1 << 3,
	kFilterInv_All		= 31
};

// 58
class InventoryMenu : public Menu
{
public:
	InventoryMenu();
	~InventoryMenu();

	TileRect		* focusBox;			//028
	TileRect		* listContents;		//02C
	TileRect		* scrollBar;		//030
	TileImage		* scrollMarker;		//034
	TileRect		* invP4P5Header;	//038 - ?
	UInt32			unk03C;				//03C
	UInt8			filterType;			//040 init'd to 1F (all), 1=weapons, 2=armor, ...
	UInt8			pad041[3];
	UInt8			unk044;				//044 init'd to FF
	UInt8			unk045[3];
	float			unk048;				//048
	UInt32			unk04C;				//04C
	UInt32			unk050;				//050
	UInt32			unk054;				//054 uninitialized
};

//68
class ContainerMenu : public Menu
{
public:
	ContainerMenu();
	~ContainerMenu();

	enum {
		kContValue_CurrentTab		= kTileValue_user0,
		kContValue_WeaponHeaderPos,
		kContValue_ApparelHeaderPos,
		kContValue_AlchemyHeaderPos,
		kContValue_MiscHeaderPos,
		kContValue_NumItemsInList,
		kContValue_Gold,
		kContValue_IsContainerMode,			//false if looking at inventory
		kContValue_CanTakeAll,				//false if "Take All" button hidden
		kContValue_CanNegotiate,
		kContValue_MagicPopupXPos,
		kContValue_NPCName,
		kContValue_BarterGoldBase,
		kContValue_CurrentEncumbrance,
		kContValue_MaxEncumbrance,
	};

	TileImage			* scrollBar;		//028
	TileImage			* scrollMarker;		//02C
	TileRect			* listContents;		//030
	TileRect			* focusBox;			//034
	TileRect			* invP4P5Header;	//038
	TileRect			* selectedItemTile;	//03C user11 = idx of item in inventory/container
	UInt8				filterType;			//040
	UInt8				pad041[3];
	TESObjectREFR		* refr;				//044
	UInt32				unk048;				//048
	UInt32				unk04C;				//04C
	float				unk050;				//050
	UInt8				unk054;				//054
	UInt8				bTransactionInProgress;	// set true after first transaction confirmed by player. ###TODO: reset when?
	UInt8				unk056;				// may be padding
	UInt8				pad057;
	UInt32				unk058;				//058
	UInt32				unk05C;				//05C
	UInt8				unk060;				//060
	bool				isBarter;			//	 1 if bartering with merchant
	UInt8				unk062;
	UInt8				unk063;
	bool				isContainerContents; //init'd to 1. 0 when switched to player's inventory view
	UInt8				pad065[3];

	UInt32		GetItemIndex();
	TESForm*	GetItem();

	static UInt32 GetQuantity();
	static void Update();
};

// 5C
class HUDInfoMenu : public Menu
{
public:
	HUDInfoMenu();
	~HUDInfoMenu();

	enum {
		kHUDInfoValue_ActionText		= kTileValue_user0,
		kHUDInfoValue_Unk01,				// default 128
		kHUDInfoValue_XboxOnly02,
		kHUDInfoValue_Unk03,				//default 20
		kHUDInfoValue_IsTelekinesisActive,
		kHudInfoValue_XboxOnly05,
	};

	TileText		* name;				// 028
	TileText		* valueText;		// 02C
	TileText		* weightText;		// 030
	TileText		* damageText;		// 034
	TileText		* armorText;		// 038
	TileText		* qualityText;		// 03C
	TileText		* healthText;		// 040
	TileText		* usesText;			// 044
	TileText		* destinationText;	// 048
	TileText		* lockText;			// 04C
	TileImage		* actionIcon;		// 050
	TESObjectREFR	* crosshairRef;		// 054
	UInt32			unk058;				// 058
};

extern HUDInfoMenu**	g_HUDInfoMenu;

// 034
class TextEditMenu : public Menu
{
public:
	TextEditMenu();
	~TextEditMenu();

	enum {
		kTextEditValue_Prompt	= kTileValue_user0,
	};

	// fields
	TileText*	text;				// 028
	TileImage*	unk02C;				// 02C
	TileImage*	unk030;				// 030
};

// 0A0
class EnchantmentMenu : public Menu
{
public:
	EnchantmentMenu();
	~EnchantmentMenu();

	struct Unk0
	{
		void*		unk0;
		UInt32		unk1;	// 1
		TESForm*	form;
	};

	enum {							// these are specified by <id> tags in xml
									// passed to SetField to initialize component tiles, etc
		kField_EnchantName			= 0x02,
		kField_UsesIcon,
		kField_SoulCostIcon,
		kField_GoldValue,
		kField_KnownEffectPane,
		kField_AddedEffectPane,
		kField_FocusBox,
		kField_KnownEffectScrollbar,
		kField_KnownEffectScrollmarker,
		kField_AddedEffectScrollmarker,
		kField_AddedEffectScrollbar,
		kField_PlayerGoldValue,
		kField_CreateButton,
		kField_ExitButton,
		kField_KnownEffectsText,			// 0x10
		kField_AddedEffectsText,
		kField_EnchItemRect			= 0x14,
		kField_SoulGemRect			= 0x16,
		kField_EnchItemIcon			= 0x19,
		kField_SoulGemIcon			= 0x1A,
	};

	// fields
	EnchantmentItem*	enchantItem;			// 028 - temp! not the item player gets when enchantment is finished
	Unk0*				soulGemInfo;			// 02C
	Unk0*				enchantableInfo;		// 030 - enchantableInfo->form == unenchanted item
	void*				unk034;					// 034
	UInt32				cost;					// 038
	TileText*			enchantNameTile;		// 03C
	TileRect*			nameBackground;			// 040
	TileImage*			usesIcon;				// 044
	TileText*			goldValue;				// 048
	TileText*			playerGoldValue;		// 04C
	TileImage*			soulCostIcon;			// 050
	TileRect*			knownEffectPane;		// 054
	TileRect*			addedEffectPane;		// 058
	TileRect*			focusBox;				// 05C
	TileImage*			knownEffectScrollBar;	// 060
	TileImage*			knownEffectScrollMarker;// 064
	TileImage*			addedEffectScrollBar;	// 068
	TileImage*			addedEffectScrollMarker;// 06C
	TileImage*			createButton;			// 070
	TileImage*			exitButton;				// 074
	TileText*			knownEffectsText;		// 078
	TileText*			addedEffectsText;		// 07C
	TileRect*			enchItemRect;			// 080
	TileRect*			soulgemRect;			// 084
	TileImage*			soulgemIcon;			// 088
	TileImage*			enchItemIcon;			// 08C
	void				* unk090;				// 090 pointer to some struct with EffectSetting info
	TileRect			* unk094;				// 094 - active tile?
	BSStringT				enchantName;			// 098 as entered by player
};
STATIC_ASSERT(sizeof(EnchantmentMenu) == 0x0A0);

//3C
class BookMenu : public Menu
{
public:
	BookMenu();
	~BookMenu();

	enum {			//user values for book menu
		kBookValue_IsBook			= kTileValue_user0, //false if scroll
		kBookValue_NumTotalLines	= kTileValue_user1,
		kBookValue_Text				= kTileValue_user2,	//string
		kBookValue_CanBeTaken		= kTileValue_user3,
		kBookValue_PageWidth		= kTileValue_user4,
		kBookValue_Font				= kTileValue_user5,
		kBookValue_ForceAdjust		= kTileValue_user6,
		kBookValue_LineHeight		= kTileValue_user7,
		kBookValue_PageTurnSound	= kTileValue_user8,
		kBookValue_CurrentPageNumber= kTileValue_user9,
	};

	enum {
		kButtonID_Exit =	0x1F,
		kButtonID_Take =	0x20		// Next and Prev buttons not handled by HandleClick()...
	};

	TileImage		* scrollBackground;		//028
	TileImage		* bookBackground;		//02C
	TESObjectREFR	* bookRef;				//030
	TESObjectBOOK	* book;					//034

	void UpdateText(const char* newText);
};

// this appears to cache the most recently read book
struct BookMenuData {
	TESDescription		* description;	//00
	const char			* bookHTML;		//04
	UInt16				htmlLength;		//08
	UInt16				unk10;			//10 seems always set in conjunction with htmlLength and to same value
};
//global BookMenuData* 0x00B33C04 in v1.2

struct MenuSpellList
{
	SpellItem		* spell;
	MenuSpellList	* next;

	SpellItem* Info() const	{ return spell;	}
	MenuSpellList* Next() const	{ return next;	}
};

typedef Visitor<MenuSpellList, SpellItem> MenuSpellListVisitor;

struct MagicItemAndIndex
{
	struct MagicItemData {
		void		* unk00;		// appears to be pointer to ExtraDataList / EntryExtendData for object
		UInt32		count;
		TESForm		* object;
	};

	MagicItemData	* data;
	UInt32			index;
};

//5C
class MagicMenu : public Menu
{
public:
	MagicMenu();
	~MagicMenu();

	enum {
		kMagicValue_CurrentTab				= kTileValue_user0,
		kMagicValue_GreaterPowerHeaderPos,
		kMagicValue_LesserPowerHeaderPos,
		kMagicValue_SpellsHeaderPos,
		kMagicValue_ScrollsHeaderPos,
		kMagicValue_NumItemsInList,
		kMagicValue_MagicEffectiveness,
		kMagicValue_PopupXPos,
		kMagicValue_IsSelectedItemEquipped	= kTileValue_user11,
		kMagicValue_PopupMinDrop,
		kMagicValue_BackgroundDepth		= kTileValue_user23,
	};

	enum {
		kFilter_Target	= 1 << 0,
		kFilter_Touch	= 1 << 1,
		kFilter_Self	= 1 << 2,
		kFilter_Active  = 1 << 3,
		kFilter_All		= 7,
		kFilter_ActiveEffects = 8
	};

	struct ActiveEffectData {
		ValueModifierEffect	* effectMod;
		UInt32				magnitude;
		//might be 1 more (duration?)
	};

	struct ActiveEffectEntry {
		ActiveEffectData	* data;
		ActiveEffectEntry	* next;

		ActiveEffectData* Info() const;
		ActiveEffectEntry* Next() const;
	};

	TileRect			* focusBox;			//028
	TileRect			* listContents;		//02C
	TileImage			* scrollBar;		//030
	TileImage			* scrollMarker;		//034
	MenuSpellList		spells;				//038
	ActiveEffectEntry	activeEffects;		//040
	UInt32				unk048;				//048
	TileRect			activeSpell;		//04C player's current spell
	UInt32				filterType;			//050 init'd to 7
	UInt32				unk054;				//054
	float				unk058;				//058

	TESForm	* GetMagicItemForIndex(UInt32 idx);
	static NiTListBase<MagicItemAndIndex> * GetMagicItemList();
};

typedef Visitor<MagicMenu::ActiveEffectEntry, MagicMenu::ActiveEffectData> MenuActiveEffectVisitor;

//68
class SpellPurchaseMenu : public Menu
{
public:
	SpellPurchaseMenu();
	~SpellPurchaseMenu();

	TileImage		* unk028;		//028
	TileRect		* unk02C;		//02C
	TileImage		* unk030[4];	//030 .. 03C
	TileText		* unk040;		//040
	TileRect		* unk044;		//044
	TileRect		* unk048;		//048
	UInt32			unk04C;			//04C
	Character		* spellMerchant;//050
	UInt32			unk054;			//054
	UInt32			unk058;			//058
	UInt8			unk05C;			//05C
	UInt8			pad05D[3];
	MenuSpellList	spells;			//060

	void Update();
};

template <typename T>
class RepairMenuList : public NiTPointerListBase <T>
{		//list of repair item and index
public:
	RepairMenuList();
	~RepairMenuList();
};

//78
class RepairMenu : public Menu
{
public:
	RepairMenu();
	~RepairMenu();

	struct RepairMenuItemAndIndex {
		BaseExtraList	* extraList;
		UInt32			unk04;		//index?
		TESForm			* object;
	};

	enum						// RepairMenu is used to select an item from inventory.
	{							// Which type of selection are we currently doing?
		kSelect_Repair = 1,
		kSelect_RepairBuy,
		kSelect_Alchemy,
		kSelect_Enchantment,
		kSelect_Soulgem,
		kSelect_Sigilstone
	};

	enum {
		kValue_SelectionType		= kTileValue_user0,
	};

	enum {
		kButton_Exit			= 2,
		kButton_Remove			= 15,
		kButton_RepairAll		= 16,
		kButton_Filter			= 17
	};

	UInt32			unk028;		//028
	TileImage		* unk02C;	//02C
	TileImage		* unk030;	//030
	TileText		* unk034;	//034
	TileImage		* unk038;	//038
	TileRect		* unk03C;	//03C
	TileImage		* unk040;	//040
	TileRect		* unk044;	//044
	UInt32			unk048;		//048
	TileImage		* unk04C;	//04C
	TileImage		* unk050;	//050
	UInt32			unk054;		//054
	UInt32			selectionType;	//058 from kSelect_XXX enum, init'd to 1
	UInt32			unk05C;		//05C
	Character		* armorer;	//060
	UInt8			unk064;		//064
	UInt8			unk065;
	UInt8			pad066[2];
	RepairMenuList<RepairMenuItemAndIndex>	repairMenuList;	//068
};

//54
class RechargeMenu : public Menu
{
public:
	RechargeMenu();
	~RechargeMenu();

	TileRect		* unk028;		//028
	TileImage		* unk02C[3];	//02C .. 034
	TileRect		* unk038;		//038
	UInt32			unk03C;			//03C
	UInt32			unk040;			//040
	UInt32			unk044;			//044
	Character		* recharger;	//048
	UInt32			unk04C;			//04C
	UInt8			unk050;			//050 init'd to FF
	UInt8			pad051[3];
};

//98+
class DialogMenu : public Menu
{
public:
	DialogMenu();
	~DialogMenu();

	TileRect			* topicsScrollPane;		//028
	TileText			* dialogText;			//02C
	TileImage			* goodbyeButton;		//030
	TileRect			* dialogTextLayout;		//034
	TileRect			* focusBox;				//038
	TileRect			* topics;				//03C
	TileImage			* persuasion;			//040
	TileImage			* scrollBar;			//044
	TileImage			* scrollMarker;			//048
	TileImage			* barterButton;			//04C
	TileImage			* trainButton;			//050
	TileImage			* rechargeButton;		//054
	TileImage			* repairButton;			//058
	TileImage			* spellsButton;			//05C
	Character			* speaker;		//060
	UInt32				unk064;			//064
	UInt32				unk068;			//068
	UInt32				unk06C;			//06C init'd to 1
	UInt32				unk070;			//070
	UInt32				unk074;			//074
	UInt32				unk078;			//078
	UInt8				unk07C;			//07C
	UInt8				pad07D[3];
	UInt32				unk080;			//080 init'd to 1
	UInt32				unk084;			//084
	UInt8				unk088;			//088
	UInt8				pad087[3];
	BSStringT				str08C;			//08C
	UInt8				unk094[3];		//094
	UInt8				pad097;
};

class AlchemyItem;
class TESObjectAPPA;
class IngredientItem;

//C0
class AlchemyMenu : public Menu
{
public:
	AlchemyMenu();
	~AlchemyMenu();

	struct ApparatusInfo {
		BaseExtraList	** extraList;
		UInt8			unk4;			// quality?
		TESObjectAPPA	* apparatus;
	};

	struct EffectData {
		UInt32			effectCode;
		UInt32			unk4;			// magnitude?
	};

	struct EffectEntry {
		EffectData	* data;
		EffectEntry	* next;
	};

	struct IngredientInfo {
		BaseExtraList		** extraList;
		UInt32				count;
		IngredientItem		* ingredient;
	};

	struct Unk0A0 {
		UInt8	unk00;
		UInt8	pad01[3];
		UInt32	unk04;
		UInt8	unk08;
		UInt8	pad09[3];
		UInt32	unk0C;			//init'd to -1
		UInt32	unk10;
		UInt32	unk14;			//init'd to 1
		char	* effectName18;
		UInt16	unk1C;
		UInt16  unk1E;
		char	* effectName20;
		UInt16	unk24;
		UInt16	unk26;
	};

	TileRect			* nameBackground;		//028
	TileText			* nameText;				//02C
	TileImage			* appaIcons[4];			//030 .. 03C
	TileRect			* ingredRects[4];		//040 .. 04C
	TileRect			* effectList;			//050
	TileRect			* focusBox;				//054
	TileImage			* createButton;			//058
	TileImage			* exitButton;			//05C
	TileImage			* scrollBar;			//060
	TileImage			* scrollMarker;			//064
	TileImage			* ingredIcons[4];		//068..074
	ApparatusInfo		* apparatus[4];			//078 .. 084
	float				unk088;					//088
	UInt32				unk08C;					//08C
	TileRect			* unk090;				//090	- active tile?
	AlchemyItem			* potion;				//094
	float				unk098;					//098
	UInt32				unk09C;					//09C
	Unk0A0				* unk0A0;				//0A0
	UInt8				unk0A4;					//0A4
	UInt8				unk0A5;					//	  - not initialized
	UInt8				unk0A6;					//    - init'd to 0xFF
	UInt8				unk0A7;
	EffectEntry			effects;				//0A8 .. 0AC
	IngredientInfo		* ingreds[4];			//0B0 .. 0BC

	IngredientItem* GetIngredientItem(UInt32 whichIngred);
	TESObjectAPPA* GetApparatus(UInt32 whichAppa);
	UInt32 GetIngredientCount(UInt32 whichIngred);
};

STATIC_ASSERT(sizeof(AlchemyMenu) == 0xC0);
STATIC_ASSERT(offsetof(AlchemyMenu, effects) == 0x0A8);

class TESWorldSpace;

//100
class MapMenu : public Menu
{
public:
	MapMenu();
	~MapMenu();

	enum {
		kMapValue_LocationName	= kTileValue_user1,
		kMapValue_Date,								// (as string)
		kMapValue_QuestListTitle,
		kMapValue_Scale,
	};

	//TODO: How to get currently selected mapmarker??
	// worldIconPaper:string is marker name
	// game converts map icon coords to world coords, then walks mapmarker list
	// compares names, coords until it finds a match
	// only does this when clicking on an icon to travel, otherwise would be painfully inefficient
	// can get map coords for active icon tile from user1 and user2
	struct MapMarkerEntry {
		TESObjectREFR	* mapMarker;
		MapMarkerEntry	* next;

		TESObjectREFR	* Info() const { return mapMarker;	}
		MapMarkerEntry	* Next() const { return next;		}
	};

	TileImage			* background;				//028
	TileRect			* pageTabTargets[5];		//02C .. 03C
	TileImage			* journalScrollBar;			//040
	TileImage			* journalScrollMarker;		//044
	TileRect			* journalPane;				//048
	TileRect			* focusBox;					//04C
	TileImage			* journalMapButton;			//050
	TileImage			* journalBackButton;		//054
	TileImage			* worldMap;					//058
	TileImage			* worldIconPaper;			//05C
	TileImage			* localLayout;				//060
	TileRect			* localMap;					//064
	TileRect			* localIcons;				//068
	TileImage			* worldCursor;				//06C
	TileImage			* localCursor;				//070
	TileRect			* worldWindow;				//074
	UInt32				unk078[3];					//078 .. 080
	UInt8				unk084;						//084 - init'd to -1
	UInt8				pad085[3];
	float				unk088;						//088
	float				unk08C;						//08C
	SInt32				unk090;						//090
	SInt32				unk094;						//094
	SInt32				worldMapWidth;				//098 unk098 thru unk0AC used in converting map coords to world coords
	SInt32				worldMapHeight;				//09C yes, signed. add 4.2949673e9 when converting if negative
	SInt32				unk0A0;						//0A0
	UInt32				unk0A4;						//0A4 signed?
	SInt32				unk0A8;						//0A8
	UInt32				unk0AC;						//0Ac signed?
	BSStringT				destinationName;			// 0B0 updated only when clicking on a marker, not on mouseover
	float				targetX;					// 0B8 x, y of selected map marker icon (NOT world coords)
	float				targetY;					// 0BC these only updated on click
	UInt32				unk0C0;						// 0C0
	MapMarkerEntry		* mapMarkers;				//0C4
	void				* unk0C8;					//0C8
	UInt32				unk0CC;						//0CC
	TESWorldSpace		* worldSpace;				//0D0
	UInt32				unk0D4;						//0D4
	UInt32				unk0D8;						//0D8
	UInt32				unk0DC;						//0DC
	TileImage			* unk0E0;					//0E0
	UInt32				unk0E4[5];					//0E4 .. 0F4
	TileImage			* unk0F8;					//0F8
	TileImage			* unk0FC;					//0FC

	const char* GetSelectedMarkerName();
	TESObjectREFR* GetSelectedMarker();
	void UpdateMarkerName(TESObjectREFR* mapMarker, const char* newName);
};

typedef Visitor<MapMenu::MapMarkerEntry, TESObjectREFR> MapMarkerEntryVisitor;
STATIC_ASSERT(sizeof(MapMenu) == 0x100);

//08C
class ClassMenu : public Menu
{
public:
	ClassMenu();
	~ClassMenu();

	//user1..7: major skill names	|	user11..17 major skill actor values
	//user8:	specialization name |	user18: specialization actor value
	//user9..10: favored attributes |	user19..20: favored attribute actor values

	TileRect		* unk028;					//028
	TileImage		* unk02C;					//02C
	TileImage		* unk030;					//030
	TileRect		* unk034;					//034
	UInt32			unk038;						//038
	TESClass		* selectedClass;			//03C
	TESClass		* class040;					//040
	UInt32			unk044[(0x8C - 0x44) >> 2];	//044..088
};

STATIC_ASSERT(sizeof(ClassMenu) == 0x8C);

//05C
class MagicPopupMenu : public Menu
{
public:
	MagicPopupMenu();
	~MagicPopupMenu();

	TileRect		* background;				//028
	TileRect		* effect1;					//02C
	TileRect		* effect2;					//030
	TileRect		* effect3;					//034
	TileRect		* effect4;					//038
	TileRect		* effect5;					//03C
	TileRect		* effect6;					//040
	TileRect		* effect7;					//044
	TileRect		* effect8;					//048
	TileRect		* backgroundbottom;			//04C
	UInt32			endPosX;					//050 always the same within a UI setup, sum of active window's x and width
	UInt32			startPosX;					//054 gets lower as the popup width increases (since it needs to start earlier in order to be hidden at the start)
	UInt8			state;						//058 seen: 0 for displayed, 1 for to-display, 2 for hidden, 3 for to-hide.
	UInt8			pad058[3];					//
	UInt32			unk05C;						//05C
};

STATIC_ASSERT(sizeof(MagicPopupMenu) == 0x60);