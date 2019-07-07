
#include <assert.h>
#include <stdint.h>
#include <vector>
#include <queue>
#include "comm_core.h"
#include "comm_define.h"
#include "sysheader.h"
#include "comm_res.h"
#include "ssnet_err.h"
#include "comm_protocol.h"
#include "gen_id.h"

#define MAX_RESP_BUF        (1024 * 1024 * 4)           // 服务端单次任务返回的数据最大长度不能超过此限制


void svr_work_cb(void* param)
{
    resp_task_t* task = (resp_task_t*)param;

    unsigned char* outbuf = new unsigned char[MAX_RESP_BUF];
    int outlen = MAX_RESP_BUF;
    if (task->cb)
    {
        task->cb(task->data.c_str(), task->data.size(), outbuf, &outlen);
        task->data.assign((char*)outbuf, outlen);
    }
    delete[] outbuf;

    uv_async_t* async = sl_create_async();

    async->data = task;
    uv_async_send(async);
    uv_sem_wait(&task->common.notify);

    uv_sem_destroy(&task->common.notify);
    delete task;
    return;
}



void close_cb(uv_handle_t* handle)
{
    if (handle->type == UV_TCP)
    {
        tcp_conn_t* conn = (tcp_conn_t*)handle->data;
        if (conn && conn->tcp.type == TCP_CLIENT)
        {
            // client tcp free cache
            delete[] conn->tcp.cache;
            conn->tcp.cache = NULL;
            conn->tcp.maxlength = 0;
            conn->tcp.length = 0;

            // 关闭回调执行是不是可以代表该连接的所有任务都结束了？

            // 把连接从loop中删除，回调通知
            if (loop_type(handle->loop) == SERVER_LOOP)
                sl_conn_del(conn->connId);
            else
            {
                client_loop_t* loopInfo = (client_loop_t*)handle->loop->data;
                if (cl_conn_del(conn->connId) && loopInfo->conn_cb)
                    loopInfo->conn_cb(conn->connId, conn->info.ip.c_str(), false);
            }
        }
        delete conn;
        // delete handle;
    }
    else if (handle->type == UV_TIMER)
    {
        delete handle->data;
    }
    else if (handle->type == UV_ASYNC)
    {
        delete handle;
    }
    else
    {
        // assert(0);
    }

    return;
}



static void _safe_uv_close(uv_handle_t* handle, uv_close_cb cb)
{
    if (!uv_is_closing(handle))
        uv_close(handle, cb);
    return;
}


void idle_cb(uv_idle_t* handle)
{
    // to nothing
}



static void _cl_finish_task(abs_task_t* task)
{
    strcpy(task->errmsg, ssn_errmsg(task->err));
    

    cl_task_del(task->taskId);
    timer_data_t* td = cl_timer_find(task->taskId);
    if (td)
    {
        uv_timer_stop(&td->timer);
        _safe_uv_close((uv_handle_t*)&td->timer, close_cb);
        cl_timer_del(task->taskId);
    }
    
    // notify
    uv_sem_post(&task->notify);

    return;
}


static void _sl_finish_task(abs_task_t* task)
{
    strcpy(task->errmsg, ssn_errmsg(task->err));
    // notify
    uv_sem_post(&task->notify);

    return;
}

static void timer_cb(uv_timer_t* handle)
{
    timer_data_t* td = (timer_data_t*)handle->data;

    cl_timer_del(td->taskId);
    abs_task_t* task = cl_task_del(td->taskId);
    if (task != NULL)
    {
        // if (task->type == async_task_type::CONNECT)
        // {
        //     uv_connect_t* req = (uv_connect_t*)td->data;
        //     _safe_uv_close((uv_handle_t*)req->handle, close_cb);
        //     delete req->data;
        //     req->data = NULL;
        //     uv_cancel((uv_req_t*)req);
        //     // delete req;
        // }
        task->err = ERR_TIMEOUT;
        _cl_finish_task(task);
    }

    _safe_uv_close((uv_handle_t*)handle, close_cb);
    
    return;
}


