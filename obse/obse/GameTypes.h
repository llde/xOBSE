#pragma once

#include "obse/NiTypes.h"

class TESForm;
class EffectSetting;

extern NiTPointerMap <TESForm>			* g_formTable;
extern NiTPointerMap <EffectSetting>	* g_EffectSettingCollection;	// object is more complex than this

class TESObjectCELL;

// 004?
class TESChildCell
{
public:
	TESChildCell();
	~TESChildCell();

	// no virtual destructor
	virtual TESObjectCELL *	GetChildCell(void);
};

class BSSimpleList
{
public:
	BSSimpleList();
	~BSSimpleList();

	virtual void	Destructor(void);
};

// 8
class BSStringT
{
public:
	BSStringT();
	~BSStringT();

	char	* m_data;
	UInt16	m_dataLen;
	UInt16	m_bufLen;

	bool	Set(const char * src);
	bool	Includes(const char* toFind) const;
	bool	Replace(const char* toReplace, const char* replaceWith); // replaces instance of toReplace with replaceWith
	bool	Append(const char* toAppend);
	double	Compare(const BSStringT& compareTo, bool caseSensitive = false);
};

extern NiTPointerList <TESForm>	* g_quickKeyList;	//array of 8 NiTPointerLists of size 0-1 with pointers to hotkeyed items/spells

// 10
class ThreadSpecificInterfaceManager {
public:
	UInt32 maxthread;
	UInt32 tlsStorage;
	void* unk08;
	UInt32 numCurrentThreads;

	//No vtbl.
	//Some other non virtual methods are present.
	ThreadSpecificInterfaceManager(UInt32 maxthread) { ThisStdCall(0x00435E70, this, maxthread ); }
	UInt32 AddInterface(void* funcPtr) { return ThisStdCall(0x00431D60, this, funcPtr); }
};
STATIC_ASSERT(sizeof(ThreadSpecificInterfaceManager) == 0x10);

// 1C
template <typename T>
class LockFreeMap
{
public:
	virtual void Unk_00(UInt32 arg0) = 0;
	virtual bool Unk_01(UInt32 arg0, UInt32 arg1, UInt32 arg2) = 0;	// lookup, arg1 = ptr to output buffer
	virtual void Unk_02(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4) = 0;
	virtual void Unk_03(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3) = 0;
	virtual void Unk_04(UInt32 arg0, UInt32 agr1) = 0;
	virtual void Unk_05(UInt32 arg0, UInt32 comperand) = 0;
	virtual void Unk_06(UInt32 arg0) = 0;
	virtual void Unk_07(UInt32 arg0, UInt32 arg1) = 0;		// compute hash
	virtual void Unk_08(void) = 0;
	virtual void Unk_09(UInt32 arg0, UInt32 arg1) = 0;
	virtual void Unk_0A(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3) = 0;
	virtual void Unk_0B(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3) = 0;
	virtual void IncrementUnk0CCount(void) = 0;
	virtual void DecrementUnk0CCount(void) = 0;
	virtual UInt32 GetUnk0CCount(void) = 0;

	void	* unk04;				// 004
	UInt32	numBuckets;				// 008
	void	* buckets;				// 00C has unk0C_count elements; init to 0
	UInt32	unk10;					// 010
	ThreadSpecificInterfaceManager* unk14;				// 014 ptr to 0x10 byte struct
	UInt32	unk18;					// 018 init to 0
};
STATIC_ASSERT(sizeof(LockFreeMap<int>) == 0x1C);
// 1C
template <typename T>
class LockFreeQueue
{
public:
	virtual void Unk_00(void) = 0;

	struct Node {
		Node	* next;
		T		data;
	};

	// void** vtbl
	Node	* head;						// 004
	Node	* tail;						// 008
	UInt32	unk0C;						// 00C -init to c'tr arg1
	void	* unk10;					// 010 -dynamic alloc
	ThreadSpecificInterfaceManager	* unk14;					// 0x14 (0x10 byte struct, called with arg0)
	UInt32	unk18;						// 0x18 -init to 0
};

enum {
	eListCount = -3,
	eListEnd = -2,
	eListInvalid = -1,
};

template <class Item, bool _bHeadIsPtr>
class tListBase
{
	typedef Item tItem;

public:
	struct _Node {
		tItem*	item;
		_Node*	next;

		tItem* Item() const { return item; }
		_Node* Next() const { return next; }
		void   SetNext(_Node* nuNext) { next = nuNext;	}	// assumes next is currently NULL or doesn't need to be freed

