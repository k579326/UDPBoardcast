


#pragma once


#include <map>

#include "ssnet_define.h"
#include "comm_define.h"

typedef enum
{
	CLIENT_LOOP = 0x01,
	SERVER_LOOP = 0x02,
}loop_type_t;

typedef enum
{
    TCP_CLIENT,
    TCP_SERVER,
}tcp_type_t;


typedef struct
{
    uv_tcp_t*   handle;
    tcp_type_t  type;
    void*       cache;
    int         maxlength;  // cache max length
    int         length;     // cache length
}comm_tcp_t;


typedef struct
{
    comm_tcp_t  tcp;
    peer_info_t	info;
}tcp_conn_t;

typedef struct{
	std::map<uint16_t, tcp_conn_t> table;
	uv_rwlock_t connLock;
}safe_conn_table;

typedef struct{
	std::map<uint64_t, abs_task_t*> taskTable;
	uv_mutex_t taskLock;
}safe_task_table;


struct loop_info_t
{
    bool running;
    bool inited;
    loop_type_t type;
    uv_loop_t loop;
};

struct client_loop_t
{
    loop_info_t loop_info;

	safe_conn_table connTable;
	safe_task_table taskTable;
	
	
	uv_thread_t thread;
    uv_mutex_t  condlock;
    uv_cond_t   cond;
};

struct server_loop_t
{
    loop_info_t loop_info;

    safe_conn_table connTable;

    uv_thread_t thread;
    comm_tcp_t  listen;
};



void init_client_loop();
int uninit_client_loop();
int start_client_loop();
int stop_client_loop();

loop_type_t loop_type(uv_loop_t* loop);





int create_clt_tcp(comm_tcp_t* clt_tcp);


int cl_conn_add();
tcp_conn_t* cl_conn_del();
tcp_conn_t* cl_conn_find(uint16_t connId);



int cl_task_add(const abs_task_t* task);
abs_task_t* cl_task_del(uint64_t taskId);
abs_task_t* cl_task_find(uint64_t taskId);


bool cl_conn_valid(uint16_t connId);

uv_loop_t* cl_loop();
uv_async_t* cl_create_async();




void init_server_loop();
int uninit_server_loop();


uv_loop_t* sl_loop();
uv_async_t* sl_create_async();













