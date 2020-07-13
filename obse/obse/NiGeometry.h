#pragma once

#include "obse/NiNodes.h"

/*** geometry
 *	NiAdditionalGeometryData
 *		BSPackedAdditionalGeometryData
 *	NiGeometryData
 *		NiLinesData
 *		NiTriBasedGeomData
 *			NiTriStripsData
 *				NiTriStripsDynamicData
 *			NiTriShapeData
 *				NiScreenElementsData
 *				NiTriShapeDynamicData
 *				NiScreenGeometryData
 *		NiParticlesData						5C
 *			NiPSysData						68
 *				NiMeshPSysData				84
 *			NiParticleMeshesData			64
 *	NiSkinInstance
 *	NiSkinPartition
 *	NiSkinData
 */

class NiSkinPartition;
class NiSkinData;

// 2C
class NiAdditionalGeometryData : public NiObject
{
public:
	NiAdditionalGeometryData();
	~NiAdditionalGeometryData();

	// 10
	class NiAGDDataBlock
	{
	public:
		NiAGDDataBlock();
		~NiAGDDataBlock();

		virtual void *	Allocate(UInt32 size);
		virtual void	Fn_01(UInt32 arg0, UInt32 arg1);	// derived calls NiRenderer::Unk_3D
		virtual void	Fn_02(UInt32 arg0, UInt32 arg1);	// derived calls NiRenderer::Unk_3E
		virtual void	Free(void * buf);

//		void	** _vtbl;			// 00
		UInt32	m_uiDataBlockSize;	// 04
		void	* m_pucDataBlock;	// 08
		UInt8	dataIsFilled;		// 0C - m_pucDataBlock only free'd in Free if this is nonzero
		UInt8	unk0D;				// 0D - used by derived classes
		UInt8	pad0E[2];			// 0E
	};

	// 1C
	class NiAdditionalGeoStream
	{
		enum
		{
			AGD_NITYPE_INVALID = 0,
			AGD_NITYPE_FLOAT1,
			AGD_NITYPE_FLOAT2,
			AGD_NITYPE_FLOAT3,
			AGD_NITYPE_FLOAT4,
			AGD_NITYPE_LONG1,
			AGD_NITYPE_LONG2,
			AGD_NITYPE_LONG3,
			AGD_NITYPE_LONG4,
			AGD_NITYPE_ULONG1,
			AGD_NITYPE_ULONG2,
			AGD_NITYPE_ULONG3,
			AGD_NITYPE_ULONG4,
			AGD_NITYPE_SHORT1,
			AGD_NITYPE_SHORT2,
			AGD_NITYPE_SHORT3,
			AGD_NITYPE_SHORT4,
			AGD_NITYPE_USHORT1,
			AGD_NITYPE_USHORT2,
			AGD_NITYPE_USHORT3,
			AGD_NITYPE_USHORT4,
			AGD_NITYPE_BYTE1,
			AGD_NITYPE_BYTE2,
			AGD_NITYPE_BYTE3,
			AGD_NITYPE_BYTE4,
			AGD_NITYPE_UBYTE1,
			AGD_NITYPE_UBYTE2,
			AGD_NITYPE_UBYTE3,
			AGD_NITYPE_UBYTE4,
			AGD_NITYPE_BLEND1,
			AGD_NITYPE_BLEND2,
			AGD_NITYPE_BLEND3,
			AGD_NITYPE_BLEND4,
			AGD_NITYPE_COUNT,
		};

		enum
		{
			kFlag_Keep =					1 << 0,

			kFlag_ConsistencyShift =		1,
			kFlag_ConsistencyMask =			3 << kFlag_ConsistencyShift,
			kFlag_Consistency_Mutable =		1 << kFlag_ConsistencyShift,
			kFlag_Consistency_Static =		2 << kFlag_ConsistencyShift,
			kFlag_Consistency_Volatile =	3 << kFlag_ConsistencyShift,
		};

		UInt8	flags;				// 00
		UInt8	pad01[3];			// 01
		UInt32	m_uiType;			// 04
		UInt32	m_uiUnitSize;		// 08
		UInt32	m_uiTotalSize;		// 0C
		UInt32	m_uiStride;			// 10
		UInt32	m_uiBlockIndex;		// 14
		UInt32	m_uiBlockOffset;	// 18
	};

	virtual bool				Unk_13(void);
	virtual NiAGDDataBlock *	AllocateBlock(void);

	UInt32	unk08;								// 08
	UInt16	m_usVertexCount;					// 0C
	UInt8	pad0E[2];							// 0E
	UInt32	m_uiDataStreamCount;				// 10
	NiAdditionalGeoStream	* streams;			// 14
	UInt32	unk18;								// 18
	NiTArray <NiAGDDataBlock *>	m_aDataBlocks;	// 1C
};

