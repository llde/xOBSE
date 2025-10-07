#pragma once

#include "obse/GameForms.h"
#include "obse/GameObjects.h"


// derived from NiFile, which derives from NiBinaryStream
// 154
class BSFile
{
public:
	BSFile();
	~BSFile();

	virtual void	Destructor(bool freeMemory);				// 00
	virtual void	Unk_01(void);								// 04
	virtual void	Unk_02(void);								// 08
	virtual void	Seek(SInt32 offset, UInt32 origin); //(void);								// 0C
	virtual void	Unk_04(void);								// 10
	virtual void	DumpAttributes(NiTArray <char *> * dst);	// 14
	virtual UInt32	GetSize(void);								// 18
	virtual void	Unk_07(void);								// 1C  //Check SkyBSA def.
	virtual void	Unk_08(void);								// 20
	virtual void	Unk_09(void);								// 24
	virtual void	Unk_0A(void);								// 28
	virtual void	Unk_0B(void);								// 2C
	virtual void	Unk_0C(void);								// 30
	virtual void	Unk_Read(void* destination, UInt32 sizeToRead);								// 34
	virtual void	Unk_Write(void);							// 38

	//	void	** m_vtbl;		// 000
	void	* m_readProc;	// 004 - function pointer
	void	* m_writeProc;	// 008 - function pointer
	UInt32	m_bufSize;		// 00C
    UInt32 lastReadSize; // 010 // return value of fread, fread_s, and friends, when we're reading into our own buffer
    UInt32	m_unk014;		// 014
	void	* m_buf;		// 018
	FILE	* m_file;		// 01C
	UInt32	m_writeAccess;	// 020
	UInt8	m_good;			// 024
	UInt8	m_pad025[3];	// 025
	UInt8	m_unk028;		// 028
	UInt8	m_pad029[3];	// 029
	UInt32	m_unk02C;		// 02C
	UInt32	m_pos;			// 030
	UInt32	m_unk034;		// 034
	UInt32	m_unk038;		// 038
	char	m_path[0x104];	// 03C
	UInt32	m_unk140;		// 140
	UInt32	m_unk144;		// 144
	UInt32	m_pos2;			// 148 - used if m_pos is 0xFFFFFFFF
	UInt32	m_unk14C;		// 14C
	UInt32	m_fileSize;		// 150
};
static_assert(sizeof(BSFile) == 0x154, "Size Not Matching" );

enum BSAFlags : UInt32 {
	kBSAFlag_HasFolderNames  = 0x0001,
	kBSAFlag_HasFileNames    = 0x0002,
	kBSAFlag_Compressed      = 0x0004,
	kBSAFlag_Unk0008         = 0x0008, // related to retaining directory strings/offsets
	kBSAFlag_RetainFilenameStrings = 0x0010,
	kBSAFlag_RetainFilenameOffsets = 0x0020,
	kBSAFlag_IsXbox360Archive      = 0x0040,
	kBSAFlag_Unk0080         = 0x0080, // related to retaining directory strings/offsets
	kBSAFlag_Unk0100         = 0x0100,
	kBSAFlag_Unk0200         = 0x0200,
	kBSAFlag_Unk0400         = 0x0400,
};
enum BSAFiletype : UInt32 {
	kBSAFiletype_Meshes   = 0,
	kBSAFiletype_Textures = 1,
	kBSAFiletype_Menus    = 2,
	kBSAFiletype_Sounds   = 3,
	kBSAFiletype_Voices   = 4,
	kBSAFiletype_Shaders  = 5,
	kBSAFiletype_Trees    = 6,
	kBSAFiletype_Fonts    = 7,
	kBSAFiletype_Misc     = 8,
	//
	kBSAFiletype_COUNT    = 9,
};
enum BSAFiletypeFlags : UInt32 {
	kBSAFiletypeFlag_Meshes   = 0x0001,
	kBSAFiletypeFlag_Textures = 0x0002,
	kBSAFiletypeFlag_Menus    = 0x0004,
	kBSAFiletypeFlag_Sounds   = 0x0008,
	kBSAFiletypeFlag_Voices   = 0x0010,
	kBSAFiletypeFlag_Shaders  = 0x0020,
	kBSAFiletypeFlag_Trees    = 0x0040,
	kBSAFiletypeFlag_Fonts    = 0x0080,
	kBSAFiletypeFlag_Misc     = 0x0100,
};
enum BSAFileFlags : UInt32 { // consumes bits from the BSAEntry::size field
	kBSAFileFlags_NonDefaultCompression = 0x40000000,
	kBSAFileFlags_Unk80000000 = 0x80000000, // possibly "is invalid" // wait, is this in BSAEntry::size or BSAEntry::offset? 0042E4B3 has it in the latter
};

