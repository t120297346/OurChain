#include <ourcontract.h>
#include <iostream>
#include <json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct stateBuf {
  long mtype;
  char buf[1024];
};

using json = nlohmann::json;

extern "C" int contract_main(int argc, char **argv) {
  // try json lib
  json j;
  j.push_back("baby cute");
  j.push_back(1);
  j.push_back(true);
  std::string str = j.dump();
  std::cerr << str << std::endl;
  std::cerr << "start contract" << std::endl;
  // try state
  struct stateBuf buf;
  if (state_read(&buf, sizeof(struct stateBuf)) == -1) {
     std::cerr << "read state error" << std::endl;
  };
  std::cerr << "get state" << buf.buf << std::endl;
  buf.mtype = 1;
  strcpy(buf.buf, "Hello World!");
  if (state_write(&buf, sizeof(struct stateBuf)) == -1) {
    std::cerr << "send state error" << buf.buf << std::endl;
  };
  return 0;
}