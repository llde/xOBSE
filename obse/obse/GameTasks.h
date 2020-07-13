#pragma once

#include "GameTypes.h"
#include "NiNodes.h"

/*******************************************************
*
* BSTask
*	IOTask
*		QueuedFile
*			QueuedFileEntry
*				QueuedModel
*					QueuedDistantLOD
*					QueuedTreeModel
*				QueuedTexture
*					QueuedTreeBillboard
*				QueuedKF
*					QueuedAnimIdle
*				DistantLODLoaderTask
*				TerrainLODQuadLoadTask
*				SkyTask
*				LipTask
*				GrassLoadTask
*			QueuedReference
*				QueuedTree
*				QueuedActor
*					QueuedCharacter
*						QueuedPlayer
*					QueuedCreature
*			QueuedHead
*			QueuedHelmet
*			QueuedMagicItem
*		AttachDistant3DTask
*		ExteriorCellLoaderTask
*
* NiTArray< NiPointer<QueuedFile> >
*	QueuedChildren
*
*********************************************************/

class TESObjectREFR;
class TESModel;
class QueuedCharacter;
class TESNPC;
class BSFaceGenNiNode;
class BackgroundCloneThread;
class TESAnimGroup;
class BSFaceGenModel;
class QueuedChildren;
class Character;
class AttachDistant3DTask;
class BSTaskManagerThread;

// 4?
class BSTask
{
public:
	virtual void Destroy(bool noDealloc) = 0;
	virtual void Run(void) = 0;
	virtual void Unk_02(void) = 0;
	virtual void Unk_03(UInt32 arg0) = 0;
	virtual bool GetDebugDescription(char* outDesc) = 0;

	// void		** vtbl
	// no data
};

// 18
class IOTask : public BSTask
{
public:
	virtual void Unk_05(void) = 0;
	virtual void Unk_06(void) = 0;
	virtual void Unk_07(UInt32 arg0) = 0; // most (all?) implementations appear to call IOManager::432820(this, arg0) eventually

	IOTask();
	~IOTask();

	// void** vtbl
	BSTask	* unk04;		// 004 
	UInt32  unk08;			// 008 looks like a counter
	UInt32	unk0C;			// 00C 
	UInt32	unk10;			// 010 
	UInt32	unk14;			// 014 
};

class QueuedFile : public IOTask
{
public:
	QueuedFile();
	~QueuedFile();

	virtual void Unk_08(void) = 0;
	virtual void Unk_09(UInt32 arg0) = 0;
	virtual void Unk_0A(void) = 0;

	// no data
};

// 028
class QueuedFileEntry : public QueuedFile
{
public:
	QueuedFileEntry();
	~QueuedFileEntry();

	virtual bool Unk_0B(void);

	// size?
	struct FileEntry {
		UInt32		unk00;
		UInt32		unk04;
		UInt32		size;
		UInt32		offset;
	};

	BSTask			* unk018;			// 018 for QueuedModel, seen QueuedReference (ref to which model will be attached)
	UInt32			unk01C;				// 01C
	char			* filePath;			// 020
	FileEntry		* fileEntry;		// 024
};

// 38
class QueuedReference : public QueuedFile
{
public:
	QueuedReference();
	~QueuedReference();

	UInt32			unk18;			// 018 
	QueuedChildren	* queuedChildren;	// 01C
	TESObjectREFR	* refr;			// 020 
	UInt32			unk24;			// 024 
	UInt32			unk28;			// 028 
	UInt32			unk2C;			// 02C 
	UInt32			unk30;			// 030 
};

// 38
class QueuedModel : public QueuedFileEntry
{
public:
	QueuedModel();
	~QueuedModel();

	virtual void Unk_0C(UInt32 arg0) = 0;

	struct NiNodeInfo
	{
		char		* nifPath;
		UInt32		unk04;
		NiNode		* niNode;
	};

