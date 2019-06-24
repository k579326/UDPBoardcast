

cmake_minimum_required(VERSION 3.0)

include_directories(../AutoDiscover)

add_executable(testBc test.cpp)
add_executable(testuv uvtest.cpp)

target_link_libraries(testuv libuv)
target_link_libraries(testBc net_discover)
