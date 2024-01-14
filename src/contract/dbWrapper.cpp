#include "contract/dbWrapper.h"

std::string ContractDBWrapper::getState(std::string key)
{
    std::string value;
    mystatus = db->Get(leveldb::ReadOptions(), key, &value);
    return value;
}

void ContractDBWrapper::setState(std::string key, std::string value)
{
    mystatus = db->Put(leveldb::WriteOptions(), key, value);
}

void ContractDBWrapper::deleteState(std::string key)
{
    mystatus = db->Delete(leveldb::WriteOptions(), key);
}

std::map<std::string, std::string> ContractDBWrapper::getAllStates()
{
    std::map<std::string, std::string> states;
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        states[it->key().ToString()] = it->value().ToString();
    }
    assert(it->status().ok());
    delete it;
    return states;
}

void ContractDBWrapper::clearAllStates()
{
    std::map<std::string, std::string> states = getAllStates();
    for (auto it = states.begin(); it != states.end(); it++) {
        deleteState(it->first);
        assert(isOk());
    }
}