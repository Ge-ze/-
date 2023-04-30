#ifndef THREADPOOL_H
#define THREADPOOL_H
#include"EventLoop.h"
#include"WorkerThread.h"
#include<vector>


/**
 * 线程池模型
*/
class ThreadPool
{
public:
    ThreadPool(EventLoop* evloop,int threadNum);    
    ~ThreadPool();
    //启动线程池
    void run();
    //从线程池中取出一个反应堆模型
    EventLoop* takeWorkerEventLoop();

private:
    //主反应堆模型
    EventLoop* mainLoop;
    std::vector<WorkerThread*> workerThreads;
    //线程池是否启动
    bool isStart;
    //线程
    int threadNum;
    int index;
};


#endif