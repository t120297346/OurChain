#ifndef BITCOIN_CONTRACT_OURCONTRACT_H
#define BITCOIN_CONTRACT_OURCONTRACT_H

#define BYTE_READ_STATE 0
#define BYTE_SEND_TO_ADDRESS -1
#define BYTE_SEND_TO_CONTRACT -2
#define BYTE_CALL_CONTRACT -3

#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <json.hpp>

using json = nlohmann::json;

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmount;

#define COIN ((CAmount)100000000)
#define CENT ((CAmount)1000000)

/** No amount larger than this (in satoshi) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Bitcoin
 * currently happens to be less than 21,000,000 BTC for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */
static const CAmount MAX_MONEY = 21000000 * COIN;
inline bool MoneyRange(const CAmount nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

static const int64_t UPPER_BOUND = 1000000000000000000LL - 1LL;

/* non-reentrant entry point of runtime */
int start_runtime(int argc, char** argv);

/* contract call can be nested */
int call_contract(const char* contract, int argc, char** argv);

/* print to runtime error log */
int err_printf(const char* format, ...);

/* read the state file of the calling contract */
json state_read();

/* write the state file of the calling contract */
void state_write(json buf);

/* check if state exist*/
bool state_exist();

/* check if runtime can write db*/
bool check_runtime_can_write_db();

/* read pre state*/
json pre_state_read();

class ContractLocalState
{
private:
    std::string* stateStr;
    json state;
    json preState;
public:
    ContractLocalState(std::string *stateStr);
    ~ContractLocalState();
    void setState(json state);
    void setPreState(json preState);
    json getPreState();
    json getState();
    bool isStateNull();
};

#endif // BITCOIN_CONTRACT_OURCONTRACT_H