#ifndef THREADPOOL_H
#define THREADPOOL_h
#include <pthread.h>
#include <string>
#include <vector>
using namespace std;

class CTask
{
protected:
	string m_strTaskname;
	void *m_ptrData;
public:
	CTask(){}
	CTask(string taskName)
	{
		m_strTaskname = taskName;
		m_ptrData = NULL;
	}
	virtual int Run() = 0;
	void SetData(void *data);
};

class CThreadPool
{
private:
	vector<CTask *> m_vecTaskList;
	int m_iThreadNum;
	static pthread_mutex_t m_pthreadMutex;
	static pthread_cond_t m_pthreadCond;
protected:
	static void* ThreadFunc(void *threadData);
	static int MoveToIdle(pthread_t tid);
	static int MoveToBusy(pthread_t tid);
	int Create();
public:
	static vector<pthread_t> m_vecIdleThread;
	static vector<pthread_t> m_vecBusyThread;
	CThreadPool(int threadnum);
	int AddTask(CTask *task);
	int StopAll();
};

#endif
