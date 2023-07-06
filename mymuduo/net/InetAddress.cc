/*
 * @FilePath: InetAddress.cc
 * @Author: just
 * @Date: 2023-06-27 10:38:49
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:33:36
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#include <string>
#include <string.h>
#include <arpa/inet.h>

#include "mymuduo/net/InetAddress.h"
#include "mymuduo/base/Logger.h"

using namespace mymuduo;

static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    ::memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = kInaddrAny;
    addr_.sin_addr.s_addr = ::htonl(ip);
    addr_.sin_port = ::htons(port);

}

InetAddress::InetAddress(std::string ip, uint16_t port)
{
    ::memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    // htobe16(port) linux endian.h中提供的主机字节序和小/大端字节序的转化
    // htobe* 大端 htole* 小端
    //htons htonl, c标准库 netinet/in.h 中提供的主机字节序与网络字节序(大端字节序)的转化
    addr_.sin_port = ::htons(port);
    // 将string ip地址转化到uint32上来, x.x.x.x -> 数字
    // 方法1：
    // addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    // 方法2：
    // 将点分文本的IP地址转换为二进制网络字节序”的IP地址
    if(::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0)
    {
        LOGERROR("InetAddress(ip, port)");
    }
}

std::string InetAddress::toIp() const
{
    char buf[64] = "";
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof buf));
    return buf;
}
std::string InetAddress::toIpPort() const
{
    char buf[64] = "";
    buf[0] = '[';
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf + 1, static_cast<socklen_t>(sizeof buf));
    size_t end = strlen(buf);
    uint16_t port = ::ntohs(addr_.sin_port);
    // sprintf snprintf:带长度的
    snprintf(buf + end, sizeof buf - end, ":%u", port);
    buf[strlen(buf)] = ']';
    return buf;
}
uint16_t InetAddress::port() const
{
    return ::ntohs(addr_.sin_port);
}