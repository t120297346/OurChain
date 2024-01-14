#ifndef CONTRACT_PROCESSING_H
#define CONTRACT_PROCESSING_H

#include "contract/cache.h"
#include "contract/contract.h"
#include "contract/dbWrapper.h"
#include "contract/observer.h"
#include "primitives/transaction.h"

#include "chain.h"
#include "util.h"
#include "validation.h"
#include <leveldb/db.h>
#include <string>
#include <vector>

bool ProcessContract(const Contract& contract, const CTransactionRef& curTx, ContractStateCache* cache);

std::string call_rt_pure(ContractDBWrapper* cache, const uint256& contract, const std::vector<std::string>& args);

#endif // CONTRACT_PROCESSING_H
