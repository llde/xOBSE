#include "obse/Script.h"
#include "EditorAPI.h"
#include "string.h"

#if CS_VERSION == CS_VERSION_1_2
	FormMap* g_FormMap = (FormMap *)0x009EE18C;		// currently unused
	DataHandler ** g_dataHandler = (DataHandler **)0x00A0E064;
	TES** g_TES = (TES**)0x00A0ABB0;
	const _GetFormByID GetFormByID = (_GetFormByID)(0x0047B340);
	const _FormHeap_Allocate FormHeap_Allocate = (_FormHeap_Allocate)0x00401E80;
	const _FormHeap_Free FormHeap_Free = (_FormHeap_Free)0x00401EA0;
	const _Oblivion_DynamicCast Oblivion_DynamicCast = (_Oblivion_DynamicCast)0x0088DC0C;
	const _ShowCompilerError ShowCompilerError = (_ShowCompilerError)0x004FFF40;
#elif CS_VERSION == CS_VERSION_1_0
	const _GetFormByID GetFormByID = (_GetFormByID)(0x00475430);
	const _FormHeap_Allocate FormHeap_Allocate = (_FormHeap_Allocate)0x00401E40;
	const _FormHeap_Free FormHeap_Free = (_FormHeap_Free)0x00401E60;
	const _Oblivion_DynamicCast Oblivion_DynamicCast = (_Oblivion_DynamicCast)0x008AB025;
#else
#error unsupported CS version
#endif

bool BSStringT::Set(const char * src)
{
	if (!src) {
		FormHeap_Free(m_data);
		m_data = 0;
		m_bufLen = 0;
		m_dataLen = 0;
		return true;
	}

	UInt32	srcLen = strlen(src);

	// realloc if needed
	if(srcLen > m_bufLen)
	{
		FormHeap_Free(m_data);
		m_data = (char *)FormHeap_Allocate(srcLen + 1);
		m_bufLen = m_data ? srcLen : 0;
	}

	if(m_data)
	{
		strcpy_s(m_data, m_bufLen + 1, src);
		m_dataLen = srcLen;
	}
	else
	{
		m_dataLen = 0;
	}

	return m_data != NULL;
}

BSStringT::~BSStringT()
{
}

RenderWindowSelection* RenderWindowSelection::GetSingleton()
{
#if CS_VERSION == CS_VERSION_1_2
	RenderWindowSelection** singleton = (RenderWindowSelection**)0x00A0AF60;
	return *singleton;
#else
#error unsupported CS version
#endif
}

