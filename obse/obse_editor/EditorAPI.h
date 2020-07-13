#pragma once

#if OBLIVION
#error EditorAPI.h included in runtime project
#endif

#define EDITOR_SPECIFIC(X) X
#define RUNTIME_SPECIFIC(X)

#include "EditorRTTI.h"
#include "GameTypes.h"

struct ScriptLineBuffer;
class TESForm;
class DataHandler;
class TESObjectREFR;
class TES;
struct ScriptBuffer;

typedef NiTPointerMap<TESForm> FormMap;
extern FormMap* g_FormMap;	//	currently unused
extern DataHandler** g_dataHandler;
extern TES** g_TES;

typedef TESForm * (__stdcall * _GetFormByID)(const char* editorID);
extern const _GetFormByID GetFormByID;

typedef void * (* _FormHeap_Allocate)(UInt32 size);
extern const _FormHeap_Allocate FormHeap_Allocate;

typedef void (* _FormHeap_Free)(void * ptr);
extern const _FormHeap_Free FormHeap_Free;

typedef void			(__cdecl *_ShowCompilerError)(ScriptBuffer* Buffer, const char* format, ...);
extern const _ShowCompilerError		ShowCompilerError;

// 18
class RenderWindowSelection
{
public:
	RenderWindowSelection();
	~RenderWindowSelection();

	struct Node {
		TESObjectREFR	* refr;
		Node			* prev;
		Node			* next;
	};

	Node			* selectionGroup;		// 00
	UInt32			selectionCount;			// 04
	float			x, y, z;				// 08 sum of position vectors of selected refr's
	float			unk14;					// 14

	static RenderWindowSelection * GetSingleton();
};





