

#pragma once




typedef struct
{
	char ip[64];
};



int conn_add_client();
int conn_del_client();
int conn_clr_client();

int conn_add_server();
int conn_del_server();
int conn_clr_server();


