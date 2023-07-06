#include "mymuduo/net/EventLoopThread.h"
#include "mymuduo/net/EventLoop.h"

using namespace mymuduo;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                    const std::string& name):
            existing_(false),
            loop_(nullptr),
            thread_(std::bind(&EventLoopThread::threadFunc, this), name),
            mutex_(),
            cond_(),
            callback_(std::move(cb))
{

}
EventLoopThread::~EventLoopThread()
{   
    existing_ = true;
    if(loop_ != nullptr){
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    // 开启线程，此时loop并没有创建
    thread_.start();

    // 等待线程调用绑定的线程回调函数threadFunc创建loop,然后返回创建的loop
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr){
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;

}
// 线程回调函数，只在单独的新线程里面运行的
void EventLoopThread::threadFunc()
{
     // 创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread
    EventLoop loop;
    // loop中有回调事件 todo
    if(callback_){
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    // 开启事件循环
    loop.loop(); // Eventloop loop -> poller poll -> epoll_wait()

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
