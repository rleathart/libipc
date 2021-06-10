#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <check.h>
#include <ipc/socket.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
char* sockname = "\\\\.\\pipe\\socket.c.test_sock";
#else
char* sockname = "socket.c.test_sock";
#endif

unsigned server_thread(void* arg)
{
  Socket sock;
  socket_init(&sock, sockname, SocketIsServer);
  while (socket_connect(&sock))
    ;

  char* buffer = malloc(512);
  strncpy(buffer, "Hello from server", 512);
  socket_write_bytes(&sock, buffer, 512);
  socket_read_bytes(&sock, buffer, 512);
  if (strcmp(buffer, "Hello from client") != 0)
    return 1;

  return 0;
}

unsigned client_thread(void* arg)
{
  Socket sock;
  socket_init(&sock, sockname, SocketNoFlags);
  while (socket_connect(&sock))
    ;

  char* buffer = malloc(512);
  socket_read_bytes(&sock, buffer, 512);
  if (strcmp(buffer, "Hello from server") != 0)
    return 1;
  strncpy(buffer, "Hello from client", 512);
  socket_write_bytes(&sock, buffer, 512);
  return 0;
}

START_TEST(test_new_socket_server_client)
{
#ifdef _WIN32
  HANDLE threads[2];
  threads[0] = (HANDLE)_beginthreadex(NULL, 0, server_thread, NULL, 0, NULL);
  threads[1] = (HANDLE)_beginthreadex(NULL, 0, client_thread, NULL, 0, NULL);
  WaitForMultipleObjectsEx(2, threads, TRUE, 2000, FALSE);
  unsigned long errcodes[2];
  for (int i = 0; i < 2; i++)
  {
    GetExitCodeThread(threads[i], &errcodes[i]);
    ck_assert_int_eq(errcodes[i], 0);
  }
#endif
}
END_TEST

int main(int argc, char** argv)
{
  Suite* s1 = suite_create("Socket");
  TCase* tc1_1 = tcase_create("Socket");
  SRunner* sr = srunner_create(s1);
  int num_failed;

  tcase_add_test(tc1_1, test_new_socket_server_client);
  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

#ifdef _WIN32
#else
  unlink(sockname);
#endif

  return num_failed == 0 ? 0 : 1;
}
