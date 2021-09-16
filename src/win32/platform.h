#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <afunix.h>

#define socket_last_error() (WSAGetLastError() | ipcErrorIsWin32)
