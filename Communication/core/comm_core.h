


#pragma once


#define CLIENT_LOOP 0
#define SERVER_LOOP 1




int CreateClient(const char* ip);
int CreateServer(const char* ip, short port);


int init_client_loop();
int init_server_loop();










