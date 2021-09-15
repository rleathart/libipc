#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <afunix.h>

#define SYSTEMTIME_SECONDS(st) st.wSecond
#define SYSTEMTIME_MS(st) st.wMilliseconds

inline void socket_platform_init(void)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}
