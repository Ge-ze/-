#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/TcpServer.h"
 

int main(int argc, const char* argv[])
{
    // const char* arr[3] = {"NULL", "10001", "./res"};
    // argv = arr;
    // argc = 3;
    // if (argc < 3) 
    // {
    //     printf("./a.out port path\n");
    //     return -1;
    // }
  
    unsigned short port = atoi("8080");
    chdir("/home/lili/VScode/ReactorHttp/res"); 
    // 创建服务器
    TcpServer* server = new TcpServer(port, 4);
    server->run();

    return 0;
}