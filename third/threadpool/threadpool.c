
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "uv.h"
#include "threadpool.h"
#include "Gqueue.h"


typedef struct
{
    void*			data;
    work_cb			proc;
    after_work_cb	after_proc;
}thread_param_t;

typedef struct
{
    int				thread_status;		// 0 or 1
    uv_thread_t		thread;
    thread_param_t	param;
}thread_infor_t;


typedef struct _threadpool_t
{
    uv_sem_t        exit_sem;
    uv_mutex_t      mutex;
    uv_cond_t       cond;
    easy_queue_t*   queue;
    uv_thread_t*    threads;
    int             thread_num;
}threadpool_t;

//#define get_container_addr(element_ptr, type, element) (type*)(element_ptr - (&((type*)0)->element))

static void queue_free_func(void* data, unsigned int len)
{
	thread_param_t* param = (thread_param_t*)data;

	assert(len == sizeof(thread_param_t));

	param->after_proc(param->data, 1);
	return;
}


static void thread_func(void* param)
{
    int ret = 0;
    thread_param_t element;
	int element_len;
    threadpool_t* tp = (threadpool_t*)param;


    while (1)
    {
        if (uv_sem_trywait(&tp->exit_sem) == 0)
        {
            uv_sem_post(&tp->exit_sem);	// ┴г╦Э╝циб
            break;
        }

        uv_mutex_lock(&tp->mutex);
		
		element_len = sizeof(thread_param_t);
        if (-1 == easy_queue_pop(tp->queue, &element, &element_len))
        {
            // empty();
            uv_cond_wait(&tp->cond, &tp->mutex);
            uv_mutex_unlock(&tp->mutex);
            continue;
        }
        uv_mutex_unlock(&tp->mutex);

        if (element.proc)
        {
            element.proc(element.data);
        }
        if (element.after_proc)
        {
            element.after_proc(element.data, 1);
        }
    }


    return;
}

int threadpool_init(threadpool_t** tp, int threadnum)
{
    int i = 0;

    *tp = (threadpool_t*)malloc(sizeof(threadpool_t));
    (*tp)->threads = (uv_thread_t*)malloc(sizeof(uv_thread_t) * threadnum);
    memset((*tp)->threads, 0, sizeof(uv_thread_t) * threadnum);

    uv_sem_init(&(*tp)->exit_sem, 0);
    uv_mutex_init(&(*tp)->mutex);
    uv_cond_init(&(*tp)->cond);

    easy_queue_init(&(*tp)->queue, queue_free_func);

    (*tp)->thread_num = threadnum;

    for (; i < threadnum; i++)
    {
        uv_thread_create(&(*tp)->threads[i], thread_func, *tp);
    }

    return 0;
}


int threadpool_push_work(threadpool_t* tp, void* thread_param, work_cb work, after_work_cb after)
{
    int i = 0;
    thread_param_t param;

    uv_mutex_lock(&tp->mutex);
    if (easy_queue_size(tp->queue) > 64)
    {
        uv_mutex_unlock(&tp->mutex);
        return -1;
    }
	
    memset(&param, 0, sizeof(thread_param_t));
    param.proc = work;
    param.after_proc = after;
    param.data = thread_param;
    easy_queue_push(tp->queue, &param, sizeof(thread_param_t));
    uv_cond_broadcast(&tp->cond);

    uv_mutex_unlock(&tp->mutex);

    return 0;
}


static int _threadpool_cancel_all(threadpool_t* tp)
{
    int i = 0;
    thread_param_t outbuf;
	int outlen = sizeof(thread_param_t);
	
    uv_sem_post(&tp->exit_sem);

    uv_mutex_lock(&tp->mutex);
    uv_cond_broadcast(&tp->cond);

	// 清理任务队列
	easy_queue_clear(tp->queue);
	easy_queue_uninit(tp->queue);
	tp->queue = NULL;
	
    uv_mutex_unlock(&tp->mutex);

    for (; i < tp->thread_num; i++)
    {
        uv_thread_join(&tp->threads[i]);
    }

    return 0;
}


int threadpool_uninit(threadpool_t* tp)
{
    _threadpool_cancel_all(tp);

    uv_sem_destroy(&tp->exit_sem);
    uv_mutex_destroy(&tp->mutex);
    uv_cond_destroy(&tp->cond);

    free(tp->threads);
    free(tp);
    tp = NULL;

    return 0;
}
