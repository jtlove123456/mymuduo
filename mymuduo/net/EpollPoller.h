/*
 * @FilePath: EpollPoller.h
 * @Author: jt
 * @Date: 2023-06-25 11:11:06
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-26 10:57:33
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#ifndef MYMUDUO_NET_EPOLLPOLLER_H
#define MYMUDUO_NET_EPOLLPOLLER_H

#include <vector>
#include <sys/epoll.h>

#include "mymuduo/net/Poller.h"

namespace mymuduo
{
class EventLoop;

class EpollPoller : public Poller{

public:
    EpollPoller(EventLoop* loop_);
    ~EpollPoller() override;
    /**
     * @brief 调用epoll_wait()等待事件的发生，遍历所有事件，
     * 将相应的channel对应的fd上发生的事件设置到channel的revent中
     * 
     * 每个poller中有多个channel,每个channel上注册了相应的感兴趣事件
     * @param activateChannels 
     * @param timeout 
     * @return Timestamp 
     */
    Timestamp poll(ChannelLists* activateChannels, int timeout) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:

    void fillActivateChannels(int numEvents, ChannelLists* activateChannels);

    // epoll_ctl,根据op来修改channel
    void update(int op, Channel* channel);
    
    // epoll事件列表的初始大小16
    static const int kInitEventListSize = 16;
    // epoll事件数组，保存epoll上发生的事件
    using EventList = std::vector<struct epoll_event>;
    EventList events_;
    // 创建出来的epoll对象的fd
    int epollfd_;
};

} // namespace mymuduo

#endif