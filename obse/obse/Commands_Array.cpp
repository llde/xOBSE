#include "ParamInfos.h"
#include "obse/Commands_Array.h"
#include "CommandTable.h"
#include "GameForms.h"
#include "ScriptUtils.h"

#if OBLIVION

#include "GameAPI.h"

static const double s_arrayErrorCodeNum = -99999;		// sigil return values for cmds returning array keys
static std::string s_arrayErrorCodeStr = "";		// indicating invalid/non-existent key

////////////////////////////
// DW Nifscript stuff
////////////////////////////

#include "obse_common/SafeWrite.h"
#include "GameObjects.h"

static const UInt32 kLoadFile_HookAddr = 0x006F9999;
static const UInt32 kLoadFile_RetnAddr = 0x006F999E;
static const UInt32 kLoadFile_CallAddr = 0x00748100;	// __cdecl BSFile* LoadFile(char* path, UInt32 arg1, UInt32 arg2)

static const UInt32 kFinishedWithFile_HookAddr = 0x004395A9;
static const UInt32 kFinishedWithFile_RetnAddr = 0x004395AE;

// This is the folder in which NifScript will keep all of its dynamically-generated .nifs
// must be within Meshes. Folder name can be changed, keep it to 4 chars or less for efficiency's sake. MUST be all-lowercase
static char s_nifScriptFullPath[] = "meshes\\ns\\";	
static char s_nifScriptPath[] = "ns\\";											

static UInt32 s_nifScriptFullPathLen = sizeof(s_nifScriptFullPath) - 1;	// don't count the null terminator
static UInt32 s_nifScriptPathLen = sizeof(s_nifScriptPath) - 1;

// returns true if the filepath points to nifScript's folder. Lets us quickly & easily determine if NifScript is interested in this file or not
static bool IsNifScriptFilePath(const char* path, bool bUseFullPath)
{

	if (!path)
		return false;
	
	const char* nsPath = bUseFullPath ? s_nifScriptFullPath : s_nifScriptPath;
	UInt32 nsPathLen = bUseFullPath ? s_nifScriptFullPathLen : s_nifScriptPathLen;

	UInt32 i = 0;
	while (path[i] && i < nsPathLen)
	{
		if (tolower(path[i]) != nsPath[i])
			break;

		i++;
	}

	return (i == nsPathLen);
}

// this is called when FileFinder attempts to load a .nif which doesn't exist
// should create a .nif with the requested name/path on disk
// nifPath includes "meshes\\" prefix
// return true if nif created, false otherwise
// presumably all "dynamic" .nifs would go in a particular folder i.e. "meshes\nifScript" to allow
// you to easily distinguish between your .nifs and genuinely missing .nifs
// add code elsewhere to clean up .nif files after they've been loaded
static bool __stdcall CreateNifFile(const char* nifPath)
{
	if (!IsNifScriptFilePath(nifPath, true))
		return false;

	_MESSAGE("FileFinder requesting nifScript file %s", nifPath);
	
	// Create the .nif and save to disk
	
	return true;
}

static __declspec(naked) void LoadFileHook(void)
{
	static UInt32 s_path;

	__asm
	{
		mov	eax,		[esp]		// grab the first arg (nifPath)
		mov [s_path],	eax

		// does nif already exist?
		call	[kLoadFile_CallAddr]
		test	eax,	eax
		jnz		Done

		// nif doesn't exist, create it
		mov		eax,	[s_path]
		push	eax
		call	CreateNifFile
		test	al, al
		jnz		ReloadFile
		
		// CreateFile didn't create the file, so return NULL
		xor		eax,	eax
		jmp		Done
		
	ReloadFile:
		// ask FileFinder to load the nif we just created
		call	[kLoadFile_CallAddr]

	Done:
		jmp		[kLoadFile_RetnAddr]
	}
}

// This is called when QueuedModel is finished with .nif file
// if it's a nifscript file, you can safely delete the file
// nifPath does not include "meshes\\" prefix
static void __stdcall DeleteNifFile(const char* nifPath)
{
	if (!IsNifScriptFilePath(nifPath, false))
		return;

	_MESSAGE("FileFinder has finished with nifScript file %s", nifPath);

	// delete the file
	
}

static __declspec(naked) void FinishedWithFileHook(void)
{
	__asm
	{
		mov		eax,	[esi+0x20]		// filepath
		push	eax
		call	DeleteNifFile

		// overwritten code
		pop		ecx
		pop		edi
		pop		esi
		pop		ebp
		pop		ebx
		
		jmp		[kFinishedWithFile_RetnAddr]
	}
}