static void write_cb(uv_write_t* req, int status)
{
    int ret = 0;

    abs_task_t* task = (abs_task_t*)req->data;

    switch (task->type)
    {
    case RW:
        if (0 != status)
        {
            ret = uverr_convert(status);
            task->err = ret;
            _cl_finish_task((abs_task_t*)task);
        }
        else
        {
            rw_task_t* rw_task = (rw_task_t*)task;

            if (rw_task->timeout > 0)
            {
                timer_data_t* td = new timer_data_t;
                uv_timer_init(req->handle->loop, &td->timer);
                uv_timer_start(&td->timer, timer_cb, rw_task->timeout, 0);
                td->timer.data = td;
                td->taskId = rw_task->common.taskId;
                // 定时器放到timer队列
                cl_timer_add(task->taskId, td);
            }
            
        }
        break;

    case PUSH:
        delete req->data;       // 推送任务要释放
        break;
    case RESP:
        ret = uverr_convert(status);
        task->err = ret;
        _sl_finish_task((abs_task_t*)task);
        break;
    default:
        assert(0);
        break;
    }

    delete req;
    return;
}


static void _server_handler_req(server_loop_t* svrLoop, uint16_t connId, uint64_t taskId, const void* data, int len)
{
    resp_task_t* task = new resp_task_t;
    
    uv_sem_init(&task->common.notify, 1);
    uv_sem_wait(&task->common.notify);
    task->common.type = RESP;
    task->connId = connId;
    task->common.taskId = taskId;
    task->data.assign((char*)data, len);
    task->cb = svrLoop->work_cb;

    int err = threadpool_push_work(svrLoop->pool, task, svr_work_cb, NULL);
    while (err == -1)
    {// 队列已满，再继续试，概率较小
        threadpool_push_work(svrLoop->pool, task, svr_work_cb, NULL);
    }


    return;
}


static void _read_content_process(uv_tcp_t* handle, int size, const void* content)
{
    int err;
    loop_info_t* info = (loop_info_t*)handle->loop->data;
    std::vector<comm_pkg_t*> taskList;

    comm_pkg_t* pkg = NULL;
    void* pkgs = NULL;
    void* cursor = NULL;
    int remainsize = 0;
    tcp_conn_t* conn = (tcp_conn_t*)handle->data;
    comm_tcp_t* tcp = &conn->tcp;
    
    pkgs = new char[tcp->length + size];
    if (tcp->length != 0){
        assert(tcp->cache != NULL);
        memcpy(pkgs, tcp->cache, tcp->length);
    }

    memcpy((char*)pkgs + tcp->length, content, size);

    cursor = pkgs;
    remainsize = tcp->length + size;
    while (1)
    {
        pkg = proto_parse_package(&cursor, &remainsize);
        if (NULL == pkg){ 
            break;
        }

        err = proto_check_pkg(pkg);
        if (0 != err){ 
            // LOG;
            printf("[BAD PACKAGE] package received is invalid!\n");
            continue;
        }

        taskList.push_back(pkg);
    }

    if (tcp->maxlength < remainsize){
        delete[] tcp->cache;
        tcp->cache = new char[remainsize];
        tcp->maxlength = remainsize;
    }
    tcp->length = remainsize;
    memcpy(tcp->cache, cursor, remainsize);


    if (info->type == CLIENT_LOOP){ 
        
        // handle task
        client_loop_t* clientLoop = (client_loop_t*)info;

        for (int i = 0; i < taskList.size(); i++)
        {
            if (taskList[i]->type == PKG_TYPE_COMMON)
            {
                rw_task_t* task = (rw_task_t*)cl_task_del(taskList[i]->taskId);
                timer_data_t* td = cl_timer_del(taskList[i]->taskId);
                if (td)
                {
                    uv_timer_stop(&td->timer);
                    _safe_uv_close((uv_handle_t*)&td->timer, close_cb);
                }
                if (task)
                {
                    task->common.err = 0;
                    task->outdata.assign((char*)taskList[i]->data, taskList[i]->length);
                    _cl_finish_task((abs_task_t*)task);
                }
            }
            else if (taskList[i]->type == PKG_TYPE_PUSH)
            {
                if (clientLoop->pushmsg_cb)
                    clientLoop->pushmsg_cb(conn->connId, taskList[i]->data, taskList[i]->length);
            }
            else
            {
                assert(0);
            }
        }
    }
    else if (info->type == SERVER_LOOP)
    {
        // handle task
        server_loop_t* serverLoop = (server_loop_t*)info;

        if (sl_conn_find(conn->connId))
        {
            for (int i = 0; i < taskList.size(); i++)
            {
                if (taskList[i]->type == PKG_TYPE_COMMON)
                {
                    _server_handler_req(serverLoop, conn->connId, taskList[i]->taskId, taskList[i]->data, taskList[i]->length);
                }
                else if (taskList[i]->type == PKG_TYPE_ALIVE)
                {
                    // TODO:
                }
                else
                {
                    assert(0);
                }
            }
        }
    }
    else
    {
        assert(0);
    }


    delete[] pkgs;
    return;
}




