#ifndef TCPSERVER_H
#define TCPSERVER_H
#include"EventLoop.h"
#include"ThreadPool.h"

/**
 * 服务器对象
*/
class TcpServer
{
public:
    TcpServer(unsigned short port, int threadNum):port(port),threadNum(threadNum){
        //创建主反应堆
        mainLoop = new EventLoop();
        //创建线程池
        pool = new ThreadPool(mainLoop,threadNum);
        printf("TcpServer构造完成\n");
        setListen();
    }

    ~TcpServer(){}

    //初始化监听套接字
    void setListen();

    //运行TCP服务器，创建套接字，并运行
    void run();

    //连接请求的回调函数
    int acceptConn(); 

private:
    int threadNum;
    EventLoop* mainLoop;
    ThreadPool* pool;
    int lfd;
    unsigned short port;
};

#endif