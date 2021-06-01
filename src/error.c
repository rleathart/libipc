#include <ipc/error.h>

char *ipcError_str(ipcError e) {
  switch (e) {
  case ipcErrorNone:
    return "No error";
  case ipcErrorSocketOpen:
    return "Could not open socket";
  case ipcErrorSocketConnect:
    return "Could not connect to socket";
  case ipcErrorSocketCreate:
    return "Could not create socket";
  case ipcErrorHandleNull:
    return "Returned handle was NULL";
  case ipcErrorFileRemove:
    return "Error removing file";
  default:
    return "Unknown error";
  }
}
