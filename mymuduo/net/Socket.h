/*
 * @FilePath: Socket.h
 * @Author: jt
 * @Date: 2023-06-27 10:50:46
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-27 15:44:15
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 封装了服务端用于监听和接受客户端连接的socket
 * 1、创建套接字socket
 * 2、绑定 bind 地址
 * 3、监听 listen
 * 4、接收连接 accept
 */

#ifndef MYMUDUO_NET_SOCKET_H
#define MYMUDUO_NET_SOCKET_H
#pragma once

#include "mymuduo/base/noncopyable.h"

namespace mymuduo
{

class InetAddress;


class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd){}
    
    ~Socket();

    // 绑定本地地址
    void bindAddress(const InetAddress& localAddr);
    void listen();
    // 接收对端地址,返回连接的fd
    int accept(InetAddress& peerAddr);
    
    void shutdownWrite();

    //  Enable/disable TCP_NODELAY
    void setTcpNoDelay(bool on);
    
    // Enable/disable SO_REUSEADDR
    void setReuseAddr(bool on);

    // Enable/disable SO_REUSEPORT
    void setReusePort(bool on);

    // Enable/disable SO_KEEPALIVE
    void setKeepAlive(bool on);

    int fd() const { return sockfd_;}

private:
    const int sockfd_;
};

} // namespace mymuduo

#endif

