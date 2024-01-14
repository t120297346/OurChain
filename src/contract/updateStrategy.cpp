#include "contract/updateStrategy.h"
#include "contract/processing.h"

UpdateStrategy* UpdateStrategyFactory::createUpdateStrategy(CChain& chainActive, ContractStateCache* cache)
{
    int curHeight = chainActive.Height();
    uint256 curHash = chainActive.Tip()->GetBlockHash();
    auto cacheObj = cache->getFirstBlockCache();
    if (cacheObj == nullptr) {
        return new UpdateStrategyRebuild();
    }
    int cacheHeight = cacheObj->blockHeight;
    uint256 cacheHash = cacheObj->blockHash;
    if (curHash == cacheHash && curHeight == cacheHeight) {
        return new UpdateStrategyUnDo();
    }
    if (curHash != cacheHash) {
        if (curHeight <= cacheHeight) {
            // return UpdateStrategyRollback();
            return new UpdateStrategyRebuild();
        }
        // curHeight > cacheHeight
        // return UpdateStrategyContinue();
        return new UpdateStrategyRebuild();
    }
    // curHash == cacheHash && curHeight != cacheHeight
    // return UpdateStrategyRollback();
    return new UpdateStrategyRebuild();
}

bool UpdateStrategyRebuild::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    cache.getSnapShot()->clear();
    auto newCacheBlocks = std::vector<ContractStateCache::BlcokCache>();
    auto realBlock = std::stack<CBlock*>();
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        CBlock* block = new CBlock();
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();
        newCacheBlocks.push_back(ContractStateCache::BlcokCache(height, hash));
        if (!ReadBlockFromDisk(*block, pindex, consensusParams)) {
            return false;
        }
        realBlock.push(block);
    }
    while (realBlock.size() > 0) {
        CBlock* tmpBlock = realBlock.top();
        realBlock.pop();
        for (const CTransactionRef& tx : tmpBlock->vtx) {
            if (!ProcessContract(tx.get()->contract, tx, &cache)) {
                return false;
            }
        }
        delete tmpBlock;
    }
    cache.setBlockCache(newCacheBlocks);
    return true;
}

bool UpdateStrategyContinue::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    return true;
}

bool UpdateStrategyRollback::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    return true;
}

bool UpdateStrategyUnDo::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    // default is todo nothing
    return true;
}