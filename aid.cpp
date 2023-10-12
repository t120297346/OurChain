#include <ourcontract.h>
#include <iostream>
#include <json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

using json = nlohmann::json;

// state type define
// []{
//     "name": "baby",
//     "uuid": "xxxxxx",
//     "password": true
//   }

// output state
void state_output(json &j) {
  std::string* newBuf = new std::string(j.dump());
  int ret = state_write(newBuf);
  if (ret < 0) {
   std::cerr << "send state error" << newBuf->c_str() << std::endl;
  }
  delete newBuf;
}

// do some operation by subcommand
int do_operation(std::string subcommand, json* j, char **argv) {
  if (subcommand == "register") {
    json newJ = json::object();
    newJ["name"] = argv[2];
    newJ["uuid"] = argv[3];
    newJ["password"] = argv[4];
    j->push_back(newJ);
    state_output(*j);
  } else if (subcommand == "query") {
    json newJ = json::object();
    for (auto &element : *j) {
      if (element["uuid"] == argv[2]) {
        newJ = element;
        break;
      }
    }
    state_output(newJ);
  } else {
    std::cerr << "subcommand error" << std::endl;
    return -1;
  }
  return 0;
}

/*
 * argv[0]: contract id
 * argv[1]: subcommand
 * argv[2...]: args
 */
extern "C" int contract_main(int argc, char **argv) {
  // try state
  std::string* buf = state_read();
  if (buf != nullptr) {
    json j = j.parse(*buf);
    // some operation
    int ret = do_operation(argv[1], &j, argv);
    if (ret < 0) {
      std::cerr << "do operation error" << std::endl;
      return 0;
    }
    // release resource
    delete buf;
    return 0;
  }
  // init state
  std::cerr << "read state error" << std::endl;
  json j = json::array();
  std::string* newBuf = new std::string(j.dump());
  int ret = state_write(newBuf);
  if (ret < 0) {
    std::cerr << "send state error" << newBuf->c_str() << std::endl;
  }
  delete newBuf;
  return 0;
}
