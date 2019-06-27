

#include <vector>
#include "comm_define.h"
#include "async_res.h"

#define LOOP_ASYNC_RES_LIMIT 64


typedef struct{
    async_t element;
    bool used;
}async_element_t;

typedef struct _async_res_t {
    async_element_t async_list[LOOP_ASYNC_RES_LIMIT];
	int size;
	uv_mutex_t 	mutex;
	uv_sem_t 	sem;
}async_res_t;

async_res_t* init_async_res(uv_loop_t* loop, uint8_t size, uv_async_cb cb)
{
    async_res_t* res = new async_res_t;

    if ((int8_t)size > LOOP_ASYNC_RES_LIMIT)
    {
        res->size = LOOP_ASYNC_RES_LIMIT;
    }
    else
    {
        res->size = size;
    }

    uv_mutex_init(&res->mutex);
    uv_sem_init(&res->sem, res->size);

    for (int i = 0; i < res->size; i++)
    {
        uv_async_init(loop, &res->async_list[i].element.async, cb);
        uv_sem_init(&res->async_list[i].element.sem, 1);
        res->async_list[i].used = false;
    }

    return res;
}
async_t* pick_async_res(async_res_t* res)
{
    async_t* p = NULL;
    uv_sem_wait(&res->sem);

    uv_mutex_lock(&res->mutex);

    for (int i = 0; i < res->size; i++)
    {
        if (!res->async_list[i].used)
        {
            p = &res->async_list[i].element;
            res->async_list[i].used = true;
            break;
        }
    }
    uv_mutex_unlock(&res->mutex);

    return p;
}
int back_async_res(async_res_t* res, async_t* async)
{
    uv_mutex_lock(&res->mutex);
    for (int i = 0; i < res->size; i++)
    {
        if (async == &res->async_list[i].element)
        {
            res->async_list[i].used = false;
            uv_sem_post(&res->sem);
            break;
        }
    }
    uv_mutex_unlock(&res->mutex);

    return 0;
}
int clean_async_res(async_res_t* res)
{
    uv_mutex_lock(&res->mutex);
    for (int i = 0; i < res->size; i++)
    {
        uv_close((uv_handle_t*)&res->async_list[i].element.async, NULL);
        uv_sem_destroy(&res->async_list[i].element.sem);
    }
    uv_mutex_unlock(&res->mutex);

    uv_mutex_destroy(&res->mutex);
    uv_sem_destroy(&res->sem);

    delete res;

    return 0;
}