static void alloc_cb(uv_handle_t* handle,
              size_t suggested_size,
              uv_buf_t* buf)
{
    buf->len = suggested_size;
    buf->base = new char[suggested_size];
    return;
}

static void read_cb(uv_stream_t* stream,
             ssize_t nread,
             const uv_buf_t* buf)
{
    int err = 0;
    char* pRemain = NULL;

    tcp_conn_t* conn = (tcp_conn_t*)stream->data;

    if (nread == 0){
        goto exit;
    }

    if (nread < 0){

        err = uverr_convert(nread);
        uv_read_stop(stream);
        _safe_uv_close((uv_handle_t*)stream, close_cb);

        loop_info_t* loopInfo = (loop_info_t*)stream->loop->data;
        if (loopInfo->type == CLIENT_LOOP){
            client_loop_t* clientLoop = (client_loop_t*)loopInfo;
            cl_conn_del2(conn);
            if (clientLoop->conn_cb)
                clientLoop->conn_cb(conn->connId, conn->info.ip.c_str(), false);
        }
        else
        {
            sl_conn_del2(conn);
        }
        // delete conn;
        goto exit;
    }

    _read_content_process((uv_tcp_t*)stream, nread, buf->base);

exit:
    delete[] buf->base;
    //buf->len = 0;
    return;
}



// 服务端监听回调
void listen_cb(uv_stream_t* server, int status)
{
    int err = 0;
    if (status != 0){
        err = uverr_convert(status);
        // TODO: LOG
        return;
    }

    tcp_conn_t* conn = new tcp_conn_t;

    err = init_tcp_conn(SERVER_LOOP, conn);
    if (err != 0)
    {
        delete conn;
        err = uverr_convert(status);
        // TODO: LOG
        return;
    }
    err = uv_accept(server, (uv_stream_t*)&conn->tcp.handle);
    if (0 != err){
        delete conn;
        err = uverr_convert(err);
        // TODO: LOG
        return;
    }

    char ip[64];
    sockaddr_in addr;
    int addrlen = sizeof(addr);
    err = uv_tcp_getpeername(&conn->tcp.handle, (sockaddr*)&addr, &addrlen);
    if (0 != err){
        _safe_uv_close((uv_handle_t*)&conn->tcp.handle, close_cb);
        err = uverr_convert(err);
        delete conn;
        // TODO: LOG
        return;
    }

    inet_ntop(AF_INET, &addr, ip, 64);
    conn->info.ip = ip;
    conn->info.port = -1;    // do nothing
    
    // 检查黑白名单
    if (0){
        _safe_uv_close((uv_handle_t*)&conn->tcp.handle, close_cb);
        delete conn;
        return;
    }

    // 把连接添加至服务loop的连接表
    uint16_t connId = ApplyConnIdForSvr();
    sl_conn_add(connId, conn);

    // 给连接挂上读取请求
    uv_read_start((uv_stream_t*)&conn->tcp.handle, alloc_cb, read_cb);

    return;
}


