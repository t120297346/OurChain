#include "contract/dbWrapper.h"

std::shared_mutex tmp_contract_db_mutex;

ContractDBWrapper::ContractDBWrapper(std::string name)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    fs::path path = getContractDBPath(name);
    TryCreateDirectories(path);
    mystatus = rocksdb::DB::Open(options, path.string(), &db);
    assert(mystatus.ok());
}

ContractDBWrapper::ContractDBWrapper(std::string name, std::string mode)
{
    assert(mode == "readOnly" || mode == "checkPoint");
    rocksdb::Options options;
    if (mode == "readOnly") {
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        fs::path path = getContractDBPath(name);
        mystatus = rocksdb::DB::OpenForReadOnly(options, path.string(), &db);
    } else if (mode == "checkPoint") {
        options.create_if_missing = false;
        fs::path path = getContractCheckPointPath(name);
        mystatus = rocksdb::DB::Open(options, path.string(), &db);
    }
    assert(mystatus.ok());
}

ContractDBWrapper::~ContractDBWrapper()
{
    delete db;
    db = nullptr;
}

rocksdb::Status ContractDBWrapper::getStatus()
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
rocksdb::Iterator* ContractDBWrapper::getIterator()
{
    return db->NewIterator(rocksdb::ReadOptions());
}

void ContractDBWrapper::transferAllState(ContractDBWrapper& target)
{
    target.clearAllStates();
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        target.setState(it->key().ToString(), it->value().ToString());
    }
    delete it;
}

std::string ContractDBWrapper::getState(std::string key)
{
    std::string value;
    mystatus = db->Get(rocksdb::ReadOptions(), key, &value);
    return value;
}

void ContractDBWrapper::setState(std::string key, std::string value)
{
    mystatus = db->Put(rocksdb::WriteOptions(), key, value);
}

void ContractDBWrapper::deleteState(std::string key)
{
    mystatus = db->Delete(rocksdb::WriteOptions(), key);
}

std::map<std::string, std::string> ContractDBWrapper::getAllStates()
{
    std::map<std::string, std::string> states;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
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
    rocksdb::DB* newdb;
    rocksdb::Options options;
    options.create_if_missing = true;
    WriteLock w_lock(tmp_contract_db_mutex);
    rocksdb::Status status = rocksdb::DB::Open(options, path.string(), &newdb);
    assert(status.ok());
    rocksdb::WriteOptions writeOptions;
    writeOptions.sync = true;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        status = newdb->Put(writeOptions, it->key(), it->value());
        if (!status.ok()) {
            LogPrintf("saveCheckPoint: %s, should rebuild\n", status.ToString());
            remove_all(path);
            assert(false);
        }
    }
    w_lock.unlock();
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