


#pragma once




#define ERR_PARAM               0x00000001
#define ERR_NOT_READY           0x00000002
#define ERR_CONN_NOT_EXIST      0x00000003
#define ERR_CONN_ALREADY_EXIST  0x00000004
#define ERR_TIMEOUT             0x00000005
#define ERR_SHUTDOWN            0x00000006


#define ERR_COMM_PROTO_VERSION  0x00000007
#define ERR_BAD_COMM_PACKAGE    0x00000008
#define ERR_UV_LIBRARY          0x00000009

const char* ssn_errmsg(int err);


