#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"

#if OBLIVION
#include "obse/GameAPI.h"

/*	As of 0020, ExtractArgsEx() and ExtractFormatStringArgs() are no longer directly included in plugin builds.
	They are available instead through the OBSEScriptInterface.
	To make it easier to update plugins to account for this, the following can be used.
	It requires that g_scriptInterface is assigned correctly when the plugin is first loaded.
*/
#define ENABLE_EXTRACT_ARGS_MACROS 1	// #define this as 0 if you prefer not to use this

#if ENABLE_EXTRACT_ARGS_MACROS

OBSEScriptInterface * g_scriptInterface = NULL;	// make sure you assign to this
#define ExtractArgsEx(...) g_scriptInterface->ExtractArgsEx(__VA_ARGS__)
#define ExtractFormatStringArgs(...) g_scriptInterface->ExtractFormatStringArgs(__VA_ARGS__)

#endif

#else
#include "obse_editor/EditorAPI.h"
#endif

#include "obse/ParamInfos.h"
#include "obse/Script.h"
#include "obse/GameObjects.h"
#include <string>

IDebugLog		gLog("obse_plugin_example.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;
OBSEScriptInterface			* g_scriptIntfc = NULL;

/*********************
	Array API Example
 *********************/

typedef OBSEArrayVarInterface::Array	OBSEArray;
typedef OBSEArrayVarInterface::Element	OBSEElement;

// helper function for creating an OBSE StringMap from a std::map<std::string, OBSEElement>
OBSEArray* StringMapFromStdMap(const std::map<std::string, OBSEElement>& data, Script* callingScript)
{
	// create empty string map
	OBSEArray* arr = g_arrayIntfc->CreateStringMap(NULL, NULL, 0, callingScript);

	// add each key-value pair
	for (std::map<std::string, OBSEElement>::const_iterator iter = data.begin(); iter != data.end(); ++iter) {
		g_arrayIntfc->SetElement(arr, iter->first.c_str(), iter->second);
	}

	return arr;
}

// helper function for creating an OBSE Map from a std::map<double, OBSEElement>
OBSEArray* MapFromStdMap(const std::map<double, OBSEElement>& data, Script* callingScript)
{
	OBSEArray* arr = g_arrayIntfc->CreateMap(NULL, NULL, 0, callingScript);
	for (std::map<double, OBSEElement>::const_iterator iter = data.begin(); iter != data.end(); ++iter) {
		g_arrayIntfc->SetElement(arr, iter->first, iter->second);
	}

	return arr;
}

// helper function for creating OBSE Array from std::vector<OBSEElement>
OBSEArray* ArrayFromStdVector(const std::vector<OBSEElement>& data, Script* callingScript)
{
	OBSEArray* arr = g_arrayIntfc->CreateArray(&data[0], data.size(), callingScript);
	return arr;
}

/***************************
* Serialization routines
***************************/

std::string	g_strData;

static void ResetData(void)
{
	g_strData.clear();
}

static void ExamplePlugin_SaveCallback(void * reserved)
{
	// write out the string
	g_serialization->OpenRecord('STR ', 0);
	g_serialization->WriteRecordData(g_strData.c_str(), g_strData.length());

	// write out some other data
	g_serialization->WriteRecord('ASDF', 1234, "hello world", 11);
}

static void ExamplePlugin_LoadCallback(void * reserved)
{
	UInt32	type, version, length;

	ResetData();

	char	buf[512];

	while(g_serialization->GetNextRecordInfo(&type, &version, &length))
	{
		_MESSAGE("record %08X (%.4s) %08X %08X", type, &type, version, length);

		switch(type)
		{
			case 'STR ':
				g_serialization->ReadRecordData(buf, length);
				buf[length] = 0;

				_MESSAGE("got string %s", buf);

				g_strData = buf;
				break;

			case 'ASDF':
				g_serialization->ReadRecordData(buf, length);
				buf[length] = 0;

				_MESSAGE("ASDF chunk = %s", buf);
				break;
			default:
				_MESSAGE("Unknown chunk type $08X", type);
		}
	}
}

static void ExamplePlugin_PreloadCallback(void * reserved)
{
	_MESSAGE("Preload Callback start");
	ExamplePlugin_LoadCallback(reserved);
	_MESSAGE("Preload Callback finished");
}

static void ExamplePlugin_NewGameCallback(void * reserved)
{
	ResetData();
}

/**********************
* Command handlers
**********************/

#if OBLIVION

bool Cmd_TestExtractArgsEx_Execute(COMMAND_ARGS)
{
	UInt32 i = 0;
	char str[0x200] = { 0 };
	*result = 0.0;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &i, str)) {
		Console_Print("TestExtractArgsEx >> int: %d str: %s", i, str);
		*result = 1.0;
	}
	else {
		Console_Print("TestExtractArgsEx >> couldn't extract arguments");
	}

	return true;
}

