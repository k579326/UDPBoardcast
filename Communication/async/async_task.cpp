

#include "comm_define.h"
#include "async_task.h"
#include "core/comm_res.h"
#include "ssnet_err.h"
#include "core/task_id.h"



int async_send(uint16_t connId, const void* indata, int inlen, void** outdata, int* outlen, uint32_t timeout)
{
    int ret = 0;
    if (cl_conn_valid(connId))
    {
        return ERR_CONN_NOT_EXIST;
    }

    uv_async_t* async = cl_create_async();
    if (!async)
    {
        return ERR_NOT_READY;
    }

    rw_task_t task;

    task.common.type = asyn_task_type::RW;
    task.connId = connId;
    task.timeout = timeout;
    task.indata.assign((const char*)indata, inlen);
    uv_sem_init(&task.common.notify, 1);
    uv_sem_wait(&task.common.notify);

    task.common.taskId = ApplyTaskId();
    //uv_timer_init(comm_client_loop(), &task.timer);

    async->data = (void*)&task;
    // 放入任务队列
    cl_task_add((abs_task_t*)&task);

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

int async_conn(char* ip, short port, uint32_t timeout)
{

}

int async_push(const void* indata, int inlen)
{

}
















