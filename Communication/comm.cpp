

#include "comm.h"
#include "comm_internal.h"
#include "comm_define.h"
#include "async/async_task.h"
#include "ssnet_err.h"
#include "Communication/core/comm_res.h"

int ssn_startup_client(ssn_pushmsg_cb pushmsg_cb, ssn_conn_changed_cb conn_cb)
{
    init_client_loop(pushmsg_cb, conn_cb);
    start_client_loop();
    return 0;
}

int ssn_startup_server(ssn_work_process_cb cb, size_t workthread_num)
{
    init_server_loop();
    start_server_loop(cb, workthread_num);
    return 0;
}


int ssn_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout)
{
    int ret = 0;

    if (indata == NULL || inlen <= 0 || outdata == NULL || outlen == NULL)
    {
        return ERR_PARAM;
    }

    ret = async_send(connId, indata, inlen, outdata, outlen, timeout);
    
    return ret;
}

int ssn_connect(char* ip, short port, uint32_t timeout)
{
    int ret = 0;

    if (ip == NULL)
    {
        return ERR_PARAM;
    }

    return async_conn(ip, port, timeout);
}

int ssn_push(const void* indata, int inlen)
{
    int ret = 0;

    if (indata == NULL || inlen <= 0)
    {
        return ERR_PARAM;
    }

    return async_push(indata, inlen);
}