	NiNodeInfo		* niNodeInfo;		// 028
	TESModel		* model;			// 02C
	UInt32			unk30;				// 030
	UInt8			flags;				// 034
	UInt8			pad35[3];
};

class Model
{
	const char	* path;
	UInt32		unk04;
	NiNode		* ninode;
};

class kfModel
{
	const char				* path;
	NiControllerSequence	* controllerSequence;
	TESAnimGroup			* animGroup;
	UInt32					unk0C;
};

// 30
class QueuedKF : public QueuedFileEntry
{
public:
	QueuedKF();
	~QueuedKF();

	kfModel		* kf;		// 28
	UInt32		unk2C;		// 2C
};

class QueuedAnimIdle : public QueuedKF
{
public:
	QueuedAnimIdle();
	~QueuedAnimIdle();
};

// 30
class QueuedHead : public QueuedFile
{
public:
	QueuedHead();
	~QueuedHead();

	QueuedCharacter	* queuedCharacter;	// 018
	UInt32			unk01C;				// 01C
	TESNPC			* npc;				// 020
	BSFaceGenNiNode * faceNiNodes[2];	// 024 presumably male and female
	UInt32			unk02C;				// 02C
};

// 38
class QueuedHelmet : public QueuedFile
{
public:
	QueuedHelmet();
	~QueuedHelmet();

	QueuedCharacter		* queuedCharacter;		// 18
	QueuedChildren		* queuedChildren;		// 1C
	void				* unk20;				// 20
	QueuedModel			* queuedModel;			// 24
	BSFaceGenModel		* faceGenModel;			// 28
	NiNode				* niNode;				// 2C
	Character			* character;			// 30
	UInt32				unk34;					// 34
};

// 30
class BSTaskManager : public LockFreeMap< NiPointer< BSTask > >
{
public:
	virtual void Unk_0F(UInt32 arg0) = 0;
	virtual void Unk_10(UInt32 arg0) = 0;
	virtual void Unk_11(UInt32 arg0) = 0;
	virtual void Unk_12(void) = 0;
	virtual void Unk_13(UInt32 arg0) = 0;

	UInt32				unk1C;			// 1C
	UInt32				unk20;			// 20
	UInt32				numThreads;		// 24
	BSTaskManagerThread	** threads;		// 28 array of size numThreads
	UInt32				unk2C;			// 2C
};

// 3C
class IOManager : public BSTaskManager
{
public:
	virtual void Unk_14(UInt32 arg0) = 0;

	static IOManager* GetSingleton();

	UInt32									unk30;			// 30
	LockFreeQueue< NiPointer< IOTask > >	* taskQueue;	// 34
	UInt32									unk38;			// 38

	bool IsInQueue(TESObjectREFR *refr);
	void QueueForDeletion(TESObjectREFR* refr);
	void DumpQueuedTasks();
};

extern IOManager** g_ioManager;

// 1C
class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();

	// #TODO: generalize key for LockFreeMap, document LockFreeStringMap

	LockFreeMap<Model *>							* modelMap;		// 00 LockFreeCaseInsensitiveStringMap<Model>
	LockFreeMap<kfModel *>							* kfMap;		// 04 LockFreeCaseInsensitiveStringMap<kfModel>
	LockFreeMap< NiPointer<QueuedReference *> >		* refMap;		// 08 key is TESObjectREFR*
	LockFreeMap< NiPointer<QueuedAnimIdle *> >		* idleMap;		// 0C key is AnimIdle*
	LockFreeMap< NiPointer<QueuedHelmet *> >		* helmetMap;	// 10 key is TESObjectREFR*
	LockFreeQueue< NiPointer<AttachDistant3DTask *> >	* distant3DMap;	// 14
	BackgroundCloneThread							* bgCloneThread;	// 18

	static ModelLoader* GetSingleton();

	void QueueReference(TESObjectREFR* refr, UInt32 arg1);
};