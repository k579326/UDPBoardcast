
#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H



/*!
*   依赖libuv的跨平台线程池
*
*   @data 2018-04-12
*   @author guok
*/

#ifdef __cplusplus
extern "C" {
#endif

    typedef void(*work_cb)(void* param);
    typedef void(*after_work_cb)(void* param, int status);

    typedef struct _threadpool_t threadpool_t;

    int threadpool_init(threadpool_t** tp, int threadnum);

    int threadpool_push_work(threadpool_t* tp, void* thread_param, work_cb work, after_work_cb after);

    int threadpool_uninit(threadpool_t* tp);

#ifdef __cplusplus
}
#endif

#endif







