


#pragma once

#include <stdint.h>
#include "comm_define.h"

int ssn_startup_client(ssn_pushmsg_cb pushmsg_cb, ssn_conn_changed_cb conn_cb);
int ssn_startup_server(ssn_work_process_cb cb, size_t workthread_num);

// 如果返回0，表示成功，*outdata指向的内存需要使用free释放；如果返回非0，*outdata不需要释放
// 如果timeout被设置为0，表示忽略超时设置
int ssn_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout);
int ssn_push(const void* indata, int inlen);
int ssn_connect(char* ip, short port, uint32_t timeout);


int regiester_pushmsg_cb(ssn_pushmsg_cb cb);
int regiester_connect_changed_cb(ssn_conn_changed_cb cb);



// 锁插拔
int send_dongle_change_msg();
int regiester_task_cb();

int server_stop();

