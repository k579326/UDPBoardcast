

#include "comm.h"
#include "comm_define.h"
#include "async/async_task.h"


int send(uint16_t connId, const void* indata, int inlen, void* outdata, int* outlen)
{
    write_task_t task = { 0 };

    if (inlen > RECV_BUF_MAX)
    {
        return -1;
    }

    task.data = malloc(RECV_BUF_MAX);
    if (task.data == NULL)
    {
        return -1;
    }

    task.buflen = RECV_BUF_MAX;
    task.datalen = inlen;
    memcpy(task.data, indata, inlen);

    task.connId = connId;
    task.taskId = ;

    
}