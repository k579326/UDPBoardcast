


#pragma once


#define NONE_RUN_TYPE 0x00
#define CLT_RUN_TYPE 0x01
#define SVR_RUN_TYPE 0x02
#define MIX_RUN_TYPE  0x03


int nd_set_running_type(int runType);

int nd_boardcast_init();

int nd_boardcast_uninit();




