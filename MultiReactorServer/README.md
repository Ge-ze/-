# 基于多反应堆和线程池的高并发服务器

## 介绍

+ Channel 是对文件描述符fd的封装
+ Buffer 是读、写缓冲区
+ Dispatcher 是基类分发模型
+ EPollDispatcher 为epoll分发模型
+ PollDispatcher 为poll分发模型
+ SelectDispatcher 为select分发模型
+ EventLoop 是反应堆模型
+ HttpRequest Http请求对象，负责接收解析Http请求并组织响应信息
+ HttpResponse Http响应对象，负责响应Http请求
+ Log 负责控制打印日志
+ TcpConnection 负责维护TCP通信过程中所需要的对象
+ TcpServer 服务器对象，负责管理运行TCP服务器
+ ThreadPool 线程池模型，负责线程池中线程的管理
+ WorkerThread 工作线程




多反应堆+线程池高并发服务器 多反应堆，线程池，IO模型，TCP服务器，Http 
IO模型：主要用于套接字通信，接收和发送数据，TcpConnection封装的是建立连接后得到的用于通信的文件描述符。发送数据时，先将数据写入到一块内存buffer，在发送；接收数据也一样，先接收到buffer，再读。 TCP服务器：提供一个循环，一直监测有没有客户端连接到达，有没有客户端给服务器发送数据

## 架构

Reactor多反应堆+线程池
线程池中，主线程负责和客户端建立连接，子线程负责和客户端通讯，多反应堆体现在每个线程都对应一个反应堆模型。 两大模块：反应堆模型EventLoop和TcpConnection。 EventLoop中有分发器Dispatcher（poll，epoll，select），事件检测集合DispatcherData，任务队列，channle，channleMap。EventLoop是一个生产者消费者模型，其他线程添加任务，Dispatcher消费。 TcpConnection包括readBuf，writeBuf，httpRequest，httpResponse，EventLoop。主要负责通讯，接收到客户端数据放到readBuf，httpRequest解析readBuf中数据，httpResponse组织回复的数据块放到wirteBuf发送给客户端。


## 工作流程

将监听的fd绑定端口，fd想要工作需要放到反应堆模型里。首先将fd封装成channel，然后将channel添加到TaskQueue，EventLoop就会遍历TaskQueue，取出对应的任务节点，看任务节点中的type，基于type对这个节点进行添加删除修改。fd需要添加到反应堆模型的dispatcher中，对应的反应堆模型会对读事件进行检测，通过epoll_wait、select、poll传出的数据就可以知道fd对应的读事件触发了。就可以基于fd对应channelMap的数组下标，找到对应channel的地址。找到channel地址后就知道了fd对应读事件的回调函数。后续就可以和客户端建立连接，得到通信的文件描述符。 将通信文件描述符封装成channel类型，再把channel封装到tcpConnection里，tcpConnection需要在子线程里运行。需要从主线程访问线程池，从线程池中找出子线程，每个子线程都有一个EventLoop，把子线程的EventLoop也放到TcpConnection里（把子线程反应堆实例传给TcpConnection），因此就可以在TcpConnection里，通过channel里封装的通信fd和客户端进行通信（通信fd的事件监测都是通过子线程EventLoop实现的）