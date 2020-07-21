#include "CommandTable.h"
#include "PluginManager.h"
#include "common/IDirectoryIterator.h"
#include "Commands_Console.h"
#include "ParamInfos.h"
#include "Utilities.h"
#include "obse_common/SafeWrite.h"

#ifdef OBLIVION
#include "GameAPI.h"
#include "Serialization.h"
#include "StringVar.h"
#include "ArrayVar.h"
#include "Hooks_DirectInput8Create.h"
#include "FunctionScripts.h"

#else

#include "Hooks_Script.h"

#endif

PluginManager	g_pluginManager;

PluginManager::LoadedPlugin *	PluginManager::s_currentLoadingPlugin = NULL;
PluginHandle					PluginManager::s_currentPluginHandle = 0;

#ifdef OBLIVION
static OBSEConsoleInterface g_OBSEConsoleInterface =
{
	OBSEConsoleInterface::kVersion,
	RunScriptLine,
	RunScriptLineOnREFR
};

static OBSEStringVarInterface g_OBSEStringVarInterface =
{
	OBSEStringVarInterface::kVersion,
	PluginAPI::GetString,
	PluginAPI::SetString,
	PluginAPI::CreateString,
	RegisterStringVarInterface,
	AssignToStringVar
};

static OBSEIOInterface g_OBSEIOInterface = 
{
	OBSEIOInterface::kVersion,
	Plugin_IsKeyPressed
};

static OBSEArrayVarInterface g_OBSEArrayVarInterface =
{
	PluginAPI::ArrayAPI::CreateArray,
	PluginAPI::ArrayAPI::CreateStringMap,
	PluginAPI::ArrayAPI::CreateMap,
	PluginAPI::ArrayAPI::AssignArrayCommandResult,
	PluginAPI::ArrayAPI::SetElement,
	PluginAPI::ArrayAPI::AppendElement,
	PluginAPI::ArrayAPI::GetArraySize,
	PluginAPI::ArrayAPI::LookupArrayByID,
	PluginAPI::ArrayAPI::GetElement,
	PluginAPI::ArrayAPI::GetElements,
};

static OBSEScriptInterface g_OBSEScriptInterface =
{
	PluginAPI::CallFunctionScript,
	UserFunctionManager::GetFunctionParamTypes,
	ExtractArgsEx,
	ExtractFormatStringArgs,
	PluginAPI::IsUserFunction
};

#endif

static OBSEMessagingInterface g_OBSEMessagingInterface =
{
	OBSEMessagingInterface::kVersion,
	PluginManager::RegisterListener,
	PluginManager::Dispatch_Message
};

static OBSECommandTableInterface g_OBSECommandTableInterface =
{
	PluginAPI::GetCmdTblStart,
	PluginAPI::GetCmdTblEnd,
	PluginAPI::GetCmdByOpcode,
	PluginAPI::GetCmdByName,
	PluginAPI::GetCmdRetnType,
	PluginAPI::GetReqVersion,
	PluginAPI::GetCmdParentPlugin
};

static const OBSEInterface g_OBSEInterface =
{
	OBSE_VERSION_INTEGER,

#ifdef OBLIVION
	OBLIVION_VERSION,
	0,
	0,
#else
	0,
	CS_VERSION,
	1,
#endif
	PluginManager::RegisterCommand,
	PluginManager::SetOpcodeBase,
	PluginManager::QueryInterface,
	PluginManager::GetPluginHandle,
	PluginManager::RegisterTypedCommand,
	PluginManager::GetOblivionDir,
	PluginManager::GetPluginLoaded,
	PluginManager::GetPluginVersion,
};

PluginManager::PluginManager()
{
	//
}

PluginManager::~PluginManager()
{
	DeInit();
}

bool PluginManager::Init(void)
{
	bool	result = false;
	if(FindPluginDirectory())
	{
		_MESSAGE("plugin directory = %s", m_pluginDirectory.c_str());

		__try
		{
			InstallPlugins();

			result = true;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			// something very bad happened
			_ERROR("exception occurred while loading plugins");
		}
	}

	return result;
}

