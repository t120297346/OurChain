#include "ourcontract.h"
#include <stdio.h>

struct
{
  int is_freezed; // true if the vote already ended
  int user_count;
} state;

int contract_main(int argc, char **argv)
{
  // string print (should add '\n')
  str_printf("sample contract\n", 17 * sizeof(char), "%s");
  // error print (should add '\n'),can check in storage err.log
  err_printf("error log (sample)\n");
  // state access
  if (state_read(&state, sizeof(state)) == -1)
  {
    /* first time call */
    state.is_freezed = 0;
    state.user_count = 0;
    state_write(&state, sizeof(state));
    state_read(&state, sizeof(state));
  }
  state.is_freezed = 1;
  state.user_count = 10;
  // save back to state
  state_write(&state, sizeof(state));
  return 0;
}
