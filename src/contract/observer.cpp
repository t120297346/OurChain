#include "contract/observer.h"

ContractObserver::ContractObserver(ContractStateCache* cache)
{
    this->cache = cache;
    this->updateStrategyFactory = UpdateStrategyFactory();
}

bool ContractObserver::onChainStateSet(CChain& chainActive, const Consensus::Params consensusParams)
{
    auto curUpdateStrategy = updateStrategyFactory.createUpdateStrategy(chainActive, cache);
    if (curUpdateStrategy->getName() == UpdateStrategyType::UpdateStrategyTypeUnDo) {
        return true;
    }
    auto snapshot = cache->getSnapShot();
    if (!curUpdateStrategy->UpdateSnapShot(*cache, *snapshot, chainActive, consensusParams)) {
        LogPrintf("snapshot: update\n");
        return false;
    }
    cache->saveCheckPoint();
    cache->saveTmpState();
    return true;
}