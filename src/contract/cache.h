#ifndef CONTRACT_CACHE_H
#define CONTRACT_CACHE_H
#include "contract/contract.h"
#include "json/json.hpp"

using json = nlohmann::json;

class SnapShot
{
public:
    SnapShot();
    ~SnapShot();

    // duplicate this class
    SnapShot& operator=(const SnapShot& snapShot);

    void setContractState(uint256 address, json state);
    json getContractState(uint256 address);
    void clear()
    {
        contractStateMap.clear();
    }


private:
    std::map<uint256, json> contractStateMap;
};

class ContractStateCache
{
public:
    class BlcokCache
    {
    public:
        BlcokCache(int heigh, uint256 hash)
        {
            blockHeight = heigh;
            blockHash = hash;
        }
        ~BlcokCache() {}
        int blockHeight;
        uint256 blockHash;
    };

    ContractStateCache();
    ~ContractStateCache();
    SnapShot& getSnapShot();
    void clearSnapShot()
    {
        snapShot.clear();
    }
    void setSnapShot(SnapShot& snapShot);
    ContractStateCache::BlcokCache* getFirstBlockCache()
    {
        if (blockCache.size() == 0)
            return nullptr;
        return &blockCache[0];
    }
    std::vector<BlcokCache>* getBlockCache()
    {
        return &blockCache;
    }
    void setBlockCache(std::vector<BlcokCache>& blockCache)
    {
        this->blockCache = blockCache;
    }


private:
    std::vector<BlcokCache> blockCache;
    SnapShot snapShot;
};

#endif // CONTRACT_CACHE_H
