

#pragma once




typedef struct
{
	char ip[64];
}host_info_t;



int conn_add_client(char* ip);
int conn_del_client(char* ip);
int conn_clr_client(char* ip);

int conn_add_server(char* ip);
int conn_del_server(char* ip);
int conn_clr_server(char* ip);


