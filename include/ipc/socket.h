#pragma once

#include <ipc/error.h>
#include <stdlib.h>
#include <stdbool.h>

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
  SocketReading = 1 << 3,
  SocketWriting = 1 << 4,
} SocketStateFlags;

typedef struct
{
  SocketStateFlags flags;
  size_t bytes_read;    // Bytes read in this IO transaction
  size_t bytes_written; // Bytes written in this IO transaction
#ifdef _WIN32
  OVERLAPPED overlap;
  OVERLAPPED overlap_read;
  OVERLAPPED overlap_write;
#endif
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
} Socket;

void socket_init(Socket* sock, char* name, SocketFlags flags);
ipcError socket_connect(Socket* sock);
ipcError socket_disconnect(Socket* sock);
ipcError socket_destroy(Socket* sock);
ipcError socket_write_bytes(Socket* sock, void* buffer, size_t bytes_to_write);
ipcError socket_read_bytes(Socket* sock, void* buffer, size_t bytes_to_read);
bool socket_is_connected(Socket* sock);
