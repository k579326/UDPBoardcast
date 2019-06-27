

#pragma once


#include "sysheader.h"



typedef struct
{
    uv_async_t  h;
    bool        used;
}async_res_t;

typedef struct
{
    uv_tcp_t    handle;
    async_res_t async[2];
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


enum asyn_req_type
{
    WRITE,
    READ,
    CONNECT,
    CLOSE
};

typedef struct
{
    uint32_t        err;
    uint16_t        connId;         // 标识回应来自哪条连接
    uint64_t        taskId;         // 任务ID,标识回应属于哪个任务
    asyn_resp_type  type;           // 回应的类型，可能这个结构体不需要
    uint32_t        datalen;        // 回应的数据长度
    void*           data;           // 回应的数据
}asyn_resp_t;



typedef struct
{
    uint16_t        connId;         // 标识请求发往哪个连接
    uint64_t        taskId;         // 任务ID
    asyn_resp_type  type;           // 任务类型
    uint32_t        datalen;        // 请求的数据长度
    void*           data;           // 请求的数据
}asyn_req_t;

typedef struct
{
    uv_async_t async;

}async_task_t;