bool Cmd_TestExtractFormatString_Execute(COMMAND_ARGS)
{
	char str[0x200] = { 0 };
	int i = 0;
	TESForm* form = NULL;
	*result = 0.0;

	if (ExtractFormatStringArgs(0, str, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, 
		SIZEOF_FMT_STRING_PARAMS + 2, &i, &form)) {
			Console_Print("TestExtractFormatString >> str: %s int: %d form: %08X", str, i, form ? form->refID : 0);
			*result = 1.0;
	}
	else {
		Console_Print("TestExtractFormatString >> couldn't extract arguments.");
	}

	return true;
}

bool Cmd_ExamplePlugin_0019Additions_Execute(COMMAND_ARGS)
{
	// tests and demonstrates 0019 additions to plugin API
	// args:
	//	an array ID as an integer
	//	a function script with the signature {int, string, refr} returning a string
	// return:
	//	an array containing the keys and values of the original array

	UInt32 arrID = 0;
	TESForm* funcForm = NULL;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &arrID, &funcForm)) {

		// look up the array
		 OBSEArray* arr = g_arrayIntfc->LookupArrayByID(arrID);
		 if (arr) {
			 // get contents of array
			 UInt32 size = g_arrayIntfc->GetArraySize(arr);
			 if (size != -1) {
				 OBSEElement* elems = new OBSEElement[size];
				 OBSEElement* keys = new OBSEElement[size];

				 if (g_arrayIntfc->GetElements(arr, elems, keys)) {
					 OBSEArray* newArr = g_arrayIntfc->CreateArray(NULL, 0, scriptObj);
					 for (UInt32 i = 0; i < size; i++) {
						 g_arrayIntfc->SetElement(newArr, i*2, elems[i]);
						 g_arrayIntfc->SetElement(newArr, i*2+1, keys[i]);
					 }

					 // return the new array
					 g_arrayIntfc->AssignCommandResult(newArr, result);
				 }

				 delete[] elems;
				 delete[] keys;

			 }
		 }

		 if (funcForm) {
			 Script* func = OBLIVION_CAST(funcForm, TESForm, Script);
			 if (func) {
				 // call the function
				 OBSEElement funcResult;
				 if (g_scriptIntfc->CallFunction(func, thisObj, NULL, &funcResult, 3, 123456, "a string", *g_thePlayer)) {
					 if (funcResult.GetType() == funcResult.kType_String) {
						 Console_Print("Function script returned string %s", funcResult.String());
					 }
					 else {
						 Console_Print("Function did not return a string");
					 }
				 }
				 else {
					 Console_Print("Could not call function script");
				 }
			 }
			 else {
				 Console_Print("Could not extract function script argument");
			 }
		 }
	}

	return true;
}

bool Cmd_ExamplePlugin_MakeArray_Execute(COMMAND_ARGS)
{
	// Create an array of the format
	// { 
	//	 0:"Zero"
	//	 1:1.0
	//	 2:PlayerRef
	//	 3:StringMap { "A":"a", "B":123.456, "C":"manually set" }
	//	 4:"Appended"
	//	}

	// create the inner StringMap array
	std::map<std::string, OBSEElement> stringMap;
	stringMap["A"] = "a";
	stringMap["B"] = 123.456;

	// create the outer array
	std::vector<OBSEElement> vec;
	vec.push_back("Zero");
	vec.push_back(1.0);
	vec.push_back(*g_thePlayer);
	
	// convert our map to an OBSE StringMap and store in outer array
	OBSEArray* stringMapArr = StringMapFromStdMap(stringMap, scriptObj);
	vec.push_back(stringMapArr);

	// manually set another element in stringmap
	g_arrayIntfc->SetElement(stringMapArr, "C", "manually set");

	// convert outer array
	OBSEArray* arr = ArrayFromStdVector(vec, scriptObj);

	// append another element to array
	g_arrayIntfc->AppendElement(arr, "appended");

	if (!arr)
		Console_Print("Couldn't create array");

	// return the array
	if (!g_arrayIntfc->AssignCommandResult(arr, result))
		Console_Print("Couldn't assign array to command result.");

	// result contains the new ArrayID; print it
	Console_Print("Returned array ID %.0f", *result);

	return true;
}

