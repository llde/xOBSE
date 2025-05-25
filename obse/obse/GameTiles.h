#pragma once

#include "GameTypes.h"
#include "NiNodes.h"

/**** Tile::Value::id table ****************************************************
 *
 *	not sure how they are generated, they clearly aren't hashes
 *
 *	00000001 &false;
 *	00000001 &left;
 *	00000001 &xbox;
 *	00000001 &xboxhint;
 *	00000001 &xbuttona;
 *	00000001 &xbuttonb;
 *	00000001 &xbuttonlb;
 *	00000001 &xbuttonls;
 *	00000001 &xbuttonlt;
 *	00000001 &xbuttonrb;
 *	00000001 &xbuttonrs;
 *	00000001 &xbuttonrt;
 *	00000001 &xbuttonx;
 *	00000001 &xbuttony;
 *	00000001 &xenon;
 *	00000002 &center;
 *	00000002 &true;
 *	00000004 &right;
 *	00000065 &click_past;
 *	00000066 &no_click_past;
 *	00000067 &mixed_menu;
 *	00000068 &prev;
 *	00000069 &next;
 *	0000006A &first;
 *	0000006B &last;
 *	0000006C &xlist;
 *	0000006D &xitem;
 *	000000FF &scale;
 *	00000385 rect
 *	00000386 image
 *	00000387 text
 *	00000388 3d
 *	00000388 nif
 *	00000389 menu
 *	0000038B window
 *	000003E7 template
 *	000003E9 &MessageMenu;
 *	000003EA &InventoryMenu;
 *	000003EB &StatsMenu;
 *	000003EC &HUDMainMenu;
 *	000003ED &HUDInfoMenu;
 *	000003EE &HUDReticle;
 *	000003EF &LoadingMenu;
 *	000003F0 &ContainerMenu;
 *	000003F1 &DialogMenu;
 *	000003F2 &HUDSubtitleMenu;
 *	000003F3 &GenericMenu;
 *	000003F4 &SleepWaitMenu;
 *	000003F5 &PauseMenu;
 *	000003F6 &LockPickMenu;
 *	000003F7 &OptionsMenu;
 *	000003F8 &QuantityMenu;
 *	000003F9 &AudioMenu;
 *	000003FA &VideoMenu;
 *	000003FB &VideoDisplayMenu;
 *	000003FC &GameplayMenu;
 *	000003FD &ControlsMenu;
 *	000003FE &MagicMenu;
 *	000003FF &MapMenu;
 *	00000400 &MagicPopupMenu;
 *	00000401 &NegotiateMenu;
 *	00000402 &BookMenu;
 *	00000403 &LevelUpMenu;
 *	00000404 &TrainingMenu;
 *	00000405 &BirthSignMenu;
 *	00000406 &ClassMenu;
 *	00000408 &SkillsMenu;
 *	0000040A &PersuasionMenu;
 *	0000040B &RepairMenu;
 *	0000040C &RaceSexMenu;
 *	0000040D &SpellPurchaseMenu;
 *	0000040E &LoadMenu;
 *	0000040F &SaveMenu;
 *	00000410 &AlchemyMenu;
 *	00000411 &SpellmakingMenu;
 *	00000412 &EnchantmentMenu;
 *	00000413 &EffectSettingMenu;
 *	00000414 &MainMenu;
 *	00000415 &BreathMenu;
 *	00000416 &QuickKeysMenu;
 *	00000417 &CreditsMenu;
 *	00000418 &SigilStoneMenu;
 *	00000419 &RechargeMenu;
 *	0000041B &TextEditMenu;
 *	000007D1 copy
 *	000007D2 add
 *	000007D3 sub
 *	000007D4 mul
 *	000007D4 mult
 *	000007D5 div
 *	000007D6 rand
 *	000007D7 user
 *	000007D8 gt
 *	000007D9 gte
 *	000007DA eq
 *	000007DB lte
 *	000007DC lt
 *	000007DD min
 *	000007DE max
 *	000007DF and
 *	000007E0 or
 *	000007E1 neq
 *	000007E2 mod
 *	000007E3 floor
 *	000007E3 trunc
 *	000007E4 abs
 *	000007E5 onlyif
 *	000007E6 onlyifnot
 *	000007E7 ln
 *	000007E8 log
 *	000007E9 ceil
 *	000007EA not
 *	000007EB ref
 *	00000BB9 value
 *	00000BBA name
 *	00000BBB src
 *	00000BBC trait
 *	00000FA1 visible
 *	00000FA2 class
 *	00000FA3 listclip
 *	00000FA4 clipwindow
 *	00000FA5 stackingtype
 *	00000FA6 locus
 *	00000FA7 alpha
 *	00000FA8 id
 *	00000FA9 disablefade
 *	00000FAA listindex
 *	00000FAB depth
 *	00000FAC y
 *	00000FAD x
 *	00000FAE user0
 *	00000FAF user1
 *	00000FB0 user2
 *	00000FB1 user3
 *	00000FB2 user4
 *	00000FB3 user5
 *	00000FB4 user6
 *	00000FB5 user7
 *	00000FB6 user8
 *	00000FB7 user9
 *	00000FB8 user10
 *	00000FB9 user11
 *	00000FBA user12
 *	00000FBB user13
 *	00000FBC user14
 *	00000FBD user15
 *	00000FBE user16
 *	00000FBF user17
 *	00000FC0 user18
 *	00000FC1 user19
 *	00000FC2 user20
 *	00000FC3 user21
 *	00000FC4 user22
 *	00000FC5 user23
 *	00000FC6 user24
 *	00000FC7 user25
 *	00000FC8 clips
 *	00000FC9 target
 *	00000FCA height
 *	00000FCB width
 *	00000FCC red
 *	00000FCD green
 *	00000FCE blue
 *	00000FCF tile
 *	00000FD0 child_count
 *	00000FD0 childcount
 *	00000FD1 justify
 *	00000FD2 zoom
 *	00000FD3 font
 *	00000FD4 wrapwidth
 *	00000FD5 wraplimit
 *	00000FD6 wraplines
 *	00000FD7 pagenum
 *	00000FD8 ishtml
 *	00000FD9 cropoffsety
 *	00000FD9 cropy
 *	00000FDA cropoffsetx
 *	00000FDA cropx
 *	00000FDB menufade
 *	00000FDC explorefade
 *	00000FDD mouseover
 *	00000FDE string
 *	00000FDF shiftclicked
 *	00000FE0 focusinset
 *	00000FE1 clicked
 *	00000FE2 clickcountbefore
 *	00000FE3 clickcountafter
 *	00000FE4 clickedfunc
 *	00000FE5 clicksound
 *	00000FE6 filename
 *	00000FE7 filewidth
 *	00000FE8 fileheight
 *	00000FE9 repeatvertical
 *	00000FEA repeathorizontal
 *	00000FEB returnvalue
 *	00000FEC animation
 *	00000FED depth3d
 *	00000FEE linecount
 *	00000FEF pagecount
 *	00000FF0 xdefault
 *	00000FF1 xup
 *	00000FF2 xdown
 *	00000FF3 xleft
 *	00000FF4 xright
 *	00000FF5 xscroll
 *	00000FF6 xlist
 *	00000FF7 xbuttona
 *	00000FF8 xbuttonb
 *	00000FF9 xbuttonx
 *	00000FFA xbuttony
 *	00000FFB xbuttonlt
 *	00000FFC xbuttonrt
 *	00000FFD xbuttonlb
 *	00000FFE xbuttonrb
 *	00001001 xbuttonstart
 *	00001389 parent
 *	0000138A me
 *	0000138C sibling
 *	0000138D child
 *	0000138E screen
 *	0000138F strings
 *	00001778 &does_not_stack;
 *
 ******************************************************************************/

