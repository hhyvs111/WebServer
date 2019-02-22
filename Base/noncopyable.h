//
// Created by 谭文波 on 2019/2/22.
//

#ifndef WEBSERVER_NONCOPYABLE_H
#define WEBSERVER_NONCOPYABLE_H
#pragma once
//不可复制的类，就是直接把拷贝函数声明为私有的就等于禁用,或者声明为delete
class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}

public:
    noncopyable(const noncopyable&) = delete;
    const noncopyable& operator=(const noncopyable&) = delete;
};
#endif //WEBSERVER_NONCOPYABLE_H
