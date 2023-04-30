#include"../include/HttpRequest.h"
#include"../include/Log.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/sendfile.h>


HttpRequest::HttpRequest(){
    reset();
}

HttpRequest::~HttpRequest(){}

//重置HttpRequest结构体
void HttpRequest::reset(){
    method = "";
    url = "";
    version = "";
    headers.clear();
    curState = ProcessState::ParseRequestLine;
}

//添加状态头，传入指向堆内存的指针
void HttpRequest::addHeader(const std::string key,const std::string value){
    if(key.empty() || value.empty()){
        return;
    }
    headers.insert(std::make_pair(key,value));
}

//获取请求头的value(堆内存地址)
std::string HttpRequest::getHeader(const std::string key){
    if(key.empty()){
        return "";
    }
    auto it = headers.find(key);
    if(it == headers.end()){
        return "";
    }
    return it->second; 
}

//解析请求行
bool HttpRequest::parseRequestLine(Buffer* readBuffer){
    //读出请求行，保存字符串结束地址(找\r\n)
    char* end = readBuffer->findCRLF();
    //保存字符串起始地址
    char* start = readBuffer->date();
    //请求行总长度
    int linesize = end - start;

    if(linesize){
        //创建了一个函数对象 methodFunc，用于调用 HttpRequest 类的成员函数 setMethod()，
        //并将该函数与当前对象 this 绑定在一起，以便后续调用。
        //同时指定了一个占位符 _1，表示该函数接受一个参数。
        auto methodFunc = bind(&HttpRequest::setMethod,this,std::placeholders::_1);
        start = splitRequestline(start,end," ",methodFunc);

        auto urlFunc = bind(&HttpRequest::setUrl,this,std::placeholders::_1);
        start = splitRequestline(start,end," ",urlFunc);

        auto versionFunc = bind(&HttpRequest::setVersion,this,std::placeholders::_1);
        start = splitRequestline(start,end,nullptr,versionFunc);

        //为解析请求头做准备
        readBuffer->readPosIncrease(linesize + 2);
        //修改状态
        setState(ProcessState::ParseRequestHeader);
        return true;
    }
    return false;
}

//解析请求头
bool HttpRequest::parseRequestHeader(Buffer* readBuffer){

    char* end = readBuffer->findCRLF();
    //判断请求行是否被解析完
    if(end != nullptr){
        char* start = readBuffer->date();
        int lineSize = end - start;
        //查找": "的位置
        char* mid = static_cast<char*>(memmem(start,lineSize,": ",2));
        if(mid != nullptr){
            int keyLen = mid - start;
            int valueLen = end - mid - 2;
            if(keyLen > 0 && valueLen > 0){
                //定义std::string 变量并使用构造函数经进行初始化
                std::string key(start,keyLen);
                std::string value(mid+2,valueLen);
                //加入状态头
                addHeader(key,value);
            }
            readBuffer->readPosIncrease(lineSize + 2);
        }
        else{
            //请求头（首部行）解析完了，跳过空行(\r\n)
            readBuffer->readPosIncrease(2);
            //修改解析状态
            //忽略POST请求，按照get请求处理
            curState = ProcessState::ParseRequestDone;            
        }
        return true;
    }
    return false;
}

//解析并处理Http请求
bool HttpRequest::parseHttpRequest(Buffer* recvBuffer,HttpResponse* response,Buffer* sendBuffer,int socket){

    bool flag = true;
    while (curState != ProcessState::ParseRequestDone)
    {
        //根线程当前解析状态，调用不同的解析函数
        switch (curState)
        {
        case ProcessState::ParseRequestLine:
            flag = parseRequestLine(recvBuffer);             
            break;
        case ProcessState::ParseRequestHeader:
            flag = parseRequestHeader(recvBuffer);
            break;
        case ProcessState::ParseRequestBody:
            break;        
        default:
            break;
        }
        
        if(!flag){
            return flag;
        }
        //判断是否解析完毕，如果解析完毕，需要回复数据
        if(curState == ProcessState::ParseRequestDone){
            //解析请求数据，根据reques中的数据处理
            processHttpRequest(response);
            //组织响应并发送数据
            response->prepareMsg(sendBuffer,socket);
        }
    }
    //还原线程解析状态，保证继续处理第二条请求
    curState = ProcessState::ParseRequestLine;
    return flag;    
}


//处理Http请求(解析后的动作)
bool HttpRequest::processHttpRequest(HttpResponse* response){
    //不区分大小写，返回0表示字符串相等
    if(strcasecmp(method.data(),"get") != 0){
        return false;
    }

    //unicode转char
    url = unicodeTochar(url);

    //处理客户端静态资源（目录或文件）
    const char* file = nullptr;
    if(strcmp(url.data(), "/") == 0){
        file = "./";
    }
    else{
        //取出首位的/
        file = url.data() + 1;
    }

    //获取文件的属性
    struct stat st;
    int ret = stat(file,&st);
    if(ret == -1){
        //文件不存在，回复404
        response->setfileName("404.html");
        response->setStatusCode(StatusCode::NotFound);
        response->addHeader("Content-Type",getFileType(".html"));
        response->sendDatafunc = sendFile;
        return false;    
    }

    //200 OK
    response->setfileName(file);
    response->setStatusCode(StatusCode::OK);

    //判断文件属性
    if(S_ISDIR(st.st_mode)){
        //把目录内容发送给客户端
        response->addHeader("Content-Type",getFileType(".html"));
        response->sendDatafunc = sendDir;
    }
    else{
        response->addHeader("Content-Type",getFileType(file));
        response->addHeader("Content-Length",std::to_string(st.st_size));
        response->sendDatafunc = sendFile;
    }
    return true;
}


