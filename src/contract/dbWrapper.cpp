#include "contract/dbWrapper.h"

boost::mutex tmp_contract_state_access;

ContractDBWrapper::ContractDBWrapper(std::string name)
{
    leveldb::Options options;
    options.create_if_missing = true;
    fs::path path = getContractDBPath(name);
    TryCreateDirectories(path);
    mystatus = leveldb::DB::Open(options, path.string(), &db);
    assert(mystatus.ok());
}

ContractDBWrapper::ContractDBWrapper(std::string checkPointBlockHash, bool isCheckPoint)
{
    assert(isCheckPoint);
    leveldb::Options options;
    options.create_if_missing = false;
    fs::path path = getContractCheckPointPath(checkPointBlockHash);
    mystatus = leveldb::DB::Open(options, path.string(), &db);
    assert(mystatus.ok());
}

ContractDBWrapper::~ContractDBWrapper()
{
    delete db;
    db = nullptr;
}

leveldb::Status ContractDBWrapper::getStatus()
{
    return mystatus;
}
bool ContractDBWrapper::isOk()
{
    return mystatus.ok();
}
// set critical save
void ContractDBWrapper::setCriticalSave()
{
    writeOptions.sync = true;
}
leveldb::Iterator* ContractDBWrapper::getIterator()
{
    return db->NewIterator(leveldb::ReadOptions());
}

void ContractDBWrapper::transferAllState(ContractDBWrapper& target)
{
    target.clearAllStates();
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        target.setState(it->key().ToString(), it->value().ToString());
    }
    delete it;
}

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

void ContractDBWrapper::saveDuplicateState(fs::path path)
{
    leveldb::DB* newdb;
    leveldb::Options options;
    options.create_if_missing = true;
    boost::mutex::scoped_lock lock(tmp_contract_state_access);
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &newdb);
    assert(status.ok());
    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        status = newdb->Put(writeOptions, it->key(), it->value());
        if (!status.ok()) {
            LogPrintf("saveCheckPoint: %s, should rebuild\n", status.ToString());
            remove_all(path);
            assert(false);
        }
    }
    delete it;
    delete newdb;
}

// save contract checkPoint
void ContractDBWrapper::saveCheckPoint(std::string tipBlockHash)
{
    fs::path path = getContractCheckPointPath(tipBlockHash);
    TryCreateDirectories(path);
    saveDuplicateState(path);
}

void ContractDBWrapper::saveTmpState()
{
    fs::path path = getContractDBPath("tmp");
    TryCreateDirectories(path);
    // overwrite all states
    saveDuplicateState(path);
}

// find check point
bool ContractDBWrapper::findCheckPoint(std::string tipBlockHash)
{
    fs::path path = getContractCheckPointPath(tipBlockHash);
    return exists(path);
}