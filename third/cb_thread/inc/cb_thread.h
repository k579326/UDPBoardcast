/**
*   @file       cb_thread.h
*   @version    v1.0.0
*   @date       2017-9-13
*/
#ifndef _CB_THREAD_H_
#define _CB_THREAD_H_

/** @defgroup cbb_basic CBB基础库
*  http://10.10.1.14/cbb/basic
*  @{
*/
/** @defgroup cb_thread 线程库
*  @ingroup cbb_basic
*  http://10.10.1.14/cbb/basic/cb_thread
*/
/** @} */ /* end @defgroup cbb_basic CBB基础库 */

#include "cb_thread_define.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
*   @brief      获取系统启动到现在的毫秒数
*   @return     成功返回毫秒数，失败返回 0
*   @ingroup    cb_thread
*/
int cb_tick_count();

/*!
*   @brief                      创建线程
*   @param[out] thread          接收线程句柄
*   @param[in]  thread_proc     线程函数指针
*   @param[in]  thread_arg      线程函数参数
*   @param[in]  thread_attr     线程属性
*   @param[in]  thread_state    线程状态
*   @param[out] thread_id       接收线程ID
*   @return                     成功返回 CB_THREAD_SUCCESS，其它错误见 cb_thread_define.h 中，宏 CB_THREAD_ERROR_*
*   @remarks                    thread_state 暂支持Windows平台。
*   @ingroup    cb_thread
*/
int cb_thread_create(
                    cb_thread_t    *thread,
                    cb_thread_proc thread_proc,
                    void           *thread_arg,
                    void           *thread_attr,
                    unsigned int   thread_state,
                    cb_thread_id   *thread_id);


/*!
*   @brief                      等待线程
*   @param[in]  thread          线程句柄
*   @param[in]  milliseconds    CB_THREAD_WAIT_IGNORE 立即返回，CB_THREAD_WAIT_INFINITE 阻塞，其它值代表时间间隔[毫秒]
*   @param[out] thread_ret      线程返回值
*   @return                     成功返回 CB_THREAD_SUCCESS，其它错误见 cb_thread_define.h中，宏 CB_THREAD_ERROR_*
*   @remarks                    thread_ret 参数暂不支持,Mac平台不支持 milliseconds 参数,只会阻塞等待
*   @ingroup    cb_thread
*/
int cb_thread_wait(cb_thread_t thread, unsigned int milliseconds,  void **thread_ret);

/*!
*   @brief      获取当前线程句柄
*   @return     成功返回当前句柄(非0值)
*   @ingroup    cb_thread
*/
cb_thread_t cb_thread_get_handle();

/*!
*   @brief      获取当前线程ID
*   @return     成功返回当前线程ID(非0值)
*   @ingroup    cb_thread
*/
cb_thread_id cb_thread_get_id();

/*!
*   @brief                  关闭线程句柄
*   @param[in]  thread      线程句柄
*   @return                 成功返回 CB_THREAD_SUCCESS，其它错误见 cb_thread_define.h中，宏 CB_THREAD_ERROR_*
*   @ingroup    cb_thread
*/
int cb_thread_close(cb_thread_t thread);

/*!
*   @brief      初始化临界区[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_cs_init (cb_cs_t *cs);

/*!
*   @brief      锁定临界区[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_cs_lock (cb_cs_t *cs);

/*!
*   @brief      解锁临界区[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_cs_unlock (cb_cs_t *cs);

/*!
*   @brief      尝试锁定临界区[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_cs_trylock (cb_cs_t *cs);

/*!
*   @brief      销毁临界区[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_cs_destroy (cb_cs_t *cs);

/*!
*   @brief      初始化互斥量
*   @return     成功返回 CB_THREAD_SUCCESS，其它错误见 cb_thread_define.h中，宏 CB_THREAD_ERROR_*
*   @remarks    mutex_name 参数限于Windows平台。
*   @ingroup    cb_thread
*/
int cb_mutex_init       (cb_mutex_t *mutex, unsigned int init_owner, const unsigned char *mutex_name);

