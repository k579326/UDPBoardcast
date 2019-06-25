

#pragma once


#include "comm_define.h"


#define INVALID_CONN_ID -1


int add_client_connect(const tcp_conn_t* conn);
int del_client_connect();


int add_server_connect(const tcp_conn_t* conn);
int del_server_connect();








