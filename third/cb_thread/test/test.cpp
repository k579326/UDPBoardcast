#include <stdio.h>
#include "cb_thread.h"
#include "gtest/gtest.h"

cb_thread_ret CB_THREAD_CALL _thread_proc_state(void * arg)
{
    CB_THREAD_SLEEP_MS(500);
    printf("thread id = %u\n", cb_thread_get_id());
    printf("thread id = %u\n", cb_thread_get_handle());

    return 0;
}

TEST(cb_thread, thread_time)
{
    EXPECT_EQ(!cb_tick_count(), 0);
}

TEST(cb_thread, thread_state)
{
	
	int             wait_short_time = 100;
	cb_thread_t     err_invalid_t = (cb_thread_t)12345;
	cb_thread_t     err_null_t = (cb_thread_t)NULL;

    cb_thread_t     _thread;
	cb_thread_id    _thread_id;

#ifdef CB_THREAD_OS_WIN
	SECURITY_ATTRIBUTES                 _thread_attr;
	_thread_attr.bInheritHandle         = 0;
	_thread_attr.nLength                = 0;
    _thread_attr.lpSecurityDescriptor   = "";
    EXPECT_EQ(cb_thread_create(&_thread, _thread_proc_state, NULL, &_thread_attr, NULL, &_thread_id),CB_THREAD_ERROR_SYS_API);
#else
    pthread_attr_t  _thread_attr;
    if (pthread_attr_init(&_thread_attr))
    {
    	printf("pthread_attr_init error.\n");
    }
    EXPECT_EQ(cb_thread_create(&_thread, _thread_proc_state, NULL, &_thread_attr, NULL, &_thread_id),CB_THREAD_SUCCESS);
    EXPECT_EQ(cb_thread_wait(_thread, CB_THREAD_WAIT_INFINITE, NULL), CB_THREAD_SUCCESS);
    EXPECT_EQ(cb_thread_close(_thread), CB_THREAD_SUCCESS);
#endif

    EXPECT_EQ(cb_thread_create(NULL, _thread_proc_state, NULL, NULL, NULL, &_thread_id), CB_THREAD_ERROR_INVALID_PARAM);
    EXPECT_EQ(cb_thread_create(&_thread, NULL, NULL, NULL, NULL, &_thread_id), CB_THREAD_ERROR_INVALID_PARAM);
    EXPECT_EQ(cb_thread_create(NULL, NULL, NULL, NULL, NULL, &_thread_id), CB_THREAD_ERROR_INVALID_PARAM);

	EXPECT_EQ(cb_thread_create(&_thread, _thread_proc_state, NULL, NULL, NULL, &_thread_id), CB_THREAD_SUCCESS);
    EXPECT_EQ(cb_thread_wait(_thread, CB_THREAD_WAIT_INFINITE, NULL), CB_THREAD_SUCCESS);
    EXPECT_EQ(cb_thread_close(_thread), CB_THREAD_SUCCESS);

	EXPECT_EQ(cb_thread_create(&_thread, _thread_proc_state, NULL, NULL, NULL, NULL), CB_THREAD_SUCCESS);

#ifdef CB_THREAD_OS_WIN
    EXPECT_EQ(cb_thread_wait(_thread, CB_THREAD_WAIT_IGNORE, NULL), CB_THREAD_ERROR_WAIT_TIMEOUT);
#else
    EXPECT_EQ(cb_thread_wait(_thread, CB_THREAD_WAIT_IGNORE, NULL), CB_THREAD_ERROR_WAIT_BUSY);
#endif   

	EXPECT_EQ(cb_thread_wait(_thread, wait_short_time, NULL), CB_THREAD_ERROR_WAIT_TIMEOUT);
    EXPECT_EQ(cb_thread_wait(_thread, CB_THREAD_WAIT_INFINITE, NULL), CB_THREAD_SUCCESS);
    EXPECT_EQ(cb_thread_close(_thread), CB_THREAD_SUCCESS);


	EXPECT_EQ(cb_thread_wait(err_null_t, CB_THREAD_WAIT_INFINITE, NULL), CB_THREAD_ERROR_INVALID_PARAM);

#ifdef CB_THREAD_OS_WIN
    EXPECT_EQ(cb_thread_wait(err_invalid_t, CB_THREAD_WAIT_INFINITE, NULL), CB_THREAD_ERROR_HANDLE_INVALID);
    EXPECT_EQ(cb_thread_close(err_invalid_t), CB_THREAD_ERROR_HANDLE_INVALID);
    EXPECT_EQ(cb_thread_close(err_null_t), CB_THREAD_ERROR_INVALID_PARAM);
#endif
}
cb_thread_ret CB_THREAD_CALL _thread_proc_cs(void* arg)
{
	int ret = CB_THREAD_SUCCESS;
	cb_thread_id _thread_id;
	cb_cs_t *tem_cs_t = (cb_cs_t *)arg;

	_thread_id = cb_thread_get_id();

	EXPECT_EQ(cb_cs_lock(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_cs_trylock(NULL), CB_THREAD_ERROR_INVALID_PARAM);

	ret = cb_cs_trylock(tem_cs_t);
	if (CB_THREAD_SUCCESS != ret) 
	{
		EXPECT_EQ(cb_cs_lock(tem_cs_t), CB_THREAD_SUCCESS);
		/*in one thread can lock more same to win-cs but no use mostly*/
		EXPECT_EQ(cb_cs_lock(tem_cs_t), CB_THREAD_SUCCESS);
		EXPECT_EQ(cb_cs_lock(tem_cs_t), CB_THREAD_SUCCESS);
		EXPECT_EQ(cb_cs_unlock(tem_cs_t), CB_THREAD_SUCCESS);
		EXPECT_EQ(cb_cs_unlock(tem_cs_t), CB_THREAD_SUCCESS);
	}
	printf("thread id : %u running\n", _thread_id);
	//do something
	CB_THREAD_SLEEP_MS(1000);
	EXPECT_EQ(cb_cs_unlock(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_cs_unlock(tem_cs_t), CB_THREAD_SUCCESS);
	printf("thread id : %u end\n", _thread_id);
	return 0;
}

TEST(cb_thread, thread_cs)
{
	unsigned int i = 0;
	unsigned int cnt = 3;
	cb_cs_t cs;

	cb_thread_t _thread[3] = { 0 };
	EXPECT_EQ(cb_cs_init(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_cs_init(&cs), CB_THREAD_SUCCESS);

	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_cs, &cs, 0, 0, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
	}
	EXPECT_EQ(cb_cs_destroy(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_cs_destroy(&cs), CB_THREAD_SUCCESS);
}

cb_thread_ret CB_THREAD_CALL _thread_proc_mutex(void* arg)
{
	int ret = CB_THREAD_SUCCESS;
	cb_thread_id _thread_id;
	cb_mutex_t *tem_mutex_t = (cb_mutex_t *)arg;

	_thread_id = cb_thread_get_id();

	EXPECT_EQ(cb_mutex_lock(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_mutex_trylock(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	
	ret = cb_mutex_trylock(tem_mutex_t);
	if (CB_THREAD_SUCCESS != ret)
	{
		cb_mutex_lock(tem_mutex_t);
	}
	printf("thread id : %u running\n", _thread_id);
	//do something
	CB_THREAD_SLEEP_MS(1000);
	printf("thread id : %u end\n", _thread_id);

	EXPECT_EQ(cb_mutex_unlock(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_mutex_unlock(tem_mutex_t), CB_THREAD_SUCCESS);
	return 0;
}
TEST(cb_thread, thread_mutex)
{
	unsigned int i = 0;
	unsigned int cnt = 3;
	cb_mutex_t _mutex;
	cb_mutex_t error_mutex;

	cb_thread_t _thread[3] = { 0 };

	EXPECT_EQ(cb_mutex_init(NULL, 0, NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_mutex_init(&_mutex, 0, NULL), CB_THREAD_SUCCESS);

	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_mutex, &_mutex, 0, 0, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
	}
#ifdef CB_THREAD_OS_WIN
	error_mutex = (cb_mutex_t)123456;
	EXPECT_EQ(cb_mutex_destroy(&error_mutex), CB_THREAD_ERROR_SYS_API);
#endif
	EXPECT_EQ(cb_mutex_destroy(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_mutex_destroy(&_mutex), CB_THREAD_SUCCESS);
}

int g_event_value = 0;
cb_thread_ret CB_THREAD_CALL _thread_proc_event(void* arg)
{
    struct _thread_arg
    {
        cb_event_t _event;
        int index;
    };

    cb_thread_id thread_id;
    int wait_ret = 0;
    _thread_arg* _event_st = (_thread_arg *)arg;
    thread_id = cb_thread_get_id();

    printf("thread id : %u waiting for write event\n", thread_id);


    EXPECT_EQ(cb_event_wait(NULL), CB_THREAD_ERROR_INVALID_PARAM);

    if (_event_st->index == 0)
    {
        //timeout
        wait_ret = cb_event_wait_for(_event_st->_event, 500);
    }
    else if (_event_st->index == 1)
    {
        //not timeout
        wait_ret = cb_event_wait_for(_event_st->_event, 1000);
    }
    else
    {
        //not timeout
        wait_ret = cb_event_wait(_event_st->_event);
    }
    switch (wait_ret)
    {
    case CB_THREAD_SUCCESS:
        printf("thread id %u reading from buffer : %d", thread_id, g_event_value);
        break;
    case CB_THREAD_ERROR_WAIT_TIMEOUT:
        printf("thread id  %u wait timeout...", thread_id);
        break;
    default:
        printf("thread id %u wait failed ", thread_id);
        break;
    }
    printf("	exiting\n");
    return NULL;
}
void writetobuffer(cb_event_t event)
{
    CB_THREAD_SLEEP_MS(1000);
    while (g_event_value < 3) {
        g_event_value++;
        printf("main thread writing to the shared buffer..%d\n", g_event_value);
        CB_THREAD_SLEEP_MS(100);
    }
    // set event to signaled
    EXPECT_EQ(cb_event_set(NULL), CB_THREAD_ERROR_INVALID_PARAM);
    EXPECT_EQ(cb_event_set(event), CB_THREAD_SUCCESS);
}
TEST(cb_thread, thread_event)
{
    struct _thread_arg
    {
        cb_event_t _event;
        int index;
    };
    unsigned int i = 0;
    unsigned int cnt = 3;
    cb_thread_t _thread[3];
    cb_event_t event_tem;
    _thread_arg _event_st;

    //manual - reset event. start with no signal
    //3 threads waiting for main thread set event signal.then read value of gSharedValue
    event_tem = cb_event_init(1, 0, 0);
    EXPECT_EQ(!event_tem, 0);

    
    
    _event_st._event = event_tem;
    _event_st.index = 0;

    for (i = 0; i < cnt; i++)
    {
        EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_event, &_event_st, 0, 0, 0), CB_THREAD_SUCCESS);
    }
    writetobuffer(event_tem);
    for (i = 0; i < cnt; i++)
    {
        EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
    }
    for (i = 0; i < cnt; i++)
    {
        EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
    }
    printf("All threads ended, cleaning up ...\n");

    _event_st._event = event_tem;
    _event_st.index = 1;

    for (i = 0; i < cnt; i++)
    {
        EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_event, &_event_st, 0, 0, 0), CB_THREAD_SUCCESS);
    }
    writetobuffer(event_tem);
    for (i = 0; i < cnt; i++)
    {
        EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
    }
    for (i = 0; i < cnt; i++)
    {
        EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
    }
    printf("All threads ended, cleaning up ...\n");

    g_event_value = 0;

    _event_st._event = event_tem;
    _event_st.index = 2;

    EXPECT_EQ(cb_event_reset(NULL), CB_THREAD_ERROR_INVALID_PARAM);
    EXPECT_EQ(cb_event_reset(event_tem), CB_THREAD_SUCCESS);

    EXPECT_EQ(cb_thread_create(&_thread[0], _thread_proc_event, &_event_st, 0, 0, 0), CB_THREAD_SUCCESS);
    writetobuffer(event_tem);

    EXPECT_EQ(cb_thread_wait(_thread[0], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
    EXPECT_EQ(cb_thread_close(_thread[0]), CB_THREAD_SUCCESS);


    EXPECT_EQ(cb_event_destroy(NULL), CB_THREAD_ERROR_INVALID_PARAM);
    EXPECT_EQ(cb_event_destroy(event_tem), CB_THREAD_SUCCESS);

    printf("reset again test. thread ended, cleaning up for application exit...\n");
    return;
}


cb_thread_ret CB_THREAD_CALL _thread_proc_sem(void* arg)
{

	struct _thread_arg
	{
		cb_sem_t *_sem;
		int index;
	};
	unsigned int wait_ret = 0;
	unsigned int _continue = 1;
	cb_thread_id _thread_id;
	cb_sem_t *_sem_error = (cb_sem_t *)123456;
	_thread_arg *_sem_st = ((_thread_arg *)arg);
	while (_continue)
	{
#ifdef CB_THREAD_OS_WIN
		EXPECT_EQ(cb_sem_wait_for(_sem_error, 1000), CB_THREAD_ERROR_WAIT_FAILED);
#endif
		if (_sem_st->index == 1)
		{
			wait_ret = cb_sem_wait_for(_sem_st->_sem, 3000);
		}
		if (_sem_st->index == 2)
		{
			wait_ret = cb_sem_wait_for(_sem_st->_sem, 2000);
		}
		if (_sem_st->index == 3)
		{
			wait_ret = cb_sem_trywait(_sem_st->_sem);
			if (wait_ret == CB_THREAD_ERROR_WAIT_TIMEOUT)
			{
				wait_ret = cb_sem_wait(_sem_st->_sem);
			}
		}
		_thread_id = cb_thread_get_id();
		switch (wait_ret)
		{
		case CB_THREAD_SUCCESS:
			printf("thread id : %u wait success\n", _thread_id);
			CB_THREAD_SLEEP_MS(2000);
			EXPECT_EQ(cb_sem_release(_sem_st->_sem), CB_THREAD_SUCCESS);
			EXPECT_EQ(cb_sem_release(NULL), CB_THREAD_ERROR_INVALID_PARAM);
			_continue = 0;
			break;
		case CB_THREAD_ERROR_WAIT_TIMEOUT:
			printf("thread id : %u wait timeout\n", _thread_id);
			break;
		case CB_THREAD_ERROR_WAIT_BUSY:
			printf("thread id : %u wait busy\n", _thread_id);
			break;
		default:
			printf("thread id : %u wait error 0x%08x\n", _thread_id, wait_ret);
			break;
		}
	}
	return 0;
}
TEST(cb_thread, thread_sem)
{
	struct _thread_arg
	{
		cb_sem_t *_sem;
		int index;
	};
	_thread_arg test_wait;

	unsigned int i = 0;
	unsigned int cnt = 2;

	cb_sem_t _sem;
	cb_sem_t _sem_error;

	cb_thread_t _thread[3] = { 0 };

	EXPECT_EQ(cb_sem_init(NULL, 0, NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_sem_init(&_sem, -1, 0), CB_THREAD_ERROR_SYS_API);
	EXPECT_EQ(cb_sem_init(&_sem, 1, 1), CB_THREAD_SUCCESS);
    
	test_wait._sem = &_sem;
	test_wait.index = 1;

	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_sem, &test_wait, 0, 0, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
	}
#ifdef CB_THREAD_OS_WIN
	_sem_error = (cb_sem_t)123456;
	EXPECT_EQ(cb_sem_destroy(&_sem_error), CB_THREAD_ERROR_SYS_API);
#endif
	EXPECT_EQ(cb_sem_destroy(NULL), CB_THREAD_ERROR_INVALID_PARAM);
	EXPECT_EQ(cb_sem_destroy(&_sem), CB_THREAD_SUCCESS);

	test_wait._sem = &_sem;
	test_wait.index = 2;

	EXPECT_EQ(cb_sem_init(&_sem, 1, 1), CB_THREAD_SUCCESS);
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_sem, &test_wait, 0, 0, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
	}
	EXPECT_EQ(cb_sem_destroy(&_sem), CB_THREAD_SUCCESS);

	test_wait._sem = &_sem;
	test_wait.index = 3;

	EXPECT_EQ(cb_sem_init(&_sem, 1, 1), CB_THREAD_SUCCESS);
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_sem, &test_wait, 0, 0, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i < cnt; i++)
	{
		EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
	}
	EXPECT_EQ(cb_sem_destroy(&_sem), CB_THREAD_SUCCESS);


	return;
}


cb_volatile_t g_total = 0;
#define RUN_COUNT 1000000
cb_thread_ret CB_THREAD_CALL _thread_proc_atomic(void* arg)
{
	unsigned int index = 0;
	for (index = 0; index < RUN_COUNT; index++) 
	{
		cb_atomic_inc(&g_total);
		//g_Total++; //may be error
	}
	return NULL;
}
TEST(cb_thread, thread_atomic)
{
	unsigned int i = 0;
	cb_thread_t _thread[3];

	EXPECT_EQ(cb_atomic_inc(&g_total), 1);
	EXPECT_EQ(g_total, 1);
	EXPECT_EQ(cb_atomic_dec(&g_total), 0);
	EXPECT_EQ(g_total, 0);
	EXPECT_EQ(cb_atomic_add(&g_total, 100), 0);
	EXPECT_EQ(g_total, 100);
	EXPECT_EQ(cb_atomic_sub(&g_total, 99), 100);
	EXPECT_EQ(g_total, 1);
	EXPECT_EQ(cb_atomic_cas(&g_total, 100, 1), 1);
	EXPECT_EQ(g_total, 100);
	EXPECT_EQ(cb_atomic_cas(&g_total, 1, 200), 100);
	EXPECT_EQ(g_total, 100);
	EXPECT_EQ(cb_atomic_set(&g_total, 1), 100);
	EXPECT_EQ(g_total, 1);
	EXPECT_EQ(cb_atomic_and(&g_total, 0), 1);
	EXPECT_EQ(g_total, 0);
	EXPECT_EQ(cb_atomic_or(&g_total, 1), 0);
	EXPECT_EQ(g_total, 1);
	EXPECT_EQ(cb_atomic_set(&g_total, 10101), 1);
	EXPECT_EQ(g_total, 10101);
	EXPECT_EQ(cb_atomic_xor(&g_total, 110), 10101);
	EXPECT_EQ(g_total, 10011);
	EXPECT_EQ(cb_atomic_set(&g_total, 0), 10011);
	EXPECT_EQ(g_total, 0);
	//simple data type access with atomic variables
	for (i = 0; i<3; i++) 
	{
		EXPECT_EQ(cb_thread_create(&_thread[i], _thread_proc_atomic, 0, 0, 0, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i<3; i++) 
	{
		EXPECT_EQ(cb_thread_wait(_thread[i], CB_THREAD_WAIT_INFINITE, 0), CB_THREAD_SUCCESS);
	}
	for (i = 0; i <3; i++)
	{
		EXPECT_EQ(cb_thread_close(_thread[i]), CB_THREAD_SUCCESS);
	}
	EXPECT_EQ(g_total,3000000);
	return ;
}