/*
 * @FilePath: Logger.cc
 * @Author: just
 * @Date: 2023-06-18 09:19:58
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:27:37
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
#include <iostream>
#include <string>

#include "mymuduo/base/Logger.h"
#include "mymuduo/base/util.h"
#include "mymuduo/base/Timestamp.h"

using namespace mymuduo;

Logger& Logger::getInstance(){
    static Logger logger;
    return logger; 
}

void Logger::setLogLevel(LogLevel level){
    level_ = level;
}


void Logger::log(const std::string &msg){
    switch (level_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    case WARN:
        std::cout << "[WARN]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    default:
        break;
    }
    pid_t pid = util::getPid();
    pid_t tid = util::getTid();

    std::cout << "[" << Timestamp::now().tostring() << "]"
              << "[" << pid << ":" << tid << "]"
              << msg;
}

// int main()
// {
//     std::string str = "info test";
//     int a = 10;
//     LOGINFO("test %s %d \n", str.c_str(), a);
//     return 0;
// }