#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H
#include"EventLoop.h"
#include<thread>
#include<string>
#include<mutex>
#include<condition_variable>

class WorkerThread
{
public:
    WorkerThread(int index);
    
    ~WorkerThread();

    //执行该线程
    void run();

    //获取反应堆
    inline EventLoop* getEvLoop(){
        return evloop;
    }

private:
    //线程运行函数 创建evloop并解除主线程阻塞 
    void threadRunFunc();

private:
    //线程id
    std::thread::id threadID;
    //线程名
    std::string threadName;
    //反应堆
    EventLoop* evloop;
    //线程对象
    std::thread* thread_;
    //互斥锁
    std::mutex mutex;
    //条件变量
    std::condition_variable cond;
};
 

#endif