cmake_minimum_required(VERSION 3.12)
project(WebServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(WebServer main.cpp Base/noncopyable.h)