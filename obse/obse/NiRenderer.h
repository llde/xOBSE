#pragma once

#include "obse/NiNodes.h"
#include "obse/NiProperties.h"
#include <d3d9.h>

class NiGeometryGroup;
class NiDX9RenderState;			// 0x1148
	// Fn14 = DeleteMaterial
class NiDX9VertexBufferManager;	// 0x100
class NiDX9IndexBufferManager;	// 0x4C
class NiDX9TextureManager;		// 0x10
class NiDX9LightManager;		// 0x240
class NiGeometryData;
class NiMaterialProperty;
class NiDynamicEffect;
class NiDX92DBufferData;
	// +0C = IDirect3DSurface9 *
class NiSkinInstance;
class NiSkinPartition;
class NiTriShape;
class NiTriStrips;
class NiParticles;
class NiLines;
class NiDX9RenderedTextureData;
class NiDX9RenderedCubeMapData;
class NiDX9DynamicTextureData;
class NiD3DShaderInterface;
class NiAccumulator;
class NiGeometryGroupManager;

// 14
class Ni2DBuffer : public NiObject
{
public:
	Ni2DBuffer();
	~Ni2DBuffer();

	UInt32				width;	// 008
	UInt32				height;	// 00C
	NiDX92DBufferData	* data;	// 010 - possibly NiDX9ImplicitBufferData but unlikely
};

// 18
class NiDepthStencilBuffer : public Ni2DBuffer
{
public:
	NiDepthStencilBuffer();
	~NiDepthStencilBuffer();

	UInt32	unk014;	// 014
};

// 24
class NiRenderTargetGroup : public NiObject
{
public:
	NiRenderTargetGroup();
	~NiRenderTargetGroup();

	virtual UInt32			GetTargetWidth(UInt32 idx);
	virtual UInt32			GetTargetHeight(UInt32 idx);
	virtual UInt32			GetDepthWidth(void);
	virtual UInt32			GetDepthHeight(void);
	virtual UInt32			GetTarget_Fn3(UInt32 idx);
	virtual UInt32			GetDepth_Fn3(UInt32 idx);
	virtual bool			SetTarget(Ni2DBuffer * buf, UInt32 idx);
	virtual bool			SetDepth(NiDepthStencilBuffer * buf);
	virtual Ni2DBuffer *	GetTarget(UInt32 idx);
	virtual NiDepthStencilBuffer *	GetDepth(void);
	virtual void *			GetUnk20(void);
	virtual void			SetUnk20(void * arg);	// destroys existing object
	virtual UInt32			GetTargetData(UInt32 idx);
	virtual UInt32			GetDepthData(void);

	Ni2DBuffer				* targets[4];	// 008
	UInt32					numTargets;		// 018
	NiDepthStencilBuffer	* depth;		// 01C
	void					* unk020;		// 020
};

// 210
class NiRenderer : public NiObject
{
public:
	NiRenderer();
	~NiRenderer();

	// omg so many virtual functions
	// comments say what the dx9 renderer does

	enum
	{
		kCopyFilterMode_None = 0,
		kCopyFilterMode_Point,
		kCopyFilterMode_Linear,
	};

