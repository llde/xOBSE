#include "Loops.h"
#include "ThreadLocal.h"
#include "ArrayVar.h"
#include "StringVar.h"
#include "ScriptUtils.h"
#include "GameAPI.h"
#include "GameObjects.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	static const UInt32 kDataDeltaStackOffset = 480;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	static const UInt32 kDataDeltaStackOffset = 482;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	static const UInt32 kDataDeltaStackOffset = 482;
#else
#error unsupported oblivion version
#endif

LoopManager* LoopManager::GetSingleton()
{
	ThreadLocalData& localData = ThreadLocalData::Get();
	if (!localData.loopManager) {
		localData.loopManager = new LoopManager();
	}

	return localData.loopManager;
}

ArrayIterLoop::ArrayIterLoop(const ForEachContext* context, UInt8 modIndex)
{
	m_srcID = context->sourceID;
	m_iterID = context->iteratorID;
	m_iterVar = context->var;

	// clear the iterator var before initializing it
	g_ArrayMap.RemoveReference(&m_iterVar->data, modIndex);
	g_ArrayMap.AddReference(&m_iterVar->data, context->iteratorID, 0xFF);

	ArrayElement elem;
	ArrayKey key;

	if (g_ArrayMap.GetFirstElement(m_srcID, &elem, &key))
	{
		m_curKey = key;
		UpdateIterator(&elem);		// initialize iterator to first element in array
	}
}

void ArrayIterLoop::UpdateIterator(const ArrayElement* elem)
{
	std::string val("value");
	std::string key("key");

	// iter["value"] = element data
	switch (elem->DataType())
	{
	case kDataType_String:
		g_ArrayMap.SetElementString(m_iterID, val, elem->m_data.str);
		break;
	case kDataType_Numeric:
		g_ArrayMap.SetElementNumber(m_iterID, val, elem->m_data.num);
		break;
	case kDataType_Form:
		{
			g_ArrayMap.SetElementFormID(m_iterID, val, elem->m_data.formID);
			break;
		}
	case kDataType_Array:
		{
			ArrayID arrID = elem->m_data.num;
			g_ArrayMap.SetElementArray(m_iterID, val, arrID);
			break;
		}
	default:
		DEBUG_PRINT("ArrayIterLoop::UpdateIterator(): unknown datatype %d found for element value", elem->DataType());
	}

	// iter["key"] = element key
	switch (m_curKey.KeyType())
	{
	case kDataType_String:
		g_ArrayMap.SetElementString(m_iterID, key, m_curKey.Key().str);
		break;
	default:
		g_ArrayMap.SetElementNumber(m_iterID, key, m_curKey.Key().num);
	}
}

bool ArrayIterLoop::Update(COMMAND_ARGS)
{
	ArrayElement elem;
	ArrayKey key;

	if (g_ArrayMap.GetNextElement(m_srcID, &m_curKey, &elem, &key))
	{
		m_curKey = key;
		UpdateIterator(&elem);	
		return true;
	}

	return false;
}

ArrayIterLoop::~ArrayIterLoop()
{
	//g_ArrayMap.RemoveReference(&m_iterID, 0xFF);
	g_ArrayMap.RemoveReference(&m_iterVar->data, 0xFF);
}

StringIterLoop::StringIterLoop(const ForEachContext* context)
{
	StringVar* srcVar = g_StringMap.Get(context->sourceID);
	StringVar* iterVar = g_StringMap.Get(context->iteratorID);
	if (srcVar && iterVar)
	{
		m_src = srcVar->String();
		m_curIndex = 0;
		m_iterID = context->iteratorID;
		if (m_src.length())
			iterVar->Set(m_src.substr(0, 1).c_str());
	}
}

bool StringIterLoop::Update(COMMAND_ARGS)
{
	StringVar* iterVar = g_StringMap.Get(m_iterID);
	if (iterVar)
	{
		m_curIndex++;
		if (m_curIndex < m_src.length())
		{
			iterVar->Set(m_src.substr(m_curIndex, 1).c_str());
			return true;
		}
	}

	return false;
}

