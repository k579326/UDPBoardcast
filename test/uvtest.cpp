
#include <stdio.h>

#include "uv.h"

void listen_cb(uv_stream_t* server, int status)
{
    uv_tcp_t client;
    uv_accept(server, (uv_stream_t*)&client);

    sockaddr_in addr;
    int size = sizeof(addr);

    uv_tcp_getpeername(&client, (sockaddr*)&addr, &size);

    printf("[accept client] %d.%d.%d.%d\n", (char*)(&addr.sin_addr.S_un.S_addr)[0], (char*)(&addr.sin_addr.S_un.S_addr)[1], 
        (char*)(&addr.sin_addr.S_un.S_addr)[2], (char*)(&addr.sin_addr.S_un.S_addr)[3]);

    return;
}


int server()
{
    uv_loop_t loop;
    uv_loop_init(&loop);
    
    uv_tcp_t handle;
    
    sockaddr_in addr;
    uv_ip4_addr("192.168.0.229", 10038, &addr);

    uv_tcp_init_ex(&loop, &handle, AF_INET);
    
    uv_tcp_bind(&handle, (sockaddr*)&addr, 0);
    uv_listen((uv_stream_t*)&handle, 1, listen_cb);
    
    while (1)
    {
        uv_run(&loop, UV_RUN_ONCE);
        Sleep(500);
    }

    getchar();
    uv_loop_close(&loop);

    return 0;
}

void conn_cb(uv_connect_t* req, int status)
{

}


int client()
{
    uv_loop_t loop;
    uv_loop_init(&loop);

    uv_tcp_t handle;

    sockaddr_in addr;
    uv_ip4_addr("192.168.0.229", 10038, &addr);

    uv_tcp_init_ex(&loop, &handle, AF_INET);

    uv_connect_t ct;
    uv_tcp_connect(NULL, &handle, (sockaddr*)&addr, conn_cb);


    uv_run(&loop, UV_RUN_ONCE);


    uv_loop_close(&loop);

    return 0;
}


int main()
{
    server();

    return 0;
}

