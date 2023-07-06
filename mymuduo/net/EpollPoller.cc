#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <string.h> //memset

#include "mymuduo/net/EpollPoller.h"
#include "mymuduo/base/Logger.h"
#include "mymuduo/net/Channel.h"

using namespace mymuduo;

// 标识channel对象在Poller中的状态
const int kNew = -1; //channel上的fd未添加到epoll上
const int kAdded = 1;//channel上的fd已添加到epoll上
const int kDeleted = 2;//channel上的fd已经在epoll中删除


// epoll_create1 产生一个epoll 实例，返回的是实例的句柄。
// flag 可以设置为0 或者EPOLL_CLOEXEC，为0时函数表现与epoll_create一致
EpollPoller::EpollPoller(EventLoop* loop_):
             Poller(loop_),
             events_(kInitEventListSize),
             epollfd_(::epoll_create1(EPOLL_CLOEXEC)) //创建epoll对象
{
    if(epollfd_ < 0)
    {
        LOGFATAL("EPollPoller::EPollPoller create fail %d\n", errno);
    }
}

EpollPoller::~EpollPoller()
{
    // 关闭创建的epoll句柄epollfd
    // unistd.h下
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(ChannelLists* activateChannels, int timeout)
{ 
    int numEvents = ::epoll_wait(epollfd_, 
                        &*events_.begin(),
                        static_cast<int>(events_.size()),
                        timeout);
    int savedError = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0){
        // 将发生的事件设置到channel上
        fillActivateChannels(numEvents, activateChannels);
        // 发生的事件超过了最大的events,进行扩容
        if(static_cast<int>(events_.size()) == numEvents){
            events_.resize(events_.size() * 2);
        }
    }else if(numEvents == 0){
        // 没有事件发生
        LOGINFO("nothing happened");
    }else{
        // 出现错误
        if(savedError != EINTR){
            errno = savedError;
            LOGERROR("EPollPoller::poll()");
        }
    }
    return now;
}

void EpollPoller::fillActivateChannels(int numEvents, ChannelLists* activateChannels)
{
    for(int i = 0; i < numEvents; ++i){
        /**
         * @brief 
         * typedef union epoll_data
            {
            void *ptr; 指针指向的对象 channel
            int fd;  epoll上监听的fd， channel->fd()
            uint32_t u32;
            uint64_t u64;
            } epoll_data_t;
         */
        Channel* activateChannel = static_cast<Channel*>(events_[i].data.ptr);
        activateChannel->setRevents(events_[i].events);
        activateChannels->push_back(activateChannel);
    }
}


void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->getIndex();
    if(index == kNew || index == kDeleted){
        int fd = channel->getFd();
        if(index == kNew){
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }else{
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }else{ // kadded , mod/delete

        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->getFd();
    const int index = channel->getIndex();
    assert(index == kAdded || index == kDeleted);
    // 从集合中移除
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if(index == kAdded){
        // 从epoll中移除
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EpollPoller::update(int op, Channel* channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    // 将channel上感兴趣的事件注册到poller中
    event.events = channel->getEvents();
    event.data.fd = channel->getFd();
    event.data.ptr = channel;
    int fd = channel->getFd();
    if(::epoll_ctl(epollfd_, op, fd, &event) < 0){
        if(op == EPOLL_CTL_DEL){ //删除未成功
            LOGERROR("epoll_ctl delete fd = %d\n", fd);
        }else{
           LOGFATAL("epoll_ctl mod/add fd = %d\n", fd); 
        }
    }
}