#!/bin/bash

if [ -z "$1" ]; then
    echo "need to set which bitcoin you wnat to install" 
    exit
fi

cd $1
echo "choose mode 'reconfig' or not"
read mode

if [[ $mode == "reconfig" ]]; then
    ./autogen.sh
    ./configure --disable-gui --disable-tests
fi

make -j4

