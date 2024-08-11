#include <string>
#include "StringVar.h"
#include "GameForms.h"
#include <algorithm>
#include "Script.h"
#include "Hooks_Script.h"
#include "ScriptUtils.h"
#include "GameData.h"


wchar_t* ConvertToWideString(const char* string) {
	int sizeMultibyteBuffer = MultiByteToWideChar(CP_ACP, 0, string, -1, nullptr, 0);
	wchar_t* widestring = new wchar_t[sizeMultibyteBuffer];
	MultiByteToWideChar(CP_ACP, 0, string, -1, widestring, sizeMultibyteBuffer);
	return widestring;
}


char* ConvertToMultibyteString(const wchar_t* widestring) {
	int sizeMultibyteBuffer = WideCharToMultiByte(CP_ACP, 0, widestring, -1, nullptr, 0, nullptr,nullptr);
	char* string = new char[sizeMultibyteBuffer];
	WideCharToMultiByte(CP_ACP, 0, widestring, -1, string, sizeMultibyteBuffer,nullptr,nullptr);
	return string;
}

StringVar::StringVar(const char* in_data, UInt32 in_refID)
{
	wchar_t* widestring = ConvertToWideString(in_data);
	data = std::wstring(widestring);
	delete[] widestring;
	owningModIndex = in_refID >> 24;
}

std::string StringVar::String() {
	char* string = ConvertToMultibyteString(data.c_str());
	return std::string(string);
}

const char* StringVar::GetCString()
{
	if (modified == true || multibyte_ptr.get() == nullptr) {
		multibyte_ptr = std::unique_ptr<const char[]>(ConvertToMultibyteString(data.c_str()));
	}
	return multibyte_ptr.get();
}

void StringVar::Set(const char* newString)
{
	wchar_t* widestring = ConvertToWideString(newString);
	data = std::wstring(widestring);
	delete[] widestring;
	modified = true;
}

SInt32 StringVar::Compare(char* rhs, bool caseSensitive)
{
	SInt32 cmp = 0;
	wchar_t* widecmp = ConvertToWideString(rhs);
	if (!caseSensitive)
	{
		cmp = _wcsicmp(data.c_str(), widecmp);
		delete[] widecmp;
		if (cmp > 0)
			return -1;
		else if (cmp < 0)
			return 1;
		else
			return 0;
	}
	else
	{
		cmp = wcscmp(data.c_str(), widecmp);
		delete[] widecmp;

		if (cmp > 0)
			return -1;
		else if (cmp < 0)
			return 1;
		else
			return 0;

		if (cmp > 0)
			return -1;
		else if (cmp < 0)
			return 1;
		else
			return 0;
	}
}

void StringVar::Insert(const char* subString, UInt32 insertionPos)
{
	wchar_t* wide = ConvertToWideString(subString);
	if (insertionPos < GetLength())
		data.insert(insertionPos, wide);
	else if (insertionPos == GetLength())
		data.append(wide);
	delete[] wide; 
	modified = true;

}

#pragma warning(disable : 4996)	// disable checked iterator warning for std::transform with char*
UInt32 StringVar::Find(char* subString, UInt32 startPos, UInt32 numChars, bool bCaseSensitive)
{
	UInt32 pos = -1;

	if (numChars + startPos >= GetLength())
		numChars = GetLength() - startPos;

	if (startPos < GetLength())
	{
		std::wstring source = data.substr(startPos, numChars);
		wchar_t* wide = ConvertToWideString(subString);
		if (!bCaseSensitive)
		{
			std::transform(source.begin(), source.end(), source.begin(), towlower);
			std::transform(wide, wide + wcslen(wide), wide, towlower);
		}

		 //pos = data.substr(startPos, numChars).find(subString);	//returns -1 if not found
		pos = source.find(wide);
		if (pos != -1)
			pos += startPos;
		delete[] wide;
	}

	return pos;
}

UInt32 StringVar::Count(char* subString, UInt32 startPos, UInt32 numChars, bool bCaseSensitive)
{
	if (numChars + startPos >= GetLength())
		numChars = GetLength() - startPos;

	if (startPos >= GetLength())
		return 0;

	std::wstring source = data.substr(startPos, numChars);	//only count occurences beginning before endPos

	UInt32 subStringLen = strlen(subString);
	if (!subStringLen)
		return 0;

	wchar_t* wide = ConvertToWideString(subString);
	subStringLen = wcslen(wide);
	if (!bCaseSensitive)
	{
		std::transform(source.begin(), source.end(), source.begin(), towlower);
		std::transform(wide, wide + subStringLen, wide, towlower);
	}

	UInt32 strIdx = 0;
	UInt32 count = 0;
	while (strIdx < GetLength() && ((strIdx = source.find(wide, strIdx)) != -1))
	{
		count++;
		strIdx += subStringLen;
	}
	delete[] wide;
	return count;
}
#pragma warning(default : 4996)

