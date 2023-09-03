#include <ourcontract.h>
#include <iostream>
#include <json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

using json = nlohmann::json;

extern "C" int contract_main(int argc, char **argv) {
  // try state
  std::string* buf = state_read();
  if (buf != nullptr) {
    std::cerr << "get state: " << buf->c_str() << std::endl;
    // some operation
    json j = j.parse(*buf);
    j.push_back("more click: " + std::to_string((size_t)j.size()));
    std::string* newBuf = new std::string(j.dump());
    int ret = state_write(newBuf);
    if (ret < 0) {
     std::cerr << "send state error" << newBuf->c_str() << std::endl;
    }
    // release resource
    delete buf;
    delete newBuf;
    return 0;
  }
  // init state
  std::cerr << "read state error" << std::endl;
  json j;
  j.push_back("baby cute");
  j.push_back(1);
  j.push_back(true);
  std::string* newBuf = new std::string(j.dump());
  std::cerr << "buf:" << newBuf->c_str() << std::endl;
  int ret = state_write(newBuf);
  if (ret < 0) {
    std::cerr << "send state error" << newBuf->c_str() << std::endl;
  }
  delete newBuf;
  return 0;
}