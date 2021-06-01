#include <stdio.h>
#include <stdlib.h>

#include <ipc/pipe.h>

int main(int argc, char **argv) {
  ipcError err;
  Pipe pipe;

  char *pipename = "pipe.c.test_pipe";
  err = pipe_create(&pipe, pipename);
  if (err) {
    fprintf(stderr, "Error: %s\n", ipcError_str(err));
    return EXIT_FAILURE;
  }

  err = pipe_destroy(&pipe);
  if (err) {
    fprintf(stderr, "Error: %s\n", ipcError_str(err));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
