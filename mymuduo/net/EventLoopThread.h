/*
 * @FilePath: EventLoopThread.h
 * @Author: just
 * @Date: 2023-06-26 19:55:07
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:32:55
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
/*
 * @FilePath: EventLoopThread.h
 * @Author: jt
 * @Date: 2023-06-26 17:09:45
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-26 19:55:06
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: one loop per thread
 */
#ifndef MYMUDUO_NET_EVENTLOOPTHREAD_H
#define MYMUDUO_NET_EVENTLOOPTHREAD_H
#pragma once

#include <mutex>
#include <functional>
#include <string>
#include <condition_variable>

#include "mymuduo/base/noncopyable.h"
#include "mymuduo/base/Thread.h"

namespace mymuduo
{

class EventLoop;


class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

    bool existing_;

    EventLoop* loop_;

    Thread thread_;

    // 保证线程安全
    // 互斥锁
    std::mutex mutex_;
    // 条件变量
    std::condition_variable cond_;
    
    ThreadInitCallback callback_;

    
};

} // namespace mymuduo


#endif