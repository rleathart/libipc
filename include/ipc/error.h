#pragma once

typedef enum {
  ipcErrorNone,
  ipcErrorSocketOpen,
  ipcErrorSocketConnect,
  ipcErrorSocketCreate,
  ipcErrorHandleNull,
  ipcErrorFileRemove,
} ipcError;

char *ipcError_str(ipcError e);