struct BSHash {
	UInt64 data;
	MEMBER_FN_PREFIX(BSHash);
	DEFINE_MEMBER_FN(Constructor,        BSHash&, 0x006FA2D0, const char* str, UInt32 type);
	DEFINE_MEMBER_FN(Subroutine0042BC10, UInt32,  0x0042BC10, const BSHash& other);
};
struct BSAEntry { // "File Record" or "Folder Record" in UESP docs
	BSHash hash; // 00 // hash of the file/folder path
	union {
		UInt32 count = 0; // number of files
		UInt32 size;      // size of the file in bytes
	}; // 08
	union {
		UInt32    offset = 0; // data offset within the file (where it's an offset FROM differs between folders and files)
		BSAEntry* files;      // array of file entries for this folder, once the folder is loaded (happens in Archive constructor)
	}; // 0C

	MEMBER_FN_PREFIX(BSAEntry);
	DEFINE_MEMBER_FN(Constructor, BSAEntry&, 0x0042BD20);

	UInt32 getOffset() const { // files only
		return this->offset & 0x7FFFFFFF;
	}
	UInt32 getSize() const { // files only
		return this->size & 0x3FFFFFFF;
	}
	bool isNonDefaultCompression() const { // files only
		return (this->size & 0x40000000);
	}
	bool isKnownNotToBeOverridden() const { // files only
		return (this->size & 0x80000000); // set by Archive::CheckFileIsOverridden once we know the file isn't overridden by a loose file
	}
	bool isInvalidated() const { // files only
		return (this->offset & 0x80000000) == 0;
	}
	void invalidate() { // files only
		this->offset &= 0x80000000;
	}
};
constexpr UInt32 ce_bsaSignatureBSwapped = '\0ASB';
struct BSAHeader {
	UInt32 unk00 = ce_bsaSignatureBSwapped; // 00
	UInt32 version = 0x67; // 04
	UInt32 offset  = 0x24; // 08
	UInt32 flags   = 0; // 0C // offset 0x160 in Archive
	UInt32 directoryCount = 0; // 10 // offset 0x164 in Archive
	UInt32 fileCount      = 0; // 14 // offset 0x168 in Archive
	UInt32 totalFolderNameLength = 0; // 18 // offset 0x16C in Archive
	UInt32 totalFileNameLength = 0; // 1C // offset 0x170 in Archive
	UInt16 fileFlags = 0; // 20 // offset 0x174 in Archive // BSAFiletypeFlags
	UInt16 pad22; // 22

	MEMBER_FN_PREFIX(BSAHeader);
	DEFINE_MEMBER_FN(Constructor, BSAHeader&, 0x006FA180);
};
class Archive;
class ArchiveFile : public BSFile { // sizeof == 0x15C
public:
	Archive* owner;  // 154
	UInt32   offset; // 158 // offset within the BSA file

	MEMBER_FN_PREFIX(ArchiveFile);
	DEFINE_MEMBER_FN(Constructor, ArchiveFile&, 0x0042D540, const char* path, Archive* owner, UInt32 offset, UInt32 filesize, SInt32);
};
class CompressedArchiveFile : public ArchiveFile { // sizeof == 0x174
public:
	enum { kVTBL = 0x00A35E64 };

