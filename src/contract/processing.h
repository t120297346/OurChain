#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "contract/contract.h"
#include "primitives/transaction.h"
#include "amount.h"

#include "util.h"
#include <leveldb/db.h>
#include <vector>

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
    void getState();
    // set state
    void setState();
};

#endif // BITCOIN_CONTRACT_PROCESSING_H
