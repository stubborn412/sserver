#include "ThreadPool.h"
#include <iostream>

using namespace std;

void CTask::SetData(void *data)
{
	m_ptrData = data;
}

vector<pthread_t> CThreadPool::m_vecBusyThread;
vector<pthread_t> CThreadPool::m_vecIdleThread;
pthread_mutex_t CThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CThreadPool::m_vectorMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CThreadPool::m_pthreadCond = PTHREAD_COND_INITIALIZER;

CThreadPool::CThreadPool(int threadNum)
{
	this->m_iThreadNum = threadNum;
	Create();
}

int CThreadPool::MoveToIdle(pthread_t tid)
{
	vector<pthread_t>::iterator busyIter = m_vecBusyThread.begin();
	while(busyIter != m_vecBusyThread.end())
	{
		if(tid == *busyIter)
		{
			break;
		}
		busyIter++;
	}

	m_vecBusyThread.erase(busyIter);
	m_vecIdleThread.push_back(tid);
	return 0;
}

int CThreadPool::MoveToBusy(pthread_t tid)
{
	vector<pthread_t>::iterator idleIter = m_vecIdleThread.begin();
	while(idleIter != m_vecIdleThread.end())
	{
		if(tid == *idleIter)
		{
			break;
		}
		idleIter++;
	}

	m_vecIdleThread.erase(idleIter);
	m_vecBusyThread.push_back(tid);
	return 0;
}

void* CThreadPool::ThreadFunc(void * threadData)
{
	pthread_t tid = pthread_self();
	while(1)
	{
		pthread_mutex_lock(&m_pthreadMutex);
		vector<CTask*> *taskList = (vector<CTask*> *)threadData;
		while (taskList->empty()) {
			pthread_cond_wait(&m_pthreadCond,&m_pthreadMutex);
		}
		cout << "tid:" << tid << " run" << endl;

		vector<CTask*>::iterator iter = taskList->begin();
		while(iter != taskList->end())
		{
			pthread_mutex_lock(&m_vectorMutex);
			MoveToBusy(tid);
			pthread_mutex_unlock(&m_vectorMutex);
			break;
		}
		CTask* task = *iter;
		taskList->erase(iter);
		pthread_mutex_unlock(&m_pthreadMutex);
		//cout << "idel thread number:" << CThreadPool::m_vecIdleThread.size() << endl;
		//cout << "busy thread number:" << CThreadPool::m_vecBusyThread.size() << endl;
		task->Run();
		delete task;
		pthread_mutex_lock(&m_vectorMutex);
		MoveToIdle(tid);
		pthread_mutex_unlock(&m_vectorMutex);
		//cout << "idel thread number:" << CThreadPool::m_vecIdleThread.size() << endl;
		//cout << "busy thread number:" << CThreadPool::m_vecBusyThread.size() << endl;
		//cout << "tid:" << tid << " idle" << endl;
	}
	return (void*)0;
}

int CThreadPool::AddTask(CTask *task)
{
	pthread_mutex_lock(&m_pthreadMutex);
	m_vecTaskList.push_back(task);
	pthread_mutex_unlock(&m_pthreadMutex);
	pthread_cond_signal(&m_pthreadCond);
	return 0;
}

int CThreadPool::Create()
{
	int i;
	for(i = 0; i < m_iThreadNum;i++)
	{
		pthread_t tid = 0;
		pthread_create(&tid, NULL, ThreadFunc, &m_vecTaskList);
		m_vecIdleThread.push_back(tid);
	}
	return 0;
}

int CThreadPool::StopAll()
{
	pthread_mutex_lock(&m_vectorMutex);
	vector<pthread_t>::iterator iter = m_vecIdleThread.begin();
	while(iter != m_vecIdleThread.end())
	{
		pthread_cancel(*iter);
		pthread_join(*iter,NULL);
		iter++;
	}

	iter = m_vecBusyThread.begin();
	while(iter != m_vecBusyThread.end())
	{
		pthread_cancel(*iter);
		pthread_join(*iter,NULL);
		iter++;
	}
	pthread_mutex_unlock(&m_vectorMutex);

	return 0;
}