	// TODO

	MEMBER_FN_PREFIX(CompressedArchiveFile);
	DEFINE_MEMBER_FN(Constructor, CompressedArchiveFile&, 0x0042D6D0, UInt32, UInt32, UInt32, UInt32, UInt32);
};

class Archive : public  BSFile {
public:
	enum Flags194 {
		kFlag194_Unk08 = 0x08,  //This seems to signal Archive Invalidation. Initialized to 8 if a4 of Archive constructior is != 0
	};

	BSAHeader header; // 154
	BSAEntry* folders = nullptr; // 178 // array
	UInt32 unk17C;
	__time64_t myDateModified; // 180 // same type as the Date Modified in stat()
	UInt32 unk188 = 0; // same type as unk148
	UInt32 unk18C = -1;
	UInt32 unk190 = -1;
	UInt8  unk194 = 0; // initialized to 0 or 8
	UInt8  unk195;
	UInt8  unk196;
	UInt8  unk197;
	char*   folderNames       = nullptr; // 198 // a bunch of consecutive zero-terminated strings
	UInt32* folderNameOffsets = nullptr; // 19C // possibly an array of offsets, in 198, for each folder name
	char*   fileNames   = nullptr; // 1A0 // "File Name Block" in UESP docs: a bunch of consecutive zero-terminated strings
	UInt32** fileNameOffsetsByFolder = nullptr; // 1A4
	UInt32 refCount = 0; // 1A8
	bool   queuedForDeletion = 0; // 1AC
	UInt8  unk1AD;
	UInt8  unk1AE;
	UInt8  unk1AF;
	UInt32 unk1B0;
	UInt32 unk1B4[(0x200 - 0x1B4) / 4];
	CRITICAL_SECTION unk200; // 200 // wrapped in a struct that allows for automatic initialization/destruction
	UInt32 unk218[(0x280 - 0x218) / 4];

	MEMBER_FN_PREFIX(Archive);
	DEFINE_MEMBER_FN(Constructor, Archive&, 0x0042EE80, const char* filePath, UInt32, bool, UInt32);
	DEFINE_MEMBER_FN(CheckFileIsOverridden, bool, 0x0042C1D0, BSAEntry& file, const char* looseFilePath); // called by FolderContainsFile; invalidates the file if it's older than a matching loose file
	DEFINE_MEMBER_FN(ContainsFile,   bool, 0x0042E020, const BSHash& file, const BSHash& folder, UInt32& outFolderIndex, UInt32& outFileIndexInFolder, const char* normalizedFilepath); // just calls ContainsFolder and FolderContainsFile
	DEFINE_MEMBER_FN(ContainsFolder, bool, 0x0042CE40, const BSHash& folder, UInt32& outFolderIndex, const char* normalizedFilepath);
	DEFINE_MEMBER_FN(DiscardRetainedFilenames,   void, 0x0042C0D0, UInt32); // conditional on unk194 flag 0x04
	DEFINE_MEMBER_FN(FolderContainsFile, bool, 0x0042D000, UInt32 folderIndex, const BSHash& file, UInt32& outFileIndexInFolder, const char* normalizedFilepath, UInt32 zero); // file path is used for CheckFileIsOverridden
	DEFINE_MEMBER_FN(GetFileByEntry,   ArchiveFile*, 0x0042E1A0, const BSAEntry& file, SInt32, const char* normalizedFilepath); // only used for lazy lookups
	DEFINE_MEMBER_FN(GetFileByIndices, ArchiveFile*, 0x0042E070, UInt32 folderIndex, UInt32 fileIndex, SInt32, const char* normalizedFilepath);
	DEFINE_MEMBER_FN(GetFileEntry, BSAEntry*,    0x0042D240, const BSHash& folder, const BSHash& file, const char* normalizedFilepath); // only used by LazyFileLookup
	DEFINE_MEMBER_FN(RetainsFilenameStringTable, bool, 0x0042BD30);
	DEFINE_MEMBER_FN(RetainsFilenameOffsetTable, bool, 0x0042BD50);
	DEFINE_MEMBER_FN(Subroutine0042BD70, bool, 0x0042BD70);
	DEFINE_MEMBER_FN(CheckDelete, void, 0x0042C910); //
	DEFINE_MEMBER_FN(DecRef,  void, 0x0042C910); // Bethesda calls this "CheckDelete;" define it with both names
	//
	// The below are all called during the constructor, so don't use them:
	//
	DEFINE_MEMBER_FN(InvalidateOlderFiles, UInt32, 0x0042E2D0); // returns number of files invalidated
	DEFINE_MEMBER_FN(InvalidateAgainstLooseFiles, UInt32, 0x0042D2A0, const char* pathRoot, const char* pathDeep, FILETIME& myLastModDate); // called by InvalidateOlderFiles
	DEFINE_MEMBER_FN(LoadFolderNames,      const char*, 0x0042CAE0, UInt32 stopAt); // returns loaded folder names
	DEFINE_MEMBER_FN(SetReadWriteFuncs,    void,   0x004307F0, bool);
};
static_assert(sizeof(Archive) == 0x280, "Archive wrong size!");