static int _do_write(tcp_conn_t* conn, string& indata, abs_task_t* task)
{
    int ret = 0;
    uv_write_t* req = new uv_write_t;

    // build proto package
    uint8_t pkg_type;
    if (task->type == async_task_type::RW){
        pkg_type = PKG_TYPE_COMMON;
    }
    else if (task->type == async_task_type::PUSH){
        pkg_type = PKG_TYPE_PUSH;
    }
    else if (task->type == async_task_type::RESP)
    {
        pkg_type = PKG_TYPE_COMMON;
    }
    else
    {
        assert(0);
    }

    // pkg需要释放的
    comm_pkg_t* pkg = proto_build_package(indata.c_str(), indata.size(), pkg_type, task->taskId);
    uv_buf_t buf;
    buf.base = (char*)pkg;
    buf.len = pkg->length + sizeof(comm_pkg_t);

    req->data = task;
    ret = uv_write(req, (uv_stream_t*)&conn->tcp.handle, &buf, 1, write_cb);
    proto_release_package(pkg);

    if (ret != 0)
    {
        task->err = uverr_convert(ret);
        goto exit;
    }

exit:
    if (ret != 0){
        delete req;
    }

    return ret;
}

typedef struct
{
    uint64_t taskId;
    tcp_conn_t* conn;
}conn_req_t;


// 客户端连接回调, 如果回调成功，表示定时器还未超时
void connect_cb(uv_connect_t* req, int status)
{
    conn_req_t* reqData = (conn_req_t*)req->data;
    conn_task_t* task = (conn_task_t*)cl_task_del(reqData->taskId);
    if (!task)
    {
        // 结束请求
        delete reqData;
        delete req;
        return;
    }

    // 取消定时器
    timer_data_t* td = cl_timer_del(task->common.taskId);
    if (td != NULL)
    {
        uv_timer_stop(&td->timer);
        _safe_uv_close((uv_handle_t*)&td->timer, close_cb);
    }

    if (status != 0)
    {
        _safe_uv_close((uv_handle_t*)req->handle, close_cb);
        task->common.err = uverr_convert(status);
    }
    else
    {
        // 把连接添加至loop的连接表
        uint16_t connId = ApplyConnIdForClt();
        cl_conn_add(connId, reqData->conn);

        client_loop_t* loopInfo = (client_loop_t*)req->handle->loop->data;
        if (loopInfo->conn_cb)
            loopInfo->conn_cb(connId, reqData->conn->info.ip.c_str(), true);

        // 给连接挂上读取请求
        uv_read_start((uv_stream_t*)req->handle, alloc_cb, read_cb);

        task->common.err = 0; // 设置任务成功
    }

    // 结束请求
    delete reqData;
    delete req;

    _cl_finish_task((abs_task_t*)task);

    return;
}


