#ifndef CONTRACT_CACHE_H
#define CONTRACT_CACHE_H
#include "contract/contract.h"
#include "contract/dbWrapper.h"
#include "json/json.hpp"

using json = nlohmann::json;


class SnapShot
{
public:
    SnapShot(std::string name);
    ~SnapShot();

    void setContractState(uint256 address, json state);
    json getContractState(uint256 address);
    void clear();
    void saveCheckPoint(std::string tipBlockHash);
    void saveTmpState();
    bool isCheckPointExist(std::string tipBlockHash);
    ContractDBWrapper* getDBWrapper();

private:
    ContractDBWrapper* dbWrapper;
};

class BlockCache
{
public:
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

    BlockCache();
    ~BlockCache();
    void clear();
    void setBlockIndex(uint256 blockHash, int blockHeight);
    uint256 getBlockHash(int blockHeight);
    blockIndex getHeighestBlock();
    void removeBlockIndex(int blockHeight);

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
    ContractStateCache();
    ~ContractStateCache();
    SnapShot* getSnapShot();
    void clearSnapShot();
    bool getFirstBlockCache(BlockCache::blockIndex& blockIndex);
    BlockCache* getBlockCache();
    void pushBlock(BlockCache::blockIndex blockIndex);
    void popBlock();
    void saveCheckPoint();
    void saveTmpState();
    bool restoreCheckPoint();

private:
    BlockCache* blockCache;
    SnapShot* snapShot;

    bool isSaveCheckPointNow(int height)
    {
        if (height == 0)
            return false;
        if (height % 10 == 0)
            return true;
        return false;
    }

    bool isSaveReadReplicaNow(int height)
    {
        if (height % 2 == 0)
            return true;
        return false;
    }
};

#endif // CONTRACT_CACHE_H
