/**
*   @file       cb_thread_define.h
*   @brief      cbb basic thread library define header file
*   @version    v1.0.0
*   @date       2017-09-13
*/
#ifndef _CB_THREAD_DEFINE_H_
#define _CB_THREAD_DEFINE_H_


#if defined(WIN32) || defined(_WIN32)
    #define CB_THREAD_OS_WIN
    #ifdef _WIN64
        #define CB_THREAD_OS_WIN64
    #else
        #define CB_THREAD_OS_WIN32
    #endif	//  !#ifdef _WIN64
#elif defined(__linux__) || defined(__linux)  
    #define CB_THREAD_OS_LINUX  
#elif defined(__APPLE__) && defined(__MACH__)
    #define CB_THREAD_OS_MAC  
#endif //   !#if defined(WIN32) || defined(_WIN32)

/** 用于等待对象或线程 立即返回  */
#define CB_THREAD_WAIT_IGNORE   0
/** 用于等待对象或线程 阻塞    */
#define CB_THREAD_WAIT_INFINITE 0xFFFFFFFF

#if defined (CB_THREAD_OS_WIN)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>

    typedef HANDLE              cb_thread_t;
    typedef HANDLE              cb_sem_t;
    typedef HANDLE              cb_event_t;
    typedef CRITICAL_SECTION    cb_cs_t;
    typedef HANDLE              cb_mutex_t;
    typedef unsigned int        cb_thread_id;
    typedef unsigned int        cb_thread_ret;

    #define CB_THREAD_CALL          __stdcall
    #define CB_THREAD_SLEEP_MS(x)	Sleep((x))

    #ifdef CB_THREAD_OS_WIN32
        typedef LONG            cb_normal_t;
        typedef volatile LONG   cb_volatile_t; 
    #else CB_THREAD_OS_WIN64 
        typedef LONG64          cb_normal_t;
        typedef volatile LONG64 cb_volatile_t;
    #endif //   !#ifdef CB_THREAD_OS_WIN32

#elif defined(CB_THREAD_OS_LINUX) || defined(CB_THREAD_OS_MAC)

    #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
    #endif

    #include <errno.h>
    #include <pthread.h>
    #include <sys/time.h>
    #include <time.h>
    #include <unistd.h>

    #ifdef CB_THREAD_OS_LINUX
        #include <semaphore.h>
        typedef sem_t       cb_sem_t;
    #else
        #include <mach/mach.h>
        #include <mach/clock.h>
        #include <mach/semaphore.h>
        typedef semaphore_t cb_sem_t;
    #endif //   !#ifdef CB_THREAD_OS_LINUX

    typedef pthread_t           cb_thread_t;
    typedef void*               cb_event_t;
    typedef pthread_mutex_t     cb_cs_t;
    typedef pthread_mutex_t     cb_mutex_t;
    typedef pthread_t           cb_thread_id;
    typedef void*               cb_thread_ret;

    #define CB_THREAD_CALL
    #define CB_THREAD_SLEEP_MS(x)   usleep((x)*1000)

    typedef long            cb_normal_t;
    typedef volatile long   cb_volatile_t;   
#else
    #error "This OS is unsupported"  
#endif //   !#if defined (CB_THREAD_OS_WIN)

/** 线程函数指针  */

typedef cb_thread_ret (CB_THREAD_CALL *cb_thread_proc) (void *arg);

/*!
*   @brief  错误码定义
*/

/** 成功  */
#define CB_THREAD_SUCCESS                  0
/** 一般错误    */
#define CB_THREAD_ERROR_NORMAL             0xCB020000
/** 参数无效    */
#define CB_THREAD_ERROR_INVALID_PARAM      0xCB020001
/** 内存分配失败  */
#define CB_THREAD_ERROR_MEM_ERROR          0xCB020002
/** 系统API错误，可使用相应平台错误码获取接口获取 如Windows[GetLastError] */
#define CB_THREAD_ERROR_SYS_API            0xCB021000

/** 等待超时    */
#define CB_THREAD_ERROR_WAIT_TIMEOUT       0xCB021001
/** 互斥对象已破坏 */
#define CB_THREAD_ERROR_WAIT_ABANDONED     0xCB021002
/** 等待失败    */
#define CB_THREAD_ERROR_WAIT_FAILED        0xCB021003
/** 资源繁忙    */
#define CB_THREAD_ERROR_WAIT_BUSY          0xCB021004
/** 系统等待API参数传入有误   */
#define CB_THREAD_ERROR_WAIT_PARAM_ERR     0xCB021005
/** 等待ID错误  */
#define CB_THREAD_ERROR_WAIT_ID_ERR        0xCB021006
/** 系统获取时间API错误 */
#define CB_THREAD_ERROR_GET_TIME_ERR       0xCB021007
/** 信号量资源耗尽 */
#define CB_THREAD_ERROR_SEM_IS_ZERO        0xCB021008
/** 句柄无效    */
#define CB_THREAD_ERROR_HANDLE_INVALID     0xCB021009



#endif  //   !_CB_THREAD_DEFINE_H_





