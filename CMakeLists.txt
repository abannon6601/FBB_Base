cmake_minimum_required(VERSION 2.8)
project(FBB_base)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-std=c++0x")

add_executable(FBB_base main.cpp hyperToFlow.cpp fileRead.cpp Hmain.h FBB_base_functions.cpp)