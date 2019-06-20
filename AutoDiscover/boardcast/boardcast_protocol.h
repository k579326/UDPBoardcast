


#pragma once

#include <stdint.h>
#include "boardcast_define.h"


#define BOARDCAST_PROTOCAL_VERSION 1

#define BOARDCAST_MAGIC_NUM 0x94623744

// 广播消息类型
#define BOARDCAST_MSG_STARTUP		1
#define BOARDCAST_MSG_SHUTDOWN		2



#pragma pack(push, 1)


typedef struct
{
	char cptname[64];		// 当前系统用户名称，结构体大小有限，此字段允许空间不足，copy时注意大小, 使用UTF8编码
	char sysver[128];		// 系统版本，使用UTF8编码
	uint8_t bits;				// 系统位数 0：x86, 1：x64
}system_info_t;


// 此结构体不允许超过 1450 字节 
typedef struct
{
	uint8_t version;
	uint32_t magic;
	system_info_t sys_info;
	uint8_t msg_type;			// 广播消息类型 BOARDCAST_MSG_STARTUP or BOARDCAST_MSG_SHUTDOWN
}boardcast_package_t;

#pragma pack(pop)


void make_shutdown_pkg(boardcast_package_t* pkg);
void make_active_pkg(boardcast_package_t* pkg);



