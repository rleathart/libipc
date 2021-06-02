#pragma once

#include <ipc/error.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#endif

#if !defined(DOTNET_PIPES) && !defined(_WIN32)
#error Pipes not implemented
#endif

#ifdef _WIN32
typedef HANDLE PipeHandle;
#else
typedef int PipeHandle;
#endif

typedef struct {
  PipeHandle const handle;
  char * const name;
} Pipe;

Pipe     pipe_create(char *name, ipcError *err);
Pipe     pipe_connect(char *name, ipcError *err);
ipcError pipe_destroy(Pipe *pipe);
ipcError pipe_write_bytes(Pipe pipe, void *buffer, size_t bytes_to_write);
ipcError pipe_read_bytes(Pipe pipe, void *buffer, size_t bytes_to_read);
