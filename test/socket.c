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

Socket test_server;
Socket test_client;
#ifdef _WIN32
char* sockname = "\\\\.\\pipe\\socket.c.test_sock";
#else
char* sockname = "socket.c.test_sock";
#endif

void setup()
{
  socket_destroy(&test_server);
  socket_destroy(&test_client);
}

void teardown()
{
  socket_destroy(&test_server);
  socket_destroy(&test_client);
}

START_TEST(test_socket_create)
{
  fail_if(socket_create(&test_server, sockname),
          "Could not create a simple socket");
}
END_TEST

START_TEST(test_socket_destroy)
{
  ipcError err = 0;
  char errstr[512];
  socket_create(&test_server, sockname);
  err = socket_destroy(&test_server);
  sprintf(errstr, "Could not destroy a simple socket.\n\t%s",
          ipcError_str(err));
  fail_if(err, errstr);
}
END_TEST

#ifdef _WIN32
unsigned server_thread(void* arg)
{
  socket_create(&test_server, sockname);
  socket_wait_for_connect(&test_server);
  char* buffer = malloc(512);
  strcpy(buffer, "Hello from server");

  socket_write_bytes(&test_server, buffer, strlen(buffer) + 1);

  socket_read_bytes(&test_server, buffer, 512);

  if (strcmp(buffer, "Hello from client") != 0)
    _endthreadex(1);

  _endthreadex(0);

  return 0;
}

unsigned client_thread(void* arg)
{
  if (socket_connect(&test_client, sockname))
    _endthreadex(1);

  char* buffer = malloc(512);

  socket_read_bytes(&test_client, buffer, 512);
  if (strcmp(buffer, "Hello from server") != 0)
    _endthreadex(1);
  strcpy(buffer, "Hello from client");
  socket_write_bytes(&test_client, buffer, strlen(buffer) + 1);

  _endthreadex(0);

  return 0;
}
#endif

START_TEST(test_socket_server_client)
{
#ifdef _WIN32
  HANDLE threads[2];
  threads[0] = (HANDLE)_beginthreadex(NULL, 0, server_thread, NULL, 0, NULL);
  threads[1] = (HANDLE)_beginthreadex(NULL, 0, client_thread, NULL, 0, NULL);
  WaitForMultipleObjectsEx(2, threads, TRUE, 2000, FALSE);
  TerminateThread(threads[0], 2);
  TerminateThread(threads[1], 2);
  unsigned long errcodes[2] = {0};
  GetExitCodeThread(threads[0], &errcodes[0]);
  GetExitCodeThread(threads[1], &errcodes[1]);
  fail_if(errcodes[0] || errcodes[1],
          "Client and server could not communicate");
#else
  pid_t pid = fork();
  if (pid == 0)
  {
    socket_create(&test_server, sockname);
    socket_wait_for_connect(&test_server);
    char* buffer = malloc(512);
    strcpy(buffer, "Hello from server");

    socket_write_bytes(&test_server, buffer, strlen(buffer) + 1);

    socket_read_bytes(&test_server, buffer, 512);

    fail_if(strcmp(buffer, "Hello from client") != 0);
  }
  else
  {
    fail_if(socket_connect(&test_client, sockname));

    char* buffer = malloc(512);

    socket_read_bytes(&test_client, buffer, 512);
    fail_if(strcmp(buffer, "Hello from server") != 0);
    strcpy(buffer, "Hello from client");
    socket_write_bytes(&test_client, buffer, strlen(buffer) + 1);
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

  setup();

  tcase_add_checked_fixture(tc1_1, setup, teardown);
  tcase_add_test(tc1_1, test_socket_create);
  tcase_add_test(tc1_1, test_socket_destroy);
  tcase_add_test(tc1_1, test_socket_server_client);
  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  teardown();

#ifdef _WIN32
#else
  unlink(sockname);
#endif

  return num_failed == 0 ? 0 : 1;
}
