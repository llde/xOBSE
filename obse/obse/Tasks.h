#pragma once

#include <vector>

typedef void (* TaskFunc)();

class Task {
    TaskFunc task;
    
public:
    Task(TaskFunc task) : task(task) {}
	void Run();
};

class TaskManager {
public:
    static bool HasTasks() { return !s_taskQueue.empty(); }
	static void Enqueue(Task* task);
	static Task* Enqueue(TaskFunc task);
    static bool IsTaskEnqueued(Task* task);
    static void Remove(Task* task);
	static void Run();

private:
	TaskManager();

	static std::vector<Task*> s_taskQueue;
};

namespace PluginAPI{
    void EnqueueT(Task* task);
    Task* EnqueueTask(TaskFunc task);
    bool IsTaskEnqueued(Task* task);
    void Remove(Task* task);
    bool HasTasks();
}

	
	
