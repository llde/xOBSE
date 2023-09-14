#include "ScriptUtils.h"
#include "ArrayVar.h"
#include "GameForms.h"
#include <algorithm>
#include <format>

#if OBLIVION
#include "GameAPI.h"
#include "GameData.h"
#include "FunctionScripts.h"

#endif

ArrayVarMap g_ArrayMap;

//////////////////
// ArrayElement
/////////////////

bool ArrayElement::operator<(const ArrayElement& rhs) const
{
	// if we ever try to compare 2 elems of differing types (i.e. string and number) we violate strict weak
	// no reason to do that

	if (DataType() != rhs.DataType())
	{
		_MESSAGE("ArrayElement::operator< called with non-matching element types.");
		return false;
	}

	if (DataType() == kDataType_String)
		return (_stricmp(Data.str.c_str(), rhs.Data.str.c_str()) < 0);
	else if (DataType() == kDataType_Form)
		return Data.formID < rhs.Data.formID;
	else
		return Data.num < rhs.Data.num;
}

bool ArrayElement::Equals(const ArrayElement& compareTo) const
{
	if (DataType() != compareTo.DataType())
		return false;

	switch (DataType())
	{
	case kDataType_String:
		return (Data.str.length() == compareTo.Data.str.length()) ? !_stricmp(Data.str.c_str(), compareTo.Data.str.c_str()) : false;
	case kDataType_Form:
		return Data.formID == compareTo.Data.formID;
	default:
		return Data.num == compareTo.Data.num;
	}
}

std::string ArrayElement::ToString() const
{
	switch (m_dataType)
	{
		case kDataType_Numeric:
		{
			return std::to_string(Data.num);
		}
		case kDataType_String:
		{
			return Data.str;
		}
		case kDataType_Array:
		{
			return std::vformat("Array ID {:0.f}", std::make_format_args(Data.num));
		}
		case kDataType_Form:
		{
			const UInt32 refID = Data.formID;
			if (TESForm* form = LookupFormByID(refID))
			{
				if (const char* formName = GetFullName(form))
					return formName;
			}
			return std::vformat("{:08x}", std::make_format_args(refID));
		}
	default:
		return "<INVALID>";
	}
}
			
bool ArrayElement::CompareAsString(const ArrayElement& lhs, const ArrayElement& rhs)
{
	// used by std::sort to sort elements by string representation
	// for forms, this is the name, or the actorvaluestring representation of formID if no name

	const std::string lhStr = lhs.ToString();
	const std::string rhStr = rhs.ToString();
	return _stricmp(lhStr.c_str(), rhStr.c_str()) < 0;
}

bool ArrayElement::SetForm(const TESForm* form)
{
	Unset();

	m_dataType = kDataType_Form;
	Data.formID = form ? form->refID : 0;
	return true;
}

bool ArrayElement::SetFormID(UInt32 refID)
{
	Unset();

	m_dataType = kDataType_Form;
	Data.formID = refID;
	return true;
}

bool ArrayElement::SetString(const std::string& str)
{
	Unset();

	m_dataType = kDataType_String;
	Data.str = str;
	return true;
}

bool ArrayElement::SetArray(ArrayID arr, UInt8 modIndex)
{
	Unset();

	m_dataType = kDataType_Array;
	if (m_owningArray) {
		g_ArrayMap.AddReference(&Data.num, arr, modIndex);
	}
	else {	// this element is not inside any array, so it's just a temporary
		Data.num = arr;
	}

	return true;
}

bool ArrayElement::SetNumber(double num)
{
	Unset();

	m_dataType = kDataType_Numeric;
	Data.num = num;
	return true;
}

bool ArrayElement::Set(const ArrayElement& elem)
{
	Unset();

	m_dataType = elem.DataType();
	switch (m_dataType)
	{
	case kDataType_String:
		SetString(elem.Data.str);
		break;
	case kDataType_Array:
		SetArray(elem.Data.num, g_ArrayMap.GetOwningModIndex(m_owningArray));
		break;
	case kDataType_Numeric:
		SetNumber(elem.Data.num);
		break;
	case kDataType_Form:
		SetFormID(elem.Data.formID);
		break;
	default:
		m_dataType = kTokenType_Invalid;
		return false;
	}

	return true;
}

bool ArrayElement::GetAsArray(ArrayID* out) const
{
	const auto intNum{ static_cast<ArrayID>(Data.num) };
	if (out == nullptr)
		return false;
	if (m_dataType != kDataType_Array)
		return false;
	if (intNum != 0 && !g_ArrayMap.Exists(intNum))	// it's okay for arrayID to be 0, otherwise check if array exists
		return false;

	*out = intNum;
	return true;
}

bool ArrayElement::GetAsFormID(UInt32* out) const
{
	if (!out || m_dataType != kDataType_Form)
		return false;
	*out = Data.formID;
	return true;
}

bool ArrayElement::GetAsNumber(double* out) const
{
	if (!out || m_dataType != kDataType_Numeric)
		return false;
	*out = Data.num;
	return true;
}

bool ArrayElement::GetAsString(std::string& out) const
{
	if (m_dataType != kDataType_String)
		return false;
	out = Data.str;
	return true;
}

void ArrayElement::Unset()
{
	if (m_dataType == kDataType_Array)
		g_ArrayMap.RemoveReference(&Data.num, g_ArrayMap.GetOwningModIndex(m_owningArray));
	
	m_dataType = kDataType_Invalid;
	Data.num = 0;
}

///////////////////////
// ArrayKey
//////////////////////

ArrayKey::ArrayKey(const std::string& _key)
{
	keyType = kDataType_String;
	key.str = _key;
}

ArrayKey::ArrayKey(const char* _key)
{
	keyType = kDataType_String;
	key.str = _key;
}

ArrayKey::ArrayKey(double _key)
{
	keyType = kDataType_Numeric;
	key.num = _key;
}

bool ArrayKey::operator<(const ArrayKey& rhs) const
{
	if (keyType != rhs.keyType)
	{
		_MESSAGE("Error: ArrayKey operator< mismatched keytypes");
		return true;
	}

	switch (keyType)
	{
	case kDataType_Numeric:
		return key.num < rhs.key.num;
	case kDataType_String:
		return _stricmp(key.str.c_str(), rhs.key.str.c_str()) < 0;
	default:
		_MESSAGE("Error: Invalid ArrayKey type %d", rhs.keyType);
		return true;
	}
}