static bool Cmd_InstallModelMapHook_Execute(COMMAND_ARGS)
{
	WriteRelJump(kLoadFile_HookAddr, (UInt32)&LoadFileHook);
	WriteRelJump(kFinishedWithFile_HookAddr, (UInt32)&FinishedWithFileHook);
	return true;
}

static bool Cmd_ar_Construct_Execute(COMMAND_ARGS)
{
	char arType[0x200];
	*result = 0;

	if (!ExpressionEvaluator::Active())
	{
		Console_Print("Error in script %08X: ar_Construct called outside of a valid context.", scriptObj->refID);
		return true;
	}

	if (!ExtractArgs(PASS_EXTRACT_ARGS, &arType))
		return true;

	UInt8 keyType = kDataType_Numeric;
	bool bPacked = false;
	if (!_stricmp(arType, "StringMap"))
		keyType = kDataType_String;
	else if (_stricmp(arType, "Map"))
		bPacked = true;

	ArrayID newArr = g_ArrayMap.Create(keyType, bPacked, scriptObj->GetModIndex());
	*result = newArr;
	return true;
}

static bool Cmd_ar_Size_Execute(COMMAND_ARGS)
{
	*result = -1;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0))
	{
		if (eval.Arg(0)->CanConvertTo(kTokenType_Array))
		{
			ArrayID arrID = eval.Arg(0)->GetArray();
			UInt32 size = g_ArrayMap.SizeOf(arrID);
			*result = (size == -1) ? -1.0 : (double)size;
		}
	}

	return true;
}

static bool Cmd_ar_Dump_Execute(COMMAND_ARGS)
{
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->CanConvertTo(kTokenType_Array))
		g_ArrayMap.DumpArray(eval.Arg(0)->GetArray());

	return true;
}

static bool Cmd_ar_DumpID_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &arrayID))
	{
		if (!g_ArrayMap.Exists(arrayID))
			Console_Print("Array does not exist.");
		else if (!g_ArrayMap.SizeOf(arrayID))
			Console_Print("Array is empty.");
		else
			g_ArrayMap.DumpArray(arrayID);
	}

	return true;
}

static bool Cmd_ar_Erase_Execute(COMMAND_ARGS)
{
	// returns num elems erased or -1 on error
	UInt32 numErased = -1;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0))
	{
		if (eval.Arg(0)->CanConvertTo(kTokenType_Array))
		{
			ArrayID arrID = eval.Arg(0)->GetArray();
			
			if (eval.Arg(1)) {
				// are we erasing a range or a single element?
				const Slice* slice = eval.Arg(1)->GetSlice();
				if (slice)
				{
					ArrayKey lo, hi;
					slice->GetArrayBounds(lo, hi);
					numErased = g_ArrayMap.EraseElements(arrID, lo, hi);
				}
				else
				{
					if (eval.Arg(1)->CanConvertTo(kTokenType_String))
					{
						ArrayKey toErase(eval.Arg(1)->GetString());
						numErased = g_ArrayMap.EraseElements(arrID, toErase, toErase);
					}
					else if (eval.Arg(1)->CanConvertTo(kTokenType_Number))
					{
						ArrayKey toErase(eval.Arg(1)->GetNumber());
						numErased = g_ArrayMap.EraseElements(arrID, toErase, toErase);
					}
				}
			}
			else {
				// second arg omitted - erase all elements of array
				numErased = g_ArrayMap.EraseAllElements(arrID);
			}
		}
	}

	if (numErased == -1) {
		*result = -1.0;
	}
	else {
		*result = numErased;
	}

	return true;
}

static bool Cmd_ar_Sort_Execute(COMMAND_ARGS)
{
	// we return this empty array if something goes wrong
	ArrayID sortedID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayVarMap::SortOrder order = ArrayVarMap::kSort_Ascending;
		if (eval.Arg(1) && eval.Arg(1)->GetNumber())
			order = ArrayVarMap::kSort_Descending;
		sortedID = g_ArrayMap.Sort(eval.Arg(0)->GetArray(), order, ArrayVarMap::kSortType_Default, scriptObj->GetModIndex());
	}

	*result = sortedID;
	return true;
}

