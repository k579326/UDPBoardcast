

#include "comm.h"
#include "comm_internal.h"
#include "comm_define.h"
#include "async/async_task.h"
#include "ssnet_err.h"
#include "Communication/core/comm_res.h"


int ssn_startup_client(ssn_pushmsg_cb pushmsg_cb, ssn_conn_changed_cb conn_cb)
{
    init_client_loop(pushmsg_cb, conn_cb);
    return start_client_loop();
}

int ssn_shutdown_client()
{
    async_close_client();
    return uninit_client_loop();
}



int ssn_startup_server(ssn_work_process_cb cb, size_t workthread_num)
{
    init_server_loop(cb, workthread_num);
    return start_server_loop();
}

int ssn_shutdown_server()
{
    async_close_server();
    return uninit_server_loop();
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

int ssn_connect(const char* ip, short port, uint32_t timeout)
{
    int ret = 0;

    if (ip == NULL)
    {
        return ERR_PARAM;
    }

    if (ssn_connect_is_exist(ip))
    {
        return ERR_CONN_ALREADY_EXIST;
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


bool ssn_connect_is_exist(const char* ip)
{
    if (ip == NULL)
    {
        return false;
    }
    
    return NULL != cl_conn_find2(ip);
}







