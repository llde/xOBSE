#include "GameBSExtraData.h"
#include "obse/GameAPI.h"

static LPCRITICAL_SECTION BSExtraDataSection = (LPCRITICAL_SECTION) 0x00B33800;

bool BaseExtraList::HasType(UInt32 type) const
{
	UInt32 index = (type >> 3);
	UInt8 bitMask = 1 << (type % 8);
	return (m_presenceBitfield[index] & bitMask) != 0;
}

BSExtraData* BaseExtraList::GetByType(UInt32 type) const{
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

void BaseExtraList::MarkType(UInt32 type, bool bCleared)
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

bool BaseExtraList::Remove(BSExtraData* toRemove)
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

bool BaseExtraList::RemoveByType(UInt32 type)
{
	bool res = false;
	EnterCriticalSection(BSExtraDataSection);
	if (HasType(type)) {
		res = Remove(GetByType(type));
	}
	LeaveCriticalSection(BSExtraDataSection);
	return res;
}

void BaseExtraList::RemoveAll()
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

bool BaseExtraList::Add(BSExtraData* toAdd)
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

void BaseExtraList::Copy(BaseExtraList* from)
{
#if OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	ThisStdCall(0x00428920, this, from);
#else
#error unsupported oblivion version
#endif
}

bool BaseExtraList::IsWorn()
{

	EnterCriticalSection(BSExtraDataSection);
	bool res =  (HasType(kExtraData_Worn) || HasType(kExtraData_WornLeft));
	LeaveCriticalSection(BSExtraDataSection);

	return res;
}

void BaseExtraList::DebugDump()
{
	_MESSAGE("BaseExtraList Dump:");
	gLog.Indent();

	EnterCriticalSection(BSExtraDataSection);

	if (m_data)
	{
		for(BSExtraData * traverse = m_data; traverse; traverse = traverse->next)
			_MESSAGE("%s", GetObjectClassName(traverse));
	}
	else
		_MESSAGE("No data in list");

	LeaveCriticalSection(BSExtraDataSection);
	gLog.Outdent();
}

bool BaseExtraList::MarkScriptEvent(UInt32 eventMask, TESForm* eventTarget)
{
	return MarkBaseExtraListScriptEvent(eventTarget, this, eventMask);
}