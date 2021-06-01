#pragma once

typedef enum {
  ipcErrorNone,
  ipcErrorSocketOpen,
  ipcErrorSocketConnect,
  ipcErrorHandleNull,
} ipcError;

char *ipcError_str(ipcError e);
