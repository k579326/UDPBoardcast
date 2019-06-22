

#pragma once


#include "ssnet_define.h"


typedef enum
{
    REQ_Add,
    REQ_Del,
}req_type_en;

typedef enum
{
    CLT_TYPE,
    SVR_TYPE
}peer_type_en;

typedef struct
{
    peer_info_t info;
    req_type_en req_type;
    peer_type_en peer_type;
}conn_req_t;