bool ArrayKey::operator==(const ArrayKey& rhs) const
{
	if (keyType != rhs.keyType)
	{
		_MESSAGE("Error: ArrayKey operator== mismatched keytypes");
		return true;
	}

	switch (keyType)
	{
	case kDataType_Numeric:
		return key.num == rhs.key.num;
	case kDataType_String:
		return (key.str.length() == rhs.key.str.length()) ? !(_stricmp(key.str.c_str(), rhs.key.str.c_str())) : false;
	default:
		_MESSAGE("Error: Invalid ArrayKey type %d", rhs.keyType);
		return true;
	}
}

///////////////////////
// ArrayVar
//////////////////////

ArrayElement* ArrayVar::Get(ArrayKey key, bool bCanCreateNew)
{
	//TODO what do this?
	if (IsPacked() && key.KeyType() == kDataType_Numeric)
	{
		double idx = key.Key().num;
		if (idx < 0)
			idx += Size();
		const auto intIdx = static_cast<UInt32>(idx);
		key.SetNumericKey(intIdx);
	}

	const auto it = m_elements.find(key);
	if (it != m_elements.cend()) {
		return &it->second;
	}

	if (bCanCreateNew)
	{
		if (key.KeyType() == KeyType())
		{
			if (!IsPacked() || (key.Key().num <= Size()))
			{
				// create a new, uninitialized element
				ArrayElement* newElem = &m_elements[key];
				newElem->m_owningArray = m_ID;
				return newElem;
			}
		}
	}

	return nullptr;
}

bool ArrayVar::SetElementNumber(const ArrayKey* key, double num)
{
	ArrayElement* elem = this->Get(*key, true);
	if (!elem || !elem->SetNumber(num))
		return false;

	return true;
}

bool ArrayVar::SetElementString(const ArrayKey* key, const char* str)
{
	ArrayElement* elem = this->Get(*key, true);
	if (!elem || !elem->SetString(str))
		return false;

	return true;
}


bool ArrayVar::SetElementFormID(const ArrayKey* key, UInt32 refID)
{
	ArrayElement* elem = this->Get(*key, true);
	if (!elem || !elem->SetFormID(refID))
		return false;

	return true;
}

bool ArrayVar::SetElementArray(const ArrayKey* key, ArrayID srcID)
{
	ArrayElement* elem = this->Get(*key, true);
	if (!elem || !elem->SetArray(srcID, this->m_owningModIndex))
		return false;

	return true;
}

bool ArrayVar::GetElementNumber(const ArrayKey* key, double* out)
{
	const ArrayElement* elem = this->Get(*key, false);
	return (elem && elem->GetAsNumber(out));
}

bool ArrayVar::GetElementString(const ArrayKey* key, std::string& out)
{
	ArrayElement const* elem = this->Get(*key, false);
	return (elem && elem->GetAsString(out));
}

UInt32 ArrayVar::GetUnusedIndex() const
{
	UInt32 id = 0;
	while (m_elements.contains(id))
	{
		id++;
	}

	return id;
}

void ArrayVar::Dump()
{
	const char* owningModName = (*g_dataHandler)->GetNthModName(m_owningModIndex);

	Console_Print("Refs: %d Owner %02X: %s", m_refs.size(), m_owningModIndex, owningModName);
	_MESSAGE("Refs: %d Owner %02X: %s", m_refs.size(), m_owningModIndex, owningModName);

	for (auto iter = m_elements.begin(); iter != m_elements.end(); ++iter)
	{
		std::string elementInfo("[ ");
		switch (KeyType())
		{
			case kDataType_Numeric:
			{
				elementInfo += std::to_string(iter->first.Key().num);
				break;
			}
		case kDataType_String:
			{
				elementInfo += iter->first.Key().str;
				break;
			}
		default:
			{
				elementInfo += "?Unknown Key Type?";
			}
		}

		elementInfo += " ] : ";

		switch (iter->second.m_dataType)
		{
			case kDataType_Numeric:
			{
				elementInfo += std::vformat("{}", std::make_format_args(iter->second.Data.num));
				break;
			}
			case kDataType_String:
			{
				elementInfo += iter->second.Data.str;
				break;
			}
			case kDataType_Array:
			{
				elementInfo += std::vformat("(Array ID #{})", std::make_format_args(iter->second.Data.num));
				break;
			}
			case kDataType_Form:
			{
				const auto refID = iter->second.Data.formID;
				const auto formPtr = LookupFormByID(refID);
				const std::string nameVal = formPtr != nullptr ? GetFullName(formPtr) : "<?NAME?>";
				elementInfo += std::vformat("{1} ({0:08X})", std::make_format_args(refID, nameVal));
				break;
			}
			default:
			{
				elementInfo += "?Unknown Element Type?";
			}
		}

		Console_Print("%s", elementInfo.c_str());
		_MESSAGE("%s", elementInfo.c_str());
	}
}

void ArrayVar::Pack()
{
	if (!IsPacked() || !Size())
		return;

	// assume only one hole exists (i.e. we previously erased 0 or more contiguous elements)
	// these are double but will always hold integer values for packed arrays
	double curIdx = 0;		// last correct index

	for (auto iter = m_elements.begin(); iter != m_elements.end(); )
	{
		if (!(iter->first == curIdx))
		{
			ArrayElement* elem = Get(ArrayKey(curIdx), true);
			elem->Set(iter->second);
			iter->second.Unset();
			const auto toDelete = iter;
			++iter;
			m_elements.erase(toDelete);
		}
		else
			++iter;

		curIdx += 1;
	}
}

//////////////////////////
// ArrayVarMap
/////////////////////////

UInt8 ArrayVarMap::GetOwningModIndex(ArrayID id)
{
	if (const ArrayVar* arr = Get(id))
		return arr->m_owningModIndex;

	return 0;
}

void ArrayVarMap::Erase(ArrayID toErase)
{
	if (ArrayVar* var = Get(toErase))
	{
		// delete any arrays contained in array
		for (auto iter = var->m_elements.begin(); iter != var->m_elements.end(); ++iter)
		{
			iter->second.Unset();
		}

		delete var;
	}

	Delete(toErase);
}

