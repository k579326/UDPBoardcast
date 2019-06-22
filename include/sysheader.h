

#pragma once




#ifdef _WIN32
#include <WS2tcpip.h>
#include <Windows.h>

#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "uv.h"
