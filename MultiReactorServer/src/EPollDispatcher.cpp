#include"../include/EPollDispatcher.h"
#include<unistd.h>

EPollDispatcher::EPollDispatcher(EventLoop* evloop): Dispatcher(evloop){
    //创建epollevent对象并返回文件描述符
    epfd = epoll_create(1);
    if(epfd == -1){
        perror("epoll_create");
        exit(0);
    }
    events = new struct epoll_event[MAXEVENTS];
    name = "epoll";

}

EPollDispatcher::~EPollDispatcher(){
    if(epfd != -1){
        close(epfd);
    }
    if(events != nullptr){
        delete[] events;
        events = nullptr;
    }
}

int EPollDispatcher::add(){
    int ret = epollctl(EPOLL_CTL_ADD); 
    if (ret == -1)
    {
        perror("EPOLL_CTL_ADD");
        exit(0);
    }

    return ret;

}

int EPollDispatcher::remove(){
    int ret = epollctl(EPOLL_CTL_DEL); 
    if (ret == -1)
    {
        perror("EPOLL_CTL_DEL");
        exit(0);
    }

    //通过TcpConnection释放资源
    channel_->destroyCallback(const_cast<void*>(channel_->getArgCallback()));
    return ret;

}
int EPollDispatcher::modify(){
    int ret = epollctl(EPOLL_CTL_MOD); 
    if (ret == -1)
    {
        perror("EPOLL_CTL_MOD");
        exit(0);
    }

    return ret;

}
int EPollDispatcher::dispatch(int timeout ){
    //监听文件描述符
    int num = epoll_wait(epfd,events,MAXEVENTS,timeout);

    //若有文件描述符就绪
    for(int i = 0; i < num; ++i){
        //获取事件
        int curev = events[i].events;
        //获取文件描述符
        int curfd = events[i].data.fd;

        // EPOLLERR 对方断开连接   EPOLLHUP 对方断开连接但仍向对方发送数据
        if(curev & EPOLLERR || curev & EPOLLHUP){
            //对方断开了连接，需要删除fd
            continue;
        }

        if(curev & EPOLLIN){
            eventloop_->eventActivate(curfd,static_cast<int>(FDEvent::ReadEvent));
        }
        if(curev & EPOLLOUT){
            eventloop_->eventActivate(curfd,static_cast<int>(FDEvent::WriteEvent));
        }
    }
    return 0;
}

/**
 * 将fd即需要监听的事件 加入/删除/修改 eventpoll
 * op可为 EPOLL_CTL_ADD / EPOLL_CTL_DEL / EPOLL_CTL_MOD
 * 失败返回-1
*/
int EPollDispatcher::epollctl(int op){
    if(channel_ == nullptr){
        return -1;
    }
    int event = 0;
    if(channel_->getEvent() & static_cast<int>(FDEvent::ReadEvent)){
        event |= EPOLLIN;
    }
    if(channel_->getEvent() & static_cast<int>(FDEvent::WriteEvent)){
        event |= EPOLLOUT;
    }

    struct epoll_event epevent;
    epevent.data.fd = channel_->getSocketfd();
    epevent.events = event;

    // 最后一个参数内容会被解开引用并复制，不需考虑同步互斥
    int res = epoll_ctl(epfd,op,channel_->getSocketfd(),&epevent);

    return res;
}
