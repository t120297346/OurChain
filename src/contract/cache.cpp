#include "contract/cache.h"

SnapShot::SnapShot()
{
    contractStateMap.clear();
}

SnapShot::~SnapShot()
{
}

SnapShot& SnapShot::operator=(const SnapShot& snapShot)
{
    contractStateMap = snapShot.contractStateMap;
    return *this;
}

void SnapShot::setContractState(uint256 address, json state)
{
    this->contractStateMap[address] = state;
}

json SnapShot::getContractState(uint256 address)
{
    return this->contractStateMap[address];
}

ContractStateCache::ContractStateCache()
{
    blockCache.clear();
    snapShot = SnapShot();
}

ContractStateCache::~ContractStateCache()
{
}

SnapShot& ContractStateCache::getSnapShot()
{
    return snapShot;
}

void ContractStateCache::setSnapShot(SnapShot& snapShot)
{
    this->snapShot = snapShot;
}