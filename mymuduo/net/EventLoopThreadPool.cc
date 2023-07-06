/*
 * @FilePath: EventLoopThreadPool.cc
 * @Author: just
 * @Date: 2023-06-26 21:36:52
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:33:23
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#include "mymuduo/net/EventLoopThreadPool.h"
#include "mymuduo/net/EventLoop.h"
#include "mymuduo/net/EventLoopThread.h"


using namespace mymuduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                             const std::string& nameArg):
                    baseLoop_(baseLoop),
                    name_(nameArg),
                    started_(false),
                    numThreads_(0),
                    next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    
}
// 开启线程池
void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_ = true;
    // 根据设置的线程数，创建线程，并添加到集合中
    for(int i = 0; i < numThreads_; ++i){
        char buf[name_.size() + 32] ;
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str() , i);
        // 创建线程，并在线程中创建eventloop，开启loop循环
        EventLoopThread *evlt = new EventLoopThread(std::move(cb), buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(evlt));
        loops_.push_back(evlt->startLoop());
    }
    // 只有主线程，在主线程(mainloop)中执行线程初始化回调
    if(numThreads_ == 0 && cb){
        cb(baseLoop_);
    }
}
// 轮询算法，轮询获取子线程(工作线程)处理相应的客户端连接
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseLoop_;

    if(!loops_.empty()){
        loop = loops_[next_];
        ++next_;
        if(next_ >= static_cast<int>(loops_.size())){
            next_ = 0;
        }
    }
    // 没有子线程，则返回的为baseLoop_，即mainloop
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty()){ //，没有子线程，返回主线程
        return std::vector<EventLoop*>(1, baseLoop_);
    }else{
        return loops_;
    }
}