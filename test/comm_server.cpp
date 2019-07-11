
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "discover.h"
#include "comm.h"
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
    int err;
    uv_thread_t thread;

    err = ssn_startup_server(_server_msg_handler, 16);
    if (err != 0)
    {
        return err;
    }

    uv_thread_create(&thread, server_thread, NULL);

    nd_set_running_type(SVR_RUN_TYPE);
    getchar();

    nd_set_running_type(NONE_RUN_TYPE);
    ssn_shutdown_server();
    
    return 0;
}



