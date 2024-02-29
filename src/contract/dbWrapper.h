#ifndef CONTRACT_DB_WRAPPER_H
#define CONTRACT_DB_WRAPPER_H

#include "util.h"
#include <boost/thread/shared_mutex.hpp>
#include <rocksdb/db.h>
#include <string>
#include <vector>

#include <boost/thread/shared_mutex.hpp>
#include <shared_mutex>
typedef std::unique_lock<std::shared_mutex> WriteLock;
typedef std::shared_lock<std::shared_mutex> ReadLock;

extern std::shared_mutex tmp_contract_db_mutex;

class ContractDBWrapper
{
private:
    rocksdb::DB* db;
    rocksdb::Status mystatus;
    rocksdb::WriteOptions writeOptions;

    fs::path getContractDBPath(std::string name)
    {
        return GetDataDir() / "contracts" / name;
    }
    fs::path getContractCheckPointPath(std::string name)
    {
        return CheckPointPath / name;
    }
    // 保存當前狀態快照到目標位置
    void saveDuplicateState(fs::path path);

public:
    const fs::path CheckPointPath = GetDataDir() / "contracts" / "checkPoint";
    // pre db operation status
    rocksdb::Status getStatus();
    // is pre db operation ok
    bool isOk();
    // set critical save (directly write to disk)
    void setCriticalSave();
    // get iterator, for custom operation on database
    rocksdb::Iterator* getIterator();
    // connect contract DB
    ContractDBWrapper(std::string name);
    // connect read only contract DB (mode should be "readOnly", "checkPoint")
    ContractDBWrapper(std::string name, std::string mode);
    // disconnect contract DB
    ~ContractDBWrapper();

    // transfer all state
    void transferAllState(ContractDBWrapper& target);
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
    // save contract checkPoint
    void saveCheckPoint(std::string tipBlockHash);
    // save tmp state to fix file place, read only user will read it
    void saveTmpState();
    // find check point
    bool findCheckPoint(std::string tipBlockHash);
};


#endif // CONTRACT_DB_WRAPPER_H