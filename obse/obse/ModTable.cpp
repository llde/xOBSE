#include "ModTable.h"
#include "GameData.h"

ModTable ModTable::s_modTable;

ModTable::ModTable ()
{
	//
}

ModTable& ModTable::Get ()
{
	return s_modTable;
}

bool ModTable::SetAlias (const std::string& name, UInt8 modIndex)
{
	if (0xFF == modIndex 
		|| 0 == modIndex
		|| 0xFF != GetModIndex (name))
		return false;

	m_aliasMap[name] = modIndex;
	return true;
}

bool ModTable::IsModLoaded (const std::string& name)
{
	const ModEntry* modEntry = (*g_dataHandler)->LookupModByName (name.c_str ());
	if (modEntry)
		return modEntry->IsLoaded ();
	else	// alias?
		return 0xFF != GetModIndex (name);
}

UInt8 ModTable::GetModIndex (const std::string& name)
{
	UInt8 index = (*g_dataHandler)->GetModIndex (name.c_str ());
	if (0xFF == index)
	{
		AliasMap::const_iterator iter = m_aliasMap.find (name);
		if (m_aliasMap.end () != iter)
			index = iter->second;
	}

	return index;
}

std::string ModTable::GetAlias (const std::string& actualName)
{
	UInt8 index = GetModIndex (actualName);
	if (0xFF != index)
		return (*g_dataHandler)->GetNthModName (index);
	else
		return "";
}

