

#pragma once

#include "comm_define.h"


int async_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout);

int async_conn(const char* ip, short port, uint32_t timeout);

int async_push(const void* indata, int inlen);

int async_close_client();
int async_close_server();



