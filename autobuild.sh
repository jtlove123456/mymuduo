#!/bin/bash
# 执行错误信息打印
set -e
# `pwd` 不是'' [ ! -d ./build ] 每一个之间都要有空格
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

# 回到根目录
cd ..
cd `pwd`/mymuduo/base

# 将头文件拷贝到 usr/include/mymuduo/base so库拷贝到usr/lib下， 默认在系统路径PATH

if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

if [ ! -d /usr/include/mymuduo/base ]; then
    mkdir /usr/include/mymuduo/base
fi

if [ ! -d /usr/include/mymuduo/net ]; then
    mkdir /usr/include/mymuduo/net
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo/base
done 

cd ../net

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo/net
done

cd .. 
cd ..

cp `pwd`/lib/libmymuduo.so /usr/lib

# 刷新缓存
ldconfig