void ArrayVarMap::Add(ArrayVar* var, UInt32 varID, UInt32 numRefs, UInt8* refs)
{
#if _DEBUG
	if (Exists(varID))
	{
		DEBUG_PRINT("ArrayVarMap::Add() -> ArrayID %d already exists and will be overwritten.", varID);
	}
#endif

	VarMap::Insert(varID, var);
	var->m_ID = varID;
	if (!numRefs)		// nobody refers to this array, queue for deletion
		MarkTemporary(varID, true);
	else				// record references to this array
	{
		for (UInt32 i = 0; i < numRefs; i++)
			var->m_refs.push_back(refs[i]);
	}
}

ArrayID	ArrayVarMap::Create(UInt32 keyType, bool bPacked, UInt8 modIndex)
{
	const auto newVar = new ArrayVar(keyType, bPacked, modIndex);
	const ArrayID varID = GetUnusedID();
	newVar->m_ID = varID;
	VarMap::Insert(varID, newVar);
	MarkTemporary(varID, true);		// queue for deletion until a reference to this array is made
	return varID;
}

ArrayID ArrayVarMap::Copy(ArrayID from, UInt8 modIndex, bool bDeepCopy)
{
	ArrayVar* src = Get(from);
	if (!src)
		return 0;

	const ArrayID copyID = Create(src->KeyType(), src->IsPacked(), modIndex);
	for (auto iter = src->m_elements.begin(); iter != src->m_elements.end(); ++iter)
	{
		if (iter->second.DataType() == kDataType_Array && bDeepCopy)
		{
			ArrayID innerID = 0;
			ArrayID innerCopyID = 0;
			if (iter->second.GetAsArray(&innerID))
				innerCopyID = Copy(innerID, modIndex, true);

			if (!SetElementArray(copyID, iter->first, innerCopyID))
			{
				DEBUG_PRINT("ArrayVarMap::Copy failed to make deep copy of inner array");
			}
		}
		else
		{
			if (!SetElement(copyID, iter->first, iter->second))
			{
				DEBUG_PRINT("ArrayVarMap::Copy failed to set element in copied array");
			}
		}
	}

	return copyID;
}

void ArrayVarMap::AddReference(ArrayID* ref, ArrayID toRef, UInt8 referringModIndex)
{
	if (*ref == toRef)
		return;			// already refers to this array

	if (*ref)			// refers to a different array, remove that reference
		RemoveReference(ref, referringModIndex);

	if (ArrayVar* arr = Get(toRef))
	{
		arr->m_refs.push_back(referringModIndex);	// record reference, increment refcount
		*ref = toRef;								// store ref'ed ArrayID in reference
		MarkTemporary(toRef, false);
	}
}
		
void ArrayVarMap::RemoveReference(ArrayID* ref, UInt8 referringModIndex)
{
	ArrayVar* var = Get(*ref);
	if (var)
	{
		// decrement refcount
		for (auto iter = var->m_refs.begin(); iter != var->m_refs.end(); ++iter)
		{
			if (*iter == referringModIndex)
			{
				var->m_refs.erase(iter);
				break;
			}
		}
	}

	// if refcount is zero, queue for deletion
	if (var && var->m_refs.empty())
	{
		MarkTemporary(var->ID(), true);
	}

	// store 0 in reference
	*ref = 0;
}

void ArrayVarMap::AddReference(double* ref, ArrayID toRef, UInt8 referringModIndex)
{
	ArrayID refID = *ref;
	AddReference(&refID, toRef, referringModIndex);
	*ref = refID;
}

void ArrayVarMap::RemoveReference(double* ref, UInt8 referringModIndex)
{
	ArrayID refID = *ref;
	RemoveReference(&refID, referringModIndex);
	*ref = refID;
}

ArrayID ArrayVarMap::MakeSlice(ArrayID src, const Slice* slice, UInt8 modIndex)
{
	// keys in slice match those in source unless array packed, in which case they must start at zero

	ArrayVar* srcVar = Get(src);
	if (!srcVar)
		return 0;
	
	std::map<ArrayKey, ArrayElement>::iterator start;
	ArrayKey lo;
	ArrayKey hi;

	if (slice->bIsString && srcVar->KeyType() == kDataType_String)
	{
		lo = ArrayKey(slice->m_lowerStr);
		hi = ArrayKey(slice->m_upperStr);

	}
	else if (!slice->bIsString && srcVar->KeyType() == kDataType_Numeric)
	{
		lo = ArrayKey(slice->m_lower);
		hi = ArrayKey(slice->m_upper);
	}
	else
		return 0;

	const ArrayID newID = Create(srcVar->KeyType(), srcVar->IsPacked(), modIndex);
	const bool bPacked = srcVar->IsPacked();

	for (start = srcVar->m_elements.begin(); start != srcVar->m_elements.end(); ++start)
	{
		if (start->first >= lo)
			break;
	}

	UInt32 packedIndex = 0;

	for (auto endIt = start; endIt != srcVar->m_elements.end(); ++endIt)
	{
		if (endIt->first > hi)
			break;

		if (bPacked)
			SetElement(newID, packedIndex++, endIt->second);
		else
			SetElement(newID, endIt->first, endIt->second);
	}

	return newID;
}

UInt32	ArrayVarMap::GetKeyType(ArrayID id)
{
	const ArrayVar* var = Get(id);
	return var ? var->KeyType() : kDataType_Invalid;
}

bool ArrayVarMap::Exists(ArrayID id)
{
	// redundant method, only name differs, preserved because it's used in several places
	return VarExists(id);
}

UInt32 ArrayVarMap::SizeOf(ArrayID id)
{
	const ArrayVar* var = Get(id);
	return var ? var->Size() : -1;
}

ArrayID ArrayVarMap::GetKeys(ArrayID id, UInt8 modIndex)
{
	// returns an array of all the keys
	ArrayVar* src = Get(id);
	if (!src)
		return 0;

	const ArrayID keyArrID = Create(kDataType_Numeric, true, modIndex);
	const UInt8 keysType = src->KeyType();
	UInt32 curIdx = 0;

	for (auto iter = src->m_elements.begin(); iter != src->m_elements.end(); ++iter)
	{
		if (keysType == kDataType_Numeric)
			SetElementNumber(keyArrID, curIdx, iter->first.Key().num);
		else
			SetElementString(keyArrID, curIdx, iter->first.Key().str);
		curIdx++;
	}

	return keyArrID;
}

