

#pragma once

#include <stdbool.h>
#include <string>
#include "boardcast_define.h"


int setnonblock(SOCKET sockfd);
int cleansocket(SOCKET* sock);
bool checksocket(SOCKET sockfd);

// 参数 ip: 网络序
std::string NetIpToString(in_addr ip);

// 返回值： 网络序
in_addr StringToNetIp(const char* ip);

SOCKET create_boardcast_socket();
SOCKET create_listen_udp_socket(short port);
SOCKET create_udp_socket();

system_info_t* systemInfo();