//NOTE: For boolean tile values, 1=false, 2=true
enum eTileValue {
	//...

	kTileValue_visible		= 0x00000FA1,
	kTileValue_class,
	kTileValue_listclip,
	kTileValue_clipwindow,
	kTileValue_stackingtype,
	kTileValue_locus,
	kTileValue_alpha,
	kTileValue_id,
	kTileValue_disablefade,
	kTileValue_listindex,
	kTileValue_depth,
	kTileValue_y,
	kTileValue_x,
	kTileValue_user0,
	kTileValue_user1,
	kTileValue_user2,
	kTileValue_user3,
	kTileValue_user4,
	kTileValue_user5,
	kTileValue_user6,
	kTileValue_user7,
	kTileValue_user8,
	kTileValue_user9,
	kTileValue_user10,
	kTileValue_user11,
	kTileValue_user12,
	kTileValue_user13,
	kTileValue_user14,
	kTileValue_user15,
	kTileValue_user16,
	kTileValue_user17,
	kTileValue_user18,
	kTileValue_user19,
	kTileValue_user20,
	kTileValue_user21,
	kTileValue_user22,
	kTileValue_user23,
	kTileValue_user24,
	kTileValue_user25,
	kTileValue_clips,
	kTileValue_target,
	kTileValue_height,
	kTileValue_width,
	kTileValue_red,
	kTileValue_green,
	kTileValue_blue,
	kTileValue_tile,
	kTileValue_childcount,
	kTileValue_child_count	= kTileValue_childcount,
	kTileValue_justify,
	kTileValue_zoom,
	kTileValue_font,
	kTileValue_wrapwidth,
	kTileValue_wraplimit,
	kTileValue_wraplines,
	kTileValue_pagenum,
	kTileValue_ishtml,
	kTileValue_cropoffsety,
	kTileValue_cropy		= kTileValue_cropoffsety,
	kTileValue_cropoffsetx,
	kTileValue_cropx		= kTileValue_cropoffsetx,
	kTileValue_menufade,
	kTileValue_explorefade,
	kTileValue_mouseover,
	kTileValue_string,
	kTileValue_shiftclicked,
	kTileValue_focusinset,
	kTileValue_clicked,
	kTileValue_clickcountbefore,
	kTileValue_clickcountafter,
	kTileValue_clickedfunc,
	kTileValue_clicksound,
	kTileValue_filename,

