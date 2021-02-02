#include "InventoryReference.h"
#include "GameObjects.h"
#include "GameAPI.h"

void WriteToExtraDataList(BaseExtraList* from, BaseExtraList* to)
{
	ASSERT(to || from);

	if (from) {
		if (to) {
			to->m_data = from->m_data;
			memcpy(to->m_presenceBitfield, from->m_presenceBitfield, 0xC);
		}
	}
	else if (to) {
		to->m_data = NULL;
		memset(to->m_presenceBitfield, 0, 0xC);
	}
}

std::map<UInt32, InventoryReference*> InventoryReference::s_refmap;


InventoryReference* InventoryReference::CreateInventoryRef(TESObjectREFR* container, InventoryReference::Data& data, bool bValidate)
{
	TESObjectREFR *refr = TESObjectREFR::Create(false);
	InventoryReference* invRefr = (InventoryReference*)FormHeap_Allocate(sizeof(InventoryReference));
	invRefr->m_containerRef = container;
	invRefr->m_tempRef = refr;
	invRefr->m_tempEntry = false;
	invRefr->m_bDoValidation = bValidate;
	invRefr->m_bRemoved = false;
	invRefr->m_data = Data();
	invRefr->SetData(data);
	invRefr->actions = new std::queue<DeferredAction*>();
	InventoryReference::s_refmap[refr->refID] = invRefr;
	return invRefr;
}

ExtraContainerChanges::EntryData* InventoryReference::CreateTempEntry(TESForm* itemForm, SInt32 countDelta, ExtraDataList* xData)
{
	ExtraContainerChanges::EntryData* entry = (ExtraContainerChanges::EntryData*)FormHeap_Allocate(sizeof(ExtraContainerChanges::EntryData));
	if (xData){
		entry->extendData = (tList<ExtraDataList>*)FormHeap_Allocate(sizeof(tList<ExtraDataList>));
        entry->extendData->AddAt(xData,0);
	}
	else entry->extendData = NULL;
	entry->countDelta = countDelta;
	entry->type = itemForm;
	return entry;
}

TESObjectREFR* InventoryReference::CreateInventoryRefEntry(TESObjectREFR* container, TESForm* itemForm, SInt32 countDelta, ExtraDataList* xData)
{
	ExtraContainerChanges::EntryData* entry = CreateTempEntry(itemForm, countDelta, xData);
	InventoryReference* invRef = CreateInventoryRef(container, InventoryReference::Data(itemForm, entry, xData), false);
	invRef->m_tempEntry = true;
	return invRef->m_tempRef;
}

InventoryReference::~InventoryReference(){
	DEBUG_PRINT("Destroying IR");
	if (m_data.type) Release();
	DEBUG_PRINT("Destroying IR1");
	delete actions;
	if (m_tempRef) m_tempRef->Destroy(true);
	DEBUG_PRINT("Destroying IR2");

	if (m_containerRef) {
		DEBUG_PRINT("Destroying IR3");
		ExtraContainerChanges* xChanges = (ExtraContainerChanges*)m_containerRef->baseExtraList.GetByType(kExtraData_ContainerChanges);
		if (xChanges) xChanges->Cleanup();
	}
	DEBUG_PRINT("Destroying IR4");

}

void InventoryReference::Release(){
	DoDeferredActions();
	SetData(Data());
}

void InventoryReference::DoDeferredActions() { 
	while (!actions->empty()) {
		DeferredAction* action = actions->front();
		if (!action->Execute(this)) {
			_MESSAGE("[WARNING]: Deferred action failed");
		}
		actions->pop();
		delete action;
	}
}