	virtual bool			Unk_13(UInt32 arg);
	virtual const char *	GetRendererDesc(void) = 0;	// builds a string "<adapter id> (<dev type>-<dev flags>)"
	virtual UInt32			Unk_15(void) = 0;			// return unk5E0
	virtual void			SetClearDepth(float arg) = 0;
	virtual float			GetClearDepth(void) = 0;
	virtual void			SetClearColor4(float * src) = 0;
	virtual void			SetClearColor3(float * src) = 0;
	virtual void			GetClearColor(float * dst) = 0;
	virtual void			SetClearStencil(UInt32 src) = 0;
	virtual UInt32			GetClearStencil(void) = 0;
	virtual bool			Unk_1D(NiRenderTargetGroup * renderTarget) = 0;
	virtual void			Unk_1E(void) = 0;
	virtual NiRenderTargetGroup *	GetDefaultRT(void) = 0;	// get back buffer rt?
	virtual NiRenderTargetGroup *	GetCurrentRT(void) = 0;	// get currentRTGroup
	virtual UInt8			Unk_21(void) = 0;			// get unkA90 (u8)
	virtual void			Unk_22(void) = 0;			// passthrough to currentRTGroup2->Fn1D
	virtual void			Unk_23(void) = 0;			// currentRTGroup2->Fn1C(0);
	virtual void			Unk_24(UInt32 arg) = 0;		// sub_773960(arg, &unk6F4);
	virtual void *			Unk_25(void * arg0, UInt32 arg1, UInt32 arg2) = 0;	// something with pixel format conversion
	virtual void *			Unk_26(void * arg);			// Unk_25(arg, 0x20, 8) - something with pixel format conversion
	virtual UInt32			Unk_27(void) = 0;			// get unk894 - current/max num render targets?
	virtual UInt32			Unk_28(void) = 0;			// get unk898
	virtual void			Unk_29(void) = 0;			// passthrough to textureManager->Fn03
	virtual bool			Unk_2A(UInt32 arg) = 0;		// set dword_B42050, return true
	virtual UInt32			Unk_2B(void) = 0;			// get dword_B42050
	virtual bool			Unk_2C(UInt32 arg0, UInt32 arg1, UInt32 arg2, UInt32 arg3);
	virtual void			DeleteGeometry(NiGeometryData * geo) = 0;
	virtual void			DeleteMaterial(NiMaterialProperty * material) = 0;			// passthrough to renderState->DeleteMaterial
	virtual void			DeleteEffect(NiDynamicEffect * effect) = 0;
	virtual void			Unk_30(void) = 0;
	virtual void			DeleteSkinPartition(NiSkinPartition * skinPartition) = 0;
	virtual void			DeleteSkinInstance(NiSkinInstance * skinInstance) = 0;
	virtual void			Unk_33(void) = 0;
	virtual bool			Unk_34(void) = 0;
	virtual void			Unk_35(void) = 0;
	virtual bool			FastCopy(void * src, void * dst, RECT * srcRect, SInt32 xOffset, SInt32 yOffset) = 0;
	virtual bool			Copy(void * src, void * dst, RECT * srcRect, RECT * dstRect, UInt32 filterMode) = 0;
	virtual void			Unk_38(void) = 0;			// passthrough to renderState->Fn12
	virtual bool			Unk_39(void * arg) = 0;		// renderState->Fn13(arg);
	virtual void			Unk_3A(void) = 0;			// passthrough to renderState->Fn0F
	virtual void			Unk_3B(float arg) = 0;		// renderState->Fn10(arg);
	virtual void *			Unk_3C(UInt32 size) = 0;	// FormHeap_Allocate(size);
	virtual void			Unk_3D(UInt32 arg0, UInt32 arg1) = 0;
	virtual void			Unk_3E(UInt32 arg0, UInt32 arg1) = 0;
	virtual void			Unk_3F(void * buf) = 0;		// FormHeap_Free(buf);
	virtual bool			Unk_40(UInt32 arg0);
	virtual void			InitTexture(NiSourceTexture * texture) = 0;			// locks cs2
	virtual bool			InitRenderedTexture(NiRenderedTexture * arg) = 0;	// make a NiDX9RenderedTextureData, store it in unk8C0
	virtual bool			InitCubeMap(NiSourceCubeMap * arg) = 0;				// make a NiDX9SourceCubeMapData
	virtual bool			InitRenderedCubeMap(NiRenderedCubeMap * arg) = 0;	// make a NiDX9RenderedCubeMapData, store it in unk8D0
	virtual bool			InitDynamicTexture(void * arg) = 0;					// make a NiDX9DynamicTextureData, store it in unk8E0
	virtual void			Unk_46(void * arg) = 0;		// nop
	virtual bool			InitDepthStencilBuffer(NiDepthStencilBuffer * arg, void * textureFormat) = 0;
	virtual void			LockDynamicTexture(void * arg0, void * arg1) = 0;
	virtual void			UnLockDynamicTexture(void * arg) = 0;
	virtual void			Unk_4A(void);				// nop - post-enter cs0?
	virtual void			Unk_4B(void);				// nop - pre-leave cs0?
	virtual bool			BeginScene(void) = 0;
	virtual bool			EndScene(void) = 0;
	virtual void			Unk_4E(void) = 0;			// iterate renderTargets list
	virtual void			Clear(float * rect, UInt32 flags) = 0;
	virtual void			SetupCamera(NiVector3 * pos, NiVector3 * at, NiVector3 * up, NiVector3 * right, NiFrustum * frustum, float * viewport) = 0;
	virtual void			Unk_51(void) = 0;			// reset transforms?
	virtual bool			BeginUsingRenderTargetGroup(NiRenderTargetGroup * renderTarget, UInt32 clearFlags) = 0;
	virtual bool			Unk_53(void) = 0;			// something for each render target (adds to renderTargets list, not much else)
	virtual void			Unk_54(UInt32 arg0, UInt32 arg1);	// set unk61C, unk620
	virtual void			Unk_55(void);
	virtual void			Unk_56(void * arg);
	virtual void			Unk_57(void * arg);
	virtual void			RenderTriShape(NiTriShape * obj) = 0;
	virtual void			RenderTriStrips(NiTriStrips * obj) = 0;		// points to the same function as above
	virtual void			RenderTriShape2(NiTriShape * obj) = 0;		// calls DrawIndexedPrimitive
	virtual void			RenderTriStrips2(NiTriStrips * obj) = 0;	// calls DIP/DP
	virtual void			RenderParticles(NiParticles * obj) = 0;
	virtual void			RenderLines(NiLines * obj) = 0;				// calls DIP/SetStreamSource/SetIndices
	virtual void			Unk_5E(void) = 0;			// stuff with properties

