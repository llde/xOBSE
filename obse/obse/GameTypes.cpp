#include "obse/GameTypes.h"
#include "obse/GameAPI.h"
#include <string>
#include <algorithm>
#include "Utilities.h"

NiTPointerMap <TESForm>			* g_formTable = (NiTPointerMap <TESForm> *)0x00B0613C;
NiTPointerMap <EffectSetting>	* g_EffectSettingCollection = (NiTPointerMap <EffectSetting> *)0x00B33508;
NiTPointerList <TESForm>		* g_quickKeyList = (NiTPointerList <TESForm> *)0x00B3B440;


/*** BSStringT ***/
/*
          DEFINE_MEMBER_FN(Append,             void,       0x00412FA0, const char*);   //Verified  (thiscall)
         DEFINE_MEMBER_FN(CopyValueFromOther, BSStringT*, 0x004FB4C0, BSStringT*); // returns self    //Not verified
         DEFINE_MEMBER_FN(IsNotEqualTo,       bool,       0x00517B20, BSStringT*);  //Verified (thiscall)
         DEFINE_MEMBER_FN(Replace_MinBufLen,  void,       0x004028D0, const char*, UInt32 minBufferLength);  //This is BSStringT_Set (thiscall)
         DEFINE_MEMBER_FN(Replace,            void,       0x00488810, const char*);  //A function that call BSStringT_Set  
         DEFINE_MEMBER_FN(Sprintf,            void,       0x00402D00, const char* format, UInt32* varArgs);  //BSStringT_Format (thiscall)
 
 */
bool BSStringT::Set(const char * src)
{
	if (!src) {
		FormHeap_Free(m_data);
		m_data = 0;
		m_bufLen = 0;
		m_dataLen = 0;
		return true;
	}

	UInt32	srcLen = strlen(src);

	// realloc if needed
	if(srcLen > m_bufLen)
	{
		FormHeap_Free(m_data);
		m_data = (char *)FormHeap_Allocate(srcLen + 1);
		m_bufLen = m_data ? srcLen : 0;
	}

	if(m_data)
	{
		strcpy_s(m_data, m_bufLen + 1, src);
		m_dataLen = srcLen;
	}
	else
	{
		m_dataLen = 0;
	}

	return m_data != NULL;
}

bool BSStringT::Includes(const char *toFind) const
{
	if (!m_data || !toFind)		//passing null ptr to std::string c'tor = CRASH
		return false;
	std::string curr(m_data, m_dataLen);
	std::string str2(toFind);
	std::string::iterator currEnd = curr.end();
	return (std::search(curr.begin(), currEnd, str2.begin(), str2.end(), ci_equal) != currEnd);
}

bool BSStringT::Replace(const char *_toReplace, const char *_replaceWith)
{
	if (!m_data || !_toReplace)
		return false;

	std::string curr(m_data, m_dataLen);
	std::string toReplace(_toReplace);

	std::string::iterator currBegin = curr.begin();
	std::string::iterator currEnd = curr.end();
	std::string::iterator replaceIt = std::search(currBegin, currEnd, toReplace.begin(), toReplace.end(), ci_equal);
	if (replaceIt != currEnd) {
		std::string replaceWith(_replaceWith);
		// we found the substring, now we need to do the modification
		std::string::size_type replacePos = distance(currBegin, replaceIt);
		curr.replace(replacePos, toReplace.size(), replaceWith);
		Set(curr.c_str());
		return true;
	}
	return false;
}

bool BSStringT::Append(const char* toAppend)
{
	std::string curr("");
	if (m_data)
		curr = std::string(m_data, m_dataLen);

	curr += toAppend;
	Set(curr.c_str());
	return true;
}

double BSStringT::Compare(const BSStringT& compareTo, bool caseSensitive)
{
	if (!m_data)
		return -2;		//signal value if comparison could not be made

	std::string first(m_data, m_dataLen);
	std::string second(compareTo.m_data, compareTo.m_dataLen);

	if (!caseSensitive)
	{
		std::transform(first.begin(), first.end(), first.begin(), tolower);
		std::transform(second.begin(), second.end(), second.begin(), tolower);
	}

	double comp = 0;
	if (first < second)
		comp = -1;
	else if (first > second)
		comp = 1;

	return comp;
}

BSStringT::~BSStringT()
{
}
