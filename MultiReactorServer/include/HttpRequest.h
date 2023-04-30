#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include"Buffer.h"
#include"HttpResponse.h"
#include<string>
#include<map>
#include<functional>

/**
 * 线程分析状态
*/
enum class ProcessState:char
{
    //分析请求行
    ParseRequestLine,
    //分析请求头
    ParseRequestHeader,
    //分析请求体
    ParseRequestBody,
    //分析请求完成
    ParseRequestDone
};

/**
 * http请求对象
*/
class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    //重置HttpRequest结构体
    void reset();

    //添加状态头，传入指向堆内存的指针
    void addHeader(const std::string key,const std::string value);

    //获取请求头的value(堆内存地址)
    std::string getHeader(const std::string key);

    //解析请求行
    bool parseRequestLine(Buffer* readBuffer); 

    //解析请求头
    bool parseRequestHeader(Buffer* readBuffer);

    //解析并处理Http请求
    bool parseHttpRequest(Buffer* recvBuffer,HttpResponse* response,Buffer* sendBuffer,int socket);

    //处理Http请求
    bool processHttpRequest(HttpResponse* response);

    //将unicode转为char
    std::string unicodeTochar(std::string transform);

    //根据文件后缀返回文件类型
    const std::string getFileType(const std::string filename);

    //发送目录信息
    static void sendDir(std::string dirName,Buffer* sendBufer,int cfd);

    //发送文件
    static void sendFile(std::string fileName,Buffer* sendBuf,int cfd);

    //设置方法
    inline void setMethod(std::string method){
        this->method = method;
    }

    inline void setUrl(std::string url){
        this->url = url;        
    }

    inline void setVersion(std::string version){
        this->version = version;
    }
    
    inline ProcessState getState(){
        return curState;
    }

    inline void setState(ProcessState state){
        this->curState = state;
    }
private:
    //分隔请求行，将请求行的参数保存在当前类
    //返回新的开始地址
    char* splitRequestline(const char* start,const char* end,
            const char* sub,std::function<void(std::string)>func);

    //将十六进制转换为十进制
    static int hexToDec(char ch);
private:
    std::string method;
    std::string url;
    std::string version;
    //存放首部行，第一个对应首部字段名，第二个对应值
    std::map<std::string,std::string> headers;
    //线程状态
    ProcessState curState;

};

#endif