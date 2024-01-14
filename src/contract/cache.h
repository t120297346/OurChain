#ifndef CONTRACT_CACHE_H
#define CONTRACT_CACHE_H
#include "contract/contract.h"
#include "contract/dbWrapper.h"
#include "json/json.hpp"

using json = nlohmann::json;


class SnapShot
{
public:
    SnapShot(std::string name)
    {
        dbWrapper = new ContractDBWrapper(name);
    }
    ~SnapShot()
    {
        delete dbWrapper;
    }

    void setContractState(uint256 address, json state)
    {
        dbWrapper->setState(address.ToString(), state.dump());
        assert(dbWrapper->isOk());
    }
    json getContractState(uint256 address)
    {
        std::string state = dbWrapper->getState(address.ToString());
        if (dbWrapper->isOk() == false)
            return nullptr;
        return json::parse(state);
    }
    void clear()
    {
        dbWrapper->clearAllStates();
    }

private:
    ContractDBWrapper* dbWrapper;
};

class BlockCache
{
public:
    BlockCache()
    {
        dbWrapper = new ContractDBWrapper(std::string("block_index"));
    }
    ~BlockCache()
    {
        delete dbWrapper;
    }
    struct blockIndex {
        uint256 blockHash;
        int blockHeight;

        blockIndex()
        {
            blockHash = uint256();
            blockHeight = -1;
        }
        blockIndex(uint256 blockHash, int blockHeight)
        {
            this->blockHash = blockHash;
            this->blockHeight = blockHeight;
        }
    };
    void clear()
    {
        dbWrapper->clearAllStates();
    }
    void setBlockIndex(uint256 blockHash, int blockHeight)
    {
        dbWrapper->setState(intToKey(blockHeight), blockHash.ToString());
        assert(dbWrapper->isOk());
    }
    uint256 getBlockHash(int blockHeight)
    {
        std::string blockHash = dbWrapper->getState(intToKey(blockHeight));
        if (dbWrapper->isOk() == false)
            return uint256();
        return uint256S(blockHash);
    }
    blockIndex getHeighestBlock()
    {
        leveldb::Iterator* it = dbWrapper->getIterator();
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
    void removeBlockIndex(int blockHeight)
    {
        dbWrapper->deleteState(intToKey(blockHeight));
        assert(dbWrapper->isOk());
    }

private:
    ContractDBWrapper* dbWrapper;
    std::string intToKey(int num)
    {
        std::string key;
        key.resize(sizeof(int));
        for (size_t i = 0; i < sizeof(int); ++i) {
            key[sizeof(int) - i - 1] = (num >> (i * 8)) & 0xFF;
        }
        return key;
    }
    int keyToInt(const std::string& key)
    {
        int num = 0;
        for (size_t i = 0; i < sizeof(int); ++i) {
            num = (num << 8) | static_cast<unsigned char>(key[i]);
        }
        return num;
    }
};

class ContractStateCache
{
public:
    ContractStateCache()
    {
        blockCache = new BlockCache();
        snapShot = new SnapShot(std::string("current_cache"));
    }
    ~ContractStateCache()
    {
        delete blockCache;
        delete snapShot;
    }
    SnapShot* getSnapShot();
    void clearSnapShot()
    {
        snapShot->clear();
    }
    bool getFirstBlockCache(BlockCache::blockIndex& blockIndex)
    {
        blockIndex = blockCache->getHeighestBlock();
        if (blockIndex.blockHeight == -1)
            return false;
        return true;
    }
    BlockCache* getBlockCache()
    {
        return blockCache;
    }
    void pushBlock(BlockCache::blockIndex blockIndex)
    {
        blockCache->setBlockIndex(blockIndex.blockHash, blockIndex.blockHeight);
    }
    void popBlock()
    {
        BlockCache::blockIndex blockIndex = blockCache->getHeighestBlock();
        blockCache->removeBlockIndex(blockIndex.blockHeight);
    }


private:
    BlockCache* blockCache;
    SnapShot* snapShot;
};

#endif // CONTRACT_CACHE_H
