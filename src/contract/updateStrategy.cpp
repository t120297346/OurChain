#include "contract/updateStrategy.h"
#include "contract/processing.h"

static bool processContracts(std::stack<CBlock*> realBlock, ContractStateCache& cache)
{
    while (realBlock.size() > 0) {
        CBlock* tmpBlock = realBlock.top();
        realBlock.pop();
        for (const CTransactionRef& tx : tmpBlock->vtx) {
            if (!ProcessContract(tx.get()->contract, tx, &cache)) {
                LogPrintf("contract process error: %s\n", tx.get()->contract.address.ToString());
            }
        }
        delete tmpBlock;
    }
    return true;
}

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
            return new UpdateStrategyRollback();
        }
        // curHeight > cacheHeight
        return new UpdateStrategyContinue();
    }
    // curHash == cacheHash && curHeight != cacheHeight
    return new UpdateStrategyRollback();
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
    return processContracts(realBlock, cache);
}

bool UpdateStrategyContinue::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    auto realBlock = std::stack<CBlock*>();
    auto tmpBlockIndex = std::stack<BlockCache::blockIndex>();
    BlockCache::blockIndex firstBlock;
    if (!cache.getFirstBlockCache(firstBlock)) {
        return false;
    }
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        CBlock* block = new CBlock();
        // push block index to cache
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();
        if (height == firstBlock.blockHeight && hash == firstBlock.blockHash) {
            // find pre chain state
            while (tmpBlockIndex.size() > 0) {
                BlockCache::blockIndex tmpBlock = tmpBlockIndex.top();
                tmpBlockIndex.pop();
                cache.pushBlock(tmpBlock);
            }
            break;
        }
        if (height < firstBlock.blockHeight) {
            // release memory
            while (realBlock.size() > 0) {
                CBlock* tmpBlock = realBlock.top();
                realBlock.pop();
                delete tmpBlock;
            }
            // can not find pre chain state, rollback
            UpdateStrategyRollback algo;
            return algo.UpdateSnapShot(cache, snapShot, chainActive, consensusParams);
        }
        tmpBlockIndex.push(BlockCache::blockIndex(hash, height));
        // save block data in memory
        if (!ReadBlockFromDisk(*block, pindex, consensusParams)) {
            return false;
        }
        realBlock.push(block);
    }
    return processContracts(realBlock, cache);
}

bool UpdateStrategyRollback::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        // push block index to cache
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();
        if (cache.getSnapShot()->isCheckPointExist(hash.ToString())) {
            // find closest check point
            auto newDB = ContractDBWrapper(hash.ToString(), true);
            newDB.transferAllState(*cache.getSnapShot()->getDBWrapper());
            // check heigh same
            auto newBlockIndex = cache.getBlockCache()->getHeighestBlock();
            if (newBlockIndex.blockHash != hash || newBlockIndex.blockHeight != height) {
                LogPrintf("rollback error can not continue in checkPoint \n");
                continue;
            }
            UpdateStrategyContinue algo;
            return algo.UpdateSnapShot(cache, snapShot, chainActive, consensusParams);
        }
        cache.popBlock();
    }
    UpdateStrategyRebuild algo;
    return algo.UpdateSnapShot(cache, snapShot, chainActive, consensusParams);
}

bool UpdateStrategyUnDo::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams)
{
    // default is todo nothing
    return true;
}