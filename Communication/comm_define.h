

#pragma once


#include "sysheader.h"



#define RECV_BUF_MAX    (1024 * 1024)


typedef struct
{
    uv_tcp_t    handle;
    char        ip[64];
    short       port;
}tcp_conn_t;


#define PKG_TYPE_ALIVE  0       // 保活数据包
#define PKG_TYPE_DATA   1       // 普通数据包
#define PKG_TYPE_PUSH   2       // 推送数据包



#pragma pack(push, 1)
typedef struct
{
    uint8_t         version;
    uint64_t        taskId;         // 任务ID,标识回应属于哪个任务，只有普通数据包有效
    uint8_t         type;           // 底层tcp通信的数据包类型
    uint32_t        length;         // 数据长度
    uint8_t         data[0];        // 数据
}comm_pkg_t;
#pragma pack(pop)


enum asyn_task_type
{
    WRITE,
    READ,
    CONNECT,        // 对于客户端，标识主动连接；对于服务端，标识被动连接
    CLOSE
};


typedef void abs_task_t;
// Read
typedef struct
{
    uint32_t        err;
    char            errmsg[64];
    uint16_t        connId;         // 标识请求发往哪个连接
    uint64_t        taskId;         // 任务ID
    uint32_t        datalen;        // 请求的数据长度
    uint8_t         data[];           // 请求的数据
}read_task_t;

// Write/W&R
typedef struct
{
    uint32_t        err;
    char            errmsg[64];
    uint16_t        connId;         // 标识请求发往哪个连接
    uint64_t        taskId;         // 任务ID
    uint32_t        buflen;         // data的buf总长度
    uint32_t        datalen;        // 请求的数据长度
    void*           data;           // 请求的数据
}write_task_t;

// Connect (Client)
typedef struct
{
    uint32_t        err;
    char            errmsg[64];
    uint64_t        taskId;         // 任务ID
    char            ip[64];
    short           port;           // 主机序
}conn_task_t;

// Close
typedef struct
{

}close_task_t;


typedef struct
{
    uv_sem_t 	sem;
    uv_async_t async;
}async_t;


typedef struct
{
    async_t*    msger;
    abs_task_t* task;
    asyn_task_type  type;           // 任务类型
    uv_sem_t    finish;
}async_req_t;