extern Archive** const g_firstLoadedArchivesByType;  // g_firstLoadedArchivesByType[type] == first loaded Archive flagged with that filetype
extern Archive** const g_secondaryArchiveByTypeList; // offset from the first list by sizeof(void*) * 9
extern Archive** const g_ArchiveByTypeList ; //USed in the Lazy lookup after the first list
// 10
struct BoundObjectListHead
{
	UInt32			boundObjectCount;	// 0
	TESBoundObject	* first;			// 4
	TESBoundObject	* last;				// 8
	UInt32			unkC;				// C
};

// 8
class TESRegionDataManager
{
public:
	TESRegionDataManager();
	~TESRegionDataManager();

	virtual void	Destructor(void) = 0;	// 0
	virtual void	Unk_1(void) = 0;
	virtual void	Unk_2(void) = 0;
	virtual void	Unk_3(void) = 0;
	virtual void	Unk_4(void) = 0;
	virtual void	Unk_5(void) = 0;
	virtual void	Unk_6(void) = 0;
	virtual void	Unk_7(void) = 0;
	virtual void	Unk_8(void) = 0;

//	void	** _vtbl;
	UInt32	unk4;
};

// 10
class TESRegionList
{
public:
	TESRegionList();
	~TESRegionList();

	virtual void	Destructor(void) = 0;	// 0
	// no other virtual fns

	struct Entry {
		TESRegion	* region;
		Entry		* next;
	};

//	void	** _vtbl;	// 00
	Entry	regionList;	// 04
	UInt8	unkC;		// 0C
	UInt8	padD[3];
};

// note: this list contains all esm/esp files in the Data folder, even ones that are not loaded
struct ModEntry
{
	// 41C / 420
	struct Data		// referred to as 'TESFile' by Bethesda
	{
		enum
		{
			kFlag_IsMaster =	1 << 0,
			kFlag_Loaded =		1 << 2,
			kFlag_Active =		1 << 3
		};

		struct	ChunkInfo
		{
			UInt32	type;		// e.g. 'GRUP', 'GLOB', etc
			UInt32	length;
		};

		struct	RecordInfo
		{
			ChunkInfo		chunkInfo;
			UInt32			flags;
			UInt32			recordID;
			TrackingData	trackingData;
		};
		class GroupInfo : public RecordInfo
		{// size 18/18
			//     /*00*/ RecordInfo    // for group records, the size includes the 14 bytes of the header
			UInt32        recordOffset;   // used internally to track header offsets of all open groups
		};

