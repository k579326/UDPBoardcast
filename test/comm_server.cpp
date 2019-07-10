
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "discover.h"
#include "Communication/comm.h"
#include "sysheader.h"
#include "uv.h"

void _server_msg_handler(const void* indata, int inlen, void* outdata, int* outlen)
{
    char inbuf[256] = { 0 };
    char outbuf[256] = { 0 };

    memcpy(inbuf, indata, inlen);
    memcpy(outbuf, (char*)indata, inlen);
    strcat(outbuf, " ----> RESP!");
    memcpy(outdata, outbuf, strlen(outbuf));

    printf("[Recv Msg] recv: {%s}, len: %d. send: {%s}, len: %d\n", 
           inbuf, inlen, outbuf, (int)strlen(outbuf));

    *outlen = strlen(outbuf);
    return;
}


void server_thread(void* param)
{
    char pushmsg[] = "push msg!";
    while (1)
    {
        //ssn_push(pushmsg, strlen(pushmsg));
        ssn_sleep(2000);
    }
}


int main()
{
    uv_thread_t thread;

    ssn_startup_server(_server_msg_handler, 16);

    uv_thread_create(&thread, server_thread, NULL);


    nd_boardcast_init();
    nd_set_running_type(SVR_RUN_TYPE);

    getchar();

    ssn_shutdown_server();
    nd_boardcast_uninit();
    //uv_sem_t sem;
    //uv_sem_init(&sem, 1);
    //uv_sem_wait(&sem);
    //uv_sem_wait(&sem);

    return 0;
}



