#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <check.h>
#include <ipc/socket.h>

#ifndef _WIN32
#include <unistd.h>
#endif

Socket test_server;
Socket test_client;
char *sockname = "socket.c.test_sock";

void setup() {
  socket_destroy(&test_server);
  socket_destroy(&test_client);
}

void teardown() {
  socket_destroy(&test_server);
  socket_destroy(&test_client);
}

START_TEST(test_socket_create) {
  fail_if(socket_create(&test_server, sockname),
          "Could not create a simple socket");
}
END_TEST

START_TEST(test_socket_destroy) {
  ipcError err = 0;
  char errstr[512];
  socket_create(&test_server, sockname);
  err = socket_destroy(&test_server);
  sprintf(errstr, "Could not destroy a simple socket.\n\t%s",
          ipcError_str(err));
  fail_if(err, errstr);
}
END_TEST

START_TEST(test_socket_server_client) {
#ifdef _WIN32
#else
  struct timespec ts = {
      .tv_sec = 0,
      .tv_nsec = 50 * 1000000 // give server 50ms to start up
  };

  pid_t pid = fork();
  if (pid == 0) {
    socket_create(&test_server, sockname);
    socket_wait_for_connect(&test_server);
    char *buffer = malloc(512);
    strcpy(buffer, "Hello from server");

    socket_write_bytes(&test_server, buffer, strlen(buffer) + 1);

    socket_read_bytes(&test_server, buffer, 512);

    fail_if(strcmp(buffer, "Hello from client") != 0);
  } else {
    nanosleep(&ts, &ts);
    fail_if(socket_connect(&test_client, sockname));

    char *buffer = malloc(512);

    socket_read_bytes(&test_client, buffer, 512);
    fail_if(strcmp(buffer, "Hello from server") != 0);
    strcpy(buffer, "Hello from client");
    socket_write_bytes(&test_client, buffer, strlen(buffer) + 1);
  }
#endif
}
END_TEST

int main(int argc, char **argv) {
  Suite *s1 = suite_create("Socket");
  TCase *tc1_1 = tcase_create("Socket");
  SRunner *sr = srunner_create(s1);
  int num_failed;

  setup();

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
