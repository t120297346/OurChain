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
    auto curHeight = cache->getBlockCache()->getHeighestBlock().blockHeight;
    if (isSaveCheckPointNow(curHeight)) {
        cache->saveCheckPoint();
    }
    if (isSaveReadReplicaNow(curHeight)) {
        cache->saveTmpState();
    }
    if (isClearCheckPointNow(curHeight)) {
        cache->clearCheckPoint(100);
    }
    return true;
}

bool ContractObserver::isSaveCheckPointNow(int height)
{
    if (height == 0)
        return false;
    if (height % 10 == 0)
        return true;
    return false;
}

bool ContractObserver::isSaveReadReplicaNow(int height)
{
    if (height % 2 == 0)
        return true;
    return false;
}

bool ContractObserver::isClearCheckPointNow(int height)
{
    if (height == 0)
        return false;
    if (height % 100 == 0)
        return true;
    return false;
}