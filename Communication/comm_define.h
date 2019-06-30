

#pragma once


#include <string>
#include "sysheader.h"

using namespace std;

#define RECV_BUF_MAX    (1024 * 1024)




enum asyn_task_type
{
    RW,
    PUSH,           // 推送任务
    READ,
    CONNECT,        // 对于客户端，标识主动连接；对于服务端，标识被动连接
    CLOSE
};


typedef struct
{
    uint32_t        err;
    char            errmsg[128];
    uint64_t        taskId;         // 任务ID
    asyn_task_type  type;
    uv_sem_t        notify;
}abs_task_t;


// Only Read
typedef struct
{
    abs_task_t      common;
    uint16_t        connId;
    string          data;
}read_task_t;

// W&R
typedef struct
{
    abs_task_t      common;
    //uv_timer_t      timer;
    uint32_t        timeout;        // 超时时间：ms
    uint16_t        connId;         // 标识请求发往哪个连接
    string          indata;         // 用户任务数据
    string          outdata;        // 返回的数据
}rw_task_t;

// client do Connect
typedef struct
{
    abs_task_t      common;
    uint32_t        timeout;        // 超时时间：ms
    //uv_timer_t      timer;
    char            ip[64];
    short           port;           // 主机序
}conn_task_t;

// Close
typedef struct
{
    abs_task_t      common;
}close_task_t;






