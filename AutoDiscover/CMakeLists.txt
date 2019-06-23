

cmake_minimum_required(VERSION 3.0)

FILE(GLOB BOARDCASTSRC boardcast/*.cpp)
FILE(GLOB BOARDCASTHEADER boardcast/*.h)

add_library(net_discover STATIC discover.cpp
			${BOARDCASTSRC} ${BOARDCASTHEADER} 
			boardcast/boardcast_address/select_network.cpp boardcast/boardcast_address/select_network.h)


target_link_libraries(net_discover libuv cb_thread cb_sysinfo bridge)

