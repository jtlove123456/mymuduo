/*
 * @FilePath: Channel.cc
 * @Author: just
 * @Date: 2023-06-26 16:54:39
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:30:02
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
/*
 * @FilePath: Channel.cc
 * @Author: jt
 * @Date: 2023-06-25 09:40:54
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-26 16:54:29
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */

#include <sys/epoll.h>

#include "mymuduo/net/Channel.h"
#include "mymuduo/net/EventLoop.h"
#include "mymuduo/base/Logger.h"

using namespace mymuduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd):
                loop_(loop),
                fd_(fd),
                events_(0),
                revents_(0),
                index_(-1),
                tied_(false),
                eventHandling_(false)           
{
}

Channel::~Channel()
{

}
// 事件处理
void Channel::handleEvent(Timestamp receiveTime)
{
    std::shared_ptr<void> guard;
    if(tied_){
        /**
         * @brief 
         * weak_ptr看成用来监视shared_ptr（强引用）的生命周期用。
         * 是一种对shared_ptr的扩充。
         * 检查weak_ptr所指向的对象是否存在，
         * 如果存在那么lock将返回一个指向该对象的shared_ptr（指向对象的强引用就会+1），
         * 如果指向对象不存在，lock会返回一个空的shared_ptr。
         * 通过lock可以将weak_ptr转化成shared_ptr使用。
         * 
         */
        guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
   loop_->removeChannel(this); 
}



void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOGINFO("channel handleEvent revents : %d\n", revents_);
    eventHandling_ = true;
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        if(closCallBack_){
            closCallBack_();
        }
    }
    if(revents_ & EPOLLERR){
        if(errorCallBack_){
            errorCallBack_();
        }
    }
    if(revents_ & (EPOLLIN | EPOLLRDHUP | EPOLLPRI)){
        if(readCallBack_) {
            readCallBack_(receiveTime);
        }
    }
    if(revents_ & EPOLLOUT){
        if(writeCallBack_){
            writeCallBack_();
        }
    }
    eventHandling_ = false;
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}