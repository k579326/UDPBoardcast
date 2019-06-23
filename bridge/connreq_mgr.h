

#pragma once


#include "ssnet_define.h"


typedef enum
{
    REQ_Add,
    REQ_Del,
}req_type_em;


typedef struct
{
    peer_info_t info;
    req_type_em req_type;
}conn_req_t;


// 该回调函数内部不可阻塞，不建议有耗时太多的功能
typedef void (*HOST_CHANGE_CB)(const conn_req_t* req);

// 远端客户端断开的回调
void RegisterClientDisconnectCallback(HOST_CHANGE_CB cb);

// 发现和丢失远端服务器的回调
void RegisterServerChangeCallback(HOST_CHANGE_CB cb);


void deliver_svrchange_msg(req_type_em type, const peer_info_t* peer);
void deliver_cltchange_msg(req_type_em type, const peer_info_t* peer);
void clean_svrchange_msg();
void clean_cltchange_msg();

