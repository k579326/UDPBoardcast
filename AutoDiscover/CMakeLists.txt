

cmake_minimum_required(VERSION 3.0)


add_library(net_discover STATIC discover.cpp discover.h
			boardcast/boardcast-mgr.h boardcast/boardcast-mgr.cpp
			boardcast/boardcast-server.cpp boardcast/boardcast-server.h
			boardcast/boardcast_common.h boardcast/boardcast_common.cpp)



target_link_libraries(net_discover libuv cb_thread)

