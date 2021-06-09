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
  ipcSocketType type;
  SocketHandle server;
  SocketHandle client;
} Socket;

ipcError socket_create(Socket* sock, char* name);
ipcError socket_connect(Socket* sock, char* name);
ipcError socket_destroy(Socket* sock);
ipcError socket_wait_for_connect(Socket* sock);
ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write);
ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read);
