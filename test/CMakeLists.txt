

cmake_minimum_required(VERSION 3.0)

include_directories(../include)
include_directories(../AutoDiscover)
include_directories(tinyxml)
add_subdirectory(tinyxml)
#add_executable(testBc test.cpp)
#add_executable(uvclient uvclient.cpp)
#add_executable(uvserver uvserver.cpp)

add_executable(commClient comm_client.cpp)
add_executable(commServer comm_server.cpp)


target_link_libraries(commClient libuv comm_core net_discover tinyxml)
target_link_libraries(commServer libuv comm_core net_discover tinyxml)
#target_link_libraries(uvserver libuv)
#target_link_libraries(uvclient libuv)
#target_link_libraries(testBc net_discover)
