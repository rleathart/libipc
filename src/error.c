#include <ipc/error.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static char* get_win32_error_str(DWORD err)
{
  char* buffer;
  DWORD bufLen = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0,
      NULL);
  // win32 error string has newline at the end?
  buffer[strlen(buffer) - 1] = '\0';
  return buffer;
}
#endif

ipcError ipcError_int(ipcError e)
{
  return e & ~(ipcErrorIsWin32 | ipcErrorIsErrno);
}

char* ipcError_str(ipcError e)
{
  if (e & ipcErrorIsErrno)
    return strerror(ipcError_int(e));

#ifdef _WIN32
  if (e & ipcErrorIsWin32)
    return get_win32_error_str(ipcError_int(e));
#endif

  switch (e)
  {
  case ipcErrorNone:
    return "No error";
  case ipcErrorUnknown:
    return "Unknown error";
  case ipcErrorTimeout:
    return "Opertaion timed out";
  case ipcErrorIOPending:
    return "IO operation is pending";
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
    return "Unknown error";
  }
}