bool ArrayVarMap::HasKey(ArrayID id, const ArrayKey& key)
{
	const ArrayVar* arr = Get(id);
	if (!arr || arr->KeyType() != key.KeyType())
		return false;

	return (arr->m_elements.contains(key));
}

bool ArrayVarMap::AsVector(ArrayID id, std::vector<const ArrayElement*> &vecOut)
{
	ArrayVar* arr = Get(id);
	if (!arr || !arr->IsPacked())
		return false;

	for (UInt32 i = 0; i < arr->Size(); i++)
	{
		vecOut.push_back(arr->Get(ArrayKey(i), false));
	}

	return true;
}

bool ArrayVarMap::SetSize(ArrayID id, UInt32 newSize, const ArrayElement& padWith)
{
	const ArrayVar* arr = Get(id);
	if (!arr || !arr->IsPacked())
		return false;

	if (arr->Size() < newSize)
	{
		for (UInt32 i = arr->Size(); i < newSize; i++)
			SetElement(id, ArrayKey(i), padWith);
	}
	else if (arr->Size() > newSize)
		return EraseElements(id, ArrayKey(newSize), ArrayKey(arr->Size() - 1)) != -1;

	return true;
}

bool ArrayVarMap::Insert(ArrayID id, UInt32 atIndex, const ArrayElement& toInsert)
{
	ArrayVar* arr = Get(id);
	if (!arr || !arr->IsPacked() || atIndex > arr->Size())
		return false;
	
	if (atIndex < arr->Size())	
	{
		// shift higher elements up by one
		for (SInt32 i = arr->Size(); i >= (SInt32)atIndex; i--)
			SetElement(id, ArrayKey(i), i > 0 ? arr->m_elements[i-1] : ArrayElement());
	}

	// insert
	SetElement(id, ArrayKey(atIndex), toInsert);
	return true;
}

bool ArrayVarMap::Insert(ArrayID id, UInt32 atIndex, ArrayID rangeID)
{
	ArrayVar* dest = Get(id);
	ArrayVar* src = Get(rangeID);
	if (!dest || !src || !dest->IsPacked() || !src->IsPacked() || atIndex > dest->Size())
		return false;

	const UInt32 shiftDelta = src->Size();

	// resize, pad with empty elements
	SetSize(id, dest->Size() + shiftDelta, ArrayElement());

	if (atIndex < dest->Size())
	{
		// shift up
		for (SInt32 i = dest->Size() - 1; i >= (SInt32)(atIndex + shiftDelta); i--)
			SetElement(id, ArrayKey(i), dest->m_elements[i-shiftDelta]);
	}

	// insert
	for (UInt32 i = 0; i < shiftDelta; i++)
		SetElement(id, ArrayKey(i + atIndex), src->m_elements[i]);

	return true;
}

class SortFunctionCaller : public FunctionCaller
{
	Script			* m_comparator;
	ArrayID			m_lhs{};
	ArrayID			m_rhs{};

public:
	explicit SortFunctionCaller(Script* comparator) : m_comparator(comparator)
	{ 
		if (comparator) 
		{
			m_lhs = g_ArrayMap.Create(kDataType_Numeric, true, comparator->GetModIndex());
			m_rhs = g_ArrayMap.Create(kDataType_Numeric, true, comparator->GetModIndex());
		}
	}

	~SortFunctionCaller() override = default;

	UInt8 ReadCallerVersion() override { return UserFunctionManager::kVersion; }
	auto ReadScript() -> Script* override { return m_comparator; }
	bool PopulateArgs(ScriptEventList* eventList, FunctionInfo* info) override
	{
		DynamicParamInfo& dParams = info->ParamInfo();
		if (dParams.NumParams() == 2) 
		{
			const UserFunctionParam* lhs = info->GetParam(0);
			const UserFunctionParam* rhs = info->GetParam(1);
			const bool areBothPointersGood = lhs && rhs;
			const bool areBothVarTypeArray = lhs->varType == Script::eVarType_Array && rhs->varType == Script::eVarType_Array;
			if (areBothPointersGood && areBothVarTypeArray)
			{
				const auto leftVar = eventList->GetVariable(lhs->varIdx);
				const auto rightVar = eventList->GetVariable(rhs->varIdx);
				// TODO if one of these null, should it still add the other one?
				if (!leftVar || !rightVar)
				{
					ShowRuntimeError(m_comparator, "Could not look up argument variable for function script");
					return false;
				}
				g_ArrayMap.AddReference(&leftVar->data, m_lhs, m_comparator->GetModIndex());
				g_ArrayMap.AddReference(&rightVar->data, m_rhs, m_comparator->GetModIndex());
				return true;
			}
		}

		return false;
	}
	auto ThisObj() -> TESObjectREFR* override { return nullptr; }
	auto ContainingObj() -> TESObjectREFR* override { return nullptr; }

	bool operator()(const ArrayElement& lhs, const ArrayElement& rhs)
	{
		g_ArrayMap.SetElement(m_lhs, 0.0, lhs);
		g_ArrayMap.SetElement(m_rhs, 0.0, rhs);
		const ScriptToken* result = UserFunctionManager::Call(std::move(*this));
		const bool bResult = result ? result->GetBool() : false;
		delete result;
		return bResult;
	}
};

ArrayID ArrayVarMap::Sort(ArrayID src, SortOrder order, SortType type, UInt8 modIndex, Script* comparator)
{
	// result is a packed integer-based array of the elements in sorted order
	// if array cannot be sorted we return empty array
	ArrayVar* srcVar = Get(src);
	const ArrayID result = Create(kDataType_Numeric, true, modIndex);
	if (!srcVar || !srcVar->Size())
		return result;

	// restriction: all elements of src must be of the same type for default sort
	// restriction not in effect for alpha sort (all values treated as strings) or custom sort (all values boxed as arrays)
	std::vector<ArrayElement> vec;
	auto iter = srcVar->m_elements.begin();
	const UInt32 dataType = iter->second.DataType();
	if (dataType == kDataType_Invalid || (dataType == kDataType_Array && !comparator))	// nonsensical to sort array of arrays with default sorters, custom sorter can be used
		return result;

	// copy elems to vec, verify all are of same type
	for ( ; iter != srcVar->m_elements.end(); ++iter)
	{
		if (type == kSortType_Default && iter->second.DataType() != dataType)
			return result;
		vec.push_back(iter->second);
	}

	// let STL do the sort
	if (type == kSortType_Default)
		std::sort(vec.begin(), vec.end());
	else if (type == kSortType_Alpha)
		std::sort(vec.begin(), vec.end(), ArrayElement::CompareAsString);
	else if (type == kSortType_UserFunction) 
	{
		if (!comparator) 
		{
			return result;
		}
		SortFunctionCaller sorter(comparator);
		std::sort(vec.begin(), vec.end(), sorter);
	}

	if (order == kSort_Descending)
		std::reverse(vec.begin(), vec.end());

	for (UInt32 i = 0; i < vec.size(); i++)
		SetElement(result, i, vec[i]);

	return result;
}

