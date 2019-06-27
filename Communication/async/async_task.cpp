

#include "comm_define.h"
#include "async_task.h"'
#include "async_res.h"

int send_async_task(write_task_t* task)
{
    async_req_t req;

    req.task = task;
    uv_sem_init(&req.finish, 1);
    uv_sem_wait(&req.finish);

    req.msger = pick_async_res();
    req.msger->async.data= &req;

    // insert task finish sem to queue

    uv_sem_wait(&req.msger->sem);
    uv_async_send(&req.msger->async);
    uv_sem_wait(&req.msger->sem);

    back_async_res(&req.msger);


    uv_sem_wait(&req.finish);


}


















