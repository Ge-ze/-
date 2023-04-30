#ifndef BUFFER_H
#define BUFFER_H

#include<iostream>
#include<stdlib.h>
#include<string>

/**
 * 封装读、写缓冲区 * 
*/
class Buffer
{ 
public:
    Buffer(int size);
    ~Buffer();

    //获取缓冲区剩余可写容量
    inline int writableSize(){
        return capacity - writePos;
    }

    //获取剩余可读容量
    inline int readableSize(){
        return writePos - readPos;
    }

    //得到读数据的起始地址
    inline char* date(){
        //基地址+地址偏移
        return data + readPos;
    }

    //将读位置增加n位且返回改变后的readPos
    inline int readPosIncrease(int n){
        readPos += n;
        return readPos;
    }

    //检查并拓展缓冲区(重置缓冲区大小)，size为数据长度
    void resize(int newsize);

    //找到换行符标记地址
    char* findCRLF();   

    //从socket读取数据并写入到缓存区，成功返回数据长度，失败返回-1
    int readSocket(int fd);

    //发送缓冲区数据，成功返回数据长度，失败返回-1
    int sendData(int socket);

    //向缓冲区写入数据,str不带'\0'
    int appendString(const char* str);

    //向缓冲区写入数据,str带'\0'
    int appendString(const char* str,int len);

    //向缓冲区写入数据,str带'\0'
    int appendString(const std::string str);    

private:
    //指针指向缓地址
    char* data;
    //缓存大小
    int capacity;
    //读的位置
    int readPos;
    //写的位置
    int writePos;
};

 






#endif