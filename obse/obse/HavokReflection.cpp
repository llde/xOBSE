#include "HavokReflection.h"
#include "obse_common/SafeWrite.h"
#include <vector>

void hkClassEnum::Dump(void) const
{
	_MESSAGE("%s", m_name);

	gLog.Indent();

	for(UInt32 i = 0; i < m_numItems; i++)
	{
		const Item	* item = &m_items[i];

		_MESSAGE("%08X: %s", item->m_value, item->m_name);
	}
	
	gLog.Outdent();
}

struct TypeInfo
{
	UInt32		size;
	bool		isComplex;
	const char	* name;
	const char	* printableType;
};

static const TypeInfo	kTypeInfoList[] =
{
	{	0,			false,	"TYPE_VOID",				"void" },
	{	1,			false,	"TYPE_BOOL",				"UInt8" },
	{	1,			false,	"TYPE_CHAR",				"char" },
	{	1,			false,	"TYPE_INT8",				"SInt8" },
	{	1,			false,	"TYPE_UINT8",				"UInt8" },
	{	2,			false,	"TYPE_INT16",				"SInt16" },
	{	2,			false,	"TYPE_UINT16",				"UInt16" },
	{	4,			false,	"TYPE_INT32",				"SInt32" },
	{	4,			false,	"TYPE_UINT32",				"UInt32" },
	{	8,			false,	"TYPE_INT64",				"SInt64" },
	{	8,			false,	"TYPE_UINT64",				"UInt64" },
	{	4,			false,	"TYPE_REAL",				"float" },
	{	4 * 4,		false,	"TYPE_VECTOR4",				"hkVector4" },
	{	4 * 4,		false,	"TYPE_QUATERNION",			"hkQuaternion" },
	{	3 * 4 * 4,	false,	"TYPE_MATRIX3",				"hkMatrix3" },
	{	3 * 4 * 4,	false,	"TYPE_ROTATION",			"hkRotation" },
	{	3 * 4 * 4,	false,	"TYPE_QSTRANSFORM",			"hkQsTransform" },
	{	4 * 4 * 4,	false,	"TYPE_MATRIX4",				"hkMatrix4" },
	{	4 * 4 * 4,	false,	"TYPE_TRANSFORM",			"hkTransform" },
	{	0,			true,	"TYPE_ZERO",				"zero" },				// ???
	{	4,			true,	"TYPE_POINTER",				"ptr" },				// pointer to m_class
	{	4,			false,	"TYPE_FUNCTIONPOINTER",		"fptr *" },				// void *
	{	0,			true,	"TYPE_ARRAY",				"array" },				// hkArray of m_subtypes
	{	0,			true,	"TYPE_INPLACEARRAY",		"inplace_array" },		// hkInplaceArray of m_subtypes
	{	0,			true,	"TYPE_ENUM",				"enum" },
	{	0,			true,	"TYPE_STRUCT",				"struct" },				// m_class (possible c-style array)
	{	0,			true,	"TYPE_SIMPLEARRAY",			"simple_array" },		// c-style array?
	{	0,			false,	"TYPE_HOMOGENEOUSARRAY",	"hkHomogeneousArray" },	// hkClass *, T * data, UInt32 len
	{	0,			false,	"TYPE_VARIANT",				"hkVariant" },			// hkVariant
	{	0,			false,	"TYPE_MAX",					"max" },
};

void hkClassMember::Dump(void) const
{
	_MESSAGE("%s", m_name);
	gLog.Indent();

	if(m_class)	_MESSAGE("class: %s", m_class->m_name);
	if(m_enum)	_MESSAGE("enum: %s", m_enum->m_name);
	_MESSAGE("type: %02X (%s)", m_type, kTypeInfoList[m_type].name);
	_MESSAGE("subtype: %02X", m_subtype);
	_MESSAGE("array size: %04X", m_arraySize);
	_MESSAGE("flags: %04X", m_flags);
	_MESSAGE("offset: %04X", m_offset);

	gLog.Outdent();
}

UInt32 hkClassMember::ElementSize(void) const
{
	UInt32	elementSize = kTypeInfoList[m_type].size;

	if(elementSize) return elementSize;

	switch(m_type)
	{
		case TYPE_VOID:				return 0;
		case TYPE_ZERO:				HALT("TYPE_ZERO");
		case TYPE_ARRAY:
		case TYPE_INPLACEARRAY:
		case TYPE_ENUM:
		case TYPE_STRUCT:
		case TYPE_SIMPLEARRAY:
		case TYPE_HOMOGENEOUSARRAY:
		case TYPE_VARIANT:

		default:
		case TYPE_MAX:		HALT("unhandled m_type in hkClassMember::GetSize");
	}

	return 0;
}

std::string hkClassMember::TypeName(bool useSubtype) const
{
	std::string		result;
	UInt8			typeID = useSubtype ? m_subtype : m_type;
	const TypeInfo	* typeInfo = &kTypeInfoList[typeID];

	if(typeInfo->isComplex)
	{
		switch(typeID)
		{
			case TYPE_ZERO:
				ASSERT(!useSubtype);
				result = TypeName(true);
				break;

			case TYPE_POINTER:
				if(m_class)
					result = std::string(m_class->m_name);
				else if(useSubtype)
					result = "void";
				else
					result = TypeName(true);

				result += " *";
				break;

			case TYPE_ARRAY:
				result = "hkArray <";
				result += useSubtype ? "unknown" : TypeName(true);
				result += ">";
				break;

			case TYPE_INPLACEARRAY:
				result = "hkInPlaceArray <";
				result += useSubtype ? "unknown" : TypeName(true);
				result += ">";
				break;

			case TYPE_ENUM:
				ASSERT(m_enum);
				result = m_enum->m_name;
				break;

			case TYPE_STRUCT:
				ASSERT(m_class);
				result = m_class->m_name;
				break;

			case TYPE_SIMPLEARRAY:
				result = "hkSimpleArray <";
				result += useSubtype ? "unknown" : TypeName(true);
				result += ">";
				break;
		}
	}
	else
	{
		result = typeInfo->printableType;
	}

	return result;
}

