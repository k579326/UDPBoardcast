
#include <stdio.h>

#include "uv.h"

void write_cb(uv_write_t* req, int status)
{
    int x = 0;
    int y = x;


}


void listen_cb(uv_stream_t* server, int status)
{
    uv_loop_t client_loop;
    uv_tcp_t client;

    uv_loop_init(&client_loop);
    uv_tcp_init_ex(&client_loop, &client, 0);

    uv_accept(server, (uv_stream_t*)&client);

    sockaddr_in addr;
    int size = sizeof(addr);

    uv_tcp_getpeername(&client, (sockaddr*)&addr, &size);

    printf("[accept client] %d.%d.%d.%d\n", ((unsigned char*)(&addr.sin_addr.S_un.S_addr))[0], ((unsigned char*)(&addr.sin_addr.S_un.S_addr))[1],
        ((unsigned char*)(&addr.sin_addr.S_un.S_addr))[2], ((unsigned char*)(&addr.sin_addr.S_un.S_addr))[3]);

    uv_write_t req;  
    uv_buf_t buf;
    buf = uv_buf_init("123456", 6);

    //uv_write(&req, (uv_stream_t*)&client, &buf, 1, write_cb);

    uv_run(&client_loop, UV_RUN_ONCE);


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

int main()
{
    server();

    return 0;
}

