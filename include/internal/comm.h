

#pragma once

#include <stdint.h>
#include <stddef.h>


/**************************** 客户端接口 *****************************/

// 用于客户端接收推送消息的回调
typedef void(*ssn_pushmsg_cb)(uint16_t connId, const void* data, int datalen);

// 连接状态改变回调，如果status为true, 表示新建连接；如果status为false,表示断开连接
typedef void(*ssn_conn_changed_cb)(uint16_t connId, const char* ip, bool status);


void ssn_startup_client(ssn_pushmsg_cb pushmsg_cb, ssn_conn_changed_cb conn_cb);

void ssn_connect_oriented(const char* ip, short port);

// 如果返回0，表示成功，*outdata指向的内存需要使用free释放；如果返回非0，*outdata不需要释放
// 如果timeout被设置为0，表示忽略超时设置。 单位：ms
int ssn_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout);

int ssn_shutdown_client();



/****************** 服务端接口 ***********************/
// 服务端任务处理函数
typedef void(*ssn_work_process_cb)(const void* indata, int inlen, void* outdata, int* outlen);

// workthread_num 服务端处理任务的线程数
int ssn_startup_server(ssn_work_process_cb cb, size_t workthread_num);
int ssn_push(const void* indata, int inlen);
int ssn_shutdown_server();



