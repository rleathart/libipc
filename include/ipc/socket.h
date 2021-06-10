#pragma once

#include <ipc/error.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

typedef enum
{
  ipcSocketTypeServer,
  ipcSocketTypeClient,
} ipcSocketType;

typedef enum
{
  SocketNoFlags = 0,
  SocketIsServer = 1 << 0,
  SocketNonBlocking = 1 << 1,
} SocketFlags;

// Socket should provide
//  - Local IPC
//  - Bidirectional one to one transport

#ifdef _WIN32
typedef HANDLE SocketHandle;
#else
typedef int SocketHandle;
#endif

typedef struct
{
  char* name;
  ipcSocketType type; // Deprecated?
  int is_server;
  SocketFlags flags;
  SocketHandle server;
  SocketHandle client;
#ifdef _WIN32
  OVERLAPPED _overlap;
#endif
} Socket;

ipcError socket_create(Socket* sock, char* name);
ipcError socket_connect(Socket* sock, char* name, SocketFlags flags);
ipcError socket_connect_wait(Socket* sock, char* name, SocketFlags flags, int sleep_ms);
ipcError socket_destroy(Socket* sock);
ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write);
ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read);
