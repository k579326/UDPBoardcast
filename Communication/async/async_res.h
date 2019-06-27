



#pragma once

#include "sysheader.h"

typedef struct _async_res_t async_res_t;

async_res_t* init_async_res(uv_loop_t* loop, uint8_t size, uv_async_cb cb);
async_t* pick_async_res(async_res_t* res);
int back_async_res(async_res_t* res, async_t* async);
int clean_async_res(async_res_t* res);