		// become the next entry and return my item
		tItem* RemoveMe() {
			tItem* pRemoved = item;
			_Node* pNext = next;
			if (pNext) {
				item = pNext->item;
				next = pNext->next;
				FormHeap_Free(pNext);
			} else {
				item = NULL;
				next = NULL;
			}
			return pRemoved;
		}
	};

private:
	_Node m_listHead;

	class AcceptEqual {
		const Item	* m_toMatch;
	public:
		AcceptEqual(const Item* info) : m_toMatch(info) { }
		bool Accept(const Item* info) {
			return info == m_toMatch;
		}
	};

	template <class Op>
	UInt32 FreeNodes(_Node* node, Op &compareOp) const
	{
		static UInt32 nodeCount = 0;
		static UInt32 numFreed = 0;
		static _Node* lastNode = NULL;
		static bool bRemovedNext = false;
		if (node == nullptr) return 0;
		if (node->Next())
		{
			nodeCount++;
			FreeNodes(node->Next(), compareOp);
			nodeCount--;
		}

		if (compareOp.Accept(node->Item()))
		{
			node->RemoveMe();
			numFreed++;
			bRemovedNext = true;
		}
		else
		{
			if (bRemovedNext)
				node->SetNext(lastNode);
			bRemovedNext = false;
			lastNode = node;
		}

		if (!nodeCount)	//reset vars after recursing back to head
		{
			numFreed = 0;
			lastNode = NULL;
			bRemovedNext = false;
		}

		return numFreed;
	}

	struct NodePos {
		NodePos(): node(NULL), index(eListInvalid) {}

		_Node*	node;
		SInt32	index;
	};

	NodePos GetNthNode(SInt32 index) const {
		NodePos pos;
		SInt32 n = 0;
		_Node* pCur = Head();

		while (pCur && pCur->Item()) {
			if (n == index) break;
			if (eListEnd == index && !pCur->Next()) break;
			pCur = pCur->Next();
			++n;
		}

		pos.node = pCur;
		pos.index = n;

		return pos;
	}

public:

	_Node* Head() const {
		if (m_listHead.item == NULL && m_listHead.next == NULL) return nullptr;  //First node is empty, don't construct iterator
		return const_cast<_Node*>(&m_listHead);
	 }

	class Iterator
	{
		_Node*	m_cur;
	public:
		Iterator() : m_cur(NULL) {}
		Iterator(_Node* node) : m_cur(node) { }
		Iterator operator++()	{ if (!End()) m_cur = m_cur->Next(); return *this;}
		bool End()	{	return m_cur == NULL;	}
		Item* operator->() { return (m_cur) ? m_cur->Item() : NULL; }
		const Item* operator*() { return (m_cur) ? m_cur->Item() : NULL; }
		const Iterator& operator=(const Iterator& rhs) {
			m_cur = rhs.m_cur;
			return *this;
		}

		Item* Get() { return (m_cur) ? m_cur->Item() : NULL; }
		_Node* accessNode() { return m_cur; }
		Item* RemoveMe() {
			if (!m_cur) return NULL;
			return m_cur->RemoveMe();
		}
		void SetInner(Item*  rhs) {
			if (m_cur) {
				m_cur->item = rhs;
			}
		}
	};

	const Iterator Begin() const { return Iterator(Head()); }

	UInt32 Count() const {
		NodePos pos = GetNthNode(eListCount);
		return (pos.index > 0) ? pos.index : 0;
	};

	Item* GetNthItem(SInt32 n) const {
		NodePos pos = GetNthNode(n);
		return (pos.index == n && pos.node) ? pos.node->Item() : NULL;
	}

	Item* GetLastItem() const {
		NodePos pos = GetNthNode(eListEnd);
		return pos.node->Item();
	}

	SInt32 AddAt(Item* item, SInt32 index) {
		if (!m_listHead.item) {
			m_listHead.item = item;
			return 0;
		}

		NodePos pos = GetNthNode(index);
		_Node* pTargetNode = pos.node;
		_Node* newNode = (_Node*)FormHeap_Allocate(sizeof(_Node));
		if (newNode && pTargetNode) {
			if (index == eListEnd) {
				pTargetNode->next = newNode;
				newNode->item = item;
				newNode->next = NULL;
			} else {
				newNode->item = pTargetNode->item;
				newNode->next = pTargetNode->next;
				pTargetNode->item = item;
				pTargetNode->next = newNode;
			}
			return pos.index;
		}
		return eListInvalid;
	}

	template <class Op>
	void Visit(Op& op, _Node* prev = NULL) const {
		const _Node* pCur = (prev) ? prev->next : Head();
		bool bContinue = true;
		while (pCur && bContinue) {
			if (pCur->Item() == NULL)
				bContinue = true;
			else
				bContinue = op.Accept(pCur->Item());

			if (bContinue) {
				pCur = pCur->next;
			}
		}
	}

