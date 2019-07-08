



#pragma once

#include "sysheader.h"

#define UDP_PACKAGE_MAX_LEN 1450

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



