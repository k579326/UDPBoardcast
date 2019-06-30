


#pragma once

#include "sysheader.h"

void listen_cb(uv_stream_t* server, int status);

void connect_cb(uv_connect_t* req, int status);

void async_cb(uv_async_t* handle);