static bool Cmd_ar_CustomSort_Execute(COMMAND_ARGS)
{
	// user provides a function script to perform comparison
	ArrayID sortedID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() >= 2) {
		ArrayID toSort = eval.Arg(0)->GetArray();
		Script* compare = OBLIVION_CAST(eval.Arg(1)->GetTESForm(), TESForm, Script);

		ArrayVarMap::SortOrder order = ArrayVarMap::kSort_Ascending;
		if (eval.Arg(2) && eval.Arg(2)->GetBool()) {
			order = ArrayVarMap::kSort_Descending;
		}

		if (toSort && compare) {
			sortedID = g_ArrayMap.Sort(toSort, order, ArrayVarMap::kSortType_UserFunction, scriptObj->GetModIndex(), compare);
		}
	}

	*result = sortedID;
	return true;
}

static bool Cmd_ar_SortAlpha_Execute(COMMAND_ARGS)
{
	// we return this empty array if something goes wrong
	ArrayID sortedID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayVarMap::SortOrder order = ArrayVarMap::kSort_Ascending;
		if (eval.Arg(1) && eval.Arg(1)->GetNumber())
			order = ArrayVarMap::kSort_Descending;
		sortedID = g_ArrayMap.Sort(eval.Arg(0)->GetArray(), order, ArrayVarMap::kSortType_Alpha, scriptObj->GetModIndex());
	}

	*result = sortedID;
	return true;
}

static bool Cmd_ar_Find_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "ar_Find must be called within an OBSE expression.");
		return true;
	}

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() >= 2 && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		// what type of element are we looking for?
		ArrayElement toFind;
		BasicTokenToElem(eval.Arg(0), toFind, &eval);
		
		// get the array
		ArrayID arrID = eval.Arg(1)->GetArray();

		// set result to the default (not found)
		UInt8 keyType = g_ArrayMap.GetKeyType(arrID);
		if (keyType == kDataType_String)
		{
			eval.ExpectReturnType(kRetnType_String);
			*result = g_StringMap.Add(scriptObj->GetModIndex(), s_arrayErrorCodeStr.c_str(), true);
		}
		else
		{
			eval.ExpectReturnType(kRetnType_Default);
			*result = s_arrayErrorCodeNum;
		}
		
		if (!toFind.IsGood())		// return error code if toFind couldn't be resolved
			return true;

		// got a range?
		const Slice* range = NULL;
		if (eval.Arg(2))
			range = eval.Arg(2)->GetSlice();

		// do the search
		ArrayKey idx = g_ArrayMap.Find(arrID, toFind, range);
		if (!idx.IsValid())
			return true;

		if (keyType == kDataType_Numeric)
			*result = idx.Key().num;
		else
			*result = g_StringMap.Add(scriptObj->GetModIndex(), idx.Key().str.c_str(), true);
	}

	return true;
}

static enum eIterMode {
	kIterMode_First,
	kIterMode_Last,
	kIterMode_Next,
	kIterMode_Prev
};

static bool ArrayIterCommand(COMMAND_ARGS, eIterMode iterMode)
{
	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "ar_First/Last must be called within an OBSE expression.");
		return true;
	}

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		UInt8 keyType = g_ArrayMap.GetKeyType(arrID);
		ArrayKey foundKey;
		ArrayElement foundElem;		// unused
		switch (keyType)
		{
		case kDataType_Numeric:
			{
				*result = s_arrayErrorCodeNum;
				eval.ExpectReturnType(kRetnType_Default);
				if (iterMode == kIterMode_First)
					g_ArrayMap.GetFirstElement(arrID, &foundElem, &foundKey);
				else if (iterMode == kIterMode_Last)
					g_ArrayMap.GetLastElement(arrID, &foundElem, &foundKey);
				else if (eval.NumArgs() == 2 && eval.Arg(1)->CanConvertTo(kTokenType_Number))
				{
					ArrayKey curKey = eval.Arg(1)->GetNumber();
					switch (iterMode)
					{
					case kIterMode_Next:
						g_ArrayMap.GetNextElement(arrID, &curKey, &foundElem, &foundKey);
						break;
					case kIterMode_Prev:
						g_ArrayMap.GetPrevElement(arrID, &curKey, &foundElem, &foundKey);
						break;
					}
				}

				if (foundKey.IsValid())
					*result = foundKey.Key().num;

				return true;
			}
		case kDataType_String:
			{
				eval.ExpectReturnType(kRetnType_String);
				std::string keyStr = s_arrayErrorCodeStr;
				if (iterMode == kIterMode_First)
					g_ArrayMap.GetFirstElement(arrID, &foundElem, &foundKey);
				else if (iterMode == kIterMode_Last)
					g_ArrayMap.GetLastElement(arrID, &foundElem, &foundKey);
				else if (eval.NumArgs() == 2 && eval.Arg(1)->CanConvertTo(kTokenType_String))
				{
					ArrayKey curKey = eval.Arg(1)->GetString();
					switch (iterMode)
					{
					case kIterMode_Next:
						g_ArrayMap.GetNextElement(arrID, &curKey, &foundElem, &foundKey);
						break;
					case kIterMode_Prev:
						g_ArrayMap.GetPrevElement(arrID, &curKey, &foundElem, &foundKey);
						break;
					}
				}

				if (foundKey.IsValid())
					keyStr = foundKey.Key().str;

				AssignToStringVar(PASS_COMMAND_ARGS, keyStr.c_str());
				return true;
			}
		default:
			{
				eval.Error("Invalid array key type in ar_First/Last. Check that the array has been initialized.");
				return true;
			}
		}
	}
	else
	{
		eval.Error("Invalid array access in ar_First/Last. Is the array initialized?");
		return true;
	}

	return true;
}
						
