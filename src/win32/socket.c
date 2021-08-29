#include <assert.h>
#include <ipc/socket.h>
#include <stdbool.h>
#include <stdio.h>

// All named pipes on windows must start with this.
static char* win32_pipe_prefix = "\\\\.\\pipe\\";

void socket_init(Socket* sock, char* name, SocketFlags flags)
{
  memset(sock, 0, sizeof(*sock));

  assert(strncmp(name, win32_pipe_prefix, strlen(win32_pipe_prefix)) == 0);

  sock->name = strdup(name);
  sock->flags = flags;

  sock->state.overlap.hEvent = CreateEvent(0, TRUE, TRUE, 0);
  sock->state.overlap_read.hEvent = CreateEvent(0, TRUE, TRUE, 0);
  sock->state.overlap_write.hEvent = CreateEvent(0, TRUE, TRUE, 0);
}

ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read)
{
  ipcError err = ipcErrorNone;

  SocketHandle handle =
      (sock->flags & SocketServer) ? sock->server : sock->client;

  DWORD bytes_read = 0;
  BOOL success = ReadFile(handle, buffer, bytes_to_read, &bytes_read, NULL);

  if (!success && !err)
    err = GetLastError() | ipcErrorIsWin32;

  return err;
}

ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write)
{
  ipcError err = ipcErrorNone;

  SocketHandle handle =
      (sock->flags & SocketServer) ? sock->server : sock->client;

  DWORD bytes_written = 0;
  BOOL success =
      WriteFile(handle, buffer, bytes_to_write, &bytes_written, NULL);

  if (!success && !err)
    err = GetLastError() | ipcErrorIsWin32;

  return err;
}

ipcError socket_connect(Socket* sock, int timeout)
{
  ipcError err = ipcErrorNone;

  if (sock->flags & SocketServer)
  {
    sock->server = sock->client =
        CreateNamedPipeA(sock->name, PIPE_ACCESS_DUPLEX,
                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                         PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);

    if (sock->server == INVALID_HANDLE_VALUE && !err)
      err = GetLastError() | ipcErrorIsWin32;

    BOOL success = ConnectNamedPipe(sock->server, NULL);

    if (!success && !err)
      err = GetLastError() | ipcErrorIsWin32;
  }
  else
  {
    SYSTEMTIME st;
    GetSystemTime(&st);

    int start_seconds = st.wSecond;
    int start_ms = st.wMilliseconds;

    int time_elapsed = 0; // ms
    BOOL success = false;

    while (time_elapsed < timeout)
    {
      success = WaitNamedPipeA(sock->name, timeout);
      GetSystemTime(&st);
      time_elapsed +=
          (st.wSecond - start_seconds) * 1000 + st.wMilliseconds - start_ms;
    }

    if (time_elapsed > timeout && !success && !err)
      err = ipcErrorTimeout;

    if (!success && !err)
      err = GetLastError() | ipcErrorIsWin32;

    sock->server = sock->client =
        CreateFileA(sock->name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, 0, NULL);

    if (sock->server == INVALID_HANDLE_VALUE && !err)
      err = GetLastError() | ipcErrorIsWin32;
  }

  return err;
}