	template <class Op>
	Item* Find(Op& op) const
	{
		const _Node* pCur = Head();

		bool bFound = false;
		while (pCur && !bFound)
		{
			if (!pCur->Item())
				pCur = pCur->Next();
			else
			{
				bFound = op.Accept(pCur->Item());
				if (!bFound)
					pCur = pCur->Next();
			}
		}
		return (bFound && pCur) ? pCur->Item() : NULL;
	}

	template <class Op>
	Iterator Find(Op& op, Iterator prev) const
	{
		Iterator curIt = (prev.End()) ? Begin() : ++prev;
		bool bFound = false;

		while(!curIt.End() && !bFound) {
			const tItem * pCur = *curIt;
			if (pCur) {
				bFound = op.Accept(pCur);
			}
			if (!bFound) {
				++curIt;
			}
		}
		return curIt;
	}

	const _Node* FindString(char* str, Iterator prev) const
	{
		return Find(StringFinder_CI(str), prev);
	}

	template <class Op>
	UInt32 CountIf(Op& op) const
	{
		UInt32 count = 0;
		const _Node* pCur = Head();
		while (pCur)
		{
			if (pCur->Item() && op.Accept(pCur->Item()))
				count++;
			pCur = pCur->Next();
		}
		return count;
	}

	void RemoveAll() const {
		_Node* head = Head();
		if (head == nullptr || head->item == nullptr) return;  //Assume empty list if first element has null item 
		_Node* n = head;
		while (n->item != nullptr) {
			n->RemoveMe();
		}
	}

	Item* RemoveNth(SInt32 n)
	{
		Item* pRemoved = NULL;
		if (n == 0) {
			pRemoved =  m_listHead.RemoveMe();
		} else if (n > 0) {
			NodePos nodePos = GetNthNode(n);
			if (nodePos.node && nodePos.index == n) {
				pRemoved = nodePos.node->RemoveMe();
			}
		}
		return pRemoved;
	};

	Item* ReplaceNth(SInt32 n, tItem* item)
	{
		Item* pReplaced = NULL;
		NodePos nodePos = GetNthNode(n);
		if (nodePos.node && nodePos.index == n) {
			pReplaced = nodePos.node->item;
			nodePos.node->item = item;
		}
		return pReplaced;
	}

	template <class Op>
	UInt32 RemoveIf(Op& op) {
		return FreeNodes(const_cast<_Node*>(Head()), op);
	}

	SInt32 IndexOf(_Node* node) {
		SInt32 idx = -1;
		_Node* cur = NULL;
		if (node) {
			for (cur = Head(); cur; cur = cur->Next()) {
				idx++;
				if (cur == node) {
					return idx;
				}
			}
		}
		return -1;
	}

	SInt32 IndexOf(Item* item) {
		SInt32 idx = -1;
		_Node* cur = NULL;
		if (item) {
			for (cur = Head(); cur; cur = cur->Next()) {
				idx++;
				if (cur->item == item) {
					return idx;
				}
			}
		}

		return -1;
	}

	bool Remove(Item* item) {
		_Node* head = Head();
		if (head == nullptr || head->item == nullptr) return false;  //Assume empty list if first element has null item 
		for (_Node* n = head; n != nullptr; n = n->next) {
			if (item == n->item) {
				n->RemoveMe();
				return true;
			}
		}
		return false;
	}

	bool IsEmpty() {
		_Node* head = Head();
		return (head == NULL || head->Item() == NULL);
	}

	static tListBase<Item, _bHeadIsPtr> * Create() {
		tListBase<Item, _bHeadIsPtr>* list = (tListBase<Item, _bHeadIsPtr>*)FormHeap_Allocate(sizeof(tListBase<Item, _bHeadIsPtr>));
		memset(list, 0, sizeof(tListBase<Item, _bHeadIsPtr>));
		return list;
	}
};

template <typename T>
class tList : public tListBase<T, false> {
	//
};

template <typename T>
class tListPtr {
	tListBase<T, true>	*	m_list;

public:
	tListBase<T, true> * operator->() {
		return m_list;
	}

	tListBase<T, true> & operator*() {
		return *m_list;
	}

	bool IsNull() const {
		return m_list != NULL;
	}

	void SetList(tListBase<T, true> * newList) {
		Free();
		m_list = newList;
	}

	void Free() {
		if (!IsNull()) {
			m_list->RemoveAll();
			FormHeap_Free(m_list->Head());
			m_list = NULL;
		}
	}
};

STATIC_ASSERT(sizeof(tList<void *>) == 0x8);
STATIC_ASSERT(sizeof(tListPtr<UInt32>) == 0x4);