	// 080
	struct CriticalSection
	{
		CRITICAL_SECTION	cs;	// 000
		UInt32	pad018[(0x078 - 0x018) >> 2];	// 018
		UInt32	curThread;	// 078
		UInt32	entryCount;	// 07C
	};

	NiAccumulator			* accumulator;			// 008
	NiPropertyState			* propertyState;		// 00C
	NiDynamicEffectState	* dynamicEffectState;	// 010
	UInt32	pad014[(0x080 - 0x014) >> 2];	// 00C
	CriticalSection	unk080;	// 080
	CriticalSection	unk100;	// 100
	CriticalSection	unk180;	// 180
	UInt32	unk200;			// 200
	UInt32	unk204;			// 204
	UInt32	unk208;			// 208
	UInt8	unk20C;			// 20C
	UInt8	unk20D;			// 20D
	UInt8	pad20E[2];		// 20E
};

class LightEntry
{
public:
	LightEntry();
	~LightEntry();
};

// 10
class NiDX9LightManager : public NiTPointerMap <LightEntry *>
{
public:
	NiDX9LightManager();
	~NiDX9LightManager();
};

// B00
class NiDX9Renderer : public NiRenderer
{
public:
	NiDX9Renderer();
	~NiDX9Renderer();

	class PrePackObject;

	// 58
	class Unk6F4
	{
	public:
		UInt32	unk00;
		UInt32	unk04;
		UInt32	unk08;
		UInt32	unk0C;
		UInt32	unk10;
		UInt32	unk14;
		UInt32	unk18;
		UInt32	unk1C;
		UInt32	unk20;
		UInt32	unk24;
		UInt32	unk28;
		UInt32	unk2C;
		UInt32	unk30;
		UInt32	unk34;
		UInt32	unk38;
		UInt32	unk3C;
		UInt32	unk40;
		UInt32	unk44;
		UInt32	unk48;
		UInt32	unk4C;
		UInt32	unk50;
		UInt32	unk54;
	};

	typedef bool (* CallbackA98)(bool arg0, void * arg1);
	typedef bool (* LostDeviceCallback)(void * arg);

