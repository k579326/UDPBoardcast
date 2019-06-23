

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


#define SVR_BOARDCAST_TIMESPACE (10 * 1000)		// ����˹㲥��� 10s
#define SVR_BC_LISTEN_TIMESPACE 300             // ����˹㲥������� 300ms
#define CLT_BC_LISTEN_TIMESPACE 30              // �ͻ��˹㲥������� 30ms
#define SVR_KEEPALIVE_TIMEOUT   (SVR_BOARDCAST_TIMESPACE * 3)    // ����˶Կͻ������ӵı���ʱ��, 3���Ĺ㲥���


