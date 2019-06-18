


#pragma once



#define CLT_RUN_TYPE 0
#define SVR_RUN_TYPE 1
#define CS_RUN_TYPE  2


int nd_set_running_type(int runType);

int nd_test_oriented_server();

int nd_add_oriented_server();

int nd_boardcast_init();

int nd_boardcast_startup(int runType);

int nd_boardcast_shutdown(int runType);

int nd_boardcast_uninit();