void hkClass::Dump(void) const
{
	_MESSAGE("%s", m_name);
	gLog.Indent();

	if(m_parent) _MESSAGE("parent: %s", m_parent->m_name);

	_MESSAGE("size: %08X%s", m_size, m_hasVtable ? " (has vtbl)" : "");

	if(m_numImplementedInterfaces) _MESSAGE("num interfaces: %08X", m_numImplementedInterfaces);

	if(m_numDeclaredEnums)
	{
		_MESSAGE("enums:");
		gLog.Indent();

		for(UInt32 i = 0; i < m_numDeclaredEnums; i++)
		{
			const hkClassEnum	* enumInfo = &m_declaredEnums[i];

			enumInfo->Dump();
		}

		gLog.Outdent();
	}

	if(m_numDeclaredMembers)
	{
		_MESSAGE("members:");
		gLog.Indent();

		for(UInt32 i = 0; i < m_numDeclaredMembers; i++)
		{
			const hkClassMember	* member = &m_declaredMembers[i];

			member->Dump();
		}

		gLog.Outdent();
	}

	gLog.Outdent();
}

/*

// <size>
class theClass : public parentClass
{
public:
	enum // EnumName
	{
		ENUM_A = 0x00000000,
		ENUM_B = 0x00000004,
	};

	// ...
};

*/

void hkClass::DumpAsClass(void) const
{
	_MESSAGE("// %03X", m_size);

	if(m_parent)
		_MESSAGE("class %s : public %s", m_name, m_parent->m_name);
	else
		_MESSAGE("class %s", m_name);

	_MESSAGE("{");

	if(m_numDeclaredEnums || m_numDeclaredMembers)
	{
		_MESSAGE("public:");

		gLog.Indent();

		for(UInt32 i = 0; i < m_numDeclaredEnums; i++)
		{
			const hkClassEnum	* enumInfo = &m_declaredEnums[i];

			_MESSAGE("enum // %s", enumInfo->m_name);
			_MESSAGE("{");

			gLog.Indent();

			if(enumInfo->m_numItems)
			{
				// check if the entire enum is in incremental order
				bool	isIncrementing = true;

				for(UInt32 i = 1; i < enumInfo->m_numItems; i++)
				{
					if(enumInfo->m_items[i].m_value != enumInfo->m_items[i - 1].m_value + 1)
					{
						isIncrementing = false;
						break;
					}
				}

				if(isIncrementing)
				{
					_MESSAGE("%s = 0x%08X,", enumInfo->m_items[0].m_name, enumInfo->m_items[0].m_value);

					for(UInt32 i = 1; i < enumInfo->m_numItems; i++)
					{
						_MESSAGE("%s,", enumInfo->m_items[i].m_name);
					}
				}
				else
				{
					for(UInt32 i = 0; i < enumInfo->m_numItems; i++)
					{
						_MESSAGE("%s = 0x%08X,", enumInfo->m_items[i].m_name, enumInfo->m_items[i].m_value);
					}
				}
			}
			else
			{
				_MESSAGE("// no values");
			}

			gLog.Outdent();

			_MESSAGE("};");
			_MESSAGE("");
		}

		for(UInt32 i = 0; i < m_numDeclaredMembers; i++)
		{
			const hkClassMember	* member = &m_declaredMembers[i];

#if 0
			_MESSAGE("// %s %s %s %04X %04X %04X%s%s%s%s",
				member->m_name, kTypeInfoList[member->m_type].name, kTypeInfoList[member->m_subtype].name,
				member->m_arraySize, member->m_flags, member->m_offset,
				member->m_class ? " class = " : "",
				member->m_class ? member->m_class->m_name : "",
				member->m_enum ? " enum = " : "",
				member->m_enum ? member->m_enum->m_name : "");
			}
#endif

			if(member->m_arraySize)
				_MESSAGE("%s %s[%d]; // %03X", member->TypeName().c_str(), member->m_name, member->m_arraySize, member->m_offset);
			else
				_MESSAGE("%s %s; // %03X", member->TypeName().c_str(), member->m_name, member->m_offset);
		}

		gLog.Outdent();
	}

	_MESSAGE("};");
	_MESSAGE("");
}

/*** hook ***/

std::vector <const hkClass *>	g_hkClassList;

static void __stdcall hkClass_ctor_hook(const hkClass * obj)
{
	g_hkClassList.push_back(obj);
}

static const UInt32 khkClass_ctor_retnAddr = 0x0090D196;

static __declspec(naked) void _hkClass_ctor_hook(void)
{
	__asm {
		pushad

		push	ecx
		call	hkClass_ctor_hook

		popad

		// deleted code
		mov		edx, [esp+8]
		mov		eax, ecx

		jmp		[khkClass_ctor_retnAddr]
	}
}

void HavokReflection_Init(void)
{
	WriteRelJump(0x0090D190, (UInt32)&_hkClass_ctor_hook);
}

void HavokReflection_Dump(void)
{
	for(UInt32 i = 0; i < g_hkClassList.size(); i++)
	{
		const hkClass	* obj = g_hkClassList[i];

//		obj->Dump();
		obj->DumpAsClass();
	}
}
