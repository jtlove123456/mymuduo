/*
 * @FilePath: Acceptor.h
 * @Author: jt
 * @Date: 2023-06-27 10:40:53
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-27 11:16:55
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 负责监听、接收客户端连接。只存在于mainLoop中
 */

#ifndef MYMUDUO_NET_ACCEPTOR_H
#define MYMUDUO_NET_ACCEPTOR_H
#pragma once

#include <functional>

#include "mymuduo/base/noncopyable.h"
#include "mymuduo/net/Socket.h"
#include "mymuduo/net/Channel.h"
#include "mymuduo/net/InetAddress.h"

// 参数以引用方式传递，如果不加const，则在函数里可能就会修改到传进来的参数
// 一般需要加const修饰
namespace mymuduo
{
class EventLoop;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void listen();

    bool listening() const {return listening_;}

    void setNewConnectionCallback(const NewConnectionCallback& cb){
        // cb = newConnectionCallback_; //const报错，无const不报错
        newConnectionCallback_ = cb;
    }

private:
    void handleRead();

    // 只在mainLoop中存在
    EventLoop* loop_; //mainLoop

    // 服务端创建的用于接收客户端连接的Socket
    Socket acceptSocket_;

    // 服务端创建的socket返回的文件描述符fd的channel对象
    Channel acceptChannel_;

    // 新的连接到达，做回调操作：唤醒subloop来处理新的连接
    // 参数：新连接客户的fd以及地址
    NewConnectionCallback newConnectionCallback_;

    bool listening_;
};

} // namespace mymuduo

#endif
