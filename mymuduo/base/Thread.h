#ifndef MYMUDUO_BASE_THREAD_H
#define MYMUDUO_BASE_THREAD_H
#pragma once

#include <thread>
#include <string>
#include <atomic>
#include <functional>
#include <memory>

#include "mymuduo/base/noncopyable.h"


namespace mymuduo
{

class Thread : noncopyable
{

public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string& name = std::string());

    ~Thread();
    void start();

    void join();

    bool started(){return started_;}

    bool joined(){return joined_;}

    pid_t tid(){return tid_;}

    const std::string& name() const {return name_;}

    static int numCreated(){return numCreated_;}

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    // 线程id
    pid_t tid_;
    // 线程名
    std::string name_;
    // 使用智能指针管理该线程对象
    std::shared_ptr<std::thread> thread_;

    // 线程专门执行的函数
    ThreadFunc func_;
    // 线程编号
    static std::atomic_int numCreated_;
};

    
} // namespace mymuduo

#endif