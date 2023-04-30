#ifndef LOG_H
#define LOG_H
#include<stdio.h>
#include<stdarg.h>

#define DEBUG 1

#if DEBUG

//使用do{}while(0)封装多条逻辑语句，防止编译报错


//输出日志信息
#define LOG(type,fmt,args...) \
    do{\
        printf("%s: %s@%s, line: %d\n***LogInfo[",type,__FILE__, __FUNCTION__,__LINE__);\
        printf(fmt,##args);\
        printf("]\n\n");\
    }while (0)

//输出调试信息
#define Debug(fmt,args...) LOG("DEBUG",fmt,##args)

//输出错误信息
#define Error(fmt,args...) do{LOG(ERROR,fmt,##args);exit(0);}while(0)



#else

//定义为空
#define LOG(type,args...)
#define Debug(fmt,args...)
#define Error(fmt,args...)



#endif

//匹配ifndef LOG_H
#endif