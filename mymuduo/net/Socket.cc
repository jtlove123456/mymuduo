#include <unistd.h>
#include <sys/socket.h> //IPPO, SO_, SOL_
#include <netinet/tcp.h> // TCP_
#include <netinet/in.h>
#include <string.h>

#include "mymuduo/net/Socket.h"
#include "mymuduo/base/Logger.h"
#include "mymuduo/net/InetAddress.h"

using namespace mymuduo;


Socket::~Socket()
{
    // 关闭fd
    if(::close(sockfd_) < 0)
    {
        LOGERROR("Socket close sockfd %d fail \n", sockfd_);
    }
}

// 绑定本地地址 封装了bind()
void Socket::bindAddress(const InetAddress& localAddr)
{
    // const struct sockaddr* sockaddr_ = localAddr.getSockAddr();
    struct sockaddr_in sockaddr_ = localAddr.getSockAddrIn();
    socklen_t len = sizeof sockaddr_;
    int ret = ::bind(sockfd_, (struct sockaddr*)&sockaddr_ , len);
    if(ret < 0){
        LOGFATAL("Socket::bindAddress sockfd %d fail \n", sockfd_);
    }
}
// 封装了listen()
void Socket::listen()
{
    // 设置最大能够连接的客户端数量
    int ret = ::listen(sockfd_, SOMAXCONN);
    if(ret < 0){
         LOGFATAL("Socket::listen sockfd %d fail \n", sockfd_);
    }
}
// 接收对端地址, 封装了accept()
int Socket::accept(InetAddress& peerAddr)
{
    /**
     * 1. accept函数的参数不合法
     * 2. 对返回的connfd没有设置非阻塞
     * Reactor模型 one loop per thread
     * poller + non-blocking IO
     * 使用accept4，来进行non-blocking IO的设置
     */ 
    struct sockaddr_in sockaddr_;
    socklen_t len = sizeof sockaddr_;
    memset(&sockaddr_, 0, len);
    int connfd = ::accept4(sockfd_, (sockaddr*)(&sockaddr_), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0){
        peerAddr.setSockAddr(sockaddr_);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOGERROR("Socket shutdownWrite sockfd %d fail \n", sockfd_);
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                            &optval, static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0 && on){
       LOGERROR("Socket setTcpNoDelay sockfd %d fail \n", sockfd_); 
    }
}

// Enable/disable SO_REUSEADDR
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                           &optval, static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0 && on){
       LOGERROR("Socket setReuseAddr sockfd %d fail \n", sockfd_); 
    }
}

// Enable/disable SO_REUSEPORT
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, 
                            &optval, static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0 && on){
        LOGERROR("Socket setReusePort sockfd %d fail \n", sockfd_); 
    }
}

// Enable/disable SO_KEEPALIVE
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, 
                            &optval, static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0 && on){
        LOGERROR("Socket setKeepAlive sockfd %d fail \n", sockfd_); 
    } 
}