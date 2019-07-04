

#pragma once


#include <string>
#include "sysheader.h"

using namespace std;

#define RECV_BUF_MAX    (1024 * 1024)



// 用于客户端接收推送消息的回调
typedef void(*ssn_pushmsg_cb)(uint16_t connId, const void* data, int datalen);

// 连接状态改变回调，如果status为true, 表示新建连接；如果status为false,表示断开连接
typedef void(*ssn_conn_changed_cb)(uint16_t connId, const char* ip, bool status);

// 服务端任务处理函数
typedef void(*ssn_work_process_cb)(const void* indata, int inlen, void* outdata, int* outlen);


enum async_task_type
{
    RW,
    PUSH,           // 推送任务，每个异步请求需指定推送的连接
    RESP,           // 服务端异步任务，用于回应客户端请求
    CONNECT,        // 对于客户端，标识主动连接；对于服务端，标识被动连接
    CLOSE
};


typedef struct
{
    uint32_t        err;
    char            errmsg[128];
    uint64_t        taskId;         // 任务ID
    async_task_type  type;
    uv_sem_t        notify;
}abs_task_t;


// Resp (Server)
typedef struct
{
    abs_task_t      common;
    uint16_t        connId;
    string          data;
    ssn_work_process_cb cb;
}resp_task_t;

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

// PUSH
typedef struct
{// 推送消息，无超时，推向所有连接，无返回
    abs_task_t      common;
    string          indata;         // 用户任务数据
}push_task_t;


// client do Connect
typedef struct
{
    abs_task_t      common;
    uint32_t        timeout;        // 超时时间：ms
    char            ip[64];
    short           port;           // 主机序
}conn_task_t;

// Close
typedef struct
{
    abs_task_t      common;
}close_task_t;









