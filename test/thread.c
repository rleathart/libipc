#include <ipc/thread.h>

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#define THREAD_CAST(type) (type)
#endif

struct TestThreadStruct
{
  int x;
  char* name;
};

ThreadReturn test_thread(void* _arg)
{
  struct TestThreadStruct arg = *(struct TestThreadStruct*)_arg;
  printf("Hello, World!\n");
  printf("%d, %s\n", arg.x, arg.name);
  return (ThreadReturn)15;
}

int main(void)
{
  Thread thread;
  struct TestThreadStruct data = {
      .x = 42,
      .name = strdup("This is a test"),
  };
  thread_create(&thread, test_thread, &data);
  printf("main\n");
  thread_wait(&thread);

  printf("%d\n", THREAD_CAST(int) thread.rv);
  return 0;
}
