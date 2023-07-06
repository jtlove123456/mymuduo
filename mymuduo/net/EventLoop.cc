#include <sys/eventfd.h>

#include "mymuduo/net/EventLoop.h"
#include "mymuduo/base/Logger.h"

using namespace mymuduo;

// 防止一个线程创建多个EventLoop   thread_local
thread_local EventLoop* t_loopInThisThread = nullptr; 

// 定义默认的Poller IO复用接口的超时时间10s
const int kPollerTimeMs = 10000; 

//创建wakeupfd，用来notify唤醒subReactor处理新来的channel
int createEventfd()
{

    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOGFATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    threadId_(util::getTid()),
    poller_(Poller::newDefaultPoller(this)),
    callingPendingFunctors_(false),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentActiveChannel_(nullptr)  
{
    LOGDEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread){ //已经创建过
        LOGFATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    t_loopInThisThread = this;
    //绑定回调，用于唤醒线程
    // 监听channel上是否有读事件，监听到读事件，相应的线程就被唤醒
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}
// 开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOGINFO("EventLoop %p start looping\n", this);
    while(!quit_){
        activeChannels_.clear();
        // 调用epoll_wait()等待事件的发生，并将相应的事件设置到channel中
        pollerReturnTime_ = poller_->poll(&activeChannels_, kPollerTimeMs);
        // 遍历channel处理channel上方式的相应的事件
        for(Channel* channel : activeChannels_){
            currentActiveChannel_ = channel;
            // Poller监听的哪些channel发生事件了，
            // 然后上报给EventLoop，通知channel处理相应的事件
            currentActiveChannel_->handleEvent(pollerReturnTime_);
        }
        currentActiveChannel_ = NULL;
        // 执行loop中需要执行的回调操作
        /**
         * @brief Construct a new do Pending Functors object
         *  mainLoop 事先注册一个回调cb（需要subloop来执行）   
         *  wakeup subloop后，执行下面的方法，执行之前mainloop注册的cb操作
         */
        doPendingFunctors();
    }
     LOGINFO("EventLoop %p stop looping\n", this);
     looping_ = false;
}
    
// 退出事件循环
void EventLoop::quit()
{
   quit_ = true;
    // 如果是在其它线程中，调用的quit   
    // 在一个subloop(woker)中，调用了mainLoop(IO)的quit,需要唤醒相应的线程，
   if(!isInLoopThread()) {
        wakeup();
   }
}

// 在当前loop中执行回调
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()){ //在当前loop线程中，执行回调
        cb();
    }else{ //在非当前loop线程中执行cb , 就需要唤醒loop所在线程，执行cb
        queueInLoop(std::move(cb));
    }
}
// 把回调cb放入队列中，唤醒cb所在loop线程，执行回调
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    // 唤醒相应的需要执行上面回调操作的loop的线程了
    // callingPendingFunctors_:当前loop正在执行回调，但是loop又有了新的回调
    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }
}

// 唤醒loop所在线程
// 通过给该线程写一个数据
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one){
        LOGERROR("EventLoop::wakeup() writes %lu bytes instead of 8", n);
    }
}

void  EventLoop::handleRead() // wake up, 用于唤醒线程
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one){
         LOGERROR("EventLoop::handleRead() writes %lu bytes instead of 8", n);
    }
}

// 底层使用poller相应的方法，为channel注册不同的事件
void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void  EventLoop::doPendingFunctors() //处理回调事件
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    // todo
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor : functors){
         functor();
    }
    callingPendingFunctors_ = false;
}