#include"../include/ThreadPool.h"
#include<assert.h>

ThreadPool::ThreadPool(EventLoop* evloop,int threadNum):index(0){
    mainLoop = evloop;
    workerThreads.clear();
    isStart = false;
    threadNum = threadNum;    
}


ThreadPool::~ThreadPool(){
    for(int i = 0; i < workerThreads.size(); ++i){
        if(workerThreads[i] == nullptr){
            delete workerThreads[i];
            workerThreads[i] = nullptr;
        }
    }
}


void ThreadPool::run(){
    assert(!isStart);
    if(mainLoop->getThreadid() != std::this_thread::get_id()){
        exit(0);
    }

    isStart = true;
    if(threadNum > 0){
        for(int i = 0; i < threadNum; ++i){
            WorkerThread* worker = new WorkerThread(i);
            worker->run();
            workerThreads.push_back(worker);
        }
    }
}

EventLoop* ThreadPool::takeWorkerEventLoop(){
    assert(isStart);
    if(mainLoop->getThreadid() != std::this_thread::get_id()){
        exit(0);
    }

    EventLoop* loop = mainLoop;
    if(threadNum > 0 ){
        loop = workerThreads[index]->getEvLoop();
        index = (++index)%threadNum;

    }
    return loop;
}


