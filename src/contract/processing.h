#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "contract/contract.h"
#include "primitives/transaction.h"

#include "contract/cache.h"
#include "contract/dbWrapper.h"
#include "contract/observer.h"

#include "chain.h"
#include "util.h"
#include "validation.h"
#include <leveldb/db.h>
#include <string>
#include <vector>

// global variable
static ContractStateCache contractStateCache;

bool ProcessContract(const Contract& contract, const CTransactionRef& curTx, ContractStateCache* cache);

std::string call_rt_pure(ContractStateCache* cache, const uint256& contract, const std::vector<std::string>& args);

#endif // BITCOIN_CONTRACT_PROCESSING_H
