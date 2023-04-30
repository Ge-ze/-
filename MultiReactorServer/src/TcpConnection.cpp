#include"../include/TcpConnection.h"
#include"../include/Log.h"
#include<string>
#include<stdlib.h>
#include<unistd.h>


TcpConnection::TcpConnection(int fd,EventLoop* evloop){
    this->name = "TcpConnection-"+std::to_string(fd);
    this->evloop = evloop;
    this->readBuf = new Buffer(10240);
    this->writeBuf = new Buffer(10240);
    this->request = new HttpRequest();
    this->response = new HttpResponse();
    this->channel = new Channel(fd,FDEvent::ReadEvent,processRead,processWrite,processDestroy,this);
    evloop->addTask(channel,ElenmType::ADD);
}


TcpConnection::~TcpConnection(){
    if(readBuf && writeBuf &&
        readBuf->readableSize() == 0 &&
        writeBuf->readableSize() == 0){

            if(readBuf != nullptr){
                delete readBuf;
                readBuf = nullptr;
            }
            if(writeBuf != nullptr){
                delete writeBuf;
                writeBuf = nullptr;
            }
            if(request != nullptr){
                delete request;
                request = nullptr;
            }
            if(response != nullptr){
                delete response;
                response = nullptr;
            }
            if(evloop != nullptr){
                evloop->freeChannel(channel);
            }
        }

    Debug("连接断开,释放资源,gameover,connName: %s",name.data());
}

//读回调
int TcpConnection::processRead(void* arg){

    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    //获取文件描述符
    int socketFd = conn->channel->getSocketfd();
    //将数据写入读缓冲区
    int len = conn->readBuf->readSocket(socketFd);
    Debug("收到的http请求数据:%s\n %s",conn->name.data(),conn->readBuf->date());
    if(len > 0){
        //接收到http请求，解析http请求
 #ifdef MSG_SED_AUTO
        //修改fd的写检测事件
        conn->channel->writeEventEnable(true);
        //需要等到下一轮epoll_wait 才会触发
        //弊端：无法及时发送，会积压在缓冲区，积压过多会导致内存紧张
        conn->evloop->addTask(conn->channel,ElenmType::MODIFY);
 #endif 
        int flag = conn->request->parseHttpRequest(conn->readBuf,conn->response,conn->writeBuf,socketFd);
        if(!flag){
        //失败返回404
        const char* errMsg = "HTTP/1.1 400 Bad Request\r\n\r\n";
        conn->writeBuf->appendString(errMsg);
        }
    }

 #ifndef MSG_SED_AUTO
    //断开连接，不能立刻发送数据，需要等到下一轮epoll_wait
    conn->evloop->addTask(conn->channel,ElenmType::DELETE);
 #endif

    return len;
}

//写回调
int TcpConnection::processWrite(void* arg){
    Debug("开始发送数据,基于写事件发送...");
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    //发送数据
    int len = conn->writeBuf->sendData(conn->channel->getSocketfd());
    if(len > 0){
        //判断数据是否发送完毕
        if(conn->writeBuf->readableSize() == 0){
            //不再检测写事件
            conn->channel->writeEventEnable(false);
            //将修改加入任务队列
            conn->evloop->addTask(conn->channel,ElenmType::MODIFY);
            //从任务队列删除这个节点
            conn->evloop->addTask(conn->channel,ElenmType::DELETE);
        }
    }
    return len;
}


//内存回收回调
int TcpConnection::processDestroy(void* arg){

    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    if(conn != nullptr){
        delete conn;
        return 0;
    }
    return -1;
}













