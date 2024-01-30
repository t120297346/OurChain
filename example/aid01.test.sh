deploycontract() {
    bitcoin-cli deploycontract ~/Desktop/ourchain/example/aid01.cpp > log.txt
    # 使用 grep 和 awk 从 log.txt 文件中提取合同地址
    contract_address=$(grep "contract address" log.txt | awk -F'"' '{print $4}')
    rm log.txt
    echo "$contract_address"
}

contract_address=$(deploycontract)
echo "$contract_address"
bitcoin-cli generate 1
bitcoin-cli callcontract "$contract_address" "registerNewUser" "user1" "password1"
bitcoin-cli generate 1
bitcoin-cli dumpcontractmessage "$contract_address" "login" "user1" "password1" > log.txt
user_address=$(grep "aid" log.txt | awk -F'"' '{print $4}')
echo "$user_address"
rm log.txt
bitcoin-cli callcontract "$contract_address" "setCoin" "$user_address" "coin1"
bitcoin-cli callcontract "$contract_address" "setCoin" "$user_address" "coin2"
bitcoin-cli callcontract "$contract_address" "setCoin" "$user_address" "coin3"
bitcoin-cli callcontract "$contract_address" "setCoin" "$user_address" "coin4"
bitcoin-cli callcontract "$contract_address" "setCoin" "$user_address" "coin5"
bitcoin-cli callcontract "$contract_address" "setCoin" "$user_address" "coin5"
bitcoin-cli generate 1
bitcoin-cli dumpcontractmessage "$contract_address" "getCoins" "$user_address"
bitcoin-cli callcontract "$contract_address" "removeCoin" "$user_address" "coin5"
bitcoin-cli callcontract "$contract_address" "removeCoin" "$user_address" "coin4"
bitcoin-cli callcontract "$contract_address" "removeCoin" "$user_address" "coin3"
bitcoin-cli dumpcontractmessage "$contract_address" "getCoins" "$user_address"
bitcoin-cli generate 1
bitcoin-cli dumpcontractmessage "$contract_address" "getCoins" "$user_address"