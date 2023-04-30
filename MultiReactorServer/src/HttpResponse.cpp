#include"../include/HttpResponse.h"
#include<string.h>


HttpResponse::HttpResponse(){
    statusCode = StatusCode::Unknow;
    fileName = "";
    headers.clear();
    sendDatafunc = nullptr;
}


HttpResponse::~HttpResponse(){}


//添加http响应头键值对
void HttpResponse::addHeader(const std::string key,std::string value){
    if(!key.empty() && !value.empty()){
        headers.insert(std::make_pair(key,value));
    }
}


//准备相应数据
void HttpResponse::prepareMsg(Buffer* sendBuf, int socket){
    //状态行
    char tmp[1024] = {0};
    int code = static_cast<int>(statusCode);
    sprintf(tmp,"HTTP/1.1 %d %s\r\n",code,inf0.at(code).data());
    sendBuf->appendString(tmp,strlen(tmp));

    //响应头
    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        //清空数据
        bzero(tmp,1024);
        // int sprintf(char *str, const char *format, ...) 发送格式化输出到 str 所指向的字符串
        sprintf(tmp,"%s: %s\r\n",it->first.data(),it->second.data());
        //将数据写入缓冲区
        sendBuf->appendString(tmp,strlen(tmp));
    }
    //空行
    sendBuf->appendString("\r\n");
#ifndef MSG_SEND_AUTO
    //发送数据
    sendBuf->sendData(socket);
#endif
    //回复的响应体
    sendDatafunc(fileName,sendBuf,socket);
}


 