UInt32 ArrayVarMap::EraseElements(ArrayID id, const ArrayKey& lo, const ArrayKey& hi)
{
	ArrayVar* var = Get(id);
	if (!var || lo.KeyType() != hi.KeyType() || lo.KeyType() != var->KeyType())
		return -1;

	// find first elem to erase
	auto iter = var->m_elements.begin();
	while (iter != var->m_elements.end() && iter->first < lo)
		++iter;

	UInt32 numErased = 0;

	// erase. if element is an arrayID, clean up that array
	while (iter != var->m_elements.end() && iter->first <= hi)
	{
		iter->second.Unset();
		iter = var->m_elements.erase(iter);
		numErased++;
	}

	// if array is packed we must shift elements down
	if (var->IsPacked())
		var->Pack();

	return numErased;
}

UInt32 ArrayVarMap::EraseAllElements(ArrayID id)
{
	UInt32 numErased = -1;
	if (ArrayVar* var = Get(id)) 
	{
		while (var->m_elements.begin() != var->m_elements.end())
		{
			var->m_elements.begin()->second.Unset();
			var->m_elements.erase(var->m_elements.begin());
			numErased++;
		}
	}

	return numErased;
}

bool ArrayVarMap::SetElementNumber(ArrayID id, const ArrayKey& key, double num)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	ArrayElement* elem = arr->Get(key, true);
	if (!elem || !elem->SetNumber(num))
		return false;

	return true;
}

bool ArrayVarMap::SetElementString(ArrayID id, const ArrayKey& key, const std::string& str)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	ArrayElement* elem = arr->Get(key, true);
	if (!elem || !elem->SetString(str))
		return false;

	return true;
}

bool ArrayVarMap::SetElementFormID(ArrayID id, const ArrayKey& key, UInt32 refID)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	ArrayElement* elem = arr->Get(key, true);
	if (!elem || !elem->SetFormID(refID))
		return false;

	return true;
}

bool ArrayVarMap::SetElementArray(ArrayID id, const ArrayKey& key, ArrayID srcID)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	ArrayElement* elem = arr->Get(key, true);
	if (!elem || !elem->SetArray(srcID, arr->m_owningModIndex))
		return false;

	return true;
}

bool ArrayVarMap::SetElement(ArrayID id, const ArrayKey& key, const ArrayElement& val)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	ArrayElement* elem = arr->Get(key, true);
	if (!elem || !elem->Set(val))
		return false;

	return true;
}

bool ArrayVarMap::GetElementNumber(ArrayID id, const ArrayKey& key, double* out)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	const ArrayElement* elem = arr->Get(key, false);
	return (elem && elem->GetAsNumber(out));
}

bool ArrayVarMap::GetElementString(ArrayID id, const ArrayKey& key, std::string& out)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	const ArrayElement* elem = arr->Get(key, false);
	return (elem && elem->GetAsString(out));
}

bool ArrayVarMap::GetElementCString(ArrayID id, const ArrayKey& key, const char** out)
{
	if (ArrayVar* arr = Get(id))
	{
		if (const ArrayElement* elem = arr->Get(key, false); elem && elem->DataType() == kDataType_String)
		{
			*out = elem->Data.str.c_str();
			return true;
		}
	}

	return false;
}

bool ArrayVarMap::GetElementFormID(ArrayID id, const ArrayKey& key, UInt32* out)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	const ArrayElement* elem = arr->Get(key, false);
	return (elem && elem->GetAsFormID(out));
}

bool ArrayVarMap::GetElementForm(ArrayID id, const ArrayKey& key, TESForm** out)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	const ArrayElement* elem = arr->Get(key, false);
	UInt32 refID = 0;
	if (elem && elem->GetAsFormID(&refID))
	{
		*out = LookupFormByID(refID);
		return true;
	}

	return false;
}

UInt8 ArrayVarMap::GetElementType(ArrayID id, const ArrayKey& key)
{
	if (ArrayVar* arr = Get(id))
	{
		if (const ArrayElement* elem = arr->Get(key, false))
			return elem->DataType();
	}

	return kDataType_Invalid;
}

bool ArrayVarMap::GetElementArray(ArrayID id, const ArrayKey& key, ArrayID* out)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	const ArrayElement* elem = arr->Get(key, false);
	return (elem && elem->GetAsArray(out));
}

bool ArrayVarMap::GetElementAsBool(ArrayID id, const ArrayKey& key, bool* out)
{
	ArrayVar* arr = Get(id);
	if (!arr)
		return false;

	const ArrayElement* elem = arr->Get(key, false);
	if (!elem)
		return false;

	if (elem->DataType() == kDataType_String)
		*out = true;			// strings are always "true", whatever that means in this context
	else
		*out = elem->Data.num ? true : false;

	return true;
}

void ArrayVarMap::DumpArray(ArrayID toDump)
{
	Console_Print("** Dumping Array #%d **", toDump);
	_MESSAGE("**\nDumping Array #%d **", toDump);

	if (ArrayVar* arr = Get(toDump))
		arr->Dump();
	else
	{
		Console_Print("  Array does not exist");
		_MESSAGE("  Array does not exist");
	}
}

