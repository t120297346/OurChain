#include <ourcontract.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static struct {
  int a;
  int b;
} state;

int contract_main(int argc, char **argv) {
  printf("hello world");
  // state_read(&state, sizeof(state));
  err_printf("state is %d %d!\n", state.a, state.b);
  return 0;
}