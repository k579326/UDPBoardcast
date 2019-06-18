/** 用于 _Interlocked_* 函数    */
#include <intrin.h>
#include <process.h>
#include "cb_thread.h"

#if defined(CB_THREAD_OS_WIN32)  

#define _atomic_inc(var)            InterlockedIncrement((long*)(var))
#define _atomic_dec(var)            InterlockedDecrement((long*)(var))
#define _atomic_add(var, val)       InterlockedExchangeAdd((long*)(var), (val))
#define _atomic_sub(var, val)       InterlockedExchangeAdd((long*)(var),-(val))
#define _atomic_set(var, val)       InterlockedExchange((long*)(var), (val))
#define _atomic_cas(var, cmp, val)  InterlockedCompareExchange((long*)(var), (val), (cmp))
#define _atomic_or(var, val)        _InterlockedOr((long*)(var), (val))
#define _atomic_xor(var, val)       _InterlockedXor((long*)(var), (val))
#define _atomic_and(var, val)       _InterlockedAnd((long*)(var), (val))

#else

#define _atomic_inc(var)            InterlockedIncrement64((long*)(var))
#define _atomic_dec(var)            InterlockedDecrement64((long*)(var))
#define _atomic_add(var, val)       InterlockedExchangeAdd64((long*)(var), (val))
#define _atomic_sub(var, val)       InterlockedExchangeAdd64((long*)(var),-(val))
#define _atomic_set(var, val)       InterlockedExchange64((long*)(var), (val))
#define _atomic_cas(var, cmp, val)  InterlockedCompareExchange64((long*)(var), (val), (cmp))
#define _atomic_or(var, val)        _InterlockedOr64((long*)(var), (val))
#define _atomic_xor(var, val)       _InterlockedXor64((long*)(var), (val))
#define _atomic_and(var, val)       _InterlockedAnd64((long*)(var), (val))

#endif //   !#if defined(CB_THREAD_OS_WIN32)

int cb_thread_create(
        cb_thread_t    *thread,
        cb_thread_proc thread_proc,
        void           *thread_arg,
        void           *thread_attr,
        unsigned int   thread_state,
        cb_thread_id   *thread_id)
{
    cb_thread_id* tid_tmp = NULL;

    if( !thread || !thread_proc )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if( thread_id )
    {
        tid_tmp = thread_id;
    }
    *thread = (cb_thread_t)_beginthreadex(
        thread_attr, 
        0, 
        thread_proc, 
        thread_arg, 
        thread_state, 
        (unsigned int *)tid_tmp
        );

    if (NULL == *thread)
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_thread_wait(cb_thread_t thread, unsigned int milliseconds, void **thread_ret)
{
    unsigned int ret = CB_THREAD_SUCCESS;
    unsigned int wait_ret = 0;
   
    if( !thread )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    wait_ret = WaitForSingleObject( thread, milliseconds);	
    switch( wait_ret )
    {
    case WAIT_OBJECT_0:
        ret = CB_THREAD_SUCCESS;
        break;
    case WAIT_ABANDONED:
        ret = CB_THREAD_ERROR_WAIT_ABANDONED;
     break;
    case WAIT_TIMEOUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    case WAIT_FAILED:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        if (ERROR_INVALID_HANDLE == GetLastError())
        {
            ret = CB_THREAD_ERROR_HANDLE_INVALID;
        }
        break;
    default:
        ret = CB_THREAD_ERROR_SYS_API;
        break;
    }
    return ret;
}

cb_thread_t cb_thread_get_handle()
{
    cb_thread_t _thread;

    if ( !DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &_thread, 0, FALSE, DUPLICATE_SAME_ACCESS) )
    {
        return NULL;
    }
    return _thread;
}

cb_thread_id cb_thread_get_id()
{
    return GetCurrentThreadId();
}

