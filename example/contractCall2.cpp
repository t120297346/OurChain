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
  if(check_runtime_can_write_db() == true){
    std::cerr << "runtime can write db" << std::endl;
    return 0;
  }
  // std::cerr << argv[0] << std::endl; // contract address itself
  // std::cerr << argv[1] << std::endl; // first argument is target contract address
  auto targetContractAddess = argv[1];
  auto ret = call_contract(targetContractAddess,argc,argv);
  if (ret == 0) {
    std::cerr << "call contract success" << std::endl;
  } else {
    std::cerr << "call contract error" << std::endl;
  }
  json j = pre_state_read();
  j.push_back("contract can call other contract 2");
  state_write(j);
  return 0;
}