
#include <assert.h>
#include <stdint.h>
#include <vector>
#include <queue>
#include "comm_core.h"
#include "conn_table.h"
#include "comm_define.h"
#include "sysheader.h"
#include "comm_res.h"
#include "ssnet_err.h"
#include "comm_protocol.h"




typedef struct
{
    abs_task_t* task;
    uv_timer_t* timer;
    uv_req_t* req;
}timer_data_t;










static void _finish_task(abs_task_t* task)
{
    strcpy(task->errmsg, ssn_errmsg(task->err));
    // 找到任务

    // 删除任务

    // notify
    uv_sem_post(&task->notify);

    return;
}


void close_cb(uv_handle_t* handle)
{
    if (handle->type == UV_TCP)
    {
        if (loop_type(handle->loop) == CLIENT_LOOP)
        {
            // client tcp free cache
            // 清理连接中的任务，让任务返回
            // 把连接从loop中删除，回调通知
        }
        else if (loop_type(handle->loop) == SERVER_LOOP)
        {
            // server tcp closed，server loop should stop

            
        }

        delete handle;
    }
    else if (handle->type == UV_TIMER)
    {
        delete handle;
    }
    else if (handle->type == UV_ASYNC)
    {
        delete handle;
    }
    else
    {
        assert(0);
    }

    return;
}


static void timer_cb(uv_timer_t* handle)
{
    timer_data_t* data = (timer_data_t*)handle->data;

    if (data->task->type == async_task_type::CONNECT)
    {
        conn_task_t* connTask = (conn_task_t*)data->task;
        uv_close(connTask->conn->handle, close_cb);
        delete connTask->conn;
        connTask->conn = NULL;
    }

    uv_close((uv_handle_t*)handle, close_cb);
    
    data->task->err = ERR_TIMEOUT;
    _finish_task(data->task);
    delete data->timer;
    delete data;

    return;
}


