# dev by docker

## 環境設置

參考 https://www.docker.com/ 安裝 docker

參考 https://www.docker.com/products/docker-desktop/ 安裝 docker desktop

參考 https://code.visualstudio.com/ 安裝 vscode

參考 https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers 安裝 vscode container 插件

## 第一次執行

在本專案根目錄執行

```bash
# 安裝 image
docker build .
# 獲取 id
docker image ls
# 從 image 生成 container 且啟動
docker run -it [IMAGE ID]
```

設置環境變數與啟動測試

```bash
# 貼上設置內容
vim /root/.bitcoin/bitcoin.conf
# 啟動
bitcoind --regtest --daemon
# 停止
bitcoin-cli stop
# 教學
bitcoin-cli help
# 獲取餘額
bitcoin-cli getbalance
# 挖礦給自己
bitcoin-cli generate 1
# 發布合約
bitcoin-cli deploycontract ~/Desktop/ourchain/sampleContract.c
# 執行合約 (can check info in ~/.bitcoin/regtest/contracts)
bitcoin-cli callcontract "contract txid when deploy" "arg1" "arg2" ...
```

設置內容

```
server=1
rpcuser=test
rpcpassword=test
rpcport=8332
rpcallowip=0.0.0.0/0
regtest=1
```

啟動後可以在 Desktop 找到專案

## 重複使用 container

```bash
# 獲取所有 container ID
docker container ls -a
# 啟動之前的 container
docker start [CONTAINER ID]
```

## 檔案編輯與執行

請點開 dev-container 插件, 即可直接編輯 container 內部檔案
可以利用 docker desktop 開啟 container 內部 CMD 執行專案

## 刪除

請利用 docker desktop 照直覺操作即可