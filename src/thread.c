#include <ipc/thread.h>

ipcError thread_create(Thread* thread, ThreadFunction func, void* args)
{
  thread->func = func;
#ifdef _WIN32
  thread->id = (ThreadId)_beginthreadex(NULL, 0, func, args, 0, NULL);
  if (thread->id == INVALID_HANDLE_VALUE)
    return ipcErrorHandleInvalid;
#endif

  return ipcErrorNone;
}

ipcError thread_wait(Thread* thread)
{
#ifdef _WIN32
  WaitForMultipleObjectsEx(1, &thread->id, TRUE, 2000, FALSE);
  GetExitCodeThread(thread->id, (LPDWORD)&thread->rv);
#endif

  return ipcErrorNone;
}