		struct  SizeInfo
		{
			UInt32		fileSizeHigh;			// WIN32_FIND_DATA::nFileSizeHigh
			UInt32		fileSizeLow;			// WIN32_FIND_DATA::nFileSizeLow
		};

		// static members: B33C1C, B33C20
		typedef BSSimpleList<GroupInfo*> GroupList;

		UInt32	errorState;							// 000 appears to indicate status of file (open, closed, etc) 2, 9, 0C do stuff
		UInt32	ghostFileParent;							// 004
		UInt32	childThreadGhostFiles;							// 008
		BSFile*	unkFile00C;							// 00C
		BSFile	* bsFile;						// 010
		UInt32	unk014;							// 014
		UInt32	unk018;							// 018
		char	name[0x104];					// 01C
		char	filepath[0x104];				// 120 relative to "Oblivion\"
		UInt32	unk224;							// 224
		UInt32	unk228;							// 228 init to *(0xB055CC), seen 0x2800
		UInt32	unk22C[(0x23C - 0x22C) >> 2];	// 22C
		RecordInfo	currentRecordInfo;					// 23C
		ChunkInfo	currentChunk;					// 250
		UInt32               fileSize; // same as FileSizeLow in find data
		UInt32               currentRecordOffset; // offset of current record in file
		UInt32               currentChunkOffset; // offset of current chunk in record
		UInt32               fetchedChunkDataSize; // number of bytes read in last GetChunkData() call
		GroupInfo            unkFile268; // used when saving empty form records, e.g. for deleted forms
		UInt32               unkFile280; // used when saving empty form records, e.g. for deleted forms  //280
		GroupList            openGroups; // stack of open group records, from lowest level to highest //284
		bool                 headerRead; // set after header has been successfully parsed //28C
		UInt8                padFile28D[3];
		WIN32_FIND_DATA	findData;				// 290
		UInt32	version;						// 3D0 plugin version (0.8/1.0)
		UInt32	formCount;						// 3D4 record/form count
		UInt32	nextFormID;						// 3D8 used by TESFile::sub_486BF0 in the editor
		UInt32	flags;							// 3DC
		tList<char>		masterList;				// 3E0 linked list of .esm dependencies
		tList<SizeInfo>	masterSizeInfo;			// 3E8 linked list of file size info for above list
		UInt32	idx;							// 3F0 //This seems to represent the index of the current file (equals to the size of the master counts )
		void	* unk3F4;						// 3F4
		UInt32	unk3F8;							// 3F8
		UInt32	unk3FC;							// 3FC
		UInt8	unk400;							// 400 init to -1
		UInt8	pad401[3];
		BSStringT	authorName;						// 404
		BSStringT	modDescription;					// 40C
	
		void* currentRecordDCBuffer;		 // buffer for decompressed record data //414
		UInt32 currentRecordDCLength; // length of decompressed record data //418
		//TESFile*             unkFile41C; // file this object was cloned from. used for local copies of network files?  COEF also report this member increasing the max size
	};

	Data		* data;
	ModEntry	* next;

	ModEntry * Next() const	{	return next;	}
	Data * Info() const		{	return data;	}
	bool IsLoaded()	const	{	return (data && (data->flags & Data::kFlag_Loaded)) ? true : false;	}
};
STATIC_ASSERT(offsetof(ModEntry::Data, currentChunk) == 0x250);
STATIC_ASSERT(offsetof(ModEntry::Data, unkFile268) == 0x268);
STATIC_ASSERT(offsetof(ModEntry::Data, headerRead) == 0x28C);

STATIC_ASSERT(sizeof(ModEntry::Data) == 0x41C);


// CE0 / 1220 (editor, due entirely to difference in size of TESSkill)
class DataHandler
{
public:
	DataHandler();
	~DataHandler();

	struct _Unk8B8											// as seen in the editor
	{
		UInt32				unk00;							// 00
		UInt32				unk04;							// 04
		UInt32				unk08;							// 08 initialized to (numLoadedMods << 24) | 0x800 during plugin load
		ModEntry::Data*		activeFile;						// 0C active plugin
	};

