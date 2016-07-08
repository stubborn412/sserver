#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include "ThreadPool.h"

using namespace std;

#define BUF_SIZE 1024*4

short port;
char host[16];

class CSendTask: public CTask
{
public:
	int Run();
};

int CSendTask::Run()
{
	int *pTaskId = (int*)this->m_ptrData;
	fprintf(stdout, "start process task: %d\n", *pTaskId);
	int connfd;
        connfd = socket(AF_INET, SOCK_STREAM, 0);
        if (connfd < 0)
        {       
                fprintf(stderr, "create socket error: (%d) %s\n", connfd, strerror(errno));
		delete pTaskId;
                return -1;
        }
	
        int rt;
	in_addr inahost;
	rt = inet_pton(AF_INET, host, (void *)&inahost);
	if (rt != 1)
	{
		fprintf(stderr,"inet_pton error: %s\n", strerror(errno));
		delete pTaskId;
		return -1;
	}
	sockaddr_in svraddr;
        svraddr.sin_family = AF_INET; 
        svraddr.sin_port = htons(port);
        svraddr.sin_addr = inahost;

	rt = connect(connfd, (sockaddr *)&svraddr, sizeof(svraddr));
	if (rt < 0)
	{
		fprintf(stderr,"connect error: %s\n", strerror(errno));
		delete pTaskId;
		return -1;
	}

	char buf_send[BUF_SIZE];
	sprintf(buf_send, "hello: %d", *pTaskId);
	char buf_recv[BUF_SIZE];
	char data_recv[10*BUF_SIZE];
	data_recv[0]='\0';
	int i;
	ssize_t sendsize;
	ssize_t recvsize;
	timeval tv;
	tm *ptm;
	for (i=0; i<1; i++)
	{
		//timeval tv;
		//tv.tv_sec = 0;
		//tv.tv_usec = 500000;
		//select(0, NULL, NULL, NULL, &tv);
		//sleep(1);
		gettimeofday(&tv, NULL);
                ptm = localtime(&tv.tv_sec);
		sendsize = send(connfd, buf_send, strlen(buf_send), 0);
		fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d:%06d] ", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec);
		fprintf(stdout, "i:%d,data:%s,size:%d\n", i, buf_send, sendsize);
		while ((recvsize = recv(connfd, buf_recv, BUF_SIZE, 0)) > 0)
		{
			strncat(data_recv,buf_recv,recvsize);
		}
		gettimeofday(&tv, NULL);
		ptm = localtime(&tv.tv_sec);
		fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d:%06d] ", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec);
		fprintf(stdout, "Response:");
		fwrite(data_recv, sizeof(char), strlen(data_recv), stdout);
		fprintf(stdout, "(Size: %d)\n", strlen(data_recv));
	}
	close(connfd);
	fprintf(stdout, "finish process task: %d\n", *pTaskId);
	delete pTaskId;
	return 0;
}

int main(int argc, char *argv[])
{
	char ch;
        //short port;
	//char host[16];
	int threadnum = 1;

        while (-1 != (ch = getopt(argc, argv, "i:p:t:")))
        {
                switch(ch)
                {
			case 'i':
				strcpy(host, optarg);
				break;
                        case 'p':
                                port = atoi(optarg);
                                break;
                        case 't':
                                threadnum = atoi(optarg);
                                break;
                        case 'h':
                                fprintf(stderr, "Usage: %s -i host -p port -t threadnum\n", argv[0]);
                                return 0;
                        default:
                                fprintf(stderr, "Usage: %s -i host -p port -t threadnum\n", argv[0]);
                                exit(EXIT_FAILURE);
                }
        }

	timeval tvStart;
	gettimeofday(&tvStart, NULL);
	CThreadPool threadpool(threadnum);
	
	int i;
	CSendTask *psendtask;
	timeval tvSleep;
	for (i=0; i<100000; i++)
	{
		//tvSleep.tv_sec = 0;
		//tvSleep.tv_usec = 10;
		//to avoid pthread_cond_signal losing
		//select(0, NULL, NULL, NULL, &tvSleep);		
		psendtask = new CSendTask;
		int *pTaskId = new int(i);
		psendtask->SetData(pTaskId);
		threadpool.AddTask(psendtask);
	}

	timeval tvCur;
	while (1)
	{
		gettimeofday(&tvCur, NULL);
		if (tvCur.tv_sec - tvStart.tv_sec < 10)
		{
			tvSleep.tv_sec = 0;
			tvSleep.tv_usec = 100;
			select(0, NULL, NULL, NULL, &tvSleep);		
		}
		else
		{	
			fprintf(stdout, "start stop all thread!\n");
			threadpool.StopAll();
			fprintf(stdout, "finish stop all thread!\n");
			break;
		}
	}
	tm *ptm = localtime(&tvCur.tv_sec);
	fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d:%06d] client exit!\n", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tvCur.tv_usec);
	
	return 0;
}

