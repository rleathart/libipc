#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <afunix.h>

#include <assert.h>
#include <ipc/socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
   socket_connect
      Only fail if timeout expires
   socket_*_bytes
      Only fail if the socket is not connected
*/

static bool did_winsock_setup;
static WSADATA wsaData;

void socket_init(Socket* sock, char* name, SocketFlags flags)
{
  if (!did_winsock_setup)
    WSAStartup(MAKEWORD(2, 2), &wsaData);

  memset(sock, 0, sizeof(*sock));

  sock->name = strdup(name);
  sock->flags = flags;

  sock->server = sock->client = INVALID_SOCKET;
}

ipcError socket_read(Socket* sock, void* buffer, size_t bytes_to_read)
{
  ipcError err = ipcErrorNone;

  SocketHandle handle =
      (sock->flags & SocketServer) ? sock->client : sock->server;

  int bytes_read = 0;
  while (bytes_read < bytes_to_read)
  {
    int rv = recv(handle, buffer, bytes_to_read - bytes_read, 0);

    if (rv == SOCKET_ERROR && !err)
      err = WSAGetLastError() | ipcErrorIsWin32;

    bytes_read += rv;
  }
  return err;
}

ipcError socket_write(Socket* sock, void* buffer, size_t bytes_to_write)
{
  ipcError err = ipcErrorNone;

  SocketHandle handle =
      (sock->flags & SocketServer) ? sock->client : sock->server;

  int rv = send(handle, buffer, bytes_to_write, 0);

  if (rv < 0 && !err)
    err = WSAGetLastError() | ipcErrorIsWin32;

  return err;
}

ipcError socket_connect(Socket* sock, int timeout)
{
  ipcError err = ipcErrorNone;

  struct sockaddr_un sock_name = {.sun_family = AF_UNIX};
  strncpy(sock_name.sun_path, sock->name, sizeof(sock_name.sun_path));

  if (sock->flags & SocketServer)
  {
    sock->server = socket(AF_UNIX, SOCK_STREAM, 0);
    unlink(sock->name);
    int bind_err =
        bind(sock->server, (struct sockaddr*)&sock_name, sizeof(sock_name));
    listen(sock->server, 10);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sock->server, &rfds);

    struct timeval tv = {
        .tv_sec = 0,
        .tv_usec = 1000 * timeout,
    };

    int sockets_ready = select(sock->server + 1, &rfds, &rfds, 0, &tv);

    if (sockets_ready > 0)
      sock->client = accept(sock->server, NULL, NULL);
    else if (!err)
      err = ipcErrorTimeout;
  }
  else
  {
    sock->server = socket(sock_name.sun_family, SOCK_STREAM, 0);

    SYSTEMTIME st;
    GetSystemTime(&st);
    int start_seconds = st.wSecond;
    int start_ms = st.wMilliseconds;

    int connect_err = 1;
    int elapsed_time = 0; // ms

    while (elapsed_time < timeout && connect_err)
    {
      connect_err = connect(sock->server, (struct sockaddr*)&sock_name,
                                sizeof(sock_name));

      GetSystemTime(&st);
      elapsed_time +=
          (st.wSecond - start_seconds) * 1000 + st.wMilliseconds - start_ms;
    }

    if (connect_err)
      err = ipcErrorTimeout;
  }

  return err;
}