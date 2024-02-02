/*
Leon Lin in Lab408, NTU CSIE, 2024/1/30

Distribution Store Contract Implementation

FUNCTIONS
PURE:getStoreList()
// Returns the store list.
PURE:getProductList()
// Returns the product list of aid.
PURE:getStoreInfo()
// Returns the store info of aid.
setStore()
// Set a store.
createProduct()
// Create a product.
removeProduct()
// Remove a product.

COMMENTS
this is just a demo for distribution store, not a real store
*/

#include <ourcontract.h>
#include <iostream>
#include <json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

using json = nlohmann::json;

enum Command
{
  getStoreList,
  getProductList,
  getStoreInfo,
  setStore,
  createProduct,
  removeProduct,
};

static std::unordered_map<std::string, Command> const string2Command = {{"getStoreList", Command::getStoreList}, {"getProductList", Command::getProductList}, {"getStoreInfo", Command::getStoreInfo}, {"setStore", Command::setStore}, {"createProduct", Command::createProduct}, {"removeProduct", Command::removeProduct}};
static std::string aidContractAddress = "";

/**
 * Utils
 */

char *getDynamicString(const char *str)
{
  char *ret = (char *)malloc(sizeof(char) * strlen(str));
  strcpy(ret, str);
  return ret;
}

void removeDynamicStrings(char **argv)
{
  for (int i = 0; i < 4; i++)
  {
    delete[] argv[i];
  }
  delete[] argv;
}

bool verifyUser(std::string aid, std::string password)
{
  // check if aid is valid
  char **subArgv = (char **)malloc(sizeof(char *) * 4);
  // put aidContractAddress into argv[0]
  subArgv[0] = getDynamicString(aidContractAddress.c_str());
  subArgv[1] = getDynamicString("verify");
  subArgv[2] = getDynamicString(aid.c_str());
  subArgv[3] = getDynamicString(password.c_str());
  int ret = call_contract(aidContractAddress.c_str(), 4, subArgv);
  removeDynamicStrings(subArgv);
  if (ret != 0)
  {
    std::cerr << "aid contract exe error" << std::endl;
    return false;
  }
  json verifyResponse = pre_state_read();
  if (!verifyResponse["isExist"].get<bool>())
  {
    std::cerr << "aid not exist" << std::endl;
    return false;
  }
  if (!verifyResponse["result"].get<bool>())
  {
    std::cerr << "aid password error" << std::endl;
    return false;
  }
  return true;
}

/**
 * Data Structure
 */
struct product
{
  std::string name;
  int price;
  std::string address;
};

struct store
{
  std::string name;
  std::string aid;
  std::vector<product> productList;
};

struct mall
{
  std::string name;
  std::string coinAddress;
  std::string aidContractAddress;
  std::vector<store> storeList;
};

void to_json(json &j, const product &p)
{
  j = json{{"name", p.name}, {"price", p.price}, {"address", p.address}};
}

void from_json(const json &j, product &p)
{
  j.at("name").get_to(p.name);
  j.at("price").get_to(p.price);
  j.at("address").get_to(p.address);
}

void to_json(json &j, const store &s)
{
  j = json{{"name", s.name}, {"aid", s.aid}, {"productList", s.productList}};
}

void from_json(const json &j, store &s)
{
  j.at("name").get_to(s.name);
  j.at("aid").get_to(s.aid);
  j.at("productList").get_to(s.productList);
}

void to_json(json &j, const mall &m)
{
  j = json{
      {"name", m.name}, {"coinAddress", m.coinAddress}, {"storeList", m.storeList}, {"aidContractAddress", m.aidContractAddress}};
}

void from_json(const json &j, mall &m)
{
  j.at("name").get_to(m.name);
  j.at("coinAddress").get_to(m.coinAddress);
  j.at("storeList").get_to(m.storeList);
  j.at("aidContractAddress").get_to(m.aidContractAddress);
}

/**
 * Main
 */
