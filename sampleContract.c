#include <ourcontract.h>
#include <stdio.h>

int contract_main(int argc, char **argv)
{
  int x = state_read();
  err_printf("TESTTEST End DB operate %d\n", x);
  return 0;
}