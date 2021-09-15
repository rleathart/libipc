#include "../platform.h"

void socket_platform_init(void)
{
}

void ipc_get_utc_time(ipcTimeOfDay* time)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time->sec = tv.tv_sec;
  time->usec = tv.tv_usec;
}
