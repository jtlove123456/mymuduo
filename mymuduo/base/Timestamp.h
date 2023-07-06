/*
 * @FilePath: Timestamp.h
 * @Author: jt
 * @Date: 2023-06-16 16:36:43
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-25 21:17:04
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 时间戳,获取时间相关操作
 */
#ifndef MYMUDUO_BASE_TIMESTAMP_H
#define MYMUDUO_BASE_TIMESTAMP_H
#include<string>

namespace mymuduo
{
    class Timestamp{

    public:
        Timestamp();
        // 当构造函数只有一个参数时，使用explicit，防止隐式调用
        // 该构造函数必须显示调用
        explicit Timestamp(int64_t microSecondsSinceEpochArg);

        /**
         * @brief 获得当前时间
         * 
         * @return Timestamp 
         */
        static Timestamp now();
        /**
         * @brief 格式化输出时间
         * [%y-%m-%d %H:%M:%S]
         * @return std::string 
         */
        std::string tostring() const;

    private:
        int64_t microSecondsSinceEpoch_;

};
}
#endif