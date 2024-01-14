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

    void clearAndSaveDuplicateState(fs::path path)
    {
        // use fs delete all files in path
        remove_all(path);
        // save duplicate state
        saveDuplicateState(path);
    }

    void saveDuplicateState(fs::path path)
    {
        leveldb::DB* newdb;
        leveldb::Options options;
        options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(options, path.string(), &newdb);
        assert(status.ok());
        leveldb::WriteOptions writeOptions;
        writeOptions.sync = true;
        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            status = newdb->Put(writeOptions, it->key(), it->value());
            if (!status.ok()) {
                LogPrintf("saveCheckPoint: %s, should rebuild\n", status.ToString());
                remove_all(path);
                assert(false);
            }
        }
        delete it;
        delete newdb;
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
    // connrct to check point db
    ContractDBWrapper(std::string checkPointBlockHash, bool isCheckPoint)
    {
        assert(isCheckPoint);
        leveldb::Options options;
        options.create_if_missing = false;
        fs::path path = getContractCheckPointPath(checkPointBlockHash);
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

    // transfer all state
    void transferAllState(ContractDBWrapper& target)
    {
        target.clearAllStates();
        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            target.setState(it->key().ToString(), it->value().ToString());
        }
        delete it;
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

    leveldb::Iterator* getIterator()
    {
        return db->NewIterator(leveldb::ReadOptions());
    }

    // save contract checkPoint
    void saveCheckPoint(std::string tipBlockHash)
    {
        fs::path path = getContractCheckPointPath(tipBlockHash);
        TryCreateDirectories(path);
        saveDuplicateState(path);
    }

    void saveTmpState()
    {
        fs::path path = getContractDBPath("tmp");
        TryCreateDirectories(path);
        clearAndSaveDuplicateState(path);
    }

    // find check point
    bool findCheckPoint(std::string tipBlockHash)
    {
        fs::path path = getContractCheckPointPath(tipBlockHash);
        return exists(path);
    }
};


#endif // CONTRACT_DB_WRAPPER_H