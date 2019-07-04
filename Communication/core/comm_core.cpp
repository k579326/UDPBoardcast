
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



typedef struct
{
    uint16_t connId;
    uint64_t taskId;
    ssn_work_process_cb cb;
    string data;
}svr_work_param;

void svr_work_cb(void* param)
{
    svr_work_param* task = (svr_work_param*)param;

    resp_task_t* resp = new resp_task_t;
    resp->common.taskId = task->taskId;
    resp->connId = task->connId;
    resp->common.type = RESP;

    uv_sem_init(&resp->common.notify, 1);
    uv_sem_wait(&resp->common.notify);

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
                cl_conn_del(conn->connId);
                if (loopInfo->conn_cb)
                    loopInfo->conn_cb(conn->connId, conn->info.ip.c_str(), false);
            }
                
        }
        delete handle->data;
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


static void _finish_task(abs_task_t* task)
{
    strcpy(task->errmsg, ssn_errmsg(task->err));
    // 删除任务
    cl_task_del(task->taskId);

    timer_data_t* td;
    if (td = cl_timer_find(task->taskId)){
        uv_timer_stop(td->timer);
        uv_close((uv_handle_t*)td->timer, close_cb);
        delete td;
        cl_timer_del(task->taskId);
    }

    // notify
    uv_sem_post(&task->notify);

    return;
}



static void timer_cb(uv_timer_t* handle)
{
    timer_data_t* data = (timer_data_t*)handle->data;

    cl_timer_del(data->task->taskId);

    // if (data->task->type == async_task_type::CONNECT)
    // {
    //     conn_task_t* connTask = (conn_task_t*)data->task;
    //     conn_req_t* reqData = (conn_req_t*)data->req->data;
    //     uv_close((uv_handle_t*)reqData->conn->tcp.handle, close_cb);
    //     delete reqData->conn;
    //     delete reqData;
    // }

    uv_close((uv_handle_t*)handle, close_cb);
    
    data->task->err = ERR_TIMEOUT;
    _finish_task(data->task);
    delete data->timer;
    //delete data->req;
    delete data;

    data->task = NULL;
    //data->req = NULL;

    return;
}


static void write_cb(uv_write_t* req, int status)
{
    int ret = 0;

    abs_task_t* task = (abs_task_t*)req->data;

    switch (task->type)
    {
    case RW:
        if (!status)
        {
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
            //td->req = NULL; // 这个req不需要，但是必须置为NULL

            // 定时器放到timer队列
            cl_timer_add(task->taskId, td);
        }
        break;

    case PUSH:
        delete req->data;       // 推送任务要释放
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
  
    int err = threadpool_push_work(svrLoop->pool, task, svr_work_cb, NULL);
    while (err == -1)
    {// 队列已满，再次继续试，概率较小
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
        if (!err){ 
            // LOG;
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

                task->common.err = 0;
                task->outdata.assign((char*)taskList[i]->data, taskList[i]->length);
                timer_data_t* td = cl_timer_del(taskList[i]->taskId);
                if (td)
                {
                    uv_timer_stop(td->timer);
                    uv_close((uv_handle_t*)td->timer, close_cb);
                    delete td;
                }
                
                _finish_task((abs_task_t*)task);
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
        for (int i = 0; i < taskList.size(); i++)
        {
            if (taskList[i]->type == PKG_TYPE_COMMON)
            {
                threadpool_push_work(,);
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
        uv_close((uv_handle_t*)stream, close_cb);

        loop_type_t type = loop_type(stream->loop);
        if (type == CLIENT_LOOP){
            cl_conn_del2(conn);
        }
        else
        {
            sl_conn_del2(conn);
        }

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
        uv_close((uv_handle_t*)&conn->tcp.handle, close_cb);
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
        uv_close((uv_handle_t*)&conn->tcp.handle, close_cb);
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
    else{ 
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
    conn_task_t* task;
    tcp_conn_t* conn;
}conn_req_t;


// 客户端连接回调
void connect_cb(uv_connect_t* req, int status)
{
    conn_req_t* reqData = (conn_req_t*)req->data;
    conn_task_t* task = reqData->task;

    if (status != 0)
    {
        uv_close((uv_handle_t*)req->handle, close_cb);
        task->common.err = uverr_convert(status);
        // 释放tcp_conn_t
        delete reqData->conn;
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

    _finish_task((abs_task_t*)task);

    return;
}


static int _do_connect(abs_task_t* task)
{
    int err;
    //comm_tcp_t conn;
    conn_task_t* ct = (conn_task_t*)task;

    conn_req_t* reqData = new conn_req_t;
    reqData->conn = new tcp_conn_t;
    reqData->task = ct;
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
        uv_close((uv_handle_t*)&reqData->conn->tcp.handle, close_cb);
        delete reqData->conn;
        delete reqData;
        return -1;
    }

    // 计时器关闭时如何释放需要考虑
    // timer_data_t* td = new timer_data_t;
    // uv_timer_t* timer = new uv_timer_t;
    // uv_timer_init(reqData->conn->tcp.handle->loop, timer);
    // uv_timer_start(timer, timer_cb, ct->timeout, 0);
    // td->task = (abs_task_t*)ct;
    // td->timer = timer;
    // td->req = (uv_req_t*)req;
    // timer->data = td;

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
    _finish_task(task);
    return;
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
        tcp_conn_t* conn = cl_conn_find(rt->connId);
        if (!conn)
        {
            task->err = ERR_CONN_NOT_EXIST;
            _finish_task(task);
        }
        if (_do_write(conn, rt->indata, task) != 0)
        {
            _finish_task(task);
        }
        break;
    }
    case async_task_type::RESP:
    {
        // 没有这个异步请求吧？
        break;
    }
    case async_task_type::PUSH:
    {
        _do_push(task);
        _finish_task(task);
        break;
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

