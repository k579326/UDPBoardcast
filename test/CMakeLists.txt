

cmake_minimum_required(VERSION 3.0)

include_directories(../AutoDiscover)

add_executable(testBc test.cpp)
add_executable(uvclient uvclient.cpp)
add_executable(uvserver uvserver.cpp)

target_link_libraries(uvserver libuv)
target_link_libraries(uvclient libuv)
target_link_libraries(testBc net_discover)
