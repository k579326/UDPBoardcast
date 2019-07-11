
FILE(GLOB COMMCoreSRC core/*)
FILE(GLOB ASYNCSRC async/*)

include_directories(./)

add_library(comm_core comm.cpp ${SELECTADDRSRC} ${COMMCoreSRC} ${ASYNCSRC})

target_link_libraries(comm_core libuv threadpool queue)

