#include <ipc/socket.h>

#ifndef __WIN32

#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <unistd.h>

static ipcError _socket_create(Socket *sock, char *name, int is_server) {
  struct sockaddr_un sock_name = {.sun_family = PF_LOCAL, .sun_path = {0}};
  strncpy(sock_name.sun_path, name, sizeof(sock_name.sun_path));
  sock->name = strdup(name);

  if (is_server)
    sock->type = ipcSocketTypeServer;
  else
    sock->type = ipcSocketTypeClient;

  if ((sock->server = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
    return ipcErrorSocketOpen;

  if (is_server) {
    unlink(sock->name);
    if (bind(sock->server, (struct sockaddr *)&sock_name,
             SUN_LEN(&sock_name)) != 0)
      return ipcErrorSocketCreate;
    listen(sock->server, 10); // TODO: sock->max_connections
  } else {
    if ((sock->client = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
      return ipcErrorSocketOpen;
    if (connect(sock->server, (struct sockaddr *)&sock_name,
                SUN_LEN(&sock_name)) != 0)
      return ipcErrorSocketConnect;
  }

  return ipcErrorNone;
}

ipcError socket_create(Socket *sock, char *name) {
  return _socket_create(sock, name, 1);
}

ipcError socket_connect(Socket *sock, char *name) {
  return _socket_create(sock, name, 0);
}

ipcError socket_wait_for_connect(Socket *sock) {
  if (sock->type == ipcSocketTypeServer)
    sock->client = accept(sock->server, NULL, NULL);
  return ipcErrorNone;
}

ipcError socket_write_bytes(Socket *sock, void *buffer, size_t bytes_to_write) {
  SocketHandle handle;
  if (sock->type == ipcSocketTypeServer)
    handle = sock->client;
  else
    handle = sock->server;

  write(handle, buffer, bytes_to_write);
  return ipcErrorNone;
}

ipcError socket_read_bytes(Socket *sock, void *buffer, size_t bytes_to_read) {
  SocketHandle handle;
  if (sock->type == ipcSocketTypeServer)
    handle = sock->client;
  else
    handle = sock->server;

  read(handle, buffer, bytes_to_read);
  return ipcErrorNone;

}

ipcError socket_destroy(Socket *sock) {
  if (unlink(sock->name))
    return ipcErrorFileRemove;

  if (sock->name)
    free(sock->name);

  sock->name = NULL;

  return ipcErrorNone;
}

#endif
