#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include"Channel.h"
#include"Dispatcher.hpp"
#include<thread>
#include<string>
#include<mutex>
#include<map>
#include<queue>

/**
 * 任务处理监听文件描述符（事件）的方式
*/
enum class ElenmType:char
{
    ADD,
    DELETE,
    MODIFY
};


/**
 * 任务节点
*/
struct ChannelElement
{
    Channel* channel;
    ElenmType type;
};

struct Dispatcher;

/**
 * 反应堆模型
 * 负责配置IO多路复用、fd监听事件、dispatcher所需数据、任务队列 * 
*/
class EventLoop
{
public:
    //主线程创建反应堆，无需名字
    EventLoop();

    //子线程创建反应堆，需要名字
    EventLoop(const std::string threadname);

    ~EventLoop();

    //启动反应堆，监听并处理文件描述符事件，维护任务队列
    int run();

    //相应事件触发后的执行内容,
    //返回-1表示失败
    int eventActivate(int fd,int event);

    //将Channel任务添加到任务队列,
    //主线程添加监听socket任务,
    //子线程添加通信socket任务
    int addTask(struct Channel* channel,ElenmType type);

    //增删改任务队列
    int processTaskQ();

    //将channel添加到dispatcher和map
    int add(struct Channel* channel);

    //删除channnel fd事件结构体
    int remove(struct Channel* channel);

    //修改Channel fd事件结构体
    int modify(struct Channel* channel);

    //释放Channel所占用的内存
    int freeChannel(struct Channel* channel);

    //读取唤醒socket信息
    int readMessage();

    //返回所在线程id
    inline std::thread::id getThreadid(){
        return threadID;
    }
private:
    //向socketPair[0]写数据，唤醒阻塞的epoll_wait
    void taskWakeup();


private:
    //分发器
    Dispatcher* dispatcher;
    //任务队列
    std::queue<ChannelElement*> taskQueue;
    //map
    std::map<int,Channel*> channelMap;
    //线程ID
    std::thread::id threadID;
    //线程名字
    std::string threadName;
    //互斥锁
    std::mutex mutex_;
    //是否退出
    bool isquit;

    //socketPair[0]用于发数据
    //socketPair[1]用于接收数据
    int socketPair[2];

};
 















#endif