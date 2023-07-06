/*
 * @FilePath: Poller.h
 * @Author: jt
 * @Date: 2023-06-25 10:25:02
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 11:01:36
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion:  Poller接口，epoll,poll的一个抽象
 */

#ifndef MYMUDUO_NET_POLLER_H
#define MYMUDUO_NET_POLLER_H

#include <vector>
#include <unordered_map>

#include "mymuduo/base/noncopyable.h"
#include "mymuduo/base/Timestamp.h"

namespace mymuduo
{

class Channel;
class EventLoop;
/**
 * @brief 
 * I/O 多路复用的基类
 * 它没有拥有channel对象
 */
class Poller : noncopyable{
    
    public:
        using ChannelLists = std::vector<Channel*>;

        Poller(EventLoop* loop_);
        virtual ~Poller();

        /**
         * @brief 等待事件的发生，并将发生的事件设置（setrevent）到channel上
         * 必须在loop线程中调用
         * @param activateChannels
         * @param timeout 
         * @return Timestamp 
         */
        virtual Timestamp poll(ChannelLists* activateChannels, int timeout) = 0;

        /**
         * @brief 更新channel上感兴趣的IO事件
         * 必须在loop线程中调用
         * epoll_ctl()的封装
         * @param channel 需要更新的channel
         */
        virtual void updateChannel(Channel* channel) = 0;

        /**
         * @brief 移除channel，既将fd从poller中移除
         * epoll_ctl()的封装
         * @param channel 需要移除的channel
         */
        virtual void removeChannel(Channel* channel) = 0;

        /**
         * @brief channel是否在poller上注册过
         * 
         * @param channel 
         * @return true 
         * @return false 
         */
        virtual bool hasChannel(Channel* channel) const;
        // 默认构造
        static Poller* newDefaultPoller(EventLoop* loop);

    protected:
       // 每一个poller上会监听多个Channel
       using ChannelMap = std::unordered_map<int, Channel*>;
       //polle->epollfd上注册的fd(channel)集合
       ChannelMap channels_;
       
    private:
      //poller所在的loop
      EventLoop* ownerLoop_;
};

} // namespace mymuduo



#endif

