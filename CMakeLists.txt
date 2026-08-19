cmake_minimum_required(VERSION 3.20)

project(concurrency)

set(CMAKE_CXX_STANDARD 17)

add_executable(race_condition race_condition.cpp)
add_executable(thread_safe_queue thread_safe_queue.cpp)
add_executable(thread_safe_queue_v2 thread_safe_queue_v2.cpp)
