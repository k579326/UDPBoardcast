
#include "boardcast_common.h"
#include "boardcast_define.h"

int bc_setnonblock(SOCKET sockfd)
{
	int ret = 0;
#ifndef _WIN32
	int flags = fcntl(sockfd, F_GETFL, 0);
	ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#else
	u_long mode = 1;
	ret = ioctlsocket(sockfd, FIONBIO, &mode);
#endif

	return ret;
}

int bc_cleansocket(SOCKET* sock)
{
	if (*sock < 0)
	{
		return 0;
	}

#ifdef _WIN32
	closesocket(*sock);
#else
	close(*sock);
#endif
	* sock = -1;
	return 0;
}


bool bc_checksocket(SOCKET sockfd)
{
#ifdef _WIN32
	return sockfd != INVALID_SOCKET;
#else
	return sockfd >= 0;
#endif
}

