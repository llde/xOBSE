#pragma once

#include "GameExtraData.h"
#include <map>
#include <vector>
#include <queue>

// InventoryReference represents a temporary reference to a stack of items in an inventory
// temp refs are valid only for the frame during which they were created

class InventoryReference{
public:
    struct Data{
        TESForm* type;
        ExtraContainerChanges::EntryData*	entry;
		ExtraDataList*						xData;
		SInt32								count; //count of the IR if xData NULL or without xCount, -1 otherwise
		UInt32                              temporary;  //0 non temp or null, 1 temp xData persistent EntryData, 2 both temp 
		Data(TESForm* t, ExtraContainerChanges::EntryData* en, ExtraDataList* ex) : type(t), entry(en), xData(ex), count(-1),temporary(0) { }
		Data(const Data& rhs) : type(rhs.type), entry(rhs.entry), xData(rhs.xData), count(rhs.count), temporary(rhs.temporary) { }
		Data() : type(NULL), entry(NULL), xData(NULL),count(-1),temporary(0) { }
		Data(TESForm* t, ExtraContainerChanges::EntryData* en, SInt32 count) : type(t), entry(en), xData(nullptr), count(count){ //if EntryData* NULL, it's an object in ther base container.
			temporary = (en->extendData == nullptr) ? 2 : 1;
			//TODO use flags
		}  
    };
    
    
    Data						m_data;
	TESObjectREFR*				m_containerRef;
	TESObjectREFR*				m_tempRef;
	bool						m_bRemoved;
    bool                        m_tempEntry;  //If the current ExtraContainerChanges::EntryData* in Data is temporary.
	bool						m_bDoValidation;
    
    ~InventoryReference();

	TESObjectREFR* GetContainer() { return m_containerRef; }
	void SetContainer(TESObjectREFR* cont) { m_containerRef = cont; }
	bool SetData(Data &data);
	TESObjectREFR* GetRef() { return Validate() ? m_tempRef : NULL; }
	static TESObjectREFR* GetRefBySelf(InventoryReference* self) { return self ? self->GetRef() : NULL; }	// Needed to get convert the address to a void*

	bool WriteRefDataToContainer();
	bool RemoveFromContainer();			// removes and frees Data pointers
	bool MoveToContainer(TESObjectREFR* dest);
	bool CopyToContainer(TESObjectREFR* dest);
	bool SetEquipped(bool bEquipped);
	void SetRemoved() { m_bRemoved = true; }
	void Release();
	bool Validate();
	void DoDeferredActions();
	SInt32 GetCount();
    
	static InventoryReference* GetForRefID(UInt32 refID);

	static InventoryReference* CreateInventoryRef(TESObjectREFR* container, InventoryReference::Data& data, bool bValidate);
	static ExtraContainerChanges::EntryData* CreateTempEntry(TESForm* itemForm, SInt32 countDelta, ExtraDataList* xData);
	static TESObjectREFR* CreateInventoryRefEntry(TESObjectREFR* container, TESForm* itemForm, SInt32 countDelta, ExtraDataList* xData);

    static void Clean();									// called from main loop to destroy any temp refs
	static bool HasData() { return !s_refmap.empty(); }	// provides a quick check from main loop to avoid unnecessary calls to Clean()
	static std::map<UInt32, InventoryReference*> InventoryReference::s_refmap;
};

