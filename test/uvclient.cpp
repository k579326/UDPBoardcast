
#include <stdio.h>

#include "uv.h"

void close_cb(uv_handle_t* handle)
{
    int x = 0;
    int y = x;
}
void alloc_cb(uv_handle_t* handle,
              size_t suggested_size,
              uv_buf_t* buf)
{
    buf->len = suggested_size;
    buf->base = (char*)malloc(suggested_size);
}
void read_cb(uv_stream_t* stream,
             ssize_t nread,
             const uv_buf_t* buf)
{
    char buff[64] = { 0 };

    memcpy(buff, buf->base, nread);

    printf("[read buf] %s \n", buf->base);

    uv_read_stop(stream);
    uv_read_start(stream, alloc_cb, read_cb);
}

void conn_cb(uv_connect_t* req, int status)
{
    int x = 0;
    int y = x;

    uv_read_start(req->handle, alloc_cb, read_cb);

    //uv_close((uv_handle_t*)param, close_cb);
}
void async_cb(uv_async_t* handle)
{

}

void thread_func(void* param)
{
    Sleep(1000);
    //uv_close((uv_handle_t*)param, close_cb);

    //uv_async_t async;
    //uv_async_init(((uv_handle_t*)param)->loop, &async, async_cb);
    //uv_async_send(&async);
    uv_close((uv_handle_t*)param, close_cb);
    return;
}

void timer_cb(uv_timer_t* handle)
{
    uv_close((uv_handle_t*)handle->data, close_cb);
}
int client()
{
    uv_loop_t loop;
    uv_loop_init(&loop);

    uv_tcp_t handle;

    sockaddr_in addr;
    //uv_ip4_addr("192.168.52.1", 10038, &addr);
    uv_ip4_addr("192.168.0.229", 10038, &addr);
    uv_tcp_init_ex(&loop, &handle, AF_INET);

    uv_connect_t ct;
    uv_tcp_connect(&ct, &handle, (sockaddr*)&addr, conn_cb);
    //uv_close((uv_handle_t*)&handle, NULL);

    uv_thread_t thread;
    uv_thread_create(&thread, thread_func, &handle);

    //uv_timer_t t;
    //uv_timer_init(&loop, &t);
    //uv_timer_start(&t, timer_cb, 1000, 0);
    //t.data = &handle;

    while (1)
    {
        uv_run(&loop, UV_RUN_ONCE);
        //uv_loop_close(&loop);

        Sleep(200);
    }


    uv_loop_close(&loop);

    return 0;
}


int main()
{
    client();

    return 0;
}