extern "C" int contract_main(int argc, char **argv)
{
  // init state
  if (!state_exist())
  {
    mall newMall;
    newMall.name = argv[1];
    newMall.coinAddress = argv[2];
    newMall.aidContractAddress = argv[3];
    state_write(newMall);
    return 0;
  }
  // execute command
  if (argc == 1)
  {
    std::cerr << "argc error" << std::endl;
    return 0;
  }
  std::string command = argv[1];
  auto eCommand = string2Command.find(command);
  if (eCommand == string2Command.end())
  {
    std::cerr << "command error" << std::endl;
    return 0;
  }
  switch (eCommand->second)
  {
  case Command::getProductList:
    if (check_runtime_can_write_db())
    {
      return 0;
    }
    {
      std::string aid = argv[2];
      json tmp = json::array();
      mall curMall = state_read().get<mall>();
      for (auto &store : curMall.storeList)
      {
        if (store.aid == aid)
        {
          for (auto &product : store.productList)
          {
            json j = json::object();
            j["name"] = product.name;
            j["price"] = product.price;
            j["address"] = product.address;
            j["coin"] = curMall.coinAddress;
            tmp.push_back(j);
          }
          state_write(tmp);
          return 0;
        }
      }
      // std::cerr << "store not exist" << std::endl;
      state_write(tmp);
      return 0;
    }
    break;
  case Command::getStoreInfo:
    if (check_runtime_can_write_db())
    {
      return 0;
    }
    {
      mall curMall = state_read().get<mall>();
      std::string aid = argv[2];
      json tmp = json::object();
      for (auto &it : curMall.storeList)
      {
        if (it.aid == aid)
        {
          tmp["name"] = it.name;
          tmp["aid"] = it.aid;
          tmp["coin"] = curMall.coinAddress;
          tmp["isExist"] = true;
          state_write(tmp);
          return 0;
        }
      }
      // std::cerr << "store not exist" << std::endl;
      tmp["isExist"] = false;
      state_write(tmp);
      return 0;
    }
    break;
  case Command::getStoreList:
    if (check_runtime_can_write_db())
    {
      return 0;
    }
    {
      mall curMall = state_read().get<mall>();
      json storeList = json::array();
      for (auto &it : curMall.storeList)
      {
        json tmp;
        tmp["name"] = it.name;
        tmp["aid"] = it.aid;
        storeList.push_back(tmp);
      }
      state_write(storeList);
    }
    break;
  case Command::setStore:
    if (!check_runtime_can_write_db())
    {
      return 0;
    }
    {
      std::string aid = argv[2];
      std::string password = argv[3];
      std::string storeName = argv[4];
      mall curMall = state_read().get<mall>();
      aidContractAddress = curMall.aidContractAddress;
      if (!verifyUser(aid, password))
      {
        return 0;
      }
      store newStore;
      newStore.name = storeName;
      newStore.aid = aid;
      // check if store exist
      for (auto &it : curMall.storeList)
      {
        if (it.aid == aid)
        {
          // std::cerr << "store exist" << std::endl;
          it.name = storeName;
          state_write(curMall);
          return 0;
        }
      }
      curMall.storeList.push_back(newStore);
      state_write(curMall);
    }
    break;
  case Command::createProduct:
    if (!check_runtime_can_write_db())
    {
      return 0;
    }
    {
      std::string aid = argv[2];
      std::string password = argv[3];
      std::string productName = argv[4];
      int price = atoi(argv[5]);
      std::string productAddress = argv[6];
      mall curMall = state_read().get<mall>();
      aidContractAddress = curMall.aidContractAddress;
      if (!verifyUser(aid, password))
      {
        return 0;
      }
      for (auto &store : curMall.storeList)
      {
        if (store.aid == aid)
        {
          product newProduct;
          newProduct.name = productName;
          newProduct.price = price;
          newProduct.address = productAddress;
          store.productList.push_back(newProduct);
          state_write(curMall);
          return 0;
        }
      }
      // std::cerr << "store not exist" << std::endl;
      return 0;
    }
    break;
  case Command::removeProduct:
    if (!check_runtime_can_write_db())
    {
      return 0;
    }
    {
      std::string aid = argv[2];
      std::string password = argv[3];
      std::string productName = argv[4];
      mall curMall = state_read().get<mall>();
      aidContractAddress = curMall.aidContractAddress;
      if (!verifyUser(aid, password))
      {
        return 0;
      }
      for (auto &store : curMall.storeList)
      {
        if (store.aid == aid)
        {
          auto it = store.productList.begin();
          while (it != store.productList.end())
          {
            if (it->name == productName)
            {
              it = store.productList.erase(it);
            }
            else
            {
              ++it;
            }
          }
          state_write(curMall);
          return 0;
        }
      }
      // std::cerr << "store not exist" << std::endl;
      return 0;
    }
    break;
  default:
    break;
  }
  return 0;
}