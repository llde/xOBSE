#include "NiRenderer.h"
#include "GameAPI.h"

NiRenderer ** g_renderer = (NiRenderer **)0x00B3F928;

void BSTextureManager::DiscardShadowMap(BSRenderedTexture* Texture)
{
	ThisStdCall(0x007C1A30, this, Texture);
}

BSRenderedTexture* BSTextureManager::FetchShadowMap(void)
{
	return (BSRenderedTexture*)ThisStdCall(0x007C1960, this);
}

BSTextureManager* BSTextureManager::CreateInstance(void)
{
	BSTextureManager* Instance = (BSTextureManager*)FormHeap_Allocate(0x48);
	ThisStdCall(0x007C1FF0, Instance);
	return Instance;
}

void BSTextureManager::DeleteInstance(void)
{
	ThisStdCall(0x007C2100, this);
	FormHeap_Free(this);
}

void BSTextureManager::ReserveShadowMaps(UInt32 Count)
{
	ThisStdCall(0x007C2710, this, *g_renderer, Count);
}

BSRenderedTexture* BSTextureManager::GetDefaultRenderTarget(UInt32 Type)
{
	return (BSRenderedTexture*)ThisStdCall(0x007C23C0, this, *g_renderer, Type);
}
