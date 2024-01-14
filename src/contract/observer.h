#ifndef CONTRACT_OBSERVER_H
#define CONTRACT_OBSERVER_H

#include "chain.h"
#include "contract/cache.h"
#include "contract/updateStrategy.h"
#include "primitives/transaction.h"
#include "util.h"
#include "validation.h"

class ContractObserver
{
public:
    ContractObserver(ContractStateCache* cache);
    ~ContractObserver();
    bool onChainStateSet(CChain& chainActive, const Consensus::Params consensusParams);

private:
    ContractStateCache* cache;
    UpdateStrategyFactory updateStrategyFactory;
};

#endif // CONTRACT_OBSERVER_H