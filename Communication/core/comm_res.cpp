

#include "comm_res.h"
#include "comm_core.h"
#include "select_network.h"
#include "ssnet_err.h"


static client_loop_t g_ClientLoop;
static server_loop_t g_serverLoop;


static void _noIdle(client_loop_t* cl)
{
    uv_mutex_lock(&cl->condlock);
    uv_cond_wait(&cl->cond, &cl->condlock);
    uv_mutex_unlock(&cl->condlock);
}

static void _clientloop_process(void* param)
{
    client_loop_t* cl = (client_loop_t*)param;
    while (1) {

        if (cl->connTable.table.empty()){
            //_noIdle(cl);
        }
        uv_run(&cl->loop_info.loop, UV_RUN_DEFAULT);
    }
}

static void _serverloop_process(void* param)
{
    server_loop_t* cl = (server_loop_t*)param;
    
    while (1)
    {
        if (!uv_loop_alive(&cl->loop_info.loop))
        {
            break;
        }
        uv_run(&cl->loop_info.loop, UV_RUN_DEFAULT);
    }
}


void init_client_loop(ssn_pushmsg_cb pushmsg_cb, ssn_conn_changed_cb conn_cb)
{
	uv_loop_init(&g_ClientLoop.loop_info.loop);
	
    g_ClientLoop.loop_info.type = CLIENT_LOOP;
    g_ClientLoop.loop_info.inited = true;
	                      
    g_ClientLoop.loop_info.running = false;
	uv_rwlock_init(&g_ClientLoop.connTable.connLock);
	uv_mutex_init(&g_ClientLoop.taskTable.taskLock);
    uv_mutex_init(&g_ClientLoop.timerTable.timerLock);

    uv_mutex_init(&g_ClientLoop.condlock);
    uv_cond_init(&g_ClientLoop.cond);
	// uv_thread_create(&g_ClientLoop->thread, NULL, &g_ClientLoop);
	
    g_ClientLoop.loop_info.loop.data = &g_ClientLoop;
    g_ClientLoop.pushmsg_cb = pushmsg_cb;
    g_ClientLoop.conn_cb = conn_cb;

    return;
}

int start_client_loop()
{
    if (g_ClientLoop.loop_info.running)
    {
        return 0;
    }

    if (!g_ClientLoop.loop_info.inited)
    {
        return -1;
    }

    uv_thread_create(&g_ClientLoop.thread, _clientloop_process, &g_ClientLoop);
    g_ClientLoop.loop_info.running = true;
    
    // uv_thread_join(&g_ClientLoop.thread);

    return 0;
}

int stop_client_loop()
{
    if (!g_ClientLoop.loop_info.running)
    {
        return 0;
    }

    if (!g_ClientLoop.loop_info.inited)
    {
        return -1;
    }


    return 0;
}

int uninit_client_loop()
{
	// stop loop
	
    return 0;
}

int init_tcp_conn(loop_type_t type, tcp_conn_t* conn)
{
    int err = 0;
    conn->tcp.cache = NULL;
    conn->tcp.maxlength = 0;
    conn->tcp.length = 0;
    conn->tcp.type = TCP_CLIENT;

    if (type == CLIENT_LOOP)
        err = uv_tcp_init_ex(&g_ClientLoop.loop_info.loop, &conn->tcp.handle, AF_INET);
    else
        err = uv_tcp_init_ex(&g_serverLoop.loop_info.loop, &conn->tcp.handle, AF_UNSPEC);

    conn->tcp.handle.data = conn;

    if (err != 0)
    {
        err = uverr_convert(err);
    }
    return err;
}


uv_async_t* cl_create_async()
{
    if (!g_ClientLoop.loop_info.inited && !g_ClientLoop.loop_info.running)
    {
        return NULL;
    }
    uv_async_t* async = new uv_async_t;
    uv_async_init(&g_ClientLoop.loop_info.loop, async, async_cb);
    return async;
}


uv_loop_t* cl_loop()
{
    return &g_ClientLoop.loop_info.loop;
}

