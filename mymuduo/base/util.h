/*
 * @FilePath: util.h
 * @Author: jt
 * @Date: 2023-06-16 16:05:43
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 20:12:32
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 工具类
 */
#ifndef MYMUDUO_BASE_UTIL_H
#define MYMUDUO_BASE_UTIL_H
#pragma once

#include <sys/types.h>
#include <unistd.h>


namespace mymuduo{


class util{
 public:
    /**
     * @brief 获取进程id号
     * 
     * @return pid_t 进程id号
     */
    static pid_t getPid();
    /**
     * @brief 获取线程id号
     * 
     * @return pid_t 线程id号
     */
    static pid_t getTid();

    
    
};


    
}


#endif