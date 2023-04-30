#include"../include/WorkerThread.h"

WorkerThread::WorkerThread(int index){
        threadID = std::thread::id();
        evloop = nullptr;
        threadName = "Subthread-"+std::to_string(index);
        thread_ = nullptr;
}


WorkerThread::~WorkerThread(){
    if(evloop != nullptr){
        delete evloop;
        evloop = nullptr;
    }
    if(thread_ != nullptr){
        delete thread_;
        thread_ = nullptr;
    }
}


void WorkerThread::run(){
    //创建子线程
    thread_ = new std::thread(&WorkerThread::threadRunFunc,this);
    //等待子线程创建EventLoop完毕
    std::unique_lock<std::mutex> locker(mutex);
    //如果表达式返回true则直接返回。否则释放锁，等待唤醒，唤醒后再次判断是否符合条件
    cond.wait(locker,[this](){
        return evloop != nullptr;
    });
}


void WorkerThread::threadRunFunc(){
    mutex.lock();
    //初始化EventLoop
    evloop = new EventLoop(threadName);
    mutex.unlock();
    //唤醒主线程条件变量阻塞
    cond.notify_one();
    //运行EventLoop
    evloop->run();
}

