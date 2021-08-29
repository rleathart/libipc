#include <ipc/socket.h>

#include "util.h"

char* payload = "Hello, World!";
char buffer[512] = {};

thread_rv_t server()
{
  Sleep(1000);
  ipcError err = 0;

  Socket sock;
  socket_init(&sock, test_socket_name, SocketServer);

  err = socket_connect(&sock, 200);

  if (err)
    ipcLog(err);

  err = socket_write_bytes(&sock, payload, strlen(payload) + 1);

  if (err)
    ipcLog(err);

  err = socket_read_bytes(&sock, buffer, sizeof(buffer));

  if (err)
    ipcLog(err);

  Assert(strcmp(payload, buffer) == 0);

  return err;
}

thread_rv_t client()
{
  ipcError err = 0;

  Socket sock;
  socket_init(&sock, test_socket_name, 0);

  err = socket_connect(&sock, 200);

  if (err)
    ipcLog(err);

  err = socket_read_bytes(&sock, buffer, sizeof(buffer));

  if (err)
    ipcLog(err);

  Assert(strcmp(payload, buffer) == 0);

  memset(buffer, 0, sizeof(buffer));

  err = socket_write_bytes(&sock, payload, strlen(payload) + 1);

  if (err)
    ipcLog(err);

  return err;
}

int main(int argc, char** argv)
{
  unlink(test_socket_name);
  thread_t threads[2];
  create_thread(&threads[0], server, 0);
  create_thread(&threads[1], client, 0);

  unsigned long err[2];

  for (int i = 0; i < 2; i++)
    join_thread(threads[i], &err[i]);

  return err[0] || err[1];
}
