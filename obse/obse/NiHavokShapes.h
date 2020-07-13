#pragma once

/**** havok shapes
 *	bhkShape
 *		bhkTransformShape
 *		bhkSphereRepShape
 *			bhkConvexShape
 *				bhkSphereShape
 *				bhkCapsuleShape
 *				bhkBoxShape
 *				bhkTriangleShape
 *				bhkCylinderShape
 *				bhkConvexVerticesShape
 *					bhkCharControllerShape
 *				bhkConvexTransformShape
 *				bhkConvexSweepShape
 *			bhkMultiSphereShape
 *		bhkBvTreeShape
 *			bhkTriSampledHeightFieldBvTreeShape
 *			bhkMoppBvTreeShape
 *		bhkShapeCollection
 *			bhkListShape
 *			bhkPackedNiTriStripsShape
 *			bhkNiTriStripsShape
 *		bhkHeightFieldShape
 *			bhkPlaneShape
 */

#include "obse/NiHavok.h"

// 14
class bhkShape : public bhkSerializable
{
public:
	enum
	{
		kMaterial_Stone = 0,
		kMaterial_Cloth,
		kMaterial_Dirt,
		kMaterial_Glass,
		kMaterial_Grass,
		kMaterial_Metal,
		kMaterial_Organic,
		kMaterial_Skin,
		kMaterial_Water,
		kMaterial_Wood,
		kMaterial_HeavyStone,
		kMaterial_HeavyMetal,
		kMaterial_HeavyWood,
		kMaterial_Chain,
		kMaterial_Snow,
		kMaterial_StoneStairs,
		kMaterial_ClothStairs,
		kMaterial_DirtStairs,
		kMaterial_GlassStairs,
		kMaterial_GrassStairs,
		kMaterial_MetalStairs,
		kMaterial_OrganicStairs,
		kMaterial_SkinStairs,
		kMaterial_WaterStairs,
		kMaterial_WoodStairs,
		kMaterial_HeavyStoneStairs,
		kMaterial_HeavyMetalStairs,
		kMaterial_HeavyWoodStairs,
		kMaterial_ChainStairs,
		kMaterial_SnowStairs,
		kMaterial_Elevator,
		kMaterial_Default,
	};

	bhkShape();
	~bhkShape();

	virtual void	Unk_20(bhkShape * dstShape, void * arg);	// copy this to dst?
	virtual bool	Unk_21(void * arg);
	virtual UInt32	Unk_22(void);
	virtual bool	Unk_23(float * arg);
	virtual void	Unk_24(void * arg);
	virtual void	Unk_25(void * arg0, void * arg1, void * arg2);
	virtual void	Unk_26(void * arg);	// builds NiMaterialProperty, outputs in arg?

	UInt32	material;	// 10
};

// 14
class bhkTransformShape : public bhkShape
{
public:
	bhkTransformShape();
	~bhkTransformShape();
};

// 14
class bhkSphereRepShape : public bhkShape
{
public:
	bhkSphereRepShape();
	~bhkSphereRepShape();
};

// 14
class bhkConvexShape : public bhkSphereRepShape
{
public:
	bhkConvexShape();
	~bhkConvexShape();
};

// 14
class bhkSphereShape : public bhkConvexShape
{
public:
	bhkSphereShape();
	~bhkSphereShape();
};

// 14
class bhkCapsuleShape : public bhkConvexShape
{
public:
	bhkCapsuleShape();
	~bhkCapsuleShape();
};

// 14
class bhkBoxShape : public bhkConvexShape
{
public:
	bhkBoxShape();
	~bhkBoxShape();
};

// 14
class bhkTriangleShape : public bhkConvexShape
{
public:
	bhkTriangleShape();
	~bhkTriangleShape();
};

// 14
class bhkCylinderShape : public bhkConvexShape
{
public:
	bhkCylinderShape();
	~bhkCylinderShape();
};

// 14
class bhkConvexVerticesShape : public bhkConvexShape
{
public:
	bhkConvexVerticesShape();
	~bhkConvexVerticesShape();
};

// 14
class bhkCharControllerShape : public bhkConvexVerticesShape
{
public:
	bhkCharControllerShape();
	~bhkCharControllerShape();
};

// 14
class bhkConvexTransformShape : public bhkConvexShape
{
public:
	bhkConvexTransformShape();
	~bhkConvexTransformShape();
};

// 14
class bhkConvexSweepShape : public bhkConvexShape
{
public:
	bhkConvexSweepShape();
	~bhkConvexSweepShape();
};

// 14
class bhkMultiSphereShape : public bhkSphereRepShape
{
public:
	bhkMultiSphereShape();
	~bhkMultiSphereShape();
};

// 14
class bhkBvTreeShape : public bhkShape
{
public:
	bhkBvTreeShape();
	~bhkBvTreeShape();
};

// 14
class bhkTriSampledHeightFieldBvTreeShape : public bhkBvTreeShape
{
public:
	bhkTriSampledHeightFieldBvTreeShape();
	~bhkTriSampledHeightFieldBvTreeShape();
};

// 14
class bhkMoppBvTreeShape : public bhkBvTreeShape
{
public:
	bhkMoppBvTreeShape();
	~bhkMoppBvTreeShape();
};

// 14
class bhkShapeCollection : public bhkShape
{
public:
	bhkShapeCollection();
	~bhkShapeCollection();

	virtual void *	Unk_27(void * arg) = 0;
};

// 14
class bhkListShape : public bhkShapeCollection
{
public:
	bhkListShape();
	~bhkListShape();
};

// 14
class bhkPackedNiTriStripsShape : public bhkShapeCollection
{
public:
	bhkPackedNiTriStripsShape();
	~bhkPackedNiTriStripsShape();
};

// 14
class bhkNiTriStripsShape : public bhkShapeCollection
{
public:
	bhkNiTriStripsShape();
	~bhkNiTriStripsShape();
};

// 14
class bhkHeightFieldShape : public bhkShape
{
public:
	bhkHeightFieldShape();
	~bhkHeightFieldShape();
};

// 14
class bhkPlaneShape : public bhkHeightFieldShape
{
public:
	bhkPlaneShape();
	~bhkPlaneShape();
};
