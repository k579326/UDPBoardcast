

#pragma once




typedef struct
{
	char ip[64];
	char cptname[128];
	
};



int conn_add();
int conn_del();
int conn_clr();