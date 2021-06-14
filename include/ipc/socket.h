#pragma once

#include <ipc/error.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

typedef enum
{
  SocketNoFlags = 0,
  SocketServer = 1 << 0,
  SocketNonBlocking = 1 << 1,
} SocketFlags;

typedef enum
{
  SocketBound = 1 << 0,
  SocketListening = 1 << 1,
  SocketConnected = 1 << 2,
} SocketState;

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
  SocketFlags flags;
  SocketHandle server;
  SocketHandle client;
  SocketState state;
#ifdef _WIN32
  OVERLAPPED _overlap;
#endif
} Socket;

void socket_init(Socket* sock, char* name, SocketFlags flags);
ipcError socket_connect(Socket* sock);
ipcError socket_destroy(Socket* sock);
/* ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write); */
/* ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read); */
ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write, size_t* bytes_written);
ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read, size_t* bytes_written);
