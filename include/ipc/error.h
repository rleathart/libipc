#pragma once

typedef enum {
  ipcErrorNone,
  ipcErrorUnknown,
  ipcErrorSocketOpen,
  ipcErrorSocketConnect,
  ipcErrorSocketCreate,
  ipcErrorSocketClose,
  ipcErrorSocketDoesntExist,
  ipcErrorSocketAlreadyExists,
  ipcErrorHandleInvalid,
  ipcErrorFileRemove,
} ipcError;

char *ipcError_str(ipcError e);