void PluginManager::DeInit(void)
{
	for(LoadedPluginList::iterator iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		LoadedPlugin	* plugin = &(*iter);

		if(plugin->handle)
		{
			FreeLibrary(plugin->handle);
		}
	}

	m_plugins.clear();
}

UInt32 PluginManager::GetNumPlugins(void)
{
	UInt32	numPlugins = m_plugins.size();

	// is one currently loading?
	if(s_currentLoadingPlugin) numPlugins++;

	return numPlugins;
}

UInt32 PluginManager::GetBaseOpcode(UInt32 idx)
{
	return m_plugins[idx].baseOpcode;
}

PluginHandle PluginManager::LookupHandleFromBaseOpcode(UInt32 baseOpcode)
{
	UInt32	idx = 1;

	for(LoadedPluginList::iterator iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		LoadedPlugin	* plugin = &(*iter);

		if(plugin->baseOpcode == baseOpcode)
			return idx;

		idx++;
	}

	return kPluginHandle_Invalid;
}

PluginInfo * PluginManager::GetInfoByName(const char * name)
{
	for(LoadedPluginList::iterator iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		LoadedPlugin	* plugin = &(*iter);

		if(plugin->info.name && !_stricmp(name, plugin->info.name))
			return &plugin->info;
	}

	return NULL;
}

PluginInfo * PluginManager::GetInfoFromHandle(PluginHandle handle)
{
	if(handle > 0 && handle <= m_plugins.size())
		return &m_plugins[handle - 1].info;

	return NULL;
}

PluginInfo * PluginManager::GetInfoFromBase(UInt32 baseOpcode)
{
	PluginHandle	handle = LookupHandleFromBaseOpcode(baseOpcode);

	if(handle > 0 && handle <= m_plugins.size())
		return &m_plugins[handle - 1].info;

	return NULL;
}

const char * PluginManager::GetPluginNameFromHandle(PluginHandle handle)
{
	if (handle > 0 && handle <= m_plugins.size())
		return (m_plugins[handle - 1].info.name);
	else if (handle == 0)
		return "OBSE";

	return NULL;
}

bool PluginManager::RegisterCommand(CommandInfo* _info)
{
	return RegisterTypedCommand(_info, kRetnType_Default);
}

bool PluginManager::RegisterTypedCommand(CommandInfo * _info, CommandReturnType retnType)
{
	ASSERT(_info);
	ASSERT_STR(s_currentLoadingPlugin, "PluginManager::RegisterCommand: called outside of plugin load");

	CommandInfo	info = *_info;

#ifndef OBLIVION
	// modify callbacks for editor

	info.execute = Cmd_Default_Execute;
	info.eval = NULL;	// not supporting this yet
#endif

	if(!info.parse) {
#ifndef OBLIVION
		// 0020 incremented PluginInfo::kInfoVersion to 2. Plugins compiled with previous versions of OBSE are not
		// necessarily able to take advantage of CompilerOverride functionality; to be on the safe side we disallow it.

		if (s_currentLoadingPlugin->info.infoVersion < 2) {
			info.parse = CompilerOverride::Cmd_Plugin_Default_Parse;
		}
		else {
			info.parse = Cmd_Default_Parse;
		}
#else
		info.parse = Cmd_Default_Parse;
#endif
	}

	if(!info.shortName) info.shortName = "";
	if(!info.helpText) info.helpText = "";

	_MESSAGE("RegisterCommand %s (%04X)", info.longName, g_scriptCommands.GetCurID());

	if (retnType >= kRetnType_Max)
		retnType = kRetnType_Default;

	g_scriptCommands.Add(&info, retnType, s_currentLoadingPlugin->baseOpcode);

	return true;
}

