#include "Tasks.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameExtraData.h"

std::vector<Task*> TaskManager::s_taskQueue;

void TaskManager::Enqueue(Task* task) 
{
	s_taskQueue.push_back(task);
}

void TaskManager::Run() 
{
	UInt32 i = 0;
	std::vector<Task*>::iterator iter = s_taskQueue.begin();
	while (iter != s_taskQueue.end()) {
		Task* task = *iter;
		if (task->IsReady()) {
			task->Run();
			iter = s_taskQueue.erase(iter);
			delete task;
		}
		else {
			++iter;
		}
	}
}