STATIC_ASSERT(sizeof(NiAdditionalGeometryData::NiAGDDataBlock) == 0x10);

// 34
class BSPackedAdditionalGeometryData : public NiAdditionalGeometryData
{
public:
	BSPackedAdditionalGeometryData();
	~BSPackedAdditionalGeometryData();

	// 10
	class NiBSPackedAGDDataBlock : public NiAGDDataBlock
	{
	public:
		NiBSPackedAGDDataBlock();
		~NiBSPackedAGDDataBlock();
	};

	UInt32	m_uiShaderDeclarationIndex;	// 2C
	UInt32	unk30;						// 30
};

STATIC_ASSERT(sizeof(BSPackedAdditionalGeometryData::NiBSPackedAGDDataBlock) == 0x10);

// 40
class NiGeometryData : public NiObject
{
public:
	NiGeometryData();
	~NiGeometryData();

	virtual void	SetNumVertices(UInt32 arg);
	virtual UInt16	GetNumVertices(void);
	virtual void	UpdateNormals(void) = 0;

	enum
	{
		kFormat_TextureSetCountShift =	0,
		kFormat_TextureSetCountMask =	0x3F << kFormat_TextureSetCountShift,

		kFormat_NBTShift =		12,
		kFormat_NBTMask =		0x0F << kFormat_NBTShift,
	};

	UInt16						m_usVertices;				// 08
	UInt16						serial;						// 0A - serial number incremented in ctor
	NiSphere					m_kBound;					// 0C
	void						* m_pkVertex;				// 1C
	void						* m_pkNormal;				// 20
	void						* m_pkColor;				// 24
	void						* m_pkTexture;				// 28
	UInt16						format;						// 2C
	UInt16						m_usDirtyFlags;				// 2E
	UInt8						m_ucKeepFlags;				// 30
	UInt8						m_ucCompressFlags;			// 31
	UInt8						pad32[2];					// 32
	NiAdditionalGeometryData	* m_spAdditionalGeomData;	// 34
	UInt32						unk38;						// 38
	UInt8						unk3C;						// 3C
	UInt8						unk3D;						// 3D
	UInt8						pad3E[2];					// 3E
};

// 44
class NiLinesData : public NiGeometryData
{
public:
	NiLinesData();
	~NiLinesData();

	void	* m_pkFlags;	// 40
};

// 44
class NiTriBasedGeomData : public NiGeometryData
{
public:
	NiTriBasedGeomData();
	~NiTriBasedGeomData();

	virtual void	SetNumTris(UInt32 arg);
	virtual UInt16	GetNumTris(void);
	virtual void	GetTriIndices(UInt32 idx, UInt16 * a, UInt16 * b, UInt16 * c);
	virtual void	GetStripData(UInt16 * numStrips, UInt16 ** stripLengths, UInt16 ** stripLists, UInt32 * numStripsAndTris);

	UInt16	m_usTriangles;	// 40
	UInt8	pad42[2];		// 42
};

// 50
class NiTriStripsData : public NiTriBasedGeomData
{
public:
	NiTriStripsData();
	~NiTriStripsData();

	UInt16	m_usStrips;				// 44
	UInt8	pad46[2];				// 46
	UInt16	* m_pusStripLengths;	// 48
	UInt16	* m_pusStripLists;		// 4C
};

// 54
class NiTriStripsDynamicData : public NiTriStripsData
{
public:
	NiTriStripsDynamicData();
	~NiTriStripsDynamicData();

	// single strip of triangles

	UInt16	m_usActiveVertices;		// 50
	UInt16	m_usActiveTriangles;	// 52
};

// 58
class NiTriShapeData : public NiTriBasedGeomData
{
public:
	NiTriShapeData();
	~NiTriShapeData();

	// trilist

	UInt32	m_uiTriListLength;	// 44
	UInt16	* m_pusTriList;		// 48
	UInt32	unk4C;				// 4C
	UInt16	unk50;				// 50
	UInt8	pad52[2];			// 52
	UInt32	unk54;				// 54
};

// 70
class NiScreenElementsData : public NiTriShapeData
{
public:
	NiScreenElementsData();
	~NiScreenElementsData();

	void	* m_akPolygon;		// 58
	void	* m_ausPIndexer;	// 5C
	UInt16	m_usMaxPQuantity;	// 60
	UInt16	m_usPGrowBy;		// 62
	UInt16	m_usPQuantity;		// 64
	UInt16	m_usMaxVQuantity;	// 66
	UInt16	m_usVGrowBy;		// 68
	UInt16	m_usMaxIQuantity;	// 6A
	UInt16	m_usIGrowBy;		// 6C
	UInt8	pad6E[2];			// 6E
};

