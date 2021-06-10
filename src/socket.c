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

typedef enum
{
  SocketCreateIsServer = 1 << 0,
  SocketCreateNonBlocking = 1 << 1,
} SocketCreateFlags;

static ipcError _socket_create(Socket* sock, char* name, int is_server)
{
  sock->name = strdup(name);
  if (is_server)
    sock->type = ipcSocketTypeServer;
  else
    sock->type = ipcSocketTypeClient;
#ifdef _WIN32
  if (is_server)
  {
    sock->server =
        CreateNamedPipe(name, PIPE_ACCESS_DUPLEX,
                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                        PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
  }
  else
  {
    while (WaitNamedPipe(name, INFINITE) == 0)
      ;
    sock->server = CreateFile(name, GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  }
  sock->client = sock->server;
  if (sock->server == INVALID_HANDLE_VALUE)
    return ipcErrorHandleInvalid;

#else
  struct sockaddr_un sock_name = {.sun_family = PF_LOCAL, .sun_path = {0}};
  strncpy(sock_name.sun_path, name, sizeof(sock_name.sun_path));

  if ((sock->server = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
    return ipcErrorSocketOpen;

  if (is_server)
  {
    unlink(sock->name);
    if (bind(sock->server, (struct sockaddr*)&sock_name, SUN_LEN(&sock_name)) !=
        0)
      return ipcErrorSocketCreate;
    listen(sock->server, 10); // TODO: sock->max_connections
  }
  else
  {
    if ((sock->client = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
      return ipcErrorSocketOpen;
    while (connect(sock->server, (struct sockaddr*)&sock_name,
                   SUN_LEN(&sock_name)) != 0)
      ;
  }
#endif

  return ipcErrorNone;
}

ipcError socket_create(Socket* sock, char* name)
{
  return _socket_create(sock, name, 1);
}

ipcError socket_connect(Socket* sock, char* name, SocketFlags flags)
{
  if (!sock->name)
    sock->name = strdup(name);
  sock->flags = flags;

#ifdef _WIN32
  if (flags & SocketIsServer)
  {
    sock->type = ipcSocketTypeServer;
    if (!sock->server)
      sock->server = sock->client =
          CreateNamedPipe(name, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                          PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                          PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);

    ConnectNamedPipe(sock->server, &sock->_overlap);
    switch (GetLastError())
    {
    case ERROR_IO_PENDING:
      return ipcErrorSocketConnect;
    case ERROR_PIPE_CONNECTED:
      return ipcErrorNone;
    default:
      return ipcErrorUnknown;
    }
  }
  else
  {
    sock->type = ipcSocketTypeClient;
    if (WaitNamedPipe(name, 0) == 0)
      return ipcErrorSocketConnect;
    sock->server = sock->client =
        CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                   OPEN_EXISTING, 0, NULL);
  }
#else
#endif

  return ipcErrorNone;
}

ipcError socket_connect_wait(Socket *sock, char *name, SocketFlags flags, int sleep_ms)
{
#ifdef _WIN32
  while (socket_connect(sock, name, flags) != ipcErrorNone)
    Sleep(sleep_ms);
#else
#endif

  return ipcErrorNone;
}

/* ipcError socket_connect(Socket* sock, char* name) */
/* { */
/*   return _socket_create(sock, name, 0); */
/* } */

ipcError socket_wait_for_connect(Socket* sock)
{
#ifdef _WIN32
  if (sock->type == ipcSocketTypeServer)
    ConnectNamedPipe(sock->server, NULL);
#else
  if (sock->type == ipcSocketTypeServer)
    sock->client = accept(sock->server, NULL, NULL);
#endif
  return ipcErrorNone;
}

ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write)
{
  SocketHandle handle;
  if (sock->type == ipcSocketTypeServer)
    handle = sock->client;
  else
    handle = sock->server;
#ifdef _WIN32
  while (1)
  {
    WriteFile(handle, buffer, bytes_to_write, NULL, NULL);
    if (GetLastError() != ERROR_IO_PENDING)
      break;
  }
#else
  write(handle, buffer, bytes_to_write);
#endif
  return ipcErrorNone;
}

ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read)
{
  SocketHandle handle;
  if (sock->type == ipcSocketTypeServer)
    handle = sock->client;
  else
    handle = sock->server;
#ifdef _WIN32
  while (1)
  {
    ReadFile(handle, buffer, bytes_to_read, NULL, NULL);
    if (GetLastError() != ERROR_IO_PENDING)
      break;
  }
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
