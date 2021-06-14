#pragma once

typedef enum
{
  ipcErrorNone,
  ipcErrorSocketOpen,
  ipcErrorSocketConnect,
  ipcErrorSocketCreate,
  ipcErrorSocketClose,
  ipcErrorSocketDoesntExist,
  ipcErrorSocketAlreadyExists,
  ipcErrorSocketHasMoreData,
  ipcErrorHandleInvalid,
  ipcErrorFileRemove,
  ipcErrorActualNeqExpected,
  ipcErrorNoBytesToRead,
  ipcErrorUnknown, // Leave this at the end, we use it as an errno offset
} ipcError;

char* ipcError_str(ipcError e);
