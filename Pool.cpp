#include "Pool.h"

ThreadPool::ThreadPool()
	: m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4),
	m_thread_queues(m_thread_count)
{
	cout << "Инициированно потоков: " << m_thread_count << endl;
}

void ThreadPool::start()
{
	for (int i = 0; i < m_thread_count; i++)
	{
		m_threads.emplace_back(&ThreadPool::threadFunc, this, i);
	}
}

void ThreadPool::stop()
{
	for (int i = 0; i < m_thread_count; i++)
	{
		task_type empty_task;
		m_thread_queues[i].push(empty_task);
	}
	//ждем завершения всех потоков
	for (auto& t : m_threads)
		t.join();
}

void ThreadPool::push_task(FuncType f, vector<int>& arr, int a, int b)
{
	//определяем индекс очереди, в которую нужно положить задачу
	int queue_to_push = m_qindex++ % m_thread_count;
	//создаем задачу
	task_type task = task_type([&arr, a, b, f] {f(arr, a, b); });
	// помещаем в очередь
	m_thread_queues[queue_to_push].push(task); 
}


void ThreadPool::threadFunc(int qindex)
{
	//бесконечный цикл читает задачи из очереди
	while (true)
	{
		task_type task_to_do;
		bool res; 
		int i = 0;
		for (; i < m_thread_count; i++) 
		{
			if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
				break;
		}
		if (!res)
		{
			m_thread_queues[qindex].pop(task_to_do);
		}
		else if (!task_to_do)
		{
			m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
		}
		if (!task_to_do) 
		{
			return;
		}

		task_to_do();
	}
}

RequestHandler::RequestHandler()
{
	this->m_tpool.start();
}

RequestHandler::~RequestHandler(){}

void RequestHandler::push_task(FuncType f, vector<int>& arr, int a, int b)
{
	this->m_tpool.push_task(f, arr, a, b);
	this->m_tpool.stop();
}