static void write_cb(uv_write_t* req, int status)
{
    int ret = 0;

    abs_task_t* task = (abs_task_t*)req->data;
    if (task->type == PUSH){
        delete req->data;       // 推送任务要释放
        delete req;
        return;
    }

    if (!status){
        ret = uverr_convert(status);
        task->err = ret;
        _finish_task((abs_task_t*)task);
    }
    else
    {
        rw_task_t* rw_task = (rw_task_t*)task;
        timer_data_t* td = new timer_data_t;
        td->timer = new uv_timer_t;
        uv_timer_init(req->handle->loop, td->timer);
        uv_timer_start(td->timer, timer_cb, rw_task->timeout, 0);
        td->timer->data = td;
        td->task = (abs_task_t*)rw_task;
        td->req = NULL; // 这个req不需要再释放了，但是必须置为NULL

        // 定时器放到timer队列
    }

    delete req;
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
    comm_tcp_t* conn = (comm_tcp_t*)handle->data;
    
    pkgs = new char[conn->length + size];
    if (conn->length != 0){ 
        assert(conn->cache != NULL);
        memcpy(pkgs, conn->cache, conn->length);
    }

    memcpy((char*)pkgs + conn->length, content, size);

    cursor = pkgs;
    remainsize = conn->length + size;
    while (1)
    {
        pkg = proto_parse_package(&cursor, &remainsize);
        if (NULL == pkg){ 
            break;
        }

        err = proto_check_pkg(pkg);
        if (!err){ 
            // LOG;
        }

        taskList.push_back(pkg);
    }

    if (conn->maxlength < remainsize){ 
        delete[] conn->cache;
        conn->cache = new char[remainsize];
        conn->maxlength = remainsize;
    }
    conn->length = remainsize;
    memcpy(conn->cache, cursor, remainsize);


    if (info->type == CLIENT_LOOP){ 
        
        // handle task



    }
    else if (info->type == SERVER_LOOP)
    {
        // handle task



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

    if (nread == 0){
        goto exit;
    }

    if (nread < 0){

        err = uverr_convert(nread);
        uv_close((uv_handle_t*)stream, close_cb);

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
    conn->tcp.handle = new uv_tcp_t;
    conn->tcp.cache = NULL;
    conn->tcp.length = 0;
    conn->tcp.maxlength = 0;
    conn->tcp.type = TCP_CLIENT;

    err = uv_accept(server, (uv_stream_t*)conn->tcp.handle);
    if (0 != err){
        delete conn->tcp.handle;
        delete conn;
        err = uverr_convert(err);
        // TODO: LOG
        return;
    }

    char ip[64];
    sockaddr_in addr;
    int addrlen = sizeof(addr);
    err = uv_tcp_getpeername(conn->tcp.handle, (sockaddr*)&addr, &addrlen);
    if (0 != err){
        uv_close((uv_handle_t*)conn->tcp.handle, close_cb);
        err = uverr_convert(err);
        delete[] conn;
        // TODO: LOG
        return;
    }

    inet_ntop(AF_INET, &addr, ip, 64);
    conn->info.ip = ip;
    conn->info.port = -1;    // do nothing
    
    // 检查黑白名单
    if (0){
        uv_close((uv_handle_t*)conn->tcp.handle, close_cb);
        delete[] conn;
        return;
    }

    // 把连接添加至loop的连接表
    //TOOD: add conn to connTable

    // 给连接挂上读取请求
    uv_read_start((uv_stream_t*)conn->tcp.handle, alloc_cb, read_cb);

    return;
}

// 客户端连接回调
void connect_cb(uv_connect_t* req, int status)
{
    // 中止计时器 TODO:

    conn_task_t* task = (conn_task_t*)req->data;
    if (status != 0){
        uv_close((uv_handle_t*)req->handle, close_cb);
        task->common.err = uverr_convert(status);
    }
    else
    {
        // 连接写入连接表
        tcp_conn_t* conn = new tcp_conn_t;

        memcpy(&conn->tcp, (comm_tcp_t*)req->handle->data, sizeof(comm_tcp_t));
        conn->tcp.handle->data = &conn->tcp;    // 重置一下handle->data的指向
        conn->info.ip = task->ip;
        conn->info.port = task->port;
        // TODO: add conn to conntable

        // 给连接挂上读取请求
        uv_read_start((uv_stream_t*)req->handle, alloc_cb, read_cb);

        task->common.err = 0; // 设置任务成功
    }

    // 释放comm_tcp_t
    delete req->handle->data;   
    // 结束任务
    delete req;

    _finish_task((abs_task_t*)task);

    return;
}


static int _do_write(uint16_t connId, string indata, abs_task_t* task)
{
    int ret = 0;
    //rw_task_t* task = (rw_task_t*)at;
    uv_write_t* req = new uv_write_t;

    tcp_conn_t* conn = cl_conn_find(connId);
    // 检查连接
    if (!conn)
    {
        ret = -1;
        task->err = ERR_CONN_NOT_EXIST;
        goto exit;
    }

    // build proto package
    uint8_t pkg_type;
    if (task->type == async_task_type::RW){
        pkg_type = PKG_TYPE_COMMON;
    }
    else if (task->type == async_task_type::PUSH){
        pkg_type = PKG_TYPE_PUSH;
    }
    else{ 
        assert(0);
    }

    // pkg需要释放的
    comm_pkg_t* pkg = proto_build_package(indata.c_str(), indata.size(), pkg_type, task->taskId);
    // task->indata.assign((char*)pkg, pkg->length + sizeof(comm_pkg_t));
    //pkg = NULL;

    uv_buf_t buf;
    //buf.base = (char*)task->indata.c_str();
    //buf.len = task->indata.size();

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

static int _do_connect(abs_task_t* task)
{
    int err;
    //comm_tcp_t conn;
    conn_task_t* ct = (conn_task_t*)task;
    ct->conn = new comm_tcp_t;
    //_conn_data_t* data = new _conn_data_t;
    err = create_clt_tcp(ct->conn);
    if (0 != err)
    {
        delete ct->conn;
        task->err = err;
        return -1;
    }

    uv_connect_t* req = new uv_connect_t;
    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = ct->port;
    inet_pton(AF_INET, ct->ip, &addr.sin_addr);

    req->data = ct;
    err = uv_tcp_connect(req, ct->conn->handle, (sockaddr*)&addr, connect_cb);
    if (err != 0){
        task->err = uverr_convert(err);
        uv_close((uv_handle_t*)ct->conn->handle, close_cb);
        delete ct->conn;
        return -1;
    }

    // 计时器关闭时如何释放需要考虑
    timer_data_t* td = new timer_data_t;
    uv_timer_t* timer = new uv_timer_t;
    uv_timer_init(ct->conn->handle->loop, timer);
    uv_timer_start(timer, timer_cb, ct->timeout, 0);
    td->task = (abs_task_t*)ct;
    td->timer = timer;
    td->req = (uv_req_t*)req;
    timer->data = td;

    // 将timer放到timer队列中，taskId作为key，timer_data_t*为value

    return 0;
}

static void _do_push(abs_task_t* task)
{
    int err = 0;
    push_task_t* pushTask = (push_task_t*)task;

    // 获取所有客户端连接
    for ()
    {
        push_task_t* elementTask = new push_task_t;
        elementTask->common.type = async_task_type::PUSH;
        if (_do_write(, pushTask->indata, (abs_task_t*)elementTask) != 0)
        {
            delete elementTask;
        }
    }

    task->err = 0;  // 推送任务没有错误
}


void async_cb(uv_async_t* handle)
{
    int err;
    abs_task_t* task = (abs_task_t*)handle->data;

    switch (task->type)
    {
    case async_task_type::RW:
    {
        rw_task_t* rt = (rw_task_t*)rt;
        err = _do_write(rt->connId, rt->indata, task);
        if (err != 0)
        {
            _finish_task(task);
        }
        break;
    }
    case async_task_type::READ:
    {
        // 没有这个异步请求吧？
        break;
    }
    case async_task_type::PUSH:
    {
        _do_push(task);
        _finish_task(task);
    }
    case async_task_type::CONNECT:
    {
        err = _do_connect(task);
        if (err != 0){
            _finish_task(task);
        }
        break;
    }
    default:
        assert(0);
        break;
    }

    uv_close((uv_handle_t*)handle, close_cb);

    return;
}