	BoundObjectListHead		* boundObjects;					// 000
	tList<TESPackage>		packages;						// 004
	tList<TESWorldSpace>	worldSpaces;					// 00C
	tList<TESClimate>		climates;						// 014
	tList<TESWeather>		weathers;						// 01C
	tList<EnchantmentItem>	enchantmentItems;				// 024
	tList<SpellItem>		spellitems;						// 02C
	tList<TESHair>			hairs;							// 034
	tList<TESEyes>			eyes;							// 03C
	tList<TESRace>			races;							// 044
	tList<TESLandTexture>	landTextures;					// 04C
	tList<TESClass>			classes;						// 054
	tList<TESFaction>		factions;						// 05C
	tList<Script>			scripts;						// 064
	tList<TESSound>			sounds;							// 06C
	tList<TESGlobal>		globals;						// 074
	tList<TESTopic>			topics;							// 07C
	tList<TESQuest>			quests;							// 084
	tList<BirthSign>		birthsigns;						// 08C
	tList<TESCombatStyle>	combatStyles;					// 094
	tList<TESLoadScreen>	loadScreens;					// 09C
	tList<TESWaterForm>		waterForms;						// 0A4
	tList<TESEffectShader>	effectShaders;					// 0AC
	tList<TESObjectANIO>	objectAnios;					// 0B4
	TESRegionList			* regionList;					// 0BC
	NiTLargeArray <TESObjectCELL *>	cellArray;				// 0C0
//	UInt32					unk0D0[2];						// 0D0
	TESSkill				skills[0x15];					// 0D8
	_Unk8B8					unk8B8;							// 8B8
	ModEntry				modList;						// 8C8
	UInt32					numLoadedMods;					// 8D0
	ModEntry::Data			* modsByID[0xFF];				// 8D4
	UInt32					unkCD0[(0xCD8 - 0xCD0) >> 2];	// CD0
	TESRegionDataManager	* regionDataManager;			// CD8
	void*					unkCDC;							// CDC //ptr to ExtraData?

	bool	ConstructObject(ModEntry::Data* tesFile, bool unk1);

	const ModEntry * LookupModByName(const char * modName);
	const ModEntry ** GetActiveModList();		// returns array of modEntry* corresponding to loaded mods sorted by mod index
	UInt8 GetModIndex(const char* modName);
	UInt8 GetActiveModCount();
	const char* GetNthModName(UInt32 modIndex);
	TESGlobal* GetGlobalVarByName(const char* varName, UInt32 nameLen);
	TESQuest* GetQuestByEditorName(const char* questName, UInt32 nameLen = -1);
};

#if OBLIVION
STATIC_ASSERT(sizeof(DataHandler) == 0xCE0);
#else
STATIC_ASSERT(sizeof(DataHandler) == 0x1220);
#endif


typedef Visitor<ModEntry, ModEntry::Data> ModEntryVisitor;

class ChangesMap;
class InteriorCellNewReferencesMap;
class ExteriorCellNewReferencesMap;
class NumericIDBufferMap;

#if 0			// this is a redefinition of TESSaveLoadGame
// 1C8
class ChangeHandler
{
	ChangesMap						* changesMap000;		// 000
	ChangesMap						* changesMap004;		// 004
	InteriorCellNewReferencesMap	* interiorCellMap;		// 008
	ExteriorCellNewReferencesMap*	exteriorCellMaps[2];	// 00C
	UInt32							unk1[2];				// 014
	void*							unk2;					// 01C
	UInt32							unk3[3];				// 020

	struct RefIDList {
		UInt32	refID;
		RefIDList* next;
	};
	RefIDList						baseFormNewList;		// 02C
	UInt32							unk4[5];				// 034
	UInt32							unk5[2];				// 048
	void*							unk6;					// 050
	UInt32							unk7;					// 054
	NumericIDBufferMap*				bufferMaps[4];			// 058
	UInt32							unk8[4];				// 068
	NiTLargeArray*					largeArrays[2];			// 078
	UInt32							unk9[83];				// 080
};
#endif