static int _do_connect(abs_task_t* task)
{
    int err;
    //comm_tcp_t conn;
    conn_task_t* ct = (conn_task_t*)task;

    // 查找连接是否已存在
    if (NULL != cl_conn_find2(ct->ip))
    {
        task->err = ERR_CONN_ALREADY_EXIST;
        return -1;
    }


    conn_req_t* reqData = new conn_req_t;
    reqData->conn = new tcp_conn_t;
    reqData->taskId = ct->common.taskId;
    reqData->conn->info.ip = ct->ip;
    reqData->conn->info.port = ct->port;

    err = init_tcp_conn(CLIENT_LOOP, reqData->conn);
    if (0 != err)
    {
        delete reqData->conn;
        delete reqData;
        task->err = err;
        return -1;
    }

    uv_connect_t* req = new uv_connect_t;
    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ct->port);
    inet_pton(AF_INET, ct->ip, &addr.sin_addr);

    req->data = reqData;

    err = uv_tcp_connect(req, &reqData->conn->tcp.handle, (sockaddr*)&addr, connect_cb);
    if (err != 0){
        task->err = uverr_convert(err);
        _safe_uv_close((uv_handle_t*)&reqData->conn->tcp.handle, close_cb);
        // delete reqData->conn;
        delete reqData;
        return -1;
    }

    if (ct->timeout > 0)
    {
        timer_data_t* td = new timer_data_t;
        uv_timer_init(reqData->conn->tcp.handle.loop, &td->timer);
        uv_timer_start(&td->timer, timer_cb, ct->timeout, 0);
        td->taskId = ct->common.taskId;
        td->data = req;
        td->timer.data = td;
        // 定时器放到timer队列
        cl_timer_add(task->taskId, td);
    }
    
    // 将timer放到timer队列中，taskId作为key，timer_data_t*为value

    return 0;
}

static void _do_push(abs_task_t* task)
{
    int err = 0;
    push_task_t* pushTask = (push_task_t*)task;

    // 获取所有客户端连接
    std::map<uint16_t, tcp_conn_t*> connList = sl_conn_list();
    for (std::map<uint16_t, tcp_conn_t*>::iterator it = connList.begin();
         it != connList.end(); it++)
    {
        push_task_t* elementTask = new push_task_t;
        elementTask->common.type = async_task_type::PUSH;
        if (_do_write(it->second, pushTask->indata, (abs_task_t*)elementTask) != 0)
        {
            delete elementTask;
        }
    }

    task->err = 0;  // 推送任务没有错误
    return;
}


static void _client_shutdown(client_loop_t* loopInfo)
{
    uv_mutex_t condlock;
    uv_cond_t cond;

    uv_mutex_init(&condlock);
    uv_cond_init(&cond);

    loopInfo->loop_info.running = false;

    uv_mutex_lock(&condlock);
    uv_cond_timedwait(&cond, &condlock, 500);   // 等待500毫秒，接收已经投递的任务
    uv_mutex_unlock(&condlock);

    //uv_run(&loopInfo->loop_info.loop, UV_RUN_DEFAULT);

    std::map<uint16_t, tcp_conn_t*> allconn = cl_conn_list();
    std::map<uint16_t, tcp_conn_t*>::iterator it_conn;
    for (it_conn = allconn.begin(); it_conn != allconn.end(); it_conn++)
    {
        uv_read_stop((uv_stream_t*)&it_conn->second->tcp.handle);
        _safe_uv_close((uv_handle_t*)&it_conn->second->tcp.handle, close_cb);
    }

    std::map<uint64_t, timer_data_t*> alltimer = cl_list_timer();
    std::map<uint64_t, timer_data_t*>::iterator it_timer;
    for (it_timer = alltimer.begin(); it_timer != alltimer.end(); it_timer++)
    {
        _safe_uv_close((uv_handle_t*)&it_timer->second->timer, close_cb);
    }
    //uv_run(&loopInfo->loop_info.loop, UV_RUN_DEFAULT);
    cl_timer_clr();
    cl_conn_clr();

    std::map<uint64_t, abs_task_t*> alltask = cl_list_task();
    std::map<uint64_t, abs_task_t*>::iterator it_task;
    for (it_task = alltask.begin(); it_task != alltask.end(); it_task++)
    {
        it_task->second->err = ERR_SHUTDOWN;
        _cl_finish_task(it_task->second);
    }
    // uv_run(&loopInfo->loop_info.loop, UV_RUN_DEFAULT);
    cl_task_clr();

    // _safe_uv_close((uv_handle_t*)&loopInfo->no_exit, close_cb);
    uv_mutex_destroy(&condlock);
    uv_cond_destroy(&cond);
    return;
}

