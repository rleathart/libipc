#include <ipc/socket.h>
#include <string.h>

#if !_WIN32
#include <unistd.h>

void Sleep(int ms)
{
  struct timeval tv = {
    .tv_sec = ms / 1000,
    .tv_usec = (ms % 1000) * 1000,
  };

  select(0, 0, 0, 0, &tv);
}
#endif

#include "util.h"

char payload[] = "Hello, World!";

enum
{
  ConnectTimeout = 100,
};

thread_rv_t server()
{
  char buffer[512] = {};
  ipcError err = 0;

  Socket sock;
  socket_init(&sock, test_socket_name, SocketServer);

  err = socket_connect(&sock, ConnectTimeout);

  if (err)
    ipcLog(err);

  err = socket_write(&sock, payload, sizeof(payload));

  if (err)
    ipcLog(err);

  err = socket_read(&sock, buffer, sizeof(payload));

  if (err)
    ipcLog(err);

  Assert(strcmp(payload, buffer) == 0);

  return (thread_rv_t)err;
}

thread_rv_t client()
{
  char buffer[512] = {};
  ipcError err = 0;

  Socket sock;
  socket_init(&sock, test_socket_name, 0);

  err = socket_connect(&sock, ConnectTimeout);

  if (err)
    ipcLog(err);

  err = socket_read(&sock, buffer, sizeof(payload));

  if (err)
    ipcLog(err);

  Assert(strcmp(payload, buffer) == 0);

  memset(buffer, 0, sizeof(buffer));

  err = socket_write(&sock, payload, sizeof(payload));

  if (err)
    ipcLog(err);

  return (thread_rv_t)err;
}

int main(int argc, char** argv)
{
  unlink(test_socket_name);
  thread_t threads[2];
  create_thread(&threads[1], client, 0);
  Sleep(20);
  create_thread(&threads[0], server, 0);

  unsigned long err[2];

  for (int i = 0; i < 2; i++)
    join_thread(threads[i], &err[i]);

  unlink(test_socket_name);
  return err[0] || err[1];
}
