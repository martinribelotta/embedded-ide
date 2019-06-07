#!/bin/bash

set -e

sudo add-apt-repository --yes ppa:beineri/opt-qt593-trusty
sudo apt-get update -qq

cd /tmp/
wget -c "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod +x appimagetool*AppImage
./appimagetool*AppImage --appimage-extract
sudo cp squashfs-root/usr/bin/* /usr/local/bin/
sudo cp -r squashfs-root/usr/lib/appimagekit /usr/local/lib/
sudo chmod +rx /usr/local/lib/appimagekit
cd -

sudo apt-get install -y xpra build-essential qt59base qt59tools qt59svg qt59imageformats qt59x11extras libglu1-mesa-dev wget fuse

mkdir -p /tmp/qsci
cp ./ci/BuildQSCI.mk /tmp/qsci
cd /tmp/qsci
make -f BuildQSCI.mk
cd -
