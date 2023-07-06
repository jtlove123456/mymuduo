#include <mymuduo/net/TcpServer.h>
#include <mymuduo/base/Logger.h>

#include <string>

using namespace mymuduo;

class EchoServer
{
public:
    EchoServer(EventLoop* loop,
            const InetAddress& addr,
            const std::string& name): loop_(loop), server_(loop, addr, name)
        {
            // 设置回调
            server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
            server_.setMessageCallback(std::bind(&EchoServer::onMessage, this,
            std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3));

            // 设置subLoop线程数量
            server_.setThreadNum(3);
        }
        void start(){
            server_.start();
        }
private:
    void onConnection(const TcpConnectionPtr& conn){
        if(conn->connected()){
            LOGINFO("Connection Up : from %s", conn->peerAddr().toIpPort().c_str());
        }else{
            LOGINFO("Connection Down : from %s", conn->peerAddr().toIpPort().c_str());
        }
    }
    void onMessage(const TcpConnectionPtr& conn,
                    Buffer* buffer,
                    Timestamp time){
        std::string msg = buffer->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown(); //关闭写端
    }

    EventLoop* loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress listenAddr(8880);
    EchoServer server(&loop, listenAddr, "echoServer-01");
    server.start();
    loop.loop(); //mainloop 启动底层poller
}