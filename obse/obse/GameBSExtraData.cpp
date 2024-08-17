#include "GameBSExtraData.h"
#include "obse/GameAPI.h"
#include <obse/GameExtraData.h>

static LPCRITICAL_SECTION BSExtraDataSection = (LPCRITICAL_SECTION) 0x00B33800;

bool ExtraDataList::HasType(UInt32 type) const
{
	UInt32 index = (type >> 3);
	UInt8 bitMask = 1 << (type % 8);
	return (m_presenceBitfield[index] & bitMask) != 0;
}

BSExtraData* ExtraDataList::GetByType(UInt32 type) const{
	EnterCriticalSection(BSExtraDataSection);
	bool hasType = HasType(type);
	LeaveCriticalSection(BSExtraDataSection);
	if (!hasType) return NULL;

	EnterCriticalSection(BSExtraDataSection);
	for (BSExtraData* traverse = m_data; traverse; traverse = traverse->next) {
		if (traverse->type == type) {
			LeaveCriticalSection(BSExtraDataSection);
			return traverse;
		}
	}
	LeaveCriticalSection(BSExtraDataSection);
#ifdef _DEBUG
	Console_Print("ExtraData HasType(%d) is true but it wasn't found!", type);
#endif
	return NULL;
}

void ExtraDataList::MarkType(UInt32 type, bool bCleared)
{
	UInt32 index = (type >> 3);
	UInt8 bitMask = 1 << (type % 8);
	UInt8& flag = m_presenceBitfield[index];
	if (bCleared) {
		flag &= ~bitMask;
	} else {
		flag |= bitMask;
	}
}

bool ExtraDataList::Remove(BSExtraData* toRemove)
{
	if (!toRemove) return false;
	EnterCriticalSection(BSExtraDataSection);
	if (HasType(toRemove->type)) {
		bool bRemoved = false;
		if (m_data == toRemove) {
			m_data = m_data->next;
			bRemoved = true;
		}

		for (BSExtraData* traverse = m_data; traverse; traverse = traverse->next) {
			if (traverse->next == toRemove) {
				traverse->next = toRemove->next;
				bRemoved = true;
				break;
			}
		}
		if (bRemoved) {
			MarkType(toRemove->type, true);
		}
		LeaveCriticalSection(BSExtraDataSection);
		return true;
	}
	LeaveCriticalSection(BSExtraDataSection);
	return false;
}

BSExtraData* ExtraDataList::RemoveByType(UInt32 type)
{
	BSExtraData* res = nullptr;
	EnterCriticalSection(BSExtraDataSection);
	if (HasType(type)) {
		res = GetByType(type);
		Remove(res);
	}
	LeaveCriticalSection(BSExtraDataSection);
	return res;
}

void ExtraDataList::RemoveAll()
{
	EnterCriticalSection(BSExtraDataSection);
	while (m_data) {
		BSExtraData* data = m_data;
		m_data = data->next;
		MarkType(data->type, true);
		FormHeap_Free(data);
	}
	LeaveCriticalSection(BSExtraDataSection);
}

bool ExtraDataList::Add(BSExtraData* toAdd)
{
	if (!toAdd || HasType(toAdd->type)) return false;

	EnterCriticalSection(BSExtraDataSection);
	BSExtraData* next = m_data;
	m_data = toAdd;
	toAdd->next = next;
	MarkType(toAdd->type, false);
	LeaveCriticalSection(BSExtraDataSection);
	return true;
}

bool ExtraDataList::Compare(ExtraDataList* oth) {
	return ThisStdCall(0x0041E550, this, oth);
}

void ExtraDataList::Copy(ExtraDataList* from)
{
	ThisStdCall(0x00428920, this, from);

}

bool ExtraDataList::IsWorn()
{

	EnterCriticalSection(BSExtraDataSection);
	bool res =  (HasType(kExtraData_Worn) || HasType(kExtraData_WornLeft));
	LeaveCriticalSection(BSExtraDataSection);

	return res;
}

bool ExtraDataList::IsEmpty(){
	return m_data == nullptr; //TODO lock CS? Should be unnecessary
}

void ExtraDataList::DebugDump()
{
	_MESSAGE("ExtraDataList Dump:");
	gLog.Indent();

	EnterCriticalSection(BSExtraDataSection);

	if (m_data)
	{
		for (BSExtraData* traverse = m_data; traverse; traverse = traverse->next) {
			_MESSAGE("%s", GetObjectClassName(traverse));
		}
	}
	else
		_MESSAGE("No data in list");

	LeaveCriticalSection(BSExtraDataSection);
	gLog.Outdent();
}

bool ExtraDataList::MarkScriptEvent(UInt32 eventMask, TESForm* eventTarget)
{
	return MarkBaseExtraListScriptEvent(eventTarget, this, eventMask);
}