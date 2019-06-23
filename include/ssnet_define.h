

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


#define SVR_BOARDCAST_TIMESPACE (10 * 1000)		// 服务端广播间隔 10s
#define SVR_BC_LISTEN_TIMESPACE 300             // 服务端广播监听间隔 300ms
#define CLT_BC_LISTEN_TIMESPACE 30              // 客户端广播监听间隔 30ms
#define SVR_KEEPALIVE_TIMEOUT   (SVR_BOARDCAST_TIMESPACE * 3)    // 服务端对客户端连接的保活时间, 3倍的广播间隔


