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
    fs::path getContractCheckPointPath(std::string name)
    {
        return GetDataDir() / "contracts" / "checkPoint" / name;
    }
    // 清除該位置前一個快照並保存當前狀態快照到目標位置
    void clearAndSaveDuplicateState(fs::path path);
    // 保存當前狀態快照到目標位置
    void saveDuplicateState(fs::path path);

public:
    // pre db operation status
    leveldb::Status getStatus();
    // is pre db operation ok
    bool isOk();
    // set critical save (directly write to disk)
    void setCriticalSave();
    // get iterator, for custom operation on database
    leveldb::Iterator* getIterator();
    // connect contract DB
    ContractDBWrapper(std::string name);
    // connrct to check point db
    ContractDBWrapper(std::string checkPointBlockHash, bool isCheckPoint);
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