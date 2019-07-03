


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
    abs_task_t* task;
    uv_timer_t* timer;
}timer_data_t;

typedef struct
{
    comm_tcp_t  tcp;
    peer_info_t	info;
}tcp_conn_t;

typedef struct{
	std::map<uint16_t, tcp_conn_t*> table;
	uv_rwlock_t connLock;
}safe_conn_table;

typedef struct{
	std::map<uint64_t, abs_task_t*> taskTable;
	uv_mutex_t taskLock;
}safe_task_table;

typedef struct
{
    std::map<uint64_t, timer_data_t*> timerTable;
    uv_mutex_t timerLock;
}safe_timer_table;


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
    safe_timer_table timerTable;
	
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


int cl_conn_add(uint16_t connId, const tcp_conn_t* conn);
tcp_conn_t* cl_conn_del(uint16_t connId);
tcp_conn_t* cl_conn_find(uint16_t connId);
std::map<uint16_t, tcp_conn_t*> cl_conn_list();


int cl_task_add(uint64_t taskId, const abs_task_t* task);
abs_task_t* cl_task_del(uint64_t taskId);
abs_task_t* cl_task_find(uint64_t taskId);

int cl_timer_add(uint64_t taskId, const timer_data_t* timer);
timer_data_t* cl_timer_del(uint64_t taskId);
timer_data_t* cl_timer_find(uint64_t taskId);

bool cl_conn_valid(uint16_t connId);

uv_loop_t* cl_loop();
uv_async_t* cl_create_async();




void init_server_loop();
int uninit_server_loop();


int sl_conn_add(uint16_t connId, const tcp_conn_t* conn);
tcp_conn_t* sl_conn_del(uint16_t connId);
tcp_conn_t* sl_conn_find(uint16_t connId);





uv_loop_t* sl_loop();
uv_async_t* sl_create_async();













