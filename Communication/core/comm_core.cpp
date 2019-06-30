
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


static void _finish_task(abs_task_t* task)
{
    strcpy(task->errmsg, ssn_errmsg(task->err));
    // 找到任务

    // 删除任务

    // notify
    uv_sem_post(&task->notify);

    return;
}


static void close_cb(uv_handle_t* handle)
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
    abs_task_t* task = (abs_task_t*)handle->data;
    uv_close((uv_handle_t*)handle, close_cb);

    task->err = ERR_TIMEOUT;
    _finish_task((abs_task_t*)task);
}


static void write_cb(uv_write_t* req, int status)
{
    int ret = 0;

    abs_task_t* task = (abs_task_t*)req->data;
    if (task->type == PUSH){
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
        uv_timer_t* timer = new uv_timer_t;
        uv_timer_init(req->handle->loop, timer);
        uv_timer_start(timer, timer_cb, rw_task->timeout, 0);
        timer->data = rw_task;

        // 定时器关闭的时候怎么释放呢？
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

    uv_tcp_t peer;

    err = uv_accept(server, (uv_stream_t*)& peer);
    if (!err){
        err = uverr_convert(err);
        // TODO: LOG
        return;
    }

    sockaddr_in addr;
    int addrlen = sizeof(addr);
    uv_tcp_getpeername(&peer, (sockaddr*)&addr, &addrlen);

    // 检查黑白名单
    if (0){
        uv_close((uv_handle_t*)&peer, NULL);
        return;
    }

    // 添加客户端到连接表
    
    
    // 给连接挂上读取请求
    uv_read_start((uv_stream_t*)&peer, );

}

// 客户端连接回调
void connect_cb(uv_connect_t* req, int status)
{
    // 检查status
    // 成功则连接表，停止计时器，将结果写到任务队列
    // 给连接挂上读取请求

}


static void do_write(abs_task_t* at)
{
    int ret = 0;
    rw_task_t* task = (rw_task_t*)at;
    uv_write_t* req = new uv_write_t;

    tcp_conn_t* conn = cl_conn_find(task->connId);
    // 检查连接
    if (!conn)
    {
        ret = -1;
        task->common.err = ERR_CONN_NOT_EXIST;
        goto exit;
    }

    // build proto package
    uint8_t pkg_type;
    if (task->common.type == asyn_task_type::RW){
        pkg_type = PKG_TYPE_COMMON;
    }
    else if (task->common.type == asyn_task_type::PUSH){
        pkg_type = PKG_TYPE_PUSH;
    }
    else{ 
        assert(0);
    }

    // pkg需要释放的
    comm_pkg_t* pkg = proto_build_package(task->indata.c_str(), task->indata.size(), pkg_type, task->common.taskId);


    // 没法释放pkg了？ 考虑把indata覆盖掉






    uv_buf_t buf;
    buf.base = (char*)pkg;
    buf.len = sizeof(comm_pkg_t) + pkg->length;

    req->data = task;
    ret = uv_write(req, (uv_stream_t*)&conn->tcp.handle, &buf, 1, write_cb);
    if (ret != 0)
    {// ret == EPIPE
        proto_release_package(pkg);
        task->common.err = uverr_convert(ret);
        goto exit;
    }

exit:
    if (ret != 0){ 
        delete req;
        task->outdata.clear();
        _finish_task((abs_task_t*)task);
    }

    return;
}

static void do_read(abs_task_t* at)
{
    
}




void async_cb(uv_async_t* handle)
{
    abs_task_t* task = (abs_task_t*)handle->data;

    switch (task->type)
    {
    case asyn_task_type::RW:
    {
        do_write(task);
        break;
    }
    case asyn_task_type::READ:
    {
        // 没有这个异步请求吧？
        break;
    }
    case asyn_task_type::CONNECT:
    {
        break;
    }

    }

    uv_close((uv_handle_t*)handle, close_cb);

    return;
}


