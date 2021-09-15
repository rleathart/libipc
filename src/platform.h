#pragma once

#if _WIN32
#include "win32/platform.h"
#else
#include "unix/platform.h"
#endif

typedef struct
{
  size_t sec;
  size_t usec;
} ipcTimeOfDay;

// Does platform specific setup (e.g. WSAStartup)
void socket_platform_init(void);

// Populates an ipcTimeOfDay structure with the current UTC time (usec pres.)
void ipc_get_utc_time(ipcTimeOfDay*);
