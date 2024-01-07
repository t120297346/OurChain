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

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>

using json = nlohmann::json;

bool verifySignature(const std::string& publicKeyPEM, const std::string& message, const std::string& signature) {
    // TODO: verify signature
    return true;
}

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
    // auto preTxi = get_pre_txid(); // shold add pre txid in production
    if(!verifySignature(j[0]["publicKey"].get<std::string>(), "55688", argv[1])) {
      std::cerr << "verify fail" << std::endl;
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
  j["publicKey"] = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuL+nop7KpIOCy19bFZID\n32XIr/fSTp0KPVIxELMUFvmrA7ZDoozFHjMbMZmZNxPGOgGJ4Jx6usklFLAykGty\n2QebGtFmC8DQgDy6yXDbRf2N8YE4b8GFNr+tHzRthtQ8V5A3070AyWFozEKze9eH\n5MMOv2SjC3/fuSd3q7eUPBI1mO7Segu3cX+rf8s+7JHcTDHySpV7h1IdTn8t/QDy\ngcObJpHUJucHuGiM9elxRs5TBRgyk6J6t46kZVK9fyJ4A0ulhSIaY5oC7fQmPn5g\n57ItuexVliuWUCxG5DQnnETDBFL33HcSon1P2nu808Wxh26INevmdvhnOnT+u72n\n9wIDAQAB\n-----END PUBLIC KEY-----";
  j["balance"] = 1;
  json state;
  state.push_back(j);
  state_write(state);
  return 0;
}