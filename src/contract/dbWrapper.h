#ifndef CONTRACT_DB_WRAPPER_H
#define CONTRACT_DB_WRAPPER_H

#include "util.h"
#include <leveldb/db.h>
#include <string>
#include <vector>

class ContractDBWrapper
{
private:
    leveldb::DB* db;
    leveldb::DB* cacheDB;
    leveldb::Status mystatus;
    fs::path path = GetDataDir() / "contracts" / "index";
    fs::path cachePath = GetDataDir() / "contracts" / "cache";
    fs::path checkPointPath = GetDataDir() / "contracts" / "checkPoints";

public:
    leveldb::Status getStatus()
    {
        return mystatus;
    }
    // connect contract DB
    ContractDBWrapper();
    // disconnect contract DB
    ~ContractDBWrapper();

    // get state
    std::string getState(std::string key);
    // set state
    void setState(std::string key, void* buf, size_t size);
};


#endif // CONTRACT_DB_WRAPPER_H