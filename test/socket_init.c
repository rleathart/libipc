#include <stdio.h>
#include <string.h>

#include <ipc/socket.h>

#include "util.h"

int main(int argc, char** argv)
{
  Socket sock;
  socket_init(&sock, test_socket_name, SocketServer);

  Assert(sock.flags & SocketServer);

  Assert(strcmp(sock.name, test_socket_name) == 0);

  /* Assert(sock.client == 0); */
  /* Assert(sock.server == 0); */

  Assert(sock.state.bytes_read == 0);
  Assert(sock.state.bytes_written == 0);
  Assert(sock.state.flags == 0);
}