class FileFinder
{
public:
	FileFinder();
	~FileFinder();

	enum Flags{
		FindFile_ArchiveAndLooseFile = 0x0, //The default search, both checking loose files and bsas
		FindFile_LooseFileOnly = 0x1,
		FindFile_NoCheckAbsolutePath= 0x2, // If this is setted with FindFile_NoSearchPathUsage it's basically BSA only search'
		FindFile_NoSearchPathUsage = 0x4, // If setted exit before all checks using the FileFinder searchPaths . Setted in FindFile if path starts with '.\'
		FindFile_AbsoluteLooseFileOnly = FindFile_LooseFileOnly | FindFile_NoSearchPathUsage, //0x5 This is setted in FindFile if the path have ':' at position 1
		FindFile_ArchiveOnly = FindFile_NoSearchPathUsage | FindFile_NoCheckAbsolutePath // 0x6
	};
	virtual void Unk_00(void) = 0; //Seems to grow some storage
	// arg2 seems a bitflag is passed  to Unk_03 after some mangling
	virtual UInt32 FindFile(const char* filePath, UInt32 arg1, Flags arg2, UInt32 arg3) = 0;	//seen (char*, 0, 0, -1)  or (char*, 0,6,-1)
	virtual void InitalizeDataByFolder(const char* folder); // Saw arg "Data\\" called by initialization. Seems to only register the folder but don't load content'
 	virtual UInt32 SearchForFile(const char* filePath, Flags arg1, UInt32 arg2) = 0; //seen (char*,0xC1, 0xFFFF). This actually do the file search, invoked by FindFile after some path sanitization, argt1 is passed from FindFile arg2 . Not sure where arg1 0xC1 was seen considering this is depending on the Flags PArameter (and the function only check for 1,2 and 4 (5 and 6 are OR combinations of the bitflags variant))
	virtual void Unk_04(void) = 0;

	enum {
		kFileStatus_NotFound = 0,
		kFileStatus_Unpacked,
		kFileStatus_Packed
	};	//return values for FindFile(), Unk_03()

	//vtbl					00
	NiTArray<const char*>	searchPaths;	//NiTArray@PBD@@ - Single entry containing filepath "Data\"
};

extern FileFinder** g_FileFinder;

class TimeGlobals
{
	TESGlobal	* gameYear;
	TESGlobal	* gameMonth;
	TESGlobal	* gameDay;
	TESGlobal	* gameHour;
	TESGlobal	* gameDaysPassed;
	TESGlobal	* timeScale;
public:
	static TimeGlobals* Singleton();

	TimeGlobals();
	~TimeGlobals();

	static UInt32 GameYear() ;
	static UInt32 GameMonth() ;
	static UInt32 GameDay() ;
	static float  GameHour() ;
	static UInt32 GameDaysPassed() ;
	static float  TimeScale() ;
	static UInt32 GameHoursPassed() ;
	static UInt32 HoursToRespawnCell() ;

	// monthID in range [1, 12]
	static UInt16 GetFirstDayOfMonth(UInt32 monthID);
	static UInt16 GetNumDaysInMonth(UInt32 monthID);
};

class GridDistantArray;
class NiNode;
class BSTempNodeManager;
class NiDirectionalLight;
class BSFogProperty;
class NiSourceTexture;
class Sky;

// 04
class GridArray
{
public:
	GridArray();
	~GridArray();

	virtual void Fn_00(void) = 0;
	virtual void Fn_01(void) = 0;
	virtual void Fn_02(void) = 0;
	virtual void Fn_03(void) = 0;
	virtual void Fn_04(void) = 0;
	virtual void Fn_05(void) = 0;
	virtual void Fn_06(void) = 0;
	virtual void Fn_07(void) = 0;
	virtual void Fn_08(void) = 0;
	virtual void Fn_09(void) = 0;

