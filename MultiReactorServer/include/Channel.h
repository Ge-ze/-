#ifndef CHANNEL_H
#define CHANNEL_H
#include<functional>

/**
 * 监听事件的枚举
*/
enum class FDEvent
{
    Timeout = 0x1,
    ReadEvent = 0x2,
    WriteEvent = 0x4
};


/**
 * 对于监听事件fd的二次封装
 * 包含文件描述符、对应事件、对应回调函数、对应参数
*/
class Channel
{
public:

    //C++新特性，使用using定义类型别名，
    //并使用function对象包装器包装返回值为int,参数为void*的函数。
    using handleFunc = std::function<int(void*)>;
    //读回调函数
    handleFunc readCallback;
    //写回调函数
    handleFunc writeCallback;
    //资源销毁函数
    handleFunc destroyCallback;

    Channel(int fd,FDEvent event,handleFunc readFunc,handleFunc writeFunc,handleFunc destroyFunc,void* arg):
        fd_(fd),
        event_((int)event),
        arg_(arg),
        readCallback(readFunc),
        writeCallback(writeFunc),
        destroyCallback(destroyFunc){}

    ~Channel(){}

    //修改fd的写事件（检测 or 不检测）
    void writeEventEnable(bool flag);

    //判断是否检测写事件
    bool isWriteEventable();

    //获取文件描述符fd
    inline int getSocketfd(){
        return fd_;
    }

    //获取事件
    inline int getEvent(){
        return event_;
    }

    //获取回调函数
    inline const void* getArgCallback(){
        return arg_;
    }

private:
    //文件描述符
    int fd_;
    //事件
    int event_;
    //读回调函数
    void* arg_;

};

  







#endif