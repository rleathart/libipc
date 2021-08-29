#pragma once

typedef enum
{
  ipcErrorNone,
  ipcErrorTimeout,
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

  // We use the last two bits of this enum as flags that idicate whether we are
  // returning a C library error or a Win32 API error. This means you can do
  // things like
  // return errno | ipcErrorIsErrnoError;
  // and then strerror(err & ~ipcErrorIsErrnoError);
  ipcErrorIsWin32 = 1 << (sizeof(int) * 8 - 2),
  ipcErrorIsErrno = 1 << (sizeof(int) * 8 - 1),
} ipcError;

/// @return Integer error code with indicator flags removed.
ipcError ipcError_int(ipcError e);
char* ipcError_str(ipcError e);
