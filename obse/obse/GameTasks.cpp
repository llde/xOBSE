#include "GameTasks.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "Hooks_Gameplay.h"

#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
	IOManager** g_ioManager = (IOManager**)0x00AEBE80;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
	IOManager** g_ioManager = (IOManager**)0x00B33A10;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
	IOManager** g_ioManager = (IOManager**)0x00B33A10;
	ModelLoader** g_modelLoader = (ModelLoader**)0x00B33A1C;
#else
#error unsupported Oblivion version
#endif

bool IOManager::IsInQueue(TESObjectREFR *refr) 
{
	for (LockFreeQueue< NiPointer<IOTask> >::Node *node = taskQueue->head->next; node; node = node->next) 
	{
		QueuedReference *qr = OBLIVION_CAST(node->data, IOTask, QueuedReference);
		if (!qr)
			continue;

		if (qr->refr == refr)
			return true;
	}

	return false;
}

void IOManager::DumpQueuedTasks()
{
#if 0
	_MESSAGE("Dumping queued tasks:");
	for (LockFreeQueue< NiPointer<IOTask> >::Node *node = taskQueue->head->next; node; node = node->next)
	{
		QueuedReference* qr = OBLIVION_CAST(node->data, IOTask, QueuedReference);
		if (!qr)
			continue;
		else if (qr->refr)
		{
			Console_Print("\t%s (%08x)", GetFullName(qr->refr), qr->refr->refID);
			_MESSAGE("\t%s (%08x)", GetFullName(qr->refr), qr->refr->refID);
		}
		else
			_MESSAGE("NULL reference");
	}
#endif

}

IOManager* IOManager::GetSingleton()
{
	return *g_ioManager;
}

ModelLoader* ModelLoader::GetSingleton()
{
	return *g_modelLoader;
}

void ModelLoader::QueueReference(TESObjectREFR* refr, UInt32 arg1)
{
	ThisStdCall(0x00438060, this, refr, arg1);
}
