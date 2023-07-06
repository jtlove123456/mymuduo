#include <string.h>

#include "mymuduo/net/TcpServer.h"
#include "mymuduo/base/Logger.h"
#include "mymuduo/net/Acceptor.h"
#include "mymuduo/net/EventLoopThreadPool.h"


using namespace mymuduo;

static EventLoop* checkLoop(EventLoop* loop){
    if(loop == nullptr){
        LOGFATAL("main loop must is not null");
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* baseLoop,
              const InetAddress& listenAddr,
              const std::string& nameArg,
              Option option):
            loop_(checkLoop(baseLoop)),
            acceptor_(new Acceptor(loop_, listenAddr, option == kReusePort)), 
            threadPool_(new EventLoopThreadPool(loop_, nameArg)),
            ipPort_(listenAddr.toIpPort()),
            name_(nameArg),
            started_(0),
            nextConnId_(1),
            connectionCallback_(),
            messageCallback_(),
            writeCompleteCallback_()
{
    // 给Acceptor绑定新连接到达的回调
    // 新连接到达，TcpConnection->channel->readCallBack->(TcpConnection)handRead->accept->
    // acceptor_::newConnectionCallback_ -> TcpServer::newConnection
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, 
                                    std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    // 将管理的连接全部释放
    LOGINFO("TcpServer::~TcpServer [ %s ] destructing",name_.c_str());
    for(auto& item : connections_){
        // 临时记录item中的连接，出了循环就会自动释放
        TcpConnectionPtr conn(item.second);
        //释放item中的连接
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}
void TcpServer::start()
{
    // 开启TcpServer,只能创建一次
    if(started_++ == 0){
        // 开启线程池，创建线程池，以及对应的Eventloop
        threadPool_->start(threadInitCallback_);
        // 给mainloop绑定监听回调，并执行该回调
        // 当有客户端连接进来时，
        // 触发acceptorChannel的读事件->执行绑定的回调handleRead->accept客户端连接
        // ->将connfd打包成TcpConnection->执行TcpSever中acceptor_给TcpConnection绑定的newConnection回调
        // -> 轮询算法，选择一个subLoop来管理该连接
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}
// 轮询算法，选择一个subLoop管理新到达的连接
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    nextConnId_++;
    std::string connName = name_ + buf;
    LOGINFO("TcpServer::newConnection [ %s ] - new connection [ %s ] from %s",
            name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    // 获取本地地址
    sockaddr_in addr;
    socklen_t len = static_cast<socklen_t>(sizeof addr);
    memset(&addr, 0, len);
    if(::getsockname(sockfd, (sockaddr*)&addr, &len) < 0){
         LOGERROR("get LocalAddr failed");
    }
    InetAddress localAddr(addr);
    // 根据连接 创建TcpConnection对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));

    connections_[connName] = conn;
    // 这些回调由用户定义传递给TcpServer->TcpConnection来处理客户端连接后相应的业务逻辑
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    // 在subloop执行连接建立操作，并执行用户传入的ConnectionCallback
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}
// 
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    // 在loop所在线程中对相应的连接进行关闭
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    
    LOGINFO("TcpServer::removeConnectionInLoop [ %s ]-connection %s", name_.c_str(), conn->name().c_str());
    // 1、将连接从TcpServer管理的连接Map中删除
    connections_.erase(conn->name());
    // 2. 在conn对应的loop中删除该连接
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));;
}


