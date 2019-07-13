


#pragma once


#define NONE_RUN_TYPE 0x00      // 客户端、服务端均停止
#define CLT_RUN_TYPE 0x01       // 仅客户端启动
#define SVR_RUN_TYPE 0x02       // 仅服务端启动
#define MIX_RUN_TYPE  0x03      // 客户端、服务端同时启动


int ssn_set_boardcast_model(int runType);




