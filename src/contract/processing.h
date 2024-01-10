#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "amount.h"
#include "contract/contract.h"
#include "primitives/transaction.h"

#include "util.h"
#include <leveldb/db.h>
#include <string>
#include <vector>

typedef unsigned char uchar;

bool ProcessContract(const Contract& contract, std::vector<CTxOut>& vTxOut, std::vector<uchar>& state, CAmount balance, std::vector<Contract>& nextContract, const CTransaction& curTx);

std::string call_rt_pure(const uint256& contract, const std::vector<std::string>& args);

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
    ContractDBWrapper()
    {
        leveldb::Options options;
        options.create_if_missing = true;
        TryCreateDirectories(path);
        TryCreateDirectories(cachePath);
        TryCreateDirectories(checkPointPath);
        mystatus = leveldb::DB::Open(options, path.string(), &db);
        assert(mystatus.ok());
        mystatus = leveldb::DB::Open(options, cachePath.string(), &cacheDB);
        assert(mystatus.ok());
    }
    // disconnect contract DB
    ~ContractDBWrapper()
    {
        delete db;
        delete cacheDB;
        db = nullptr;
        cacheDB = nullptr;
    }
    // get state
    std::string getState(std::string key)
    {
        std::string buf;
        mystatus = cacheDB->Get(leveldb::ReadOptions(), key, &buf);
        if (mystatus.ok()) {
            return buf;
        }
        mystatus = db->Get(leveldb::ReadOptions(), key, &buf);
        if (mystatus.ok()) {
            leveldb::Slice valueSlice = leveldb::Slice(buf);
            mystatus = cacheDB->Put(leveldb::WriteOptions(), key, valueSlice);
            assert(mystatus.ok());
            return buf;
        }
        return "";
    }
    // set state
    void setState(std::string key, void* buf, size_t size)
    {
        leveldb::Slice valueSlice = leveldb::Slice((const char*)buf, size);
        mystatus = cacheDB->Put(leveldb::WriteOptions(), key, valueSlice);
        // LogPrintf("put result: %d\n", mystatus.ok());
        assert(mystatus.ok());
    }
    // sync state (move all state in cache to DB)
    void syncState()
    {
        leveldb::Iterator* it = cacheDB->NewIterator(leveldb::ReadOptions());
        leveldb::WriteOptions writeOptions;
        // writeOptions.sync = true; // sync write when want no error
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            std::string key = it->key().ToString();
            std::string value = it->value().ToString();
            mystatus = db->Put(writeOptions, key, value);
            assert(mystatus.ok());
            mystatus = cacheDB->Delete(writeOptions, key);
            assert(mystatus.ok());
        }
        delete it;
    }
    // flush state (delete all state in cache)
    void flushState()
    {
        leveldb::Iterator* it = cacheDB->NewIterator(leveldb::ReadOptions());
        leveldb::WriteOptions writeOptions;
        // writeOptions.sync = true; // sync write when want no error
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            std::string key = it->key().ToString();
            std::string value = it->value().ToString();
            mystatus = cacheDB->Delete(writeOptions, key);
            assert(mystatus.ok());
        }
        delete it;
    }
    // create check point
    bool createCheckPoint(std::string pointId)
    {
        leveldb::DB* pointDB;
        leveldb::Options options;
        options.create_if_missing = true;
        fs::path pointPath = checkPointPath / pointId;
        TryCreateDirectories(pointPath);
        mystatus = leveldb::DB::Open(options, pointPath.string(), &pointDB);
        if (!mystatus.ok()) {
            return false;
        }
        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
        leveldb::WriteOptions writeOptions;
        writeOptions.sync = true; // sync write when want no error
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            std::string key = it->key().ToString();
            std::string value = it->value().ToString();
            mystatus = pointDB->Put(writeOptions, key, value);
            assert(mystatus.ok());
        }
        delete it;
        delete pointDB;
        pointDB = nullptr;
        return true;
    }
    // recover from check point
    bool recoverFromCheckPoint(std::string pointId)
    {
        fs::path pointPath = checkPointPath / pointId;
        if (!fs::is_directory(pointPath)) {
            return false;
        }
        // remove all state in DB
        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
        leveldb::WriteOptions writeOptions;
        writeOptions.sync = true; // sync write when want no error
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            std::string key = it->key().ToString();
            std::string value = it->value().ToString();
            mystatus = db->Delete(writeOptions, key);
            assert(mystatus.ok());
        }
        delete it;
        // move state from check point to DB
        leveldb::DB* pointDB;
        leveldb::Options options;
        options.create_if_missing = false;
        mystatus = leveldb::DB::Open(options, pointPath.string(), &pointDB);
        if (!mystatus.ok()) {
            return false;
        }
        it = pointDB->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            std::string key = it->key().ToString();
            std::string value = it->value().ToString();
            mystatus = db->Put(writeOptions, key, value);
            assert(mystatus.ok());
        }
        delete it;
        delete pointDB;
        pointDB = nullptr;
        return true;
    }
};

#endif // BITCOIN_CONTRACT_PROCESSING_H