bool Cmd_PluginTest_Execute(COMMAND_ARGS)
{
	_MESSAGE("plugintest");

	*result = 42;

	Console_Print("plugintest running");

	return true;
}

bool Cmd_ExamplePlugin_PrintString_Execute(COMMAND_ARGS)
{
	Console_Print("PrintString: %s", g_strData.c_str());

	return true;
}

bool Cmd_ExamplePlugin_SetString_Execute(COMMAND_ARGS)
{
	char	data[512];

	if(ExtractArgs(PASS_EXTRACT_ARGS, &data))
	{
		g_strData = data;
		Console_Print("Set string %s in script %08x", data, scriptObj->refID);
	}
	auto formatString = ScriptFormatStringArgs(0, 0, 0, 0);
	ExtractFormattedString( formatString, data);
	return true;
}

#endif

/**************************
* Command definitions
**************************/

static CommandInfo kPluginTestCommand =
{
	"plugintest",
	"",
	0,
	"test command for obse plugin",
	0,		// requires parent obj
	0,		// doesn't have params
	NULL,	// no param table

	HANDLER(Cmd_PluginTest_Execute)
};

static ParamInfo kParams_ExamplePlugin_0019Additions[2] =
{
	{ "array var", kParamType_Integer, 0 },
	{ "function script", kParamType_InventoryObject, 0 },
};

DEFINE_COMMAND_PLUGIN(ExamplePlugin_SetString, "sets a string", 0, 1, kParams_OneString)
DEFINE_COMMAND_PLUGIN(ExamplePlugin_PrintString, "prints a string", 0, 0, NULL)
DEFINE_COMMAND_PLUGIN(ExamplePlugin_MakeArray, test, 0, 0, NULL);
DEFINE_COMMAND_PLUGIN(ExamplePlugin_0019Additions, "tests 0019 API", 0, 2, kParams_ExamplePlugin_0019Additions);

static ParamInfo kParams_TestExtractArgsEx[2] =
{
	{	"int",		kParamType_Integer,	0	},
	{	"string",	kParamType_String,	0	},
};

static ParamInfo kParams_TestExtractFormatString[SIZEOF_FMT_STRING_PARAMS + 2] =
{
	FORMAT_STRING_PARAMS,
	{	"int",		kParamType_Integer,	0	},
	{	"object",	kParamType_InventoryObject,	0	},
};

DEFINE_COMMAND_PLUGIN(TestExtractArgsEx, "tests 0020 changes to arg extraction", 0, 2, kParams_TestExtractArgsEx);
DEFINE_COMMAND_PLUGIN(TestExtractFormatString, "tests 0020 changes to format string extraction", 0, 
					  SIZEOF_FMT_STRING_PARAMS+2, kParams_TestExtractFormatString);

/*************************
	Messaging API example
*************************/

OBSEMessagingInterface* g_msg;

void MessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_ExitGame:
		_MESSAGE("Plugin Example received ExitGame message");
		break;
	case OBSEMessagingInterface::kMessage_ExitToMainMenu:
		_MESSAGE("Plugin Example received ExitToMainMenu message");
		break;
	case OBSEMessagingInterface::kMessage_PostLoad:
		_MESSAGE("Plugin Example received PostLoad mesage");
		break;
	case OBSEMessagingInterface::kMessage_LoadGame:
	case OBSEMessagingInterface::kMessage_SaveGame:
		_MESSAGE("Plugin Example received save/load message with file path %s", msg->data);
		break;
	case OBSEMessagingInterface::kMessage_Precompile: 
		{
			ScriptBuffer* buffer = (ScriptBuffer*)msg->data;		
			_MESSAGE("Plugin Example received precompile message. Script Text:\n%s", buffer->scriptText);
			break;
		}
	case OBSEMessagingInterface::kMessage_PreLoadGame:
		_MESSAGE("Plugin Example received pre-loadgame message with file path %s", msg->data);
		break;
	case OBSEMessagingInterface::kMessage_ExitGame_Console:
		_MESSAGE("Plugin Example received quit game from console message");
		break;
	default:
		_MESSAGE("Plugin Example received unknown message");
		break;
	}
}

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "obse_plugin_example";
	info->version = 1;

	// version checks
	if(!obse->isEditor)
	{
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %u expected at least %u)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