void ArrayVarMap::Save(OBSESerializationInterface* intfc)
{
	Clean();

	intfc->OpenRecord('ARVS', kVersion);

	std::map<UInt32, ArrayVar*> & vars = m_state->vars;
	for (auto iter = vars.begin(); iter != vars.end(); ++iter)
	{
		if (IsTemporary(iter->first))
			continue;

		intfc->OpenRecord('ARVR', kVersion);
		intfc->WriteRecordData(&iter->second->m_owningModIndex, sizeof(UInt8));
		intfc->WriteRecordData(&iter->first, sizeof(UInt32));
		intfc->WriteRecordData(&iter->second->m_keyType, sizeof(UInt8));
		intfc->WriteRecordData(&iter->second->m_bPacked, sizeof(bool));
		
		UInt32 numRefs = iter->second->m_refs.size();
		intfc->WriteRecordData(&numRefs, sizeof(numRefs));
		if (!numRefs)
			_MESSAGE("ArrayVarMap::Save(): saving array with no references");

		for (UInt32 i = 0; i < numRefs; i++)
			intfc->WriteRecordData(&iter->second->m_refs[i], sizeof(UInt8));

		UInt32 numElements = iter->second->Size();
		intfc->WriteRecordData(&numElements, sizeof(UInt32));

		const UInt8 keyType = iter->second->m_keyType;
		for (auto elems = iter->second->m_elements.begin();
			elems != iter->second->m_elements.end(); ++elems)
		{
			ArrayType key = elems->first.Key();
			if (keyType == kDataType_Numeric)
				intfc->WriteRecordData(&key.num, sizeof(double));
			else
			{
				auto len = static_cast<UInt16>(key.str.length());
				intfc->WriteRecordData(&len, sizeof(len));
				intfc->WriteRecordData(key.str.c_str(), key.str.length());
			}

			intfc->WriteRecordData(&elems->second.m_dataType, sizeof(UInt8));
			switch (elems->second.m_dataType)
			{
				case kDataType_Numeric:
				{
					intfc->WriteRecordData(&elems->second.Data.num, sizeof(double));
					break;
				}
				case kDataType_String:
				{
				auto len = static_cast<UInt16>(elems->second.Data.str.length());
					intfc->WriteRecordData(&len, sizeof(len));
					intfc->WriteRecordData(elems->second.Data.str.c_str(), elems->second.Data.str.length());
					break;
				}
				case kDataType_Array:
				{
					ArrayID id = elems->second.Data.num;
					intfc->WriteRecordData(&id, sizeof(id));
					break;
				}
				case kDataType_Form:
				{
					intfc->WriteRecordData(&elems->second.Data.formID, sizeof(UInt32));
					break;
				}
			default:
				{
					_MESSAGE("Error in ArrayVarMap::Save() - unhandled element type %d. Element not saved.", elems->second.m_dataType);
				}
			}
		}
	}

	intfc->OpenRecord('ARVE', kVersion);
}

void ArrayVarMap::Load(OBSESerializationInterface* intfc)
{
	_MESSAGE("Loading array variables");

	Clean();		// clean up any vars queued for garbage collection
	UInt32 	type,
			length,
			version,
			arrayID,
			numElements;

	UInt16 strLength{};
	UInt8 modIndex{}, keyType{};
	bool bPacked{ false };
	std::array<char, kMaxMessageLength> buffer{};
	bool bContinue{ true };

	UInt32 lastIndexRead = 0;

	while (bContinue && intfc->GetNextRecordInfo(&type, &version, &length))
	{
		switch (type)
		{
			case 'ARVE':			//end of block
			{
				bContinue = false;
				break;
			}
			case 'ARVR':
			{
				bool isUnloaded{ false };
				UInt32 tempRefIdVal{};
				intfc->ReadRecordData(&modIndex, sizeof(modIndex));
				if (!intfc->ResolveRefID(modIndex << 24, &tempRefIdVal))
				{
					// owning mod was removed, but there may be references to it from other mods
					// assign ownership to the first mod which refers to it and is still loaded
					// if no loaded mods refer to it, discard
					// we handle all of that below
					_MESSAGE("Mod owning array was removed from load order; will attempt to assign ownership to a referring mod.");
					modIndex = 0;  //index 0 by itself don't indicate an unloaded mod mod. 
					isUnloaded = true;  
				}
				else
					modIndex = (tempRefIdVal >> 24);

				intfc->ReadRecordData(&arrayID, sizeof(arrayID));
				intfc->ReadRecordData(&keyType, sizeof(keyType));
				intfc->ReadRecordData(&bPacked, sizeof(bPacked));

				// read refs, fix up mod indexes, discard refs from unloaded mods
				UInt32 numRefs = 0;		// # of references to this array
				std::vector<UInt8> refs;

				// reference-counting implemented in v1
				if (version >= 1)
				{
					intfc->ReadRecordData(&numRefs, sizeof(numRefs));
					if (numRefs)
					{
						refs.resize(numRefs);
						UInt32 tempRefID = 0;
						UInt8 curModIndex = 0;
						UInt32 refIdx = 0;
						for (UInt32 i = 0; i < numRefs; i++) 
						{
							intfc->ReadRecordData(&curModIndex, sizeof(curModIndex));

							if (intfc->ResolveRefID(curModIndex << 24, &tempRefID)) 
							{
								if (isUnloaded) 
								{
									modIndex = tempRefID >> 24;
									_MESSAGE("ArrayID %d was owned by an unloaded mod. Assigning ownership to mod #%d", arrayID, modIndex);
									isUnloaded = false;
								}
								refs[refIdx++] = (tempRefID >> 24);
							}	
						}

						numRefs = refIdx;
					}
				}
				else		// v0 arrays assumed to have only one reference (the owning mod)
				{
					if (!isUnloaded)		// owning mod is loaded
					{
						numRefs = 1;
						refs.resize(1);
						refs[0] = modIndex;
					}
				}
				
				if (isUnloaded && !modIndex )
				{
					_MESSAGE("Array ID %d is referred to by no loaded mods. Discarding", arrayID);
					refs.clear();
					continue;
				}

				// record gaps between IDs for easy lookup later in GetUnusedID()
				lastIndexRead++;
				while (lastIndexRead < arrayID)
				{
					SetIDAvailable(lastIndexRead);
					lastIndexRead++;
				}
				
				// create array and add to map
				auto newArr = new ArrayVar(keyType, bPacked, modIndex);
				Add(newArr, arrayID, numRefs, refs.data());
				refs.clear();

				// read the array elements			
				intfc->ReadRecordData(&numElements, sizeof(numElements));
				for (UInt32 i = 0; i < numElements; i++)
				{
					ArrayKey newKey;
					if (keyType == kDataType_Numeric)
					{
						double num;
						intfc->ReadRecordData(&num, sizeof(double));
						newKey = num;
					}
					else
					{
						intfc->ReadRecordData(&strLength, sizeof(strLength));
						intfc->ReadRecordData(buffer.data(), strLength);
						buffer[strLength] = 0;
						newKey = std::string(buffer.data());
					}

					UInt8 elemType;
					if (intfc->ReadRecordData(&elemType, sizeof(elemType)) == 0)
					{
						_MESSAGE("ArrayVarMap::Load() reading past end of file");
						return;
					}

					switch (elemType)
					{
						case kDataType_Numeric:
						{
							double num;
							intfc->ReadRecordData(&num, sizeof(num));
							SetElementNumber(arrayID, newKey, num);
							break;
						}
						case kDataType_String:
						{
							intfc->ReadRecordData(&strLength, sizeof(strLength));
							intfc->ReadRecordData(buffer.data(), strLength);
							buffer[strLength] = 0;
							SetElementString(arrayID, newKey, std::string(buffer.data()));
							break;
						}
						case kDataType_Array:
						{
							ArrayID id;
							intfc->ReadRecordData(&id, sizeof(id));
							if (newArr)
							{
								ArrayElement* elem = newArr->Get(newKey, true);
								if (elem)
								{
									elem->m_dataType = kDataType_Array;
									elem->Data.num = id;
									elem->m_owningArray = arrayID;
								}
							}

							break;
						}
						case kDataType_Form:
						{
							UInt32 formID;
							intfc->ReadRecordData(&formID, sizeof(formID));
							if (!intfc->ResolveRefID(formID, &formID))
								formID = 0;

							SetElementFormID(arrayID, newKey, formID);
							break;
						}
						default:
						{
							_MESSAGE("Unknown element type %d encountered while loading array var, element discarded.", elemType);
							break;
						}
					}
				}

				break;
			}
			default:
			{
				_MESSAGE("Error loading array var map: unexpected chunk type %d", type);
				break;
			}
		}
	}
}

