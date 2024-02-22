#include "contract/cache.h"

SnapShot::SnapShot(std::string name)
{
    dbWrapper = new ContractDBWrapper(name);
}
SnapShot::~SnapShot()
{
    delete dbWrapper;
}

void SnapShot::setContractState(uint256 address, json state)
{
    dbWrapper->setState(address.ToString(), state.dump());
    assert(dbWrapper->isOk());
}
json SnapShot::getContractState(uint256 address)
{
    std::string state = dbWrapper->getState(address.ToString());
    if (dbWrapper->isOk() == false)
        return nullptr;
    return json::parse(state);
}
void SnapShot::clear()
{
    dbWrapper->clearAllStates();
}
void SnapShot::saveCheckPoint(std::string tipBlockHash)
{
    dbWrapper->saveCheckPoint(tipBlockHash);
}
void SnapShot::saveTmpState()
{
    dbWrapper->saveTmpState();
}
bool SnapShot::isCheckPointExist(std::string tipBlockHash)
{
    return dbWrapper->findCheckPoint(tipBlockHash);
}
ContractDBWrapper* SnapShot::getDBWrapper()
{
    return dbWrapper;
}

BlockCache::BlockCache()
{
    dbWrapper = new ContractDBWrapper(std::string("block_index"));
}
BlockCache::~BlockCache()
{
    delete dbWrapper;
}

void BlockCache::clear()
{
    dbWrapper->clearAllStates();
}
void BlockCache::setBlockIndex(uint256 blockHash, int blockHeight)
{
    dbWrapper->setState(intToKey(blockHeight), blockHash.ToString());
    assert(dbWrapper->isOk());
}
uint256 BlockCache::getBlockHash(int blockHeight)
{
    std::string blockHash = dbWrapper->getState(intToKey(blockHeight));
    if (dbWrapper->isOk() == false)
        return uint256();
    return uint256S(blockHash);
}
BlockCache::blockIndex BlockCache::getHeighestBlock()
{
    rocksdb::Iterator* it = dbWrapper->getIterator();
    // 定位到数据库的最后一个条目
    it->SeekToLast();
    if (it->Valid() == false) {
        delete it;
        return blockIndex{uint256(), -1};
    }
    blockIndex result = blockIndex(uint256S(it->value().ToString()), keyToInt(it->key().ToString()));
    delete it;
    return result;
}
void BlockCache::removeBlockIndex(int blockHeight)
{
    dbWrapper->deleteState(intToKey(blockHeight));
    assert(dbWrapper->isOk());
}
ContractStateCache::ContractStateCache()
{
    blockCache = new BlockCache();
    snapShot = new SnapShot(std::string("current_cache"));
}
ContractStateCache::~ContractStateCache()
{
    delete blockCache;
    delete snapShot;
}
SnapShot* ContractStateCache::getSnapShot()
{
    return snapShot;
}
void ContractStateCache::clearSnapShot()
{
    snapShot->clear();
}
bool ContractStateCache::getFirstBlockCache(BlockCache::blockIndex& blockIndex)
{
    blockIndex = blockCache->getHeighestBlock();
    if (blockIndex.blockHeight == -1)
        return false;
    return true;
}
BlockCache* ContractStateCache::getBlockCache()
{
    return blockCache;
}
void ContractStateCache::pushBlock(BlockCache::blockIndex blockIndex)
{
    blockCache->setBlockIndex(blockIndex.blockHash, blockIndex.blockHeight);
}
void ContractStateCache::popBlock()
{
    BlockCache::blockIndex blockIndex = blockCache->getHeighestBlock();
    blockCache->removeBlockIndex(blockIndex.blockHeight);
}
void ContractStateCache::saveCheckPoint()
{
    auto blockIndex = blockCache->getHeighestBlock();
    if (snapShot->isCheckPointExist(blockIndex.blockHash.ToString()))
        return;
    snapShot->saveCheckPoint(blockIndex.blockHash.ToString());
}
void ContractStateCache::saveTmpState()
{
    snapShot->saveTmpState();
}
bool ContractStateCache::restoreCheckPoint()
{
    auto blockIndex = blockCache->getHeighestBlock();
    if (snapShot->isCheckPointExist(blockIndex.blockHash.ToString())) {
        auto checkSnapShot = ContractDBWrapper(std::string("checkPoint/") + blockIndex.blockHash.ToString());
        return true;
    }
    return false;
}
void ContractStateCache::clearCheckPoint(int maxBlockCheckPointCount)
{
    // record recent block hash
    std::vector<std::string> recentBlockHash;
    // get recent block hash
    {
        auto curHeight = blockCache->getHeighestBlock().blockHeight;
        for (int i = 0; i < maxBlockCheckPointCount; i++) {
            auto blockHash = blockCache->getBlockHash(curHeight);
            if (blockHash.IsNull())
                break;
            recentBlockHash.push_back(blockHash.ToString());
            curHeight--;
        }
    }
    // get check point folder path
    fs::path checkPointPath = snapShot->getDBWrapper()->CheckPointPath;
    // iterate all checkPoint in folder
    for (auto& p : fs::directory_iterator(checkPointPath)) {
        if (!fs::is_directory(p))
            continue;
        // get checkPoint file path
        fs::path checkPointFilePath = p.path();
        // get checkPoint file name
        std::string checkPointFileName = checkPointFilePath.filename().string();
        // check if checkPoint file name is in recent block hash
        auto isShouldRemove = true;
        for (auto& blockHash : recentBlockHash) {
            if (blockHash == checkPointFileName) {
                isShouldRemove = false;
                break;
            }
        }
        if (isShouldRemove) {
            fs::remove_all(checkPointFilePath);
        }
    }
}