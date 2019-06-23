

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


// �ûص������ڲ������������������к�ʱ̫��Ĺ���
typedef void (*HOST_CHANGE_CB)(const conn_req_t* req);

// Զ�˿ͻ��˶Ͽ��Ļص�
void RegisterClientDisconnectCallback(HOST_CHANGE_CB cb);

// ���ֺͶ�ʧԶ�˷������Ļص�
void RegisterServerChangeCallback(HOST_CHANGE_CB cb);


void deliver_svrchange_msg(req_type_em type, const peer_info_t* peer);
void deliver_cltchange_msg(req_type_em type, const peer_info_t* peer);
void clean_svrchange_msg();
void clean_cltchange_msg();