static bool Cmd_ar_First_Execute(COMMAND_ARGS)
{
	return ArrayIterCommand(PASS_COMMAND_ARGS, kIterMode_First);
}

static bool Cmd_ar_Last_Execute(COMMAND_ARGS)
{
	return ArrayIterCommand(PASS_COMMAND_ARGS, kIterMode_Last);
}

static bool Cmd_ar_Next_Execute(COMMAND_ARGS)
{
	return ArrayIterCommand(PASS_COMMAND_ARGS, kIterMode_Next);
}

static bool Cmd_ar_Prev_Execute(COMMAND_ARGS)
{
	return ArrayIterCommand(PASS_COMMAND_ARGS, kIterMode_Prev);
}

static bool Cmd_ar_Keys_Execute(COMMAND_ARGS)
{
	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "ar_Keys must be called within an OBSE expression.");
		return true;
	}

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.Arg(0) && eval.Arg(0)->CanConvertTo(kTokenType_Array))
		*result = g_ArrayMap.GetKeys(eval.Arg(0)->GetArray(), scriptObj->GetModIndex());

	return true;
}

static bool Cmd_ar_HasKey_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 2 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		UInt8 keyType = g_ArrayMap.GetKeyType(arrID);
		if (keyType == kDataType_String && eval.Arg(1)->CanConvertTo(kTokenType_String))
			*result = g_ArrayMap.HasKey(arrID, ArrayKey(eval.Arg(1)->GetString())) ? 1 : 0;
		else if (keyType == kDataType_Numeric && eval.Arg(1)->CanConvertTo(kTokenType_Number))
			*result = g_ArrayMap.HasKey(arrID, ArrayKey(eval.Arg(1)->GetNumber())) ? 1 : 0;
	}

	return true;
}

static bool Cmd_ar_BadStringIndex_Execute(COMMAND_ARGS)
{
	AssignToStringVar(PASS_COMMAND_ARGS, s_arrayErrorCodeStr.c_str());
	return true;
}

static bool Cmd_ar_BadNumericIndex_Execute(COMMAND_ARGS)
{
	*result = s_arrayErrorCodeNum;
	return true;
}

static bool ArrayCopyCommand(COMMAND_ARGS, bool bDeepCopy)
{
	*result = 0;
	if (!ExpressionEvaluator::Active())
	{
		ShowRuntimeError(scriptObj, "Array copy command must be called within an OBSE expression.");
		return true;
	}

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 1 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
		*result = g_ArrayMap.Copy(eval.Arg(0)->GetArray(), scriptObj->GetModIndex(), bDeepCopy);

	return true;
}

static bool Cmd_ar_Copy_Execute(COMMAND_ARGS)
{
	return ArrayCopyCommand(PASS_COMMAND_ARGS, false);
}

static bool Cmd_ar_DeepCopy_Execute(COMMAND_ARGS)
{
	return ArrayCopyCommand(PASS_COMMAND_ARGS, true);
}

static bool Cmd_ar_Null_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	return true;
}

