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


InventoryReference* InventoryReference::CreateInventoryRef(TESObjectREFR* container, const InventoryReference::Data& data, bool bValidate)
{
	TESObjectREFR *refr = TESObjectREFR::Create(false);
	InventoryReference* invRefr = (InventoryReference*)FormHeap_Allocate(sizeof(InventoryReference));
	invRefr->m_containerRef = container;
	invRefr->m_tempRef = refr;
	invRefr->m_tempEntry = false;
	invRefr->m_bDoValidation = bValidate;
	invRefr->m_bRemoved = false;
	invRefr->SetData(data);
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
	if (m_data.type) Release();
	if (m_tempRef) m_tempRef->Destroy(true);
	if (m_tempEntry) {}
	if (m_containerRef) {
		ExtraContainerChanges* xChanges = (ExtraContainerChanges*)m_containerRef->baseExtraList.GetByType(kExtraData_ContainerChanges);
		if (xChanges) xChanges->Cleanup();
	}
}

void InventoryReference::Release(){
	DoDeferredActions();
	SetData(Data());
}

bool InventoryReference::SetData(const Data &data){
	m_bRemoved = false;
	m_tempRef->baseForm = data.type;
	m_data = data;
	WriteToExtraDataList(data.xData, &m_tempRef->baseExtraList);
	return true;
}

bool InventoryReference::WriteRefDataToContainer(){
	if (m_bRemoved) return true;
	if (!m_containerRef || !Validate()) return false;
	if (m_data.xData) WriteToExtraDataList(&m_tempRef->baseExtraList , m_data.xData);
	return true;
}

SInt32 InventoryReference::GetCount(){
    ExtraCount* xCount = (ExtraCount*)m_data.xData->GetByType(kExtraData_Count);
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
    if (m_bRemoved) return false;
	if (!m_bDoValidation) return true;

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
		delete iter->second;
	}
	s_refmap.clear();
}

bool InventoryReference::RemoveFromContainer(){
    return false;
}

bool InventoryReference::MoveToContainer(TESObjectREFR* dest){

	return false;
}

bool InventoryReference::CopyToContainer(TESObjectREFR* dest){
    return false;
}

bool InventoryReference::SetEquipped(bool bEquipped){
    return false;
}

