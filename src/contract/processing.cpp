#include "contract/processing.h"
#include "amount.h"
#include "base58.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "uint256.h"
#include "util.h"

#include <fstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BYTE_READ_STATE 0
#define BYTE_SEND_TO_ADDRESS -1
#define BYTE_SEND_TO_CONTRACT -2
#define BYTE_CALL_CONTRACT -3

static fs::path contracts_dir;

const static fs::path& GetContractsDir()
{
    if (!contracts_dir.empty()) return contracts_dir;

    contracts_dir = GetDataDir() / "contracts";
    fs::create_directories(contracts_dir);

    return contracts_dir;
}

ContractDBWrapper::ContractDBWrapper()
{
    options.create_if_missing = true;
    fs::path path = GetDataDir() / "contracts" / "index";
    TryCreateDirectories(path);
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &db);
    if (status.ok()) {
        LogPrintf("Opening ContractLevelDB in %s\n", path.string());
    }
};

ContractDBWrapper::~ContractDBWrapper()
{
    delete db;
    db = nullptr;
};

void ContractDBWrapper::setState(std::string key, void* buf, size_t size)
{
    leveldb::Slice valueSlice = leveldb::Slice((const char*)buf, size);
    mystatus = db->Put(leveldb::WriteOptions(), key, valueSlice);
    // LogPrintf("put result: %d\n", mystatus.ok());
    assert(mystatus.ok());
};

std::string ContractDBWrapper::getState(std::string key)
{
    std::string buf;
    mystatus = db->Get(leveldb::ReadOptions(), key, &buf);
    // LogPrintf("get result: %d\n", mystatus.ok());
    return buf;
};

static void exec_dll(const uint256& contract, const std::vector<std::string>& args, int fd_state_read[2], int fd_state_write[2] )
{
    int fd_error = open((GetContractsDir().string() + "/err").c_str(),
            O_WRONLY | O_APPEND | O_CREAT,
            0664);
    dup2(fd_error, STDERR_FILENO);
    close(fd_error);
    // state & TX
    dup2(fd_state_read[1], STDOUT_FILENO);
    dup2(fd_state_write[0], STDIN_FILENO);
    close(fd_state_read[0]);
    close(fd_state_read[1]);
    close(fd_state_write[0]);
    close(fd_state_write[1]);
    const char** argv = (const char**)malloc((args.size() + 4) * sizeof(char*));
    argv[0] = "ourcontract-rt";
    argv[1] = GetContractsDir().string().c_str();
    std::string hex_ctid(contract.GetHex());
    argv[2] = hex_ctid.c_str();
    for (unsigned i = 0; i < args.size(); i++)
        argv[i + 3] = args[i].c_str();
    argv[args.size() + 3] = NULL;
    execvp("ourcontract-rt", (char* const*)argv);
    exit(EXIT_FAILURE);
}

static void read_state_from_db(ContractDBWrapper &cdb, std::string &hex_ctid, int &flag, FILE* pipe_state_write){
    std::string newbuffer = cdb.getState(hex_ctid.c_str());
    if (cdb.mystatus.ok()) {
        flag = newbuffer.size();
        fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
        fflush(pipe_state_write);
        fwrite((void*)newbuffer.data(), newbuffer.size(), 1, pipe_state_write);
        fflush(pipe_state_write);
    } else {
        // client will not recive data after flag is 0
        fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
        fflush(pipe_state_write);
    }
}

static int read_buffer_size(FILE* pipe_state_read){
    int size;
    int ret = fread((void*)&size, sizeof(int), 1, pipe_state_read);
    assert(ret >= 0);
    return size;
}

static void write_state_to_db(ContractDBWrapper &cdb, std::string &hex_ctid, int &size, FILE* pipe_state_read){
    // LogPrintf("message recieve write %d\n", flag);
    // state.resize(flag);
    char* tmp = (char*)malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    cdb.setState(hex_ctid.c_str(), tmp, size);
    assert(ret >= 0);
    free(tmp);
}

static std::string read_contract_address(FILE* pipe_state_read){
    int size = sizeof(char) * 64;
    char* tmp = (char*)malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    assert(ret >= 0);
    std::string address(tmp, 64);
    free(tmp);
    return address;
}

static std::string write_state_as_string(ContractDBWrapper &cdb, std::string &hex_ctid, int &size, FILE* pipe_state_read){
    char* tmp = (char*)malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    assert(ret >= 0);
    return std::string(tmp);
}

