/*
Leon Lin in Lab408, NTU CSIE, 2023/12/31

ORC20 Contract Implementation
(ERC20 Compatible)

FUNCTIONS
PURE:totalSupply()
// Returns the amount of tokens in existence.
PURE:balanceOf(account)
// Returns the amount of tokens owned by account.
transfer(recipient, amount)
// Moves amount tokens from the caller’s account to recipient.
PURE:allowance(owner, spender)
// Returns the remaining number of tokens that spender will be allowed to spend on behalf of owner through transferFrom. This is zero by default.
approve(spender, amount)
// Sets amount as the allowance of spender over the caller’s tokens.
transferFrom(sender, recipient, amount)
// Moves amount tokens from sender to recipient using the allowance mechanism. amount is then deducted from the caller’s allowance.

COMMENTS
approve, transferFrom, allowance 用來花費別人的 TOKEN

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

enum Command {
    totalSupply,
    balanceOf,
    transfer,
    allowance,
    approve,
    transferFrom
};

static std::unordered_map<std::string,Command> const string2Command = { {"totalSupply",Command::totalSupply}, {"balanceOf",Command::balanceOf}, {"transfer",Command::transfer}, {"allowance",Command::allowance}, {"approve",Command::approve}, {"transferFrom",Command::transferFrom} };

struct Token {
    unsigned int id;
    std::string name;
    std::string description;
    std::string data;
    // private
    std::string owner;
    std::vector<std::string> approved;

    // function
    std::string Serializer() {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;
        j["data"] = data;
        j["owner"] = owner;
        j["approved"] = approved;
        return j.dump();
    }
};

class Repository {
    public:
        Repository() {
            std::string* buf = state_read();
            if (buf != nullptr) {
                std::cerr << "get state: " << buf->c_str() << std::endl;
                // some operation
                json j = j.parse(*buf);
                for (auto& element : j) {
                    Token token;
                    token.id = element["id"];
                    token.name = element["name"];
                    token.description = element["description"];
                    token.data = element["data"];
                    tokens.push_back(token);
                }
                // release resource
                delete buf;
            }
        }
        unsigned int length() {
            return tokens.size();
        }
        void addToken(Token token) {
            tokens.push_back(token);
        }
        Token getToken(unsigned int id) {
            for (auto& token : tokens) {
                if (token.id == id) {
                    return token;
                }
            }
            Token token;
            return token;
        }
        std::vector<Token> getTokens(unsigned int index) {
            std::vector<Token> result;
            for (auto& token : tokens) {
                if (token.id >= index) {
                    result.push_back(token);
                }
            }
        }
        void updateToken(Token token) {
            for (auto& t : tokens) {
                if (t.id == token.id) {
                    t = token;
                    return;
                }
            }
        }
    private:
        std::vector<Token> tokens;
};

extern "C" int contract_main(int argc, char **argv) {
  Repository repository;
  if (argc == 1) {
      std::cerr << "argc error" << std::endl;
      return 0;
  }
  std::string command = argv[1];
  auto eCommand = string2Command.find(command);
  if (eCommand == string2Command.end()) {
      std::cerr << "command error" << std::endl;
      return 0;
  }
  switch (eCommand->second)
  {
  case Command::totalSupply:
    /* code */
    break;
  case Command::balanceOf:
    /* code */
    break;
  case Command::transfer:
    /* code */
    break;
  case Command::allowance:
    /* code */
    break;
  case Command::approve:
    /* code */
    break;
  case Command::transferFrom:
    /* code */
    break;
  default:
    break;
  }
  return 0;
}