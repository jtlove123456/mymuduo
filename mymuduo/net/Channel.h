/*
 * @FilePath: Channel.h
 * @Author: jt
 * @Date: 2023-06-19 14:49:22
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 21:51:09
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#ifndef MYMUDUO_NET_CHANNEL_H
#define MYMUDUO_NET_CHANNEL_H

#include <memory>
#include <functional>

// 使用相对路径方式导入
// #include "../base/Timestamp.h"
// #include "../base/noncopyable.h"
// 在cmake中配置头文件搜索路径
#include "mymuduo/base/noncopyable.h"
#include "mymuduo/net/EventLoop.h"
#include "mymuduo/base/Timestamp.h"


namespace mymuduo{

// 前置声明，使用指针，并没有真正使用该对象
class EventLoop;

/**
 * @brief 封装监听/通信所使用的文件描述符fd
 * 将fd对事件的处理通过设置回调函数来实现
 * 文件描述符fd的事件注册通过EventLoop调用Poller来进行事件注册
 */

class Channel : noncopyable{

public:
    // typedef / using 来重命名
    // typedef std::function<void()> EventCallBack;
    using EventCallBack = std::function<void()>;
    // 读事件需要设置超时时间
    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    // 设置回调
    void setReadCallBack(ReadEventCallBack cb){readCallBack_ = std::move(cb);}
    void setWriteCallBack(EventCallBack cb){writeCallBack_ = std::move(cb);}
    void setCloseCallBack(EventCallBack cb){closCallBack_ = std::move(cb);}
    void setErrorCallBack(EventCallBack cb){errorCallBack_ = std::move(cb);}

    // 事件处理
    void handleEvent(Timestamp receiveTime);

    int getFd() const {return fd_;}
    int getEvents() const {return events_;}
    
    // for poller
    int getIndex() const {return index_;}
    void setIndex(int idx){index_ = idx;}

    // poller通过它将发生的事件告知channel
    void setRevents(int evt){revents_ = evt;}
    
    // 将这个通道绑定到由shared_ptr管理的所有者对象，
    // 防止所有者对象在handleEvent中被销毁。
    void tie(const std::shared_ptr<void>&);

    bool isNoneEvent() const {return events_ == kNoneEvent;}

    // fd上的读写事件的开启与关闭
    void enableReading(){events_ |= kReadEvent; update();}
    void disableReading(){events_ &= ~kReadEvent; update();}
    void enableWriting(){events_ |= kWriteEvent; update();}
    void disableWriting(){events_ &= ~kWriteEvent; update();}
    // 关闭fd上监听的所有事件
    void disableAll(){events_ = kNoneEvent; update();}

    bool isWriting() const {return events_ & kWriteEvent;}
    bool isReading() const {return events_ & kReadEvent;}

    EventLoop* ownerLoop(){return loop_;}

    // 移除channel
    void remove();

private:
    // 更新fd上的事件
    // 通过loop获得poller，poller调用event_ctl来注册事件
    void update();

    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    // 事件循环
    EventLoop* loop_;
    // 文件描述符
    const int fd_;
    // fd上注册的感兴趣事件
    int events_;
    // fd上发生的事件
    int revents_;

    // 用于标识channel的状态，knew, kadd, kdelete
    int index_;

    //todo
    std::weak_ptr<void> tie_;
    bool tied_;

    // 是否正在处理事件
    bool eventHandling_;

    // 事件回调
    ReadEventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closCallBack_;
    EventCallBack errorCallBack_;
};

}
#endif