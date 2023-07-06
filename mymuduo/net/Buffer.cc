#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>

#include "mymuduo/net/Buffer.h"

using namespace mymuduo;

const size_t Buffer::kCheapPrependSize;
const size_t Buffer::kInitialSize;

// 从fd中读取数据到缓冲区中
size_t Buffer::readFd(int fd, int* savedErrno)
{
    // 使用分散读，先将超出写缓冲区的数据读到临时的栈空间中
    // 然后在将其写入到缓冲区中
    char extraBuf[65536] = {0};
    struct iovec vec[2];
    const size_t writable = writableBytes();
    // const char* !-> void*, char* -> void*
    vec[0].iov_base = begin() + writeIndex_; // beginWrite()
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    const int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    const size_t n = ::readv(fd, vec, iovcnt);
    if(n < 0){
        *savedErrno = errno;
    }else if(n <= writable){
        writeIndex_ += n;
    }else{
        // buffer中写满了
        writeIndex_ = buffer_.size();
        append(extraBuf, n - writable);
    }
    return n;
}

// 将缓冲区中的数据写入到fd中
size_t Buffer::writeFd(int fd, int* savedErrno)
{
    size_t n = ::write(fd, peek(), readableBytes());
    if(n < 0){
        *savedErrno = errno;
    }
    return n;
}