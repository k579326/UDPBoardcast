
#include <string.h>
#include "cb_sysinfo.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"

int setnonblock(SOCKET sockfd)
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

int cleansocket(SOCKET* sock)
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


bool checksocket(SOCKET sockfd)
{
#ifdef _WIN32
	return sockfd != INVALID_SOCKET;
#else
	return sockfd >= 0;
#endif
}

// ���� ip: ������
std::string NetIpToString(in_addr ip)
{
	char ipstring[64];
	inet_ntop(AF_INET, &ip, ipstring, 64);
	return ipstring;
}

// ����ֵ�� ������
in_addr StringToNetIp(const char* ip)
{
	in_addr addr;
	inet_pton(AF_INET, ip, &addr);
	return addr;
}


SOCKET create_boardcast_socket()
{
	int ret = 0;
	SOCKET skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!checksocket(skfd))
	{
		return -1; // TODO:
	}

	int optval = 1;
	ret = setsockopt(skfd, SOL_SOCKET, SO_BROADCAST, (char*)& optval, sizeof(int));
	if (ret < 0)
	{
		cleansocket(&skfd); // TODO:
		return -1;
	}

	return skfd;
}

SOCKET create_listen_udp_socket(short port)
{
	int ret = 0;
	sockaddr_in addr;

	SOCKET skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!checksocket(skfd))
	{
		return -1; // TODO:
	}
	ret = setnonblock(skfd);
	if (ret < 0)
	{
		cleansocket(&skfd); // TODO:
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	ret = bind(skfd, (sockaddr*)& addr, sizeof(addr));
	if (ret < 0)
	{
		cleansocket(&skfd); // TODO:
		return -1;
	}

	return skfd;
}


SOCKET create_udp_socket()
{
	int ret = 0;
	SOCKET skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!checksocket(skfd))
	{
		return -1; // TODO:
	}
	return skfd;
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

