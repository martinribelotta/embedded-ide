#!/bin/bash

set -x

source /opt/qt*/bin/qt*-env.sh

set -e

qmake --version

VERSION=$(git rev-parse --short HEAD)

INSTALL_DIR=/tmp/embedded-ide
APP_IMAGE_NAME=Embedded_IDE-x86_64.AppImage
DEPLOY_OPT="-no-translations -verbose=2 -executable=$INSTALL_DIR/usr/bin/embedded-ide"
DESKTOP_FILE=$INSTALL_DIR/usr/share/applications/embedded-ide.desktop

wget https://github.com/martinribelotta/embedded-ide-builder/blob/master/linux-x86_64/universal-ctags?raw=true -O /tmp/universal-ctags

echo ************** LINUX BUILD ***********************

/opt/qt*/bin/qmake CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j
make install INSTALL_ROOT=${INSTALL_DIR}
linuxdeployqt $DESKTOP_FILE $DEPLOY_OPT -appimage
linuxdeployqt $INSTALL_DIR/usr/bin/qtshdialog -no-translations -verbose=2 -no-plugins
linuxdeployqt $INSTALL_DIR/usr/bin/socketwaiter -no-translations -verbose=2 -no-plugins
cp /opt/qt*/lib/libQt5Svg.so.5 $INSTALL_DIR/usr/lib
cp /opt/qt*/plugins/imageformats/libqsvg.so $INSTALL_DIR/usr/plugins/imageformats/
cp /tmp/universal-ctags $INSTALL_DIR/usr/bin
chmod a+x $INSTALL_DIR/usr/bin/universal-ctags
linuxdeployqt $DESKTOP_FILE -appimage

echo ************** WINDOWS BUILD ***********************

make distclean
export MXE_PREFIX=i686-w64-mingw32.shared
export MXE=/usr/lib/mxe/usr
export MXEQT=${MXE}/i686-w64-mingw32.shared/qt5
export PATH=${MXE}/bin:${PATH}
${MXEQT}/bin/qmake CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j4
pydeployqt --objdump ${MXE_PREFIX}-objdump ${PWD}/build/embedded-ide.exe \
	--libs ${MXE}/${MXE_PREFIX}/bin/:${MXEQT}/bin/:${MXEQT}/lib/ \
	--extradll Qt5Svg.dll:libjpeg-9.dll \
	--qmake ${MXEQT}/bin/qmake
mv build embedded-ide
zip -9 -r Embedded_IDE-${VERSION}-win32.zip embedded-ide
