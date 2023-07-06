#include <stdlib.h>

#include "mymuduo/net/Poller.h"
#include "mymuduo/net/EpollPoller.h"
using namespace mymuduo;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  if(::getenv("MYMUDUO_USE_POLL")){
    return nullptr;  //生成pollPoller实例 todo
  }else{
    return new EpollPoller(loop);
  }
}

