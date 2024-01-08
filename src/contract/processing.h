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
public:
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status mystatus;
    // connect contract DB
    ContractDBWrapper(){
        options.create_if_missing = true;
        fs::path path = GetDataDir() / "contracts" / "index";
        TryCreateDirectories(path);
        leveldb::Status status = leveldb::DB::Open(options, path.string(), &db);
        if (status.ok()) {
            LogPrintf("Opening ContractLevelDB in %s\n", path.string());
        }
    }
    // disconnect contract DB
    ~ContractDBWrapper(){
        delete db;
        db = nullptr;
    }
    // get state
    std::string getState(std::string key){
        std::string buf;
        mystatus = db->Get(leveldb::ReadOptions(), key, &buf);
        // LogPrintf("get result: %d\n", mystatus.ok());
        return buf;
    }
    // set state
    void setState(std::string key, void* buf, size_t size){
        leveldb::Slice valueSlice = leveldb::Slice((const char*)buf, size);
        mystatus = db->Put(leveldb::WriteOptions(), key, valueSlice);
        // LogPrintf("put result: %d\n", mystatus.ok());
        assert(mystatus.ok());
    }
};

#endif // BITCOIN_CONTRACT_PROCESSING_H
