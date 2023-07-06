#include "mymuduo/net/TcpConnection.h"
#include "mymuduo/base/Logger.h"
#include "mymuduo/net/Socket.h"
#include "mymuduo/net/Channel.h"

using namespace mymuduo;

TcpConnection::TcpConnection(EventLoop* loop, 
                  const std::string& nameArg,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr):
                state_(kConnecting),
                name_(nameArg),
                reading_(true),
                highWaterMark_(64 * 1024 * 1024),
                loop_(loop),
                socket_(new Socket(sockfd)),
                channel_(new Channel(loop_, sockfd)),
                localAddr_(localAddr),
                peerAddr_(peerAddr)
{
    // 给channel(connfd)设置事件处理回调
    channel_->setReadCallBack(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallBack(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallBack(
        std::bind(&TcpConnection::handlClose, this));
    
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));

    LOGDEBUG("TcpConnection::ctor[ %s ] at %p fd = %d", name_, this, sockfd);
    socket_->setKeepAlive(true);

}

TcpConnection::~TcpConnection()
{
    LOGDEBUG("TcpConnection::ctor[ %s ] at %p fd = %d", name_, this, channel_->getFd()); 
}

// 将
void TcpConnection::send(const std::string& message)
{
    if(state_ = kConnected){

        if(loop_->isInLoopThread()){
            sendInLoop(message);
        }else{
            // 不在当前loop线程中，将其放到loop的回调函数队列中，等待相应的loop去处理
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, message));
        }
    }
}

void TcpConnection::shutdown()
{
    if(state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}
void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

// called when TcpServer accepts a new connection
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    // 延长TcpConnection的生命周期
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}
// called when TcpServer has removed me from its ConnectMap
void TcpConnection::connectDestroyed()
{
    if(state_ = kConnected){
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    // 连接中，或未连接的channel给移除掉(释放掉)
    channel_->remove();
}

// 发送数据 应用写的快，而内核发送数据慢，需要把待发送数据写入缓冲区 而且设置了水位回调
// 给客服端发送数据
void TcpConnection::sendInLoop(const std::string& message)
{
    size_t nwrote = 0;
    size_t remaining = message.size();
    bool faultError = false;

    if(state_ == kDisconnected){ //连接已经关闭，停止写数据
        LOGERROR("disconnected, give up writing");
        return;
    }
    // 第一次写数据，且输出缓冲区的没有可读数据需要被写出去
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        // 直接将数据写入到内核中
        nwrote = ::write(channel_->getFd(), message.c_str(), message.size());
        if(nwrote >= 0){ //数据写成功
            remaining = message.size() - nwrote;
            if(remaining == 0 && writeCompleteCallback_){ //表明数据已经准备好了
                // 唤醒相应的loop执行写完成回调，
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }else{//数据写失败
            nwrote = 0;
            // EWOULDBLOCK：由于非阻塞而导致没有数据
            if(errno != EWOULDBLOCK){ 
                LOGERROR("TcpConnection::sendInLoop");
                // ECONNRESET:Connection reset by peer
                // EPIPE: Broken pipe
                if(errno == EPIPE || errno == ECONNRESET){
                    //客服端出问题
                    faultError = true; 
                }
            }
        }
    }
    // 这一次write数据没写完，客服端没有出现问题
    // 剩余的数据需要保存到缓冲区当中，然后给channel注册epollout事件，
    // poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
    // 也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成
    if(!faultError && remaining > 0){
        // 之前未发送出去的数据
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_
            && oldlen < highWaterMark_
            && highWaterMarkCallback_){
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining));
        }
        // 将数据放入缓冲区中
        outputBuffer_.append(message.c_str() + nwrote, remaining);
        if(!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting()){
        socket_->shutdownWrite();
    }
}

void TcpConnection::startReadInLoop()
{
    if(!reading_ || !channel_->isReading()){
        channel_->enableReading();
        reading_ = true;
    }
}
void TcpConnection::stopReadInLoop()
{
    if(reading_ || channel_->isReading()){
        channel_->disableReading();
        reading_ = false;
    }
}

// 给连接进来的connfd(channel_)预先绑定的事件回调函数
// 当channel上有相应的事件发生，就会执行绑定的这些回调操作

// 将fd内核区域上的数据读入到输入缓冲区的可写区域中
void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    size_t n = inputBuffer_.readFd(channel_->getFd(), &savedErrno);   
    if(n > 0){ //读取到数据，交由用户注册的onMessage回调处理数据
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }else if(n == 0){
        handlClose();
    }else{
         errno = savedErrno;
         LOGERROR("TcpConnection::handleRead");
         handleError();
    }
}
// 将输出缓冲区的可读数据写入到fd的内核区域
void TcpConnection::handleWrite()
{   
    int savedErrno = 0;
    if(channel_->isWriting()){

        size_t n = outputBuffer_.writeFd(channel_->getFd(), &savedErrno);
        if(n > 0){
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0){
                //数据写完了
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }else{
            LOGERROR("TcpConnection::handleWrite");
        }
    }else{ 
        LOGDEBUG("Connection fd = %d is down, no more writing", channel_->getFd());
    }
}
void TcpConnection::handlClose()
{
    LOGINFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->getFd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());

    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}
void TcpConnection::handleError()
{
    int optval;
    int err = 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if(::getsockopt(channel_->getFd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){
        err = errno;
    }else{
        err = optval;
    }
    LOGERROR("TcpConnection::handleError [%s] - SO_ERROR = %d", (char*)name_.c_str(), err);
}