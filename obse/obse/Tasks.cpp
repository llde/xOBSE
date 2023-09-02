#include "Tasks.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameExtraData.h"


std::vector<Task<void>*> TaskManager::s_taskQueue;
std::vector<Task<bool>*> TaskManager::taskRemovableQueue;

template<>
bool Task<bool>::Run() noexcept {
    __try {
        return this->task();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        _MESSAGE("Catched Exception while Running Task %08X", this);
        return true;
    }
}
template<>
void Task<void>::Run() noexcept {
    __try {
        this->task();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        _MESSAGE("Catched Exception while Running Task %08X", this);
    }
}

///TODO put CS in place.
template<IsIntOrVoid T>
void TaskManager::Enqueue(Task<T>* task)  {
    auto& queue = TaskManager::getQueue(task);
    queue.push_back(task);
}

template<IsIntOrVoid T>
Task<T>* TaskManager::Enqueue(TaskFunc<T> task) {
    Task<T>* func = new Task(task);
    auto& queue = TaskManager::getQueue(func);
    queue.push_back(func);
    return func;
}

template<>
std::vector<Task<bool>*>& TaskManager::getQueue(Task<bool>* task) {
    return taskRemovableQueue;
}

template<>
std::vector<Task<void>*>& TaskManager::getQueue(Task<void>* task) {
    return s_taskQueue;
}


template<IsIntOrVoid T>
void TaskManager::Remove(Task<T>* toRemove){
    auto& queue = TaskManager::getQueue(toRemove);
    for (auto task = queue.begin(); task != queue.end(); ++task) {
        if(*task == toRemove){
            queue.erase(task);
            break;
        }
    }
}

template<IsIntOrVoid T>
bool TaskManager::IsTaskEnqueued(Task<T>* toCheck){
    auto& queue = TaskManager::getQueue(toCheck);

    for (auto task = queue.begin(); task != queue.end(); ++task) {
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
    auto task = taskRemovableQueue.begin();
    while(task != taskRemovableQueue.end()){
        if ((*task)->Run() == true) {
            task = taskRemovableQueue.erase(task);
        }
        else {
            task = ++task;
        }
    }
}


namespace PluginAPI {
    void ReEnqueueTask(Task<void>* task) { return TaskManager::Enqueue(task); }
    Task<void>* EnqueueTask(TaskFunc<void> task) { return TaskManager::Enqueue(task); }
    bool IsTaskEnqueued(Task<void>* task) { return TaskManager::IsTaskEnqueued(task); }
    void Remove(Task<void>* toRemove) { return TaskManager::Remove(toRemove); }
    void ReEnqueueTask(Task<bool>* task) { return TaskManager::Enqueue(task); }
    Task<bool>* EnqueueTask(TaskFunc<bool> task) { return TaskManager::Enqueue(task); }
    bool IsTaskEnqueued(Task<bool>* task) { return TaskManager::IsTaskEnqueued(task); }
    void Remove(Task<bool>* toRemove) { return TaskManager::Remove(toRemove); }

    bool HasTasks() { return TaskManager::HasTasks(); }

} 