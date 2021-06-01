#include <stdio.h>
#include <stdlib.h>

#include <ipc/pipe.h>

int main(int argc, char **argv) {
  ipcError err;
  Pipe pipe;
  err = pipe_create(&pipe, "pipe.c.test_pipe");
  if (err) {
    fprintf(stderr, "Error: %s\n", ipcError_str(err));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
