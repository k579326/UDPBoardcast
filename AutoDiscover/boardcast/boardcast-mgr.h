

#pragma once

#include <stdint.h>


typedef struct
{
	char ip[64];
	char name[128];
	int peertype;
}peer_info_t;


typedef struct
{
	peer_info_t peer;
	uint64_t time;
}discover_info_t;



int auto_sch_init();

int auto_sch_runas_client();
int auto_sch_runas_server();


int auto_sch_stop_client();
int auto_sch_stop_server();

int auto_sch_uninit();

