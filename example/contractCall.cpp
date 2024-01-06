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
  auto targetContractAddess = "65de9877d5f1fe7f48c0e43b95c67a1e88a4a6357d5076b972b0f1e0c35f3192";
  auto ret = call_contract(targetContractAddess,argc,argv);
  if (ret == 0) {
    std::cerr << "call contract success" << std::endl;
  } else {
    std::cerr << "call contract error" << std::endl;
  }
  json j = pre_state_read();
  j.push_back("contract can call other contract");
  state_write(j);
  return 0;
}