
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "comm_define.h"
#include "async_task.h"
#include "core/comm_res.h"
#include "ssnet_err.h"
#include "core/gen_id.h"



int async_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout)
{
    int ret = 0;
    if (!cl_conn_valid(connId))
    {
        return ERR_CONN_NOT_EXIST;
    }

    uv_async_t* async = cl_create_async();
    if (!async)
    {
        return ERR_NOT_READY;
    }

    rw_task_t task;

    task.common.type = RW;
    task.connId = connId;
    task.timeout = timeout;
    task.indata.assign((const char*)indata, inlen);
    uv_sem_init(&task.common.notify, 1);
    uv_sem_wait(&task.common.notify);

    task.common.taskId = ApplyTaskId();
    //uv_timer_init(comm_client_loop(), &task.timer);

    async->data = (void*)&task;

    // 发送异步任务
    uv_async_send(async);

    // 等待处理完成
    uv_sem_wait(&task.common.notify);


    // 检查返回结果，获取数据
    if (task.common.err != 0){
        ret = task.common.err;
        // LOG task.common.errmsg
        goto exit;
    }

    *outdata = malloc(task.outdata.size());
    *outlen = task.outdata.size();
    memcpy(*outdata, task.outdata.c_str(), *outlen);

exit:
    // 清理资源
    // destory_async(async);    // 异步资源交给loop循环释放
    uv_sem_destroy(&task.common.notify);
    return ret;
}

int async_conn(const char* ip, short port, uint32_t timeout)
{
    uv_async_t* async = cl_create_async();
    if (!async)
    {
        return ERR_NOT_READY;
    }
    
    conn_task_t task;

    task.timeout = timeout;
    strcpy(task.ip, ip);
    task.port = port;
    task.common.type = CONNECT;
    uv_sem_init(&task.common.notify, 1);
    uv_sem_wait(&task.common.notify);
    task.common.taskId = ApplyTaskId();

    async->data = (void*)&task;
    
    // 发送异步任务
    uv_async_send(async);

    // 等待处理完成
    uv_sem_wait(&task.common.notify);

    if (task.common.err != 0)
    {
        // LOG task.common.errmsg
    }
    uv_sem_destroy(&task.common.notify);

    return task.common.err;
}

int async_push(const void* indata, int inlen)
{
    uv_async_t* async = sl_create_async();
    if (!async)
    {
        return ERR_NOT_READY;
    }

    push_task_t task;

    task.common.type = PUSH;
    task.common.taskId = 0;
    task.indata.assign((char*)indata, inlen);
    uv_sem_init(&task.common.notify, 1);
    uv_sem_wait(&task.common.notify);

    async->data = (void*)& task;
    
    // 发送异步任务
    uv_async_send(async);

    // 等待处理完成
    uv_sem_wait(&task.common.notify);

    if (task.common.err != 0)
    {
        // LOG task.common.errmsg
    }
    uv_sem_destroy(&task.common.notify);
    return task.common.err;
}


int async_close_client()
{
    uv_async_t* async = cl_create_async();
    if (!async)
    {
        return ERR_NOT_READY;
    }

    close_task_t task;

    task.common.type = CLOSE;
    task.common.taskId = 0;
    uv_sem_init(&task.common.notify, 1);
    uv_sem_wait(&task.common.notify);
    async->data = (void*)& task;
    // 发送异步任务
    uv_async_send(async);

    // 等待处理完成
    uv_sem_wait(&task.common.notify);

    uv_sem_destroy(&task.common.notify);
    return task.common.err;
}
int async_close_server()
{
    uv_async_t* async = sl_create_async();
    if (!async)
    {
        return ERR_NOT_READY;
    }

    close_task_t task;

    task.common.type = CLOSE;
    task.common.taskId = 0;
    uv_sem_init(&task.common.notify, 1);
    uv_sem_wait(&task.common.notify);
    async->data = (void*)& task;
    // 发送异步任务
    uv_async_send(async);

    // 等待处理完成
    uv_sem_wait(&task.common.notify);

    uv_sem_destroy(&task.common.notify);
    return task.common.err;
}














