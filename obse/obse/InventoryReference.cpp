#include "InventoryReference.h"
#include "GameObjects.h"
#include "GameAPI.h"
#include "Settings.h"

std::map<UInt32, InventoryReference*> InventoryReference::s_refmap;
bool InventoryReference::releaseNonDeferred = true;

InventoryReference* InventoryReference::CreateInventoryRef(TESObjectREFR* container, InventoryReference::Data data, bool bValidate)
{
	TESObjectREFR *refr = TESObjectREFR::Create(false);
	InventoryReference* invRefr = (InventoryReference*)FormHeap_Allocate(sizeof(InventoryReference));
	ZeroMemory(invRefr, sizeof(InventoryReference));
	invRefr->m_containerRef = container;
	invRefr->m_tempRef = refr;
	invRefr->m_bDoValidation = bValidate;
	invRefr->m_bRemoved = false;
	invRefr->m_backupList = ExtraDataList::Create();
	invRefr->m_data = Data();
	invRefr->SetData(data);
	invRefr->actions = new std::queue<DeferredAction*>();
	InventoryReference::s_refmap[refr->refID] = invRefr;
	InventoryReference::releaseNonDeferred = true; //Allow fixup for newer IR
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
	return invRef->m_tempRef;
}

InventoryReference::~InventoryReference(){
//	DEBUG_PRINT("Destroying IR  %08X  %u   %s",  this, m_bRemoved, GetFullName(m_data.type));
	if (m_tempRef) Release();
//	DEBUG_PRINT("Destroying IR1");
	delete actions;
	if (m_tempRef){
		m_tempRef->baseExtraList.RemoveAll(); //Possible distruction of xScript in Destroy, causing CTD
		m_tempRef->Destroy(true);
	}
//	DEBUG_PRINT("Destroying IR2");

	if (m_containerRef) {
//		DEBUG_PRINT("Destroying IR3");
		ExtraContainerChanges* xChanges = (ExtraContainerChanges*)m_containerRef->baseExtraList.GetByType(kExtraData_ContainerChanges);
		if (xChanges) xChanges->Cleanup();
	}
	if (m_backupList) {
		m_backupList->RemoveAll();
		FormHeap_Free(m_backupList);
		m_backupList = nullptr;
	}
//	DEBUG_PRINT("Destroying IR4");

}

void InventoryReference::Release(){
	DoDeferredActions();
	if(m_bRemoved) return;
	if (IR_WriteAllRef) WriteRefDataToContainer();
	m_bRemoved = true;
}

void InventoryReference::DoDeferredActions() { 
	if(InventoryReference::releaseNonDeferred && !actions->empty()) ReleaseNonDeferred(m_containerRef);
	while (!actions->empty()) {
		DeferredAction* action = actions->front();
		if (!action->Execute(this)) {
			_MESSAGE("[WARNING]: Deferred action failed");
		}
		actions->pop();
		delete action;
	}
}