void PluginManager::SetOpcodeBase(UInt32 opcode)
{
	_MESSAGE("SetOpcodeBase %08X", opcode);

	ASSERT(opcode < 0x8000);	// arbitrary maximum for sanity check
	ASSERT(opcode >= 0x2000);	// beginning of plugin opcode space
	ASSERT_STR(s_currentLoadingPlugin, "PluginManager::SetOpcodeBase: called outside of plugin load");

	if(opcode == 0x2000)
	{
		const char	* pluginName = "<unknown name>";

		if(s_currentLoadingPlugin && s_currentLoadingPlugin->info.name)
			pluginName = s_currentLoadingPlugin->info.name;

		_ERROR("You have a plugin installed that is using the default opcode base. (%s)", pluginName);
		_ERROR("This is acceptable for temporary development, but not for plugins released to the public.");
		_ERROR("As multiple plugins using the same opcode base create compatibility issues, plugins triggering this message may not load in future versions of OBSE.");
		_ERROR("Please contact the authors of the plugin and have them request and begin using an opcode range assigned by the OBSE team.");

#ifdef _DEBUG
		_ERROR("WARNING: serialization is being allowed for this plugin as this is a debug build of OBSE. It will not work in release builds.");
#endif
	}
#ifndef _DEBUG
	else	// disallow plugins using default opcode base from using it as a unique id
#endif
	{
		// record the first opcode registered for this plugin
		if(!s_currentLoadingPlugin->baseOpcode)
			s_currentLoadingPlugin->baseOpcode = opcode;
	}

	g_scriptCommands.PadTo(opcode);
	g_scriptCommands.SetCurID(opcode);
}

void * PluginManager::QueryInterface(UInt32 id)
{
	void	* result = NULL;


	switch(id)
	{
#ifdef OBLIVION
		case kInterface_Console:
			result = (void *)&g_OBSEConsoleInterface;
			break;

		case kInterface_Serialization:
			result = (void *)&g_OBSESerializationInterface;
			break;

		case kInterface_StringVar:
			result = (void *)&g_OBSEStringVarInterface;
			break;
		case kInterface_ArrayVar:
			result = (void*)&g_OBSEArrayVarInterface;
			break;
		case kInterface_IO:
			result = (void *)&g_OBSEIOInterface;
			break;
		case kInterface_Script:
			result = (void *)&g_OBSEScriptInterface;
			break;
#endif
		case kInterface_Messaging:
			result = (void *)&g_OBSEMessagingInterface;
			break;
		case kInterface_CommandTable:
			result = (void*)&g_OBSECommandTableInterface;
			break;
		default:
			_WARNING("unknown QueryInterface %08X", id);
			break;
	}
	
	return result;
}

PluginHandle PluginManager::GetPluginHandle(void)
{
	ASSERT_STR(s_currentPluginHandle, "A plugin has called OBSEInterface::GetPluginHandle outside of its Query/Load handlers");

	return s_currentPluginHandle;
}

bool PluginManager::FindPluginDirectory(void)
{
	bool	result = false;

	// find the path <oblivion directory>/data/obse/
	std::string	oblivionDirectory = GetOblivionDir();
	
	if(!oblivionDirectory.empty())
	{
		m_pluginDirectory = oblivionDirectory + "Data\\OBSE\\Plugins\\";
		result = true;
	}

	return result;
}