	//...

	kTileValue_xdefault		= 0x00000FF0,
	kTileValue_xup,
	kTileValue_xdown,
	kTileValue_xleft,
	kTileValue_xright,
	kTileValue_xscroll,
	kTileValue_xlist,
	kTileValue_xbuttona,
	kTileValue_xbuttonb,
	kTileValue_xbuttonx,
	kTileValue_xbuttony,
	kTileValue_xbuttonlt,
	kTileValue_xbuttonrt,
	kTileValue_xbuttonlb,
	kTileValue_xbuttonrb,

	//...
};

class Menu;

// 40
class Tile
{
public:
	Tile();
	~Tile();

	void	DebugDump(void);

	static const char *	StrIDToStr(UInt32 id);
	static UInt32		StrToStrID(const char * str);

	virtual void			Destructor(void);
	virtual void			Unk_01(UInt32 unk0, const char * str, UInt32 unk2);	// initialize? doesn't read data
	virtual NiNode *		Unk_02(void);	// create render objects?
	virtual UInt32			GetTypeID(void);
	virtual const char *	GetType(void);
	virtual UInt32			UpdateField(UInt32 valueID, float floatValue, const char* strValue);	//checks for equality, doesn't always update...
	virtual void			Unk_06(void);	// does something with tile's NiNode

	// 1C
	struct Value
	{
		union Operand {
			Value	* ref;
			float	* immediate;
		};

		struct Expression
		{
			Expression		* prev;
			Expression		* next;
			Operand			operand;	// how does it tell if it's ref or immediate?
			UInt16			opcode;		// i.e. 7D1 "copy", 7D2 "add", etc
			UInt16			unkE;
			Value			* src;
		};

		// linked list of compiled xml expression. Preliminary!
		struct ExpressionList {
			Expression		* info;
			ExpressionList	* next;
		};

		bool IsNum() { return bIsNum == 1; }
		bool IsString() { return bIsNum == 0; }
		void DumpExpressionList();

		Tile	* parentTile;	// 00 - Tile * back to owning tile
		float	num;			// 04
		BSStringT	str;			// 08
		ExpressionList	exprList;	// 10
		UInt16	id;				// 18
		UInt8	bIsNum;			// 1A 0 = string, 1 = number
		UInt8	pad1B;			// 1B
	};

