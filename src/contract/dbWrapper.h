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
    leveldb::Status mystatus;
    leveldb::WriteOptions writeOptions;

    fs::path getContractDBPath(std::string name)
    {
        return GetDataDir() / "contracts" / name;
    }

public:
    leveldb::Status getStatus()
    {
        return mystatus;
    }
    bool isOk()
    {
        return mystatus.ok();
    }
    // connect contract DB
    ContractDBWrapper(std::string name)
    {
        leveldb::Options options;
        options.create_if_missing = true;
        fs::path path = getContractDBPath(name);
        TryCreateDirectories(path);
        mystatus = leveldb::DB::Open(options, path.string(), &db);
        assert(mystatus.ok());
    }
    // disconnect contract DB
    ~ContractDBWrapper()
    {
        delete db;
        db = nullptr;
    }
    // set critical save
    void setCriticalSave()
    {
        writeOptions.sync = true;
    }

    // get state
    std::string getState(std::string key);


    // set state
    void setState(std::string key, std::string value);

    // delete state
    void deleteState(std::string key);

    // get all states
    std::map<std::string, std::string> getAllStates();

    // clear all states
    void clearAllStates();
};


#endif // CONTRACT_DB_WRAPPER_H