#include"../include/TcpServer.h"
#include"../include/Log.h"
#include"../include/TcpConnection.h"
#include<arpa/inet.h>
#include<functional>


void TcpServer::setListen(){

    //创建socket
    lfd = socket(AF_INET,SOCK_STREAM,0);
    if(lfd == -1){
        perror("socket");
        return;
    }

    //设置端口复用
    int opt = 1;
    int ret = setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(ret == -1){
        perror("setsockeopt");
        return;
    }

    //绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    ret = bind(lfd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret == -1){
        perror("bind");
        return;
    }

    printf("setListen\n");
    //设置监听 系统里最大监听树为128，超过仍是128
    ret = listen(lfd,128);
    if(ret == -1){
        perror("listen");
        return;
    }
}


void TcpServer::run(){
    Debug("服务器程序已启动...");
    //启动线程池
    pool->run();
    
    auto func = std::bind(&TcpServer::acceptConn,this);
    //主线程监听套接字的任务参数为TcpServer对象
    Channel* channel = new Channel(lfd,FDEvent::ReadEvent,func,nullptr,nullptr,this);
    mainLoop->addTask(channel,ElenmType::ADD);
    //启动反应堆
    mainLoop->run();

}


int TcpServer::acceptConn(){
    int cfd = accept(lfd,NULL,NULL);
    //从线程池中取出一个子线程的反应堆实例，并处理这个通信事件cfd
    EventLoop* evloop = pool->takeWorkerEventLoop();
    // 处理
    new TcpConnection(cfd,evloop);
    return 0;
}



















