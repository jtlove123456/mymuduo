#include "mymuduo/net/Poller.h"
#include "mymuduo/net/Channel.h"

using namespace mymuduo;


Poller::Poller(EventLoop* loop_):
            ownerLoop_(loop_)
{
}
Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
    auto it = channels_.find(channel->getFd());
    return it != channels_.end() && it->second == channel;
}

