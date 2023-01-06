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
