#pragma once

// 0C
template <typename T>
class hkArray
{
public:
	enum
	{
		kFlag_CapacityMask =	0x3FFFFFFF,
		kFlag_NoDealloc =		0x80000000,	// m_data not owned by us
		kFlag_Locked =			0x40000000,	// will never be destroyed
	};

	T		* m_data;			// 00
	UInt32	m_size;				// 04
	UInt32	m_capacityAndFlags;	// 08
};

// 0C + sizeof(T) * N
template <typename T, UInt32 N>
class hkInplaceArray : public hkArray <T>
{
public:
	hkInplaceArray();
	~hkInplaceArray();

	T	m_storage[N];	// 0C
};

// 10 - always 16-byte aligned
__declspec(align(16)) struct hkVector4
{
	float	x;
	float	y;
	float	z;
	float	w;
};

// 10
struct hkQuaternion
{
	hkVector4	vec;
};

// 30
struct hkMatrix3
{
	hkVector4	col0;
	hkVector4	col1;
	hkVector4	col2;
};

// 40
struct hkMatrix4
{
	hkVector4	col0;
	hkVector4	col1;
	hkVector4	col2;
	hkVector4	col3;
};

// 30
struct hkRotation : public hkMatrix3
{
	//
};

// 40
struct hkTransform
{
	hkRotation	m_rotation;
	hkVector4	m_translation;
};

// 30
struct hkQsTransform
{
	hkVector4	m_translation;
	hkVector4	m_rotation;
	hkVector4	m_scale;
};

// 20
struct hkContactPoint
{
	hkVector4	m_position;
	hkVector4	m_separatingNormal;	// distance stored in w
};

typedef UInt32	hkShapeKey;
