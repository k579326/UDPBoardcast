

#include <stdint.h>
#include <queue>
#include "comm_core.h"
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