	typedef NiTListBase <Tile>	RefList;
	typedef NiTListBase <Value>	ValueList;

	Tile*	ReadXML(const char * xmlPath);
	Tile*	GetRoot(void);
	Menu*   GetContainingMenu();


	Value * GetValueByType(UInt32 valueType);
	Value * GetValueByName(char * name);
	Value * GetValueByNameAndListIndex(char * name, UInt32 indexToMatch);
//	bool	SetValueByName(char* name, const char* strVal, float floatVal);
	Tile  * GetChildByName(const char * name);
	Tile  * GetChildByIDTrait(UInt32 idToMatch);	// find child with <id> trait matching idToMatch
	Tile  * GetChildByListIndexTrait(UInt32 indexToMatch); // find child with <listindex> trait matching indexToMatch
	bool GetFloatValue(UInt32 valueType, float* out);
	bool SetFloatValue(UInt32 valueType, float newValue);
	bool GetStringValue(UInt32 valueType, const char** out);
	bool SetStringValue(UInt32 valueType, const char* newValue);
	bool DeleteValue(UInt32 valueType);
	void UpdateString(UInt32 valueType, const char* newValue);		// sets field and updates display
	void UpdateFloat(UInt32 valueType, float newValue);				// ditto for floats
	void DoActionEnumeration();
	std::string GetQualifiedName();

	enum {
		kTileFlag_ChangedXY				= 0x01,
		kTileFlag_ChangedString			= 0x02,
		kTileFlag_ChangedWidthHeight	= 0x10,
		kTileFlag_ChangedFilename		= 0x20
	};

//	void	** _ctor;		// 00
	UInt8	unk04;			// 04 - 0 = base tile
	UInt8	unk05;			// 05 - looks like bool, possibly bModified? Seems only used for x, y, width, height changed
	UInt8	unk06;			// 06
	UInt8	pad07;			// 07
	BSStringT	name;			// 08
	Tile	* parent;		// 10
	ValueList	valueList;	// 14
	UInt32	unk24;			// 24	// NiNode *
	void	* unk28;		// 28
	UInt32	flags;			// 2C
	RefList	childList;		// 30
};

// 44
class TileRect : public Tile
{
public:
	TileRect();
	~TileRect();

	enum
	{
		kID =	0x0385
	};

	UInt32	unk40;	// 40
};

// 4C
class TileImage : public Tile
{
public:
	TileImage();
	~TileImage();

	enum
	{
		kID =	0x0386
	};

	float	unk40;		// 40
	NiNode	* unk44;	// 44
	UInt8	unk48;		// 48
	UInt8	pad49[3];	// 49
};

// 5C
class Tile3D : public Tile
{
public:
	Tile3D();
	~Tile3D();

	enum
	{
		kID =	0x0388
	};

	UInt32	unk40;	// 40
	UInt32	unk44;	// 44
	UInt32	unk48;	// 48
	UInt16	unk4C;	// 4C
	UInt16	unk4E;	// 4E
	UInt32	unk50;	// 50
	UInt16	unk54;	// 54
	UInt16	unk56;	// 56
	float	unk58;	// 58 - initialized to -1
};

// 54
class TileText : public Tile
{
public:
	TileText();
	~TileText();

	enum
	{
		kID =	0x0387
	};

	UInt32	pad40[4];	// 40
	UInt8	unk50;		// 50
	UInt8	pad51[3];	// 51
};

// 48
class TileMenu : public TileRect
{
public:
	TileMenu();
	~TileMenu();

	enum
	{
		kID =	0x0389
	};

	Menu	* menu;	// 44
};

// 40
class TileWindow : public Tile
{
public:
	TileWindow();
	~TileWindow();

	enum
	{
		kID =	0x038B
	};
};

// doesn't work for getting TileRect from TileMenu
template <class T>
T * tile_cast(Tile * src)
{
	T	* result = NULL;

	if(src && (src->GetTypeID() == T::kID))
		result = static_cast <T *>(src);

	return result;
}
