

#pragma once


#include "sysheader.h"



typedef struct
{
    uv_tcp_t    handle;
    char        ip[64];
    short       port;
}tcp_conn_t;







enum asyn_resp_type
{
    WRITE_DONE,
    READ_DONE,
    CONN_COMING,
    ACCEPT_DONE,
    TIMEOUT,
};

enum asyn_req_type
{
    WRITE,
    READ,
    CONNECT,

};

typedef struct
{
    uint32_t        err;
    uint16_t        connId;         // 标识回应来自哪条连接
    uint64_t        taskId;         // 任务ID,标识回应属于哪个任务
    asyn_resp_type  type;           // 回应的类型，可能这个结构体不需要
    uint32_t        datalen;        // 回应的数据长度
    uint8_t* data;           // 回应的数据
}asyn_resp_t;



typedef struct
{
    uint16_t        connId;         // 标识请求发往哪个连接
    uint64_t        taskId;         // 任务ID
    asyn_resp_type  type;           // 任务类型
    uint32_t        datalen;        // 请求的数据长度
    uint8_t* data;                  // 请求的数据
}asyn_req_t;








