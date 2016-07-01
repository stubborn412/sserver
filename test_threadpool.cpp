#include "ThreadPool.h"
#include <unistd.h>
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
		return 0;
	}
};

int main()
{
	CWorkTask taskObj;
	char szTmp[] = "this is the first thread running,haha success";
	taskObj.SetData((void*)szTmp);
	CThreadPool threadPool(10);
	int i;
	for(i = 0;i < 11;i++)
	{
		threadPool.AddTask(&taskObj);
	}
	while(1)
	{
		cout << "idel thread number:" << CThreadPool::m_vecIdleThread.size() << endl;
		cout << "busy thread number:" << CThreadPool::m_vecBusyThread.size() << endl;
		sleep(2);
	}
	return 0;
}
