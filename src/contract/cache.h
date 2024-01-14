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

    ContractStateCache()
    {
        blockCache.clear();
        snapShot = new SnapShot(std::string("current_cache"));
    }
    ~ContractStateCache()
    {
        delete snapShot;
    }
    SnapShot* getSnapShot();
    void clearSnapShot()
    {
        snapShot->clear();
    }
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
    SnapShot* snapShot;
};

#endif // CONTRACT_CACHE_H