//将unicode转为char
std::string HttpRequest::unicodeTochar(std::string transform){
    const char* from = transform.data();
    std::string to = std::string();
    for(; *from != '\0'; ++from){
        //isdigit 判断是不是16进制格式，取值0~f
        if(from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])){
            //将第二个、第三个字符转换为十六进制
            to.append(1,hexToDec(from[1] * 16 + hexToDec(from[2])));
            from += 2;
        }
        else{
            to.append(1,*from);
        }
    }
    to.append(1,'\0');
    return to;
}

//根据文件后缀返回文件类型
const std::string HttpRequest::getFileType(const std::string filename){
    //自左向右查找'.'字符，不存在则返回NULL
    // char *strrchr(const char *str, int c) 在参数 str 所指向的字符串中搜索
    //最后一次出现字符 c（一个无符号字符）的位置。
    const char* dot = strrchr(filename.data(),'.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

//发送目录信息
void HttpRequest::sendDir(std::string dirName,Buffer* sendBufer,int cfd){
    char buf[4096] = {0};
    //写入html文件前半部分
    sprintf(buf,"<html><head><title>%s</title></head><body><table>",dirName.data());

    //作为传出参数，返回struct dirent* nameList[]
    struct dirent** nameList;
    int num = scandir(dirName.data(),&nameList,NULL,alphasort);
    for(int i = 0; i < num; ++i){
        char* name = nameList[i]->d_name;
        char fullPath[1024] = {0};
        sprintf(fullPath,"%s/%s",dirName.data(),name);
        //根据不同文件类型处理
        struct stat st;
        stat(fullPath,&st);
        if(S_ISDIR(st.st_mode)){
            //判断是目录
            sprintf(buf+strlen(buf),
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
                name,name,st.st_size);
        }
        else{
            sprintf(buf+strlen(buf),
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name,name,st.st_size);
        }

        sendBufer->appendString(buf);

 #ifndef MSG_SEND_AUTO
        //发送数据
        sendBufer->sendData(cfd);
 #endif
        //清空缓冲区
        memset(buf,0,sizeof(buf));
        //释放函数创建的内存
        free(nameList[i]);
    }

    sprintf(buf,"</table></body></html>");
    //将数据写如缓冲区
    sendBufer->appendString(buf);
 #ifndef MSG_SEND_AUTO
        //发送数据
        sendBufer->sendData(cfd);
 #endif
        free(nameList);
}

//发送文件
void HttpRequest::sendFile(std::string fileName,Buffer* sendBuf,int cfd){

    int fd = open(fileName.data(),O_RDONLY);
    assert(fd > 0);    
    if(fd <= 0){
        perror("open");
        return;
    }
#if 1
//使用自己写的发送函数
    while (true)
    {
        char buf[1024] = {0};
        int len = read(fd,buf,sizeof(buf));
        if(len > 0){
            sendBuf->appendString(buf,len);
 #ifndef MSG_SEND_AUTO
        sendBuf->sendData(cfd);
 #endif
        }
        else if(len == 0){
            break;
        }
        else{
            close(fd);
            perror("read");
        } 
    }

# else
//使用sendfile发送函数，零拷贝.sendfile内部缓冲区有限

    //确定文件大小
    off_t size = lseek(fd,0,SEEK_END);
    //将文件指针移到头部
    lseek(fd,0,SEEK_SET);
    off_t off = 0;
    //缓冲区有限故需要循环发送
    while (off < size)
    {
        int ret = sendfile(cfd,fd,&off,size-off);
        usleep(10);
        Debug("ret value: %d\n",ret);
        if(ret == -1 && errno != EAGAIN){
            //产生异常
            perror("sendfile");
            break;
        }
    }   

#endif
        close(fd);
}

//分隔请求行，将请求行的参数保存在当前类
//返回新的开始地址
char* HttpRequest::splitRequestline(const char* start,const char* end,
            const char* sub,std::function<void(std::string)>func){
        
        //将常量指针转化为常指针
        char* space = const_cast<char*>(end);
        if(sub != nullptr){
            int sublen = strlen(sub);
            //memmem()在内存中寻找匹配另一块内存内容的第一个位置
            space = static_cast<char*>(memmem(start,end-start,sub,sublen));
            //assert计算表达式为假则打印出错信息
            assert(space != nullptr);
        }
        int length = space - start;
        //调用回调函数
        func(std::string(start,length));
        return space + 1;
}

//将十六进制转换为十进制
int HttpRequest::hexToDec(char ch){
    if(ch >= '0' && ch <= '9'){
        return ch - '0';
    }
    else if(ch >= 'a' && ch <= 'f'){
        return ch - 'a' + 10;
    }
    else if(ch >= 'A' && ch <= 'F'){
        return ch - 'A' + 10;
    }

    return 0;
}