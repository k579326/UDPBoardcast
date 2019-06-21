

#pragma once

#include <stdbool.h>
#include "boardcast_define.h"


int setnonblock(SOCKET sockfd);
int cleansocket(SOCKET* sock);
bool checksocket(SOCKET sockfd);

SOCKET create_boardcast_socket();
SOCKET create_listen_udp_socket(short port);
SOCKET create_udp_socket();
unsigned long get_boardcast_addr();

system_info_t* systemInfo();

