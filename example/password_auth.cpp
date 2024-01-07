#include <ourcontract.h>
#include <iostream>
#include <json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using json = nlohmann::json;

extern "C" int contract_main(int argc, char **argv) {
  // pure mode
  if (!check_runtime_can_write_db()) {
    json j = state_read();
    state_write(j[0]);
    return 0;
  }
  // call contract state
  if (state_exist()) {
    json j = state_read();
    if(j[0]["password"].get<std::string>() != argv[1]) {
      err_printf("password error");
      return 0;
    }
    j[0]["balance"] = j[0]["balance"].get<int>() + 1;
    state_write(j);
    return 0;
  }
  // init state
  json j;
  j["uuid"] = boost::uuids::to_string(boost::uuids::random_generator()());
  j["name"] = "bunny";
  j["password"] = "55688";
  j["balance"] = 1;
  json state;
  state.push_back(j);
  state_write(state);
  return 0;
}