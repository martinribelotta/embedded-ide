#!/bin/bash

set -e

sudo add-apt-repository --yes ppa:beineri/opt-qt593-trusty
sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
sudo apt-get update -qq
sudo fallocate -l 1G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
cd /tmp/
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
sudo cp linuxdeployqt-continuous-x86_64.AppImage /usr/bin/linuxdeployqt
sudo chmod a+x /usr/bin/linuxdeployqt
cd -

sudo apt-get install -y gcc-7 xpra build-essential qt59base qt59tools qt59svg qt59imageformats qt59x11extras libglu1-mesa-dev wget fuse

mkdir -p /tmp/qsci
cp ./ci/BuildQSCI.mk /tmp/qsci
cd /tmp/qsci
make -f BuildQSCI.mk
cd -