UInt32 StringVar::GetLength()
{
	return data.length();
}

UInt32 StringVar::Replace(char* toReplace, const char* replaceWith, UInt32 startPos, UInt32 numChars, bool bCaseSensitive, UInt32 numToReplace)
{
	// calc length of substring
	if (startPos >= GetLength())
		return 0;
	else if (numChars + startPos > GetLength())
		numChars = GetLength() - startPos;

	UInt32 numReplaced = 0;
	wchar_t* toReplaceWide = ConvertToWideString(toReplace);
	wchar_t* replaceWithWide = ConvertToWideString(replaceWith);

	UInt32 replacementLen = wcslen(replaceWithWide);
	UInt32 toReplaceLen = wcslen(toReplaceWide);

	// create substring
	std::wstring srcStr = data.substr(startPos, numChars);

	// remove substring from original string
	data.erase(startPos, numChars);

	UInt32 strIdx = 0;
	while (numReplaced < numToReplace)// && (strIdx = srcStr.find(toReplace, strIdx)) != -1)
	{
		if (bCaseSensitive)
		{
			strIdx = srcStr.find(toReplaceWide, strIdx);
			if (strIdx == -1)
				break;
		}
		else
		{
			std::wstring strToReplace = toReplaceWide;
			std::wstring::iterator iter = std::search(srcStr.begin() + strIdx, srcStr.end(), strToReplace.begin(), strToReplace.end(), ci_equal);
			if (iter != srcStr.end())
				strIdx = iter - srcStr.begin();
			else
				break;
		}

		numReplaced++;
		srcStr.erase(strIdx, toReplaceLen);
		if (strIdx == srcStr.length())
		{
			srcStr.append(replaceWithWide);
			break;						// reached end of string so all done
		}
		else
		{
			srcStr.insert(strIdx, replaceWithWide);
			strIdx += replacementLen;
		}
	}

	// paste altered string back into original string
	if (startPos == GetLength())
		data.append(srcStr);
	else
		data.insert(startPos, srcStr);

	delete[] replaceWithWide;
	delete[] toReplaceWide;
	modified = true;

	return numReplaced;

}

void StringVar::Erase(UInt32 startPos, UInt32 numChars)
{
	if (numChars + startPos >= GetLength())
		numChars = GetLength() - startPos;

	if (startPos < GetLength())
		data.erase(startPos, numChars);
	modified = true;

}

std::string StringVar::SubString(UInt32 startPos, UInt32 numChars)
{
	if (numChars + startPos >= GetLength())
		numChars = GetLength() - startPos;

	if (startPos < GetLength()) {
		std::wstring sub = data.substr(startPos, numChars);
		char* string = ConvertToMultibyteString(sub.data());
		return std::string(string);
	}
	else
		return "";
}

UInt8 StringVar::GetOwningModIndex()
{
	return owningModIndex;
}

UInt32 StringVar::GetCharType(char ch)
{
	UInt32 charType = 0;
	if (isalpha(ch))
		charType |= kCharType_Alphabetic;
	if (isdigit(ch))
		charType |= kCharType_Digit;
	if (ispunct(ch))
		charType |= kCharType_Punctuation;
	if (isprint(ch))
		charType |= kCharType_Printable;
	if (isupper(ch))
		charType |= kCharType_Uppercase;

	return charType;
}

char StringVar::At(UInt32 charPos)
{
	if (charPos < GetLength())
		return data[charPos];
	else
		return -1;
}

void StringVarMap::Save(OBSESerializationInterface* intfc)
{
	Clean();

	intfc->OpenRecord('STVS', 0);

	for (std::map<UInt32, StringVar*>::iterator iter = m_state->vars.begin();
			iter != m_state->vars.end();
			iter++)
	{
		if (IsTemporary(iter->first))	// don't save temp strings
			continue;

		intfc->OpenRecord('STVR', 0);
		UInt8 modIndex = iter->second->GetOwningModIndex();

		intfc->WriteRecordData(&modIndex, sizeof(UInt8));
		intfc->WriteRecordData(&iter->first, sizeof(UInt32));
		UInt16 len = iter->second->GetLength();
		intfc->WriteRecordData(&len, sizeof(len));
		intfc->WriteRecordData(iter->second->GetCString(), len);
	}

	intfc->OpenRecord('STVE', 0);
}

