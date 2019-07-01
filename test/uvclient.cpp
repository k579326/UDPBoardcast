
#include <stdio.h>

#include "uv.h"


void alloc_cb(uv_handle_t* handle,
              size_t suggested_size,
              uv_buf_t* buf)
{
    int x = 0;
    int y = x;

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


}


int client()
{
    uv_loop_t loop;
    uv_loop_init(&loop);

    uv_tcp_t handle;

    sockaddr_in addr;
    uv_ip4_addr("192.168.52.1", 10038, &addr);

    uv_tcp_init_ex(&loop, &handle, AF_INET);

    uv_connect_t ct;
    uv_tcp_connect(&ct, &handle, (sockaddr*)& addr, conn_cb);
    //uv_close((uv_handle_t*)&handle, NULL);
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

