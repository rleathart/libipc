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
} ipcTime;

// Does platform specific setup (e.g. WSAStartup)
void socket_platform_init(void);

// Populates an ipcTime structure with the number of seconds + microseconds
// since unix epoch (UTC).
void ipc_get_utc_time(ipcTime*);
