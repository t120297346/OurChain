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
// #include <boost/uuid/uuid.hpp>
// #include <boost/uuid/uuid_generators.hpp>

#include "ourcontract.h"

/* call stack */
FILE* in;
FILE* out;

/* argv of ourcontract-rt */
static int runtime_argc;
static char** runtime_argv = NULL;

static inline const char* get_contracts_dir()
{
    return runtime_argv[1];
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

    int ret = contract_main(argc, argv);

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

std::string* state_read()
{
    int flag = 0;
    fwrite((void*)&flag, sizeof(int), 1, out);
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

int state_write(const std::string* buf)
{
    size_t buf_size =  sizeof(char) * (buf->length() + 1);
    int ret = fwrite((void*)&buf_size, sizeof(int), 1, out);
    if(ret <= 0){
        err_printf("state_write: write control code error\n");
        return -1;
    }
    ret = fwrite((void*)buf->c_str(), buf_size, 1, out);
    if(ret <= 0){
        err_printf("state_write: write control code error\n");
        return -1;
    }
    fflush(out);
    return ret;
}
