

#pragma once


#ifdef _WIN32
#include <WS2tcpip.h>
#include <Windows.h>

#ifndef DEPRECATED
#define DEPRECATED __declspec(deprecated)
#endif

#define ssn_sleep(ms) Sleep(ms)

#else

#include <fcntl.h>
#include <unistd.h>
#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#define ssn_sleep(ms) usleep(ms * 1000)

#endif

#include "uv.h"






