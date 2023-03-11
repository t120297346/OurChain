#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "contract/contract.h"
#include "primitives/transaction.h"
#include "amount.h"

#include "util.h"
#include <leveldb/db.h>
#include <vector>
#include <string>

typedef unsigned char uchar;

bool ProcessContract(const Contract &contract, std::vector<CTxOut> &vTxOut, std::vector<uchar> &state, CAmount balance,
					 std::vector<Contract> &nextContract);

class ContractDBWrapper {
public:
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status mystatus;
    // connect contract DB
    ContractDBWrapper();
    // disconnect contract DB
    ~ContractDBWrapper();
    // get state
    std::string getState(std::string key);
    // set state
    void setState(std::string key, std::string buf);
};

#endif // BITCOIN_CONTRACT_PROCESSING_H