int cl_conn_add(uint16_t connId, tcp_conn_t* conn)
{
    int err;
    uv_rwlock_rdlock(&g_ClientLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_ClientLoop.connTable.table.find(connId);
    if (it == g_ClientLoop.connTable.table.end())
    {
        conn->connId = connId;
        g_ClientLoop.connTable.table[connId] = conn;
        err = 0;
    }
    else
    {
        err = -1;
    }

    uv_rwlock_rdunlock(&g_ClientLoop.connTable.connLock);
    return err;
}

tcp_conn_t* cl_conn_del(uint16_t connId)
{
    tcp_conn_t* conn = NULL;
    uv_rwlock_rdlock(&g_ClientLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_ClientLoop.connTable.table.find(connId);
    if (it != g_ClientLoop.connTable.table.end())
    {
        conn = it->second;
        g_ClientLoop.connTable.table.erase(it);
    }
    uv_rwlock_rdunlock(&g_ClientLoop.connTable.connLock);
    return conn;
}

void cl_conn_del2 (const tcp_conn_t* conn)
{
    uv_rwlock_rdlock(&g_ClientLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;

    it = g_ClientLoop.connTable.table.find(conn->connId);
    if (it != g_ClientLoop.connTable.table.end())
    {
        g_ClientLoop.connTable.table.erase(it);
    }
    uv_rwlock_rdunlock(&g_ClientLoop.connTable.connLock);
}


tcp_conn_t* cl_conn_find(uint16_t connId)
{
    tcp_conn_t* conn = NULL;
    uv_rwlock_wrlock(&g_ClientLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_ClientLoop.connTable.table.find(connId);
    if (it != g_ClientLoop.connTable.table.end())
    {
        conn = it->second;
    }

    uv_rwlock_wrunlock(&g_ClientLoop.connTable.connLock);
    return conn;
}
std::map<uint16_t, tcp_conn_t*> cl_conn_list()
{
    std::map<uint16_t, tcp_conn_t*> list;
    uv_rwlock_wrlock(&g_ClientLoop.connTable.connLock);
    list = g_ClientLoop.connTable.table;
    uv_rwlock_wrunlock(&g_ClientLoop.connTable.connLock);
    return list;
}

bool cl_conn_valid(uint16_t connId)
{
    bool flag = false;
    uv_rwlock_wrlock(&g_ClientLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_ClientLoop.connTable.table.find(connId);
    if (it != g_ClientLoop.connTable.table.end())
    {
        flag = true;
    }

    uv_rwlock_wrunlock(&g_ClientLoop.connTable.connLock);
    return flag;
}


int cl_task_add(uint64_t taskId, const abs_task_t* task)
{
    //uv_mutex_lock(&g_ClientLoop.taskTable.taskLock);
    g_ClientLoop.taskTable.table[taskId] = (abs_task_t*)task;
    //uv_mutex_unlock(&g_ClientLoop.taskTable.taskLock);
    return 0;
}
abs_task_t* cl_task_del(uint64_t taskId)
{
    abs_task_t* task = NULL;
    //uv_mutex_lock(&g_ClientLoop.taskTable.taskLock);

    std::map<uint64_t, abs_task_t*>::iterator it;

    it = g_ClientLoop.taskTable.table.find(taskId);
    if (it != g_ClientLoop.taskTable.table.end())
    {
        task = it->second;
        g_ClientLoop.taskTable.table.erase(it);
    }
    //uv_mutex_unlock(&g_ClientLoop.taskTable.taskLock);
    return task;
}
abs_task_t* cl_task_find(uint64_t taskId)
{
    abs_task_t* task = NULL;
    //uv_mutex_lock(&g_ClientLoop.taskTable.taskLock);

    std::map<uint64_t, abs_task_t*>::iterator it;

    it = g_ClientLoop.taskTable.table.find(taskId);
    if (it != g_ClientLoop.taskTable.table.end())
    {
        task = it->second;
    }
    //uv_mutex_unlock(&g_ClientLoop.taskTable.taskLock);
    return task;
}

int cl_timer_add(uint64_t taskId, const timer_data_t* timer)
{
    //uv_mutex_lock(&g_ClientLoop.timerTable.timerLock);
    g_ClientLoop.timerTable.table[taskId] = (timer_data_t*)timer;
    //uv_mutex_unlock(&g_ClientLoop.timerTable.timerLock);
    return 0;
}
timer_data_t* cl_timer_del(uint64_t taskId)
{
    timer_data_t* timer = NULL;
    //uv_mutex_lock(&g_ClientLoop.timerTable.timerLock);

    std::map<uint64_t, timer_data_t*>::iterator it;
    it = g_ClientLoop.timerTable.table.find(taskId);
    if (it != g_ClientLoop.timerTable.table.end())
    {
        timer = it->second;
        g_ClientLoop.timerTable.table.erase(it);
    }
    //uv_mutex_unlock(&g_ClientLoop.timerTable.timerLock);
    return timer;
}
timer_data_t* cl_timer_find(uint64_t taskId)
{
    timer_data_t* timer = NULL;
    //uv_mutex_lock(&g_ClientLoop.timerTable.timerLock);

    std::map<uint64_t, timer_data_t*>::iterator it;
    it = g_ClientLoop.timerTable.table.find(taskId);
    if (it != g_ClientLoop.timerTable.table.end())
    {
        timer = it->second;
    }
    //uv_mutex_unlock(&g_ClientLoop.timerTable.timerLock);
    return timer;
}



static int _init_svr_tcp(uv_tcp_t* svrTcp)
{
    int err;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = PhyIpAddress();
    addr.sin_port = htons(10038);

    err = uv_tcp_init_ex(&g_serverLoop.loop_info.loop, svrTcp, AF_INET);
    if (0 != err)
    {
        return uverr_convert(err);
    }
    err = uv_tcp_bind(svrTcp, (sockaddr*)& addr, 0);
    if (0 != err)
    {
        goto exit;
    }
    err = uv_listen((uv_stream_t*)svrTcp, 1, listen_cb);
    if (0 != err)
    {
        goto exit;
    }

    return 0;

exit:

    uv_close((uv_handle_t*)svrTcp, close_cb);
    uv_run(&g_serverLoop.loop_info.loop, UV_RUN_ONCE);

    return uverr_convert(err);
}




void init_server_loop()
{
    uv_loop_init(&g_serverLoop.loop_info.loop);

    g_serverLoop.loop_info.type = SERVER_LOOP;
    uv_rwlock_init(&g_serverLoop.connTable.connLock);

    // uv_thread_create(&g_ClientLoop->thread, NULL, &g_ClientLoop);

    g_serverLoop.loop_info.loop.data = &g_serverLoop;
    g_serverLoop.loop_info.inited = true;
    g_serverLoop.loop_info.running = false;

    return;
}

int start_server_loop(ssn_work_process_cb cb, size_t workthread_num)
{
    int err;
    if (g_serverLoop.loop_info.running)
    {
        return 0;
    }

    if (!g_serverLoop.loop_info.inited)
    {
        return -1;
    }

    err = _init_svr_tcp(&g_serverLoop.listen);
    if (err != 0){
        return -1;
    }
    g_serverLoop.listen.data = NULL; 

    threadpool_init(&g_serverLoop.pool, workthread_num);
    g_serverLoop.work_cb = cb;
    uv_thread_create(&g_serverLoop.thread, _serverloop_process, &g_serverLoop);
    g_serverLoop.loop_info.running = true;
    
    //uv_thread_join(&g_serverLoop.thread);

    return 0;
}


int stop_server_loop()
{
    
    return 0;
}


int uninit_server_loop()
{
    return 0;
}

uv_async_t* sl_create_async()
{
    if (!g_serverLoop.loop_info.inited && !g_serverLoop.loop_info.running)
    {
        return NULL;
    }
    uv_async_t* async = new uv_async_t;
    uv_async_init(&g_serverLoop.loop_info.loop, async, async_cb);
    return async;
}


loop_type_t loop_type(uv_loop_t* loop)
{
    loop_info_t* info = (loop_info_t*)loop->data;
    return info->type;
}


int sl_conn_add(uint16_t connId, tcp_conn_t* conn)
{
    int err;
    uv_rwlock_rdlock(&g_serverLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_serverLoop.connTable.table.find(connId);
    if (it == g_serverLoop.connTable.table.end())
    {
        conn->connId = connId;
        g_serverLoop.connTable.table[connId] = conn;
        err = 0;
    }
    else
    {
        err = -1;
    }
    
    uv_rwlock_rdunlock(&g_serverLoop.connTable.connLock);
    return err;
}

tcp_conn_t* sl_conn_del(uint16_t connId)
{
    tcp_conn_t* conn = NULL;
    uv_rwlock_rdlock(&g_serverLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_serverLoop.connTable.table.find(connId);
    if (it != g_serverLoop.connTable.table.end()){ 
        conn = it->second;
        g_serverLoop.connTable.table.erase(it);
    }
    uv_rwlock_rdunlock(&g_serverLoop.connTable.connLock);
    return conn;
}

void sl_conn_del2(const tcp_conn_t* conn)
{
    uv_rwlock_rdlock(&g_serverLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;

    it = g_serverLoop.connTable.table.find(conn->connId);
    if (it != g_serverLoop.connTable.table.end())
    {
        g_serverLoop.connTable.table.erase(it);
    }
    uv_rwlock_rdunlock(&g_serverLoop.connTable.connLock);
}


tcp_conn_t* sl_conn_find(uint16_t connId)
{
    tcp_conn_t* conn = NULL;
    uv_rwlock_wrlock(&g_serverLoop.connTable.connLock);

    std::map<uint16_t, tcp_conn_t*>::iterator it;
    it = g_serverLoop.connTable.table.find(connId);
    if (it != g_serverLoop.connTable.table.end())
    {
        conn = it->second;
    }

    uv_rwlock_wrunlock(&g_serverLoop.connTable.connLock);
    return conn;
}

std::map<uint16_t, tcp_conn_t*> sl_conn_list()
{
    std::map<uint16_t, tcp_conn_t*> list;
    uv_rwlock_wrlock(&g_serverLoop.connTable.connLock);
    list = g_serverLoop.connTable.table;
    uv_rwlock_wrunlock(&g_serverLoop.connTable.connLock);
    return list;
}

uv_loop_t* sl_loop()
{
    return &g_serverLoop.loop_info.loop;
}




