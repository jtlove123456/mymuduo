/*
 * @FilePath: Buffer.h
 * @Author: just
 * @Date: 2023-06-29 15:33:53
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-03 22:29:36
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 
 */
/*
 * @FilePath: Buffer.h
 * @Author: jt
 * @Date: 2023-06-28 15:45:38
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-06-29 15:33:52
 * Copyright: 2023 xxxTech CO.,LTD. All Rights Reserved.
 * @Descripttion: 数据缓冲区
 */
#ifndef MYMUDOUO_NET_BUFFER_H
#define MYMUDOUO_NET_BUFFER_H
#pragma once

#include <vector>
#include <assert.h>
#include <string>

#include "mymuduo/base/copyable.h"

namespace mymuduo
{
/**
 * @brief 缓冲区设计，主要由三个部分组成：
 * 预留区、可读区，以及可写区
 * @code
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 * @endcode
 */
class Buffer : public copyable
{
public:
    // prependable bytes  8bytes
    static const size_t kCheapPrependSize = 8;
    // readable bytes + writable bytes 1024bytes
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize):
                    buffer_(kCheapPrependSize + initialSize),
                    readerIndex_(kCheapPrependSize),
                    writeIndex_(kCheapPrependSize){}

    // 获取可读数据区域的大小
    size_t readableBytes() const {return writeIndex_ - readerIndex_;}
    // 获取可写数据区域的大小
    size_t writableBytes() const {return buffer_.size() - writeIndex_;}
    // 获取预置区的大小
    size_t prependableBytes() const {return readerIndex_;}

    // 获取缓冲区中可读数据的开始位置
    const char* peek() const {return begin() + readerIndex_;}

    // 更新读取了缓冲区可读数据后readerIndex_的位置
    // len:读取了的数据的长度
    void retrieve(size_t len){
        // 读取的数据长度不能超过可读数据的长度
        assert(len <= readableBytes());
        if(len < readableBytes()){
            readerIndex_ += len;
        }else{  // 数据读完了，缓冲区无数据了，重置为初始状态
           retrieveAll(); 
        }
    }
    void retrieveAll(){
        // 无可读数据
        readerIndex_ = kCheapPrependSize;
        // 全为可写数据区
        writeIndex_ = kCheapPrependSize;
    }
    
    // 把onMessage 上报的buffer数据转化为string类型数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len); //获取到数据
        retrieve(len); //重置可读缓冲区
        return result;
    }

    // 缓冲区可写区域的起始地址
    const char* beginWrite() const { return begin() + writeIndex_;}
    char* beginWrite(){return begin() + writeIndex_;}
    //缓冲区可写完长度为len数据后，更新可写区域的起始地址
    // 表示已经写了多少数据
    void hasWritten(size_t len){
        // 保证写入的数据长度不超过可写区域的大小
        assert(len <= writableBytes());
        writeIndex_ += len;
    } 
     
    // 把[data, data+len]内存上的数据 往缓冲区中写
    void append(const char* data, size_t len){
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }
    
    // 保证可写区域的大小能够装下长度为len的数据
    void ensureWritableBytes(size_t len){
        if(writableBytes() < len){
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    // 从fd中读取数据到缓冲区中
    size_t readFd(int fd, int* savedErrno);

    // 将缓冲区中的数据写入到fd中
    size_t writeFd(int fd, int* savedErrno);

private:

    char* begin(){
       return &*buffer_.begin(); 
    }
    // 获取缓冲区起始位置
    const char* begin() const{
        return &*buffer_.begin();
    }
    // 扩容/碎片化整理
    void makeSpace(size_t len){
        if(writableBytes() + prependableBytes() < len + kCheapPrependSize){
            // 缓冲区空闲的大小不能放下长度为len的数据，扩容
            buffer_.resize(writeIndex_ + len);
        }else{ //缓冲区空闲的大小能放下，将可读区域数据往前移动，流出连续的可写空间
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writeIndex_,
                      begin() + kCheapPrependSize);
            readerIndex_ = kCheapPrependSize;
            writeIndex_ = readerIndex_ + readable;
        }
    }

    // 缓冲区使用动态数组来维护，可以进行扩容
    std::vector<char> buffer_;
    //可读缓冲区的开始位置, 可以将数据写出去
    size_t readerIndex_;

    // 可写缓冲区的开始位置， 用于保存读取到的数据
    size_t writeIndex_;

};

} // namespace mymuduo



#endif