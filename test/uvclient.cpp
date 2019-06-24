
#include <stdio.h>

#include "uv.h"

void conn_cb(uv_connect_t* req, int status)
{
    int x = 0;
    int y = x;


    uv_read_start(, );


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

    uv_run(&loop, UV_RUN_ONCE);

    while (1)
    {
        uv_run(&loop, UV_RUN_ONCE);
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

