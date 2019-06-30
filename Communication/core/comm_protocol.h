


#pragma once

#include <stdint.h>

#define PKG_TYPE_ALIVE      0       // 保活数据包
#define PKG_TYPE_COMMON     1       // 普通数据包
#define PKG_TYPE_PUSH       2       // 推送数据包

#define COMM_PROTOCOL_VERSION_1   1
#define COMM_PROTOCOL_VERSION_2   2   

#define COMM_CURRENT_VERSION       COMM_PROTOCOL_VERSION_1 

#define COMM_MAGIC_NUM          0x12345678


#pragma pack(push, 1)
typedef struct
{
    uint8_t         version;
    uint32_t        magic;          // 魔数
    uint64_t        taskId;         // 任务ID,标识回应属于哪个任务，只有普通数据包有效
    uint8_t         type;           // 底层tcp通信的数据包类型
    uint32_t        length;         // 数据长度
    uint8_t         data[0];        // 数据
}comm_pkg_t;
#pragma pack(pop)



comm_pkg_t* proto_build_package(const void* buf, int buflen, uint8_t pkgType, uint64_t taskId);

// 该函数返回的pkg不要使用proto_release_package释放
comm_pkg_t* proto_parse_package(void** buf, int* remainSize);

int proto_check_pkg(comm_pkg_t* pkg);

// 只用于释放proto_build_package 创建的pkg
void proto_release_package(comm_pkg_t* pkg);

