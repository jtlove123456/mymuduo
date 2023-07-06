#include <semaphore.h>

#include "mymuduo/base/Thread.h"
#include "mymuduo/base/util.h"

using namespace mymuduo;

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string& name):
                started_(false),
                joined_(false),
                tid_(0),
                name_(name),
                func_(std::move(func))
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_){
        // 分离线程
        thread_->detach();
    }
}

// 一个Thread对象，记录的就是一个新线程的详细信息
void Thread::start()
{
    started_ = true;
    // 创建线程，使用信号量，来保证线程安全
    ::sem_t sem;
    ::sem_init(&sem, false, 0);
    // 开启线程,使用lamda表达式
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        // 获取当前线程id
         tid_ = util::getTid();
         ::sem_post(&sem);
          // 开启一个新线程，专门执行该线程函数
         func_();
     }));
    // 这里必须等待获取上面新创建的线程的tid值
    ::sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty()){
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }

}