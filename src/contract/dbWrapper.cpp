#include "contract/dbWrapper.h"

ContractDBWrapper::ContractDBWrapper()
{
    leveldb::Options options;
    options.create_if_missing = true;
    TryCreateDirectories(path);
    TryCreateDirectories(cachePath);
    TryCreateDirectories(checkPointPath);
    mystatus = leveldb::DB::Open(options, path.string(), &db);
    assert(mystatus.ok());
    mystatus = leveldb::DB::Open(options, cachePath.string(), &cacheDB);
    assert(mystatus.ok());
}

ContractDBWrapper::~ContractDBWrapper()
{
    delete db;
    delete cacheDB;
    db = nullptr;
    cacheDB = nullptr;
}

std::string ContractDBWrapper::getState(std::string key)
{
    std::string buf;
    mystatus = cacheDB->Get(leveldb::ReadOptions(), key, &buf);
    if (mystatus.ok()) {
        return buf;
    }
    mystatus = db->Get(leveldb::ReadOptions(), key, &buf);
    if (mystatus.ok()) {
        leveldb::Slice valueSlice = leveldb::Slice(buf);
        mystatus = cacheDB->Put(leveldb::WriteOptions(), key, valueSlice);
        assert(mystatus.ok());
        return buf;
    }
    return "";
}

void ContractDBWrapper::setState(std::string key, void* buf, size_t size)
{
    leveldb::Slice valueSlice = leveldb::Slice((const char*)buf, size);
    mystatus = cacheDB->Put(leveldb::WriteOptions(), key, valueSlice);
    // LogPrintf("put result: %d\n", mystatus.ok());
    assert(mystatus.ok());
}