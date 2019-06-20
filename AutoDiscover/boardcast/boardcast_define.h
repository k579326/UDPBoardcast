



#pragma once

#include "uv.h"

#ifdef _WIN32
#include <Windows.h>
#include <WS2tcpip.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#define SVR_BOARDCAST_TIMESPACE (10 * 1000)		// 服务端广播间隔 10s
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