static void _server_shutdown(server_loop_t* loopInfo)
{
    uv_mutex_t condlock;
    uv_cond_t cond;

    uv_mutex_init(&condlock);
    uv_cond_init(&cond);

    loopInfo->loop_info.running = false;
    _safe_uv_close((uv_handle_t*)&loopInfo->listen, close_cb);

    std::map<uint16_t, tcp_conn_t*> allconn =  sl_conn_list();
    std::map<uint16_t, tcp_conn_t*>::iterator it;
    for (it = allconn.begin(); it != allconn.end(); it++)
    {
        uv_read_stop((uv_stream_t*)&it->second->tcp.handle);
    }

    // uv_run(&loopInfo->loop_info.loop, UV_RUN_DEFAULT);

    uv_mutex_lock(&condlock);
    uv_cond_timedwait(&cond, &condlock, 800);   // 等待800毫秒，让线程池任务尽量完成
    uv_mutex_unlock(&condlock);
    // uv_run(&loopInfo->loop_info.loop, UV_RUN_DEFAULT);

    threadpool_uninit(loopInfo->pool);

    for (it = allconn.begin(); it != allconn.end(); it++)
    {
        _safe_uv_close((uv_handle_t*)&it->second->tcp.handle, close_cb);
    }
    sl_conn_clr();

    uv_mutex_destroy(&condlock);
    uv_cond_destroy(&cond);

    return;
}

static void _do_shutdown(loop_info_t* loopInfo)
{
    if (loopInfo->type == CLIENT_LOOP)
    {
        _client_shutdown((client_loop_t*)loopInfo);
    }
    else if (loopInfo->type == SERVER_LOOP)
    {
        _server_shutdown((server_loop_t*)loopInfo);
    }
    else
    {
        assert(0);
    }
}




void async_cb(uv_async_t* handle)
{
    int err;
    abs_task_t* task = (abs_task_t*)handle->data;

    switch (task->type)
    {
    case async_task_type::RW:
    {
        // 放入任务队列
        cl_task_add(task->taskId, (abs_task_t*)task);
        rw_task_t* rt = (rw_task_t*)task;
        tcp_conn_t* conn = cl_conn_find(rt->connId);
        if (!conn)
        {
            task->err = ERR_CONN_NOT_EXIST;
            _cl_finish_task(task);
            break;
        }
        if (_do_write(conn, rt->indata, task) != 0)
        {
            _cl_finish_task(task);
        }
        break;
    }
    case async_task_type::RESP:
    {
        resp_task_t* resptask = (resp_task_t*)task;
        tcp_conn_t* conn = sl_conn_find(resptask->connId);
        if (!conn)
        {
            task->err = ERR_CONN_NOT_EXIST;
            _sl_finish_task(task);
            break;
        }

        err = _do_write(conn, resptask->data, task);
        if (err != 0)
        {
            _sl_finish_task(task);
        }
        break;
    }
    case async_task_type::PUSH:
    {
        _do_push(task);
        _sl_finish_task(task);
        break;
    }
    case async_task_type::CONNECT:
    {
        // 放入任务队列
        cl_task_add(task->taskId, (abs_task_t*)task);
        err = _do_connect(task);
        if (err != 0){
            _cl_finish_task(task);
        }
        break;
    }
    case async_task_type::CLOSE:
    {
        loop_info_t* loopInfo = (loop_info_t*)handle->loop->data;

        _do_shutdown(loopInfo);

        if (loopInfo->type == CLIENT_LOOP)
        {
            _cl_finish_task(task);
        }
        else if(loopInfo->type == SERVER_LOOP)
        {
            _sl_finish_task(task);
        }
        else
        {
            assert(0);
        }

        break;
    }
    default:
        assert(0);
        break;
    }

    _safe_uv_close((uv_handle_t*)handle, close_cb);

    return;
}

