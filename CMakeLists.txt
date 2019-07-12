
cmake_minimum_required(VERSION 3.0)

include (cmake/platform_verify.cmake)	# 平台检测及设置
include (cmake/compiler_setting.cmake)  # 各编译器的编译选项设置

include_directories(./)
include_directories(include/internal)
include_directories(third/cb_sysinfo/inc)
include_directories(third/libuv/include)
include_directories(third/queue)
include_directories(third/threadpool)
include_directories(common)

FILE(GLOB SELECTADDRSRC common/*)

add_subdirectory(third/queue)
add_subdirectory(third/threadpool)
add_subdirectory(third/cb_sysinfo)
add_subdirectory(third/libuv)
add_subdirectory(bridge)
add_subdirectory(AutoDiscover)
add_subdirectory(Communication)
add_subdirectory(test)
#add_subdirectory(Communication)