bool InventoryReference::SetData(Data data){
	if (IR_WriteAllRef) WriteRefDataToContainer();
	DEBUG_PRINT("Set IR %08X %s", this, GetFullName(data.type));
	m_bRemoved = false;
	m_tempRef->baseForm = data.type;
	m_data = data;
	m_backupList->RemoveAll();
	if (!m_data.xData  && m_data.type) {
		DEBUG_PRINT("Fixup REF stack  %d", m_data.count);
		m_tempRef->baseExtraList.RemoveAll();
		//	ZeroMemory(&m_tempRef->baseExtraList, sizeof(ExtraDataList));
		if (m_data.count > 1) {
			DEBUG_PRINT("Create ExtraCount for > 1 Inventory stack");
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
		m_backupList->Copy(m_data.xData); /* TODO if this mechanism is shown necessary even with per script ir, then we may need to expand it to other casistics*/
//		m_tempRef->baseExtraList.DebugDump();
		DEBUG_PRINT("Wrote data to original ref");
	}
	if (m_data.entry) DEBUG_PRINT("ED* %0X   %s", m_data.entry->extendData, GetFullName(m_data.type));

	return true;
}


bool InventoryReference::WriteRefDataToContainer(){  //IR operates directly on container, maybe non-IR aware commands can modifiy the XDataList  
	if (m_bRemoved) return true;
	if (!m_containerRef || !m_data.type || !Validate()) return false;
	if (m_data.xData) {
		if (m_backupList->Compare(&m_tempRef->baseExtraList)) {
			DEBUG_PRINT("ExtraData changed. Mirror changes %s", GetFullName(m_data.type));
			m_data.xData->RemoveAll();
			//this->Copy(other) cause items to lose the IsWorn xData  from other, unless the this is an empty xDataList. WHY?
			//Maybe it's the effect of ExtraDataList__RemoveAllCopyableExtraData at 0x0041E3D0?
			m_data.xData->Copy(&m_tempRef->baseExtraList); 
			//TODO we may still need to signal and update modification to other IR involving the same object. However considering the plan is to make IR  resolution per script may actually not be needed.
			//Check if a mod actually require it.
			// A possible optimization may be to share all reference vars involving a stack  to a single IR. Investigate.
			//		UpdateRefDataFromContainer();
		}
		else {
			DEBUG_PRINT("ExtraDataList is equal, avoid  copying  %s", GetFullName(m_data.type));
		}
	}
	else if(m_data.entry){ /*PAth if the original item didn't have an ExtraDataList but only an Entry*/
		DEBUG_PRINT("Write ExtraData to Entry   %08X %08X %u  %s", m_data.entry, m_tempRef->baseExtraList.m_data, m_bRemoved, GetFullName(m_data.type));
		// THis xCount is generated by the IR routines, but this represent the extra of the stack
		// Apparently an xCount only stack isn't properly handled in some game code? 
		// Or is this an error somewhere here?
		BSExtraData* xCount =  m_tempRef->baseExtraList.RemoveByType(kExtraData_Count);
		if(!m_tempRef->baseExtraList.IsEmpty()) {
			m_tempRef->baseExtraList.Add(xCount);
			ExtraDataList* extra = ExtraDataList::Create();
			extra->Copy(&m_tempRef->baseExtraList);
			m_data.entry->Add(extra);
		}
		else{
			FormHeap_Free(xCount);
		}
	}
	else if(m_data.entry){
		DEBUG_PRINT("Original reference don't have ExtraData and there are no ExtraData in the temp Ref. Don't write anything");
	}
	else{
		DEBUG_PRINT("No Xdata nor EntryData present. MAybe an item from a base container? Changes made to IR tempRef won't mirror to the real Reference");
	}
	return true;
}

SInt32 InventoryReference::GetCount(){
    ExtraCount* xCount = (ExtraCount*)m_tempRef->baseExtraList.GetByType(kExtraData_Count);
    SInt32  count = xCount ? xCount->count : 1;
	if (count < 0)
	{
		_MESSAGE("Warning: InventoryReference::GetCount() found an object with a negative count (%d)", count);
	}

	return count;
}

bool InventoryReference::Validate()
{
	//TODO remake Validate
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

void InventoryReference::InvalidateByItemAndContainer(TESObjectREFR* cont, TESForm* item){
	for (auto iter = s_refmap.begin(); iter != s_refmap.end(); ++iter) {
		InventoryReference* ir = iter->second;
		if(ir->m_containerRef == cont && ir->m_data.type == item){
			ir->SetRemoved();
		}
	}
}

void InventoryReference::Clean(){
	for (auto iter = s_refmap.begin(); iter != s_refmap.end();) {//
		InventoryReference* ir = iter->second;
		iter = s_refmap.erase(iter);
		ir->~InventoryReference();
		FormHeap_Free(ir); //TODO override new and delete
	}
	InventoryReference::releaseNonDeferred = true;
//	s_refmap.clear();
}

/*
 * Release IR that doens't have deferred actions. 
 * To be called before executing a deferred action to avoid these IR to be invalidated
 * TODO try to fixup instead? 
 * Most of this wouldn't be needed if we can use per script IR Releases
 */
void InventoryReference::ReleaseNonDeferred(TESObjectREFR* cont){
	for (auto iter = s_refmap.begin(); iter != s_refmap.end(); iter++) {
		InventoryReference* ir = iter->second;
		if(ir->actions->empty() && ir->m_containerRef == cont){
			ir->Release();
		}
	}
	InventoryReference::releaseNonDeferred = false;
}

/*
	Assume m_data.entry m_data.entry->extendData and m_data.xData
	This means that the xData is is derived from the EntryData for the specifici form instead of being constructed after
*/
static void RemoveFromContainerXData(InventoryReference::Data& data, ExtraContainerChanges* from) {}

/*
	Assume m_data.entry but not a valid or valid but empty m_data.entry->extendData
	This means that the xData is constructed from the entryData countDelta.
*/

static void RemoveFromContainerEntry(InventoryReference::Data& data, ExtraContainerChanges* from) {}

bool InventoryReference::RemoveFromContainer(){
	if (m_containerRef && m_tempRef && Validate()) {
		DEBUG_PRINT("Porcoddio %0X %0X   %0X   %s",  this, m_data.entry, m_data.xData , GetFullName(m_data.type));
		if (m_data.xData &&  m_data.xData->IsWorn()) {
			ExtraCount* count = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			actions->push(new DeferredAction(Action_Remove, m_data, nullptr , count ? count->count : 1));
			return true;
		}
		if (m_data.entry && m_data.entry->extendData && m_data.xData) {
			ExtraCount* count = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			m_data.entry->extendData->Remove(m_data.xData); 
			FormHeap_Free(m_data.xData);
			m_data.entry->countDelta -= count != NULL ? count->count : 1;
		}
		if (m_data.entry) {
			if (m_data.count > 0 && !m_data.xData) {
				//USe the countDelta
				m_data.entry->countDelta -= m_data.count;
			}
			if (m_data.entry->countDelta <= 0 && !m_data.fromBaseContainer) {
				ExtraContainerChanges* xChanges = ExtraContainerChanges::GetForRef(m_containerRef);
				xChanges->data->objList->Remove(m_data.entry);
				FormHeap_Free(m_data.entry);
			}
		}
		else if(m_data.count > 0){   //If m_data.count is 0 or negative then there is nothing to remove
			actions->push(new DeferredAction(Action_Remove, m_data, nullptr, m_data.count));
			return true;
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
	//TODO from and dest can be the same?
	ExtraCount* count = (ExtraCount*)data.xData->GetByType(kExtraData_Count);
	data.entry->extendData->Remove(data.xData);
	DEBUG_PRINT("MoveToDestContainerXData %d      %d   %d  %s", count != NULL ? count->count : 1, data.entry->countDelta, data.count, GetFullName(data.type));
	data.entry->countDelta -= count != NULL ? count->count : 1;
	//TODO can data.entry->countDelta be different then ExtraCount?
	if (destEntry == nullptr) {
		destEntry = ExtraContainerChanges::EntryData::Create(count != NULL ? count->count : 1, data.type);
		dest->data->objList->AddAt(destEntry, 0);
		DEBUG_PRINT("Create EntryData and append");
	}
	else {
		destEntry->countDelta += count != NULL ? count->count : 1;
		DEBUG_PRINT("Increase destination countDelta");
	}
	if (destEntry->extendData == nullptr) {
		destEntry->extendData = (tList<ExtraDataList>*) tList<ExtraDataList>::Create();
		DEBUG_PRINT("Create ExtraDataList list");
	}
	destEntry->extendData->AddAt(data.xData, 0);
	if (data.entry->countDelta <= 0 && !data.fromBaseContainer ) {
		from->data->objList->Remove(data.entry);
		DEBUG_PRINT("REmove entry from original container");
	}

	dest->Cleanup();
}

/*
	Assume m_data.entry but not a valid or valid but empty m_data.entry->extendData
	This means that the xData is constructed from the entryData countDelta.
*/

static void MoveToDestContainerEntry(InventoryReference::Data& data, ExtraContainerChanges* from, ExtraContainerChanges* dest) {
	if (data.count <= 0 || from == dest) return;
	DEBUG_PRINT("MoveToDestContainerEntry  %d %d  %08X   %08X", data.entry->countDelta, data.count , from, dest);
	//USe the countDelta
	//TODO can we avoid reallocation?
	data.entry->countDelta -= data.count;
	if (data.entry->countDelta <= 0  && !data.fromBaseContainer) {
		from->data->objList->Remove(data.entry);
		FormHeap_Free(data.entry);
	}
	ExtraContainerChanges::EntryData* destEntry = dest->GetByType(data.type);
	if (destEntry == nullptr) {
		destEntry = ExtraContainerChanges::EntryData::Create(data.count, data.type);
		dest->data->objList->AddAt(destEntry, 0);
	}
	else {
		destEntry->countDelta += data.count;
		if (destEntry->extendData != nullptr && data.xData && data.count > 1) {
			destEntry->extendData->AddAt(data.xData, 0);  //TODO maybe unnecessary to add an explicit extraData*
			_MESSAGE("Ma ce s'arriva mai qua?");
		}
	}
	dest->Cleanup();
//	dest->DebugDump();
}

bool InventoryReference::MoveToContainer(TESObjectREFR* dest){
	if (dest == nullptr || !dest->GetContainer()) return false; //Check if dest reference is a valid container
//	if (dest == m_containerRef) return true;
	ExtraContainerChanges* destCont =  ExtraContainerChanges::GetForRef(dest);
	if (destCont == nullptr) return false;
	ExtraContainerChanges* xChanges = ExtraContainerChanges::GetForRef(m_containerRef);
	DEBUG_PRINT("Porcoddio %0X   %0X   %s", m_data.entry, m_data.xData , GetFullName(m_data.type));
	if (m_containerRef && m_tempRef && Validate()) {
		if (m_data.xData && m_data.xData->IsWorn()) {
			ExtraCount* count = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			actions->push(new DeferredAction(Action_Remove, m_data, dest, count ? count->count : 1));
			return true;
		}
		else if (m_data.entry && m_data.entry->extendData && m_data.xData) {
			m_data.xData->RemoveAll();
			m_data.xData->Copy(&m_tempRef->baseExtraList);  //Copy the changes from the temp ref. 
			MoveToDestContainerXData(m_data, xChanges, destCont);
		}
		else if (m_data.entry) {
			MoveToDestContainerEntry(m_data, xChanges, destCont);
		}
		else if (m_data.count > 0) {   //If m_data.count is 0 or negative then there is nothing to remove
			actions->push(new DeferredAction(Action_Remove, m_data, dest, m_data.count));
			return true;
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
	//_MESSAGE("%0X  %0X  %0X", m_containerRef,m_tempRef, m_data.xData);
	bool valid = m_containerRef != nullptr ? Validate() : true;
	if (m_tempRef && valid) {
		ExtraCount* xCount = nullptr;
		SInt32 count = 0;
		if (m_data.xData) {
			xCount = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
			count = xCount != nullptr ? xCount->count : 1;
		}
		else {
			count = m_data.count;
		}
//		_MESSAGE("%d", count);
		if (destEntry == nullptr) {
			destEntry = ExtraContainerChanges::EntryData::Create(count , m_data.type);
			destCont->data->objList->AddAt(destEntry,0);
		}
		else {
			destEntry->countDelta += count;
		
		}
		ExtraDataList* newData = ExtraDataList::Create();
		newData->Copy(&m_tempRef->baseExtraList);
		if (destEntry->extendData == nullptr) destEntry->extendData = (tList<ExtraDataList>*) tList<ExtraDataList>::Create();
		BSExtraData* xWorn = newData->RemoveByType(kExtraData_Worn);
		BSExtraData* xWornLeft =newData->RemoveByType(kExtraData_WornLeft);
		FormHeap_Free(xWorn);
		FormHeap_Free(xWornLeft);
		destEntry->extendData->AddAt(newData, 0);
		if(count > 1 && !xCount){
			ExtraDataList* newData = ExtraDataList::Create();
			xCount = ExtraCount::Create(count);
			newData->Add(xCount);
			destEntry->extendData->AddAt(newData, 0);			
		}
		
		destCont->Cleanup();
		return true;
	}
    return false;
}

bool InventoryReference::SetEquipped(bool bEquipped){
	DEBUG_PRINT("%08X %s  %0X   %0X  %0X  %s", this, GetFullName(m_data.type), m_data.xData , m_data.xData ? m_data.xData->IsWorn() : false , bEquipped, GetFullName(m_containerRef));
	if (m_data.xData == nullptr && bEquipped == false) return false; //No extra data items can't be already equipped. So bail off if bEquipped is false
	if (m_data.xData && m_data.xData->IsWorn() == bEquipped) return false; //  If both IsWorn and bEquipped are equals the state is already what we want. Bail out.
	SInt32 count = 1;
	if (m_data.xData) {
		m_data.xData->RemoveAll();
		m_data.xData->Copy(&m_tempRef->baseExtraList);  //Copy the changes from the temp ref. Equip action is deferred and the IR is invalidated after, so do the copy here
		ExtraCount* co = (ExtraCount*) m_data.xData->GetByType(kExtraData_Count);
		count = co ? co->count : 1;
	}
	else {
		//TODO no extradata in origin, but ir modified with extradata, handle
		count = m_data.count;
	}
	actions->push(new DeferredAction(Action_Equip, m_data, nullptr, count));
    return true;
}

bool InventoryReference::DeferredAction::Execute(InventoryReference* iref) {
	TESObjectREFR* cont = iref->GetContainer();
	DEBUG_PRINT("Deferred action for %08X %s", iref, GetFullName(data.type));
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
			iref->SetRemoved();
			InvalidateByItemAndContainer(cont,data.type); //HACK: invalidate other IR with the same form and container, it may be invalid after this point. This should become unnecessary after reworking IR to be per Script.	
			return true;
		}
		case Action_Remove: {
			cont->RemoveItem(data.type, data.xData, count, 0, 0, dest, nullptr, nullptr, 1, 0);
			iref->SetRemoved();
			InvalidateByItemAndContainer(cont,data.type); //HACK: invalidate other IR with the same form and container, it may be invalid after this point. This should become unnecessary after reworking IR to be per Script.	
//			iref->SetData(InventoryReference::Data());
			return true;
		}
		default: {
			return false;
		}
	}
}
