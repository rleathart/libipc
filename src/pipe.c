#include <ipc/pipe.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

ipcError pipe_create(Pipe *pipe, char *name) {
#ifdef _WIN32
  pipe->handle = CreateFile(name, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (!pipe->handle)
    return ipcErrorHandleNull;
#elif defined DOTNET_PIPES
  if ((pipe->handle = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    return ipcErrorSocketOpen;

  struct sockaddr_un sock = {.sun_family = AF_LOCAL, .sun_path = {0}};
  strncpy(sock.sun_path, name, sizeof(sock.sun_path));

  int can_bind =
      bind(pipe->handle, (struct sockaddr *)&sock, sizeof(sock)) != 0;
  int can_connect =
      connect(pipe->handle, (struct sockaddr *)&sock, sizeof(sock)) != 0;

  if (!can_bind && !can_connect) {
    return ipcErrorSocketConnect;
  }

#endif

  pipe->name = name;
  return ipcErrorNone;
}

ipcError pipe_destroy(Pipe *pipe) {
  if (remove(pipe->name) != 0)
    return ipcErrorFileRemove;

  return ipcErrorNone;
}

ipcError pipe_read_bytes(Pipe pipe, void *buffer, size_t bytes_to_read) {
  size_t bytes_read = 0;
#ifdef _WIN32
  ReadFile(pipe.handle, buffer, bytes_to_read, &bytes_read, NULL);
#else
  bytes_read = read(pipe.handle, buffer, bytes_to_read);
#endif
  return ipcErrorNone;
}

ipcError pipe_write_bytes(Pipe pipe, void *buffer, size_t bytes_to_write) {
  size_t bytes_written = 0;
#ifdef _WIN32
  WriteFile(pipe.handle, buffer, bytes_to_read, &bytes_written, NULL);
#else
  bytes_written = write(pipe.handle, buffer, bytes_to_write);
#endif
  return ipcErrorNone;
}