	// these may be wrong
	virtual bool			DeleteRenderedCubeMap(NiRenderedCubeMap * arg);	// unk8D0 - may also handle textures?
	virtual bool			DeleteTexture(NiTexture * arg);					// unk8C0
	virtual bool			DeleteDynamicTexture(UInt32 arg);				// unk8E0

	// 210
	UInt32		pad210[(0x280 - 0x210) >> 2];	// 210

	IDirect3DDevice9	* device;			// 280
	D3DCAPS9	caps;						// 284
	HANDLE		deviceWindow;				// 3B4
	HANDLE		focusWindow;				// 3B8
	char		rendererInfo[0x200];		// 3BC
	UInt32		adapterIdx;					// 5BC
	UInt32		d3dDevType;					// 5C0 - D3DDEVTYPE
	UInt32		d3dDevFlags;				// 5C4 - D3DCREATE
	UInt8		softwareVertexProcessing;	// 5C8 - !D3DCREATE_HARDWARE_VERTEXPROCESSING
	UInt8		mixedVertexProcessing;		// 5C9 - D3DCREATE_MIXED_VERTEXPROCESSING
	UInt8		pad5CA[2];					// 5CA
	UInt32		pad5CC;						// 5CC - may point to driver/adapter info
	
	void		* unk5D0;					// 5D0
	UInt32		clearColor;					// 5D4
	float		clearDepth;					// 5D8
	UInt32		clearStencil;				// 5DC
	UInt32		unk5E0;						// 5E0
	UInt32		pad5E4[(0x604 - 0x5E4) >> 2];	// 5E4
	
	NiTPointerMap <PrePackObject *>	unk604;		// 604 - NiTPointerMap <NiVBBlock *, NiDX9Renderer::PrePackObject *>
	UInt32		pad614[(0x61C - 0x614) >> 2];	// 614
	
	UInt32	unk61C;							// 61C
	UInt32	unk620;							// 620
	UInt32	pad624[(0x680 - 0x624) >> 2];	// 624
	
	D3DMATRIX		unk680;					// 680
	D3DVIEWPORT9	viewport;				// 6C0
	UInt32			pad6D8[(0x6E8 - 0x6D8) >> 2];	// 6D8
	
	UInt8		pad6E8;							// 6E8
	UInt8		unk6E9;							// 6E9
	UInt8		pad6EA[2];						// 6EA
	UInt32		pad6EC;							// 6EC
	
