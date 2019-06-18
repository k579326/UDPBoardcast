

cmake_minimum_required(VERSION 3.0)

include_directories(../AutoDiscover)

add_executable(testBc test.cpp)

target_link_libraries(testBc net_discover)
