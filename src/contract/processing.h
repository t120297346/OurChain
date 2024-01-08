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
    leveldb::Status getStatus(){
        return mystatus;
    }
    // connect contract DB
    ContractDBWrapper(){
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
    ~ContractDBWrapper(){
        delete db;
        delete cacheDB;
        db = nullptr;
        cacheDB = nullptr;
    }
    // get state
    std::string getState(std::string key){
        std::string buf;
        mystatus = cacheDB->Get(leveldb::ReadOptions(), key, &buf);
        if(mystatus.ok()){
            return buf;
        }
        mystatus = db->Get(leveldb::ReadOptions(), key, &buf);
        if(mystatus.ok()){
            leveldb::Slice valueSlice = leveldb::Slice(buf);
            mystatus = cacheDB->Put(leveldb::WriteOptions(), key, valueSlice);
            assert(mystatus.ok());
            return buf;
        }
        return "";
    }
    // set state
    void setState(std::string key, void* buf, size_t size){
        leveldb::Slice valueSlice = leveldb::Slice((const char*)buf, size);
        mystatus = cacheDB->Put(leveldb::WriteOptions(), key, valueSlice);
        // LogPrintf("put result: %d\n", mystatus.ok());
        assert(mystatus.ok());
    }
    // sync state
    void syncState(){
        leveldb::Iterator* it = cacheDB->NewIterator(leveldb::ReadOptions());
        leveldb::WriteOptions writeOptions;
        // writeOptions.sync = true; // sync write when want no error
        for(it->SeekToFirst(); it->Valid(); it->Next()){
            std::string key = it->key().ToString();
            std::string value = it->value().ToString();
            mystatus = db->Put(writeOptions, key, value);
            assert(mystatus.ok());
            mystatus = cacheDB->Delete(writeOptions, key);
            assert(mystatus.ok());
        }
        delete it;
    }
    // create check point
    void createCheckPoint(){
        // TODO: create check point
    }
    // recover from check point
    void recoverFromCheckPoint(){
        // TODO: recover from check point
    }
};

#endif // BITCOIN_CONTRACT_PROCESSING_H
