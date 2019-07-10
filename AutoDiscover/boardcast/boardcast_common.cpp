
#include <string.h>
#include "cb_sysinfo.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"
#include "select_network.h"

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

// 参数 ip: 网络序
std::string NetIpToString(in_addr ip)
{
	return inet_ntoa(ip);
}

// 返回值： 网络序
in_addr StringToNetIp(const char* ip)
{
    sockaddr_in addr;
    uv_ip4_addr(ip, 0, &addr);
	return addr.sin_addr;
}


SOCKET create_boardcast_socket()
{
	int ret = 0;
    sockaddr_in localaddr;

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
	addr.sin_addr.s_addr = PhyIpAddress();
    // addr.sin_addr = StringToNetIp("192.168.109.1");
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
    sockaddr_in localaddr;

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