ContainerIterLoop::ContainerIterLoop(const ForEachContext* context)
{
	TESObjectREFR* contRef = (TESObjectREFR*)context->sourceID;
	m_refVar = context->var;
	m_iterIndex = 0;
	m_invRef = InventoryReference::CreateInventoryRef(contRef, InventoryReference::Data(), false );
    TESContainer* cont = contRef->GetContainer();
	std::map<TESForm*, SInt32> baseContainer;
	DEBUG_PRINT("Nuovo loop cazzoni");
	if (cont) {
		for (TESContainer::Entry* cur = &cont->list; cur; cur = cur->next) {
			if (cur->data && cur->data->type->typeID != kFormType_LeveledItem) {
				DEBUG_PRINT("Base container has %d %s", cur->data->count, GetFullName(cur->data->type));
				baseContainer[cur->data->type] = cur->data->count;
	//			m_elements.push_back(IRefData(cur->data->type, NULL, cur->data->count));
			}
		}
		ExtraContainerChanges* xChanges = (ExtraContainerChanges*)contRef->baseExtraList.GetByType(kExtraData_ContainerChanges);
        if(xChanges && xChanges->data){
            for (tList<ExtraContainerChanges::EntryData>::Iterator entry = xChanges->data->objList->Begin(); !entry.End(); ++entry){
                if(! *entry){
                    DEBUG_PRINT("Warning: encountered NULL ExtraContainerChanges::EntryData pointer in ContainerIterLoop constructor.");
                    continue;
                }
                TESForm* form = entry->type;
                SInt32 countExtraData = entry->countDelta;
                DEBUG_PRINT("ExtraContainer has %d %s", countExtraData, GetFullName(form));
                //TODO what's the difference between countDelta and iterating the extendextradatas to get the count?
				//if entry->countDelta <0 negate a base container item
				if (countExtraData < 0) {
					baseContainer[form] += countExtraData;
					continue;
				}
                if (entry->extendData) {
                    for (tList<ExtraDataList>::Iterator iter = entry->extendData->Begin(); !iter.End(); ++iter) {
                        /*Every EntryExtendData represent a separate stack?*/
						if (*iter) {
							ExtraCount* xCount = (ExtraCount*)iter->GetByType(kExtraData_Count);
							SInt32 count = xCount != NULL ? xCount->count : 1;
							DEBUG_PRINT("Got stack of %d  for %s", count, GetFullName(form));
							countExtraData -= count;
							m_elements.push_back(IRefData(form, entry.Get(), iter.Get()));
						}
                    }
                }
				if (countExtraData > 0) {//There are still leftovers items not associated with an ExtraDataList TODO does this case actually happen?
					DEBUG_PRINT("Got remaining  stack of %d  for %s", countExtraData, GetFullName(form));
					m_elements.push_back(IRefData(form, entry.Get(), countExtraData));
					//TODO Add these to the baseContainer objects if any
				}
                DEBUG_PRINT("ExtraContainer has %d %s after loop", countExtraData, GetFullName(form));
            }
        }
		for (auto& elem : baseContainer) {
			if (elem.second > 0) {
				m_elements.push_back(IRefData(elem.first, nullptr, elem.second));
			}
		}
		baseContainer.clear();
    }
	// initialize the iterator
	SetIterator();
}
/*
ContainerIterLoop::ContainerIterLoop(const ForEachContext* context)
{
	TESObjectREFR* contRef = (TESObjectREFR*)context->sourceID;
	m_refVar = context->var;
	m_iterIndex = 0;
	m_invRef = InventoryReference::Create(contRef, IRefData(NULL, NULL, NULL), false);	

	// first: figure out what items exist by default
	std::map<TESForm*, SInt32> baseObjectCounts;
//	TESContainer* cont = OBLIVION_CAST(contRef->baseForm, TESForm, TESContainer);
    TESContainer* cont = contRef->GetContainer();
	if (cont) {
		for (TESContainer::Entry* cur = &cont->list; cur; cur = cur->next) {
			if (cur->data && cur->data->type->typeID != kFormType_LeveledItem) {
				DEBUG_PRINT("Base container has %d %s", cur->data->count, GetFullName(cur->data->type));
				baseObjectCounts[cur->data->type] = cur->data->count;
			}
		}
	
		// now populate the vec
		ExtraContainerChanges* xChanges = (ExtraContainerChanges*)contRef->baseExtraList.GetByType(kExtraData_ContainerChanges);
		if (xChanges && xChanges->data) {
			for (ExtraContainerChanges::Entry* entry = xChanges->data->objList; entry; entry = entry->next) {
				if (entry->data) {
					TESForm* baseObj = entry->data->type;

					SInt32 countDelta = entry->data->countDelta;
					SInt32 actualCount = countDelta;
					bool isInBaseContainer = baseObjectCounts.find(baseObj) != baseObjectCounts.end();
					if (isInBaseContainer) {
						baseObjectCounts[baseObj] += countDelta;
						actualCount = baseObjectCounts[baseObj];
					}

					if (entry->data->extendData) {
						UInt32 total = 0;
						ExtraContainerChanges::EntryExtendData* prev = NULL;
						for (ExtraContainerChanges::EntryExtendData* extend = entry->data->extendData; extend; extend = extend->next) {
							if (total >= actualCount) {
								break;
							}

							total += GetCountForExtraDataList(extend->data);
							m_elements.push_back(IRefData(baseObj, entry, extend));
							prev = extend;
						}

						SInt32 remainder = isInBaseContainer ? baseObjectCounts[baseObj] : countDelta;
						remainder -= total;
						if (remainder > 0) {
							InventoryReference::Data::CreateForUnextendedEntry(entry, remainder, m_elements);
						}
					}
					else {
						SInt32 actualCount = countDelta;
						if (isInBaseContainer) {
							actualCount += baseObjectCounts[baseObj];
						}
						if (actualCount > 0) {
							InventoryReference::Data::CreateForUnextendedEntry(entry, actualCount, m_elements);
						}
					}

					if (isInBaseContainer) {
						baseObjectCounts.erase(baseObj);
					}
				}
				else {
					// wtf??
					DEBUG_PRINT("Warning: encountered NULL ExtraContainerChanges::Entry::Data pointer in ContainerIterLoop constructor.");
				}
			}
		}
		else if (baseObjectCounts.size()) {
			if (!xChanges) {
				xChanges = ExtraContainerChanges::Create();
			}

			xChanges->data = ExtraContainerChanges::Data::Create(contRef);
			xChanges->data->objList = (ExtraContainerChanges::Entry*)FormHeap_Allocate(sizeof(ExtraContainerChanges::Entry));

			std::map<TESForm*, SInt32>::iterator first = baseObjectCounts.begin();
			xChanges->data->objList->data = ExtraContainerChanges::EntryData::Create(first->second, first->first);
			xChanges->data->objList->next = NULL;
			baseObjectCounts.erase(first);
		}

		// now add entries for objects in base but without associated ExtraContainerChanges
		// these extra entries will be removed when we're done with the loop
		if (baseObjectCounts.size()) {
			for (std::map<TESForm*, SInt32>::iterator iter = baseObjectCounts.begin(); iter != baseObjectCounts.end(); ++iter) {
				if (iter->second > 0) {
					ExtraContainerChanges::Entry* entry = FORM_HEAP_ALLOCATE(ExtraContainerChanges::Entry);
					entry->next = xChanges->data->objList;
					xChanges->data->objList = entry;

					ExtraContainerChanges::EntryData* ed = ExtraContainerChanges::EntryData::Create(0, iter->first);
					ed->extendData = FORM_HEAP_ALLOCATE(ExtraContainerChanges::EntryExtendData);
					ed->extendData->next = NULL;
					ed->extendData->data = ExtraDataList::Create();

					if (iter->second > 1) {
						ExtraCount* xCount = ExtraCount::Create();
						xCount->count = iter->second;
						ed->extendData->data->Add(xCount);
					}

					entry->data = ed;

					m_elements.push_back(IRefData(iter->first, entry, ed->extendData));
				}
			}
		}
	}

	// initialize the iterator
	SetIterator();
}
*/
bool ContainerIterLoop::UnsetIterator()
{
	return m_invRef->WriteRefDataToContainer();

	/*
	// copy extra data back to container entry
	ExtraContainerChanges::EntryExtendData* data = m_elements[m_iterIndex].data;
	if (!data->data && m_tempRef->baseExtraList.m_data) {
		data->data = ExtraDataList::Create();
	}

	if (data->data) {
		data->data->m_data = m_tempRef->baseExtraList.m_data;
		memcpy(&data->data->m_presenceBitfield, &m_tempRef->baseExtraList.m_presenceBitfield, sizeof(data->data->m_presenceBitfield));
	}

	m_tempRef->baseExtraList.m_data = NULL;
	memset(&m_tempRef->baseExtraList.m_presenceBitfield, 0, 0xC);

	return true;
	*/
}