int cb_thread_close(cb_thread_t thread)
{
    if( !thread )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }

    if ( !CloseHandle(thread) )
    {
        if (ERROR_INVALID_HANDLE == GetLastError())
        {
            return CB_THREAD_ERROR_HANDLE_INVALID;
        }
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_cs_init(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    InitializeCriticalSection(cs);
    return CB_THREAD_SUCCESS;
}

int cb_cs_lock(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    EnterCriticalSection(cs);
    return CB_THREAD_SUCCESS;
}

int cb_cs_unlock(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    LeaveCriticalSection(cs);
    return CB_THREAD_SUCCESS;
}

int cb_cs_trylock(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    return ( TryEnterCriticalSection(cs) ? CB_THREAD_SUCCESS : CB_THREAD_ERROR_NORMAL );
}

int cb_cs_destroy(cb_cs_t *cs)
{
    if( !cs )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    DeleteCriticalSection(cs);
    return CB_THREAD_SUCCESS;
}

int cb_mutex_init(cb_mutex_t *mutex, unsigned int init_owner, const unsigned char *mutex_name)
{
    if( !mutex )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    *mutex = CreateMutex(NULL, init_owner, (LPCTSTR)mutex_name);
    if (NULL == *mutex)
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_mutex_lock(cb_mutex_t *mutex)
{
    if( !mutex )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }

    if ( WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0 )
    {
        return CB_THREAD_SUCCESS;
    }
    return CB_THREAD_ERROR_SYS_API;
}

int cb_mutex_unlock(cb_mutex_t *mutex)
{
    if( !mutex )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (!ReleaseMutex(*mutex))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_mutex_trylock(cb_mutex_t *mutex)
{
    if( !mutex )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if ( WaitForSingleObject(*mutex, IGNORE) == WAIT_OBJECT_0 )
    {
        return CB_THREAD_SUCCESS;
    }
    return CB_THREAD_ERROR_SYS_API;
}

int cb_mutex_destroy(cb_mutex_t *mutex)
{
    if( !mutex )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if ( !CloseHandle(*mutex) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

cb_event_t cb_event_init(int manual_reset, int initial_state, const unsigned char *event_name)
{
    HANDLE event_tem;
    event_tem = CreateEvent(NULL, manual_reset, initial_state, event_name);//(LPCWSTR)
    if (event_tem)
    {
        return event_tem;
    }
    return NULL;
}

int cb_event_set(cb_event_t event)
{
    if (NULL == event)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (!SetEvent(event))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_event_reset(cb_event_t event)
{
    if (NULL == event)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (!ResetEvent(event))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_event_destroy(cb_event_t event)
{
    if (NULL == event)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    if (!CloseHandle(event))
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_event_wait(cb_event_t event)
{
    int ret = CB_THREAD_SUCCESS;
    int wait_ret = CB_THREAD_SUCCESS;

    if (NULL == event)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    wait_ret = WaitForSingleObject(event, INFINITE);
    switch (wait_ret)
    {
    case WAIT_OBJECT_0:
        ret = CB_THREAD_SUCCESS;
        break;
    case WAIT_ABANDONED:
        ret = CB_THREAD_ERROR_WAIT_ABANDONED;
        break;
    case WAIT_TIMEOUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    case WAIT_FAILED:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        if (ERROR_INVALID_HANDLE == GetLastError())
        {
            ret = CB_THREAD_ERROR_HANDLE_INVALID;
        }
        break;
    default:
        ret = CB_THREAD_ERROR_SYS_API;
        break;
    }
    return ret;
}

int cb_event_wait_for(cb_event_t event, unsigned int milliseconds)
{
    int ret = CB_THREAD_SUCCESS;
    int wait_ret = CB_THREAD_SUCCESS;

    if (NULL == event)
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }
    wait_ret = WaitForSingleObject(event, milliseconds);
    switch (wait_ret)
    {
    case WAIT_OBJECT_0:
        ret = CB_THREAD_SUCCESS;
        break;
    case WAIT_ABANDONED:
        ret = CB_THREAD_ERROR_WAIT_ABANDONED;
        break;
    case WAIT_TIMEOUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    case WAIT_FAILED:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        if (ERROR_INVALID_HANDLE == GetLastError())
        {
            ret = CB_THREAD_ERROR_HANDLE_INVALID;
        }
        break;
    default:
        ret = CB_THREAD_ERROR_SYS_API;
        break;
    }
    return ret;
}

int cb_sem_init(cb_sem_t *sem, int init_value, unsigned int max_count)
{
    if( !sem )
    {
        return CB_THREAD_ERROR_INVALID_PARAM;
    }

    *sem = CreateSemaphore(NULL, init_value, max_count, NULL);	
    if ( NULL == *sem )
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
    if( !ReleaseSemaphore(*sem, 1, NULL) )
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
    unsigned int ret = 0;
    unsigned int wait_ret = 0;

    wait_ret = WaitForSingleObject( *sem, milliseconds);
    switch( wait_ret )
    {
    case WAIT_OBJECT_0:
        ret = CB_THREAD_SUCCESS;
        break;
    case WAIT_ABANDONED:
        ret = CB_THREAD_ERROR_WAIT_ABANDONED;
        break;
    case WAIT_TIMEOUT:
        ret = CB_THREAD_ERROR_WAIT_TIMEOUT;
        break;
    case WAIT_FAILED:
        ret = CB_THREAD_ERROR_WAIT_FAILED;
        break;
    default:
        ret = CB_THREAD_ERROR_SYS_API;
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
    if ( !CloseHandle(*sem) )
    {
        return CB_THREAD_ERROR_SYS_API;
    }
    return CB_THREAD_SUCCESS;
}

int cb_tick_count()
{
    return (int)GetTickCount();
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