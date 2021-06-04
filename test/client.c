#include <ipc/socket.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
  char* sockname = "\\\\.\\pipe\\test_sock";
  Socket *client = malloc(sizeof(Socket));
  printf("%d\n", socket_connect(client, sockname));

  char* buffer = malloc(512);

  socket_read_bytes(client, buffer, 512);
  printf("%s\n", buffer);
  strcpy(buffer, "Hello from client");
  socket_write_bytes(client, buffer, strlen(buffer) + 1);

  return 0;
}
