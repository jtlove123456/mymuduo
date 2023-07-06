/*
 * @FilePath: InetAddress.h
 * @Author: just
 * @Date: 2023-06-27 09:19:31
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-04 09:06:02
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#ifndef MYMUDUO_NET_INETADDRESS_H
#define MYMUDUO_NET_INETADDRESS_H
#pragma once

#include <netinet/in.h>
#include <string>

#include "mymuduo/base/copyable.h"

namespace mymuduo
{

/**
 * @brief 封装通信所用的地址(ip, port)
 * 目前只支持ipv4
 */
class InetAddress : copyable
{
 public:
    explicit InetAddress(uint16_t port = 0);
    InetAddress(std::string ip, uint16_t port);
    explicit InetAddress(const struct sockaddr_in& addr): addr_(addr){}

    sa_family_t family() {return addr_.sin_family;}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;

    const struct sockaddr* getSockAddr() const {
       return static_cast<const struct sockaddr*>(static_cast<const void*>(&addr_));
    } 
    const struct sockaddr_in getSockAddrIn() const {
       return addr_;
    }

    void setSockAddr(const struct sockaddr_in& addr){ addr_ = addr;}

 private:
    struct sockaddr_in addr_;

};
} // namespace mymuduo



#endif