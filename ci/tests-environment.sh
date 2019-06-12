#!/bin/bash

set -e
set -x

sudo add-apt-repository --yes ppa:beineri/opt-qt593-trusty
sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
echo "deb http://pkg.mxe.cc/repos/apt trusty main" \
    | sudo tee /etc/apt/sources.list.d/mxeapt.list
#echo 'APT::Get::AllowUnauthenticated "true";' \
#    | sudo tee /etc/apt/apt.conf.d/99myown
sudo apt-get update -qq --allow-unauthenticated
sudo fallocate -l 1G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

MXE=mxe-i686-w64-mingw32.static-


sudo apt-get install -y --allow-unauthenticated \
	gcc-8 g++-8 build-essential \
	qt59base qt59tools qt59svg qt59imageformats qt59x11extras libglu1-mesa-dev \
	wget fuse \
	${MXE}qtbase ${MXE}qtsvg ${MXE}qtdeclarative ${MXE}gcc ${MXE}g++ ${MXE}qscintilla2

gcc --version
# sudo update-alternatives --remove-all gcc
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 90 --slave /usr/bin/g++ g++ /usr/bin/g++-8
gcc --version

mkdir -p /tmp/qsci
cp ./ci/BuildQSCI.mk /tmp/qsci
cd /tmp/qsci
make -f BuildQSCI.mk

cd /tmp/
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
sudo cp linuxdeployqt-continuous-x86_64.AppImage /usr/bin/linuxdeployqt
sudo chmod a+x /usr/bin/linuxdeployqt
cd -
