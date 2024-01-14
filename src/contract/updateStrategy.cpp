#include "contract/updateStrategy.h"
#include "contract/processing.h"

UpdateStrategy* UpdateStrategyFactory::createUpdateStrategy(CChain& chainActive, ContractStateCache* cache)
{
    int curHeight = chainActive.Height();
    uint256 curHash = chainActive.Tip()->GetBlockHash();
    BlockCache::blockIndex firstBlock;
    if (!cache->getFirstBlockCache(firstBlock)) {
        return new UpdateStrategyRebuild();
    }
    int cacheHeight = firstBlock.blockHeight;
    uint256 cacheHash = firstBlock.blockHash;
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
    cache.getBlockCache()->clear();
    auto realBlock = std::stack<CBlock*>();
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        CBlock* block = new CBlock();
        // push block index to cache
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();
        cache.pushBlock(BlockCache::blockIndex(hash, height));
        // save block data in memory
        if (!ReadBlockFromDisk(*block, pindex, consensusParams)) {
            return false;
        }
        realBlock.push(block);
    }
    // process all contract in blocks
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