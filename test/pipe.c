#include <stdio.h>
#include <stdlib.h>

#include <ipc/pipe.h>
#include <check.h>

START_TEST(test_pipe_create_and_destroy) {
#ifdef _WIN32
  char *pipename = "\\\\.\\pipe\\pipe.c.test_pipe";
#else
  char *pipename = "pipe.c.test_pipe";
#endif
  ipcError err = 0;
  Pipe pipe = pipe_create(pipename, &err);
  fail_if(err, "Could not create a simple pipe");
  err = pipe_destroy(&pipe);
  fail_if(err, "Could not destroy a pipe");
}
END_TEST

int main(int argc, char **argv) {
    Suite *s1 = suite_create("Pipe");
    TCase *tc1_1 = tcase_create("Pipe");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_test(tc1_1, test_pipe_create_and_destroy);

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}
