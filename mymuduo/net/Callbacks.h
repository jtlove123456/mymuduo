#ifndef MYMUDUO_NET_CALLBACKS_H
#define MYMUDUO_NET_CALLBACKS_H
#pragma once
#include <memory>
#include <functional>

#include "mymuduo/base/Timestamp.h"

namespace mymuduo
{

class TcpConnection;
class Buffer;

// 定义一些回调操作
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

using MessageCallback = std::function<void(
            const TcpConnectionPtr&,
            Buffer* buf,
            Timestamp)>;

} // namespace mymuduo





#endif