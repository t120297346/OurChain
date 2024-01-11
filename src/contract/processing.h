#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "amount.h"
#include "contract/contract.h"
#include "primitives/transaction.h"

#include "chain.h"
#include "util.h"
#include "validation.h"
#include <leveldb/db.h>
#include <string>
#include <vector>

bool ProcessContract(const Contract& contract, std::vector<CTxOut>& vTxOut, std::vector<unsigned char>& state, CAmount balance, std::vector<Contract>& nextContract, const CTransaction& curTx);

std::string call_rt_pure(const uint256& contract, const std::vector<std::string>& args);

class ContractStateCache
{
public:
    ContractStateCache(){
        // LogPrintf("ContractStateCache init\n");
    };
    ~ContractStateCache(){
        // LogPrintf("ContractStateCache destroy\n");
    };

private:
    uint256 blockTipHash;
    int blockTipHeight;
};

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

class ContractObserver
{
public:
    ContractObserver()
    {
        // LogPrintf("ContractObserver init\n");
    }
    ~ContractObserver()
    {
        // LogPrintf("ContractObserver destroy\n");
    }
    bool onBlockConnected(CChain& chainActive, const Consensus::Params consensusParams)
    {
        // get blocks in chainActive
        for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
            CBlock block;
            ReadBlockFromDisk(block, pindex, consensusParams);
            // print transaction in block
            LogPrintf("block %d\n", pindex->nHeight);
            for (const CTransactionRef& tx : block.vtx) {
                if (tx.get()->contract.action == ACTION_NEW) {
                    LogPrintf("contract %s\n", tx.get()->contract.address.ToString().c_str());
                    LogPrintf("tx %s\n", tx.get()->contract.code.c_str());
                } else if (tx.get()->contract.action == ACTION_CALL) {
                    LogPrintf("call %s\n", tx.get()->contract.address.ToString().c_str());
                    // print args
                    for (const std::string& arg : tx.get()->contract.args) {
                        LogPrintf("arg %s\n", arg.c_str());
                    }
                }
            }
        }
        return true;
    }
    bool onBlockDisconnected(const CBlock& block)
    {
        // LogPrintf("onBlockDisconnected\n");
        return true;
    }
    bool onChainInitialized(const CChain& chainActive, const Consensus::Params consensusParams)
    {
        // LogPrintf("onChainInitialized\n");
        return true;
    }

private:
};

// global variable
static ContractStateCache contractStateCache;

#endif // BITCOIN_CONTRACT_PROCESSING_H