	UInt8		lostDevice;						// 6F0 - disables drawing
	UInt8		pad6F1[3];						// 6F1
	Unk6F4		unk6F4[4];						// 6F4
	UInt32		unk854[4];						// 854
	NiPixelData	* unk864[4];					// 864
	UInt32	unk874;	// 874 - init'd to 0x16
	NiRenderTargetGroup	* defaultRTGroup;		// 878 - probably back buffer
	NiRenderTargetGroup	* currentRTGroup;		// 87C
	NiRenderTargetGroup	* currentRTGroup2;		// 880
	NiTPointerMap <NiRenderTargetGroup *>	unk884;	// 884 - NiTPointerMap <HWND *, NiPointer <NiRenderTargetGroup> >
	UInt32		unk894;						// 894
	UInt8		unk898;						// 898
	UInt8		pad899[3];					// 899
	NiObject	* unk89C;					// 89C - NiPropertyState (0x30)
	NiGeometryGroupManager	* geometryGroupMgr;		// 8A0
	NiGeometryGroup	* unk8A4;				// 8A4 - NiUnsharedGeometryGroup
	NiGeometryGroup	* unk8A8;				// 8A8 - NiDynamicGeometryGroup
	NiDX9RenderState			* renderState;		// 8AC
	NiDX9VertexBufferManager	* vertexBufferMgr;	// 8B0
	NiDX9IndexBufferManager		* indexBufferMgr;	// 8B4
	NiDX9TextureManager			* textureMgr;		// 8B8
	NiDX9LightManager			* lightMgr;			// 8BC
	NiTPointerMap <NiDX9RenderedTextureData *>	renderedTextures;	// 8C0 - NiTPointerMap <NiRenderedTexture *, NiDX9RenderedTextureData *>
	NiTPointerMap <NiDX9RenderedCubeMapData *>	renderedCubeMaps;	// 8D0 - NiTPointerMap <NiRenderedCubeMap *, NiDX9RenderedCubeMapData *>
	NiTPointerMap <NiDX9DynamicTextureData *>	dynamicTextures;	// 8E0 - NiTPointerMap <NiDynamicTexture *, NiDX9DynamicTextureData *>
	UInt32	unk8F0;							// 8F0
	NiTPointerList <NiDX92DBufferData *>	renderTargets;	// 8F4
	NiTPointerList <NiD3DShaderInterface *>	shaderInterfaces;	// 904
	UInt32		unk914;						// 914
	UInt32		unk918;						// 918
	UInt32		unk91C;						// 91C
	UInt32		unk920;						// 920
	UInt32		unk924;						// 924
	UInt32		unk928;						// 928
	UInt32		unk92C;						// 92C
	UInt32		unk930;						// 930
	UInt32		unk934;						// 934
	UInt32		unk938;						// 938
	UInt32		pad93C;						// 93C
	D3DMATRIX	unk940;						// 940
	D3DMATRIX	viewMatrix;					// 980
	D3DMATRIX	projMatrix;					// 9C0
	D3DMATRIX	invViewMatrix;				// A00
	void		* unkA40;					// A40
	void		* unkA44;					// A44
	void		* unkA48;					// A48
	UInt16		unkA4C;						// A4C
	UInt8		unkA4E[2];					// A4E
	UInt16		* unkA50;					// A50
	UInt32		unkA54;						// A54
	UInt32		width;						// A58
	UInt32		height;						// A5C
	UInt32		padA60[(0xA90 - 0xA60) >> 2];	// A60

	UInt8		unkA90;						// A90
	UInt8		padA91[3];					// A91
	NiObject	* defaultShader;			// A94 - NiD3DDefaultShader (0x70)
	NiTArray <CallbackA98>	unkA98;			// A98
	NiTArray <void *>	unkAA8;				// AA8
	NiTArray <LostDeviceCallback>	lostDeviceCallbacks;	// AB8
	NiTArray <void *>	lostDeviceCallbacksRefcons;	// AC8
	UInt32		padAD0[(0xB00 - 0xAD8) >> 2];	// AD8
};

STATIC_ASSERT(offsetof(NiDX9Renderer, viewport) == 0x6C0);
STATIC_ASSERT(offsetof(NiDX9Renderer, height) == 0xA5C);
STATIC_ASSERT(sizeof(NiDX9Renderer) == 0xB00);
STATIC_ASSERT(offsetof(D3DCAPS9, MaxTextureBlendStages) == 0x94);

extern NiRenderer **	g_renderer;

// 60
class NiDX9TextureData
{
public:
	NiDX9TextureData();
	~NiDX9TextureData();

	// 44
	// ### are all members signed?
	struct Unk0C
	{								//			initialized to
		UInt8			unk00;		// 00		1
		UInt8			pad00[3];
		UInt32			unk04;		// 04		2
		UInt32			unk08;		// 08		0
		SInt32			unk0C;		// 0C		-1
		UInt32			unk10;		// 10		0
		UInt32			unk14;		// 14		16
		UInt32			unk18;		// 18		3
		UInt8			unk1C;		// 1C		8
		UInt8			pad1C[3];
		UInt32			unk20;		// 20		19
		UInt32			unk24;		// 24		5
		UInt8			unk28;		// 28		0
		UInt8			unk29;		// 29		1
		UInt8			pad2A[2];
		UInt32			unk2C;		// 2C		19
		UInt32			unk30;		// 30		5
		UInt8			unk34;		// 34		0
		UInt8			unk35;		// 35		1
		UInt8			pad36[2];
		UInt32			unk38;		// 38		19
		UInt32			unk3C;		// 3C		5
		UInt8			unk40;		// 40		0
		UInt8			unk41;		// 41		1
		UInt8			pad42[2];
	};

