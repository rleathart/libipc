#pragma once

#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

// Win32 uses WSAGetLastError()
#define socket_last_error() (errno | ipcErrorIsErrno)
