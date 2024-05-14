cmake_minimum_required(VERSION 3.26)
project(hsec)

set(CMAKE_CXX_STANDARD 17)

add_executable(hsec main.cpp
        CPA.cpp
        CPA.h
        traceHandler.cpp
        traceHandler.h)
