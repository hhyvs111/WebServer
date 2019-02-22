//
// Created by 谭文波 on 2019/2/22.
//

#ifndef WEBSERVER_NONCOPYABLE_H
#define WEBSERVER_NONCOPYABLE_H
#pragma once
//不可复制的类，就是直接把拷贝函数声明为私有的不就等于禁用
class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};
#endif //WEBSERVER_NONCOPYABLE_H
