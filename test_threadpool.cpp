#include "ThreadPool.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class CWorkTask: public CTask
{
public:
	CWorkTask(){}
	int Run()
	{
		cout << (char*)this->m_ptrData << endl;
		sleep(10);
		cout << "over" << endl;
		return 0;
	}
};

int main()
{
	CThreadPool threadPool(10);
	for(int i = 0;i < 20;i++)
	{
		CWorkTask *taskObj = new CWorkTask;
		char szTmp[] = "this is the first thread running,haha success";
		taskObj->SetData((void*)szTmp);
		sleep(0.1);
		threadPool.AddTask(taskObj);
		/*CWorkTask *taskObj = new CWorkTask();
		char szTmp[] = "this is the first thread running,haha success";
		char buf[200];
		sprintf(buf,"%s:%d", szTmp, i);
		taskObj->SetData((void*)buf);
		cout << buf << endl;
		threadPool.AddTask(taskObj);*/
	}
	while(1)
	{
		cout << "idle thread number:" << CThreadPool::m_vecIdleThread.size() << endl;
		cout << "busy thread number:" << CThreadPool::m_vecBusyThread.size() << endl;
		sleep(2);
	}
	return 0;
}
