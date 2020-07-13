#pragma once

#include <vector>

struct BaseExtraList;
class TESForm;
class TESObjectREFR;
class Actor;

class Task {
public:
	virtual ~Task() { }
	virtual void Run() = 0;
	virtual bool IsReady() = 0;
};

class TaskManager {
public:
	static bool HasTasks() { return s_taskQueue.size() != 0; }
	static void Enqueue(Task* task);
	static void Run();

private:
	TaskManager();

	static std::vector<Task*> s_taskQueue;
};





	
	