static bool Cmd_ar_Resize_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() >= 2 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Number))
	{
		ArrayElement pad;
		if (eval.NumArgs() == 3)
			BasicTokenToElem(eval.Arg(2), pad, &eval);
		else
			pad.SetNumber(0.0);

		*result = g_ArrayMap.SetSize(eval.Arg(0)->GetArray(), eval.Arg(1)->GetNumber(), pad) ? 1 : 0;
	}

	return true;
}

static bool Cmd_ar_Append_Execute(COMMAND_ARGS)
{
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 2 && eval.Arg(0)->CanConvertTo(kTokenType_Array)) {
		ArrayID arrID = eval.Arg(0)->GetArray();
		ArrayElement toAppend;
		if (BasicTokenToElem(eval.Arg(1), toAppend, &eval)) {
			*result = g_ArrayMap.Insert(arrID, g_ArrayMap.SizeOf(arrID), toAppend) ? 1 : 0;
		}
	}

	return true;
}

static bool ar_Insert_Execute(COMMAND_ARGS, bool bRange)
{
	// inserts single element or range of elements in an Array-type array
	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() == 3 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Number))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		double index = eval.Arg(1)->GetNumber();

		if (bRange)
		{
			if (eval.Arg(2)->CanConvertTo(kTokenType_Array))
				*result = g_ArrayMap.Insert(arrID, index, eval.Arg(2)->GetArray()) ? 1 : 0;
		}
		else
		{
			ArrayElement toInsert;
			if (BasicTokenToElem(eval.Arg(2), toInsert, &eval))
				*result = g_ArrayMap.Insert(arrID, index, toInsert) ? 1 : 0;
		}
	}

	return true;
}

static bool Cmd_ar_Insert_Execute(COMMAND_ARGS)
{
	return ar_Insert_Execute(PASS_COMMAND_ARGS, false);
}

static bool Cmd_ar_InsertRange_Execute(COMMAND_ARGS)
{
	return ar_Insert_Execute(PASS_COMMAND_ARGS, true);
}

static bool Cmd_ar_List_Execute(COMMAND_ARGS)
{
	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs())
	{
		bool bContinue = true;
		for (UInt32 i = 0; i < eval.NumArgs(); i++)
		{
			ScriptToken* tok = eval.Arg(i)->ToBasicToken();
			ArrayElement elem;
			if (BasicTokenToElem(tok, elem, &eval))
				g_ArrayMap.SetElement(arrID, (ArrayKey)i, elem);
			else
				bContinue = false;

			delete tok;
			if (!bContinue)
				break;
		}
	}

	return true;
}

static bool Cmd_ar_Map_Execute(COMMAND_ARGS)
{
	// given at least one key-value pair, creates a map
	// if keys are found which don't match the type of the first key, they are ignored

	*result = 0;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0) {
		// first figure out the key type
		const TokenPair* pair = eval.Arg(0)->GetPair();
		if (!pair)
			return true;

		UInt32 keyType = kDataType_Invalid;
		if (pair->left->CanConvertTo(kTokenType_Number))
			keyType = kDataType_Numeric;
		else if (pair->left->CanConvertTo(kTokenType_String))
			keyType = kDataType_String;

		if (keyType == kDataType_Invalid)
			return true;

		// create the array and populate it
		ArrayID arrID = g_ArrayMap.Create(keyType, false, scriptObj->GetModIndex());
		*result = arrID;

		for (UInt32 i = 0; i < eval.NumArgs(); i++) {
			const TokenPair* pair = eval.Arg(i)->GetPair();
			if (pair) {
				// get the value first
				ScriptToken* val = pair->right->ToBasicToken();
				ArrayElement elem;
				if (BasicTokenToElem(val, elem, &eval)) {
					if (keyType == kDataType_String) {
						const char* key = pair->left->GetString();
						if (key) {
							g_ArrayMap.SetElement(arrID, key, elem);
						}
					}
					else if (pair->left->CanConvertTo(kTokenType_Number)) {
						double key = pair->left->GetNumber();
						g_ArrayMap.SetElement(arrID, key, elem);
					}
				}

				delete val;
			}
		}
	}

	return true;
}

static bool Cmd_ar_Range_Execute(COMMAND_ARGS)
{
	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	SInt32 start = 0;
	SInt32 end = 0;
	SInt32 step = 1;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() >= 2)
	{
		start = eval.Arg(0)->GetNumber();
		end = eval.Arg(1)->GetNumber();
		if (eval.NumArgs() == 3)
			step = eval.Arg(2)->GetNumber();

		UInt32 idx = 0;
		while (true)
		{
			if ((step > 0 && start > end) || (step < 0 && start < end))
				break;
			g_ArrayMap.SetElementNumber(arrID, idx, start);
			start += step;
			idx++;
		}
	}

	return true;
}

