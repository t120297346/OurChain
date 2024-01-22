/*
Leon Lin in Lab408, NTU CSIE, 2024/01/08

AID01 Contract Implementation

FUNCTIONS
PURE:login(name, password)
// Returns the user information of name.
PURE:verify(aid, password)
// Returns if password is correct.
registerNewUser(name, password)

COMMENTS
only use password can not used in prod, but it's ok in this example

*/
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

enum Command
{
    login,
    verify,
    registerNewUser,
    getCoins,
    setCoin,
    removeCoin,
};

static std::unordered_map<std::string, Command> const string2Command = {{"login", Command::login}, {"verify", Command::verify}, {"registerNewUser", Command::registerNewUser}, {"getCoins", Command::getCoins}, {"setCoin", Command::setCoin}, {"removeCoin", Command::removeCoin}};

extern "C" int contract_main(int argc, char **argv)
{
    // init state
    if (!state_exist())
    {
        json j = json::array();
        state_write(j);
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
    case Command::login:
        if (check_runtime_can_write_db())
        {
            return 0;
        }
        {
            std::string name = argv[2];
            std::string password = argv[3];
            json j = state_read();
            for (auto &i : j)
            {
                if (i["name"] == name && i["password"] == password)
                {
                    json tmp = json::object();
                    tmp["isExist"] = true;
                    tmp["aid"] = i["aid"];
                    tmp["name"] = i["name"];
                    state_write(tmp);
                    return 0;
                }
            }
            json tmp = json::object();
            tmp["isExist"] = false;
            state_write(tmp);
            return 0;
        }
    case Command::verify:
        if (check_runtime_can_write_db())
        {
            return 0;
        }
        {
            std::string aid = argv[2];
            std::string password = argv[3];
            json j = state_read();
            for (auto &i : j)
            {
                if (i["aid"] == aid)
                {
                    json tmp = json::object();
                    tmp["isExist"] = true;
                    if (i["password"] == password)
                    {
                        tmp["result"] = true;
                    }
                    else
                    {
                        tmp["result"] = false;
                    }
                    state_write(tmp);
                    return 0;
                }
            }
            json tmp = json::object();
            tmp["isExist"] = false;
            state_write(tmp);
            return 0;
        }
    case Command::registerNewUser:
        if (!check_runtime_can_write_db())
        {
            return 0;
        }
        {
            std::string name = argv[2];
            std::string password = argv[3];
            json j = state_read();
            json tmp;
            tmp["name"] = name;
            tmp["password"] = password;
            tmp["aid"] = boost::uuids::to_string(boost::uuids::random_generator()());
            j.push_back(tmp);
            state_write(j);
        }
        break;
    case Command::getCoins:
        if (check_runtime_can_write_db())
        {
            return 0;
        }
        {
            std::string aid = argv[2];
            json j = state_read();
            for (auto &i : j)
            {
                if (i["aid"] == aid)
                {
                    json tmp = json::object();
                    tmp["isExist"] = true;
                    // check if coins exist
                    if (i.find("coins") != i.end())
                    {
                        tmp["coins"] = i["coins"];
                    }
                    else
                    {
                        tmp["coins"] = json::array();
                    }
                    state_write(tmp);
                    return 0;
                }
            }
            json tmp = json::object();
            tmp["isExist"] = false;
            state_write(tmp);
            break;
        }
    case Command::setCoin:
        if (!check_runtime_can_write_db())
        {
            return 0;
        }
        {
            std::string aid = argv[2];
            std::string coinAddress = argv[3];
            json j = state_read();
            for (auto &i : j)
            {
                if (i["aid"] == aid)
                {
                    // check if coins exist
                    if (i.find("coins") != i.end())
                    {
                        i["coins"].push_back(coinAddress);
                    }
                    else
                    {
                        i["coins"] = json::array();
                        i["coins"].push_back(coinAddress);
                    }
                    state_write(j);
                    return 0;
                }
            }
            break;
        }
    case Command::removeCoin:
        if (!check_runtime_can_write_db())
        {
            return 0;
        }
        {
            std::string aid = argv[2];
            std::string coinAddress = argv[3];
            json j = state_read();
            for (auto &i : j)
            {
                if (i["aid"] == aid)
                {
                    // check if coins exist
                    if (i.find("coins") != i.end())
                    {
                        auto tmp = json::array();
                        for (auto &coin : i["coins"])
                        {
                            if (coin != coinAddress)
                            {
                                tmp.push_back(coin);
                            }
                        }
                        i["coins"] = tmp;
                        state_write(j);
                        return 0;
                    }
                    break;
                }
            }
            break;
        }
    }
    return 0;
}