
cmake_minimum_required(VERSION 3.0)

include_directories(./)
include_directories(include)
include_directories(third/cb_sysinfo/inc)
include_directories(third/cb_thread/inc)
include_directories(third/libuv/include)
include_directories(common)

FILE(GLOB SELECTADDRSRC common/*)


add_subdirectory(third/cb_sysinfo)
add_subdirectory(third/libuv)
add_subdirectory(bridge)
add_subdirectory(AutoDiscover)
add_subdirectory(third/cb_thread)
add_subdirectory(Communication)
add_subdirectory(test)
#add_subdirectory(Communication)