

#pragma once

#include <stdbool.h>
#include "boardcast_define.h"


int bc_setnonblock(SOCKET sockfd);
int bc_cleansocket(SOCKET* sock);
bool bc_checksocket(SOCKET sockfd);