static int call_mkdll(const uint256& contract)
{
    int pid, status;
    pid = fork();
    if (pid == 0) {
        int fd = open((GetContractsDir().string() + "/err").c_str(),
            O_WRONLY | O_APPEND | O_CREAT,
            0664);
        dup2(fd, STDERR_FILENO);
        close(fd);
        execlp("ourcontract-mkdll",
            "ourcontract-mkdll",
            GetContractsDir().string().c_str(),
            contract.GetHex().c_str(),
            NULL);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return -1;
    if (WEXITSTATUS(status) != 0) return -1;
    return 0;
}

static int call_rt(const uint256& contract, const std::vector<std::string>& args, std::vector<CTxOut>& vTxOut, std::vector<uchar>& state, std::vector<Contract>& nextContract)
{
    int pid, status;
    int fd_state_read[2], fd_state_write[2];
    if (pipe(fd_state_read) == -1) return -1;
    if (pipe(fd_state_write) == -1) return -1;

    pid = fork();
    if (pid == 0) {
        exec_dll(contract, args, fd_state_read, fd_state_write);
    }

    // read or write state or send money
    close(fd_state_read[1]);
    close(fd_state_write[0]);

    FILE* pipe_state_read = fdopen(fd_state_read[0], "rb");
    FILE* pipe_state_write = fdopen(fd_state_write[1], "wb");

    ContractDBWrapper cdb;
    std::string hex_ctid(contract.GetHex());
    int flag;
    while (fread((void*)&flag, sizeof(int), 1, pipe_state_read) != 0) {
        if (flag == 0) { // read state
            auto targetAddress = read_contract_address(pipe_state_read);
            read_state_from_db(cdb, targetAddress, flag, pipe_state_write);
        } else if (flag == 1) { // write state
            int size = read_buffer_size(pipe_state_read);
            write_state_to_db(cdb, hex_ctid, size, pipe_state_read);
        } else if(flag == 2) { // check mode (pure = 0, not pure = 1)
            flag = 1;
            fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
            fflush(pipe_state_write);
        } else {
            break;
        }
    }
    fclose(pipe_state_read);
    fclose(pipe_state_write);

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return -1;
    if (WEXITSTATUS(status) != 0) return -1;
    return 0;
}

std::string call_rt_pure(const uint256& contract, const std::vector<std::string>& args){
    int pid, status;
    int fd_state_read[2], fd_state_write[2];
    if (pipe(fd_state_read) == -1) return "";
    if (pipe(fd_state_write) == -1) return "";

    pid = fork();
    if (pid == 0) {
        exec_dll(contract, args, fd_state_read, fd_state_write);
    }

    // read or write state or send money
    close(fd_state_read[1]);
    close(fd_state_write[0]);

    FILE* pipe_state_read = fdopen(fd_state_read[0], "rb");
    FILE* pipe_state_write = fdopen(fd_state_write[1], "wb");

    ContractDBWrapper cdb;
    std::string hex_ctid(contract.GetHex());
    int flag;
    std::string result = "";
    while (fread((void*)&flag, sizeof(int), 1, pipe_state_read) != 0) {
        if (flag == 0) { // read state
            auto targetAddress = read_contract_address(pipe_state_read);
            read_state_from_db(cdb, targetAddress, flag, pipe_state_write);
        } else if (flag == 1) { // write state
            int size = read_buffer_size(pipe_state_read);
            result = write_state_as_string(cdb, hex_ctid, size, pipe_state_read);
        } else if(flag == 2) { // check mode (pure = 0, not pure = 1)
            flag = 0;
            fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
            fflush(pipe_state_write);
        } else {
            break;
        }
    }
    fclose(pipe_state_read);
    fclose(pipe_state_write);

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return "";
    if (WEXITSTATUS(status) != 0) return "";
    return result;
}

bool ProcessContract(const Contract& contract, std::vector<CTxOut>& vTxOut, std::vector<uchar>& state, CAmount balance, std::vector<Contract>& nextContract)
{
    if (contract.action == contract_action::ACTION_NEW) {
        fs::path new_dir = GetContractsDir() / contract.address.GetHex();
        fs::create_directories(new_dir);
        std::ofstream contract_code(new_dir.string() + "/code.cpp");
        contract_code.write(contract.code.c_str(), contract.code.size());
        contract_code.close();

        if (call_mkdll(contract.address) < 0) {
            /* TODO: clean up files */
            return false;
        }

        if (call_rt(contract.address, contract.args, vTxOut, state, nextContract) < 0) {
            /* TODO: perform state recovery */
            return false;
        }
    } else if (contract.action == contract_action::ACTION_CALL) {
        if (call_rt(contract.address, contract.args, vTxOut, state, nextContract) < 0) {
            /* TODO: perform state recovery */
            return false;
        }
    }

    return true;
}
