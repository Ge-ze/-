#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include"EventLoop.h"
#include"Buffer.h"
#include"HttpRequest.h"
#include"HttpResponse.h"

/**
 * 定义该宏变量则在监听到socket的写事件触发后，不会立刻发出会等到下一轮的监听；
 * 否则，每段数据准备好就立刻发送
*/
//#define MSG_SED_AUTO

/**
 * 维护TCP通信过程需要的对象如evloop、监听的事件、读/写缓冲、请求/响应
*/
class TcpConnection
{
public:
    TcpConnection(int fd,EventLoop* evloop);
    ~TcpConnection();

private:
    //读回调函数
    static int processRead(void* arg);
    //写回调函数
    static int processWrite(void* arg);
    //回收内存回调函数
    static int processDestroy(void* arg);

private:
    std::string name;
    EventLoop* evloop;
    Channel* channel;
    Buffer* readBuf;
    Buffer* writeBuf;
    //http响应
    HttpRequest* request;
    //http应答
    HttpResponse* response;
};


#endif
