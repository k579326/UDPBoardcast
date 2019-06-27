

#pragma once


#include "comm_define.h"


#define INVALID_CONN_ID -1


int add_client_connect(const tcp_conn_t* conn);
tcp_conn_t* find_client_connect(uint16_t connId);
int del_client_connect();


int add_server_connect(const tcp_conn_t* conn);
int del_server_connect();








