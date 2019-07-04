


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Communication/comm.h"
#include "uv.h"


// ���ڿͻ��˽���������Ϣ�Ļص�
typedef void(*ssn_pushmsg_cb)(uint16_t connId, const void* data, int datalen);

// ����״̬�ı�ص������statusΪtrue, ��ʾ�½����ӣ����statusΪfalse,��ʾ�Ͽ�����
typedef void(*ssn_conn_changed_cb)(uint16_t connId, const char* ip, bool status);

// �������������
typedef void(*ssn_work_process_cb)(const void* indata, int inlen, void* outdata, int* outlen);


void push_msg_handler(uint16_t connId, const void* data, int datalen)
{
    char pushmsg[256] = {0};

    memcpy(pushmsg, data, datalen);
    printf("[Clt Recv Pushmsg] %s\n", pushmsg);
}


void client_send_msg(void* param)
{
    void* outbuf;
    

    while (1)
    {
        int outlen = 256;
        char outstr[256] = { 0 };
        ssn_send(0, "123456", 6, &outbuf, &outlen, 10000);

        memcpy(outstr, outbuf, outlen);
        printf("[clt recv resp] %s\n", outstr);
        free(outbuf);

        Sleep(200);
    }
}

int main()
{
    uv_thread_t thread;
    uv_sem_t sem;
    uv_sem_init(&sem, 1);
    uv_sem_wait(&sem);

    ssn_startup_client(push_msg_handler, NULL);
    ssn_connect("192.168.1.3", 10038, 3000);

    uv_thread_create(&thread, client_send_msg, NULL);
    
    uv_sem_wait(&sem);
    

    return 0;
}





