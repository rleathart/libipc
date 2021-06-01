#pragma once

#include <ipc/error.h>
#include <stddef.h>

#if !defined(DOTNET_PIPES) && !defined(_WIN32)
#error Pipes not implemented
#endif

#ifdef _WIN32
typedef HANDLE PipeHandle;
#else
typedef int PipeHandle;
#endif

typedef struct {
  PipeHandle handle;
  char *name;
} Pipe;

ipcError pipe_create(Pipe *pipe, char *name);
ipcError pipe_destroy(Pipe *pipe);
ipcError pipe_write_bytes(Pipe pipe, void *buffer, size_t bytes_to_write);
ipcError pipe_read_bytes(Pipe pipe, void *buffer, size_t bytes_to_read);
