/*
 * @FilePath: Logger.h
 * @Author: jt
 * @Date: 2023-06-16 09:45:43
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:27:12
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 日志输出类
 */

#ifndef MYMUDUO_BASE_LOGGER_H
#define MYMUDUO_BASE_LOGGER_H
#pragma once

#include <string>

#include "mymuduo/base/noncopyable.h"

namespace mymuduo{

/**
 * @brief 
 *  日志级别枚举类
 */
enum LogLevel{
    INFO,  // 正常信息
    DEBUG, //调试信息
    WARN,  //警告信息
    ERROR, //错误信息
    FATAL, //core错误
};

/**
 * @brief 日志器类，用于日志的打印 
 * 由于只需要一个日志器类，因此使用单例模式，采用懒汉式方式实现
 * 1.构造器私有化，提供静态方法获取初始化的日志器
 */
class Logger : noncopyable{

public:
    /**
     * @brief 获取日志器实例对象
     * 
     * @return Logger& 日志器实例对象的引用
     */
    static Logger& getInstance();
    /**
     * @brief 设置日志级别
     * 
     * @param level 需要设置的日志级别
     */
    void setLogLevel(LogLevel level);
    /**
     * @brief 打印日志
     * 输出格式为：[level][time][pid:tid]msg
     * @param msg 用户需要打印的日志信息
     */
    void log(const std::string &msg);

private:
    //日志级别
    LogLevel level_; 
};

/**
 * @brief 定义格式化打印日志的宏
 * 使用可变参数来格式化 // sizeof buf sizeof(buf) 
 */
#define LOGINFO(msgFormat, ...)                          \
    do                                                   \
    {                                                    \
        mymuduo::Logger& logger = mymuduo::Logger::getInstance(); \
        logger.setLogLevel(mymuduo::LogLevel::INFO);        \
        char buf[1024] = {0};                              \
        snprintf(buf, 1024, msgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                 \
    }while(0)

#define LOGERROR(msgFormat, ...)                          \
    do                                                   \
    {                                                    \
        mymuduo::Logger& logger = mymuduo::Logger::getInstance(); \
        logger.setLogLevel(mymuduo::LogLevel::ERROR);        \
        char buf[1024] = {0};                              \
        snprintf(buf, 1024, msgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                 \
    }while(0)

#define LOGWARN(msgFormat, ...)                          \
    do                                                   \
    {                                                    \
        mymuduo::Logger& logger = mymuduo::Logger::getInstance(); \
        logger.setLogLevel(mymuduo::LogLevel::WARN);        \
        char buf[1024] = {0};                              \
        snprintf(buf, 1024, msgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                 \
    }while(0)

#define LOGFATAL(msgFormat, ...)                          \
    do                                                   \
    {                                                    \
        mymuduo::Logger& logger = mymuduo::Logger::getInstance(); \
        logger.setLogLevel(mymuduo::LogLevel::FATAL);        \
        char buf[1024] = {0};                              \
        snprintf(buf, 1024, msgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                 \
        exit(-1);                                       \
    }while(0)

#ifdef MUDEBUG
    #define LOGDEBUG(msgFormat, ...)                          \
        do                                                   \
        {                                                    \
            mymuduo::Logger& logger = mymuduo::Logger::getInstance(); \
            logger.setLogLevel(mymuduo::LogLevel::DEBUG);        \
            char buf[1024] = {0};                              \
            snprintf(buf, 1024, msgFormat, ##__VA_ARGS__);   \
            logger.log(buf);                                 \
        }while(0)
#else
    #define LOGDEBUG(msgFormat, ...)
#endif

}
#endif