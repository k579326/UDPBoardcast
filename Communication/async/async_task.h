



#pragma once

#include "sysheader.h"

typedef struct{
    uv_async_t async;  
    uv_mutex_t mutex;
    uv_cond_t  cond;
};


int create_async_res(uv_loop_t* loop);
int pick_async_res(uv_async_t* async);
int back_async_res(uv_async_t* async);