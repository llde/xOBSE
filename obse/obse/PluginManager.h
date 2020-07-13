#pragma once

#include "obse/PluginAPI.h"

class PluginManager
{
public:
	PluginManager();
	~PluginManager();

	bool	Init(void);
	void	DeInit(void);

	PluginInfo *	GetInfoByName(const char * name);
	PluginInfo *	GetInfoFromHandle(PluginHandle handle);
	PluginInfo *	GetInfoFromBase(UInt32 baseOpcode);
	const char *	GetPluginNameFromHandle(PluginHandle handle);

	UInt32			GetNumPlugins(void);
	UInt32			GetBaseOpcode(UInt32 idx);
	PluginHandle	LookupHandleFromBaseOpcode(UInt32 baseOpcode);
	PluginHandle	LookupHandleFromName(const char* pluginName);

	static bool			RegisterCommand(CommandInfo * _info);
	static bool			RegisterTypedCommand(CommandInfo * _info, CommandReturnType retnType);
	static void			SetOpcodeBase(UInt32 opcode);
	static void *		QueryInterface(UInt32 id);
	static PluginHandle	GetPluginHandle(void);
	static const char *	GetOblivionDir();
	static bool			GetPluginLoaded(const char* pluginName);
	static UInt32		GetPluginVersion(const char* pluginName);

	static bool Dispatch_Message(PluginHandle sender, UInt32 messageType, void * data, UInt32 dataLen, const char* receiver);
	static bool	RegisterListener(PluginHandle listener, const char* sender, OBSEMessagingInterface::EventCallback handler);

private:
	struct LoadedPlugin
	{
		HMODULE		handle;
		PluginInfo	info;
		UInt32		baseOpcode;

		_OBSEPlugin_Query	query;
		_OBSEPlugin_Load	load;
	};

	bool	FindPluginDirectory(void);
	void	InstallPlugins(void);

	const char *	SafeCallQueryPlugin(LoadedPlugin * plugin, const OBSEInterface * obse);
	const char *	SafeCallLoadPlugin(LoadedPlugin * plugin, const OBSEInterface * obse);

	const char *	CheckPluginCompatibility(LoadedPlugin * plugin);
	const char *	EarlyCheckPluginCompatibility(LoadedPlugin * plugin, const char * fileName);

	typedef std::vector <LoadedPlugin>	LoadedPluginList;

	std::string			m_pluginDirectory;
	LoadedPluginList	m_plugins;

	static LoadedPlugin		* s_currentLoadingPlugin;
	static PluginHandle		s_currentPluginHandle;
};

extern PluginManager	g_pluginManager;

extern CommandInfo kCommandInfo_IsPluginInstalled;
extern CommandInfo kCommandInfo_GetPluginVersion;
