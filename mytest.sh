make -j8 && make install && ldconfig
./src/bitcoind --regtest --daemon -txindex
sleep 5
./src/bitcoin-cli generate 101
sleep 5
./src/bitcoin-cli deploycontract ~/Desktop/ourchain/sample.cpp > log.txt
./src/bitcoin-cli generate 1
# 使用 grep 和 awk 从 log.txt 文件中提取合同地址
contract_address=$(grep "contract address" log.txt | awk -F'"' '{print $4}')
rm log.txt
# 输出提取的合同地址
echo "Contract Address: $contract_address"
# 使用合同地址调用合同
./src/bitcoin-cli callcontract "$contract_address" ""
./src/bitcoin-cli generate 1
./src/bitcoin-cli dumpcontractmessage "$contract_address" ""
./src/bitcoin-cli stop