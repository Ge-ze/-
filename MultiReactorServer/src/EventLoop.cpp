#include"../include/EventLoop.h"
#include"../include/EPollDispatcher.h"
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#include<functional>

//主线程创建反应堆，不需要名字
EventLoop::EventLoop():EventLoop("MainThread"){}


//子线程创建反应堆，需要名字
EventLoop::EventLoop(const std::string threadname){
    isquit = true;
    dispatcher = new EPollDispatcher(this);

    channelMap.clear();
    threadID = std::this_thread::get_id();

    threadName = threadname;

    //建立一对本地套接字并放入socketPair数组中
    int ret = socketpair(AF_UNIX,SOCK_STREAM,0,socketPair);
    if(ret == -1){
        perror("socketpair");
        exit(0);
    }

    //使用functional库为其绑定参数
    auto fun = std::bind(&EventLoop::readMessage,this);
    Channel* channel = new Channel(socketPair[1],FDEvent::ReadEvent,fun,nullptr,nullptr,this);
    //添加任务队列
    addTask(channel,ElenmType::ADD);

}

EventLoop::~EventLoop(){}

//启动反应堆，监听并处理文件描述符事件，维护任务队列
int EventLoop::run(){
    isquit = false;
    //比较线程id是否正常
    if(threadID != std::this_thread::get_id()){
        return -1;
    }
    while(!isquit){
        //监听并处理文件描述符的事件
        //多态
        dispatcher->dispatch();

        //维护任务队列，当前阻塞解除后才会执行
        processTaskQ();
    }
    return 0;
}

//相应事件触发后的执行内容,
//返回-1表示失败
int EventLoop::eventActivate(int fd,int event){
    if(fd < 0){
        return -1;
    }

    //取出Channel
    struct Channel* channel = channelMap[fd];
    assert(channel->getSocketfd() == fd);

    if((event & (int)FDEvent::ReadEvent) && channel->readCallback){
        channel->readCallback(const_cast<void*>(channel->getArgCallback()));
    }
    if((event & (int)FDEvent::WriteEvent) && channel->writeCallback){
        channel->writeCallback(const_cast<void*>(channel->getArgCallback()));
    }

    return 0;
}

//将Channel任务添加到任务队列,
//主线程添加监听socket任务,
//子线程添加通信socket任务
int EventLoop::addTask(struct Channel* channel,ElenmType type){
    
    //创建新的任务节点
    ChannelElement* node = new ChannelElement();
    node->channel = channel;
    node->type = type;

    //加入任务队列
    mutex_.lock();
    taskQueue.push(node);
    mutex_.unlock();

    // 判断执行这个函数的是主线程还是子线程（主线程：添加监听socket的任务；子线程：添加通信socket的任务）
    /*
    * 细节: 
    *   1. 对于链表节点的添加: 可能是当前线程也可能是其他线程(主线程)
    *       1). 修改fd的事件, 当前子线程发起, 当前子线程处理
    *       2). 添加新的fd, 添加任务节点的操作是由主线程发起的
    *   2. 不能让主线程处理任务队列, 需要由当前的子线程取处理
    */
    if(threadID == std::this_thread::get_id()){

        //当前子线程维护任务队列
        processTaskQ();
    }
    else{

        //主线程告诉子线程去处理任务队列的任务
        //子线程在工作且被epoll阻塞，需要被唤醒
        taskWakeup();
    }

    return 0;
}

//增删改任务队列
int EventLoop::processTaskQ(){
    while (!taskQueue.empty()){
        mutex_.lock();
        ChannelElement* node = taskQueue.front();
        taskQueue.pop();
        mutex_.unlock();

        Channel* channel = node->channel;
        if(node->type == ElenmType::ADD){
            add(channel);
        }
        else if(node->type == ElenmType::DELETE){
            remove(channel);
        }
        else if(node->type == ElenmType::MODIFY){
            modify(channel);
        }
        //此处销毁的是 ChannelElement 对象内存，
        //其中存储的是channel的指针，channel的实际内容并没有被销毁
        delete node;
        // node = nullptr;
    }
}

//将channel添加到dispatcher和map
int EventLoop::add(struct Channel* channel){
    int fd = channel->getSocketfd();
    if(channelMap.find(fd) == channelMap.end()){
        channelMap.insert(std::make_pair(fd,channel));
        dispatcher->setChannel(channel);
        int ret = dispatcher->add();
        return ret;
    }
    return -1;
}

//删除channnel fd事件结构体
int EventLoop::remove(struct Channel* channel){
    int fd = channel->getSocketfd();
    auto it = channelMap.find(fd);
    if(it == channelMap.end()){
        return -1;
    }
    dispatcher->setChannel(channel);
    int ret = dispatcher->remove();
    return ret;
}

//修改Channel fd事件结构体
int EventLoop::modify(struct Channel* channel){
    int fd = channel->getSocketfd();
    auto it = channelMap.find(fd);
    if(it == channelMap.end()){
        return -1;
    }
    channelMap.erase(fd);
    channelMap.insert(std::make_pair(fd,channel));
    //设置待添加的Channel
    dispatcher->setChannel(channel);
    int ret = dispatcher->modify();
    return ret;

}

//释放Channel所占用的内存
int EventLoop::freeChannel(struct Channel* channel){
    if(channel == nullptr){
        return -1;
    }
    auto it = channelMap.find(channel->getSocketfd());
    if(it != channelMap.end()){
        channelMap.erase(it);
        close(channel->getSocketfd());
        delete channel;
        channel = nullptr;
        return 0;
    }
    return -1;
}

//读取唤醒socket信息
int EventLoop::readMessage(){
    char buf[128];
    int len = recv(socketPair[1],buf,sizeof(buf),0);
    return len; 
}


//向socketPair[0]写数据，唤醒阻塞的epoll_wait
void EventLoop::taskWakeup(){
    const char* msg = "Wake Up";
    send(socketPair[0],msg,strlen(msg),0);
}