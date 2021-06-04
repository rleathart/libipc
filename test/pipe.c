#include <stdio.h>
#include <stdlib.h>

#include <check.h>
#include <ipc/pipe.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#define strdup _strdup
char *pipename = "\\\\.\\pipe\\pipe.c.test_pipe";
#else
char *pipename = "pipe.c.test_pipe";
#endif

Pipe test_pipe;

void setup(void) { pipe_destroy(&test_pipe); }

void teardown(void) { pipe_destroy(&test_pipe); }

START_TEST(test_pipe_create) {
  ipcError err = 0;
  err = pipe_create(&test_pipe, pipename);
  fail_if(err, "Could not create a simple pipe");
}
END_TEST

START_TEST(test_pipe_destroy) {
  ipcError err = 0;
  pipe_create(&test_pipe, pipename);
  err = pipe_destroy(&test_pipe);
  fail_if(err, "Could not destroy a pipe");
}
END_TEST

START_TEST(test_pipe_can_write_bytes) {
#ifndef _WIN32
  ipcError err = 0;
  pipe_create(&test_pipe, pipename);
  char bytes[4];
  memcpy(bytes, "test", sizeof(bytes));
  pid_t pid = fork();
  if (pid == 0) {
    pipe_connect(&test_pipe, pipename);
    err = pipe_write_bytes(test_pipe, bytes, sizeof(bytes));
  }
  fail_if(err, "Could not write bytes to a pipe");
#endif
}
END_TEST

START_TEST(test_pipe_can_read_bytes) {
  ipcError err = 0;
  char bytes[4], resp[4];
  memcpy(bytes, "test", sizeof(bytes));

  /* pid_t pid = fork(); */
  /* if (pid == 0) { */
  /*   pipe_connect(&test_pipe, pipename); */
  /*   err = pipe_read_bytes(test_pipe, resp, sizeof(resp)); */
  /* } else { */
  /*   pipe_create(&test_pipe, pipename); */
  /*   pipe_write_bytes(test_pipe, bytes, sizeof(bytes)); */
  /* } */

  /* pipe_create(&test_pipe, pipename); */
  /* memcpy(bytes, "test", sizeof(bytes)); */
  /* pipe_write_bytes(test_pipe, bytes, sizeof(bytes)); */
  /* err = pipe_read_bytes(test_pipe, resp, sizeof(resp)); */

  fail_if(err, "Could not read bytes from a pipe");
  for (int i = 0; i < 4; i++) {
    printf("resp[%d]: %c\tbytes[%d]: %c\n", i, resp[i], i, bytes[i]);
    ck_assert_int_eq(resp[i], bytes[i]);
  }
}
END_TEST

int main(int argc, char **argv) {
  Suite *s1 = suite_create("Pipe");
  TCase *tc1_1 = tcase_create("Pipe");
  SRunner *sr = srunner_create(s1);
  int num_failed;

  test_pipe._name = strdup(pipename);
  pipe_destroy(&test_pipe);

  tcase_add_checked_fixture(tc1_1, setup, teardown);
  tcase_add_test(tc1_1, test_pipe_create);
  tcase_add_test(tc1_1, test_pipe_destroy);
  tcase_add_test(tc1_1, test_pipe_can_write_bytes);
  tcase_add_test(tc1_1, test_pipe_can_read_bytes);
  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  test_pipe._name = strdup(pipename);
  pipe_destroy(&test_pipe);

  return num_failed == 0 ? 0 : 1;
}