	//void*							vtbl;			// 00
	NiRenderedTexture*				unk04;			// 04	parent texture
	NiDX9Renderer*					unk08;			// 08	parent renderer
	Unk0C							unk0C;			// 0C
	UInt32							unk50;			// 50
	UInt32							unk54;			// 54
	UInt32							surfaceWidth;	// 58
	UInt32							surfaceHeight;	// 5C
};

// 64
class NiDX9RenderedTextureData : public NiDX9TextureData
{
public:
	NiDX9RenderedTextureData();
	~NiDX9RenderedTextureData();

	UInt32							unk60;			// 60
};
// 0C
class NiAccumulator : public NiObject
{
public:
	NiAccumulator();
	~NiAccumulator();

	virtual void	Start(NiCamera * camera);
	virtual void	Stop(void);
	virtual void	Add(NiCulledGeoList * list) = 0;
	virtual bool	Fn_16(void);

	NiCamera	* camera;	// 08
};

// 34
class NiBackToFrontAccumulator : public NiAccumulator
{
public:
	NiBackToFrontAccumulator();
	~NiBackToFrontAccumulator();

	virtual void	Fn_17(void);

	// ###
	UInt32	pad0C[(0x34 - 0x0C) >> 2];	// 0C
};

// 38
class NiAlphaAccumulator : public NiBackToFrontAccumulator
{
public:
	NiAlphaAccumulator();
	~NiAlphaAccumulator();

	UInt8	unk34;		// 34
	UInt8	unk35;		// 35
	UInt8	pad36[2];	// 36
};

STATIC_ASSERT(sizeof(NiAlphaAccumulator) == 0x38);

// ??
class ReferenceVolume
{
public:
	ReferenceVolume();
	~ReferenceVolume();
};

// 226C - yes, really
class BSShaderAccumulator : public NiAlphaAccumulator
{
public:
	BSShaderAccumulator();
	~BSShaderAccumulator();

	// ?
	class GeometryGroup
	{
	public:
		GeometryGroup();
		~GeometryGroup();
	};

	// ?
	class ImmediateGeometryGroup
	{
	public:
		ImmediateGeometryGroup();
		~ImmediateGeometryGroup();
	};

	// ?
	class ShadowVolumeRPList
	{
	public:
		ShadowVolumeRPList();
		~ShadowVolumeRPList();
	};

	// 14
	struct Unk00C8
	{
		UInt32	unk00;		// 00
		UInt8	unk04;		// 04
		UInt8	pad05[3];	// 05
		float	unk08;		// 08
		UInt32	unk0C;		// 0C
		UInt32	unk10;		// 10
	};
	// 2C  
	struct Unk0074
	{
		BSTPersistentList <NiGeometry *>					unk00;
		BSTPersistentList <BSShaderProperty::RenderPass *>	unk14;
		UInt32												unk28;
	};
	STATIC_ASSERT(sizeof(BSShaderAccumulator::Unk0074) == 0x2C);
		   

	virtual void	Fn_18(void);

