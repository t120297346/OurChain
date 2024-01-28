git pull
docker buildx build --no-cache -t our-chain -f ./Dockerfile.prod --platform linux/amd64 . 
docker tag our-chain leon1234858/our-chain
docker push leon1234858/our-chain