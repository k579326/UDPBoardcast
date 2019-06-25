



add_library(comm_core comm_core.cpp comm_core.h conn_table.cpp conn_table.h)

target_link_libraries(comm_core libuv)