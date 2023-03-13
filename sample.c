#include <ourcontract.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// should be def by server
struct stateBuf {
  long mtype;
  char buf[1024];
};

// TODO: ADD https://github.com/protobuf-c/protobuf-c to auto Serialize

// struct state {
//   int senderId;
//   int sequenceNumber;
//   char data[50];
// }

// void Serialize() {}
// void Deserialize(const std::string& iString) {}

int contract_main(int argc, char **argv) {
  err_printf("start contract\n");
  struct stateBuf buf;
  if (state_read(&buf, sizeof(buf)) == -1) {
    err_printf("read state error\n");
  };
  err_printf("get state %s\n", buf.buf);
  buf.mtype = 2; // 必須設置為二, TODO: 封裝此變數到 lib 內
  strcpy(buf.buf, "Hello World!");
  if (state_write(&buf, sizeof(buf)) == -1) {
    err_printf("send state error\n");
  };
  return 0;
}