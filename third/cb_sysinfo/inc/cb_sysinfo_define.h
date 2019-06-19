
#ifndef _CB_SYSINFO_DEFINE_H_
#define _CB_SYSINFO_DEFINE_H_

/** 操作系统版本最大长度 */
#define CB_SYSINFO_SV_LEN       256
/** 内核版本最大长度   linux系统版本和发布版本的长度都是56, 拼接起来也就不到120个字符*/ 
#define CB_SYSINFO_KV_LEN       256 

#define    CB_SYSINFO_X86               0
#define    CB_SYSINFO_X64               1


typedef struct
{
    /** 总物理内存大小(byte)*/
    unsigned long long total_phys_mem;
    /** 可用物理内存大小(byte)*/
    unsigned long long avail_phys_mem;
    /** 总虚拟内存大小(byte)*/
    unsigned long long total_vir_mem;
    /** 可用虚拟内存大小(byte)*/
    unsigned long long avail_vir_mem;
}cb_sysinfo_mem_t;




/** 错误码*/

#define CB_SYSINFO_SUCCESS                          0

/** 参数错误*/
#define CB_SYSINFO_ERROR_INVALID_PARAM              0xCB040001
/** 缓冲区不足*/
#define CB_SYSINFO_ERROR_BUFFER_NOT_ENOUGH          0xCB040002
/** 系统API返回错误*/
#define CB_SYSINFO_ERROR_SYSAPI_FAILED              0xCB040003
/** 系统版本太低*/
#define CB_SYSINFO_ERROR_NOT_SUPPORT_SYSTEM         0xCB040004
/** 未知信息*/
#define CB_SYSINFO_ERROR_UNKNOWN_INFORMATION        0xCB040005

#endif //! _CB_SYSINFO_DEFINE_H_