bool InventoryReference::SetData(Data &data){
	DEBUG_PRINT("Set IR  %s", GetFullName(data.type));
	m_bRemoved = false;
	m_tempRef->baseForm = data.type;
	m_data = data;
	if (!m_data.xData  && m_data.type) {
		DEBUG_PRINT("Fixup REF stack  %d", m_data.count);
		m_tempRef->baseExtraList.RemoveAll();
		//	ZeroMemory(&m_tempRef->baseExtraList, sizeof(ExtraDataList));
		if (m_data.count > 1) {
			ExtraCount* xCount = ExtraCount::Create(m_data.count);
			m_tempRef->baseExtraList.Add(xCount);
		}
	}
	else if (!m_data.xData && m_data.type == nullptr) {
		DEBUG_PRINT("Free last node");
		m_tempRef->baseExtraList.RemoveAll();
	}
	else {
		m_tempRef->baseExtraList.RemoveAll();
		m_tempRef->baseExtraList.Copy(m_data.xData);
		m_tempRef->baseExtraList.DebugDump();
		DEBUG_PRINT("Wrote data to ref");
	}
	if (m_data.entry) DEBUG_PRINT("ED* %0X   %s", m_data.entry->extendData, GetFullName(m_data.type));

	return true;
}

bool InventoryReference::WriteRefDataToContainer(){  //IR operates directly on container, maybe non-IR aware commands can modifiy the XDataList  
	if (m_bRemoved) return true;
	if (!m_containerRef || !Validate()) return false;
//	if (m_data.xData) WriteToExtraDataList(&m_tempRef->baseExtraList , m_data.xData);
	return true;
}

SInt32 InventoryReference::GetCount(){
    ExtraCount* xCount = (ExtraCount*)m_tempRef->baseExtraList.GetByType(kExtraData_Count);
    SInt32  count = xCount ? xCount->count : 1;
	if (count < 0)
	{
		DEBUG_PRINT("Warning: InventoryReference::GetCount() found an object with a negative count (%d)", count);
	}

	return count;
}

