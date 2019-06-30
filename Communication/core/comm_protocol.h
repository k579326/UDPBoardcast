


#pragma once

#include <stdint.h>

#define PKG_TYPE_ALIVE      0       // �������ݰ�
#define PKG_TYPE_COMMON     1       // ��ͨ���ݰ�
#define PKG_TYPE_PUSH       2       // �������ݰ�

#define COMM_PROTOCOL_VERSION_1   1
#define COMM_PROTOCOL_VERSION_2   2   

#define COMM_CURRENT_VERSION       COMM_PROTOCOL_VERSION_1 

#define COMM_MAGIC_NUM          0x12345678


#pragma pack(push, 1)
typedef struct
{
    uint8_t         version;
    uint32_t        magic;          // ħ��
    uint64_t        taskId;         // ����ID,��ʶ��Ӧ�����ĸ�����ֻ����ͨ���ݰ���Ч
    uint8_t         type;           // �ײ�tcpͨ�ŵ����ݰ�����
    uint32_t        length;         // ���ݳ���
    uint8_t         data[0];        // ����
}comm_pkg_t;
#pragma pack(pop)



comm_pkg_t* proto_build_package(const void* buf, int buflen, uint8_t pkgType, uint64_t taskId);

// �ú������ص�pkg��Ҫʹ��proto_release_package�ͷ�
comm_pkg_t* proto_parse_package(void** buf, int* remainSize);

int proto_check_pkg(comm_pkg_t* pkg);

// ֻ�����ͷ�proto_build_package ������pkg
void proto_release_package(comm_pkg_t* pkg);