	UInt32	pad0038[(0x003C - 0x0038) >> 2];	// 0038
	NiTPointerList <GeometryGroup *>			geoGroups;			// 003C
	NiTPointerList <GeometryGroup *>			geoGroups2;			// 004C
	UInt32	unk005C;												// 005C
	UInt32	unk0060;												// 0060
	NiTPointerList <ImmediateGeometryGroup *>	immGeoGroups;		// 0064
	Unk0074*	unk0074;	// 0074
	UInt32	unk0078;	// 0078
	BSTPersistentList <BSShaderProperty::RenderPass *>	unk007C;	// 007C
	BSTPersistentList <BSShaderProperty::RenderPass *>	unk0090;	// 0090
	BSTPersistentList <BSShaderProperty::RenderPass *>	unk00A4;	// 00A4
	UInt32	pad00B8[(0x00BC - 0x00B8) >> 2];	// 00B8
	UInt32	sunOcclusionPixels;	// 00BC
	UInt8	unk00C0;	// 00C0
	UInt8	pad00C1[3];	// 00C1
	float	unk00C4;	// 00C4 - init'd to 1
	Unk00C8	unk00C8[3];	// 00C8
	BSTPersistentList <BSShaderProperty::RenderPass *>	unk0104[419];	// 0104 - no, really
	UInt32	pad21C0[(0x21D0 - 0x21C0) >> 2];			// 21C0
	NiTPointerList <ShadowVolumeRPList *>	unk21D0;	// 21D0
	UInt8	unk21E0;									// 21E0
	UInt8	unk21E1;									// 21E1
	UInt8	unk21E2;									// 21E2
	UInt8	unk21E3;									// 21E3
	UInt16	unk21E4;									// 21E4
	UInt16	unk21E6;									// 21E6
	void	* unk21E8;									// 21E8 - C8 byte buffer
	BSTPersistentList <NiGeometry *>		unk21EC;	// 21EC
	BSTPersistentList <NiGeometry *>		unk2200;	// 2200
	BSTPersistentList <NiGeometry *>		unk2214;	// 2214
	UInt32	unk2228;	// 2228
	NiTPointerList <ReferenceVolume *>		unk222C;	// 222C
	UInt32	unk223C;	// 223C
	UInt8	unk2240;	// 2240
	UInt8	pad2241[3];	// 2241
	NiTPointerList <NiGeometry *>			unk2244;	// 2244
	NiTPointerList <NiGeometry *>			unk2254;	// 2254
	void	* unk2264;	// 2264 - 4 byte buffer
	UInt8	unk2268;	// 2268
	UInt8	pad2269[3];	// 2269
};

STATIC_ASSERT(offsetof(BSShaderAccumulator, unk0060) == 0x0060);
STATIC_ASSERT(offsetof(BSShaderAccumulator, unk007C) == 0x007C);
STATIC_ASSERT(offsetof(BSShaderAccumulator, sunOcclusionPixels) == 0x00BC);
STATIC_ASSERT(offsetof(BSShaderAccumulator, unk00C8) == 0x00C8);
STATIC_ASSERT(offsetof(BSShaderAccumulator, unk0104) == 0x0104);
STATIC_ASSERT(sizeof(BSShaderAccumulator) == 0x226C);

// 24
class BSRenderedTexture : public NiRefObject
{
public:
	// members
	///*00*/ NiRefObject
	/*08*/ NiRenderTargetGroup*		renderTargets;
	/*0C*/ UInt32					unk0C;
	/*10*/ UInt32					unk10;
	/*14*/ UInt32					unk14;
	/*18*/ UInt32					unk18;
	/*1C*/ UInt32					unk1C;
	/*20*/ NiRenderedTexture*		renderedTexture;
};

// manages off-screen render targets
// 48
class BSTextureManager
{
public:
	// ?
	struct RenderedTextureData
	{
		UInt32		unk00;
	};

	NiTPointerList<RenderedTextureData>				unk00;				// 00
	NiTPointerList<RenderedTextureData>				unk10;				// 10
	NiTPointerList<BSRenderedTexture>				shadowMaps;			// 20
	NiTPointerList<BSRenderedTexture>				unk30;				// 30
	void*											unk40;				// 40 - smart pointer, screenshot rendertarget?
	void*											unk44;				// 44 - smart pointer

																		// methods
	BSRenderedTexture*								FetchShadowMap(void);
	void											DiscardShadowMap(BSRenderedTexture* Texture);
	void											ReserveShadowMaps(UInt32 Count);

	BSRenderedTexture*								GetDefaultRenderTarget(UInt32 Type);

	static BSTextureManager*						CreateInstance(void);
	void											DeleteInstance(void);

	static BSTextureManager**						Singleton;
};
STATIC_ASSERT(sizeof(BSTextureManager) == 0x48);	 