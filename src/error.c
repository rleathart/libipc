#include <ipc/error.h>
#include <string.h>

char* ipcError_str(ipcError e)
{
  if (e > ipcErrorUnknown)
    return strerror(-e);

  switch (e)
  {
  case ipcErrorNone:
    return "No error";
  case ipcErrorUnknown:
    return "Unknown error";
  case ipcErrorSocketOpen:
    return "Could not open socket";
  case ipcErrorSocketConnect:
    return "Could not connect to socket";
  case ipcErrorSocketCreate:
    return "Could not create socket";
  case ipcErrorSocketClose:
    return "Could not close socket";
  case ipcErrorHandleInvalid:
    return "Invalid handle";
  case ipcErrorFileRemove:
    return "Error removing file";
  case ipcErrorActualNeqExpected:
    return "Actual value differs from expected";
  case ipcErrorSocketHasMoreData:
    return "Socket has more data to read or write";
  default:
    return strerror(-e);
  }
}
