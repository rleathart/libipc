#include <ipc/socket.h>
#include <string.h>

#include <stdio.h>

#ifdef _WIN32
#define strdup _strdup
#include <windows.h>
#else
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <unistd.h>
#endif

/* ipcError socket_connect(Socket* sock, char* name, SocketFlags flags) */
/* { */
/*   if (sock->flags & WAITING) */
/*   static Thread; */
/*   if (ThreadAlreadyCreated) */
/*     return threadstatus; */
/*   else */
/*     spawnthread; */
/* } */

void socket_init(Socket* sock, char* name, SocketFlags flags)
{
  memset(sock, 0, sizeof(Socket));
  sock->name = strdup(name);
  sock->flags = flags;
#ifdef _WIN32
  sock->_overlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
#endif
}

ipcError socket_connect(Socket* sock)
{
#ifdef _WIN32
  if (sock->flags & SocketIsServer)
  {
    if (!sock->server)
      sock->server = sock->client =
          CreateNamedPipe(sock->name, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                          PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);

    ConnectNamedPipe(sock->server, &sock->_overlap);
    switch (GetLastError())
    {
    case ERROR_IO_PENDING:
      return ipcErrorSocketConnect;
    case ERROR_PIPE_CONNECTED:
      SetEvent(sock->_overlap.hEvent);
      return ipcErrorNone;
    default:
      return ipcErrorUnknown;
    }
  }
  else
  {
    if (WaitNamedPipe(sock->name, 0) == 0)
      return ipcErrorSocketConnect;
    sock->server = sock->client =
        CreateFile(sock->name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, 0, NULL);
  }
#else
#endif

  return ipcErrorNone;
}

ipcError socket_connect_wait(Socket* sock, char* name, SocketFlags flags,
                             int sleep_ms)
{
  while (socket_connect(sock) != ipcErrorNone)
#ifdef _WIN32
    Sleep(sleep_ms);
#else
#endif

  return ipcErrorNone;
}

ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write)
{
  SocketHandle handle;
  if (sock->flags & SocketIsServer)
    handle = sock->client;
  else
    handle = sock->server;
#ifdef _WIN32
  OVERLAPPED overlap = {
    .hEvent = CreateEvent(NULL, TRUE, TRUE, NULL),
  };
  WriteFile(handle, buffer, bytes_to_write, NULL, &overlap);
  WaitForSingleObject(overlap.hEvent, INFINITE);
#else
  write(handle, buffer, bytes_to_write);
#endif
  return ipcErrorNone;
}

ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read)
{
  SocketHandle handle;
  if (sock->flags & SocketIsServer)
    handle = sock->client;
  else
    handle = sock->server;
#ifdef _WIN32
  OVERLAPPED overlap = {
    .hEvent = CreateEvent(NULL, TRUE, TRUE, NULL),
  };
  ReadFile(handle, buffer, bytes_to_read, NULL, &overlap);
  WaitForSingleObject(overlap.hEvent, INFINITE);
#else
  read(handle, buffer, bytes_to_read);
#endif
  return ipcErrorNone;
}

ipcError socket_destroy(Socket* sock)
{
  ipcError err = ipcErrorNone;
#ifdef _WIN32
  if (!DisconnectNamedPipe(sock->server))
    err = ipcErrorSocketClose;
#else
  if (unlink(sock->name))
    err = ipcErrorFileRemove;
#endif

  if (sock->name)
    free(sock->name);

  sock->name = NULL;

  return err;
}
