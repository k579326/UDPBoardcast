
#include "cb_sysinfo.h"
#include "boardcast_protocol.h"
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


system_info_t* systemInfo()
{
	char sysver[512] = { 0 };
	char cptname[256] = { 0 };
	int len = 256;
	int bits = -1;
	static bool binit = false;
	static system_info_t g_sysinfo = { 0 };
	
	if (!binit)
	{
		cb_sysinfo_version(sysver, NULL);
		strncpy(g_sysinfo.sysver, sysver, sizeof(g_sysinfo.sysver) - 1);

		cb_sysinfo_computer_name(cptname, &len);
		strncpy(g_sysinfo.cptname, cptname, sizeof(g_sysinfo.cptname) - 1);

		cb_sysinfo_bits(&bits);
		g_sysinfo.bits = (uint8_t)bits;

		binit = true;
	}

	return &g_sysinfo;
}

