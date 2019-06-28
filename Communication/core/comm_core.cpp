

#include <stdint.h>
#include <queue>
#include "comm_core.h"
#include "conn_table.h"
#include "comm_define.h"
#include "sysheader.h"

template <typename T>
struct safe_queue_t
{
    uv_mutex_t mutex;
    uv_sem_t sem_exit;
    uv_thread_t thread;
    std::queue<T> queue;
};


typedef struct
{
    safe_queue_t<asyn_req_t> reqs;
    safe_queue_t<asyn_resp_t> resps;
    uv_loop_t loop;
}common_loop_t;

static common_loop_t gClientLoop;
static common_loop_t gServerLoop;


int init_client_loop()
{
    uv_loop_init(&gClientLoop.loop);

    uv_mutex_init(&gClientLoop.reqs.mutex);
    uv_sem_init(&gClientLoop.reqs.sem_exit, 1);
    for (int i = 0; i < gClientLoop.reqs.queue.size(); i++){ 
        gClientLoop.reqs.queue.pop();
    }

    uv_mutex_init(&gClientLoop.resps.mutex);
    uv_sem_init(&gClientLoop.resps.sem_exit, 1);
    for (int i = 0; i < gClientLoop.resps.queue.size(); i++)
    {
        gClientLoop.resps.queue.pop();
    }

    return 0;
}

int init_server_loop()
{
    uv_loop_init(&gServerLoop.loop);

    uv_mutex_init(&gServerLoop.reqs.mutex);
    uv_sem_init(&gServerLoop.reqs.sem_exit, 1);
    for (int i = 0; i < gServerLoop.reqs.queue.size(); i++)
    {
        gServerLoop.reqs.queue.pop();
    }

    uv_mutex_init(&gServerLoop.resps.mutex);
    uv_sem_init(&gServerLoop.resps.sem_exit, 1);
    for (int i = 0; i < gServerLoop.resps.queue.size(); i++)
    {
        gServerLoop.resps.queue.pop();
    }

    return 0;
}


void timer_cb(uv_timer_t* handle)
{

}

void write_cb(uv_write_t* req, int status)
{
    
}

int do_write(write_task_t* wt)
{
    uv_write_t* w = new uv_write_t;
    uv_timer_t* timer = new uv_timer_t;

    w->data = wt;
    tcp_conn_t* conn = find_client_connect(wt->connId);
    if (!conn)
    {
        ?
    }

    timer->data = w;
    uv_write(w, (uv_stream_t*)&conn->handle, , write_cb);
    
    uv_timer_init(conn->handle.loop, timer);
    uv_timer_start(timer, timer_cb, 5000, 0);

    


}




void client_msg_process(asyn_task_type type, abs_task_t* task)
{
    switch (type)
    {
    case asyn_task_type::WRITE:
    {
        write_task_t* wt = (write_task_t*)task;

        do_write(wt);

        break;
    }
    case asyn_task_type::READ:

        break;
    case asyn_task_type::CONNECT:

        break;
    case asyn_task_type::CLOSE:

        break;
    default:
        
        break;
    }

}



void _async_cb(uv_async_t* handle)
{
    async_req_t* req = (async_req_t*)handle->data;

    // msger不再使用
    uv_sem_post(&req->msger->sem);


    client_msg_process(req->type, req->task);


}