bool ArrayVarMap::GetFirstElement(ArrayID id, ArrayElement* outElem, ArrayKey* outKey)
{
	ArrayVar* var = Get(id);
	if (!var || !var->Size() || !outElem || !outKey)
		return false;

	const auto iter = var->m_elements.begin();
	*outKey = iter->first;
	*outElem = iter->second;
	return true;
}

bool ArrayVarMap::GetLastElement(ArrayID id, ArrayElement* outElem, ArrayKey* outKey)
{
	ArrayVar* var = Get(id);
	if (!var || !var->Size() || !outElem || !outKey)
		return false;

	auto iter = var->m_elements.end();
	if (var->Size() > 1)
		--iter;
	else		// only one element
		iter= var->m_elements.begin();

	*outKey = iter->first;
	*outElem = iter->second;
	return true;
}

bool ArrayVarMap::GetNextElement(ArrayID id, ArrayKey* prevKey, ArrayElement* outElem, ArrayKey* outKey)
{
	ArrayVar* var = Get(id);
	if (!var || !var->Size() || !outElem || !outKey || !prevKey)
		return false;

	auto iter = var->m_elements.find(*prevKey);
	if (iter != var->m_elements.end())
	{
		++iter;
		if (iter != var->m_elements.end())
		{
			//var->m_cachedIterator = iter;

			*outKey = iter->first;
			*outElem = iter->second;
			return true;
		}
	}

	return false;
}

bool ArrayVarMap::GetPrevElement(ArrayID id, ArrayKey* prevKey, ArrayElement* outElem, ArrayKey* outKey)
{
	ArrayVar* var = Get(id);
	if (!var || !var->Size() || !outElem || !outKey || !prevKey)
		return false;

	auto iter = var->m_elements.find(*prevKey);
	if (iter != var->m_elements.end() && iter != var->m_elements.begin())
	{
		--iter;
		*outKey = iter->first;
		*outElem = iter->second;
		return true;
	}

	return false;
}

ArrayKey ArrayVarMap::Find(ArrayID toSearch, const ArrayElement& toFind, const Slice* range)
{
	ArrayKey foundIndex;
	ArrayVar* var = Get(toSearch);
	if (!var)
		return foundIndex;

	auto start = var->m_elements.begin();
	auto end = var->m_elements.end();
	if (range)
	{
		if ((range->bIsString && var->KeyType() != kDataType_String) || (!range->bIsString && var->KeyType() != kDataType_Numeric))
			return foundIndex;
		
		// locate lower and upper bounds
		ArrayKey lo;
		ArrayKey hi;
		range->GetArrayBounds(lo, hi);

		while (start != var->m_elements.end() && start->first < lo)
			++start;

		end = start;
		while (end != var->m_elements.end() && end->first <= hi)
			++end;
	}

	// do the search
	for (auto iter = start; iter != end; ++iter)
	{
		if (iter->second.Equals(toFind))
		{
			foundIndex = iter->first;
			break;
		}
	}

	return foundIndex;
}

std::string ArrayVarMap::GetTypeString(ArrayID arr)
{
	std::string result = "<Bad Array>";
	if (const ArrayVar* a = Get(arr))
	{
		if (a->KeyType() == kDataType_Numeric)
			result = a->IsPacked() ? "Array" : "Map";
		else if (a->KeyType() == kDataType_String)
			result = "StringMap";
	}

	return result;
}

void ArrayVarMap::Clean()		// garbage collection: delete unreferenced arrays
{
	// ArrayVar destructor may queue more IDs for deletion if deleted array contains other arrays
	// so on each pass through the loop we delete the first ID in the queue until none remain

	if (m_state) {
		while (!m_state->tempVars.empty())
		{
			const UInt32 idToDelete = *(m_state->tempVars.begin());
			Delete(idToDelete);
		}
	}
}

namespace PluginAPI
{
	bool ArrayAPI::SetElementFromAPI(UInt32 id, const ArrayKey& key, const OBSEArrayVarInterface::Element& elem)
	{
		switch (elem.type)
		{
		case elem.kType_Array:
			return g_ArrayMap.SetElementArray(id, key, (ArrayID)elem.arr);
		case elem.kType_Numeric:
			return g_ArrayMap.SetElementNumber(id, key, elem.num);
		case elem.kType_String:
			return g_ArrayMap.SetElementString(id, key, elem.str);
		case elem.kType_Form:
			return g_ArrayMap.SetElementFormID(id, key, elem.form ? elem.form->refID : 0);
		default:
			return false;
		}
	}