void PluginManager::InstallPlugins(void)
{
	// avoid realloc
	m_plugins.reserve(5);

	for(IDirectoryIterator iter(m_pluginDirectory.c_str(), "*.dll"); !iter.Done(); iter.Next())
	{
		std::string	pluginPath = iter.GetFullPath();

		_MESSAGE("checking plugin %s", pluginPath.c_str());

		LoadedPlugin	plugin;
		memset(&plugin, 0, sizeof(plugin));

		s_currentLoadingPlugin = &plugin;
		s_currentPluginHandle = m_plugins.size() + 1;	// +1 because 0 is reserved for internal use

		plugin.handle = (HMODULE)LoadLibrary(pluginPath.c_str());
		if(plugin.handle)
		{
			bool		success = false;

			plugin.query = (_OBSEPlugin_Query)GetProcAddress(plugin.handle, "OBSEPlugin_Query");
			plugin.load = (_OBSEPlugin_Load)GetProcAddress(plugin.handle, "OBSEPlugin_Load");

			if(plugin.query && plugin.load)
			{
				const char	* loadStatus = NULL;

				loadStatus = EarlyCheckPluginCompatibility(&plugin, iter.Get()->cFileName);

				if(!loadStatus)
				{
					loadStatus = SafeCallQueryPlugin(&plugin, &g_OBSEInterface);

					if(!loadStatus)
					{
						loadStatus = CheckPluginCompatibility(&plugin);

						if(!loadStatus)
						{
							loadStatus = SafeCallLoadPlugin(&plugin, &g_OBSEInterface);

							if(!loadStatus)
							{
								loadStatus = "loaded correctly";
								success = true;
							}
						}
					}
					else
					{
						loadStatus = "reported as incompatible during query";
					}
				}

				ASSERT(loadStatus);

				_MESSAGE("plugin %s (%08X %s %08X) %s",
						pluginPath.c_str(),
						plugin.info.infoVersion,
						plugin.info.name ? plugin.info.name : "<NULL>",
						plugin.info.version,
						loadStatus);
			}
			else
			{
				_MESSAGE("plugin %s does not appear to be an OBSE plugin", pluginPath.c_str());
			}
			
			if(success)
			{
				// succeeded, add it to the list
				m_plugins.push_back(plugin);
			}
			else
			{
				// failed, unload the library
				FreeLibrary(plugin.handle);
			}
		}
		else
		{
			DWORD errCode = GetLastError();
			_ERROR("couldn't load plugin %s (Error code %d (%08X)", pluginPath.c_str(), errCode, errCode);
		}
	}

	s_currentLoadingPlugin = NULL;
	s_currentPluginHandle = 0;

	// alert any listeners that plugin load has finished
	Dispatch_Message(0, OBSEMessagingInterface::kMessage_PostLoad, NULL, 0, NULL);
	// second post-load dispatch
	Dispatch_Message(0, OBSEMessagingInterface::kMessage_PostPostLoad, NULL, 0, NULL);
}