bool ContainerIterLoop::SetIterator()
{
	TESObjectREFR* refr = m_invRef->GetRef();
	if (m_iterIndex < m_elements.size() && refr) {
		m_invRef->SetData(m_elements[m_iterIndex]);
		*((UInt64*)&m_refVar->data) = refr->refID;
		return true;
	}
	else {
		// loop ends, ref will shortly be invalid so zero out the var
		DEBUG_PRINT("It's the end of the war, HOLD THE CORRIDOR");
		m_refVar->data = 0;
		m_invRef->SetData(IRefData(nullptr, nullptr, nullptr));
		return false;
	}
}

bool ContainerIterLoop::Update(COMMAND_ARGS)
{
	UnsetIterator();
	m_iterIndex++;
	return SetIterator();
}

ContainerIterLoop::~ContainerIterLoop()
{
	DEBUG_PRINT("Destorying loop");
	m_elements.clear();
//	delete m_invRef;
	m_refVar->data = 0;
}

void LoopManager::Add(Loop* loop, ScriptExecutionState* state, UInt32 startOffset, UInt32 endOffset, COMMAND_ARGS)
{
	// save the instruction offsets
	LoopInfo loopInfo;
	loopInfo.loop = loop;
	loopInfo.endIP = endOffset;

	// save the stack
	SavedIPInfo* savedInfo = &loopInfo.ipInfo;
	savedInfo->ip = startOffset;
	savedInfo->stackDepth = state->stackDepth;
	memcpy(savedInfo->stack, state->stack, (savedInfo->stackDepth + 1) * sizeof(UInt32));

	m_loops.push(loopInfo);
}

