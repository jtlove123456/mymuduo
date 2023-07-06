/*
 * @FilePath: EventLoop.h
 * @Author: just
 * @Date: 2023-06-25 20:03:36
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:32:21
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
/*
 * @FilePath: EventLoop.h
 * @Author: jt
 * @Date: 2023-06-19 15:53:19
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 20:02:42
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#ifndef MYMUOD_NET_EVENTLOOP_H
#define MYMUOD_NET_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

#include "mymuduo/base/Timestamp.h"
#include "mymuduo/base/noncopyable.h"
#include "mymuduo/net/Poller.h"
#include "mymuduo/net/Channel.h"
#include "mymuduo/base/util.h"

namespace mymuduo
{

/**
 * @brief 事件循环
 * 主要是管理channel 和 poller
 */
class EventLoop : noncopyable{
    
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    
    // 退出事件循环
    void quit();

    // 在当前loop中执行回调
    void runInLoop(Functor cb);

    // 把回调cb放入队列中，唤醒cb所在loop线程，执行回调
    void queueInLoop(Functor cb);

    // 唤醒loop所在线程
    // 通过给该线程写一个数据
    void wakeup();

    // 底层使用poller相应的方法，为channel注册不同的事件
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // 判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread(){return threadId_ == util::getTid();}

    Timestamp getPollerReturnTime(){return pollerReturnTime_;}

private:

    void handleRead(); // wake up, 用于唤醒线程

    void doPendingFunctors(); //处理回调事件

    // 原子操作,使用CAS实现
    // 标识事件循环是否开启
    std::atomic_bool looping_;
    // 事件循环退出标识
    std::atomic_bool quit_;

    // 记录当前loop所在的线程id
    const pid_t threadId_;

    // poller返回发生事件的channel的时间点
    Timestamp pollerReturnTime_;
    // 事件分发器
    std::unique_ptr<Poller> poller_;
    
    // 标识当前loop是否有需要执行的回调操作
    std::atomic_bool callingPendingFunctors_;
    // 存储loop需要执行的所有的回调操作
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_; //互斥锁，用来保护上面vector容器的线程安全操作

    // 用于唤醒subreator中工作线程
    // 当mainLoop获取一个新用户的channel，即新用户连接进来时
    // 通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    using ChannelList = std::vector<Channel*>;
    // 已经连接的所有用户，用于通信的fd所对应的channel
    ChannelList activeChannels_;

    Channel* currentActiveChannel_;

};

  
    
} // namespace mymuduo

#endif