// SEH-wrapped calls to plugin API functions to avoid bugs from bringing down the core
const char * PluginManager::SafeCallQueryPlugin(LoadedPlugin * plugin, const OBSEInterface * obse)
{
	__try
	{
		if(!plugin->query(obse, &plugin->info))
		{
			return "reported as incompatible during query";
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// something very bad happened
		return "disabled, fatal error occurred while querying plugin";
	}

	return NULL;
}

const char * PluginManager::SafeCallLoadPlugin(LoadedPlugin * plugin, const OBSEInterface * obse)
{
	__try
	{
		if(!plugin->load(obse))
		{
			return "reported as incompatible during load";
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// something very bad happened
		return "disabled, fatal error occurred while loading plugin";
	}

	return NULL;
}

struct MinVersionEntry
{
	const char	* name;
	UInt32		minVersion;
	const char	* reason;
};

static const MinVersionEntry	kMinVersionList[] =
{
	{	"OBSE_Elys_NoFastTravel",	92,
		"disabled, versions before 92 crash OBSE, please get the latest version of the plugin" },
	{	NULL, 0, NULL }
};

// see if we have a plugin that we know causes problems
const char * PluginManager::CheckPluginCompatibility(LoadedPlugin * plugin)
{
	__try
	{
		// stupid plugin check
		if(!plugin->info.name)
		{
			return "disabled, no name specified";
		}

		// check for 'known bad' versions of plugins
		for(const MinVersionEntry * iter = kMinVersionList; iter->name; ++iter)
		{
			if(!strcmp(iter->name, plugin->info.name))
			{
				if(plugin->info.version < iter->minVersion)
				{
					return iter->reason;
				}
				
				break;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// paranoia
		return "disabled, fatal error occurred while checking plugin compatibility";
	}

	return NULL;
}

// see if we have a plugin that we know causes problems AND is broken enough that we can't even call OBSEPlugin_Query
const char * PluginManager::EarlyCheckPluginCompatibility(LoadedPlugin * plugin, const char * fileName)
{
	const char	* result = NULL;
	UInt32		imageBase = (UInt32)plugin->handle;

#ifndef OBLIVION
	// HUD version of Pluggy (v132, possibly others) installs the same directx hook without checking whether or not it is being loaded in the runtime
	// this overwrites random code in the editor
	// there is also no versioning information on the dll, so time for a crappy heuristic
	// linker timestamp is fixed at 0x2A425E19 for all delphi projects so can't use that, checksum is 0x00000000
	// basically a giant load of stupid
	if(!_stricmp(fileName, "OBSE_Elys_Pluggy_HUD.dll"))
	{
		_MESSAGE("checking for pluggy HUD bug");

		// check if OBSEPlugin_Query is at the right address
		UInt32	expectedQueryPtr = 0x003E1EDC - 0x003B0000 + imageBase;

		if(expectedQueryPtr == (UInt32)plugin->query)
		{
			_MESSAGE("OBSEPlugin_Query matches");

			// ok, probably v132

			// check some code in the function we're going to patch
			// won't be relocated or changed and should be very unique
			UInt32		codeCheckPtr = 0x003C96F7 - 0x003B0000 + imageBase;
			const UInt8	kCodeToCheck[] =
			{
				0x81, 0xEA, 0x56, 0x1E, 0x76, 0x00,	// sub edx, 0x00761E56
				0xB8, 0x52, 0x1E, 0x76, 0x00		// mov eax, 0x00761E52
			};

			if(!memcmp((void *)codeCheckPtr, kCodeToCheck, sizeof(kCodeToCheck)))
			{
				_MESSAGE("InitializationDXHook matches");

				// ok, pretty sure at this point. patch the function hooking the code

				_MESSAGE("patching the bug");

				SafeWrite8(0x003C96F2 - 0x003B0000 + imageBase, 0xC3);	// early return to skip all the writes
			}
		}
	}
#endif

	return result;
}

// Plugin communication interface
struct PluginListener {
	PluginHandle	listener;
	OBSEMessagingInterface::EventCallback	handleMessage;
};

typedef std::vector<std::vector<PluginListener> > PluginListeners;
static PluginListeners s_pluginListeners;

bool PluginManager::RegisterListener(PluginHandle listener, const char* sender, OBSEMessagingInterface::EventCallback handler)
{
	// because this can be called while plugins are loading, gotta make sure number of plugins hasn't increased
	UInt32 numPlugins = g_pluginManager.GetNumPlugins() + 1;
	if (s_pluginListeners.size() < numPlugins)
		s_pluginListeners.resize(numPlugins + 5);	// add some extra room to avoid unnecessary re-alloc

	// handle > num plugins = invalid
	if (listener > g_pluginManager.GetNumPlugins() || !handler)
		return false;

	if (sender)
	{
		// is target loaded?
		PluginHandle target = g_pluginManager.LookupHandleFromName(sender);
		if (target == kPluginHandle_Invalid)
			return false;

		// is listener already registered?
		for (std::vector<PluginListener>::iterator iter = s_pluginListeners[target].begin(); iter != s_pluginListeners[target].end(); ++iter)
		{
			if (iter->listener == listener)
				return true;
		}

		// register new listener
		PluginListener newListener;
		newListener.handleMessage = handler;
		newListener.listener = listener;

		s_pluginListeners[target].push_back(newListener);
	}
	else
	{
		// register listener to every loaded plugin
		UInt32 idx = 0;
		for(PluginListeners::iterator iter = s_pluginListeners.begin(); iter != s_pluginListeners.end(); ++iter)
		{
			// don't add the listener to its own list
			if (idx && idx != listener)
			{
				bool skipCurrentList = false;
				for (std::vector<PluginListener>::iterator iterEx = iter->begin(); iterEx != iter->end(); ++iterEx)
				{
					if (iterEx->listener == listener)
					{
						skipCurrentList = true;
						break;
					}
				}
				if (skipCurrentList)
					continue;

				PluginListener newListener;
				newListener.handleMessage = handler;
				newListener.listener = listener;

				iter->push_back(newListener);
			}
			idx++;
		}	
	}
	return true;	
}

bool PluginManager::Dispatch_Message(PluginHandle sender, UInt32 messageType, void * data, UInt32 dataLen, const char* receiver)
{
	UInt32 numRespondents = 0;
	PluginHandle target = kPluginHandle_Invalid;

	if (!s_pluginListeners.size())	// no listeners yet registered
		return false;
	else if (sender >= s_pluginListeners.size())
		return false;

	if (receiver)
	{
		target = g_pluginManager.LookupHandleFromName(receiver);
		if (target == kPluginHandle_Invalid)
			return false;
	}

	const char* senderName = g_pluginManager.GetPluginNameFromHandle(sender);
	if (!senderName)
		return false;

	// s_pluginListeners can be resized during this call, iterate via index
	for(UInt32 i = 0; i < s_pluginListeners[sender].size(); i++)
	{
		PluginListener	* listener = &s_pluginListeners[sender][i];

		OBSEMessagingInterface::Message msg;
		msg.data = data;
		msg.type = messageType;
		msg.sender = senderName;
		msg.dataLen = dataLen;

		if (target != kPluginHandle_Invalid)	// sending message to specific plugin
		{
			if (listener->listener == target)
			{
				listener->handleMessage(&msg);
				listener = NULL;	// now potentially invalid
				return true;
			}
		}
		else
		{
			listener->handleMessage(&msg);
			listener = NULL;	// now potentially invalid
			numRespondents++;
		}
	}

	return numRespondents ? true : false;
}

PluginHandle PluginManager::LookupHandleFromName(const char* pluginName)
{
	if (!_stricmp("OBSE", pluginName))
		return 0;

	UInt32	idx = 1;

	for(LoadedPluginList::iterator iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		LoadedPlugin	* plugin = &(*iter);
		if(!_stricmp(plugin->info.name, pluginName))
			return idx;

		idx++;
	}

	return kPluginHandle_Invalid;
}

const char* PluginManager::GetOblivionDir()
{
	static std::string obDir(GetOblivionDirectory());
	return obDir.c_str();
}

bool PluginManager::GetPluginLoaded( const char* pluginName )
{
	if (pluginName == NULL)
		return false;

	PluginInfo* plugin = g_pluginManager.GetInfoByName(pluginName);
	if (plugin)
		return true;
	else
		return false;
}

UInt32 PluginManager::GetPluginVersion( const char* pluginName )
{
	if (pluginName == NULL)
		return 0;

	PluginInfo* plugin = g_pluginManager.GetInfoByName(pluginName);
	if (plugin)
		return plugin->version;
	else
		return 0;
}

#ifdef OBLIVION

bool Cmd_IsPluginInstalled_Execute(COMMAND_ARGS)
{
	char	pluginName[256];

	*result = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &pluginName)) return true;

	*result = (g_pluginManager.GetInfoByName(pluginName) != NULL) ? 1 : 0;
	if (IsConsoleMode()) Console_Print("Plugin %s  is %s", pluginName, *result == 1 ? "Active" : "Not Active");
	return true;
}

bool Cmd_GetPluginVersion_Execute(COMMAND_ARGS)
{
	char	pluginName[256];

	*result = -1;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &pluginName)) return true;

	PluginInfo	* info = g_pluginManager.GetInfoByName(pluginName);
	
	if(info) *result = info->version;
	if (IsConsoleMode()) Console_Print("Plugin %s  version %d", pluginName, *result);

	return true;
}

#endif

CommandInfo kCommandInfo_IsPluginInstalled =
{
	"IsPluginInstalled",
	"",
	0,
	"returns 1 if the specified plugin is installed, else 0",
	0,
	1,
	kParams_OneString,

	HANDLER(Cmd_IsPluginInstalled_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};

CommandInfo kCommandInfo_GetPluginVersion =
{
	"GetPluginVersion",
	"",
	0,
	"returns the version of the specified plugin, or -1 if the plugin is not installed",
	0,
	1,
	kParams_OneString,

	HANDLER(Cmd_GetPluginVersion_Execute),
	Cmd_Default_Parse,
	NULL,
	NULL
};
