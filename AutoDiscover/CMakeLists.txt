

cmake_minimum_required(VERSION 3.0)

FILE(GLOB BOARDCASTSRC boardcast/*.cpp)
FILE(GLOB BOARDCASTHEADER boardcast/*.h)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set (SPECSRC boardcast/win/select_network.cpp boardcast/win/select_network.h)
else ()
    set (SPECSRC )
endif ()


add_library(net_discover STATIC discover.cpp discover.h
			${BOARDCASTSRC} ${BOARDCASTHEADER} ${SPECSRC})


target_link_libraries(net_discover libuv cb_thread cb_sysinfo)