	// void **		vtbl
	// no data members
};

// 028
class GridCellArray : public GridArray
{
public:
	GridCellArray();
	~GridCellArray();

	// size?
	struct CellInfo
	{
		UInt32		unk00;
		NiNode		* niNode;
		// ...
	};

	// 04
	struct GridEntry
	{
		TESObjectCELL	* cell;
		CellInfo		* info;
	};

	// void **		vtbl
	UInt32			worldX;		// 04 worldspace x coordinate of cell at center of grid (player's cell)
	UInt32			worldY;		// 08 worldspace y
	UInt32			size;		// 0C grid is size^2, size = uGridsToLoad
	GridEntry		* grid;		// 10 dynamically alloc'ed array of GridEntry[size^2]
	float			posX;		// 14 4096*worldX (exterior cells are 4096 square units)
	float			posY;		// 18 4096*worldY
	UInt32			unk1C;		// 1C seen 0
	UInt32			unk20;		// 20 seen 1
	UInt32			unk24;		// 24 seem 0

	GridEntry* GetGridEntry(UInt32 x, UInt32 y);	// x and y range from 0 to (size-1)
};

// 0AC
class TES
{
public:
	TES();
	~TES();

	// only 1 virtual fn. If worldSpace is null, uses this->worldSpace
	virtual void Unk_00(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3, TESWorldSpace* worldSpace);

	static TES* GetSingleton();

	// 08
	struct Unk8C
	{
		TESNPC	* npc;
		UInt32	unk4;	// size?
	};

	// void		** vtbl >> oddly, vtbl pointer is NULL in global TES object though c'tor initializes it...
	GridDistantArray	* gridDistantArray;		// 04
	GridCellArray		* gridCellArray;		// 08
	NiNode				* sceneGraphObjectRoot;				// 0C
	NiNode				* sceneGraphLandLOD;				// 10
	BSTempNodeManager	* tempNodeManager;		// 14
	NiDirectionalLight	* niDirectionalLight;	// 18
	BSFogProperty		* fogProperty;			// 1C
	SInt32				extXCoord;					// 20 cell grid x coordinate within current worldspace
	SInt32				extYCoord;					// 24 grid y
	SInt32				unk28;					// 28 same as unk20?
	SInt32				unk2C;					// 2C same as unk24?
	TESObjectCELL		* currentExteriorCell;	// 30
	TESObjectCELL		* currentInteriorCell;	// 34
	TESObjectCELL		** interiorCellBufferArray;				// 38
	TESObjectCELL		** exteriorCellBufferArray;			// 3C ?
	UInt32				unk40;					// 40
	UInt32				unk44;					// 44
	UInt32				unk48;					// 48 seen 0x7FFFFFFF; seen caching unk20 in editor
	UInt32				unk4C;					// 4C seen 0x7FFFFFFF; seen caching unk24 in editor
	UInt32				unk50;					// 50
	void				* waterSurfaceManager;	// 54 some struct; seen { 0, 0, BSRenderedTexture*, ... }
	void				* waterNodeData;		// 58
	Sky					* sky;					// 5C
	UInt32				unk60;					// 60
	UInt32				unk64;					// 64
	UInt32				unk68;					// 68
	float				unk6C;					// 6C
	float				unk70;					// 70
	TESWorldSpace		* currentWorldSpace;	// 74
	UInt32				unk78[5];				// 78
	tList<Unk8C>		list8C;					// 8C
	NiSourceTexture		* bloodDecals[3];		// 94 blood.dds, lichblood.dds, whillothewispblood.dds
	tList<void*>		listA0;					// A0 data is some struct containing NiNode*
	UInt32				unkA8;					// A8

	bool GetTerrainHeight(float* posVec3, float* outHeight);
};

STATIC_ASSERT(sizeof(TES) == 0xAC);

bool GetWaterShaderProperty(const char* propName, float& out);
