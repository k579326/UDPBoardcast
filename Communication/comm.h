


#pragma once

#include <stdint.h>


int client_start();

// 如果返回0，表示成功，*outdata指向的内存需要使用free释放；如果返回非0，*outdata不需要释放
// 如果timeout被设置为0，表示忽略超时设置
int ssn_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout);
int ssn_push(const void* indata, int inlen);

int regiester_dongle_changed_cb();
int regiester_connect_changed_cb();
int client_stop();



int server_start();

// 锁插拔
int send_dongle_change_msg();
int regiester_task_cb();

int server_stop();

