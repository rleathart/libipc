#include <ipc/socket.h>
#include <string.h>

#include <stdio.h>

#ifdef _WIN32
#define strdup _strdup
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>

#include <unistd.h>
#endif

#define DEBUG

#ifdef DEBUG
#define SOCKLOG(fmt, ...)                                                      \
  do                                                                           \
  {                                                                            \
    fprintf(stderr, sock->flags& SocketServer ? "Server: " : "Client: ");      \
    fprintf(stderr, fmt, __VA_ARGS__);                                         \
  } while (0)
#else
#define SOCKLOG(fmt, ...)
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
  if (sock->flags & SocketServer)
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
  struct sockaddr_un sock_name = {.sun_family = PF_LOCAL};
  strncpy(sock_name.sun_path, sock->name, sizeof(sock_name.sun_path));

  if (sock->server < 1)
  {
    SOCKLOG("%s\n", "Initalising sock->server...");
    if ((sock->server = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
      return ipcErrorSocketOpen;
    SOCKLOG("sock->server: %d\n", sock->server);
    fcntl(sock->server, F_SETFL, fcntl(sock->server, F_GETFL, 0) | O_NONBLOCK);
    SOCKLOG("%s\n", "Set O_NONBLOCK");
  }

  if (sock->flags & SocketServer)
  {
    SOCKLOG("Unlinking %s\n", sock->name);
    unlink(sock->name);
    if (!(sock->state & SocketBound))
    {
      if (bind(sock->server, (struct sockaddr*)&sock_name, sizeof(sock_name)))
      {
        SOCKLOG("bind: %s\n", strerror(errno));
        return ipcErrorSocketCreate;
      }
      sock->state |= SocketBound;
    }
    SOCKLOG("%s\n", "Successfully bound address");
    if (!(sock->state & SocketListening))
    {
      listen(sock->server, 10);
      sock->state |= SocketListening;
    }

    if (sock->client < 1)
    {
      SOCKLOG("%s\n", "Initalising sock->client");
      sock->client = accept(sock->server, NULL, NULL);
      if (sock->client < 1)
        return ipcErrorSocketConnect;
      SOCKLOG("sock->client: %d\n", sock->client);
      fcntl(sock->client, F_SETFL,
            fcntl(sock->client, F_GETFL, 0) | O_NONBLOCK);
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return ipcErrorSocketConnect;
  }
  else
  {
    if (sock->client < 1)
    {
      SOCKLOG("%s\n", "Initalising sock->client");
      if ((sock->client = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
        return ipcErrorSocketOpen;
      SOCKLOG("sock->client: %d\n", sock->client);
      fcntl(sock->client, F_SETFL,
            fcntl(sock->client, F_GETFL, 0) | O_NONBLOCK);
    }
    SOCKLOG("%s\n", "Attempting connect");
    if (connect(sock->server, (struct sockaddr*)&sock_name, sizeof(sock_name)))
    {
      SOCKLOG("connect: %s\n", strerror(errno));
      return ipcErrorSocketConnect;
    }
    SOCKLOG("sock->server: %d\tsock->client: %d\n", sock->server, sock->client);
  }
#endif

  sock->state |= SocketConnected;

  return ipcErrorNone;
}

ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write,
                            size_t* bytes_written)
{
  SocketHandle handle;
  if (sock->flags & SocketServer)
    handle = sock->client;
  else
    handle = sock->server;
#ifdef _WIN32
  OVERLAPPED overlap = {
      .hEvent = CreateEvent(NULL, TRUE, TRUE, NULL),
  };
  WriteFile(handle, buffer, bytes_to_write, NULL, &overlap);
  GetOverlappedResult(handle, &overlap, (LPDWORD)bytes_written, FALSE);
  DWORD lasterr = GetLastError();
  size_t _written = *bytes_written;
  if (lasterr == ERROR_IO_INCOMPLETE)
    return ipcErrorSocketHasMoreData;
#else
  ssize_t rv =
      send(handle, buffer + *bytes_written, bytes_to_write - *bytes_written, 0);
  if (rv >= 0)
    *bytes_written += rv;
  SOCKLOG("Sent %ld\n", *bytes_written);
  if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return ipcErrorSocketHasMoreData;
  if (rv < 0)
    return -errno;
#endif
  return ipcErrorNone;
}

ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read,
                           size_t* bytes_read)
{
  SocketHandle handle;
  if (sock->flags & SocketServer)
    handle = sock->client;
  else
    handle = sock->server;
#ifdef _WIN32
  OVERLAPPED overlap = {
      .hEvent = CreateEvent(NULL, TRUE, TRUE, NULL),
  };
  ReadFile(handle, buffer, bytes_to_read, NULL, &overlap);
  GetOverlappedResult(handle, &overlap, (LPDWORD)bytes_read, FALSE);
  DWORD lasterr = GetLastError();
  size_t _read = *bytes_read;
  if (lasterr == ERROR_IO_INCOMPLETE)
    return ipcErrorSocketHasMoreData;
#else
  ssize_t rv =
      recv(handle, buffer + *bytes_read, bytes_to_read - *bytes_read, 0);
  if (rv >= 0)
    *bytes_read += rv;
  SOCKLOG("Read %ld\n", *bytes_read);
  if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return ipcErrorSocketHasMoreData;
  if (rv < 0)
    return -errno;
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
