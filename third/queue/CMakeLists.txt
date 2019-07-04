

add_library(queue STATIC Gqueue.c Gqueue.h)

set_target_properties(queue PROPERTIES FOLDER "internal")