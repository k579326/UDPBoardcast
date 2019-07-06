

#pragma once

#include <stdint.h>
#include "sysheader.h"


// 该回调函数内部不可阻塞，不建议有耗时太多的功能
typedef void (*DISCOVER_SVR_CB)(const char* ip, short port, uint32_t timeout);


// 注册连接远端服务器的接口
void RegisterConnectServerCallback(DISCOVER_SVR_CB cb);


void deliver_addsvr_msg(const char* ip, short port);




