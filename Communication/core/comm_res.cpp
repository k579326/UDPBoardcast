

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
            _noIdle(cl);
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


void init_client_loop()
{
	uv_loop_init(&g_ClientLoop.loop_info.loop);
	
    g_ClientLoop.loop_info.type = CLIENT_LOOP;
    g_ClientLoop.loop_info.inited = true;
	                      
    g_ClientLoop.loop_info.running = false;
	uv_rwlock_init(&g_ClientLoop.connTable.connLock);
	uv_mutex_init(&g_ClientLoop.taskTable.taskLock);

    uv_mutex_init(&g_ClientLoop.condlock);
    uv_cond_init(&g_ClientLoop.cond);
	// uv_thread_create(&g_ClientLoop->thread, NULL, &g_ClientLoop);
	
    g_ClientLoop.loop_info.loop.data = &g_ClientLoop;

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
	
	 
}

int create_clt_tcp(comm_tcp_t* cltTcp)
{
    int err = 0;
    cltTcp->cache = NULL;
    cltTcp->maxlength = 0;
    cltTcp->length = 0;
    cltTcp->type = TCP_CLIENT;
    cltTcp->handle.data = cltTcp;
    err = uv_tcp_init_ex(&g_ClientLoop.loop_info.loop, &cltTcp->handle, AF_INET);
    if (err != 0){ 
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













static int _init_svr_tcp(comm_tcp_t* svrTcp)
{
    int err;

    svrTcp->maxlength = 0;
    svrTcp->length = 0;
    svrTcp->cache = NULL;
    svrTcp->type = TCP_SERVER;
    svrTcp->handle.data = svrTcp;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = PhyIpAddress();
    addr.sin_port = htonl(10038);

    err = uv_tcp_init_ex(&g_serverLoop.loop_info.loop, &svrTcp->handle, AF_INET);
    if (0 != err)
    {
        return uverr_convert(err);
    }
    err = uv_tcp_bind(&svrTcp->handle, (sockaddr*)& addr, 0);
    if (0 != err)
    {
        uv_close((uv_handle_t*)& svrTcp->handle, NULL);
        return uverr_convert(err);
    }
    err = uv_listen((uv_stream_t*)&svrTcp->handle, 1, listen_cb);
    if (0 != err)
    {
        uv_close((uv_handle_t*)&svrTcp->handle, NULL);
        return uverr_convert(err);
    }

    return 0;
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

int start_server_loop()
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

    uv_thread_create(&g_serverLoop.thread, _serverloop_process, &g_serverLoop);
    g_serverLoop.loop_info.running = true;
    
    return 0;
}


int stop_server_loop()
{
    
    return 0;
}


int uninit_server_loop()
{
	
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