

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
    uint16_t        connId;         // ��ʶ��Ӧ������������
    uint64_t        taskId;         // ����ID,��ʶ��Ӧ�����ĸ�����
    asyn_resp_type  type;           // ��Ӧ�����ͣ���������ṹ�岻��Ҫ
    uint32_t        datalen;        // ��Ӧ�����ݳ���
    uint8_t* data;           // ��Ӧ������
}asyn_resp_t;



typedef struct
{
    uint16_t        connId;         // ��ʶ�������ĸ�����
    uint64_t        taskId;         // ����ID
    asyn_resp_type  type;           // ��������
    uint32_t        datalen;        // ��������ݳ���
    uint8_t* data;                  // ���������
}asyn_req_t;








