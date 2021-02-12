#include "Tasks.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameExtraData.h"

std::vector<Task*> TaskManager::s_taskQueue;


///TODO put CS in place.
void TaskManager::Enqueue(Task* task) {
	s_taskQueue.push_back(task);
}


void Task::Run() {
    this->task();
}

Task* TaskManager::Enqueue(TaskFunc task) {
    Task* func = new Task(task);
	s_taskQueue.push_back(func);
    return func;
}

void TaskManager::Remove(Task* toRemove){
    for (auto task = s_taskQueue.begin(); task != s_taskQueue.end(); ++task) {
        if(*task == toRemove){
            s_taskQueue.erase(task);
            break;
        }
    }
}

bool TaskManager::IsTaskEnqueued(Task* toCheck){
    for (auto task = s_taskQueue.begin(); task != s_taskQueue.end(); ++task) {
        if(*task == toCheck){
            return true;
        }
    }
    return false;
}

void TaskManager::Run() {
    for(auto task = s_taskQueue.begin(); task != s_taskQueue.end(); ++task){
        (*task)->Run();
    }
}


namespace PluginAPI {

    bool HasTask() { return TaskManager::HasTasks(); }
    bool IsTaskEnqueued(Task* task) { return TaskManager::IsTaskEnqueued(task); }
    Task* EnqueueTask(TaskFunc task) { return TaskManager::Enqueue(task); }
    void Remove(Task* toRemove) { return TaskManager::Remove(toRemove); }
}