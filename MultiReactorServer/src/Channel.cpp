#include"../include/Channel.h"

/**
 * 位运算符中 |= 表示按位或并赋值，
 * &= 表示按位与并赋值，
 * & 表示按位与，
 * ~ 表示按位取反
*/
void Channel::writeEventEnable(bool flag){
    if(flag){
        //将 event_ 变量（该变量存储了所有关注的事件类型）
        //按位或上 FDEvent::WriteEvent，即将写事件标志位置为 1，
        //从而开启写事件
        event_ |= static_cast<int>(FDEvent::WriteEvent);
    }
    else{        
        event_ = event_ & (~(int)FDEvent::WriteEvent);
    }
}

  
bool Channel::isWriteEventable(){
    //将WriteEvent转换为int型后，和event_按位与运算
    //含义 检查event_中是否包含WriteEvent事件
    return event_ & static_cast<int>(FDEvent::WriteEvent);
}