// 5C
class NiTriShapeDynamicData : public NiTriShapeData
{
public:
	NiTriShapeDynamicData();
	~NiTriShapeDynamicData();

	UInt16	m_usActiveVertices;		// 58
	UInt16	m_usActiveTriangles;	// 5A
};

// 70
class NiScreenGeometryData : public NiTriShapeData
{
public:
	NiScreenGeometryData();
	~NiScreenGeometryData();

	class ScreenElement
	{
	public:
		ScreenElement();
		~ScreenElement();
	};

	UInt8	m_bPixelAccurate;	// 58
	UInt8	unk59;				// 59
	UInt8	pad5A[2];			// 5A
	UInt32	unk5C;				// 5C
	NiTArray <ScreenElement *>	elements;	// 60
};

// 5C
class NiParticlesData : public NiGeometryData
{
public:
	NiParticlesData();
	~NiParticlesData();

	virtual void	Remove(UInt32 idx);	// remove a particle from the array

	UInt32			unk40;					// 40
	float			* m_pfRadii;			// 44
	UInt16			m_usActive;				// 48
	UInt8			pad4A[2];				// 4A
	float			* m_pfSizes;			// 4C
	NiQuaternion	* m_pkRotations;		// 50
	float			* m_pfRotationAngles;	// 54
	NiVector3		* m_pkRotationAxes;		// 58
};

// 68
class NiPSysData : public NiParticlesData
{
public:
	NiPSysData();
	~NiPSysData();

	virtual void	Fn_17(void);

	void	* m_pkParticleInfo;
	void	* m_pfRotationSpeeds;
	UInt16	numAddedParticles;
	UInt16	addedParticlesBase;
};

// 84
class NiMeshPSysData : public NiPSysData
{
public:
	NiMeshPSysData();
	~NiMeshPSysData();

	NiObject	* unk68;	// 68
	UInt8		unk6C;		// 6C
	UInt8		pad6D[3];	// 6D
	UInt32		unk70;		// 70
	NiTArray <NiTArray <NiAVObject *> *>	meshes;	// 74
};

// 64
class NiParticleMeshesData : public NiParticlesData
{
public:
	NiParticleMeshesData();
	~NiParticleMeshesData();

	NiObject	* unk5C;	// 5C
	UInt32		unk60;		// 60
};

// 2C
class NiSkinInstance : public NiObject
{
public:
	NiSkinInstance();
	~NiSkinInstance();

	NiSkinData		* data;			// 08
	NiSkinPartition	* partition;	// 0C
	NiObjectNET		* rootParent;	// 10
	NiObjectNET		** bones;		// 14
	UInt32			serial;			// 18
	UInt32			unk1C;			// 1C
	UInt32			unk20;			// 20
	UInt32			unk24;			// 24
};

// 10
class NiSkinPartition : public NiObject
{
public:
	NiSkinPartition();
	~NiSkinPartition();

	// 2C
	class Partition
	{
	public:
		Partition();
		~Partition();

		virtual void	Destroy(bool freeThis);

//		void	** _vtbl;		// 00
		UInt16	* bones;		// 04
		float	* weights;		// 08
		UInt16	* vertexMap;	// 0C
		UInt8	* boneIndices;	// 10
		UInt16	* stripData;	// 14
		UInt16	* stripLengths;	// 18
		UInt16	numVerts;		// 1C
		UInt16	numTris;		// 1E
		UInt16	numBones;		// 20
		UInt16	numStrips;		// 22
		UInt16	weightsPerVtx;	// 24
		UInt8	pad26[2];		// 26
		UInt32	unk28;			// 28
	};

	UInt32		numPartitions;	// 08
	Partition	* partitions;	// 0C
};

// 48
class NiSkinData : public NiObject
{
public:
	NiSkinData();
	~NiSkinData();

	// 08
	class Weight
	{
		UInt16	idx;		// 00
		UInt8	pad02[2];	// 02
		float	weight;		// 04
	};

	// 4C
	class Bone
	{
	public:
		Bone();
		~Bone();

		NiTransform	transform;		// 00
		NiSphere	boundingSphere;	// 34
		Weight		* weights;		// 44
		UInt32		numWeights;		// 48
	};

	NiObject	* skinPartition;	// 08
	NiTransform	transform;			// 0C
	UInt32		boneCount;			// 40
	Bone		* bones;			// 44
};