#if OBLIVION
		if(obse->oblivionVersion != OBLIVION_VERSION)
		{
			_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
			return false;
		}
#endif

		g_serialization = (OBSESerializationInterface *)obse->QueryInterface(kInterface_Serialization);
		if(!g_serialization)
		{
			_ERROR("serialization interface not found");
			return false;
		}

		if(g_serialization->version < OBSESerializationInterface::kVersion)
		{
			_ERROR("incorrect serialization version found (got %08X need %08X)", g_serialization->version, OBSESerializationInterface::kVersion);
			return false;
		}

		g_arrayIntfc = (OBSEArrayVarInterface*)obse->QueryInterface(kInterface_ArrayVar);
		if (!g_arrayIntfc)
		{
			_ERROR("Array interface not found");
			return false;
		}

		g_scriptIntfc = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);		
	}
	else
	{
		// no version checks needed for editor
	}

	// version checks pass

	return true;
}

bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	_MESSAGE("load");

	g_pluginHandle = obse->GetPluginHandle();

	/***************************************************************************
	 *	
	 *	READ THIS!
	 *	
	 *	Before releasing your plugin, you need to request an opcode range from
	 *	the OBSE team and set it in your first SetOpcodeBase call. If you do not
	 *	do this, your plugin will create major compatibility issues with other
	 *	plugins, and may not load in future versions of OBSE. See
	 *	obse_readme.txt for more information.
	 *	
	 **************************************************************************/

	// register commands
	obse->SetOpcodeBase(0x2000);
	obse->RegisterCommand(&kPluginTestCommand);

	obse->RegisterCommand(&kCommandInfo_ExamplePlugin_SetString);
	obse->RegisterCommand(&kCommandInfo_ExamplePlugin_PrintString);

	// commands returning array must specify return type; type is optional for other commands
	obse->RegisterTypedCommand(&kCommandInfo_ExamplePlugin_MakeArray, kRetnType_Array);
	obse->RegisterTypedCommand(&kCommandInfo_ExamplePlugin_0019Additions, kRetnType_Array);

	obse->RegisterCommand(&kCommandInfo_TestExtractArgsEx);
	obse->RegisterCommand(&kCommandInfo_TestExtractFormatString);

	// set up serialization callbacks when running in the runtime
	if(!obse->isEditor)
	{
		// NOTE: SERIALIZATION DOES NOT WORK USING THE DEFAULT OPCODE BASE IN RELEASE BUILDS OF OBSE
		// it works in debug builds
		g_serialization->SetSaveCallback(g_pluginHandle, ExamplePlugin_SaveCallback);
		g_serialization->SetLoadCallback(g_pluginHandle, ExamplePlugin_LoadCallback);
		g_serialization->SetNewGameCallback(g_pluginHandle, ExamplePlugin_NewGameCallback);
#if 0	// enable below to test Preload callback, don't use unless you actually need it
		g_serialization->SetPreloadCallback(g_pluginHandle, ExamplePlugin_PreloadCallback);
#endif

		// register to use string var interface
		// this allows plugin commands to support '%z' format specifier in format string arguments
		OBSEStringVarInterface* g_Str = (OBSEStringVarInterface*)obse->QueryInterface(kInterface_StringVar);
		g_Str->Register(g_Str);

		// get an OBSEScriptInterface to use for argument extraction
		g_scriptInterface = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);
	}

	// register to receive messages from OBSE
	OBSEMessagingInterface* msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
	msgIntfc->RegisterListener(g_pluginHandle, "OBSE", MessageHandler);
	g_msg = msgIntfc;

	// get command table, if needed
	OBSECommandTableInterface* cmdIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);
	if (cmdIntfc) {
#if 0	// enable the following for loads of log output
		for (const CommandInfo* cur = cmdIntfc->Start(); cur != cmdIntfc->End(); ++cur) {
			_MESSAGE("%s",cur->longName);
		}
#endif
	}
	else {
		_MESSAGE("Couldn't read command table");
	}

	return true;
}

};
