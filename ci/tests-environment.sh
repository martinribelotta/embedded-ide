#!/bin/bash

set -e
set -x

# sudo add-apt-repository --yes ppa:beineri/opt-qt-5.12.8-xenial

sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
echo "deb http://pkg.mxe.cc/repos/apt trusty main" | sudo tee /etc/apt/sources.list.d/mxeapt.list
sudo apt-get update -qq --allow-unauthenticated

sudo fallocate -l 1G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

sudo wget https://raw.githubusercontent.com/martinribelotta/pydeployqt/master/deploy.py -O /usr/bin/pydeployqt
sudo chmod a+x /usr/bin/pydeployqt

MXE=mxe-${MXE_TRIPLE}
#sudo apt-get install -y --allow-unauthenticated -o Dpkg::Options::="--force-overwrite" \
	#wget fuse gcc-8 g++-8 build-essential p7zip-full \
	#qt512base qt512tools qt512svg qt512imageformats qt512x11extras libglu1-mesa-dev \
	#${MXE}-gcc ${MXE}-g++ \
	#${MXE}-qtbase ${MXE}-qtsvg ${MXE}-qscintilla2 ${MXE}-qttools
#export QTDIR=$(readlink -f /opt/qt*/)

sudo apt-get install -y --allow-unauthenticated -o Dpkg::Options::="--force-overwrite" \
	wget fuse gcc-8 g++-8 build-essential p7zip-full \
	libglu1-mesa-dev libxkbcommon-dev libxkbcommon-x11-0 \
	${MXE}-gcc ${MXE}-g++ \
	${MXE}-qtbase ${MXE}-qtsvg ${MXE}-qscintilla2 ${MXE}-qttools

sudo bash ci/extract-qt-installer
export QTDIR=$(readlink -f /opt/qt*/5.12.*/gcc_64)

gcc --version
# sudo update-alternatives --remove-all gcc
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 90 --slave /usr/bin/g++ g++ /usr/bin/g++-8
gcc --version

mkdir -p /tmp/qsci
cp ./ci/BuildQSCI.mk /tmp/qsci
cd /tmp/qsci
export PATH=${QTDIR}/bin:${PATH}
make -f BuildQSCI.mk

URLS="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
     https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage \
     https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage"

sudo wget -P /usr/bin $URLS
for url in $URLS
do
	F=$(basename $url)
	P=/usr/bin/$F
	sudo chmod a+x $P
done
