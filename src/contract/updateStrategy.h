#ifndef CONTRACT_UPDATESTRATEGY_H
#define CONTRACT_UPDATESTRATEGY_H

#include "chain.h"
#include "contract/cache.h"
#include "primitives/transaction.h"
#include "util.h"
#include "validation.h"

enum UpdateStrategyType {
    UpdateStrategyTypeUnDo = 0,
    UpdateStrategyTypeRebuild = 1,
    UpdateStrategyTypeContinue = 2,
    UpdateStrategyTypeRollback = 3
};

class UpdateStrategy
{
public:
    virtual UpdateStrategyType getName() { return UpdateStrategyType::UpdateStrategyTypeUnDo; };
    virtual bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams) { return false; }
};


class UpdateStrategyRebuild : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeRebuild;
    }
};

class UpdateStrategyUnDo : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeUnDo;
    }
};

class UpdateStrategyContinue : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeContinue;
    }
};

class UpdateStrategyRollback : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeRollback;
    }
};

class UpdateStrategyFactory
{
public:
    UpdateStrategy* createUpdateStrategy(CChain& chainActive, ContractStateCache* cache);
};

#endif // CONTRACT_UPDATESTRATEGY_H