#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <shared_mutex>
#include <functional>
#include <condition_variable>
#include <future>
#include <chrono>
#include <iostream>
#include <Windows.h>


using namespace std;
using namespace std::chrono;


typedef function<void()> task_type; /// шаблон
typedef void (*FuncType) (vector<int>&, int, int); //формат функции


template<class T> class BlockedQueue
{
public:
	void push(T& item)
	{
		lock_guard<mutex> l(m_locker); 
		m_task_queue.push(item); 
		//оповещаем поток, что задача добавлена
		m_event_holder.notify_one(); 
	}

	void pop(T& item)
	{
		unique_lock<mutex> l(m_locker);
		if (m_task_queue.empty())
			m_event_holder.wait(l, [this] {return !m_task_queue.empty(); });

		item = m_task_queue.front();
		m_task_queue.pop();
	}

	bool fast_pop(T& item)
	{
		unique_lock<mutex> l(m_locker);
		if (m_task_queue.empty())
			return false;
		item = m_task_queue.front();
		m_task_queue.pop();
		return true;
	}
private:
	// очередь задач на выполнение
	queue<T> m_task_queue; 
	mutex m_locker;
	condition_variable m_event_holder;
};

class ThreadPool
{
public:
	ThreadPool();
	void start(); // запуск потоков в m_threads
	void stop(); // остановка
	//метод проброса задач в пулл потоков
	void push_task(FuncType f, vector<int>&, int, int); 
	void threadFunc(int qindex); 
private:
	int m_thread_count; 
	vector<thread> m_threads; 
	vector<BlockedQueue<task_type>> m_thread_queues;
	unsigned m_qindex; //счетчик того в какую из очередей добавлена задача
};


class RequestHandler // класс обертка над классом ThreadPool
{
public:
	RequestHandler();
	~RequestHandler();
	void push_task(FuncType, vector<int>&, int, int);
private:
	ThreadPool m_tpool;
};