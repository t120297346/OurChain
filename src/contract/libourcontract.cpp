#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stack>
// #include <boost/uuid/uuid.hpp>
// #include <boost/uuid/uuid_generators.hpp>

#include "ourcontract.h"

/* call stack */
ContractLocalState::ContractLocalState(std::string *stateStr)
{
    if(stateStr == nullptr){
        this->state = json();
        return;
    }
    this->stateStr = stateStr;
    this->state = json::parse(*stateStr);
}

ContractLocalState::~ContractLocalState()
{
    if(this->stateStr != nullptr){
        delete this->stateStr;
    }
}

json ContractLocalState::getState()
{
    return this->state;
}

void ContractLocalState::setState(json state)
{
    this->state = state;
}

bool ContractLocalState::isStateNull()
{
    return this->state.is_null();
}

void ContractLocalState::setPreState(json preState)
{
    this->preState = preState;
}

json ContractLocalState::getPreState()
{
    return this->preState;
}

thread_local std::stack<ContractLocalState*> call_stack;

/* runtime state pipe*/
FILE* in;
FILE* out;

/* argv of ourcontract-rt */
static int runtime_argc;
static char** runtime_argv = NULL;

static inline const char* get_contracts_dir()
{
    return runtime_argv[1];
}

std::string* physical_state_read(const char* contractAddress)
{
    int flag = 0;
    fwrite((void*)&flag, sizeof(int), 1, out);
    fflush(out);
    fwrite((void*)contractAddress, sizeof(char) * 64, 1, out);
    fflush(out);
    int ret = fread((void*)&flag, sizeof(int), 1, in);
    if(ret <= 0){
        err_printf("state_read: read control code error\n");
        return nullptr;
    }
    if(flag == 0){
        return nullptr;
    }
    char* buf = new char[flag]();
    size_t buf_size =  sizeof(char) * flag;
    ret = fread((void*)buf, buf_size, 1, in);
    if(ret <= 0){
        err_printf("state_read: read data error\n");
        return nullptr;
    }
    std::string* str = new std::string(buf);
    return str;
}

int physical_state_write(const std::string* buf)
{
    int flag = 1;
    int ret = fwrite((void*)&flag, sizeof(int), 1, out);
    if(ret <= 0){
        err_printf("state_write: write control code error\n");
        return -1;
    }
    size_t buf_size =  sizeof(char) * (buf->length() + 1);
    ret = fwrite((void*)&buf_size, sizeof(int), 1, out);
    if(ret <= 0){
        err_printf("state_write: write size error\n");
        return -1;
    }
    ret = fwrite((void*)buf->c_str(), buf_size, 1, out);
    if(ret <= 0){
        err_printf("state_write: write buffer error\n");
        return -1;
    }
    fflush(out);
    return ret;
}

bool check_runtime_can_write_db()
{
    if(call_stack.size() > 1){
        // only base contract can write state to db
        return false;
    }
    int flag = 2;
    fwrite((void*)&flag, sizeof(int), 1, out);
    fflush(out);
    int ret = fread((void*)&flag, sizeof(int), 1, in);
    if(ret <= 0){
        err_printf("check_runtime_can_write_db: read mode code error\n");
        return false;
    }
    if(flag == 1){
        return true; // only 1 can write state to db
    }else if(flag == 0){
        return false;
    }
    err_printf("check_runtime_can_write_db: read mode code error\n");
    return false;
}

int start_runtime(int argc, char** argv)
{
    if (runtime_argv != NULL) {
        err_printf("start_runtime: cannot be called more than once\n");
        return EXIT_FAILURE;
    }

    runtime_argc = argc;
    runtime_argv = argv;

    if (argc < 3) {
        err_printf("usage: ourcontract-rt [CONTRACTS DIR] [CONTRACT] [ARG 1] [ARG 2] ...\n");
        return EXIT_FAILURE;
    }

    in = fdopen(fileno(stdin), "rb");
    out = fdopen(fileno(stdout), "wb");

    return call_contract(argv[2], argc - 2, argv + 2);
}

int call_contract(const char* contract, int argc, char** argv)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "%s/%s/code.so", get_contracts_dir(), contract) >= PATH_MAX) {
        err_printf("call_contract: path too long\n");
        return EXIT_FAILURE;
    }
    void* handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL) {
        err_printf("call_contract: dlopen failed %s\n", filename);
        return EXIT_FAILURE;
    }
    int (*contract_main)(int, char**) = reinterpret_cast<int (*)(int, char**)>(dlsym(handle, "contract_main"));
    if (contract_main == NULL) {
        err_printf("call_contract: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    call_stack.push(new ContractLocalState(physical_state_read(contract)));
    int ret = contract_main(argc, argv);
    if(call_stack.size() == 1){
        // base contract can write state to DB or pure output
        std::string* buf = new std::string(call_stack.top()->getState().dump());
        // clear state
        delete call_stack.top();
        call_stack.pop();
        // write state to DB or pure output
        auto ret = physical_state_write(buf);
        if (ret < 0) {
            err_printf("call_contract physical_state_write error: %s\n", dlerror());
            return EXIT_FAILURE;
        }
        delete buf;
    }else {
        // nested contract can only write state to parent contract
        std::string* buf = new std::string(call_stack.top()->getState().dump());
        // clear state
        delete call_stack.top();
        call_stack.pop();
        // write state to parent contract
        call_stack.top()->setPreState(json::parse(*buf));
        delete buf;
    }
    

    dlclose(handle);
    return ret;
}

int err_printf(const char* format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vfprintf(stderr, format, args);

    va_end(args);
    return ret;
}

json state_read()
{
    return call_stack.top()->getState();
}

void state_write(json buf)
{
    call_stack.top()->setState(buf);
}

bool state_exist()
{
    return !call_stack.top()->isStateNull();
}

json pre_state_read()
{
    return call_stack.top()->getPreState();
}