bool InventoryReference::Validate()
{
	// it is possible that an inventory reference is created, and then commands like RemoveItem are used which modify the 
	// ExtraContainerChanges, potentially invalidating the InventoryReference::Data pointers
	// if m_bValidate is true, check for this occurrence before doing stuff with the temp ref
	// doing this in foreach loops is going to make them really slooooow.
	if (!m_bDoValidation) return true;
	if (m_bRemoved) return false;

	ExtraContainerChanges* xChanges = (ExtraContainerChanges*)m_containerRef->baseExtraList.GetByType(kExtraData_ContainerChanges);
	if (xChanges && xChanges->data) {
        for (tList<ExtraContainerChanges::EntryData>::Iterator cur = xChanges->data->objList->Begin(); !cur.End(); ++cur){
			if (*cur == m_data.entry && *cur && *cur == m_data.entry && cur->type == m_data.entry->type) {
                    for (tList<ExtraDataList>::Iterator ed = m_data.entry->extendData->Begin(); !ed.End(); ++ed) {
					if (*ed == m_data.xData) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

InventoryReference* InventoryReference::GetForRefID(UInt32 refID){
	std::map<UInt32, InventoryReference*>::iterator found = s_refmap.find(refID);
	if (found != s_refmap.end() && found->second->Validate()) {
		return found->second;
	}

	return NULL;
}

void InventoryReference::Clean(){
	for (auto iter = s_refmap.begin(); iter != s_refmap.end(); ++iter) {
		iter->second->~InventoryReference();
	//	iter->second->Release();
		FormHeap_Free(iter->second); //TODO override new and delete
	}
	s_refmap.clear();
}

bool InventoryReference::RemoveFromContainer(){
	if (m_containerRef && m_tempRef && Validate()) {
		if (m_data.xData->IsWorn()) {
			ExtraCount* count = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			actions->push(new DeferredAction(Action_Remove, m_data, nullptr , count ? count->count : 1));
			return true;
		}
		if (m_data.entry && m_data.entry->extendData && m_data.xData) {
			ExtraCount* count = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			m_data.entry->extendData->Remove(m_data.xData);  //TODO remember to free
			FormHeap_Free(m_data.xData);
			m_data.entry->countDelta -= count != NULL ? count->count : 1;
		}
		if (m_data.entry) {
			if (m_data.count > 0 && (m_data.entry->extendData == nullptr || m_data.entry->extendData->IsEmpty())) {
				//USe the countDelta
				m_data.entry->countDelta -= m_data.count;
			}
			if (m_data.entry->countDelta <= 0) {
				ExtraContainerChanges* xChanges = ExtraContainerChanges::GetForRef(m_containerRef);
				xChanges->data->objList->Remove(m_data.entry);
				FormHeap_Free(m_data.entry);
			}
		}
		else if(m_data.count > 0){   //If m_data.count is 0 or negative then there is nothing to remove
			//TODO free extradata
			actions->push(new DeferredAction(Action_Remove, m_data, nullptr, m_data.count));
		}
		SetRemoved();
		return true;
	}
    return false;
}
/*
	Assume m_data.entry m_data.entry->extendData and m_data.xData
	This means that the xData is is derived from the EntryData for the specifici form instead of being constructed after
*/
static void MoveToDestContainerXData(InventoryReference::Data& data, ExtraContainerChanges* from, ExtraContainerChanges* dest) {
	ExtraContainerChanges::EntryData* destEntry = dest->GetByType(data.type);

	ExtraCount* count = (ExtraCount*)data.xData->GetByType(kExtraData_Count);
	data.entry->extendData->Remove(data.xData);
	DEBUG_PRINT("%d      %d     %s", count != NULL ? count->count : 1, data.entry->countDelta, GetFullName(data.type));
	data.entry->countDelta -= count != NULL ? count->count : 1;

	if (destEntry == nullptr) {
		destEntry = ExtraContainerChanges::EntryData::Create(count != NULL ? count->count : 1, data.type);
		dest->data->objList->AddAt(destEntry, 0);
	}
	else {
		destEntry->countDelta += count != NULL ? count->count : 1;
	}
	if (destEntry->extendData == nullptr) {
		destEntry->extendData = (tList<ExtraDataList>*) tList<ExtraDataList>::Create();
	}
	destEntry->extendData->AddAt(data.xData, 0);
	if (data.entry->countDelta <= 0) {
		from->data->objList->Remove(data.entry);
	}

}

/*
	Assume m_data.entry but not a valid or valid but empty m_data.entry->extendData
	This means that the xData is constructed from the entryData countDelta.
*/

static void MoveToDestContainerEntry(InventoryReference::Data& data, ExtraContainerChanges* from, ExtraContainerChanges* dest) {
	ExtraContainerChanges::EntryData* destEntry = dest->GetByType(data.type);
	if (data.count <= 0) return;
	DEBUG_PRINT("Ma limorta2 %d", data.entry->countDelta);
	//USe the countDelta
	data.entry->countDelta -= data.count;
	
	if (data.entry->countDelta <= 0) {
		from->data->objList->Remove(data.entry);
	}
	if (destEntry == nullptr) {
		destEntry = ExtraContainerChanges::EntryData::Create(data.count, data.type);
		dest->data->objList->AddAt(destEntry, 0);
	}
	else {
		DEBUG_PRINT("Ma limorta5   %d", destEntry->countDelta);
		destEntry->countDelta += data.count;
		DEBUG_PRINT("Ma limorta5   %d", destEntry->countDelta);
		if (destEntry->extendData != nullptr && data.count > 1)
			destEntry->extendData->AddAt(data.xData, 0);  //TODO maybe unnecessary to add an explicit extraData*
	}
}

bool InventoryReference::MoveToContainer(TESObjectREFR* dest){
	if (dest == nullptr || !dest->GetContainer()) return false; //Check if dest reference is a valid container
	ExtraContainerChanges* destCont =  ExtraContainerChanges::GetForRef(dest);
	if (destCont == nullptr) return false;
	ExtraContainerChanges* xChanges = ExtraContainerChanges::GetForRef(m_containerRef);
	DEBUG_PRINT("Porcoddio %d  %0X   %0X   %0X", m_data.temporary, m_data.entry, m_data.xData , m_data.entry != NULL ? (SInt32)m_data.entry->extendData : -1);
	if (m_containerRef && m_tempRef && Validate()) {
		if (m_data.xData && m_data.xData->IsWorn()) {
			ExtraCount* count = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			actions->push(new DeferredAction(Action_Remove, m_data, dest, count ? count->count : 1));
		}
		else if (m_data.entry && m_data.entry->extendData && m_data.xData) {
			m_data.xData->DebugDump();
			MoveToDestContainerXData(m_data, xChanges, destCont);
			destCont->Cleanup();
		}
		else if (m_data.entry) {
			MoveToDestContainerEntry(m_data, xChanges, destCont);
			destCont->Cleanup();
		}
		else if (m_data.count > 0) {   //If m_data.count is 0 or negative then there is nothing to remove
			actions->push(new DeferredAction(Action_Remove, m_data, dest, m_data.count));
		}
		SetRemoved();
		return true;
	}
	return false;
}

bool InventoryReference::CopyToContainer(TESObjectREFR* dest){
	if (dest == nullptr || !dest->GetContainer()) return false; //Check if dest reference is a valid container
	ExtraContainerChanges* destCont = ExtraContainerChanges::GetForRef(dest);
	if (destCont == nullptr) return false;
	ExtraContainerChanges::EntryData* destEntry = destCont->GetByType(m_data.type);
	if (m_containerRef && m_tempRef && Validate()) {
		ExtraCount* xCount = nullptr;
		SInt32 count = 0;
		if (m_data.xData) {
			xCount = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			count = xCount != nullptr ? xCount->count : 1;
		}
		else {
			count = m_data.count;
		}
		if (destEntry == nullptr) {
			destEntry = ExtraContainerChanges::EntryData::Create(count , m_data.type);
		}
		else {
			destEntry->countDelta += count;
		}
		if (m_data.xData) {
			ExtraDataList* newData = ExtraDataList::Create();
			newData->Copy(m_data.xData);
			if (destEntry->extendData == nullptr) destEntry->extendData = (tList<ExtraDataList>*) tList<ExtraDataList>::Create();
			newData->RemoveByType(kExtraData_Worn);
			newData->RemoveByType(kExtraData_WornLeft);
			destEntry->extendData->AddAt(newData, 0);
		}
		else {
			if (destEntry->extendData != nullptr && !destEntry->extendData->IsEmpty()) {
				ExtraDataList* newData = ExtraDataList::Create();
				xCount = ExtraCount::Create(count);
				newData->Add(xCount);
				destEntry->extendData->AddAt(newData, 0);
			}
		}
		destCont->Cleanup();
		return true;
	}
    return false;
}

bool InventoryReference::SetEquipped(bool bEquipped){
	if (m_data.xData && m_data.xData->IsWorn() == bEquipped) return false;
	else if (bEquipped == false) return false;
	SInt32 count = 1;
	if (m_data.xData) {
		ExtraCount* co = (ExtraCount*) m_data.xData->GetByType(kExtraData_Count);
		count = co ? co->count : 1;
	}
	else {
		count = m_data.count;
	}
	actions->push(new DeferredAction(Action_Equip, m_data, nullptr, count));
    return true;
}

bool InventoryReference::DeferredAction::Execute(InventoryReference* iref) {
	TESObjectREFR* cont = iref->GetContainer();
	switch (type) {
		case Action_Equip: {
			if (!cont->IsActor())  return false;
			Actor* actor = (Actor*)cont;
			if (data.xData && data.xData->IsWorn()) {
				actor->UnequipItem(data.type, count, data.xData, 0, 0, 0);
			}
			else {
				//TODO check explicitly the stack count only for arrows?
				actor->EquipItem(data.type, count, data.xData, 0, 0);
			}
			return true;
		}
		case Action_Remove: {
			cont->RemoveItem(data.type, data.xData, count, 0, 0, dest, nullptr, nullptr, 1, 0);
			iref->SetRemoved();
			iref->SetData(InventoryReference::Data());
			return true;
		}
		default: {
			return false;
		}
	}
}
