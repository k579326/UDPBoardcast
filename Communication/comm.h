


#pragma once

#include <stdint.h>


int client_start();

// �������0����ʾ�ɹ���*outdataָ����ڴ���Ҫʹ��free�ͷţ�������ط�0��*outdata����Ҫ�ͷ�
// ���timeout������Ϊ0����ʾ���Գ�ʱ����
int ssn_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout);
int ssn_push(const void* indata, int inlen);

int regiester_dongle_changed_cb();
int regiester_connect_changed_cb();
int client_stop();



int server_start();

// �����
int send_dongle_change_msg();
int regiester_task_cb();

int server_stop();

