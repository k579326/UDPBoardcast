



#pragma once

#include "sysheader.h"

#define SVR_BOARDCAST_TIMESPACE (10 * 1000)		// 服务端广播间隔 10s
#define SVR_KEEPALIVE_TIMEOUT   (SVR_BOARDCAST_TIMESPACE * 3)    // 服务端对客户端连接的保活时间, 3倍的广播间隔
#define UDP_PACKAGE_MAX_LEN 1450

#define SERVER_PORT 10034
#define CLIENT_PORT 10035


#ifndef _WIN32
#define SOCKET long long 
#endif

typedef struct
{
	bool pause;
	SOCKET sockfd;
	uv_thread_t thread;
	uv_mutex_t mutex;
	uv_cond_t cond;
	uv_sem_t sem_exit;
}socket_env_t;