void LoopManager::RestoreStack(ScriptExecutionState* state, SavedIPInfo* info)
{
	state->stackDepth = info->stackDepth;
	memcpy(state->stack, info->stack, (info->stackDepth + 1) * sizeof(UInt32));
}

bool LoopManager::Break(ScriptExecutionState* state, COMMAND_ARGS)
{
	if (!m_loops.size())
		return false;

	LoopInfo* loopInfo = &m_loops.top();

	RestoreStack(state, &loopInfo->ipInfo);
	opcodeOffsetPtr[kDataDeltaStackOffset] += loopInfo->endIP - (*opcodeOffsetPtr);

	delete loopInfo->loop;

	m_loops.pop();

	return true;
}

bool LoopManager::Continue(ScriptExecutionState* state, COMMAND_ARGS)
{
	if (!m_loops.size())
		return false;

	LoopInfo* loopInfo = &m_loops.top();

	if (!loopInfo->loop->Update(PASS_COMMAND_ARGS))
	{
		Break(state, PASS_COMMAND_ARGS);
		return true;
	}

	RestoreStack(state, &loopInfo->ipInfo);
	opcodeOffsetPtr[kDataDeltaStackOffset] += loopInfo->ipInfo.ip - (*opcodeOffsetPtr);

	return true;
}


bool WhileLoop::Update(COMMAND_ARGS)
{
	// save *opcodeOffsetPtr so we can calc IP to branch to after evaluating loop condition
	UInt32 originalOffset = *opcodeOffsetPtr;

	// update offset to point to loop condition, evaluate
	*opcodeOffsetPtr = m_exprOffset;
	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	bool bResult = eval.ExtractArgs();

	*opcodeOffsetPtr = originalOffset;

	if (bResult && eval.Arg(0))
			bResult = eval.Arg(0)->GetBool();

	return bResult;
}
