#include <WinSock2.h>
#include <afunix.h>

#include "../platform.h"

#include <stdio.h>

void socket_platform_init(void)
{
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void ipc_get_utc_time(ipcTime* time)
{
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  // How many 100 nanosecond intervals since win32 epoch
  ULONGLONG ft100ns = *(ULONGLONG*)&ft;
  // How many microseconds since win32 epoch
  ULONGLONG ft_usec = ft100ns / 10;
  // Convert from win32 epoch (1601) to unix epoch (1970)
  ULONGLONG usec_since_epoch = ft_usec - 116444736000000000LL;

  time->sec = usec_since_epoch / (int)10e6;
  time->usec = usec_since_epoch % (int)10e6;
}
