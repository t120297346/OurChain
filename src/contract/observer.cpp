#include "contract/observer.h"

ContractObserver::ContractObserver(ContractStateCache* cache)
{
    this->cache = cache;
    this->updateStrategyFactory = UpdateStrategyFactory();
}

ContractObserver::~ContractObserver()
{
}

bool ContractObserver::onChainStateSet(CChain& chainActive, const Consensus::Params consensusParams)
{
    auto curUpdateStrategy = updateStrategyFactory.createUpdateStrategy(chainActive, cache);
    // print curUpdateStrategy.getName()
    // LogPrintf("curUpdateStrategy.getName(): %d\n", curUpdateStrategy->getName());
    if (curUpdateStrategy->getName() == UpdateStrategyType::UpdateStrategyTypeUnDo) {
        return true;
    }

    auto snapshot = cache->getSnapShot();
    if (!curUpdateStrategy->UpdateSnapShot(*cache, snapshot, chainActive, consensusParams)) {
        return false;
    }

    cache->setSnapShot(snapshot);
    // if (!curUpdateStrategy.UpdateCheckpoint(cache)) {
    //     return false;
    // }
    // if (!curUpdateStrategy.UpdateDiskCheckpoint(cache)) {
    //     return false;
    // }

    return true;
}

bool ContractObserver::onChainInitialized(CChain& chainActive, const Consensus::Params consensusParams)
{
    // LogPrintf("onChainInitialized\n");
    return true;
}