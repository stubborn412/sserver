#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ThreadPool.h"

using namespace std;

#define MAXLINE 1024
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

int epfd;

class CAcceptTask: public CTask
{
public:
	CAcceptTask(){}
	CAcceptTask(string taskName):CTask(taskName){}
	int Run()
	{
		cout << (char*)this->m_ptrData << endl;
		sleep(10);
		return 0;
	}
};

class CWorkTask: public CTask
{
public:
	CWorkTask(){}
	CWorkTask(string taskName):CTask(taskName){}
	int Run()
	{
		int sockfd;
		ssize_t n;
		struct epoll_event *pEvent = (struct epoll_event *)this->m_ptrData;
                if ( (sockfd = pEvent->data.fd) < 0) {
                    perror("socket error");
                    delete pEvent;
                    return -1;
                }
		char line[MAXLINE];
                if ( (n = read(sockfd, line, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
			perror("connect reset error");
                    } else {
			perror("readline error");
                    }
                    epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,pEvent);
                    close(sockfd);
                    //pEvent->data.fd = -1;
                    delete pEvent;
                    return -1;
                }
                line[n] = '\0';
                cout << "read:" << line << endl;
		ssize_t szSend = write(sockfd, line, n);
                cout << "send:" << line << ", size:"<< szSend <<endl;
                epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,pEvent);
                close(sockfd);
                //pEvent->data.fd = -1;

                delete pEvent;
		return 0;
	}
};

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int i, maxi, listenfd, connfd, sockfd,nfds, portnumber;
    ssize_t n;
    char line[MAXLINE];
    socklen_t clilen;


    if ( 2 == argc )
    {
        if( (portnumber = atoi(argv[1])) < 0 )
        {
            fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);
        return 1;
    }



    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件

    struct epoll_event ev,events[20],listenEv;
    //生成用于处理accept的epoll专用的文件描述符

    epfd=epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //把socket设置为非阻塞方式

    //setnonblocking(listenfd);

    //设置与要处理的事件相关的文件描述符

    listenEv.data.fd=listenfd;
    //设置要处理的事件类型

    listenEv.events=EPOLLIN|EPOLLET;
    //ev.events=EPOLLIN;

    //注册epoll事件

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&listenEv);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    const char *local_addr="127.0.0.1";
    inet_aton(local_addr,&(serveraddr.sin_addr));//htons(portnumber);

    serveraddr.sin_port=htons(portnumber);
    int err = bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    if (err != 0) {
        cout << "bind socket error:" << err << endl;
        return err;
    }
    err = listen(listenfd, LISTENQ);
    if (err != 0) {
        cout << "listen socket error:" << err << endl;
        return err;
    }
    maxi = 0;

    CThreadPool workerPool(10);

    for ( ; ; ) {
        //等待epoll事件的发生
	/*
 	* epoll_wait运行的原理是等待注册在epfd上的socket fd的事件的发生，如果发生则将发生的sokct fd和事件类型放入到events数组中。并且将注册在epfd上的socket fd的事件类型给清空，所以如果下一个循环你还要关注这个socket fd的话，则需要用epoll_ctl(epfd,EPOLL_CTL_MOD,listenfd,&ev)来重新设置socket fd的事件类型。这时不用EPOLL_CTL_ADD，因为socket fd并未清空，只是事件类型清空。这一步非常重要。 */
        nfds=epoll_wait(epfd,events,20,500);
        //处理所发生的所有事件

        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。

            {
                connfd = accept(listenfd,(sockaddr *)&clientaddr, &clilen);
                if(connfd<0){
                    perror("connfd<0");
                    exit(1);
                }
                //setnonblocking(connfd);

                char *str = inet_ntoa(clientaddr.sin_addr);
                cout << "accapt a connection from " << str << endl;
                //设置用于读操作的文件描述符

                ev.data.fd=connfd;
                //设置用于注测的读操作事件

                ev.events=EPOLLIN|EPOLLET;
                //ev.events=EPOLLIN;

                //注册ev

                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
                epoll_ctl(epfd,EPOLL_CTL_MOD,listenfd,&listenEv);
            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。

            {
                //CWorkTask workTask("worker task");
                CWorkTask *workTask = new CWorkTask("worker task");
		struct epoll_event *ev = new epoll_event(events[i]);
                workTask->SetData(ev);
                workerPool.AddTask(workTask);
                /*cout << "EPOLLIN" << endl;
                if ( (sockfd = events[i].data.fd) < 0)
                    continue;
                if ( (n = read(sockfd, line, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        close(sockfd);
                        events[i].data.fd = -1;
                    } else
                        std::cout<<"readline error"<<std::endl;
                } else if (n == 0) {
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                line[n] = '\0';
                cout << "read " << line << endl;
                //设置用于写操作的文件描述符

                ev.data.fd=sockfd;
                //设置用于注测的写操作事件

                ev.events=EPOLLOUT|EPOLLET;*/
                //修改sockfd上要处理的事件为EPOLLOUT

                //epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

            }
            /*else if(events[i].events&EPOLLOUT) // 如果有数据发送

            {
                sockfd = events[i].data.fd;
                write(sockfd, line, n);
                //设置用于读操作的文件描述符

                ev.data.fd=sockfd;
                //设置用于注测的读操作事件

                ev.events=EPOLLIN|EPOLLET;
                //修改sockfd上要处理的事件为EPOLIN

                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }*/
        }
    }
    return 0;
}

