#ifndef DISPATCHER_H
#define DISPATCHER_H
#include"Channel.h"
#include"EventLoop.h"
#include<string>

//向前声明
class EventLoop;

/**
 * 分发器模型
*/
class Dispatcher
{
public:
    Dispatcher(EventLoop* evloop):eventloop_(evloop){ }
    
    virtual ~Dispatcher(){}

    virtual int add() = 0;

    //删除
    virtual int remove() = 0;

    //修改
    virtual int modify() = 0;

    //纯虚函数，事件检测
    virtual int dispatch(int timeout = 200) = 0;

    //设置待添加的Channel
    inline void setChannel(Channel* channel){
        channel_ = channel;
    }

protected:
    Channel* channel_;
    EventLoop* eventloop_;
    std::string name = std::string();
};

#endif