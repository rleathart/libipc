#include <ipc/pipe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <errors.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

ipcError pipe_create(Pipe *pipe, char *name) {
  pipe->_name = strdup(name);
#ifdef _WIN32
  pipe->_handle =
      CreateNamedPipe(name, PIPE_ACCESS_DUPLEX,
                      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                      PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
  if (pipe->_handle == INVALID_HANDLE_VALUE)
    return ipcErrorHandleInvalid;

  DWORD lasterr = GetLastError();
  if (lasterr == ERROR_ALREADY_EXISTS)
    return ipcErrorSocketAlreadyExists;
#elif defined DOTNET_PIPES
  if ((pipe->_handle = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    return ipcErrorSocketOpen;

  struct sockaddr_un sock = {.sun_family = AF_LOCAL, .sun_path = {0}};
  strncpy(sock.sun_path, name, sizeof(sock.sun_path));

  if (bind(pipe->_handle, (struct sockaddr *)&sock, SUN_LEN(&sock)) != 0)
    return ipcErrorSocketCreate;
  listen(pipe->_handle, 10); // TODO: Change 10 to something else
#endif

  return ipcErrorNone;
}

ipcError pipe_connect(Pipe *pipe, char *name) {
  pipe->_name = strdup(name);
#ifdef _WIN32
  pipe->_handle = CreateFile(name, GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (pipe->_handle == INVALID_HANDLE_VALUE)
    return ipcErrorHandleInvalid;
  if (GetLastError() == ERROR_FILE_NOT_FOUND)
    return ipcErrorSocketDoesntExist;
#elif defined DOTNET_PIPES
  if ((pipe->_handle = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return ipcErrorSocketOpen;
  }

  struct sockaddr_un sock = {.sun_family = AF_LOCAL, .sun_path = {0}};
  strncpy(sock.sun_path, name, sizeof(sock.sun_path));

  if (connect(pipe->_handle, (struct sockaddr *)&sock, SUN_LEN(&sock)) != 0)
    return ipcErrorSocketConnect;
#endif

  return ipcErrorNone;
}

ipcError pipe_destroy(Pipe *pipe) {
  ipcError err = ipcErrorNone;
#ifdef _WIN32
  if (!DisconnectNamedPipe(pipe->_handle))
    err = ipcErrorSocketClose;
#else
  if (remove(pipe->_name) != 0)
    err = ipcErrorFileRemove;
#endif

  if (pipe->_name) {
    free(pipe->_name);
    pipe->_name = NULL;
  }

  return err;
}

ipcError pipe_read_bytes(Pipe pipe, void *buffer, size_t bytes_to_read) {
  long long bytes_read = 0;
#ifdef _IN32
  ReadFile(pipe._handle, buffer, bytes_to_read, (LPDWORD)&bytes_read, NULL);
#else
  bytes_read = read(pipe._handle, buffer, bytes_to_read);
#endif
  if (bytes_read != bytes_to_read)
    return ipcErrorActualNeqExpected;
  return ipcErrorNone;
}

ipcError pipe_write_bytes(Pipe pipe, void *buffer, size_t bytes_to_write) {
  long long bytes_written = 0;
#ifdef _WIN32
  WriteFile(pipe._handle, buffer, bytes_to_write, (LPDWORD)&bytes_written,
            NULL);
#else
  bytes_written = write(pipe._handle, buffer, bytes_to_write);
#endif

  if (bytes_written != bytes_to_write)
    return ipcErrorActualNeqExpected;
  return ipcErrorNone;
}
