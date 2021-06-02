#include <ipc/pipe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <errors.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

Pipe pipe_create(char *name, ipcError *err) {
  PipeHandle handle;
  *err = ipcErrorNone;
#ifdef _WIN32
  handle = CreateNamedPipe(name, PIPE_ACCESS_DUPLEX,
      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
      PIPE_UNLIMITED_INSTANCES,
      512, 512,0, NULL
      );
  if(handle == INVALID_HANDLE_VALUE)
    *err = ipcErrorHandleInvalid;

  DWORD lasterr = GetLastError();
  if (lasterr == ERROR_ALREADY_EXISTS)
    *err = ipcErrorSocketAlreadyExists;
#elif defined DOTNET_PIPES
  if ((handle = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    *err = ipcErrorSocketOpen;

  struct sockaddr_un sock = {.sun_family = AF_LOCAL, .sun_path = {0}};
  strncpy(sock.sun_path, name, sizeof(sock.sun_path));

  if (bind(handle, (struct sockaddr *)&sock, sizeof(sock)) != 0)
    *err = ipcErrorSocketCreate;
#endif

  Pipe pipe = {
    .handle = handle,
    .name = name
  };

  return pipe;
}

Pipe pipe_connect(char *name, ipcError *err) {
  PipeHandle handle;
  *err = ipcErrorNone;
#ifdef _WIN32
  handle = CreateFile(name, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (GetLastError() == ERROR_FILE_NOT_FOUND)
    *err = ipcErrorSocketDoesntExist;
#elif defined DOTNET_PIPES
  if ((handle = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    *err = ipcErrorSocketOpen;

  struct sockaddr_un sock = {.sun_family = AF_LOCAL, .sun_path = {0}};
  strncpy(sock.sun_path, name, sizeof(sock.sun_path));

  if (connect(handle, (struct sockaddr *)&sock, sizeof(sock)) != 0)
    *err = ipcErrorSocketConnect;
#endif

  Pipe pipe = {
    .handle = handle,
    .name = name
  };

  return pipe;
}

ipcError pipe_destroy(Pipe *pipe) {
#ifdef _WIN32
  if (!DisconnectNamedPipe(pipe->handle))
    return ipcErrorSocketClose;
#else
  if (remove(pipe->name) != 0)
    return ipcErrorFileRemove;
#endif

  return ipcErrorNone;
}

ipcError pipe_read_bytes(Pipe pipe, void *buffer, size_t bytes_to_read) {
  size_t bytes_read = 0;
#ifdef _WIN32
  ReadFile(pipe.handle, buffer, bytes_to_read, (LPDWORD)&bytes_read, NULL);
#else
  bytes_read = read(pipe.handle, buffer, bytes_to_read);
#endif
  return ipcErrorNone;
}

ipcError pipe_write_bytes(Pipe pipe, void *buffer, size_t bytes_to_write) {
  size_t bytes_written = 0;
#ifdef _WIN32
  WriteFile(pipe.handle, buffer, bytes_to_write, (LPDWORD)&bytes_written, NULL);
#else
  bytes_written = write(pipe.handle, buffer, bytes_to_write);
#endif
  return ipcErrorNone;
}
