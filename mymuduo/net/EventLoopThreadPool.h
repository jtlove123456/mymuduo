#ifndef MYMUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MYMUDUO_NET_EVENTLOOPTHREADPOOL_H
#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "mymuduo/base/noncopyable.h"


namespace mymuduo
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();
    // 开启线程池
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // 设置线程数量，不设置线程数量，就只有一个主线程(main函数中的线程)，所有的处理都在主线程中完成
    void setThreadNum(int numThreads){numThreads_ = numThreads;}

    // 轮询算法，轮询获取子线程(工作线程)处理相应的客户端连接
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();

    bool started(){return started_;}
    /*
    * 普通函数或成员函数（非静态成员函数）前均可加const修饰，
      表示函数的返回值为const，不可修改

      函数后加const：只有类的非静态成员函数后可以加const修饰，
      表示该类的this指针为const类型，
      不能改变类的成员变量的值，即成员变量为read only（例外情况见2），
      任何改变成员变量的行为均为非法。此类型的函数可称为只读成员函数
    */
    const std::string& name() const {return name_;}


private:

    // mainloop,只有一个, 通过主线程main中创建，传给TcpServer -> EventLoopThreadPool
    EventLoop* baseLoop_;

    // 一一对应的
    // 工作线程，subreator中的线程
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    // subloop
    std::vector<EventLoop*> loops_;

    // 线程池名
    std::string name_;
    bool started_;
    // 线程池线程数量
    int numThreads_;
    // 下一个线程的位置
    int next_;
};

} // namespace mymuduo


#endif