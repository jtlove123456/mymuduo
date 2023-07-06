/*
 * @FilePath: noncopyable.h
 * @Author: jt
 * @Date: 2023-06-16 15:42:55
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-16 16:25:06
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 用于设置派生类不能使用拷贝构造和重载赋值运算符
 */
#ifndef MYMODUO_BASE_NONCOPYABLE_H
#define MYMODUO_BASE_NONCOPYABLE_H
#pragma once

namespace mymuduo{

class noncopyable{

    public:
        //禁用拷贝构造
        noncopyable(const noncopyable&) = delete;
        // 禁用赋值运算符
        void operator=(const noncopyable&) = delete;
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
};


}


#endif