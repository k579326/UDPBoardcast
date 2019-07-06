

#pragma once



#include <stdint.h>
#include <vector>


int ssn_connect(const char* ip, short port, uint32_t timeout);

bool ssn_connect_is_exist(const char* ip);



