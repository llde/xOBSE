#pragma once

#include "obse/NiNodes.h"

/*** tasks
 *	NiTask
 *		BSTECreateTask
 *		NiParallelUpdateTaskManager::SignalTask - unref'd
 *		NiGeomMorpherUpdateTask
 *		NiPSysUpdateTask
 *	NiTaskManager - unref'd
 *		NiParallelUpdateTaskManager - unref'd
 */

// 0C
class NiTask : public NiObject
{
public:
	NiTask();
	~NiTask();

	virtual void	Run(void) = 0;
	virtual bool	Unk_14(void);
	virtual bool	Unk_15(void);	// return (unk08 == 4) || (unk08 == 3);

	UInt32	unk08;	// 08
};

// 10
class BSTECreateTask : public NiTask
{
public:
	BSTECreateTask();
	~BSTECreateTask();

	// on run, kick the task, remove the ref
	// not quite sure what the point is here

	NiTask	* task;	// 0C
};

// 14
class NiGeomMorpherUpdateTask : public NiTask
{
public:
	NiGeomMorpherUpdateTask();
	~NiGeomMorpherUpdateTask();

	NiObject	* unk0C;	// 0C
	NiObject	* unk10;	// 10
};

// 14
class NiPSysUpdateTask : public NiTask
{
public:
	NiPSysUpdateTask();
	~NiPSysUpdateTask();

	NiObject	* unk0C;	// 0C - probably the particle system
	float		unk10;		// 10 - time delta?
};

// ?
class NiTaskManager : public NiObject
{
public:
	NiTaskManager();
	~NiTaskManager();
};

// ?
class NiParallelUpdateTaskManager : public NiTaskManager
{
public:
	NiParallelUpdateTaskManager();
	~NiParallelUpdateTaskManager();

	// ?
	class SignalTask : public NiTask
	{
	public:
		SignalTask();
		~SignalTask();
	};
};
