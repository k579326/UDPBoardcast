
#include <mach/mach.h>
#include <mach/clock.h>
#include "cb_thread.h"

#define CB_THREAD_NULL_PTR ((void *)0)

#define _atomic_inc(var)            __sync_add_and_fetch((int*)(var), 1)
#define _atomic_dec(var)            __sync_sub_and_fetch((int*)(var), 1)
#define _atomic_add(var, val)       __sync_fetch_and_add((int*)(var), (val))
#define _atomic_sub(var, val)       __sync_fetch_and_sub((int*)(var), (val))
#define _atomic_set(var, val)       __sync_lock_test_and_set((int*)(var), (val))
#define _atomic_cas(var, cmp, val)  __sync_val_compare_and_swap((int*)(var), (cmp), (val))
#define _atomic_or(var, val)        __sync_fetch_and_or((int*)(var), (val))
#define _atomic_xor(var, val)       __sync_fetch_and_xor((int*)(var), (val))
#define _atomic_and(var, val)       __sync_fetch_and_and((int*)(var), (val))

/** cb_event_t 的内部实现定义    */
typedef struct cb_internal_event_t {
    int state;
    int manual_reset;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} cb_internal_event_t;

int cb_thread_create(
        cb_thread_t    *thread,
        cb_thread_proc thread_proc,
        void           *thread_arg,
        void           *thread_attr,
        unsigned int   thread_state,
        cb_thread_id   *thread_id)
{
    unsigned int ret = CB_THREAD_SUCCESS;

    if( !thread || !thread_proc )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    ret = pthread_create(thread, (pthread_attr_t*)thread_attr, thread_proc, thread_arg);
    if( ret || !(*thread) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    if( thread_id )
    {
        *thread_id = *thread;
    }
    return CB_THREAD_SUCCESS;
}

int cb_thread_wait(cb_thread_t thread, unsigned int milliseconds, void **thread_ret)
{
    unsigned int wait_ret = 0;
    unsigned int ret = CB_THREAD_SUCCESS;

    if( !thread )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    wait_ret = pthread_join(thread, CB_THREAD_NULL_PTR);
    switch (wait_ret)
    {
    case 0:
        ret = CB_THREAD_SUCCESS;
        break;
    case ETIMEDOUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    case EBUSY:
        ret = CB_THREAD_ERROR_WAIT_BUSY;
        break;
    case EINVAL:
        ret = CB_THREAD_ERROR_WAIT_PARAM_ERR;
        break;
    case EDEADLK:
        ret = CB_THREAD_ERROR_WAIT_ABANDONED;
        break;
    case ESRCH:
        ret = CB_THREAD_ERROR_WAIT_ID_ERR;
        break;
    default:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        break;
    }
    return ret;
}

cb_thread_t cb_thread_get_handle()
{
    return pthread_self();
}

cb_thread_id cb_thread_get_id()
{
    return pthread_self();
}

int cb_thread_close(cb_thread_t thread)
{
    //TODO  pthread_detach
    return CB_THREAD_SUCCESS;
}

int cb_cs_init(cb_cs_t *cs)
{
    pthread_mutexattr_t attr;
    unsigned int ret = CB_THREAD_SUCCESS;

    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (pthread_mutexattr_init(&attr))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    if (pthread_mutex_init(cs, &attr))
    {
        ret = CB_THREAD_ERROR_SYS_API;
    }
    pthread_mutexattr_destroy(&attr);

    return ret;
}

int cb_cs_lock(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if( pthread_mutex_lock(cs) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_cs_unlock(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if( pthread_mutex_unlock(cs) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_cs_trylock(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if( pthread_mutex_trylock(cs) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_cs_destroy(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if( pthread_mutex_destroy(cs) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_mutex_init(cb_mutex_t *mutex, unsigned int init_owner, const unsigned char *mutex_name)
{

    pthread_mutexattr_t attr;
    unsigned int ret = CB_THREAD_SUCCESS;

    if( !mutex )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (pthread_mutexattr_init(&attr))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    if (pthread_mutex_init(mutex, &attr))
    {
        ret = CB_THREAD_ERROR_SYS_API;
    }
    pthread_mutexattr_destroy(&attr);

    return ret;
}

int cb_mutex_lock(cb_mutex_t *mutex)
{
    return cb_cs_lock(mutex);
}

int cb_mutex_unlock(cb_mutex_t *mutex)
{
    return cb_cs_unlock(mutex);
}

int cb_mutex_trylock(cb_mutex_t *mutex)
{
    return cb_cs_trylock(mutex);
}

int cb_mutex_destroy(cb_mutex_t *mutex)
{
    return cb_cs_destroy(mutex);
}

cb_event_t cb_event_init(int manual_reset, int initial_state, const unsigned char *event_name)
{
    cb_internal_event_t *event_tem = (cb_internal_event_t *)malloc(sizeof(cb_internal_event_t));
    if (!event_tem)
    {
        return NULL;
    }
    memset(event_tem, 0, sizeof(cb_internal_event_t));
    event_tem->manual_reset = manual_reset;
    event_tem->state = initial_state;
    if (pthread_mutex_init(&event_tem->mutex, NULL))
    {
        free(event_tem);
        event_tem = NULL;
        return NULL;
    }
    if (pthread_cond_init(&event_tem->cond, NULL))
    {
        pthread_mutex_destroy(&event_tem->mutex);
        free(event_tem);
        event_tem = NULL;
        return NULL;
    }
    return (cb_event_t)event_tem;
}

int cb_event_set(cb_event_t event)
{
    cb_internal_event_t *event_tem = (cb_internal_event_t *)event;
    if (NULL == event_tem)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (pthread_mutex_lock(&event_tem->mutex) != 0)
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    event_tem->state = 1;
    if (event_tem->manual_reset)
    {
        if (pthread_cond_broadcast(&event_tem->cond))
        {
            return CB_THREAD_ERROR_SYS_API;
        }
    }
    else
    {
        if (pthread_cond_signal(&event_tem->cond))
        {
            return CB_THREAD_ERROR_SYS_API;
        }
    }
    if (pthread_mutex_unlock(&event_tem->mutex) != 0)
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_event_reset(cb_event_t event)
{
    cb_internal_event_t *event_tem = (cb_internal_event_t *)event;
    if (NULL == event_tem)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (pthread_mutex_lock(&event_tem->mutex) != 0)
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    event_tem->state = 0;
    if (pthread_mutex_unlock(&event_tem->mutex) != 0)
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_event_destroy(cb_event_t event)
{
    cb_internal_event_t *event_tem = (cb_internal_event_t *)event;
    if (NULL == event_tem)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    pthread_cond_destroy(&event_tem->cond);
    pthread_mutex_destroy(&event_tem->mutex);
    free(event_tem);
    event_tem = NULL;
    return CB_THREAD_SUCCESS;
}

int cb_event_wait(cb_event_t event)
{
    int ret = CB_THREAD_SUCCESS;
    int wait_ret = CB_THREAD_SUCCESS;

    cb_internal_event_t *event_tem = (cb_internal_event_t *)event;
    if (NULL == event_tem)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (pthread_mutex_lock(&event_tem->mutex))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    while (!event_tem->state)
    {
        wait_ret = pthread_cond_wait(&event_tem->cond, &event_tem->mutex);
        switch (wait_ret)
        {
        case 0:
            ret = CB_THREAD_SUCCESS;
            break;
        case ETIMEDOUT:
            ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
            break;
        case EINVAL:
            ret = CB_THREAD_ERROR_WAIT_PARAM_ERR;
            break;
        default:
            ret = CB_THREAD_ERROR_WAIT_FAILED;
            break;
        }
        pthread_mutex_unlock(&event_tem->mutex);
    }
    if (!event_tem->manual_reset)
    {
        event_tem->state = 0;
    }
    if (pthread_mutex_unlock(&event_tem->mutex))
    {
        ret = CB_THREAD_ERROR_SYS_API;
    }
    return ret;
}

int cb_event_wait_for(cb_event_t event, unsigned int milliseconds)
{
    int ret = CB_THREAD_SUCCESS;
    int wait_ret = CB_THREAD_SUCCESS;
   
    struct timespec ts;
    clock_serv_t servert_c;
    mach_timespec_t ts_c;
    

    cb_internal_event_t *event_tem = (cb_internal_event_t *)event;
    if (NULL == event_tem)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (pthread_mutex_lock(&event_tem->mutex))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    if (milliseconds)
    {
        host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &servert_c);
        clock_get_time(servert_c, &ts_c);
        ts.tv_sec = ts_c.tv_sec + milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000 + ts_c.tv_nsec;
        if (ts.tv_nsec >= 1000000000)
        {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }
        wait_ret = pthread_cond_timedwait(&event_tem->cond, &event_tem->mutex, &ts);
    }
    else
    {
        host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &servert_c);
        clock_get_time(servert_c, &ts_c);
        ts.tv_sec = ts_c.tv_sec;
        ts.tv_nsec = ts_c.tv_nsec;
        wait_ret = pthread_cond_timedwait(&event_tem->cond, &event_tem->mutex, &ts);
    }

    switch (wait_ret)
    {
    case 0:
        ret = CB_THREAD_SUCCESS;
        break;
    case ETIMEDOUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    case EINVAL:
        ret = CB_THREAD_ERROR_WAIT_PARAM_ERR;
        break;
    default:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        break;
    }
    if (!event_tem->manual_reset)
    {
        event_tem->state = 0;
    }
    pthread_mutex_unlock(&event_tem->mutex);
    return ret;
}

int cb_sem_init(cb_sem_t *sem, int init_value, unsigned int max_count)
{
    if( !sem )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    
    if ( semaphore_create(mach_task_self(), sem, SYNC_POLICY_FIFO,init_value) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_sem_release(cb_sem_t *sem)
{
    if( !sem )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if( semaphore_signal(*sem) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_sem_wait(cb_sem_t *sem)
{
    return cb_sem_wait_for(sem, CB_THREAD_WAIT_INFINITE);
}

int cb_sem_trywait(cb_sem_t *sem)
{
    return cb_sem_wait_for(sem, CB_THREAD_WAIT_IGNORE);
}

int cb_sem_wait_for(cb_sem_t *sem, unsigned int milliseconds)
{
    clock_serv_t servert_c;
    mach_timespec_t interval;
    unsigned int ret = 0;
    unsigned int wait_ret = 0;

    if( !sem )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
//TODO
//#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)

    if ( CB_THREAD_WAIT_IGNORE == milliseconds )
    {
        interval.tv_sec = 0;
        interval.tv_nsec = 0;
        wait_ret = semaphore_timedwait(*sem, interval);
     }
    else if ( CB_THREAD_WAIT_INFINITE == milliseconds )
    {
        do
        {
           wait_ret = semaphore_wait(*sem);
        }
        while (wait_ret == KERN_ABORTED);
    }
    else
    {
        host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &servert_c);
        clock_get_time(servert_c, &interval);

        interval.tv_sec = interval.tv_sec + milliseconds / 1000;
        interval.tv_nsec = (milliseconds % 1000) * 1000000 + interval.tv_nsec;
        if (interval.tv_nsec >= 1000000000)
        {
            interval.tv_sec += 1;
            interval.tv_nsec -= 1000000000;
        }
        do
        {
            wait_ret = semaphore_timedwait( *sem, interval );
        } while (wait_ret == KERN_ABORTED);
    }
//#else
    
  //  CB_thread_sleep(1000);
   // interval.tv_sec = 0;
   // interval.tv_nsec = 0;
   // wait_ret = semaphore_timedwait(*sem, interval);

//
//#endif // #if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)

    switch (wait_ret)
    {
    case KERN_SUCCESS:
        ret = CB_THREAD_SUCCESS;
        break;
    case KERN_OPERATION_TIMED_OUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    default:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        break;
    }
    return ret;
}
int cb_sem_destroy(cb_sem_t *sem)
{
    if( !sem )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if ( semaphore_destroy(mach_task_self(), *sem) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_tick_count()
{
    clock_serv_t servert_c;
    mach_timespec_t ts_c;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &servert_c);
    clock_get_time(servert_c, &ts_c);

    return (ts_c.tv_sec * 1000 + ts_c.tv_nsec / 1000000);
}

int cb_atomic_inc(cb_volatile_t *v)
{
    return _atomic_inc(v);
}
int cb_atomic_dec(cb_volatile_t *v)
{
    return _atomic_dec(v);
}
int cb_atomic_add(cb_volatile_t *v, cb_normal_t n)
{
    return _atomic_add(v, n);
}
int cb_atomic_sub(cb_volatile_t *v, cb_normal_t n)
{
    return _atomic_sub(v, n);
}
int cb_atomic_set(cb_volatile_t *v, cb_normal_t n)
{
    return _atomic_set(v, n);
}
int cb_atomic_cas(cb_volatile_t *v, cb_normal_t n, cb_normal_t _compare)
{
    return _atomic_cas(v, _compare, n);
}
int cb_atomic_or (cb_volatile_t *v, cb_normal_t n)
{
    return _atomic_or(v, n);
}
int cb_atomic_xor(cb_volatile_t *v, cb_normal_t n)
{
    return _atomic_xor(v, n);
}
int cb_atomic_and(cb_volatile_t *v, cb_normal_t n)
{
    return _atomic_and(v, n);
}
