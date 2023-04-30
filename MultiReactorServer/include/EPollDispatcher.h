#ifndef EPOLLDISPATCHER_H
#define EPOLLDISPATCHER_H
#include"Channel.h"
#include"Dispatcher.hpp"
#include"EventLoop.h"
#include<sys/epoll.h>
#include<string>

/**
 * 采用epoll机制的分发器
*/
class EPollDispatcher: public Dispatcher
{ 
public:
    EPollDispatcher(EventLoop* evloop);
    ~EPollDispatcher();

    //重写父类事件
    int add() override;
    int remove() override;
    int modify() override;
    int dispatch(int timeout = 200) override;

    /**
     * 将fd即需要监听的事件 加入/删除/修改 eventpoll
     * op可为 EPOLL_CTL_ADD / EPOLL_CTL_DEL / EPOLL_CTL_MOD
     * 失败返回-1
    */
    int epollctl(int op);

private:
    int epfd;
    struct epoll_event* events;
    const int MAXEVENTS = 1024;
};


#endif