


#pragma once

#include <stdint.h>
#include "boardcast_define.h"


#define BOARDCAST_PROTOCAL_VERSION 1

#define BOARDCAST_MAGIC_NUM 0x94623744

// �㲥��Ϣ����
#define BOARDCAST_MSG_STARTUP		1
#define BOARDCAST_MSG_SHUTDOWN		2



#pragma pack(push, 1)


typedef struct
{
	char cptname[64];		// ��ǰϵͳ�û����ƣ��ṹ���С���ޣ����ֶ�����ռ䲻�㣬copyʱע���С, ʹ��UTF8����
	char sysver[128];		// ϵͳ�汾��ʹ��UTF8����
	uint8_t bits;				// ϵͳλ�� 0��x86, 1��x64
}system_info_t;


// �˽ṹ�岻������ 1450 �ֽ� 
typedef struct
{
	uint8_t version;
	uint32_t magic;
	system_info_t sys_info;
	uint8_t msg_type;			// �㲥��Ϣ���� BOARDCAST_MSG_STARTUP or BOARDCAST_MSG_SHUTDOWN
}boardcast_package_t;

#pragma pack(pop)



void sys_environment_init();

void make_shutdown_pkg(boardcast_package_t* pkg);
void make_discover_pkg(boardcast_package_t* pkg);



