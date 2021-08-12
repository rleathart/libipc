#include <ipc/socket.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

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

#ifdef _DEBUG
#define SOCKLOG(fmt, ...)                                                      \
  do                                                                           \
  {                                                                            \
    fprintf(stderr, sock->flags& SocketServer ? "Server: " : "Client: ");      \
    fprintf(stderr, fmt, __VA_ARGS__);                                         \
  } while (0)
#else
#define SOCKLOG(fmt, ...)
#endif

void socket_init(Socket* sock, char* name, SocketFlags flags)
{
  memset(sock, 0, sizeof(Socket));
  sock->name = strdup(name);
  sock->flags = flags;
#ifdef _WIN32
  sock->state.overlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
  sock->state.overlap_read.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
  sock->state.overlap_write.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
#endif
}

ipcError socket_disconnect(Socket *sock)
{
#ifdef _WIN32
  DisconnectNamedPipe(sock->server);
#else
  close(sock->server);
  close(sock->client);
#endif
  sock->server = sock->client = 0;
  sock->state.flags &= ~SocketConnected;
  return ipcErrorNone;
}

ipcError socket_connect(Socket* sock)
{
  sock->state.flags &= ~SocketConnected;
  // Need to make sure there's no lingering errno
  errno = 0;

#ifdef _WIN32
  if (sock->flags & SocketServer)
  {
    if (!sock->server)
      sock->server = sock->client =
          CreateNamedPipe(sock->name, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                          PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);

    ConnectNamedPipe(sock->server, &(sock->state.overlap));
    DWORD err;
    switch ((err = GetLastError()))
    {
    case ERROR_IO_PENDING:
      return ipcErrorSocketConnect;
    case ERROR_PIPE_CONNECTED:
      SetEvent(sock->state.overlap.hEvent);
      break;
    case ERROR_NO_DATA:
      socket_disconnect(sock);
      return err | ipcErrorIsWin32;
    default:
      return err | ipcErrorIsWin32;
    }
  }
  else
  {
    if (WaitNamedPipe(sock->name, 0) == 0)
      return ipcErrorSocketConnect;
    sock->server = sock->client =
        CreateFile(sock->name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, 0, NULL);
    if (GetLastError() == ERROR_FILE_NOT_FOUND)
      return ipcErrorSocketOpen;
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
    if (!(sock->state.flags & SocketBound))
    {
      SOCKLOG("Unlinking %s\n", sock->name);
      unlink(sock->name);
      if (bind(sock->server, (struct sockaddr*)&sock_name, sizeof(sock_name)))
      {
        SOCKLOG("bind: %s\n", strerror(errno));
        return ipcErrorSocketCreate;
      }
      sock->state.flags |= SocketBound;
      SOCKLOG("%s\n", "Successfully bound address");
    }
    if (!(sock->state.flags & SocketListening))
    {
      listen(sock->server, 10);
      sock->state.flags |= SocketListening;
      SOCKLOG("%s\n", "Listening");
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

  SOCKLOG("Successfully connected socket with\n\tname: %s\n\tserver: "
          "%" PRId64 "\n\tclient: %" PRId64 "\n",
          sock->name, (int64_t)sock->server, (int64_t)sock->client);

  sock->state.flags |= SocketConnected;

  return ipcErrorNone;
}

static ipcError _socket_transact(Socket* sock, void* buffer, size_t bytes,
                                 int is_write)
{
  ipcError err = 0;
  if (!socket_is_connected(sock))
  {
    err = ipcErrorSocketConnect;
    goto end;
  }

  SocketHandle handle =
      (sock->flags & SocketServer) ? sock->client : sock->server;

  size_t* bytes_out =
      is_write ? &(sock->state.bytes_written) : &(sock->state.bytes_read);

#ifdef _WIN32

  OVERLAPPED* overlap =
      is_write ? &(sock->state.overlap_write) : &(sock->state.overlap_read);
  if (!(sock->state.flags & (is_write ? SocketWriting : SocketReading)))
  {
    if (is_write)
      WriteFile(handle, buffer, bytes, NULL, overlap);
    else
      ReadFile(handle, buffer, bytes, NULL, overlap);
    sock->state.flags |= is_write ? SocketWriting : SocketReading;
  }
  GetOverlappedResult(handle, overlap, (LPDWORD)bytes_out, FALSE);
  DWORD lasterr = GetLastError();
  if (lasterr == ERROR_IO_INCOMPLETE)
    return ipcErrorSocketHasMoreData;
  if (lasterr)
    err = lasterr | ipcErrorIsWin32;

#else

  ssize_t rv;
  if (is_write)
    rv = send(handle, buffer + *bytes_out, bytes - *bytes_out, 0);
  else
    rv = recv(handle, buffer + *bytes_out, bytes - *bytes_out, 0);
  if (rv >= 0)
    *bytes_out += rv;
  if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return ipcErrorSocketHasMoreData;
  if (rv < 0)
    err = errno | ipcErrorIsErrno;

#endif

end:

  if (err != ipcErrorSocketHasMoreData)
  {
    sock->state.flags ^= is_write ? SocketWriting : SocketReading;
    *bytes_out = 0;
  }
  return err;
}

ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write)
{
  return _socket_transact(sock, buffer, bytes_to_write, 1);
}

ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read)
{
  return _socket_transact(sock, buffer, bytes_to_read, 0);
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

bool socket_is_connected(Socket *sock)
{
  if (!(sock->state.flags & SocketConnected))
    return false;
  SocketHandle handle = (sock->flags & SocketServer) ? sock->client : sock->server;
#ifdef _WIN32
  ReadFileEx(handle, NULL, 0, &sock->state.overlap, NULL);
  switch (GetLastError())
  {
  case ERROR_PIPE_NOT_CONNECTED:
  case ERROR_BROKEN_PIPE:
  case ERROR_NO_DATA:
    sock->state.flags &= ~SocketConnected;
  }
#else
  int error = 0;
  socklen_t len = sizeof(error);
  int rv = getsockopt (handle, SOL_SOCKET, SO_ERROR, &error, &len);
  if (rv || error)
    sock->state.flags &= ~SocketConnected;
#endif

  return sock->state.flags & SocketConnected;
}
