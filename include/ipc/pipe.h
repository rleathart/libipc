#pragma once

#include <ipc/error.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#include <ipc/ipc_exports.h>
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
  PipeHandle handle;
  char *name;
} Pipe;

IPC_EXPORT ipcError pipe_create(Pipe *pipe, char *name);
IPC_EXPORT ipcError pipe_destroy(Pipe *pipe);
IPC_EXPORT ipcError pipe_write_bytes(Pipe pipe, void *buffer, size_t bytes_to_write);
IPC_EXPORT ipcError pipe_read_bytes(Pipe pipe, void *buffer, size_t bytes_to_read);
