/*
 * @FilePath: TcpConnection.h
 * @Author: just
 * @Date: 2023-06-28 15:44:47
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:35:47
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
/*
 * @FilePath: TcpConnection.h
 * @Author: jt
 * @Date: 2023-06-28 09:28:46
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-28 15:43:52
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 封装连接进来的客户端fd,处理读写事件
 */
#ifndef MYMUDUO_NET_TCPCONNECTION_H
#define MYMUDUO_NET_TCPCONNECTION_H
#pragma once 

#include <memory>
#include <atomic>

#include "mymuduo/base/noncopyable.h"
#include "mymuduo/net/InetAddress.h"
#include "mymuduo/net/Buffer.h"
#include "mymuduo/net/Callbacks.h"

namespace mymuduo
{
class EventLoop;
class Socket;
class Channel;
// public std::enable_shared_from_this<TcpConnection>
/**
 * @brief https://blog.csdn.net/gc348342215/article/details/123215888
 * 当类A被share_ptr<A>管理，
 * 且在类A的成员函数里需要把当前类对象this作为参数传给其他函数时，
 * 就需要传递一个指向自身的share_ptr。
 * TcpConnection将其this指针传给了Channel对象，
 * 所以说如果此时Channel感兴趣的事件依然被Poller监听着，
 * 这时候突然有事件发生，那么TcpConnection对象若已经销毁了呢？
 * 那么Channel对象在调用往该Channel对象中注册了的TcpConnection的成员函数时就会发生错误。
 * 
 * 因此这样可以延长TcpConnection的生命周期
 */
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{

public:
    TcpConnection(EventLoop* loop, 
                  const std::string& nameArg,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    void send(const std::string& message);
    void shutdown();

    void startRead();
    void stopRead();

    // called when TcpServer accepts a new connection
    void connectEstablished();
    // called when TcpServer has removed me from its ConnectMap
    void connectDestroyed();

    EventLoop* getLoop() const {return loop_;}
    const std::string& name() const {return name_;}
    const InetAddress& localAddr() const {return localAddr_;}
    const InetAddress& peerAddr() const {return peerAddr_;}
    
    bool connected() const {return state_ == kConnected;}
    bool disConnected() const {return state_ == kDisconnected;}
    bool isReading() const {return reading_;}

    // 设置回调接口
    // 新连接回调
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }
    // 读写事件回调
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }
    // 写事件完成回调
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }
    // 高水位回调，当连接的客户端数量到达设置的水位，进行回调处理
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    //连接关闭回调
    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; } 


private:
    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    void startReadInLoop();
    void stopReadInLoop();
    
    // 给连接进来的connfd(channel_)预先绑定的事件回调函数
    // 当channel上有相应的事件发生，就会执行绑定的这些回调操作
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handlClose();
    void handleError();
    
private:
    enum StateE{kDisconnected, kConnecting, kConnected, kDisconnecting};
    void setState(StateE state){ state_ = state;}
    
    // 连接的状态
    std::atomic_int state_;
    // 连接名
    const std::string name_;

    bool reading_;

    //读写数据量的控制
    size_t highWaterMark_;

    // 该连接在那个loop中处理，即轮询选到的subLoop
    EventLoop* loop_;

    // 连接进来的connfd->connSocket,用于设置fd的属性
    std::unique_ptr<Socket> socket_;
    // 连接进来的connfd,用于处理读写事件
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;
 
    // 读写缓冲区
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    // 供Tcp设置的回调函数
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

};

} // namespace muduo

#endif