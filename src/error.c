#include <ipc/error.h>

char *ipcError_str(ipcError e) {
  switch (e) {
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
  default:
    return "Unknown error";
  }
}
