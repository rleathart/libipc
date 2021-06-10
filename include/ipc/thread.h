#pragma once

#include <ipc/error.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
typedef HANDLE ThreadId;
typedef _beginthreadex_proc_type ThreadFunction;
typedef unsigned int ThreadReturn;
#else
#endif

typedef struct
{
  ThreadId id;
  ThreadFunction func; // Might be useful to store this
  ThreadReturn rv;     // Return value from func
} Thread;

ipcError thread_create(Thread* thread, ThreadFunction func, void* args);
ipcError thread_wait(Thread* thread);
