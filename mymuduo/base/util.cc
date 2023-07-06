/*
 * @FilePath: util.cc
 * @Author: just
 * @Date: 2023-06-18 09:21:31
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 21:17:46
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#include <sys/syscall.h>

#include "mymuduo/base/util.h"
using namespace mymuduo;

static int g_pid = 0;
// thread_local/__thread表示是只会在每个线程最开始被调用的时候进行初始化，
// 并且只会被初始化一次
// static __thread int g_tid = 0;
static thread_local int g_tid = 0;

pid_t util::getPid(){
    if(g_pid != 0){
        return g_pid;
    }
    return ::getpid();
}
pid_t util::getTid(){
    if(g_tid != 0){
        return g_tid;
    }
    return ::syscall(SYS_gettid);
}