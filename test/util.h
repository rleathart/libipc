#pragma once

#include <stdio.h>
#include <stdlib.h>

#if _WIN32
/* char* test_socket_name = "\\\\.\\pipe\\test_socket.sock"; */

#include <Windows.h>
typedef HANDLE thread_t;
typedef unsigned long thread_rv_t;
#else
/* char* test_socket_name = "test_socket.sock"; */
#include <pthread.h>

typedef pthread_t thread_t;
typedef void* thread_rv_t;
#endif

char* test_socket_name = "test_socket.sock";
typedef thread_rv_t (*thread_fn_t)(void*);

void create_thread(thread_t* thread, thread_fn_t fn, void* args)
{
#if _WIN32
  *thread = CreateThread(0, 0, fn, args, 0, 0);
#else
  pthread_create(thread, 0, fn, args);
#endif
}

void join_thread(thread_t thread, void* return_value)
{
#if _WIN32
  WaitForSingleObject(thread, INFINITE);
  if (return_value)
    GetExitCodeThread(thread, return_value);
#else
  pthread_join(thread, return_value);
#endif
}

#define ipcLog(err) {fprintf(stderr, "%s:%d (%d) %s\n", __FILE__, __LINE__, ipcError_int(err), ipcError_str(err)); abort();}

void _Assert(char* file, int line)
{
  fprintf(stderr, "Assertion failed: %s:%d\n", file, line);          
  abort();
}

#define Assert(exp) if(!(exp)) _Assert(__FILE__, __LINE__)