	OBSEArrayVarInterface::Array* ArrayAPI::CreateArray(const OBSEArrayVarInterface::Element* data, UInt32 size, Script* callingScript)
	{
		ArrayID id = g_ArrayMap.Create(kDataType_Numeric, true, callingScript->GetModIndex());
		for (UInt32 i = 0; i < size; i++)
		{
			if (!SetElementFromAPI(id, ArrayKey(i), data[i]))
			{
				_MESSAGE("Error: An attempt by a plugin to set an array element failed.");
				return nullptr;
			}
		}

		return (OBSEArrayVarInterface::Array*)id;
	}

	OBSEArrayVarInterface::Array* ArrayAPI::CreateStringMap(const char** keys, const OBSEArrayVarInterface::Element* values, UInt32 size, Script* callingScript)
	{
		ArrayID id = g_ArrayMap.Create(kDataType_String, false, callingScript->GetModIndex());

		for (UInt32 i = 0; i < size; i++) {
			if (!SetElementFromAPI(id, ArrayKey(keys[i]), values[i])) {
				_MESSAGE("Error: An attempt by a plugin to set an array element failed.");
				return nullptr;
			}
		}

		return (OBSEArrayVarInterface::Array*)id;
	}

	OBSEArrayVarInterface::Array* ArrayAPI::CreateMap(const double* keys, const OBSEArrayVarInterface::Element* values, UInt32 size, Script* callingScript)
	{
		ArrayID id = g_ArrayMap.Create(kDataType_Numeric, false, callingScript->GetModIndex());

		for (UInt32 i = 0; i < size; i++) {
			if (!SetElementFromAPI(id, ArrayKey(keys[i]), values[i])) {
				_MESSAGE("Error: An attempt by a plugin to set an array element failed.");
				return nullptr;
			}
		}

		return (OBSEArrayVarInterface::Array*)id;
	}

	bool ArrayAPI::AssignArrayCommandResult(OBSEArrayVarInterface::Array* arr, double* dest)
	{
		if (!g_ArrayMap.Get((ArrayID)arr))
		{
			_MESSAGE("Error: A plugin is attempting to return a non-existent array.");
			return false;
		}

		*dest = (UInt32)arr;
		return true;
	}

	void ArrayAPI::SetElement(OBSEArrayVarInterface::Array* arr, const OBSEArrayVarInterface::Element& key, const OBSEArrayVarInterface::Element& value)
	{
		auto arrID = (ArrayID)arr;

		switch (key.type)
		{
		case OBSEArrayVarInterface::Element::kType_Numeric:
			SetElementFromAPI(arrID, ArrayKey(key.num), value);
			break;
		case OBSEArrayVarInterface::Element::kType_String:
			SetElementFromAPI(arrID, ArrayKey(key.str), value);
			break;
		default:
			_MESSAGE("Error: a plugin is attempting to create an array element with an invalid key type.");
		}
	}

	void ArrayAPI::AppendElement(OBSEArrayVarInterface::Array* arr, const OBSEArrayVarInterface::Element& value)
	{
		auto arrID = (ArrayID)arr;
		ArrayVar* theArray = g_ArrayMap.Get(arrID);

		// verify array is the packed, numeric kind
		if (theArray && theArray->KeyType() == kDataType_Numeric && theArray->IsPacked()) {
			SetElementFromAPI(arrID, ArrayKey(theArray->Size()), value);
		}
	}

	UInt32 ArrayAPI::GetArraySize(OBSEArrayVarInterface::Array* arr)
	{
		return g_ArrayMap.SizeOf((ArrayID)arr);
	}

	OBSEArrayVarInterface::Array* ArrayAPI::LookupArrayByID(UInt32 id)
	 {
		 if (g_ArrayMap.Exists(id))
			 return (OBSEArrayVarInterface::Array*)id;
		 else
			 return nullptr;
	 }

	bool ArrayAPI::GetElement(OBSEArrayVarInterface::Array* arr, const OBSEArrayVarInterface::Element& key, OBSEArrayVarInterface::Element& out)
	 {
		 ArrayVar* var = g_ArrayMap.Get((ArrayID)arr);
		 ArrayElement* data = nullptr;
		 if (var) {
			 switch (key.type) {
				 case key.kType_String:
					 if (var->KeyType() == kDataType_String) {
						 data = var->Get(key.str, false);
					 }
					 break;
				 case key.kType_Numeric:
					 if (var->KeyType() == kDataType_Numeric) {
						 data = var->Get(key.num, false);
					 }
					 break;
			 }
		 
			 if (data) {
				 return InternalElemToPluginElem(*data, out);
			 }
		 }

		 return false;
	}
		
	bool ArrayAPI::GetElements(OBSEArrayVarInterface::Array* arr, OBSEArrayVarInterface::Element* elements,
		OBSEArrayVarInterface::Element* keys)
	{
		if (!elements)
			return false;

		ArrayVar* var = g_ArrayMap.Get((ArrayID)arr);
		if (var) {
			UInt32 size = var->Size();
			UInt8 keyType = var->KeyType();

			if (size != -1) {
				UInt32 i = 0;
				for (auto iter = var->m_elements.begin(); iter != var->m_elements.end(); ++iter) {
					if (keys) {
						switch (keyType) {
							case kDataType_Numeric:
							{
								keys[i] = iter->first.Key().num;
								break;
							}
							case kDataType_String:
							{
								keys[i] = OBSEArrayVarInterface::Element(iter->first.Key().str.c_str());
								break;
							}
						}
					}
					
					InternalElemToPluginElem(iter->second, elements[i]);
					i++;
				}

				return true;
			}
		}

		return false;
	}

	// helpers
	bool ArrayAPI::InternalElemToPluginElem(ArrayElement& src, OBSEArrayVarInterface::Element& out)
	{
		switch (src.DataType()) 
		{
			case kDataType_Numeric:
			{
				double num;
				src.GetAsNumber(&num);
				out = num;
				break;
			}
			case kDataType_Form:
			{
				UInt32 formID;
				src.GetAsFormID(&formID);
				out = LookupFormByID(formID);
				break;
			}
			case kDataType_Array:
			{
				ArrayID arrID;
				src.GetAsArray(&arrID);
				out = (OBSEArrayVarInterface::Array*)arrID;
				break;
			}
			case kDataType_String:
			{
				std::string str;
				src.GetAsString(str);
				out = OBSEArrayVarInterface::Element(str.c_str());
				break;
			}
		}

		return true;
	}
}
