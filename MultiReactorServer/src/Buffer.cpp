#include"../include/Buffer.h"
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>


Buffer::Buffer(int size):capacity(size),readPos(0),writePos(0){
    //为缓冲区分配内存空间
    data = static_cast<char *>(malloc(sizeof(char) * size));
    memset(data,0,size);
}


Buffer::~Buffer(){
    if(data != nullptr){
        free(data);
        data = nullptr;
    }
}

void Buffer::resize(int newsize){
    int readable = readableSize();
    int writabele = writableSize();

    if(writabele >= newsize){
        return;
    }
    else if(readPos + writabele >= newsize){
        //整理缓存区，避免孤独分配空间
        //将未读数据移动到缓冲区首部，并将未覆盖数据清0
        memcpy(data,data+readPos,readable);
        memset(data+readable,0,capacity - readable);
        //修改readPox、writePox
        readPos = 0;
        writePos = readable;        
    }
    else{
        //realloc 第二个参数必须要更大一些
        char* ptr = static_cast<char*>(realloc(data, capacity+newsize));
        if(ptr == nullptr){
            return ;
        }
        //重置数据
        memset(data + capacity,0,newsize);
        data = static_cast<char*>(ptr);
        capacity += newsize;

    }


}


char* Buffer::findCRLF(){
     char* ptr = (char*)memmem(data+readPos,readableSize(),"\r\n",2);
     return ptr;

}


int Buffer::sendData(int socket){
    int readable = readableSize();
    if(readable > 0){
        //MSG_NOSIGNAL 当套接字通信的一端关闭连接后，会阻止内核产生 SIGPIPE信号(管道破裂)
        int len = send(socket,data+readPos,readable,MSG_NOSIGNAL);
        if(len > 0){
            readPos += len;
            //暂停1微秒
            usleep(1);
        }
        return len;
    }
    return -1;
}


int Buffer::readSocket(int fd){
    if(data == nullptr){
        return -1;
    }

    //vec数组用来缓存
    struct iovec vec[2];
    int writable = writableSize();
    //先存放buffer
    vec[0].iov_base = data + writePos;
    vec[0].iov_len = writable;
    //放满则放入第二个
    char* ptr = (char*)malloc(40960);
    vec[1].iov_base = ptr;
    vec[1].iov_len = 40960;
    // 将数据读入struct iovec 数组，前一个iovec满后，读入后一个iovec
    int len = readv(fd,vec,2);
    //修正数据
    if(len == -1){
        return -1;
    }
    else if(len <= writable){
        writePos = writePos + len;
    }
    else{
        writePos = capacity;
        appendString(ptr,len - writable);
    }

    if(ptr != nullptr){
        free(ptr);
        ptr = nullptr;
    }
    
    return len;
}



int Buffer::appendString(const char* str){
    int len = strlen(str);
    return appendString(str,len);
}


int Buffer::appendString(const char* str,int len){
    if(len <= 0 || data == nullptr){
        return -1;
    }
    resize(len);
    memcpy(data + writePos,str,sizeof(char)*len);
    writePos += len;
    return 0;
}


int Buffer::appendString(const std::string str){
    return appendString(str.data());
}


