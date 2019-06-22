

#pragma once

#include <string>

typedef struct
{
	std::string ip;
	short port;
}peer_info_t;


#define BOARDCAST_MSG_STARTUP		1
#define BOARDCAST_MSG_KEEPALIVE     2
#define BOARDCAST_MSG_SHUTDOWN		3




