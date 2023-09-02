#pragma once

#include  <vector>
#include <typeinfo>


template<typename T>
concept IsIntOrVoid = std::is_same<T, bool>::value || std::is_same<T, void>::value;


template<IsIntOrVoid T> using TaskFuncT = T (*)();

using TaskFunc = TaskFuncT<void>;

template<IsIntOrVoid T>
class Task {
    TaskFuncT<T> task;
    
public:
    Task(TaskFuncT<T> task) noexcept: task(task) {}
	T Run() noexcept;
};

class TaskManager {
public:
    static bool HasTasks() noexcept { return !s_taskQueue.empty(); }
    template<IsIntOrVoid T> static void Enqueue(Task<T>* task);
	template<IsIntOrVoid T> static Task<T>* Enqueue(TaskFuncT<T> task);
    template<IsIntOrVoid T> static bool IsTaskEnqueued(Task<T>* task);
    template<IsIntOrVoid T> static void Remove(Task<T>* task);
	static void Run();

private:
	TaskManager();
    template<IsIntOrVoid T> static std::vector<Task<T>*>& getQueue(Task<T>* task);

	static std::vector<Task<void>*> s_taskQueue;
    static std::vector<Task<bool>*> taskRemovableQueue;

};


namespace PluginAPI{
    void ReEnqueueTask(Task<void>* task);
    Task<void>* EnqueueTask(TaskFuncT<void> task);
    bool IsTaskEnqueued(Task<void>* task);
    void Remove(Task<void>* task);
    void ReEnqueueTask(Task<bool>* task);
    Task<bool>* EnqueueTask(TaskFuncT<bool> task);
    bool IsTaskEnqueued(Task<bool>* task);
    void Remove(Task<bool>* task);

    bool HasTasks();
}


	