#else
#include "obse_editor\EditorAPI.h"
#endif

CommandInfo kCommandInfo_ar_Construct =
{
	"ar_Construct",
	"",
	0,
	"creates a new array",
	0,
	1,
	kParams_OneString,
	HANDLER(Cmd_ar_Construct_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneArray[] =
{
	{	"array",	kOBSEParamType_Array,	0	},
};

CommandInfo kCommandInfo_ar_Size =
{
	"ar_Size",
	"",
	0,
	"returns the size of an array or -1 if the array does not exist",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_Size_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_Dump =
{
	"ar_Dump",
	"",
	0,
	"dumps the contents of an array for debugging purposes.",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_Dump_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(ar_DumpID, dumps an array given an integer ID, 0, 1, kParams_OneInt);


static ParamInfo kParams_ar_Erase[] =
{
	{	"array",			kOBSEParamType_Array,	0	},
	{	"index or range",	kOBSEParamType_ArrayIndex | kOBSEParamType_Slice,	1	},
};

CommandInfo kCommandInfo_ar_Erase =
{
	"ar_Erase",
	"",
	0,
	"erases an element or range of elements from an array",
	0,
	2,
	kParams_ar_Erase,
	HANDLER(Cmd_ar_Erase_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_OneAssignableArray[] =
{
	{	"array",	kOBSEParamType_ArrayVarOrElement,	0	},
};

static ParamInfo kParams_ar_Sort[] =
{
	{	"array",		kOBSEParamType_Array,	0	},
	{	"bDescending",	kOBSEParamType_Number,	1	},
};

static ParamInfo kOBSEParams_ar_CustomSort[] =
{
	{	"array",		kOBSEParamType_Array,	0	},
	{	"comparator",	kOBSEParamType_Form,	0	},
	{	"bDescending",	kOBSEParamType_Number,	1	},
};

CommandInfo kCommandInfo_ar_Sort =
{
	"ar_Sort",
	"",
	0,
	"returns an array containing the source array's elements in sorted order",
	0,
	2,
	kParams_ar_Sort,
	HANDLER(Cmd_ar_Sort_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_CustomSort =
{
	"ar_CustomSort", "", 0,
	"returns an array containing the source array's elements sorted using the passed function script",
	0, 3, kOBSEParams_ar_CustomSort,
	HANDLER(Cmd_ar_CustomSort_Execute),
	Cmd_Expression_Parse,
	NULL, 0
};

CommandInfo kCommandInfo_ar_SortAlpha =
{
	"ar_SortAlpha",
	"",
	0,
	"returns an array containing the source array's elements in alphabetical order",
	0,
	2,
	kParams_ar_Sort,
	HANDLER(Cmd_ar_SortAlpha_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_ar_Find[] =
{
	{	"valueToFind",	kOBSEParamType_BasicType,	0	},
	{	"array",		kOBSEParamType_Array,		0	},
	{	"range",		kOBSEParamType_Slice,		1	},
};

CommandInfo kCommandInfo_ar_Find =
{
	"ar_Find",
	"",
	0,
	"finds the first occurence of a value within an array",
	0,
	3,
	kParams_ar_Find,
	HANDLER(Cmd_ar_Find_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_First =
{
	"ar_First",
	"",
	0,
	"returns the key of the first element in an array",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_First_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_Last =
{
	"ar_Last",
	"",
	0,
	"returns the key of the last element in an array",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_Last_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kParams_ArrayAndKey[] =
{
	{	"array",			kOBSEParamType_Array,		0	},
	{	"array index",	kOBSEParamType_ArrayIndex,	0	},
};

CommandInfo kCommandInfo_ar_Next =
{
	"ar_Next",
	"",
	0,
	"returns the key of the element immediately following the specified key",
	0,
	2,
	kParams_ArrayAndKey,
	HANDLER(Cmd_ar_Next_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_Prev =
{
	"ar_Prev",
	"",
	0,
	"returns the key of the element immediately preceding the specified key",
	0,
	2,
	kParams_ArrayAndKey,
	HANDLER(Cmd_ar_Prev_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_Keys =
{
	"ar_Keys",
	"",
	0,
	"returns an Array containing the keys of the specified array",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_Keys_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_HasKey =
{
	"ar_HasKey",
	"",
	0,
	"returns 1 if the array contains an element with the specified key",
	0,
	2,
	kParams_ArrayAndKey,
	HANDLER(Cmd_ar_HasKey_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(ar_BadStringIndex, returns the return value of a bad string array index, 0, 0, NULL);
DEFINE_COMMAND(ar_BadNumericIndex, returns the return value of a bad numeric array index, 0, 0, NULL);

CommandInfo kCommandInfo_ar_Copy =
{
	"ar_Copy",
	"",
	0,
	"returns a shallow copy of an array",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_Copy_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ar_DeepCopy =
{
	"ar_DeepCopy",
	"",
	0,
	"returns a deep copy of an array, copying any arrays contained in that array",
	0,
	1,
	kParams_OneArray,
	HANDLER(Cmd_ar_DeepCopy_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

DEFINE_COMMAND(ar_Null, makes an array variable refer to no array, 0, 0, NULL);

DEFINE_COMMAND(InstallModelMapHook, nothing, 0, 0, NULL);

static ParamInfo kOBSEParams_ar_Resize[3] =
{
	{	"array",		kOBSEParamType_Array,		0	},
	{	"newSize",		kOBSEParamType_Number,		0	},
	{	"padding",		kOBSEParamType_BasicType,	1	},
};

CommandInfo kCommandInfo_ar_Resize =
{
	"ar_Resize",
	"",
	0,
	"resizes an array, padding additional elements with zero or a supplied value",
	0,
	3,
	kOBSEParams_ar_Resize,
	HANDLER(Cmd_ar_Resize_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_ar_Insert[3] =
{
	{	"array",		kOBSEParamType_Array,		0	},
	{	"index",		kOBSEParamType_Number,		0	},
	{	"toInsert",		kOBSEParamType_BasicType,	0	},
};

CommandInfo kCommandInfo_ar_Insert =
{
	"ar_Insert",
	"",
	0,
	"inserts a single element in an array",
	0,
	3,
	kOBSEParams_ar_Insert,
	HANDLER(Cmd_ar_Insert_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_ar_InsertRange[3] =
{
	{	"array",		kOBSEParamType_Array,		0	},
	{	"index",		kOBSEParamType_Number,		0	},
	{	"range",		kOBSEParamType_Array,		0	},
};

CommandInfo kCommandInfo_ar_InsertRange =
{
	"ar_InsertRange",
	"",
	0,
	"inserts a range of elements into an array",
	0,
	3,
	kOBSEParams_ar_InsertRange,
	HANDLER(Cmd_ar_InsertRange_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_ar_Append[2] =
{
	{	"array",		kOBSEParamType_Array,		0	},
	{	"toInsert",		kOBSEParamType_BasicType,	0	},
};

CommandInfo kCommandInfo_ar_Append =
{
	"ar_Append", "", 0, "appends an element to an Array",
	0, 2, kOBSEParams_ar_Append,
	HANDLER(Cmd_ar_Append_Execute), Cmd_Expression_Parse, NULL, 0
};

static ParamInfo kOBSEParams_List[20] =
{
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
	{	"element",	kOBSEParamType_BasicType,	1	},
};

CommandInfo kCommandInfo_ar_List =
{
	"ar_List",
	"",
	0,
	"creates an array given a list of elements",
	0,
	20,
	kOBSEParams_List,
	HANDLER(Cmd_ar_List_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_PairList[20] =
{
	{	"key-value pair",	kOBSEParamType_Pair,	0	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
	{	"key-value pair",	kOBSEParamType_Pair,	1	},
};

CommandInfo kCommandInfo_ar_Map =
{
	"ar_Map",
	"",
	0,
	"creates a Map or Stringmap given a list of key-value pairs",
	0,
	20,
	kOBSEParams_PairList,
	HANDLER(Cmd_ar_Map_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_ar_Range[3] =
{
	{	"start",	kOBSEParamType_Number,	0	},
	{	"end",		kOBSEParamType_Number,	0	},
	{	"step",		kOBSEParamType_Number, 1	},
};

CommandInfo kCommandInfo_ar_Range =
{
	"ar_Range",
	"",
	0,
	"returns an array containing numbers beginning with 'start' up to and including 'end' in intervals 'step'",
	0,
	3,
	kOBSEParams_ar_Range,
	HANDLER(Cmd_ar_Range_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

