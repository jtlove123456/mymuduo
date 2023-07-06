/*
 * @FilePath: Acceptor.cc
 * @Author: just
 * @Date: 2023-06-28 10:28:47
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-04 08:42:35
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#include <sys/socket.h>
#include <errno.h>

#include "mymuduo/net/Acceptor.h"
#include "mymuduo/base/Logger.h"

using namespace mymuduo;

static int creatNonBlocking()
{
    // SOCK_ 表示socket相关的类型
   int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
   if(sockfd < 0){
        LOGFATAL("%s:%s:%d listen socket create err : %d \n", __FILE__, __FUNCTION__, __LINE__, errno);
   }
   return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport):
         loop_(loop),
         acceptSocket_(creatNonBlocking()),
         acceptChannel_(loop_, acceptSocket_.fd()),
         listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    // mainLoop => Acceptor
    // 给监听的fd(listenfd,服务端创建的socket对应的fd)预置一个读回调函数
    // 当有客户端发起连接时，触发读事件，
    // 执行读回调函数->接收客户端连接->处理新连接（subLoop中）
    acceptChannel_.setReadCallBack(
         std::bind(&Acceptor::handleRead, this));
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();

}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    // 添加listenfd到poller上,并注册对读事件感兴趣
    acceptChannel_.enableReading();
}
// 接收客户端连接，并将连接交给subloop做相应的任务
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(peerAddr);
    if(connfd >= 0){ //有新连接到达
        if(newConnectionCallback_){ //通过TcpServer事先设置了新连接到达的处理回调
            newConnectionCallback_(connfd, peerAddr);
        }else{ //没有设置对新连接的处理
            ::close(connfd);
        }
    }else{ //连接失败
        LOGERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        // 打开的文件描述符达到上限
        if(errno == EMFILE){
            LOGERROR("%s:%s:%d sockfd reached limit! \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}