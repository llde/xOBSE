#pragma once

#include <string>

/*
 * To deal with the merging of mods through Wrye Bash, allows mod names to be aliased.
 * For example, if mods A and B are merged into one .esp called C, any reference to
 * either A or B should be fixed up to refer to C; A, B, and C effectively share a
 * single mod index.
 * This allows commands like IsModLoaded and GetFormFromMod to function even if
 * the specified mod name is an alias to a merged mod.
 * It also allows obse's string/array datatypes, which are associated with the mod
 * index of the script which created them, to be preserved when their owning mods 
 * are merged.
 */

class ModTable
{
public:
	bool		IsModLoaded (const std::string& name);
	UInt8		GetModIndex (const std::string& name);
	bool		SetAlias (const std::string& name, UInt8 modIndex);
	std::string	GetAlias (const std::string& actualName);

	static ModTable& Get ();
private:
	static ModTable s_modTable;

	ModTable ();

	typedef std::map <std::string, UInt8> AliasMap;

	AliasMap	m_aliasMap;
};

	