#pragma once

#include <string>

// havok continues to be really nice to us
// all the interesting classes have reflection information
// this makes it even easier to bridge the gap between the public 550 sdk and the version used in oblivion

class hkClass;

class hkClassEnum
{
public:
	struct Item
	{
		int			m_value;
		const char	* m_name;
	};

	const char	* m_name;
	const Item	* m_items;
	UInt32		m_numItems;
	// no m_attributes or m_flags

	void	Dump(void) const;
};

class hkClassMember
{
public:
	enum
	{
		TYPE_VOID = 0,
		TYPE_BOOL,
		TYPE_CHAR,
		TYPE_INT8,
		TYPE_UINT8,
		TYPE_INT16,
		TYPE_UINT16,
		TYPE_INT32,
		TYPE_UINT32,		// 08
		TYPE_INT64,
		TYPE_UINT64,
		TYPE_REAL,
		TYPE_VECTOR4,
		TYPE_QUATERNION,
		TYPE_MATRIX3,
		TYPE_ROTATION,
		TYPE_QSTRANSFORM,	// 10
		TYPE_MATRIX4,
		TYPE_TRANSFORM,
		TYPE_ZERO,			// 13
		TYPE_POINTER,		// 14
		TYPE_FUNCTIONPOINTER,
		TYPE_ARRAY,			// 16
		TYPE_INPLACEARRAY,
		TYPE_ENUM,			// 18
		TYPE_STRUCT,
		TYPE_SIMPLEARRAY,
		TYPE_HOMOGENEOUSARRAY,
		TYPE_VARIANT,
		TYPE_MAX,			// 1D
	};

	const char			* m_name;
	const hkClass		* m_class;
	const hkClassEnum	* m_enum;
	UInt8				m_type;
	UInt8				m_subtype;
	UInt16				m_arraySize;
	UInt16				m_flags;
	UInt16				m_offset;

	// no m_attributes

	void	Dump(void) const;

	UInt32	GetSize(void) const;
	bool	IsArray(void) const		{ return !m_arraySize; }
	UInt32	NumElements(void) const	{ return m_arraySize ? m_arraySize : 1; }
	UInt32	ElementSize(void) const;

	std::string	TypeName(bool useSubtype = false) const;
};

class hkClass
{
public:
	const char			* m_name;
	const hkClass		* m_parent;
	UInt32				m_size;
	UInt32				m_numImplementedInterfaces;
	const hkClassEnum	* m_declaredEnums;
	UInt32				m_numDeclaredEnums;
	const hkClassMember	* m_declaredMembers;
	UInt32				m_numDeclaredMembers;
	UInt32				m_hasVtable;

	void	Dump(void) const;
	void	DumpAsClass(void) const;
};

struct hkVariant
{
	void	* m_object;
	hkClass	* m_class;
};

void HavokReflection_Init(void);
void HavokReflection_Dump(void);