/*!
*   @brief      锁定互斥量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_mutex_lock       (cb_mutex_t *mutex);

/*!
*   @brief      解锁互斥量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_mutex_unlock     (cb_mutex_t *mutex);

/*!
*   @brief      尝试锁定互斥量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_mutex_trylock    (cb_mutex_t *mutex);

/*!
*   @brief      销毁互斥量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_mutex_destroy    (cb_mutex_t *mutex);

/*!
*   @brief  事件操作函数(初始化、设置、重置、销毁、等待、等待指定时间[毫秒]、销毁)
*   @return 成功返回 CB_THREAD_SUCCESS，其它错误见 cb_thread_define.h中，宏 CB_THREAD_ERROR_*
*   @ingroup    cb_thread
*/
/*!
*   @brief      事件初始化
*   @ingroup    cb_thread
*/
cb_event_t cb_event_init (int manual_reset, int initial_state, const unsigned char *event_name);

/*!
*   @brief      设置事件[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_event_set (cb_event_t event);

/*!
*   @brief      重置事件[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_event_reset (cb_event_t event);

/*!
*   @brief      销毁事件[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_event_destroy (cb_event_t event);

/*!
*   @brief      等待事件[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_event_wait (cb_event_t event);

/*!
*   @brief      等待事件指定时间[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_event_wait_for (cb_event_t event, unsigned int milliseconds);

/*!
*   @brief      初始化信号量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_sem_init (cb_sem_t *sem, int init_value, unsigned int max_count);

/*!
*   @brief      释放化信号量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_sem_release (cb_sem_t *sem);

/*!
*   @brief      等待信号量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_sem_wait (cb_sem_t *sem);

/*!
*   @brief      尝试等待信号量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_sem_trywait (cb_sem_t *sem);

/*!
*   @brief                      等待信号量指定时间[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @param[in]  sem             信号量句柄
*   @param[in]  milliseconds    CB_THREAD_WAIT_IGNORE 立即返回，CB_THREAD_WAIT_INFINITE 阻塞，其它值代表等待时间[毫秒]
*   @ingroup    cb_thread
*/
int cb_sem_wait_for (cb_sem_t *sem, unsigned int milliseconds);

/*!
*   @brief      销毁信号量[成功返回 CB_THREAD_SUCCESS，其它错误见宏 CB_THREAD_ERROR_*]
*   @ingroup    cb_thread
*/
int cb_sem_destroy (cb_sem_t *sem);

/*!
*   @brief      原子操作 执行 *v + 1，返回 *v + 1
*   @ingroup    cb_thread
*/
int cb_atomic_inc(cb_volatile_t *v);

/*!
*   @brief      原子操作 执行 *v - 1，返回 *v - 1
*   @ingroup    cb_thread
*/
int cb_atomic_dec(cb_volatile_t *v);

/*!
*   @brief      原子操作 执行 *v + n，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_add(cb_volatile_t *v, cb_normal_t n);

/*!
*   @brief      原子操作 执行 *v - n，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_sub(cb_volatile_t *v, cb_normal_t n);

/*!
*   @brief      原子操作 执行 设置 n 到 *v，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_set(cb_volatile_t *v, cb_normal_t n);

/*!
*   @brief      原子操作 执行 设置 n 到 *v，如 *v 等于_compare，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_cas(cb_volatile_t *v, cb_normal_t n, cb_normal_t compare);

/*!
*   @brief      原子操作 执行 *v | n，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_or (cb_volatile_t *v, cb_normal_t n);

/*!
*   @brief      原子操作 执行 *v ^ n，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_xor(cb_volatile_t *v, cb_normal_t n);

/*!
*   @brief      原子操作 执行 *v & n，返回 *v
*   @ingroup    cb_thread
*/
int cb_atomic_and(cb_volatile_t *v, cb_normal_t n);


#ifdef __cplusplus
}
#endif

#endif /* !_CB_THREAD_H_ */

