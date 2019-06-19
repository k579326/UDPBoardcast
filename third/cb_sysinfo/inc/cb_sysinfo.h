/**
*   @file       cb_sysinfo.h
*   @version    v1.0.0
*   @date       2017-09-19
*/

#ifndef _CB_SYSINFO_H_
#define _CB_SYSINFO_H_

/** @defgroup cbb_basic CBB基础库
*  http://10.10.1.14/cbb/basic
*  @{
*/
/** @defgroup cb_sysinfo 系统信息库
*  @ingroup cbb_basic
*  http://10.10.1.14/cbb/basic/cb_sysinfo
*/
/** @} */ /* end @defgroup cbb_basic CBB基础库 */

#include "cb_sysinfo_define.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
*   @brief          获取操作系统相关版本信息
*   @param[out]     sys_ver         系统版本名称  建议buff长度为CB_SYSINFO_SV_LEN. 如果不需要获取该信息,这个参数可以传NULL. 两个参数不可同时为空.
*   @param[out]     kern_ver        内核版本     建议buff长度为CB_SYSINFO_KV_LEN. 如果不需要获取该信息,这个参数可以传NULL. 两个参数不可同时为空.
*   @return         如果函数成功，返回CB_SYSINFO_SUCCESS。其他错误码请查看cb_sysinfo_define.h中的错误码宏定义
*   @remarks        因为不同系统的版本号构成都不相同, 所以返回的版本信息没有统一的格式
                    Windows: 
                        系统版本名称, 例如win7 x64系统上该接口返回系统版本名称为 "Microsoft Windows 7(build *****) x64"
                        内核版本,    格式为"主版本号.从版本号.构建版本号"
                    unix & linux:
                        系统版本名称  mac:    系统名称(主版本.从版本.bugfix版本) 
                                      linux:  暂时不支持获取系统版本名称
                        内核版本,    格式为"发布版本 系统版本". 例如centos7上的该接口返回的内核版本为 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016
*   @ingroup        cb_sysinfo
*/
int cb_sysinfo_version(char *sys_ver, char *kern_ver);

/*!
*   @brief          获取当前系统的位数(x86还是x64)
*   @param[out]     bits            枚举值, 标识当前系统位数. 
                                    如果当前系统是x64, bits被设置为CB_SYSINFO_X64; 
                                    如果当前系统是32位系统, bits被设置为CB_SYSINFO_X86.
*   @return         如果函数成功 返回CB_SYSINFO_SUCCESS. 其他错误码请查看cb_sysinfo_define.h中的错误码宏定义
*   @see            CB_SYSINFO_X64, CB_SYSINFO_X86
*   @ingroup        cb_sysinfo
*/
int cb_sysinfo_bits(int *bits);

/*!
*   @brief          获取当前计算机的名称
*   @param[out]     name            计算机名称，建议此buff长度为256字节, 详细查看remarks.
*   @param[inout]   len             name指向的缓冲区的长度的指针.
*   @return         如果函数成功 返回CB_SYSINFO_SUCCESS. 并且len被设置为获取到的计算机名的长度 
                    如果传入的缓冲区不足,返回CB_SYSINFO_ERROR_BUFFER_NOT_ENOUGH
                    其他错误码请查看cb_sysinfo_define.h中的错误码宏定义.
*   @remarks        linux计算机名最大长度为255, windows计算机名最大长度为15. 
*   @ingroup        cb_sysinfo
*/
int cb_sysinfo_computer_name(char *name, int *len);


/*!
*   @brief          获取当前内存使用情况
*   @param[out]     meminfo         目前该结构体中包含总物理内存大小、可用物理内存大小、总虚拟内存大小、可用虚拟内存大小。单位全部为byte
*   @return         如果函数成功 返回CB_SYSINFO_SUCCESS. 其他错误码请查看cb_sysinfo_define.h中的错误码宏定义
*   @ingroup        cb_sysinfo
*/
int cb_sysinfo_meminfo(cb_sysinfo_mem_t *meminfo);


unsigned long long cb_sysinfo_time();

#ifdef __cplusplus
}
#endif


#endif /* !_CB_SYSINFO_H_ */

