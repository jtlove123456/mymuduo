/*
 * @FilePath: Timestamp.cc
 * @Author: just
 * @Date: 2023-06-18 09:21:12
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 21:18:04
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#include <time.h>

#include "mymuduo/base/Timestamp.h"

using namespace mymuduo;

Timestamp::Timestamp() : microSecondsSinceEpoch_(0){
}

Timestamp::Timestamp(int64_t microSecondsSinceEpochArg):
            microSecondsSinceEpoch_(microSecondsSinceEpochArg)
{
}

Timestamp Timestamp::now(){
    return Timestamp(time(NULL));
}
std::string Timestamp::tostring() const{
    char buf[128];
    struct tm* now_time;
    now_time = localtime(&microSecondsSinceEpoch_);
    strftime(buf, 128, "%y-%m-%d %H:%M:%S", now_time);
    return buf;
}


// #include <iostream>
// int main(){
//     std::cout << mymuduo::Timestamp::now().tostring() << std::endl;
// }