void StringVarMap::Load(OBSESerializationInterface* intfc)
{
	_MESSAGE("Loading strings");
	UInt32 type, length, version, stringID, tempRefID;
	UInt16 strLength;
	UInt8 modIndex;
	char buffer[kMaxMessageLength] = { 0 };

	Clean();

	// do some basic checking to weed out potential bloat caused by scripts creating large
	// numbers of string variables
	UInt32 modVarCounts[0x100] = {0};				// for each mod, # of string vars loaded
	static const UInt32 varCountThreshold = 100;	// what we'll consider a "large number" of vars; 
													// obviously a few mods may require more than this without it being a problem
	std::set<UInt8> exceededMods;

	bool bContinue = true;
	while (bContinue && intfc->GetNextRecordInfo(&type, &version, &length))
	{
		switch (type)
		{
		case 'STVE':			//end of block
			bContinue = false;

			if (exceededMods.size()) {
				_MESSAGE("  WARNING: substantial numbers of string variables exist for the following files (may indicate savegame bloat):");
				for (std::set<UInt8>::iterator iter = exceededMods.begin(); iter != exceededMods.end(); ++iter) {
					_MESSAGE("    %s (%d strings)", (*g_dataHandler)->GetNthModName(*iter), modVarCounts[*iter]);
				}
			}

			break;
		case 'STVR':
			intfc->ReadRecordData(&modIndex, sizeof(modIndex));
			if (!intfc->ResolveRefID(modIndex << 24, &tempRefID))
			{
				// owning mod is no longer loaded so discard
				continue;
			}
			else
				modIndex = tempRefID >> 24;

			intfc->ReadRecordData(&stringID, sizeof(stringID));
			intfc->ReadRecordData(&strLength, sizeof(strLength));
			
			intfc->ReadRecordData(buffer, strLength);
			buffer[strLength] = 0;

			Insert(stringID, new StringVar(buffer, tempRefID));
			modVarCounts[modIndex] += 1;
			if (modVarCounts[modIndex] == varCountThreshold) {
				exceededMods.insert(modIndex);
			}
					
			break;
		default:
			_MESSAGE("Error loading string map: unhandled chunk type %d", type);
			break;
		}
	}
}

UInt32	StringVarMap::Add(UInt8 varModIndex, const char* data, bool bTemp)
{
	UInt32 varID = GetUnusedID();
	Insert(varID, new StringVar(data, varModIndex << 24));
	if (bTemp)
		MarkTemporary(varID, true);

	return varID;
}

StringVarMap g_StringMap;

bool AssignToStringVar(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, TESObjectREFR* contObj, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr, const char* newValue)
{
	double strID = 0;
	UInt8 modIndex = 0;
	bool bTemp = ExpressionEvaluator::Active();
	StringVar* strVar = NULL;

	UInt32 len = (newValue) ? strlen(newValue) : 0;
	if (!newValue || len >= kMaxMessageLength)		//if null pointer or too long, assign an empty string
		newValue = "";

	if (ExtractSetStatementVar(scriptObj, eventList, arg1, &strID, &modIndex)) {
		strVar = g_StringMap.Get(strID);
		bTemp = false;
	}
	else if (!bTemp) {
		_WARNING("Function must be used within a Set statement or OBSE expression");
		return false;
	}

	if (!modIndex)
		modIndex = scriptObj->GetModIndex();

	if (strVar)
	{
		strVar->Set(newValue);
		g_StringMap.MarkTemporary(strID, false);
	}
	else
		strID = g_StringMap.Add(modIndex, newValue, bTemp);

	*result = strID;

#if _DEBUG	// console feedback disabled in release by request (annoying when called from batch scripts)
	if (IsConsoleMode() && !bTemp)
	{
		if (len < 480)
			Console_Print("Assigned string >> \"%s\"", newValue);
		else
			Console_Print("Assigned string (too long to print)");
	}
#endif

	return true;
}

void StringVarMap::Clean()		// clean up any temporary vars
{
	if (m_state) {
		while (m_state->tempVars.size())
		{
			UInt32 idToDelete = *(m_state->tempVars.begin());
			Delete(idToDelete);
		}
	}
}

namespace PluginAPI
{
	const char* GetString(UInt32 stringID)
	{
		StringVar* var = g_StringMap.Get(stringID);
		if (var)
			return var->GetCString();
		else
			return NULL;
	}

	void SetString(UInt32 stringID, const char* newVal)
	{
		StringVar* var = g_StringMap.Get(stringID);
		if (var)
			var->Set(newVal);
	}

	UInt32 CreateString(const char* strVal, void* owningScript)
	{
		Script* script = (Script*)owningScript;
		if (script)
			return g_StringMap.Add(script->GetModIndex(), strVal);
		else
			return 0;
	}
}
