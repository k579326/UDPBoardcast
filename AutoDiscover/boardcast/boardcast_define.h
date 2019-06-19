



#pragma once

#ifdef _WIN32
#include <Windows.h>
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





