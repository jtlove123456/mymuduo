/*
 * @FilePath: TcpServer.h
 * @Author: jt
 * @Date: 2023-06-28 09:04:10
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:36:28
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 提供给用户进行服务端开发使用的接口
 */
#ifndef MYMUDUO_NET_TCPSERVER_H
#define MYMUDUO_NET_TCPSERVER_H
#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <unordered_map>
#include <functional>

#include "mymuduo/base/noncopyable.h"
#include "mymuduo/net/Callbacks.h"
#include "mymuduo/net/InetAddress.h"
#include "mymuduo/net/EventLoop.h"
#include "mymuduo/net/TcpConnection.h"

namespace mymuduo
{

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option{kNoReusePort, kReusePort};

    TcpServer(EventLoop* baseLoop,
              const InetAddress& listenAddr,
              const std::string& nameArg,
              Option option = kNoReusePort);

    ~TcpServer();
    void setThreadNum(int numThreads);
    void start();
    void setThreadInitCallback(const ThreadInitCallback& cb){threadInitCallback_ = cb;}

    const std::string& ipPort() const {return ipPort_;}
    const std::string& name()const {return name_;}
    EventLoop* getLoop() const {return loop_;}
    std::shared_ptr<EventLoopThreadPool> threadPool() const {return threadPool_;}

    // 这些回调都是提供给用户进行设置给
    // TcpServer->TcpConnection->Channel notify poller通知channel相应的事件
    // channel -> 相应的回调
    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback& cb){messageCallback_ = cb;}
    void setCloseCallback(const CloseCallback& cb){closeCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_ = cb;}


private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_; // mainLoop, acceptor loop

    std::unique_ptr<Acceptor> acceptor_; 

    std::shared_ptr<EventLoopThreadPool> threadPool_;

    const std::string ipPort_;
    const std::string name_;

    // 原子操作，保证只有一个TcpServer创建
    std::atomic_int started_;

    // 用于设置每个新连接的名字
    int nextConnId_;
    // 根据名字拿到对应的连接
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;

    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;

};

} // namespace mymuduo


#endif