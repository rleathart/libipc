#pragma once

#ifdef _WIN32
#include <ipc/ipc_exports.h>
#endif

typedef enum {
  ipcErrorNone,
  ipcErrorSocketOpen,
  ipcErrorSocketConnect,
  ipcErrorSocketCreate,
  ipcErrorHandleNull,
  ipcErrorFileRemove,
} ipcError;

IPC_EXPORT char *ipcError_str(ipcError e);
