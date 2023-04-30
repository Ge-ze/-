#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include"Buffer.h"
#include<map>
#include<functional>

/**
 * 状态码
*/
enum class StatusCode
{
    Unknow = 0,
    OK = 200,
    //永久转移
    MovePermanently = 301,
    //临时转移
    MoveTemporarily = 302,
    //请求错误
    BadRequest = 400,
    NotFound = 404
};

/**
 * http响应对象
*/
class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    //添加http响应头键值对
    void addHeader(const std::string key,std::string value);

    //准备相应数据
    void prepareMsg(Buffer* sendBuf, int socket);

    //设置状态码
    inline void setStatusCode(StatusCode statuscode){
        this->statusCode = statusCode;
    }

    //设置文件名
    inline void setfileName(std::string fileName){
        this->fileName = fileName;
    }

    //用于发送响应体的回调函数指针
    std::function<void(const std::string,Buffer*,int)> sendDatafunc;

private:
    //状态行中的状态码
    StatusCode statusCode;
    //文件名
    std::string fileName;
    //存储响应头Map
    std::map<std::string,std::string> headers;
    //状态码及其描述
    const std::map<int,std::string> inf0 = {
        {0,"Unknow"},
        {200,"OK"},
        {301,"MovePermanently"},
        {302,"MoveTemPorarily"},
        {400,"BadRequest"},
        {404,"NotFound"}
    };

};

 

















#endif