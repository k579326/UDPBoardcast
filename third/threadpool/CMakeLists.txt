

add_library(threadpool STATIC threadpool.c threadpool.h)

target_link_libraries(threadpool libuv queue)

set_target_properties(threadpool PROPERTIES FOLDER "internal")