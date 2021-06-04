#include <ipc/socket.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  char* sockname = "\\\\.\\pipe\\test_sock";
  Socket* server = malloc(sizeof(Socket));

  socket_create(server, sockname);

  socket_wait_for_connect(server);
  char* buffer = malloc(512);
  strcpy(buffer, "Hello from server");

  socket_write_bytes(server, buffer, strlen(buffer) + 1);

  socket_read_bytes(server, buffer, 512);

  printf("%s\n", buffer);

  return 0;
}
