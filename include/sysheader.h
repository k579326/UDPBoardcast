

#pragma once


#ifdef _WIN32
#include <WS2tcpip.h>
#include <Windows.h>

#ifndef DEPRECATED
#define DEPRECATED __declspec(deprecated)
#endif

#else
#include <fcntl.h>
#include <unistd.h>
#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#endif

#include "uv.h"






